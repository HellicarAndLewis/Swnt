
// Runescape - nice water effect 
// - http://www.youtube.com/watch?feature=player_detailpage&v=Knc_Trx_-5A#t=48
// - http://static1.wikia.nocookie.net/__cb20120309090130/runescape/images/9/9d/Water.png
// - http://runescape.wikia.com/wiki/Waterfall_Quest/Quick_guide
// - http://runescape.wikia.com/wiki/Waterfall_Dungeon
// - http://static2.wikia.nocookie.net/__cb20130921201114/runescape/images/9/91/Waterfall_dungeon_entrance.png
// - nice: http://cache.desktopnexus.com/thumbnails/895336-bigthumbnail.jpg
// - nice: http://i.imgur.com/JOC3F.jpg
// - nice: http://static2.wikia.nocookie.net/__cb20110919145141/runescapeclans/images/3/30/Waterfall_1.png 
// - google images: Runescape The Citadel Waterfall 
// - waterfal video: http://www.youtube.com/watch?v=2PzFHPqAyWc 
// - REALLY NICE, guild wars: http://www.youtube.com/watch?feature=player_detailpage&v=efPdUM08Xj8#t=8
// - REALLY NICE, guild wars: http://www.youtube.com/watch?feature=player_detailpage&v=BSHNee0OAvM#t=45
// - waterfall texture effect http://www.youtube.com/watch?v=CtujsMYzCh4 
// - waterfall in unity http://www.youtube.com/watch?v=xysUQYfW29w
// - toy story 3: http://www.youtube.com/watch?feature=player_detailpage&v=shr0vprXJm0#t=37
// - animated dust texture: http://vimeo.com/26084620 

#ifndef SWNT_H
#define SWNT_H

#define USE_KINECT 1
#define USE_OCEAN 0
#define USE_WATER 1
#define USE_EFFECTS 0
#define USE_RGB_SHIFT 0
#define USE_SCENE 0

#include <swnt/Settings.h>
#include <swnt/Kinect.h>
#include <swnt/Graphics.h>
#include <swnt/Mask.h>
#include <swnt/Tracking.h>
#include <swnt/Spirals.h>
#include <swnt/Flow.h>
#include <swnt/HeightField.h>
#include <swnt/Water.h>
#include <swnt/Effects.h>
#include <swnt/RGBShift.h>
#include <swnt/Scene.h>
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
  HeightField height_field;
  Water water;
#endif

#if USE_EFFECTS
  Effects effects;
#endif

#if USE_RGB_SHIFT
  RGBShift rgb_shift;
#endif

#if USE_SCENE
  Scene scene;
#endif

  /* matrices for rendering the ocean */
  mat4 view_matrix;                  /* camera view matrix */
  mat4 persp_matrix;                 /* perspective matrix we use to render the ocean */
};

#endif
