#include <swnt/Effects.h>
#include <assert.h>

Effects::Effects() 
  :mist(*this)
{
}

Effects::~Effects() {
}

bool Effects::setup(int w, int h) {
  assert(w && h);

  if(!mist.setup()) {
    printf("Error: cannot setup mist effect.\n");
    return false;
  }

  ortho_pm.ortho(0.0f, w, h, 0.0f, 0.0, 100.0f);

  return true;
}

void Effects::update() {
  mist.update();
}

void Effects::draw() {
  mist.draw();
}
