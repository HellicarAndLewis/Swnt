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
