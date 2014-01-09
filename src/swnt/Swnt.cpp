#include <swnt/Swnt.h>

Swnt::Swnt(Settings& settings) 
  :settings(settings)
  ,graphics(settings)
  ,mask(settings, graphics)
  ,tracking(settings, graphics)
  ,flow(settings, graphics)
#if USE_WATER
  ,water(height_field)
#endif
#if USE_KINECT
  ,rgb_tex(0)
  ,depth_tex(0)
  ,rgb_image(NULL)
  ,depth_image(NULL)
#endif
  ,state(STATE_RENDER_SCENE)
  ,draw_flow(false)
  ,draw_threshold(true)
  ,draw_water(true)
  ,draw_vortex(false)
#if USE_GUI
  ,gui(*this)
#endif
#if USE_EFFECTS
  ,effects(*this)
#endif
{
}

Swnt::~Swnt() {

#if USE_KINECT
  if(rgb_image) {
    delete[] rgb_image;
    rgb_image = NULL;
  }

  if(depth_image) {
    delete[] depth_image;
    depth_image = NULL;
  }
#endif
}

bool Swnt::setup() {

#if USE_WATER
  if(!height_field.setup()) {
    printf("Error: cannot setupthe height field.\n");
    return false;
  }

  if(!water.setup(settings.win_w, settings.win_h)) {
    printf("Error: cannot setup water.\n");
    return false;
  }
  water.print();
#endif

  if(!graphics.setup()) {
    printf("Error: cannot setup the graphics model.\n");
    return false;
  }
 
#if USE_KINECT
  if(!setupKinect()) {
    return false;
  }
#endif

  if(!mask.setup()) {
    printf("Error: cannot setup the mask.\n");
    return false;
  }

  if(!tracking.setup()) {
    printf("Error: cannot setup the tracking.\n");
    return false;
  }

  if(!flow.setup()) {
    printf("Error: cannot setup the flow.\n");
    return false;
  }
  flow.print();

  //water.flow_tex = flow.flow_tex;

#if USE_RGB_SHIFT
  if(!rgb_shift.setup()) {
    printf("Error: cannot setup rgb shift.\n");
    return false;
  }
#endif

#if USE_SCENE
  if(!scene.setup(settings.win_w, settings.win_h)) {
    printf("Error: cannot setup the scene.\n");
    return false;
  }
#endif

#if USE_GUI
  if(!gui.setup(settings.win_w, settings.win_h)) {
    printf("Error: cannot setup the GUI.\n");
    return false;
  }
#endif

#if USE_WEATHER
  if(!weather.setup()) {
    printf("Error: cannot setup the weather tool.\n");
    return false;
  }
#endif

#if USE_AUDIO
  if(!audio.setup()) {
    printf("Error: cannot setup the audio player.\n");
    return false;
  }
  //  if(!audio.add(SOUND_OCEAN, rx_to_data_path("audio/ocean.mp2"))) {
  if(!audio.add(SOUND_OCEAN, rx_to_data_path("audio/short.mp3"))) {
    printf("Error while loading ocean sound");
    return false;
  }
  if(!audio.add(SOUND_WATER, rx_to_data_path("audio/water.mp2"))) {
    printf("Error while loading water sound");
    return false;
  }
#endif

#if USE_EFFECTS
  if(!effects.setup(settings.win_w, settings.win_h)) {
    printf("Error: cannot setup effects.\n");
    return false;
  }
#endif

  // matrices for rendering the ocean
  persp_matrix.perspective(65.0f, settings.win_w/settings.win_h, 0.01f, 1000.0f);
  settings.ocean.cam_pos.set(0.0, 100.0, 100.0f);
  view_matrix.lookAt(settings.ocean.cam_pos, vec3(), vec3(0.0f, 1.0f, 0.0f));

  mask.print();
  print();
  return true;
}


// Setup the kinect thread which gets the depth and color buffers + the GL textures.
#if USE_KINECT
bool Swnt::setupKinect() {

  if(!kinect.setup()) {
    printf("Cannot setup the kinect device.\n");
    return false;
  }

  glActiveTexture(GL_TEXTURE0);
  glGenTextures(1, &rgb_tex);
  glBindTexture(GL_TEXTURE_2D, rgb_tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 640, 480, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
 
  glGenTextures(1, &depth_tex);
  glBindTexture(GL_TEXTURE_2D, depth_tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, 640, 480, 0, GL_RED, GL_UNSIGNED_SHORT, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  if(!depth_tex || !rgb_tex) {
    printf("Error: cannot create kinect textures.\n");
    return false;
  }
  
  rgb_image = new unsigned char[640 * 480 * 3];
  depth_image = new unsigned char[640 * 480 * 3];

  return true;
}
#endif

void Swnt::update() {

#if USE_KINECT
  updateKinect();
#endif

  mask.update();

#if USE_WATER
  height_field.calculateHeights();
  height_field.calculatePositions();
  height_field.calculateNormals();
  water.update(1.0f/60.0f);
#endif

#if USE_WEATHER
  weather.update();
#endif

#if USE_AUDIO
  audio.update();
#endif

#if USE_EFFECTS
  effects.update();
#endif 
}

void Swnt::draw() {
#if 0  
  water.draw();
  gui.draw();
  return;
#endif

#if 0
  effects.drawExtraDiffuse();
  return;
#endif

#if 0
  effects.splashes.drawExtraDiffuse();
  effects.splashes.drawExtraFlow();
  gui.draw();
  return;
#endif

#if 0
  effects.drawExtraFlow();
  return;
#endif

  if(state == STATE_RENDER_ALIGN) {
    vec3 red(1.0f, 0.0f, 0.0f);
    vec3 green(0.0f, 1.0f, 0.0f);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    graphics.drawCircle(settings.win_w * 0.5, settings.win_h * 0.5, settings.radius, red);
    graphics.drawCircle(settings.win_w * 0.5, settings.win_h * 0.5, 20, green);
  }
  else if(state == STATE_RENDER_SCENE) {
  
#if USE_KINECT

    // capture the mask shape
    mask.beginMaskGrab();
       mask.drawMask();
    mask.endMaskGrab();
    
    // capture the depth buffer from kinect 
    mask.beginDepthGrab();
       graphics.drawDepth(depth_tex, 0.0f, 0.0f, settings.image_processing_w, settings.image_processing_h);
    mask.endDepthGrab();

#if USE_EFFECTS
  water.beginGrabFlow();
    effects.drawExtraFlow();
  water.endGrabFlow();
#endif
    
    // capture the scene
    mask.beginSceneGrab();
    {
#    if USE_WATER
      if(draw_water) {
        water.draw();
      }
#    endif

      //      effects.drawExtraDiffuse();

      if(draw_flow){ 
        flow.draw();
      }
      
    }
    mask.endSceneGrab();

    mask.maskOutDepth();

    flow.calc(mask.masked_out_pixels);
    tracking.track(mask.masked_out_pixels);

    // draw the final masked out scene
    mask.draw_hand = draw_threshold;
    mask.maskOutScene();

    tracking.draw(0.0f, 0.0f);
    flow.applyPerlinToField();
    // flow.draw();

    #if USE_RGB_SHIFT
    rgb_shift.apply();
    rgb_shift.draw();
    #endif
  }
#endif  // USE_KINECT

  //effects.splashes.drawExtraFlow();
  if(draw_vortex) {
    // water.blitFlow(0.0, 0.0, 320.0f, 240.0f);
    effects.splashes.drawExtraDiffuse();
    effects.splashes.drawExtraFlow();

  }

#if USE_GUI
  gui.draw();
#endif

}


// Updates the kinect depth and color buffers.
#if USE_KINECT
void Swnt::updateKinect() {
#  if 1
  {
    bool new_frame = false;
 
    kinect.lock();
    {
      if(kinect.hasNewRGB()) {
        memcpy(rgb_image, kinect.rgb_front, kinect.nbytes_rgb);
        new_frame = true;
      }
    }
    kinect.unlock();
 
    if(new_frame) {
      glBindTexture(GL_TEXTURE_2D, rgb_tex);
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 640, 480, GL_RGB, GL_UNSIGNED_BYTE, rgb_image);
    }
 
  }
#  endif

#  if 1
  {
    bool new_frame = false;
    kinect.lock();
    {
      if(kinect.hasNewDepth()) {
        memcpy(depth_image, kinect.depth_front, kinect.nbytes_rgb);
      }
      new_frame = true;
    }
    kinect.unlock();

    if(new_frame) {
      glBindTexture(GL_TEXTURE_2D, depth_tex);
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 640, 480, GL_RED, GL_UNSIGNED_SHORT, depth_image);
    }
  }
#  endif
#endif // USE_KINECT
 
}

void Swnt::print() {
  printf("swnt.rgb_tex: %d\n", rgb_tex);
  printf("swnt.depth_tex: %d\n", depth_tex);
  printf("-\n");
}
