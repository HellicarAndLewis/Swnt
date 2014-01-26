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

#ifndef SWNT_AUDIO_H
#define SWNT_AUDIO_H

#include <stdlib.h>
#include <vector>
#include <string>
#include <fmod.hpp>
#include <fmod_errors.h>

// -----------------------------------------

class Sound {

public:
  Sound(int name);
  ~Sound();
  void setLoop(bool loop);
  void setVolume(float v);

public:
  int name;
  FMOD::Sound* sound;
  FMOD::Channel* channel;
};

// -----------------------------------------

class Audio {

public:
  Audio();
  ~Audio();
  void update();
  bool add(int name, std::string filepath, FMOD_MODE mode = FMOD_SOFTWARE);
  void play(int name);
  void playOnce(int name);
  void stop(int name);
  void setVolume(int name, float f);
  Sound* getSound(int name);

public:
  FMOD::System* system;
  std::vector<Sound*> sounds;
};

#endif
