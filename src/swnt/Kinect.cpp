#include <swnt/Kinect.h>

// -------------------------------------

void kinect_thread(void* user) { 

  Kinect* kinect = static_cast<Kinect*>(user);
  if(!kinect) {
    printf("Error: kinect thread didn't receive a reference to the Kinect instance.\n");
    ::exit(EXIT_FAILURE);
  }

  freenect_device* dev = kinect->device;
  
  //freenect_set_tilt_degs(dev, 15);
  freenect_set_led(dev, LED_RED);
  freenect_set_depth_callback(dev, kinect_depth_callback);
  freenect_set_video_callback(dev, kinect_video_callback);
  freenect_set_video_mode(dev, freenect_find_video_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_VIDEO_RGB));
  freenect_set_depth_mode(dev, freenect_find_depth_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_DEPTH_11BIT));
  freenect_set_video_buffer(dev, kinect->rgb_back);
  freenect_set_depth_buffer(dev, kinect->depth_back);
  freenect_start_depth(dev);
  freenect_start_video(dev);

  bool must_stop = false;

  while(freenect_process_events(kinect->ctx) >= 0) {
        
    uv_mutex_lock(&kinect->mutex);
      must_stop = kinect->must_stop;
    uv_mutex_unlock(&kinect->mutex);

    if(must_stop) {
      break;
    }

  }
  
  freenect_set_led(dev, LED_GREEN);
  freenect_stop_depth(dev);
  freenect_stop_video(dev);
}

void kinect_depth_callback(freenect_device* dev, void* depth, uint32_t timestamp) {

  Kinect* kinect = static_cast<Kinect*>(freenect_get_user(dev));
  if(!kinect) {
    printf("Error: cannot get the Kinect* user ptr.\n");
    ::exit(EXIT_FAILURE);
  }

  if(depth != kinect->depth_back) {
    printf("Error: wrong depth pointer!\n");
  }

  kinect->depth_back = kinect->depth_mid;
  freenect_set_depth_buffer(dev, kinect->depth_back);
  kinect->depth_mid = (uint8_t*)depth;

  uv_mutex_lock(&kinect->mutex);
  {
    memcpy(kinect->depth_front, kinect->depth_mid, kinect->nbytes_rgb);
    kinect->has_new_depth = true;
  }
  uv_mutex_unlock(&kinect->mutex);
}

void kinect_video_callback(freenect_device* dev, void* rgb, uint32_t timestamp) {
  
  Kinect* kinect = static_cast<Kinect*>(freenect_get_user(dev));
  if(!kinect) {
    printf("Error: cannot get the Kinect* user ptr.\n");
    ::exit(EXIT_FAILURE);
  }

  if(rgb != kinect->rgb_back) {
    printf("Error: wrong rgb pointer.\n");
  }


  kinect->rgb_back = kinect->rgb_mid;
  freenect_set_video_buffer(dev, kinect->rgb_back);
  kinect->rgb_mid = (uint8_t*)rgb;

  uv_mutex_lock(&kinect->mutex);
  {
    memcpy(kinect->rgb_front, kinect->rgb_mid, kinect->nbytes_rgb);
    kinect->has_new_rgb = true;
  }
  uv_mutex_unlock(&kinect->mutex);
}


// -------------------------------------

Kinect::Kinect() 
  :ctx(NULL)
  ,device(NULL)
  ,must_stop(true)
  ,has_new_rgb(false)
  ,has_new_depth(false)
  ,rgb_back(NULL)
  ,rgb_mid(NULL)
  ,rgb_front(NULL)
  ,depth_back(NULL)
  ,depth_mid(NULL)
  ,depth_front(NULL)
{
  uv_mutex_init(&mutex);
}

Kinect::~Kinect() {
  printf("Error: need to free buffers.\n");

  uv_mutex_lock(&mutex);
  must_stop = true;
  uv_mutex_unlock(&mutex);

  uv_thread_join(&thread);

  if(ctx) {
    freenect_close_device(device);
    freenect_shutdown(ctx);
    ctx = NULL;
  }

  uv_mutex_destroy(&mutex);

  has_new_rgb = false;
  has_new_depth = false;

  if(depth_back) {
    delete[] depth_back;
  }

  if(depth_mid) {
    delete[] depth_mid;
  }

  if(depth_front) {
    delete[] depth_front;
  }

  depth_back = NULL;
  depth_mid = NULL;
  depth_front = NULL;

  if(rgb_back) {
    delete[] rgb_back;
  }

  if(rgb_mid) {
    delete[] rgb_mid;
  }

  if(rgb_front) {
    delete[] rgb_front;
  }

  rgb_back = NULL;
  rgb_mid = NULL;
  rgb_front = NULL;
  device = NULL;
}

bool Kinect::setup() {

  if(ctx) {
    printf("Error: the freenect context has been setup already.\n");
    return false;
  }

  if(freenect_init(&ctx, NULL) < 0) {
    printf("Error: cannot init libfreenect.\n");
    return false;
  }

  freenect_set_log_level(ctx, FREENECT_LOG_DEBUG);

  int ndevices = freenect_num_devices(ctx);
  if(ndevices < 1) {
    printf("Error: cannot find a kinect. @todo cleanup mem.\n");
    freenect_shutdown(ctx);
    ctx = NULL;
    return false;
  }

  printf("Number of found kinect devices: %d\n", ndevices);

  freenect_select_subdevices(ctx, (freenect_device_flags)(FREENECT_DEVICE_MOTOR | FREENECT_DEVICE_CAMERA));
  //freenect_select_subdevices(ctx, (freenect_device_flags)( FREENECT_DEVICE_CAMERA));

  int devnum = 0;
  if(freenect_open_device(ctx, &device, devnum) < 0) {
    printf("Error: cannot open device: %d\n", devnum);
    freenect_shutdown(ctx);
    ctx = NULL;
    return false;
  }

  freenect_set_user(device, this);

  uint32_t w = 640;
  uint32_t h = 480;
  uint32_t nbytes = w * h * 3;
  nbytes_rgb = nbytes;
  rgb_back = new uint8_t[nbytes];
  rgb_mid = new uint8_t[nbytes];
  rgb_front = new uint8_t[nbytes];
  depth_back = new uint8_t[nbytes];
  depth_mid = new uint8_t[nbytes];
  depth_front = new uint8_t[nbytes];

  uv_mutex_lock(&mutex);
    must_stop = false;
  uv_mutex_unlock(&mutex);

  uv_thread_create(&thread, kinect_thread, this);

  return true;
}
