#include <swnt/Effects.h>
#include <swnt/effects/BaseEffect.h>

BaseEffect::BaseEffect(Effects& effects, int name)
  :effects(effects)
  ,name(name)
{
}

BaseEffect::~BaseEffect() {
}


