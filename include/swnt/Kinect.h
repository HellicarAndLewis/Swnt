#ifndef KINECT_H
#define KINECT_H

extern "C" {
#  include <uv.h>
#  include <string.h>
#  include <stdlib.h>
#  include <stdio.h>
#  include <libfreenect/libfreenect.h>
}

// -------------------------------------

void kinect_thread(void* user);
void kinect_depth_callback(freenect_device* dev, void* depth, uint32_t timestamp);
void kinect_video_callback(freenect_device* dev, void* rgb, uint32_t timestamp);

// -------------------------------------

class Kinect {
 public:
  Kinect();
  ~Kinect();
  bool setup();
  
  void lock();
  bool hasNewRGB(); /* make sure to call lock() and unlock(), this will directly set has_new_rgb to false too */
  bool hasNewDepth(); 
  void unlock();

 public:
  freenect_context* ctx;
  freenect_device* device;
  uv_thread_t thread;
  uv_mutex_t mutex;
  bool must_stop;

  /* buffers */
  uint32_t nbytes_rgb;
  uint8_t* rgb_back;
  uint8_t* rgb_mid; 
  uint8_t* rgb_front;
  uint8_t* depth_back;
  uint8_t* depth_mid;
  uint8_t* depth_front;
  bool has_new_rgb;
  bool has_new_depth;
};

inline bool Kinect::hasNewDepth() {
  bool has = has_new_depth;
  has_new_depth = false;
  return has;
}

inline bool Kinect::hasNewRGB() {
  bool has = has_new_rgb;
  has_new_rgb = false;
  return has;
}

inline void Kinect::lock() {
  uv_mutex_lock(&mutex);
}

inline void Kinect::unlock() {
  uv_mutex_unlock(&mutex);
}
#endif
