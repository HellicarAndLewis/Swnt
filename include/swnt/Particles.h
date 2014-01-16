#ifndef PARTICLES_H
#define PARTICLES_H

#define ROXLU_USE_MATH
#include "tinylib.h"

#include <vector>

// -------------------------------------------------

class Particle {
 public:
  Particle();
  void addForce(const vec3& f);
  void update();
  void setMass(float m);

 public:
  vec3 pos;
  vec3 vel;
  vec3 forces;
  float inv_mass;
  bool enabled;
  float age;
  float age_perc;
  float lifetime;
  float strip_width; /* the width of the triangle strip */
  float size; /* e.g. use this to scale some texture */
  float size_x; 
  float size_y;
  float rotate_speed; /* e.g. to control rotation speed of your particle */
  float move_speed; /* e.g. use this together with 'dir' to control the force/speed you want for the particle */
  size_t max_tail_size; /* the maximum number of elements we store in the tail; default to 0, no tail */
  std::vector<vec3> tail;

  /* used for different purposes */
  vec3 dir; /* if you want a particle to follow a certain direction you could use this with addForce */
};

// -------------------------------------------------

class Particles {
 public: 

  Particles();
  ~Particles();

  void update(const float dt); /* integrates the particles */
  void addForce(const vec3& f);

  // vector management
  void push_back(Particle* p);
  size_t size();

  typedef std::vector<Particle*>::iterator iterator;
  Particles::iterator begin();
  Particles::iterator end();
  Particles::iterator erase(Particles::iterator it);
  Particle* operator[](size_t dx);

 public:
  std::vector<Particle*> particles; /* particles container; used by the drawer */
};

inline void Particles::push_back(Particle* p) {
  particles.push_back(p);
}

inline size_t Particles::size() {
  return particles.size();
}

inline Particles::iterator Particles::begin() {
  return particles.begin();
}

inline Particles::iterator Particles::end() {
  return particles.end();
}

inline Particles::iterator Particles::erase(Particles::iterator it) {
  Particle* p = *it;
  return particles.erase(it);
}

inline Particle* Particles::operator[](size_t dx) {
#ifndef NDEBUG
  return particles.at(dx);
#else  
  return particles[dx];
#endif
}

inline void Particle::setMass(float mm) {
  if(mm < 0.0001) {
    mm = 0.0001;
  }
  inv_mass = 1.0f / mm;
}

#endif
