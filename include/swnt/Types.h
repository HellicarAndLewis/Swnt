#ifndef SWNT_TYPES_H
#define SWNT_TYPES_H

#include <stdlib.h>
#include <stdint.h>

#define USE_KINECT 1                            /* use a kinect as input */
#define USE_WATER 1                             /* use the water simulation */
#define USE_RGB_SHIFT 0                         /* use an post processing rgb shift effect  */
#define USE_SCENE 0                            
#define USE_GUI 1                               /* use a gui to control the parameters of e.g. particles, sun, water */
#define USE_WEATHER 1                           /* use yahoo weather data */
#define USE_AUDIO 1                             /* use audio effects */
#define USE_EFFECTS 1                           /* use effects, like splashing, etc.. */
#define USE_WATER_BALLS 1                       /* use the water ball simulation */
#define USE_TIDES 1                             /* parse and use the text file (format 13) with tides info */
#define USE_SPIRALS 1

#define EFFECT_NONE 0
#define EFFECT_MIST 1 // NOT USED ATM
#define EFFECT_EDDY 2
#define EFFECT_SPLASHES 3


#define SOUND_WATER_FLOWING 0
#define SOUND_WAVES_CRASHING 1
#define SOUND_GLOOB 2 /* gloob sound when someone enters his hand */
#define SOUND_SPLASH 3 /* played when some redraws his/hers hand */

#if USE_WATER_BALLS

class WaterBall;

struct TrackedWaterBall {
  TrackedWaterBall():tracked_id(0),water_ball(NULL){}
  uint32_t tracked_id;
  WaterBall* water_ball;
};

#endif


#endif
