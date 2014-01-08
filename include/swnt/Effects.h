/*

  Some experimental effects to enhance the interactivity

  - nice haze effect: http://forums.epicgames.com/threads/708297-UDK-Video-Tutorials-Effects
 */ 

#ifndef SWNT_EFFECTS_H
#define SWNT_EFFECTS_H

#define ROXLU_USE_OPENGL
#define ROXLU_USE_MATH
#define ROXLU_USE_PNG
#include <tinylib.h>

#include <swnt/Particles.h>
#include <swnt/Blur.h>
#include <swnt/effects/Mist.h>

class Effects { 

 public:
  Effects();
  ~Effects();
  bool setup(int w, int h);
  void update();
  void draw();

 public:
  mat4 ortho_pm;
  Mist mist;
};

#endif
