#ifndef SWNT_TYPES_H
#define SWNT_TYPES_H

#include <stdlib.h>
#include <stdint.h>

#define USE_KINECT 0                            /* use a kinect as input */
#define USE_WATER 1                             /* use the water simulation */
#define USE_RGB_SHIFT 0                         /* use an post processing rgb shift effect  */
#define USE_SCENE 0                            
#define USE_GUI 0                               /* use a gui to control the parameters of e.g. particles, sun, water */
#define USE_WEATHER 0                           /* use yahoo weather data */
#define USE_AUDIO 0                             /* use audio effects */
#define USE_EFFECTS 0                           /* use effects, like splashing, etc.. */
#define USE_WATER_BALLS 0                       /* use the water ball simulation */
#define USE_TIDES 0                             /* parse and use the text file (format 13) with tides info */

#define EFFECT_NONE 0
#define EFFECT_MIST 1 // NOT USED ATM
#define EFFECT_EDDY 2
#define EFFECT_SPLASHES 3

#define SOUND_WATER 0
#define SOUND_OCEAN 1


#if USE_WATER_BALLS

class WaterBall;

struct TrackedWaterBall {
  TrackedWaterBall():tracked_id(0),water_ball(NULL){}
  uint32_t tracked_id;
  WaterBall* water_ball;
};

#endif


#endif
