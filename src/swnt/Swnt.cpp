#include <swnt/Swnt.h>

Swnt::Swnt(Settings& settings) 
  :settings(settings)
  ,water_graphics(settings.ocean)
  ,ocean(settings.ocean, water_graphics)
  ,graphics(settings)
  ,mask(settings, graphics)
  ,tracking(settings, graphics)
  ,flow(settings, graphics)
  ,spirals(settings, tracking, graphics, flow)
#if USE_WATER
  ,water(settings, flow)
#endif
#if USE_KINECT
  ,rgb_tex(0)
  ,depth_tex(0)
  ,rgb_image(NULL)
  ,depth_image(NULL)
#endif
  ,state(STATE_RENDER_SCENE)
  ,draw_flow(true)
  ,draw_spirals(true)
  ,draw_threshold(true)
  ,draw_water(false)
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

#if USE_OCEAN
  if(!water_graphics.setup(settings.win_w, settings.win_h)) {
    printf("Error: cannot setup the water graphics.\n");
    return false;
  }

  if(!ocean.setup()) {
    printf("Error: cannot setup the ocean.\n");
    return false;
  }
#endif

#if USE_WATER
  if(!water.setup()) {
    printf("Error: cannot setup water.\n");
    return false;
  }
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

  if(!spirals.setup()) {
    printf("Error: cannot setup the spirals.\n");
    return false;
  }

  if(!flow.setup()) {
    printf("Error: cannot setup the flow.\n");
    return false;
  }

  // matrices for rendering the ocean
  persp_matrix.perspective(65.0f, settings.win_w/settings.win_h, 0.01f, 1000.0f);
  settings.ocean.cam_pos.set(0.0, 100.0, 100.0f);
  view_matrix.lookAt(settings.ocean.cam_pos, vec3(), vec3(0.0f, 1.0f, 0.0f));

  mask.print();

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

#if USE_WATER 
  if(draw_water) {
    water.update();
  }
#endif

  spirals.update(1.0f/60.0f);
  mask.update();
}

void Swnt::draw() {

  if(state == STATE_RENDER_ALIGN) {
    vec3 red(1.0f, 0.0f, 0.0f);
    vec3 green(0.0f, 1.0f, 0.0f);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    graphics.drawCircle(settings.win_w * 0.5, settings.win_h * 0.5, settings.radius, red);
    graphics.drawCircle(settings.win_w * 0.5, settings.win_h * 0.5, 20, green);

  }
  else if(state == STATE_RENDER_SCENE) {

#if USE_OCEAN
    ocean.pm = persp_matrix.ptr();
    ocean.vm = view_matrix.ptr();
    ocean.update(1.0f/30.0f);
#endif

#if !USE_KINECT && USE_OCEAN
    ocean.draw();
    return;
#endif  
  
#if USE_KINECT

    // here we grab the mask shape into a texture that is later used to pre-process the image for image processing
    mask.beginMaskGrab();
    mask.drawMask();
    mask.endMaskGrab();

    // here we draw the raw depth buffer from the kinect and make sure the values are correctl transformed to distance values (in a shader)
    mask.beginDepthGrab();
    graphics.drawDepth(depth_tex, 0.0f, 0.0f, settings.image_processing_w, settings.image_processing_h);
    mask.endDepthGrab();


#if USE_OCEAN
    mask.beginSceneGrab();
    {
      if(draw_water) {
         ocean.draw();
      }
      if(draw_flow) {
        flow.draw();
      }
      if(draw_spirals) {
        spirals.draw();
      }
    }
    mask.endSceneGrab();
#else 
    mask.beginSceneGrab();
    {
      #if USE_WATER
      if(draw_water) {
        water.draw();
      }
      #endif
      
      if(draw_flow) {
        flow.draw();
      }
      if(draw_spirals) {
        spirals.draw();
      }
    }

    mask.endSceneGrab();
#endif

    // this is where we mask out the depth image using the previously grabbed mask.
    mask.maskOutDepth();

    // graphics.drawTexture(mask.masked_out_tex, 320.0f, 0.0f, 320.0f, 240.0f);
    flow.calc(mask.masked_out_pixels);
    tracking.track(mask.masked_out_pixels);

    //  #if USE_OCEAN
    // draw the final masked out scene
    mask.draw_hand = draw_threshold;
    mask.maskOutScene();

    //graphics.drawTexture(rgb_tex, 0.0f, 0.0f, 320.0f, 240.0f);  
    //graphics.drawDepth(depth_tex, 320.0f, 0.0f, settings.image_processing_w, settings.image_processing_h);
    //  #endif
    //tracking.draw(0.0f, 0.0f);

  }
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

