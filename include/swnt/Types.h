#ifndef SWNT_TYPES_H
#define SWNT_TYPES_H

#include <stdlib.h>
#include <stdint.h>

#define USE_KINECT 1
#define USE_WATER 1
#define USE_RGB_SHIFT 0
#define USE_SCENE 0
#define USE_GUI 1
#define USE_WEATHER 0
#define USE_AUDIO 0
#define USE_EFFECTS 1
#define USE_WATER_BALLS 0

#define EFFECT_NONE 0
#define EFFECT_MIST 1 // NOT USE ATM
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
