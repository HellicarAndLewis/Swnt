#include <swnt/Effects.h>
#include <swnt/Swnt.h>
#include <swnt/Settings.h>
#include <assert.h>

Effects::Effects(Swnt& swnt)
  :swnt(swnt)
  ,settings(swnt.settings)
  ,eddy(*this)
  ,splashes(*this)
{
}

Effects::~Effects() {
}

bool Effects::setup(int w, int h) {
  assert(w && h);

  if(!eddy.setup()) {
    printf("Error: cannot setup eddy effect.\n");
    return false;
  }

  if(!splashes.setup()) {
    printf("Error: cannot setup the splashes effect.\n");
    return false;
  }

  ortho_pm.ortho(0.0f, w, h, 0.0f, 0.0, 100.0f);

  return true;
}

void Effects::update() {
  eddy.update();
  splashes.update();
}

void Effects::drawExtraFlow() {
  eddy.drawExtraFlow();
  splashes.drawExtraFlow();
}

void Effects::drawExtraDiffuse() {
  splashes.drawExtraDiffuse();
}
