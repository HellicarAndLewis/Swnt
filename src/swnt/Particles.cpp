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

  /*
  // PREDICT NEW LOCATIONS
  for(Particles::iterator it = begin(); it != end(); ++it) {
    Particle* p = *it;
    
    if(!p->enabled) {
      p->tmp_pos = p->pos;
      continue;
    }

    p->vel = p->vel + (dt * p->forces * p->inv_mass);
    p->vel *= drag;

    p->tmp_pos = p->pos + (p->vel * dt);

    p->forces.set(0.0f, 0.0f, 0.0f);
  }

  // CONSTRAINTS 

  const int k = 3;
  for(int i = 0; i < k; ++i) {
    for(std::vector<Spring*>::iterator it = springs.begin(); it != springs.end(); ++it) {
      Spring* s = *it;
      s->update(dt);
    }
  }

  // UPDATE VELOCITY AND POSITIONS
  for(Particles::iterator it = begin(); it != end(); ++it) {
    Particle* p = *it;

    if(!p->enabled) {
      continue;
    }

    p->vel = (p->tmp_pos - p->pos) * fps;
    p->pos = p->tmp_pos;
    p->tmp_pos.set(0.0f, 0.0f, 0.0f);

    p->update();
  }
  */
}

void Particles::addForce(const vec3& f) {
  for(Particles::iterator it = begin(); it != end(); ++it) {
    (*it)->addForce(f);
  }
}

