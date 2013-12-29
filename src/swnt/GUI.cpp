#include <assert.h>
#include <GLFW/glfw3.h>
#include <swnt/GUI.h>
#include <swnt/Water.h>

GUI::GUI(Water& water) 
  :water(water),
   win_w(0)
  ,win_h(0)
  ,bar(NULL)
{
}

GUI::~GUI() {
  //TwTerminate();
}

bool GUI::setup(int w, int h) {
  assert(w);
  assert(h);
  win_w = w;
  win_h = h;

  TwInit(TW_OPENGL_CORE, NULL);
  bar = TwNewBar("SWNT");
  TwDefine("SWNT size='300 400'");

  TwAddVarRW(bar, "sun_pos.x", TW_TYPE_FLOAT, &water.sun_pos[0], "min=-300.0 max=300.0 step=0.01 group='Water'");
  TwAddVarRW(bar, "sun_pos.y", TW_TYPE_FLOAT, &water.sun_pos[1], "min=-300.0 max=300.0 step=0.01 group='Water'");
  TwAddVarRW(bar, "sun_pos.z", TW_TYPE_FLOAT, &water.sun_pos[2], "min=-300.0 max=300.0 step=0.01 group='Water'");
  TwAddVarRW(bar, "sun_color.r", TW_TYPE_FLOAT, &water.sun_color[0], "min=-5.0 max=5.0 step=0.1 group='Water'");
  TwAddVarRW(bar, "sun_color.g", TW_TYPE_FLOAT, &water.sun_color[1], "min=-5.0 max=5.0 step=0.1 group='Water'");
  TwAddVarRW(bar, "sun_color.b", TW_TYPE_FLOAT, &water.sun_color[2], "min=-5.0 max=5.0 step=0.1 group='Water'");
  TwAddVarRW(bar, "sun_shininess", TW_TYPE_FLOAT, &water.sun_shininess, "min=0.0 max=50.0 step=0.1 group='Water'");
  TwAddVarRW(bar, "foam_depth", TW_TYPE_FLOAT, &water.foam_depth, "min=0.0 max=50.0 step=0.1 group='Water'");
  TwAddVarRW(bar, "max_depth", TW_TYPE_FLOAT, &water.max_depth, "min=0.0 max=5.0 step=0.01 group='Water'");
  TwAddVarRW(bar, "ambient intensity", TW_TYPE_FLOAT, &water.ads_intensities[0], "min=-10.0 max=10.0 step=0.01 group='Water'");
  TwAddVarRW(bar, "diffuse intensity", TW_TYPE_FLOAT, &water.ads_intensities[1], "min=-10.0 max=10.0 step=0.01 group='Water'");
  TwAddVarRW(bar, "specular intensity", TW_TYPE_FLOAT, &water.ads_intensities[2], "min=-10.0 max=10.0 step=0.01 group='Water'");
  TwAddVarRW(bar, "sun intensity", TW_TYPE_FLOAT, &water.ads_intensities[3], "min=-10.0 max=10.0 step=0.01 group='Water'");
  TwAddVarRW(bar, "foam intensity", TW_TYPE_FLOAT, &water.ads_intensities[4], "min=-10.0 max=10.0 step=0.01 group='Water'");
  TwAddVarRW(bar, "texture intensity", TW_TYPE_FLOAT, &water.ads_intensities[5], "min=-10.0 max=10.0 step=0.01 group='Water'");
  TwAddVarRW(bar, "ambient color", TW_TYPE_COLOR3F, &water.ambient_color, "group='Water'");

  TwWindowSize(win_w, win_h);
  return true;
}

void GUI::onResize(int w, int h) {
  win_w = w;
  win_h = h;
  TwWindowSize(w, h);
}

void GUI::onMouseMoved(double x, double y) {
  TwMouseMotion(x, y);
}

void GUI::onMouseClicked(int bt, int action) {
  TwMouseButtonID btn = (bt == 0) ? TW_MOUSE_LEFT : TW_MOUSE_RIGHT;
  TwMouseAction ma = (action == GLFW_PRESS) ? TW_MOUSE_PRESSED : TW_MOUSE_RELEASED;
  TwMouseButton(ma, btn);
}

void GUI::onKeyPressed(int key, int mod) {

  switch(key) {
    case GLFW_KEY_LEFT: key = TW_KEY_LEFT; break;
    case GLFW_KEY_RIGHT: key = TW_KEY_RIGHT; break;
    case GLFW_KEY_UP: key = TW_KEY_UP; break;
    case GLFW_KEY_DOWN: key = TW_KEY_DOWN; break;
    case GLFW_KEY_BACKSPACE: key = TW_KEY_BACKSPACE; break;
    case GLFW_KEY_DELETE: key = TW_KEY_DELETE; break;
    case GLFW_KEY_ENTER: key = TW_KEY_RETURN; break;
      
    default: break;
  }

  int tw_mod = TW_KMOD_NONE;

  if(mod & GLFW_MOD_SHIFT) {
    tw_mod |= TW_KMOD_SHIFT;
  }

  if(mod & GLFW_MOD_CONTROL) {
    tw_mod |= TW_KMOD_CTRL;
  }

  if(mod & GLFW_MOD_ALT) {
    tw_mod |= TW_KMOD_ALT;
  }

  TwKeyPressed(key, TW_KMOD_NONE);
}

void GUI::draw() {
  TwDraw();
}
