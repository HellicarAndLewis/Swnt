#include <swnt/Spirals.h>
#include <swnt/Graphics.h>
#include <swnt/Settings.h>
#include <swnt/Tracking.h>
#include <swnt/Flow.h>

Spirals::Spirals(Settings& settings, Tracking& tracker, Graphics& graphics, Flow& flow)
  :settings(settings)
  ,tracker(tracker)
  ,graphics(graphics)
  ,flow(flow)
  ,bytes_allocated(0)
  ,vao(0)
  ,vbo(0)
  ,vert(0)
  ,frag(0)
  ,prog(0)
  ,center_particle(NULL)
  ,min_lifetime(5.0f)
  ,max_lifetime(10.0f)
  ,min_strip_width(1.5f)
  ,max_strip_width(7.5f)
  ,min_mass(1.0f)
  ,max_mass(1.4f)
{
}

bool Spirals::setup() {


  #if 0
  center_particle = new Particle();
  center_particle->enabled = false;
  center_particle->pos.set(settings.win_w * 0.5, settings.win_h * 0.5, 0.0);
  center_particle->tmp_pos = center_particle->pos;
  particles.push_back(center_particle);
  #endif
  
  #if 0
  int num = 5;
  for(int i = 0; i < num; ++i) {
    Particle* p = new Particle();
    p->enabled = false;
    p->pos.set(rx_random(0, 320), rx_random(0, 320), 0.0);
    particles.push_back(p);

    #if 0
    Spring* s = new Spring(center_particle, p);
    s->rest_length = rx_random(100,110);
    s->curr_length = s->rest_length;
    s->k = 1.0f;
    particles.addSpring(s);
    #endif
  }
  #endif

  if(!setupGraphics()) {
    printf("Error: cannot setup the graphics for the spirals.\n");
    return false;
  }

  pm.ortho(0.0f, settings.win_w, settings.win_h, 0.0f,  -1.0f, 100.0f);
  refresh(); // make sure that we use all the correct settings. 
  return true;
}

bool Spirals::setupGraphics() {

  // Shader that renders the spirals.
  vert = rx_create_shader(GL_VERTEX_SHADER, SPIRAL_VS);
  frag = rx_create_shader(GL_FRAGMENT_SHADER, SPIRAL_FS);
  prog = rx_create_program(vert, frag);
  glBindAttribLocation(prog, 0, "a_pos");
  glBindAttribLocation(prog, 1, "a_tex");
  glBindAttribLocation(prog, 2, "a_age_perc");
  glBindAttribLocation(prog, 3, "a_pos_perc");
  glLinkProgram(prog);
  rx_print_shader_link_info(prog);
  glUseProgram(prog);
  glUniform1i(glGetUniformLocation(prog, "u_diffuse_tex"), 0);

  // Displacement pass 
  // --
  displacement_frag = rx_create_shader(GL_FRAGMENT_SHADER, SPIRAL_DISPLACEMENT_FS);
  displacement_prog = rx_create_program(vert, displacement_frag);
  glBindAttribLocation(displacement_prog, 0, "a_pos");
  glBindAttribLocation(displacement_prog, 1, "a_tex");
  glBindAttribLocation(displacement_prog, 2, "a_age_perc");
  glBindAttribLocation(displacement_prog, 3, "a_pos_perc");
  glLinkProgram(displacement_prog);
  rx_print_shader_link_info(displacement_prog);
  glUseProgram(displacement_prog);
  glUniform1i(glGetUniformLocation(displacement_prog, "u_diffuse_tex"), 0);

  glGenFramebuffers(1, &displacement_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, displacement_fbo);
  glGenTextures(1, &displacement_tex);
  glBindTexture(GL_TEXTURE_2D, displacement_tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, settings.win_w, settings.win_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, displacement_tex, 0);
  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    printf("Error: the displacement framebuffer is not complete.\n");
    return false;
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  // -- 

  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glEnableVertexAttribArray(0); // pos
  glEnableVertexAttribArray(1); // age perc
  glEnableVertexAttribArray(2); // pos perc
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SpiralVertex), (GLvoid*)0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(SpiralVertex), (GLvoid*)12);
  glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(SpiralVertex), (GLvoid*)20);
  glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(SpiralVertex), (GLvoid*)24);


  // Load the texture for the spiral
  diffuse_tex = graphics.createTexture(rx_to_data_path("images/spiral_diffuse.png"));
  printf("spirals.diffuse_tex: %d\n", diffuse_tex);
  return true;
}

// See for an old physics implementation with : 
// - https://gist.github.com/roxlu/ddf808c2075c599b7dee   (v0.0.1)
// - https://gist.github.com/roxlu/c6228dc856915e45c912   (v0.0.2)
// repulsion and rotat
void Spirals::update(float dt) {
  spawnParticles();

  applyVelocityField();
  flow.applyPerlinToField();
  applyVortexToField();
  applyCenterForce();

  particles.update(1);
  updateVertices();
}

void Spirals::applyVelocityField() {

  float scale_x = flow.field_size / float(settings.win_w);
  float scale_y = flow.field_size / float(settings.win_h);
  float f = 0.015;
  for(Particles::iterator it = particles.begin(); it != particles.end(); ++it) {

    Particle* p = *it;
    if(!p->enabled) {
      continue;
    }
    
    int row = p->pos.y * scale_y;
    int col = p->pos.x * scale_x;

    if(row < 0 || col < 0) {
      continue;
    }

    size_t dx = MIN((flow.field_size-1)*(flow.field_size-1), row * flow.field_size + col);
    vec2& v = flow.velocities[dx];
    p->addForce(vec3(v.x * f, v.y * f, 0.0f));
  }
}

void Spirals::spawnParticles() {

  Particles::iterator it = particles.begin();

  while(it != particles.end()) {

    Particle* p = *it;

    if(p == center_particle) {
      ++it;
      continue;
    }

    if(p->age_perc >= 1.0f) {
      delete p;
      it = particles.erase(it);
      continue;
    }

    ++it;
  }

  // Spawn from tracked position (k-means)
  float scale_x = float(settings.win_w) / settings.image_processing_w;
  float scale_y = float(settings.win_h) / settings.image_processing_h;
  int spawn_per_tracked = 5;
  float displace = 50.0f;

  #if 0
  for(std::vector<Tracked*>::iterator tit = tracker.tracked.begin(); tit != tracker.tracked.end(); ++tit) {

    Tracked* tr = *tit;
    if(!tr->matched) {
      continue;
    }

    for(int i = 0; i < spawn_per_tracked; ++i) {

      Particle* p = new Particle();
      p->pos = vec3(tr->position.x * scale_x + rx_random(-displace, displace), 
                    tr->position.y * scale_y + rx_random(-displace, displace), 
                    0.0f);
      p->tmp_pos = p->pos;
      p->forces.set(0.0f, 0.0f, 0.0f);
      p->vel.set(0.0f, 0.0f, 0.0f);
      p->lifetime = rx_random(min_lifetime, max_lifetime);
      p->strip_width = rx_random(min_strip_width, max_strip_width);
      p->setMass(rx_random(min_mass, max_mass));
      particles.push_back(p);
      
      #if 0
      Spring* s = new Spring(p, center_particle);
      s->rest_length = 0.1;
      particles.addSpring(s);
      #endif
    }

  }
  #endif

  // Spawn from contour
  spawn_per_tracked = 360;
  if(tracker.contour_vertices.size()) {

    for(int i = 0; i < spawn_per_tracked; ++i) {
      size_t dx = rx_random(0, tracker.contour_vertices.size()-1);
      vec2 position = tracker.contour_vertices[dx];

      Particle* p = new Particle();
      p->pos = vec3(position.x * scale_x + rx_random(10.0f), position.y * scale_y + rx_random(10.0f), 0.0f);
      p->tmp_pos = p->pos;
      p->forces.set(0.0f, 0.0f, 0.0f);
      p->vel.set(0.0f, 0.0f, 0.0f);
      p->lifetime = rx_random(min_lifetime, max_lifetime);
      p->strip_width = rx_random(min_strip_width, max_strip_width);
      p->setMass(rx_random(min_mass, max_mass));
      particles.push_back(p);
    }
  }
}

void Spirals::updateVertices() {
  vertices.clear();
  vertex_offsets.clear();
  vertex_counts.clear();

  vec3 dir;
  vec3 perp;
  float size = 3.0f;
  float perc = 0.0f;
  vec3 rotate_axis(0.0f, 0.0f, 1.0f);

  for(Particles::iterator it = particles.begin(); it != particles.end(); ++it) {

    vertex_offsets.push_back(vertices.size());
    {
      Particle* p = *it;

      if(!p->enabled) {
        continue;
      }

      if(p->tail.size() < 2) {
        continue;
      }

      for(size_t i = 0; i < (p->tail.size()-1); ++i) {

        // Get perpendicular vertex
        perc = float(i)/p->tail.size();
        dir = p->tail[i+1] - p->tail[i];
        perp = normalized(cross(dir, rotate_axis)) * (p->strip_width * perc);

        // vertex A
        SpiralVertex v;
        v.pos = vec3(p->tail[i]) - perp;
        v.age_perc = p->age_perc;
        v.pos_perc = perc;
        v.tex.x = perc;
        v.tex.y = -1.0f;
        vertices.push_back(v);

        // vertex B
        v.pos = p->tail[i] + perp;
        v.tex.y = 1.0f;
        vertices.push_back(v);

      }
    }
    vertex_counts.push_back(vertices.size() - vertex_offsets.back());
  }

  if(!vertices.size()) {
    return;
  }

  size_t bytes_needed = sizeof(SpiralVertex) * vertices.size();

  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  if(bytes_needed > bytes_allocated) {
    glBufferData(GL_ARRAY_BUFFER, bytes_needed, vertices[0].ptr(), GL_STREAM_DRAW);
    bytes_allocated = bytes_needed;
  }
  else {
    glBufferSubData(GL_ARRAY_BUFFER, 0, bytes_needed, vertices[0].ptr());
  }
}

void Spirals::draw() {

  if(!vertices.size()) {
    return;
  }

  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  //glBlendFunc(GL_ONE, GL_ONE);
  draw(1.0f);
  glDisable(GL_BLEND);
}

void Spirals::draw(float alpha) {
  glBindVertexArray(vao);
  glUseProgram(prog);
  glUniformMatrix4fv(glGetUniformLocation(prog, "u_pm"), 1, GL_FALSE, pm.ptr());
  glUniformMatrix4fv(glGetUniformLocation(prog, "u_vm"), 1, GL_FALSE, vm.ptr());
  glUniform1f(glGetUniformLocation(prog, "u_alpha"), alpha);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, diffuse_tex);

  glMultiDrawArrays(GL_TRIANGLE_STRIP, &vertex_offsets[0], &vertex_counts[0], vertex_counts.size());

}


void Spirals::drawDisplacement() {
  GLenum db[] = { GL_COLOR_ATTACHMENT0 } ;
  glBindFramebuffer(GL_FRAMEBUFFER, displacement_fbo);
  glDrawBuffers(1, db);
  glClear(GL_COLOR_BUFFER_BIT);

  glBindVertexArray(vao);
  glUseProgram(displacement_prog);
  glUniformMatrix4fv(glGetUniformLocation(displacement_prog, "u_pm"), 1, GL_FALSE, pm.ptr());
  glUniformMatrix4fv(glGetUniformLocation(displacement_prog, "u_vm"), 1, GL_FALSE, vm.ptr());

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, diffuse_tex);

  glMultiDrawArrays(GL_TRIANGLE_STRIP, &vertex_offsets[0], &vertex_counts[0], vertex_counts.size());

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Spirals::applyVortexToField() {

  for(std::vector<Tracked*>::iterator tit = tracker.tracked.begin(); tit != tracker.tracked.end(); ++tit) {

    Tracked* tr = *tit;

    if(!tr->matched) {
      continue;
    }

    float px = tr->position.x/settings.image_processing_w;
    float py = tr->position.y/settings.image_processing_h;
    flow.createVortex(px, py);
  }
}

void Spirals::refresh() {
  assert(settings.color_dx < settings.colors.size());
  glUseProgram(prog);
  glUniform3fv(glGetUniformLocation(prog, "u_col_from"), 1, settings.colors[settings.color_dx].spiral_from.ptr());
  glUniform3fv(glGetUniformLocation(prog, "u_col_to"), 1, settings.colors[settings.color_dx].spiral_to.ptr());
}

void Spirals::applyCenterForce() {

  float radius = settings.radius;
  float min_dist = (radius * 0.7) + (radius * 0.7);
  float max_dist = radius * radius;
  vec3 center(settings.win_w * 0.5, settings.win_h * 0.5, 0.0f);

  for(Particles::iterator it = particles.begin(); it != particles.end(); ++it) {

    Particle* p = *it;

    if(!p->enabled) {
      continue;
    }

    if(p->strip_width < 4.0f) {
      continue;
    }

    if(p->age_perc < 0.3 ) {
      continue;
    }

    vec3 dir = center - p->pos;
    float ls = dot(dir, dir);

    if(ls < 0.001) {
      ls = 0.001;
    }
    else if(ls > max_dist) {
      ls = max_dist;
    }

    float df = ls / max_dist;
    dir = normalized(dir);
    vec3 f = dir * (1.0 - (1.0/ls));
    p->addForce(f * 2.6);

    f.set(-f.y, f.x, 0.0f);
    p->addForce(f * 0.8);
  }
}
