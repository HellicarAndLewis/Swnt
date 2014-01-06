#include <tinylib.h>
#include <swnt/Audio.h>

Sound::Sound(Audio& audio, int name) 
  :audio(audio)
  ,name(name)
  ,buffer_size(0)
  ,channels(0)
  ,encoding(0)
  ,rate(0)
  ,must_stop(false)
{
  uv_mutex_init(&mutex);
}

Sound::~Sound() {
  uv_mutex_destroy(&mutex);
}

// ---------------------------------------------------------------------

void audio_start(uv_work_t* req) {
  Sound* snd = static_cast<Sound*>(req->data);
  if(!snd) {
    printf("Error: cannot cast the sound object for the audio thread.\n");
    return;
  }

  int driver = ao_default_driver_id();
  ao_device* device = ao_open_live(driver, &snd->format, NULL);

  if(!device) {
    printf("Cannot open ao device.\n");
    return;
  }

  size_t done = 0;
  size_t to_play = 0;
  bool must_stop = false;

  while(!must_stop) {

    if(done + snd->buffer_size > snd->buffer.size()) {
      to_play = snd->buffer.size() - done;
    }
    else {
      to_play = snd->buffer_size;
    }

    ao_play(device, (char*)(&snd->buffer[0]) + done, to_play);
    done += to_play;

    if(done >= snd->buffer.size()) {
      break;
    }

    snd->lock();
      must_stop = snd->must_stop;
    snd->unlock();
  }
}

void audio_ready(uv_work_t* req, int status) {
  delete req;
}

// ---------------------------------------------------------------------

Audio::Audio()
  :loop(NULL)
{
  ao_initialize();
  mpg123_init();
  loop = uv_loop_new();
}

Audio::~Audio() {

  shutdown();

  for(std::vector<Sound*>::iterator it = sounds.begin(); it != sounds.end(); ++it) {
    Sound* s = *it;
    delete s;
  }
  sounds.clear();

  if(loop) {
    uv_loop_delete(loop);
  }
}

void Audio::shutdown() {

  for(std::vector<Sound*>::iterator it = sounds.begin(); it != sounds.end(); ++it) {
    Sound* s = *it;
    s->stop();
  }
}

bool Audio::setup() {
  return true;
}

bool Audio::add(int name, std::string filepath) {

  Sound* snd = new Sound(*this, name);
  if(!snd) {
    printf("Cannot allocate Sound for file: %s\n", filepath.c_str());
    return false;
  }

  if(!loadFile(filepath, *snd)) {
    printf("Cannot load the audio file.\n");
    delete snd; 
    snd = NULL;
    return false;
  }

  sounds.push_back(snd);

  return true;
}

void Audio::update() {
  uv_run(loop, UV_RUN_NOWAIT);
}

bool Audio::stop(int name) {
  Sound* s = find(name);
  if(!s) {
    printf("Cannot find audio: %d\n", name);
    return false;
  }

  s->stop();

  return true;
}

bool Audio::play(int name) {
  Sound* s = find(name);
  if(!s) {
    printf("Cannot find Sound for name: %d\n", name);
    return false;
  }

  return play(s);
}

bool Audio::play(Sound* snd) {
  uv_work_t* w = new uv_work_t();
  w->data = snd;
  uv_queue_work(loop, w, audio_start, audio_ready);
  return true;
}

bool Audio::loadFile(std::string filepath, Sound& snd) {
  
  int err = 0;
  mpg123_handle* mh = NULL;
  unsigned char* tmp_buffer = NULL;
  size_t done = 0;

  mh = mpg123_new(NULL, &err);
  if(!mh) {
    printf("Error while creating a mpg123 object, err: %d\n", err);
    return false;
  }
  
  snd.buffer_size = mpg123_outblock(mh);
  if(!snd.buffer_size) {
    printf("Error: cannot get a valid outblock.\n");
    goto audio_error;
  }

  err = mpg123_open(mh, filepath.c_str());
  if(err != MPG123_OK) {
    printf("Error while trying to open the audio file: %s, %s\n", filepath.c_str(), mpg123_plain_strerror(err));
    goto audio_error;
  }

  err = mpg123_getformat(mh, &snd.rate, &snd.channels, &snd.encoding);
  if(err != MPG123_OK) {
    printf("Error while trying to get the audio format for: %s, %s\n", filepath.c_str(), mpg123_plain_strerror(err));
    goto audio_error;
  }

  snd.format.bits = mpg123_encsize(snd.encoding) * 8;
  snd.format.rate = snd.rate;
  snd.format.channels = snd.channels;
  snd.format.byte_format = AO_FMT_NATIVE;
  snd.format.matrix = 0;

  tmp_buffer = new unsigned char[snd.buffer_size];
  if(!tmp_buffer) {
    printf("Error: cannot allocate a buffer for the audio, of size: %ld\n", snd.buffer_size);
    goto audio_error;
  }

  while(mpg123_read(mh, tmp_buffer, snd.buffer_size, &done) == MPG123_OK) {
    std::copy(tmp_buffer, tmp_buffer+done, std::back_inserter(snd.buffer));
  }
  
  mpg123_delete(mh);
  mh = NULL;
  delete[] tmp_buffer;
  tmp_buffer = NULL;

  return snd.buffer.size();

 audio_error:
  if(mh) {
    mpg123_delete(mh);
    mh = NULL;
  }
  return false;
}

Sound* Audio::find(int name) {
  Sound* s = NULL;

  for(std::vector<Sound*>::iterator it = sounds.begin(); it != sounds.end(); ++it) {
    Sound* snd = *it;
    if(snd->name == name) {
      s = snd;
      break;
    }
  }
  return s;
}
