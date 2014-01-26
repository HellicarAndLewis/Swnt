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

#ifndef PCH_H
#define PCH_H

#define ROXLU_USE_OPENGL
#define ROXLU_USE_MATH
#include "tinylib.h"

/* Ocean */
#define GEOMETRY_RESOLUTION 256
#define GEOMETRY_SIZE 4000.0f
#define GEOMETRY_ORIG_X -1500.0f
#define GEOMETRY_ORIG_Z -1000.0f
#define RESOLUTION 512

/* Texture units */
#define INITIAL_SPECTRUM_UNIT 0
#define SPECTRUM_UNIT 1
#define DISPLACEMENT_MAP_UNIT 2
#define NORMAL_MAP_UNIT 3
#define PING_PHASE_UNIT 4
#define PONG_PHASE_UNIT 5
#define PING_TRANSFORM_UNIT 6
#define PONG_TRANSFORM_UNIT 7

#include <vector>
#include <string>

#endif
