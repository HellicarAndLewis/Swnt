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
#define USE_TRIANGULATION 0

#define EFFECT_NONE 0
#define EFFECT_MIST 1 // NOT USED ATM
#define EFFECT_EDDY 2
#define EFFECT_SPLASHES 3

#define SOUND_BACKGROUND0 0
#define SOUND_BACKGROUND1 1
#define SOUND_SPLASH0 2 /* played when some redraws his/hers hand */
#define SOUND_SPLASH1 3 /* another splash */
#define SOUND_SPLASH2 4 /* and another one */

#if USE_WATER_BALLS

class WaterBall;

struct TrackedWaterBall {
  TrackedWaterBall():tracked_id(0),water_ball(NULL){}
  uint32_t tracked_id;
  WaterBall* water_ball;
};

#endif


#endif
