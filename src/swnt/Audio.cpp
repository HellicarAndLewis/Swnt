#include <swnt/Audio.h>

// -----------------------------------------

Sound::Sound(int name)
  :name(name)
  ,sound(NULL)
  ,channel(NULL)
{
}

Sound::~Sound() {
  FMOD_RESULT result;

  if(sound) {
    result = sound->release();
    if(result != FMOD_OK) {
      printf("Error releasing sound: %s\n", FMOD_ErrorString(result));
    }
  }
  sound = NULL;
}

void Sound::setLoop(bool l) {
}

// -----------------------------------------

Audio::Audio() {
  FMOD_RESULT result;
  result = FMOD::System_Create(&system);

  if(result != FMOD_OK) {
    printf("Error initialising audio: %s\n", FMOD_ErrorString(result));
    ::exit(EXIT_FAILURE);
  }

  result = system->init(32, FMOD_INIT_NORMAL, NULL);
  if(result != FMOD_OK) {
    printf("Error: cannot initialise fmod: %s\n", FMOD_ErrorString(result));
    ::exit(EXIT_FAILURE);
  }
}

Audio::~Audio() {

  printf("Cleaning up Audio.\n");

  for(std::vector<Sound*>::iterator it = sounds.begin(); it != sounds.end(); ++it) {
    Sound* s = *it;
    delete s;
  }
  sounds.clear();

  FMOD_RESULT result;

  if(system) {

    result = system->close();

    if(result != FMOD_OK) {
      printf("Error: %s\n", FMOD_ErrorString(result));
      ::exit(EXIT_FAILURE);
    }

    result = system->release();
    
    if(result != FMOD_OK) {
      printf("Error: %s\n", FMOD_ErrorString(result));
      ::exit(EXIT_FAILURE);
    }
  }

  system = NULL;
}

void Audio::update() {
  if(system) {
    system->update();
  }
}

bool Audio::add(int name, std::string filepath, FMOD_MODE mode) {
  FMOD_RESULT result; 
  Sound* s = new Sound(name);

  result = system->createSound(filepath.c_str(), mode, 0, &s->sound);

  if(result != FMOD_OK) {
    printf("Error: cannot create the sound: %s\n", FMOD_ErrorString(result));
    return false;
  }

  sounds.push_back(s);
  return true;
}

void Audio::play(int name) {
  Sound* s = getSound(name);
  if(!s) {
    printf("Warning: cannot find sound for: %d\n", name);
    return ;
  }

  FMOD_RESULT result = system->playSound(FMOD_CHANNEL_FREE, s->sound, 0, &s->channel);
  if(result != FMOD_OK) {
    printf("Error: %s\n", FMOD_ErrorString(result));
    ::exit(EXIT_FAILURE);
  }
}

void Audio::setVolume(int name, float v) {
  if(v > 1.0f) {
    v = 1.0f;
  }
  else if(v < 0.0f) {
    v = 0.0f;
  }
}

Sound* Audio::getSound(int name) {
  for(std::vector<Sound*>::iterator it = sounds.begin(); it != sounds.end(); ++it) {
    if((*it)->name == name) {
      return *it;
    }
  }
  return NULL;
}


void Audio::stop(int name) {
  Sound* s = getSound(name);
  if(!s) {
    printf("Warning: cannot find sound: %d\n", name);
    return;
  }
  printf("@todo stop\n");
}
