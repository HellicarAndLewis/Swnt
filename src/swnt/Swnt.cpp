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
#if USE_TIDES
  ,tides_timeout(0)
  ,time_of_day(0)
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
  ,draw_tracking(false)
  ,draw_gui(true)
  ,override_with_gui(false)
#if USE_GUI
  ,gui(*this)
#endif
#if USE_EFFECTS
  ,effects(*this)
#endif
#if USE_WEATHER
  ,has_weather_info(false)
#endif
  ,ocean_roughness(0.5f)
{
}

Swnt::~Swnt() {

#if USE_WATER_BALLS
  printf("Cleanup tracked balls.\n");
#endif

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

  draw_flow = false;
  draw_threshold = false;
  draw_water = false;
  draw_vortex = false;
  draw_tracking = false;
  draw_gui = false;
  override_with_gui = false;
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

#if USE_TIDES 
  if(!tides.setup(rx_to_data_path("tides.txt"))) {  
    printf("Error: cannot load the tides information.\n");
    return false;
  }
#endif

#if USE_AUDIO
  //  if(!audio.add(SOUND_OCEAN, rx_to_data_path("audio/ocean.mp2"))) {
  if(!audio.add(SOUND_WATER_FLOWING, rx_to_data_path("audio/water_flowing.wav"), FMOD_SOFTWARE | FMOD_LOOP_NORMAL)) {
    return false;
  }
  if(!audio.add(SOUND_WAVES_CRASHING, rx_to_data_path("audio/waves_crashing.wav"), FMOD_SOFTWARE | FMOD_LOOP_NORMAL)) {
    return false;
  }

  audio.play(SOUND_WATER_FLOWING);
  

#endif

#if USE_EFFECTS
  if(!effects.setup(settings.win_w, settings.win_h)) {
    printf("Error: cannot setup effects.\n");
    return false;
  }
#endif

#if USE_WATER_BALLS
  if(!ball_drawer.setup(settings.win_w, settings.win_h)) {
    printf("Error: cannot setup the water ball drawer.\n");
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

  updateActivityLevel();

#if USE_KINECT
  updateKinect();
#endif

  mask.update();

#if USE_WATER
  height_field.calculateHeights();
  height_field.calculatePositions();
  height_field.calculateNormals();
  height_field.calculateFoam();
  water.update(1.0f/60.0f);
#endif

#if USE_WEATHER
  weather.update();
  WeatherInfo new_info;
  bool weather_changed = weather.hasInfo(new_info);
  if(weather_changed) {
    has_weather_info = true;
    weather_info = new_info;
  }
#endif

#if USE_AUDIO
  audio.update();
#endif

#if USE_EFFECTS
  effects.update();
#endif 

#if USE_WATER_BALLS
  updateWaterBalls();
#endif  

#if USE_TIDES
  updateTides();
#endif
}

void Swnt::draw() {

#if 0
  water.draw();
  return ;
#endif

#if 0
  ball_drawer.draw();
  gui.draw();
  return;
#endif

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
    glDisable(GL_DEPTH_TEST);
    vec3 red(1.0f, 0.0f, 0.0f);
    vec3 green(0.0f, 1.0f, 0.0f);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    float radius = settings.radius - (mask.scale_range + (mask.scale_range * mask.scale));
    graphics.drawCircle(settings.win_w * 0.5, settings.win_h * 0.5, radius, red);
    graphics.drawCircle(settings.win_w * 0.5, settings.win_h * 0.5, 20, green);
  }
  else if(state == STATE_RENDER_SCENE) {
  
#if USE_KINECT


#if USE_WATER_BALLS
    // apply forces onto the water, when water balls are flushing away
    if(flush_points.size()) {
      height_field.beginDrawForces();
      for(std::vector<vec2>::iterator it = flush_points.begin(); it != flush_points.end(); ++it) {
        vec2& p = *it;
        height_field.drawForceTexture(water.force_tex0, 1.0 - p.x, p.y, 0.4, 0.4);
      }
      height_field.endDrawForces();
    }
#endif

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
#   if USE_WATER
    if(draw_water) {
      water.draw();
    }
#   endif
#   if USE_EFFECTS
    if(draw_vortex) {
      effects.splashes.drawExtraDiffuse();
      effects.splashes.drawExtraFlow();
    }
#   endif
    mask.endSceneGrab();

    mask.maskOutDepth();
    mask.maskOutScene();

    mask.drawHand();

    tracking.track(mask.masked_out_pixels);

    mask.draw_hand = draw_threshold;

#    if USE_WATER_BALLS
    ball_drawer.draw();
#    endif


    if(draw_tracking) {
      tracking.draw(0.0f, 0.0f);
    }

    // flow.applyPerlinToField();
    // flow.draw();

    #if USE_RGB_SHIFT
    rgb_shift.apply();
    rgb_shift.draw();
    #endif


  }
#endif  // USE_KINECT

  //effects.splashes.drawExtraFlow();


#if USE_GUI
  if(draw_gui) {
    gui.draw();
  }
#endif

  // height_field.debugDraw();
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
}

#endif // USE_KINECT

void Swnt::print() {
#if USE_KINECT  
  printf("swnt.rgb_tex: %d\n", rgb_tex);
  printf("swnt.depth_tex: %d\n", depth_tex);
#endif
  printf("-\n");
}


#if USE_WATER_BALLS 

void Swnt::updateWaterBalls() {
  flush_points.clear();
  ball_drawer.update(0.016f);

  std::vector<Tracked*>& tracked_items = tracking.tracked;
  std::vector<TrackedWaterBall> free_tracked;
  vec2 flush_point_to_water(1.0/settings.win_w, 1.0/settings.win_h);

  // Step 1: Find water balls, which don't have a tracked object anymore, so we can reuse them.
  for(std::vector<TrackedWaterBall>::iterator twit = tracked_balls.begin(); twit != tracked_balls.end(); ++twit) {

    TrackedWaterBall& tracked_ball = *twit;
    Tracked* found = NULL;
    
    for(std::vector<Tracked*>::iterator it = tracked_items.begin(); it != tracked_items.end(); ++it) {
      Tracked* tr = *it;
      if(tr->id == tracked_ball.tracked_id) {
        found = tr;
        break;
      }
    }

    if(!found) {
      if(tracked_ball.water_ball->state == WATERDROP_STATE_FREE) {
         free_tracked.push_back(tracked_ball);
      }
      else if(tracked_ball.water_ball->state == WATERDROP_STATE_FILL) {
        tracked_ball.water_ball->flush();
      }
    }
    else if(!found->matched) {
      if(tracked_ball.water_ball->state == WATERDROP_STATE_FREE) {
         free_tracked.push_back(tracked_ball);
      }
      else if(tracked_ball.water_ball->state == WATERDROP_STATE_FILL) {
        tracked_ball.water_ball->flush();
        flush_points.push_back(tracked_ball.water_ball->position * flush_point_to_water);
      }
      // printf("Found: %d, matched: %d, state: %d\n", found->id, found->matched, tracked_ball.water_ball->state);
    }
  }

  float scale_x = float(settings.win_w) / settings.image_processing_w;
  float scale_y = float(settings.win_h) / settings.image_processing_h;

  // Step: For each tracked object, find the related water ball or create a new one, when it isn't found
  for(std::vector<Tracked*>::iterator it = tracked_items.begin(); it != tracked_items.end(); ++it) {

    Tracked* tracked = *it;
    WaterBall* found = NULL;
    TrackedWaterBall found_tracked_ball;
    
    if(!tracked->matched) {
      continue;
    }

    for(std::vector<TrackedWaterBall>::iterator twit = tracked_balls.begin(); twit != tracked_balls.end(); ++twit) {
      TrackedWaterBall& tracked_ball = *twit;
      if(tracked_ball.tracked_id == tracked->id) {
        found = tracked_ball.water_ball;
        found_tracked_ball = tracked_ball;
        break;
      }
    }

    if(!found && free_tracked.size()) {
      // Reuse a previously tracked waterball that is free
      TrackedWaterBall free_tracked_ball  = *free_tracked.begin();
      found_tracked_ball = free_tracked_ball;
      free_tracked.erase(free_tracked.begin());
      free_tracked_ball.tracked_id = tracked->id;
      found = free_tracked_ball.water_ball;
      found->fill();
    }

    if(!found) {
      found =  new WaterBall();
      ball_drawer.addWaterBall(found);

      TrackedWaterBall tracked_ball;
      tracked_ball.water_ball = found;
      tracked_ball.tracked_id = tracked->id;
      tracked_balls.push_back(tracked_ball);
      found_tracked_ball = tracked_ball;

      found->fill();
    }

    if(!found) {
      printf("Error: we did not find or create a WaterBall. No memory anymore?\n");
      ::exit(EXIT_FAILURE);
    }

    //printf("Tracking: %d, WaterBall: %p, Number of WaterBalls: %ld, age: %d, (%f, %f), state: %d\n", found_tracked_ball.tracked_id, found_tracked_ball.water_ball, ball_drawer.balls.size(), tracked->age, tracked->position.x, tracked->position.y, found_tracked_ball.water_ball->state);
    //printf("Ball found: %p  total tracked now: %ld. Created balls: %ld, age: %d\n", found, tracked_balls.size(), ball_drawer.balls.size());

    //found->position.set(tracked->position.x * scale_x, settings.win_h - (tracked->position.y * scale_y)); 
    //found->enable();
    found->position.set(tracked->position.x * scale_x, (tracked->position.y * scale_y)); 
    //found->position.set(312, 0);
   
  }

}
#endif

#if USE_TIDES
void Swnt::updateTides() {

  if(override_with_gui) {
    return;
  }

  uint64_t now = rx_hrtime();
  if(now < tides_timeout) {
    return;
  }

  float t = 0.0f;
  int hour = rx_get_hour();
  if(hour) {
    t = hour / 23.0;
  }

  int minute = rx_get_minute();
  if(minute) {
    float mt = (minute/59.0f) * 0.1;
    t += mt;
  }

  setTimeOfDay(t);

  tides_timeout = now + (1000000ull * 1000ull * 60ull *  1ull); // 1 minute(s)
}

void Swnt::setTimeOfDay(float t) {

  t = CLAMP(t, 0.0f, 1.0f);
  time_of_day = t;

  float ht = fabs(t);

  float mt = fmod(t * 10, 1.0f);
  printf("Time: %d:%d\n", int(ht * 24), int(mt * 60));

  // Update tides
  TidesEntry entry;
  if(tides.getEntry(rx_get_year(), rx_get_month(), rx_get_day(), entry)) {

    float height = entry.getInterpolatedHeight(t);

    if(entry.min_value < entry.max_value) {
      float level_p = (height - entry.min_value) / (entry.max_value - entry.min_value);
      mask.setScale(level_p);
    }

  }
  else {
    printf("Warning: cannot retrieve tide information.\n");
  }
    
  // Update lighting
  float sun = 0.0f;
  if(has_weather_info) {
    sun = weather_info.getSun(t);
  }
  else {
    printf("Verbose: no weather info yet.\n");
  }

#if USE_WATER
  water.setTimeOfDay(t, sun);
#endif

}
#endif // #USE_TIDES

void Swnt::updateActivityLevel() {
  //float max_tracked = 10; // we assume that max 10 people interact at the same time
  //activity_level = activity_level * 0.9 + (float(tracking.num_tracked) / max_tracked) * 0.1;
  activity_level = CLAMP(activity_level * 0.9 + float(tracking.num_tracked) * 0.1, 0.0f, 1.0f);
  audio.setVolume(SOUND_WATER_FLOWING, activity_level);
  printf("%u, act: %f\n", tracking.num_tracked, activity_level);
}
