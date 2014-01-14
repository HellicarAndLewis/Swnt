#include <assert.h>
#include <swnt/WaterBall.h>

// ------------------------------------

WaterDrop::WaterDrop()
  :mass(0.0f)
  ,inv_mass(0.0f)
{
}

WaterDrop::~WaterDrop() {
}

// ------------------------------------

WaterBall::WaterBall() 
  :win_w(0)
  ,win_h(0)
  ,spawn_timeout(0)
  ,spawn_delay(300)
  ,flush_timeout(0)
  ,flush_delay(100)
  ,state(WATERDROP_STATE_NONE)
  ,repel_force(1000.0f) // squared
  ,attract_force(20.0f)
  ,max_speed(7.0f)
  ,radius_per_drop(0.3f) 
  ,min_drop_size(100.0f)
  ,max_drop_size(150.0f)
  ,min_drop_mass(0.1f)
  ,max_drop_mass(5.0f)
   //,enabled(false)
{
}

WaterBall::~WaterBall() {
  state = WATERDROP_STATE_NONE;
}

bool WaterBall::setup(int w, int h) {
  assert(w && h);
  win_w = w;
  win_h = h;

  position.set(w * 0.5, h * 0.5);
  spawn_timeout = rx_hrtime() + (spawn_delay * 1000);

  /*
  float cx = w * 0.5;
  float cy = h * 0.5;
  int num = 40;
  for(int i = 0; i < num; ++i) {
    addDrop(vec2(rx_random(0, w), rx_random(0, h)), 1.0f);
  }
  */

  state = WATERDROP_STATE_FREE;

  return true;
}

void WaterBall::update(float dt) {

  if(state == WATERDROP_STATE_NONE) {
    return ;
  }
  else if(state == WATERDROP_STATE_FILL) {
  
    if(drops.size() < 10) {
      uint64_t now = rx_hrtime();
      if(now >= spawn_timeout) {
        spawn_timeout = now + spawn_delay * 1000000ull;

        addRandomDrop();
    
        /*
          if(drops.size() == 10) {
          printf("We reached N drops, flush!\n");
          state = WATERDROP_STATE_FLUSH;
          flush_timeout = now + flush_delay * 1000000LLU;
          }
        */
      }
    }
  }
  else if(state == WATERDROP_STATE_FLUSH) {

#if 1
    // Remove the water drop when we need to flush (see below for a version where we remove 
    // water drops only when they are in a certain radius.
    uint64_t now = rx_hrtime();
    if(now >= flush_timeout) {
      if(drops.size()) {
        drops.erase(drops.begin());
        flush_timeout = now + flush_delay * 1000000ull;
        if(!drops.size()) {
          state = WATERDROP_STATE_FREE;
        }
      }
      else {
        state = WATERDROP_STATE_NORMAL;
      }
    }
#endif
  }

  if(!drops.size()) {
    return ;
  }

  vec2 dir;
  float dist = 0.0f;
  float dist_sq = 0.0f;
  vec2 f = 0.0f;
  float radius = drops.size() * radius_per_drop;
  float radius_sq = radius * radius;
  float neighbor_dist = 1;
  float neighbor_dist_sq = neighbor_dist * neighbor_dist;

  // REPEL FORCES:
#if 1
  for(size_t i = 0; i < drops.size(); ++i) {

    WaterDrop& a = drops[i];

    for(size_t j = i + i; j < drops.size(); ++j) {

      WaterDrop& b = drops[j];
      dir = b.position - a.position;
      dist_sq = dot(dir, dir);

      if(dist_sq > neighbor_dist_sq) {
        continue;
      }

      if(dist_sq < 0.01) {
        continue;
      }

      dir = normalized(dir);
      f = repel_force *  (1.0 - (1.0 / dist_sq)) * dir;
      a.forces -= f;
      b.forces += f;
    }
  }
#endif

  // ATTRACT FORCES
  float max_speed_sq = max_speed * max_speed;
  float speed_sq; 
  float k = 1.0f;
  uint64_t now = rx_hrtime();
   
  std::vector<WaterDrop>::iterator it = drops.begin();
  while(it != drops.end()) {
    WaterDrop& d = *it;
    {
      dir = position - d.position;
      dist = length(dir);

      if(dist < 0.01) {
        dist = 0.01;
      }

      if(dist > radius) {  /* when the particle is outiside the radius, it's attracted a lot more to the center */
        k = 15.0;
      }
#if 0
      // This is where we flush the water drops! 
      if(state == WATERDROP_STATE_FLUSH  && dist < radius) {
        if(now >= flush_timeout) {
          if(drops.size()) {
            it = drops.erase(drops.begin());
            flush_timeout = now + flush_delay * 1000000ull;

            if(!drops.size()) {
              state = WATERDROP_STATE_FREE;
              break;
            }
            continue;
          }
        }
      }
#endif

      dir /= dist;
      f = k * attract_force * (dist/radius) * dir;
      d.forces += f;
    }

    d.forces *= d.inv_mass * dt;
    d.velocity += d.forces * dt;
    d.position += d.velocity;
    d.forces = 0;

    // we do not add a fake drag force, but we limit the speed, this will make the 
    // particles bounce forever.
    speed_sq = dot(d.velocity, d.velocity);
    if(speed_sq > max_speed_sq) {
      d.velocity = normalized(d.velocity);
      d.velocity *= max_speed;
    }

    ++it;
  }
  /*
  glBindBuffer(GL_ARRAY_BUFFER, basic_vbo);

  size_t bytes_needed = sizeof(WaterDrop) * drops.size();
  if(bytes_needed > bytes_allocated) {
    glBufferData(GL_ARRAY_BUFFER, bytes_needed, drops[0].position.ptr(), GL_STREAM_DRAW);
  }
  else {
    glBufferSubData(GL_ARRAY_BUFFER, 0, bytes_needed, drops[0].position.ptr());
  }
  */
}

/*
void WaterBall::draw() {
  
  if(state == WATERDROP_STATE_NONE) {
    return;
  }

  drawParticlesWithAlpha();
  drawParticlesWithWaterEffect();
  drawRenderBuffers();
}
*/

void WaterBall::addDrop(vec2 position, float mass) {
  if(mass < 0.01) {
    mass = 0.01;
  }

  WaterDrop drop;
  drop.mass = mass;
  drop.inv_mass = 1.0f / mass;
  drop.position = position;
  drop.size = rx_random(min_drop_size, max_drop_size);

  drops.push_back(drop);
}

void WaterBall::addRandomDrop() {
  float s = 20;
  addDrop(vec2(rx_random(position.x - s, position.x + s), 
               rx_random(position.y - s, position.y + s)), 
               rx_random(min_drop_mass, max_drop_mass));
}

// --------------------------------------------------------------------

WaterBallDrawer::WaterBallDrawer() 
  :win_w(0)
  ,win_h(0)
  ,basic_vbo(0)
  ,basic_vao(0)
  ,water_vao(0)
  ,normals_tex(0)
  ,alpha_tex(0)
  ,background_tex(0)
  ,fbo(0)
  ,scene_alpha_tex(0)
  ,scene_normals_tex(0)
  ,bytes_allocated(0)
{

}

WaterBallDrawer::~WaterBallDrawer() {
  printf("Free allocated water balls.\n");
}

bool WaterBallDrawer::setup(int w, int h) {
  assert(w && h);
  win_w = w;
  win_h = h;
  pm.ortho(0, w, h, 0, 0.0f, 100.0f);

  // create basic shader 
  const char* atts[] = { "a_pos", "a_size" } ;
  const char* frags[] = { "out_normals", "out_alpha" };
  basic_prog.create(GL_VERTEX_SHADER, rx_to_data_path("shaders/waterdrop_basic.vert"));
  basic_prog.create(GL_FRAGMENT_SHADER, rx_to_data_path("shaders/waterdrop_basic.frag"));
  basic_prog.link(2, atts, 2, frags);
  glUseProgram(basic_prog.id);
  glUniformMatrix4fv(glGetUniformLocation(basic_prog.id, "u_pm"), 1, GL_FALSE, pm.ptr());
  glUniform1i(glGetUniformLocation(basic_prog.id, "u_normals_tex"), 0);
  glUniform1i(glGetUniformLocation(basic_prog.id, "u_alpha_tex"), 1);

  // create water render shader
  water_prog.create(GL_VERTEX_SHADER, rx_to_data_path("shaders/waterdrop_water.vert"));
  water_prog.create(GL_FRAGMENT_SHADER, rx_to_data_path("shaders/waterdrop_water.frag"));
  water_prog.link();
  glUseProgram(water_prog.id);
  glUniform1i(glGetUniformLocation(water_prog.id, "u_normals_tex"), 0);
  glUniform1i(glGetUniformLocation(water_prog.id, "u_alpha_tex"), 1);
  glUniform1i(glGetUniformLocation(water_prog.id, "u_background_tex"), 2);

  glGenVertexArrays(1, &water_vao);

  glGenVertexArrays(1, &basic_vao);
  glBindVertexArray(basic_vao);
  glGenBuffers(1, &basic_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, basic_vbo);
  
  glEnableVertexAttribArray(0); // pos
  glEnableVertexAttribArray(1); // size
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(WaterDrop), (GLvoid*) 0);
  glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(WaterDrop), (GLvoid*) 32);

  glVertexAttribDivisor(0, 1);
  glVertexAttribDivisor(1, 1);

  normals_tex = rx_create_texture(rx_to_data_path("images/waterdrop_normals.png"));
  alpha_tex = rx_create_texture(rx_to_data_path("images/waterdrop_alpha.png"));
  background_tex = rx_create_texture(rx_to_data_path("images/waterdrop_background.png"));
  glBindTexture(GL_TEXTURE_2D, background_tex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  // fbo
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  
  glGenTextures(1, &scene_normals_tex);
  glBindTexture(GL_TEXTURE_2D, scene_normals_tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, win_w, win_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, scene_normals_tex, 0);

  glGenTextures(1, &scene_alpha_tex);
  glBindTexture(GL_TEXTURE_2D, scene_alpha_tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, win_w, win_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, scene_alpha_tex, 0);

  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    printf("Framebuffer not complete.\n");
    return false;
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  return true;
}

void WaterBallDrawer::update(float dt) {
  
  uint64_t n = rx_hrtime();

  // Copy all drops into a buffer.
  drops.clear();
  for(std::vector<WaterBall*>::iterator it = balls.begin(); it != balls.end(); ++it) {
    WaterBall* b = *it;

    if(b->state == WATERDROP_STATE_NONE 
       || b->state == WATERDROP_STATE_FREE) 
      {
        continue;
      }

    b->update(dt);

    std::copy(b->drops.begin(), b->drops.end(), std::back_inserter(drops));
  }

  if(!drops.size()) {
    return;
  }

  // update vbo
  glBindBuffer(GL_ARRAY_BUFFER, basic_vbo);
  size_t bytes_needed = sizeof(WaterDrop) * drops.size();
  if(bytes_needed > bytes_allocated) {
    glBufferData(GL_ARRAY_BUFFER, bytes_needed, drops[0].position.ptr(), GL_STREAM_DRAW);
    bytes_allocated = bytes_needed;
  }
  else {
    glBufferSubData(GL_ARRAY_BUFFER, 0, bytes_needed, drops[0].position.ptr());
  }

  //printf("We got: %ld drops, took: %f ms.\n", drops.size(), (rx_hrtime() - n)/1000000.0);

  drawParticlesWithAlpha();
}

void WaterBallDrawer::draw() {
  drawParticlesWithWaterEffect();
}

void WaterBallDrawer::drawParticlesWithAlpha() {

  if(!drops.size()) {
    return;
  }

  GLenum drawbufs[] = { GL_COLOR_ATTACHMENT0,  GL_COLOR_ATTACHMENT1  } ;
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glDrawBuffers(2, drawbufs);
  glClear(GL_COLOR_BUFFER_BIT);
  glViewport(0, 0, win_w, win_h);

  glBindVertexArray(basic_vao);
  glUseProgram(basic_prog.id);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, normals_tex);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, alpha_tex);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  //glBlendFunc(GL_ONE, GL_ONE);
  glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, drops.size());

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void WaterBallDrawer::drawParticlesWithWaterEffect() {

  if(!drops.size()) {
    return;
  }

  //glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, scene_normals_tex);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, scene_alpha_tex);

  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, background_tex);

  glBindVertexArray(water_vao);
  glUseProgram(water_prog.id);
  glUniform1f(glGetUniformLocation(water_prog.id, "u_time"), rx_millis() * 0.1);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void WaterBallDrawer::drawRenderBuffers() {

  glDisable(GL_BLEND);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

  glReadBuffer(GL_COLOR_ATTACHMENT1);
  glBlitFramebuffer(0, 0, win_w, win_h, 0, 0, 320, 240, GL_COLOR_BUFFER_BIT, GL_LINEAR);
  
  glReadBuffer(GL_COLOR_ATTACHMENT0);
  glBlitFramebuffer(0, 0, win_w, win_h, 0, 240, 320, 480, GL_COLOR_BUFFER_BIT, GL_LINEAR);
}

void WaterBallDrawer::print() {
  printf("waterball.normals_tex: %d\n", normals_tex);
  printf("waterball.alpha_tex: %d\n", alpha_tex);
  printf("waterball.scene_alpha_tex: %d\n", scene_alpha_tex);
  printf("waterball.scene_normals_tex.: %d\n", scene_normals_tex);
}

void WaterBallDrawer::addWaterBall(WaterBall* ball) {

  if(!ball->setup(win_w, win_h)) {
    printf("Error: cannot setup the water ball.\n");
    ::exit(EXIT_FAILURE);
  }

  balls.push_back(ball);
}
