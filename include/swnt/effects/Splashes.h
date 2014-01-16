#ifndef SWNT_EFFECTS_SPLASHES_H
#define SWNT_EFFECTS_SPLASHES_H

#include <swnt/Types.h>
#include <swnt/effects/BaseEffect.h>
#include <swnt/Particles.h>
#include <set>

/*
Faked water: http://www.klemenlozar.com/wp-content/uploads/2012/05/Water_Splash.mov

*/

struct SplashVertex { 
  SplashVertex(){}
  SplashVertex(vec3 pos, vec2 tex):pos(pos),tex(tex){}
  void set(vec3 p, vec2 t) { pos = p; tex = t; }
  float* ptr() { return pos.ptr(); } 
  vec3 pos;
  vec2 tex;
};

class Splashes : public BaseEffect {

 public:
  Splashes(Effects& effects);
  ~Splashes();
  bool setup();
  void update();
  void drawExtraFlow();
  void drawExtraDiffuse();
  void createParticle(vec3 position, vec3 direction);

 private:
  void drawStrokes();
  void drawSmoke();
  void resetParticle(Particle* p, vec3 position, vec3 direction);
  void applyFlowField();
  void spawnParticlesAroundContours();

 public:
  Program prog;
  GLuint vbo;
  GLuint vao;
  GLuint bubble_tex;
  GLuint smoke_tex;
  GLuint noise_tex; /* used to offset texture coordinates */
  GLuint normal_tex; /* normals */
  
  Particles ps;
  std::vector<Particle*> smoke_particles;
  std::vector<Particle*> stroke_particles;
  std::set<Particle*> died_particles;
  float lifetime_min;
  float lifetime_max;
  float size_x_min;
  float size_x_max;
  float size_y_min;
  float size_y_max;
  float size_min; // for the smoke
  float size_max; // for the smoke
  float rotate_speed_min;
  float rotate_speed_max;
  float move_speed_min;
  float move_speed_max;
  float texture_anim_speed;
};

inline void Splashes::resetParticle(Particle* p, vec3 position, vec3 direction) {

  p->enabled = true;
  p->pos = position;
  p->dir = normalized(direction);
  p->age = 0.0f;
  p->vel = 0.0f;
  p->forces = 0.0f;
  p->age_perc = 0.0f;

  p->lifetime = rx_random(lifetime_min, lifetime_max);
  p->size_x = rx_random(size_x_min, size_x_max);
  p->size_y = rx_random(size_y_min, size_y_max);
  p->size = rx_random(size_min, size_max);
  p->rotate_speed = rx_random(rotate_speed_min, rotate_speed_max);
  p->move_speed = rx_random(move_speed_min, move_speed_max);

}

#endif
