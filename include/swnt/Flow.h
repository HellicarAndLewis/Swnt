/*
  
  Flow
  ----
  Creates a velocity/vectory field

 */
#ifndef SWNT_FLOW_H
#define SWNT_FLOW_H

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/tracking.hpp>

#define ROXLU_USE_OPENGL
#define ROXLU_USE_MATH
#define ROXLU_USE_PNG
#include "tinylib.h"

class Settings;
class Graphics;

class Flow {

 public:
  Flow(Settings& settings, Graphics& graphics);
  bool setup();
  void calc(unsigned char* curr);
  void draw();
  void createVortex(float px, float py);            /* create a vortex at the given percentages, 0.5,0.5 is the center of the field */
  void applyPerlinToField();                        /* apply a perlin noise force to the field */
  void updateFlowTexture();                         /* uploads the current velocities into a texture */
  void print();                                     /* prints some debug info */

 private:
  bool setupGraphics();
  void updateFieldVertices();                       /* update the vertices that are used to draw the vector field */
  void updateVelocityField();

  void dampVelocities();
  void calcFlow(unsigned char* curr);

 public:
  Settings& settings;
  Graphics& graphics;
  unsigned char* prev_image;

  /* OpenCV */
  std::vector<cv::Point2f> prev_good_points;
  std::vector<cv::Point2f> curr_good_points;
  std::vector<unsigned char> status;

  /* Physics */
  int field_size;                                 /* width and height of the velocity field, velocities will hold field_size * field_size entries */
  std::vector<vec2> velocities;                   /* the velocities */
  std::vector<float> heights;                     /* the actual perlin values */
  Perlin perlin;
  GLuint flow_tex; 

  /* GL */
  GLuint field_vao;
  GLuint field_vbo;
  std::vector<vec2> field_vertices;
  size_t field_bytes_allocated;
};

#endif
