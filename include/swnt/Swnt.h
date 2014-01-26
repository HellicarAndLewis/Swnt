/*
---------------------------------------------------------------------------------
 
                                               oooo
                                               `888
                oooo d8b  .ooooo.  oooo    ooo  888  oooo  oooo
                `888""8P d88' `88b  `88b..8P'   888  `888  `888
                 888     888   888    Y888'     888   888   888
                 888     888   888  .o8"'88b    888   888   888
                d888b    `Y8bod8P' o88'   888o o888o  `V88V"V8P'
 
                                                  www.roxlu.com
                                             www.apollomedia.nl
                                          www.twitter.com/roxlu
 
---------------------------------------------------------------------------------
*/

#ifndef SWNT_H
#define SWNT_H

#include <swnt/Types.h>
#include <swnt/Settings.h>
#include <swnt/Graphics.h>
#include <swnt/Mask.h>
#include <swnt/Tracking.h>
#include <swnt/Flow.h>
#include <swnt/HeightField.h>
#include <swnt/Water.h>
#include <swnt/RGBShift.h>
#include <swnt/Weather.h>
#include <swnt/Scene.h>
#include <swnt/WaterBall.h>

#if USE_KINECT
#  include <swnt/Kinect.h>
#endif

#if USE_EFFECTS
#  include <swnt/Effects.h>
#endif

#if USE_GUI
#  include <swnt/GUI.h>
#endif

#if USE_AUDIO
#  include <swnt/Audio.h>
#endif

#if USE_TIDES
#  include <swnt/Tides.h>
#endif

#if USE_SPIRALS
#  include <swnt/Spirals.h>
#endif

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
  void integrate(float dt);          /* perform a physics integration step */
  void update(float dt);
  void draw();
  void print();                      /* print some debug info */

#if USE_TIDES
  void setTimeOfDay(float t);        /* when t = 0, it's midnight, when t = 1 it's 23:59 */
#endif

  void setTimeOfYear(float t);        /* 0 = 1 january, 1 = 31 dec, used to change the colors */

 private:
  void updateActivityLevel();        /* sets the activity level. when there are more people interacting this number will go up, 1 is heighest value, 0 means no ativity */
  void updateWeatherInfo();
  void drawScene();

#if USE_KINECT
  bool setupKinect();
  void updateKinect();
#endif

#if USE_WATER_BALLS
  void updateWaterBalls();
#endif

#if USE_TIDES
  void updateTides();                /* check the current tide information and update the visuals accordingly */
#endif

 public:
  Settings& settings;
  Graphics graphics;                 /* generic helper for rendering GL stuff */
  Mask mask;                         /* does all of the thresholding and masking */
  Tracking tracking;                 /* trackes found blobs using k-means */
  Flow flow;                         /* optical flow that drives the spirals*/
  int state;                         /* used to swith the way and what we draw */

  /* @todo - check which draw_* are used */
  bool draw_flow;                    /* flag that toggles drawing of the flow field */
  bool draw_threshold;               /* flag that toggles drawing of the hand/found blobs */
  bool draw_water;
  bool draw_vortex;
  bool draw_tracking;
  bool draw_gui;                     /* draw the gui */ 
  bool override_with_gui;            /* certain values can be overriden with the gui, like the mask size */
  float time_of_day;                 /* is set in setTimeOfDay() */
  float time_of_year;                /* is set in setTimeOfYear() */
  float ocean_roughness;             /* the roughness of the ocean; used by Water and Heightfield */
  float activity_level;              /* 0 = there is no activity, 1 = a lot of activity */
  
#if USE_EFFECTS
  Effects effects;
#endif

#if USE_KINECT
  Kinect kinect;      
  GLuint rgb_tex;
  GLuint depth_tex;
  unsigned char* rgb_image;
  unsigned char* depth_image;
#endif

  HeightField height_field;

#if USE_WATER
  Water water;
#endif

#if USE_TIDES
  Tides tides;
  uint64_t tides_timeout;            /* we will update the tides every 10 minutes */
#endif

#if USE_RGB_SHIFT
  RGBShift rgb_shift;
#endif

#if USE_SCENE
  Scene scene;
#endif

#if USE_WEATHER
  Weather weather;          /* fetches weather info from yahoo */
  WeatherInfo weather_info; /* the retrieved weather info */
  bool has_weather_info;    /* is set to true when weather_info contians valid data */
#endif

#if USE_AUDIO
  Audio audio;
#endif

#if USE_WATER_BALLS
  WaterBallDrawer ball_drawer;
  std::vector<TrackedWaterBall> tracked_balls;
  std::vector<vec2> flush_points; /* the positions where the tracked balls started to flush, this is used to apply a force onto the water*/
#endif

#if USE_SPIRALS
  Spirals spirals;
#endif


#if USE_GUI
  GUI gui;
#endif

  /* matrices for rendering the ocean */
  mat4 view_matrix;                  /* camera view matrix */
  mat4 persp_matrix;                 /* perspective matrix we use to render the ocean */
};

#endif

// Some references
// - http://www.youtube.com/watch?feature=player_detailpage&v=Knc_Trx_-5A#t=48
// - http://static1.wikia.nocookie.net/__cb20120309090130/runescape/images/9/9d/Water.png
// - http://runescape.wikia.com/wiki/Waterfall_Quest/Quick_guide
// - http://runescape.wikia.com/wiki/Waterfall_Dungeon
// - http://static2.wikia.nocookie.net/__cb20130921201114/runescape/images/9/91/Waterfall_dungeon_entrance.png
// - http://cache.desktopnexus.com/thumbnails/895336-bigthumbnail.jpg
// - http://i.imgur.com/JOC3F.jpg
// - http://static2.wikia.nocookie.net/__cb20110919145141/runescapeclans/images/3/30/Waterfall_1.png 
// - Runescape The Citadel Waterfall 
// - Waterfal video:          http://www.youtube.com/watch?v=2PzFHPqAyWc 
// - Guild wars:              http://www.youtube.com/watch?feature=player_detailpage&v=efPdUM08Xj8#t=8
// - Guild wars:              http://www.youtube.com/watch?feature=player_detailpage&v=BSHNee0OAvM#t=45
// - Waterfall texture effect http://www.youtube.com/watch?v=CtujsMYzCh4 
// - Waterfall in unity       http://www.youtube.com/watch?v=xysUQYfW29w
// - Toy story 3:             http://www.youtube.com/watch?feature=player_detailpage&v=shr0vprXJm0#t=37
// - Animated dust texture:   http://vimeo.com/26084620 
// - Skylanders:              http://www.youtube.com/watch?feature=player_detailpage&v=4fptHMXlWzs#t=1367
