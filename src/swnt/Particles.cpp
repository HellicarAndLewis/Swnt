#include <swnt/Particles.h>

// ------------------------------------------------------

Spring::Spring(Particle* a, Particle* b)
  :a(a)
  ,b(b)
  ,rest_length(length(b->pos - a->pos) )
  ,curr_length(rest_length)
  ,k(0.1f)
{

}

void Spring::update(const float dt) {
  if(!a->enabled && !b->enabled) {
    return;
  }

  vec3 dir = b->tmp_pos - a->tmp_pos;
  const float len = length(dir);
  const float inv_mass = a->inv_mass + b->inv_mass;
  const float f = ((rest_length - len) / inv_mass) * k;

  curr_length = len;

  dir /= len;
  dir *= f;
  dir *= dt;

  if(a->enabled) {
    a->tmp_pos -= (dir * a->inv_mass);
  }

  if(b->enabled) {
    b->tmp_pos += (dir * b->inv_mass);
  }
}

// ------------------------------------------------------

Particle::Particle()
  :enabled(true)
  ,age(0.0f)
  ,age_perc(0.0f)
  ,lifetime(100.0f)
  ,rotate_speed(0.0f)
  ,move_speed(0.0f)
  ,size(0.0f)
{
  inv_mass = 1.0f;
}

void Particle::addForce(const vec3& f) {
  forces += f;
}

void Particle::update() {
  age++;
  age_perc = age / lifetime;

  tail.push_back(pos);

  while(tail.size() > 55) {
    tail.erase(tail.begin());
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
  float drag = 0.99f;
  float fps = 1.0f / dt;

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
}

void Particles::addForce(const vec3& f) {
  for(Particles::iterator it = begin(); it != end(); ++it) {
    (*it)->addForce(f);
  }
}

