#define ROXLU_USE_ALL
#include <tinylib.h>
#include <swnt/Swnt.h>
#include <swnt/Effects.h>
#include <swnt/effects/Splashes.h>
#include <swnt/Flow.h>
#include <swnt/Tracking.h>

Splashes::Splashes(Effects& effects) 
  :BaseEffect(effects, EFFECT_SPLASHES)
  ,vbo(0)
  ,vao(0)
  ,bubble_tex(0)
  ,lifetime_min(15.0f)
  ,lifetime_max(20.0f)
  ,size_x_min(75.0f)
  ,size_x_max(120.0f)
  ,size_y_min(215.0f)
  ,size_y_max(250.0f)
  ,rotate_speed_min(-4.5)
  ,rotate_speed_max(4.5)
  ,move_speed_min(35.0)
  ,move_speed_max(50.0f)
  ,size_min(60.0f)
  ,size_max(90.0f)
  ,texture_anim_speed(0.3)
  ,normal_tex(0)
{

}

Splashes::~Splashes() {
}

bool Splashes::setup() {

  // create the shader
  const char* attribs[] = { "a_pos", "a_tex" } ;
  prog.create(GL_VERTEX_SHADER, rx_to_data_path("shaders/splash.vert"));
  prog.create(GL_FRAGMENT_SHADER, rx_to_data_path("shaders/splash.frag"));
  prog.link(2, attribs);

  // create the vertices.
  std::vector<SplashVertex> vertices(4, SplashVertex());
  vertices[0].set(vec3(-0.5, -0.5, 0.0), vec2(0.0, 0.0));
  vertices[1].set(vec3(-0.5, 0.5, 0.0), vec2(0.0, 1.0));
  vertices[2].set(vec3(0.5, -0.5, 0.0), vec2(1.0, 0.0));
  vertices[3].set(vec3(0.5, 0.5, 0.0), vec2(1.0, 1.0));
  
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  glEnableVertexAttribArray(0); // pos
  glEnableVertexAttribArray(1); // tex
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SplashVertex), (GLvoid*)0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(SplashVertex), (GLvoid*)12);
  glBufferData(GL_ARRAY_BUFFER, sizeof(SplashVertex) * vertices.size(), vertices[0].ptr(), GL_STATIC_DRAW);
  
  // tex
  bubble_tex = rx_create_texture(rx_to_data_path("images/water_splash_diffuse.png"));
  normal_tex= rx_create_texture(rx_to_data_path("images/water_splash_normals.png"));
  smoke_tex =  rx_create_texture(rx_to_data_path("images/water_smoke.png"));
  noise_tex =  rx_create_texture(rx_to_data_path("images/water_noise.png"));

  glUseProgram(prog.id);
  rx_uniform_1i(prog.id, "u_diffuse_tex", 0);
  rx_uniform_1i(prog.id, "u_normal_tex", 1);
  return true;
}


void Splashes::update() {
  //  applyFlowField();
  spawnParticlesAroundContours();
  ps.update(1.0f/30.0f);
}

void Splashes::drawExtraFlow() {

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_DEPTH_TEST);
  // glBlendFunc(GL_ONE, GL_ONE);

  glBindVertexArray(vao);
  glUseProgram(prog.id);
  glUniformMatrix4fv(glGetUniformLocation(prog.id, "u_pm"), 1, GL_FALSE, effects.ortho_pm.ptr());
  glUniform1f(glGetUniformLocation(prog.id, "u_time"), rx_millis() * 1.0);
  
  //drawStrokes();
  drawSmoke();

  glDisable(GL_BLEND);
}

void Splashes::drawExtraDiffuse() {
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_DEPTH_TEST);
  //  glBlendFunc(GL_ONE, GL_ONE);

  glBindVertexArray(vao);
  glUseProgram(prog.id);
  glUniformMatrix4fv(glGetUniformLocation(prog.id, "u_pm"), 1, GL_FALSE, effects.ortho_pm.ptr());
  glUniform1f(glGetUniformLocation(prog.id, "u_time"), rx_millis() * texture_anim_speed);
  
  //drawSmoke();
  drawStrokes();

  glDisable(GL_BLEND);
}

#define USE_SPLASH_LINES 0

void Splashes::drawStrokes() {
#if USE_SPLASH_LINES
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
#endif

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, bubble_tex); 

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, normal_tex); 

  mat4 mm;
  GLint u_age_perc = glGetUniformLocation(prog.id, "u_age_perc");
  GLint u_mm = glGetUniformLocation(prog.id, "u_mm");

  std::vector<Particle*>::iterator it = stroke_particles.begin(); 
  while(it != stroke_particles.end()) {
    Particle* p = *it;
    vec3 d = normalized(p->vel);

    if(p->age_perc >= 1.0) {
      died_particles.insert(p);
      p->enabled = false;
      ++it;
      continue;
    }

    mm[0] = d.x * p->size_y;   mm[4] = -d.y * p->size_x;   mm[8] = 0.0f;   mm[12] = p->pos.x;
    mm[1] = d.y * p->size_y;   mm[5] = d.x * p->size_x;    mm[9] = 0.0f;   mm[13] = p->pos.y;
    mm[2] = 0.0f;              mm[6] = 0.0f;               mm[10] = 1.0f;  mm[14] = p->pos.z;
    mm[3] = 0.0f;              mm[7] = 0.0f;               mm[11] = 0.0f;  mm[15] = 1.0f;

    p->addForce(p->dir * p->move_speed); 

    glUniform1f(u_age_perc, p->age_perc);
    glUniformMatrix4fv(u_mm, 1, GL_FALSE, mm.ptr());
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    ++it;
  }
#if USE_SPLASH_LINES
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif
}

void Splashes::drawSmoke() {
#if USE_SPLASH_LINES
  return;
#endif  

  return;

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, smoke_tex); 

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, normal_tex); 

  glUniform1f(glGetUniformLocation(prog.id, "u_time"), 1.0);

  mat4 mm;
  GLint u_age_perc = glGetUniformLocation(prog.id, "u_age_perc");
  GLint u_mm = glGetUniformLocation(prog.id, "u_mm");

  std::vector<Particle*>::iterator it = smoke_particles.begin(); 
  std::vector<Particle*> to_remove;
  while(it != smoke_particles.end()) {
    Particle* p = *it;

    if(p->age_perc >= 1.0) {
      p->enabled = false;
      died_particles.insert(p);
      ++it;
      continue;
    }

    p->addForce(p->dir * p->move_speed); 

    mm.identity();
    mm.translate(p->pos.x, p->pos.y, 0.0f);
    mm.rotateZ(p->age_perc * p->rotate_speed);
    mm.scale(p->size, p->size, 1.0);

    glUniform1f(u_age_perc, p->age_perc);
    glUniformMatrix4fv(u_mm, 1, GL_FALSE, mm.ptr());
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    ++it;
  }
}

void Splashes::createParticle(vec3 position, vec3 direction) {

  Particle* p = NULL;

  if(died_particles.size()) {
    p = *died_particles.begin();
    died_particles.erase(died_particles.begin());
    resetParticle(p, position, direction);
  }
  else {
    p = new Particle();
    resetParticle(p, position, direction);
    ps.push_back(p);
    stroke_particles.push_back(p);
    smoke_particles.push_back(p);
  }

}

void Splashes::applyFlowField() {
  Flow& flow = effects.swnt.flow;
  float scale_x = 1.0/effects.settings.win_w;
  float scale_y = 1.0/effects.settings.win_h;

  for(Particles::iterator it = ps.begin(); it != ps.end(); ++it) {
    Particle* p = *it;
    int i = (scale_x * p->pos.x) * flow.field_size;
    int j = (scale_y * p->pos.y) * flow.field_size;
    i = CLAMP(i, 0, flow.field_size);
    j = CLAMP(j, 0, flow.field_size);
    int dx = j * flow.field_size + i;
    vec2 vel = flow.velocities[dx];
    p->dir.set(vel.x, vel.y, 0.0f);
    
  }
}

void Splashes::spawnParticlesAroundContours() {
  Tracking& tr = effects.swnt.tracking;

#if 0 
  // spawn from the contour points that CV found
  std::vector<std::vector<cv::Point> >::iterator cit = tr.contours.begin();
  while(cit != tr.contours.end()) {
    std::vector<cv::Point>& points = *cit;
    std::vector<cv::Point>::iterator pit = points.begin();
    while(pit != points.end()) {
      cv::Point& point = *pit;
      createParticle(vec3(point.x, point.y, 0.0), vec3(0.0, 0.0, 0.0));
      ++pit;
    }
    ++cit;
  }
#else
  // use random points on the extracted contounrs (and scale them from image processing space to window space)
  float scale_x = float(effects.settings.win_w) / effects.settings.image_processing_w;
  float scale_y = float(effects.settings.win_h) / effects.settings.image_processing_h;

  int spawn_per_tracked = 60;
  if(tr.contour_vertices.size()) {

#define USE_SPLASH_RANDOMLY 0
#define USE_SPLASH_PERPENDICULAR 1

#if USE_SPLASH_RANDOMLY
    for(int i = 0; i < spawn_per_tracked; ++i) {
      size_t dx = rx_random(0, tr.contour_vertices.size()-1);
      vec2 point = tr.contour_vertices[dx];
      createParticle(vec3(point.x * scale_x, point.y * scale_y, 0.0), vec3(0.0, 0.0, 0.0));
    }
#endif

#if USE_SPLASH_PERPENDICULAR
    int step_size = 30;

    for(int i = 0; i < tr.contour_vertices.size()-(step_size+1); i += step_size) {
      if( (i + step_size) > tr.contour_vertices.size()) { 
         break;
      }
      vec2 p0 = tr.contour_vertices[i + 0];
      vec2 p1 = tr.contour_vertices[i + step_size];
      vec2 dir = p1-p0;
      vec2 crossed(-dir.y, dir.x);
      crossed = normalized(crossed);

      createParticle(vec3(p0.x * scale_x, p0.y * scale_y, 0.0), -vec3(crossed.x, crossed.y, 0.0));
      //      if(i > 10) { 
      //          break;
      //      }
    }
#endif

  }

#endif
}
