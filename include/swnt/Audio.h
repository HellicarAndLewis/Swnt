#ifndef SWNT_AUDIO_H
#define SWNT_AUDIO_H

extern "C" {
#  include <mpg123.h>
#  include <ao/ao.h>
#  include <uv.h>
}

#include <vector>

void audio_start(uv_work_t* req);
void audio_ready(uv_work_t* req, int status);

class Audio;

class Sound {

 public:
  Sound(Audio& audio, int name);
  ~Sound();
  void lock();
  void unlock();
  void play();
  void stop();
 public:
  int name;
  size_t buffer_size;
  int channels;
  int encoding;
  long rate;
  ao_sample_format format;
  std::vector<char> buffer;
  bool must_stop;
  Audio& audio;
  uv_mutex_t mutex;
};


class Audio {
 public:
  Audio();
  ~Audio();
  bool setup();
  bool add(int name, std::string filepath);
  void update();
  bool play(int name);
  bool stop(int name);
  void shutdown();

 private:
  Sound* find(int name);
  bool play(Sound* snd);
  bool loadFile(std::string filepath, Sound& sound);

 public:
  std::vector<Sound*> sounds;
  uv_loop_t* loop;
};


inline void Sound::lock() {
  uv_mutex_lock(&mutex);
}

inline void Sound::unlock() {
  uv_mutex_unlock(&mutex);
}

inline void Sound::stop() {
  lock();
    must_stop = true;
  unlock();
}

inline void Sound::play() {
  lock();
    must_stop = false;
  unlock();
}
#endif
