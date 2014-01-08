#ifndef SWNT_BASE_EFFECT_H
#define SWNT_BASE_EFFECT_H

class Effects;

class BaseEffect {

 public:
  BaseEffect(Effects& effect, int name);
  virtual ~BaseEffect();

  virtual bool setup() = 0;
  virtual void update() = 0;
  virtual void draw() = 0;

 public:
  Effects& effects;
  int name;
};

#endif
