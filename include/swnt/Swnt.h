#ifndef SWNT_H
#define SWNT_H

#define USE_KINECT 1
#define USE_OCEAN 1
#define USE_WATER 0

#include <swnt/Settings.h>
#include <swnt/Kinect.h>
#include <swnt/Graphics.h>
#include <swnt/Mask.h>
#include <swnt/Tracking.h>
#include <swnt/Spirals.h>
#include <swnt/Flow.h>
#include <swnt/Water.h>
#include <ocean/Ocean.h>
#include <ocean/WaterGraphics.h>

#define ROXLU_USE_OPENGL
#define ROXLU_USE_MATH
#include <tinylib.h>

#define STATE_NONE 0
#define STATE_RENDER_ALIGN 1  /* render alignment helper */
#define STATE_RENDER_SCENE 2  /* render scene */

class Swnt {

 public:
  Swnt(Settings& settings);
  ~Swnt();
  bool setup();
  void update();
  void draw();

 private:
#if USE_KINECT
  bool setupKinect();
  void updateKinect();
#endif

 public:
  Settings& settings;
  WaterGraphics water_graphics;
  Ocean ocean;
  Graphics graphics;                 /* generic helper for rendering GL stuff */
  Mask mask;                         /* does all of the thresholding and masking */
  Spirals spirals;                   /* draws the spirals and apples forces to particles */
  Tracking tracking;                 /* trackes found blobs using k-means */
  Flow flow;                         /* optical flow that drives the spirals*/
  int state;                         /* used to swith the way and what we draw */
  bool draw_flow;                    /* flag that toggles drawing of the flow field */
  bool draw_spirals;                 /* flag that toggles drawing of the spirals */
  bool draw_threshold;               /* flag that toggles drawing of the hand/found blobs */
  bool draw_water;

  /* kinect input */
#if USE_KINECT
  Kinect kinect;      
  GLuint rgb_tex;
  GLuint depth_tex;
  unsigned char* rgb_image;
  unsigned char* depth_image;
#endif

#if USE_WATER
  Water water;
#endif

  /* matrices for rendering the ocean */
  mat4 view_matrix;                  /* camera view matrix */
  mat4 persp_matrix;                 /* perspective matrix we use to render the ocean */
};

#endif
