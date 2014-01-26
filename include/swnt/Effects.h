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
#include <swnt/effects/Eddy.h>
#include <swnt/effects/Splashes.h>

class Swnt;
class Settings;

class Effects { 

 public:
  Effects(Swnt& swnt);
  ~Effects();
  bool setup(int w, int h);
  void update();
  void drawExtraFlow();
  void drawExtraDiffuse();
 public:
  Swnt& swnt;
  Settings& settings;
  mat4 ortho_pm;
 private:
  Eddy eddy;
  Splashes splashes;
};

#endif
