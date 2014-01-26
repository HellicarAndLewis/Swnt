#include <swnt/Particles.h>

// ------------------------------------------------------

Particle::Particle()
  :enabled(true)
  ,age(0.0f)
  ,age_perc(0.0f)
  ,lifetime(100.0f)
  ,rotate_speed(0.0f)
  ,move_speed(0.0f)
  ,size(0.0f)
  ,max_tail_size(0)
{
  inv_mass = 1.0f;
}

void Particle::addForce(const vec3& f) {
  forces += f;
}

void Particle::update() {
  age++;
  age_perc = age / lifetime;

  if(max_tail_size) {
    tail.push_back(pos);

    while(tail.size() > max_tail_size) {
      tail.erase(tail.begin());
    }
  }
}

// ------------------------------------------------------

Particles::Particles() {
}

Particles::~Particles() {

  Particles::iterator it = begin();
  while(it != end()){
    delete *it;
    it = particles.erase(it);
  }

  particles.clear();
}

void Particles::update(const float dt) {
  float drag = 0.93f;
  float fps = 1.0f / dt;

  for(Particles::iterator it = begin(); it != end(); ++it) {
    Particle* p = *it;

    if(!p->enabled) {
      continue;
    }
    
    p->forces = p->forces * p->inv_mass * dt;
    p->vel += p->forces * dt;
    p->pos += p->vel;
    p->vel *= drag;
    p->forces = 0;
    p->update();
  }
}

void Particles::addForce(const vec3& f) {
  for(Particles::iterator it = begin(); it != end(); ++it) {
    (*it)->addForce(f);
  }
}

