/*


  Tracking
  --------

  Uses the thresholded kinect image to do tracking using K-means. Each tracked
  object is stored in Tracking::tracked. We check if a blob was around in the previous
  frame and if some we will set the "matched" member of Tracked to true and increment
  it's age member.

  We use a helper variable called "lost" which gets incremented every time we cannot
  find the blob from a previous frame. When the lost count reaches a certain age we will
  remove the Tracked object. See Tracking::clusterPoints()

  The Tracking class also takes care of the triangulation of the found blobs. We use the 
  Triangle library which has some quirks but is still the best available free solution for
  triangulation (w/o the needs of lots of dependencies). We only use every N-th point on the
  contour for triangulation (n = 5 now).

*/
#ifndef SWNT_TRACKING_H
#define SWNT_TRACKING_H

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>

#include <swnt/Triangulate.h>

#define ROXLU_USE_OPENGL
#define ROXLU_USE_MATH
#define ROXLU_USE_PNG
#include "tinylib.h"

#include <swnt/Types.h>

class Settings;
class Graphics;

static const char* BLOB_VS = ""
  "#version 150\n"
  "uniform mat4 u_pm;"
  "uniform mat4 u_mm;"
  "in vec4 a_pos;"
  "void main() {"
  "   gl_Position = u_pm * u_mm * a_pos; "
  "}"
  "";

static const char* BLOB_FS = ""
  "#version 150\n"
  "uniform vec3 u_blob_color;"
  "out vec4 fragcolor;"
  "void main() {"
  "  fragcolor = vec4(u_blob_color, 1.0);"
  "}"
  "";


// ---------------------------------------------------------

struct BlobVertex {                                     /* vertex that is used to render the triangulated hand. */
  BlobVertex();
  BlobVertex(vec2 p):pos(p){}
  vec2 pos;
};


// ---------------------------------------------------------

class Tracked {

 public:
  Tracked();

 public:
  int32_t age;
  int32_t lost;
  vec2 position;
  bool matched;                                      /* is set to true when we found a new point which matches this one */
  uint32_t id;
};

// ---------------------------------------------------------

class Tracking {

 public:
  Tracking(Settings& settings, Graphics& graphics);
  bool setup();
  void track(unsigned char* pixels);                  /* this will track the blobs in the given grayscale image */
  void draw(float tx, float ty);                      /* draw the contours + translate by `tx` and `ty` */
  void setTimeOfYear(float t);                        /* gets called when the time of year changes and the colors of the blobs needs to be updated */

 private:
  bool setupGraphics();                               /* setup opengl related objects */
  bool findContours(unsigned char* pixels);           /* returns true when we found contours */
  void updateVertices();                              /* updates the vertices for the contour */
  void clusterPoints();                               /* cluster the found peaks */
  
 public:
  Settings& settings;
  Graphics& graphics;

  bool draw_contours;                                 /* draw the contour lines, used while debugging */
#if USE_TRIANGULATION
  bool draw_triangulated_blobs;                       /* draw the triangulated versions of the detected blobs */
#endif
  bool draw_tracking_points;                          /* draw the tracked points, used while debugging */

  /* Triangulates the detected contours. */
#if USE_TRIANGULATION
  Triangulate tri;
#endif

  /* Contour detection + tracking */
  size_t contour_threshold;                           /* we only use contours whos size contains more points then this */
  std::vector<std::vector<cv::Point> > contours;      /* the contours found by CV */
  std::vector<vec2> contour_vertices;                 /* holds all the vertices of the contours */
  std::vector<GLint> contour_offsets;                 /* when we find multiple contours this holds the start */
  std::vector<GLsizei> contour_nvertices;             /* number of vertices per contour */
  size_t tan_offset;
  size_t tan_count;

  /* GL */
  GLuint vao;                                         /* used to draw the contour lines */
  GLuint vbo;                                         /* will hold the vertices of the contour lines */
  size_t allocated_bytes;                             /* used to grow the vbo when necessary */

  /* GL - drawing triangulated blobs */
#if USE_TRIANGULATION
  GLuint blob_vertices_vbo;                           /* vbo that holds the vertices of the triangulated blobs */
  size_t blob_vertices_allocated;                     /* number of allocated bytes for the blob vertices vbo */
  GLuint blob_vao;                                    /* vao, used to rended the triangulated blobs */
  GLuint blob_frag;                                   /* fragment shader for the blobs */
  GLuint blob_vert;                                   /* vertex shader for the blobs */
  GLuint blob_prog;                                   /* program to render the blobs */
  std::vector<GLint> blob_offsets;                    /* offsets for the sperate blobs; used with multi draw */
  std::vector<GLsizei> blob_counts;                   /* number of vertices per blob */
  std::vector<BlobVertex> blob_vertices;              /* the bob vertices */
  GLint u_blob_color;                                 /* uniform to the blob color */
  float blob_scale;
  float blob_offset;
#endif
  
  /* K-Means clustering */
  std::vector<vec2> closest_points;                   /* closest points in the detected contours.. closest to the center. used to detect peaks in blobs that will be tracked */
  std::vector<vec2> closest_dirs;                     /* the vectors from the center to the the closest points. this is used to calculate the points that we need to track */
  std::vector<vec2> closest_centers;                  /* the positions that we track; it's the center between closest_point and the max radius */
  std::vector<vec2> closest_points_history;           /* we keep a history of the closest points which is used with k-means to cluster */
  size_t prev_num_points;                             /* just a helper to make sure the kmeans() function of cv doesn't crash */
  std::vector<Tracked*> tracked;                      /* tracked objects; we use the 'age' to delete old tracked points. this is what you'll want to use for e.g. forming of water drops*/
  uint32_t last_id;                                   /* each tracked object gets a new id and this is the last one created */
  uint32_t num_tracked;                               /* the number of tracked AND matched objects, use after calling track() */
  uint32_t prev_num_tracked;                          /* previous number of tracked and matched objects; can be used to determine new objects */
};

#endif


