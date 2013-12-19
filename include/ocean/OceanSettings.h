#ifndef OCEAN_SETTINGS_H
#define OCEAN_SETTINGS_H

#define ROXLU_USE_OPENGL
//#include "ofMain.h"
#define ROXLU_USE_MATH
#include "tinylib.h"

/* Ocean */
#define GEOMETRY_RESOLUTION 256
#define GEOMETRY_SIZE 2000.0f
#define GEOMETRY_ORIG_X -1000.0f
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

struct OceanSettings {
  OceanSettings():
     wind_x(20.5f),
     wind_y(20.5f),
     size(250.0f)
  {} 

  float wind_x;
  float wind_y;
  float size;
  vec3 cam_pos;
  int win_w;
  int win_h;
};

#endif
