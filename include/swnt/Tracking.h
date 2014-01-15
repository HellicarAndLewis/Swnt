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

 */
#ifndef SWNT_TRACKING_H
#define SWNT_TRACKING_H

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>

#define ROXLU_USE_OPENGL
#define ROXLU_USE_MATH
#define ROXLU_USE_PNG
#include "tinylib.h"

#include <swnt/Types.h>

class Settings;
class Graphics;


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

 private:
  bool setupGraphics();                               /* setup opengl related objects */
  bool findContours(unsigned char* pixels);           /* returns true when we found contours */
  void updateVertices();                              /* updates the vertices for the contour */
  void clusterPoints();                               /* cluster the found peaks */

 public:
  Settings& settings;
  Graphics& graphics;

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


