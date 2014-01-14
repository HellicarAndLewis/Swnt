#include <assert.h>
#include <GLFW/glfw3.h>
#include <swnt/GUI.h>
#include <swnt/Swnt.h>
#include <swnt/Types.h>


// -------------------------------------------------------------------------------

void TW_CALL set_time_of_day(const void* value, void* user) {
  GUI* g = static_cast<GUI*>(user);
  g->time_of_day = *(float*)value;

#if USE_TIDES
  if(g->swnt.override_with_gui) {
    g->swnt.setTimeOfDay(g->time_of_day);
  }
#endif
}

void TW_CALL get_time_of_day(void* value, void* user) {
  GUI* g = static_cast<GUI*>(user);
  if(g->swnt.override_with_gui) {
    *(float*)value = g->time_of_day;
  }
  else {
    *(float*)value = g->swnt.time_of_day;
  }
}

// -------------------------------------------------------------------------------

GUI::GUI(Swnt& swnt)
  :swnt(swnt)
  ,win_w(0)
  ,win_h(0)
  ,bar(NULL)
  ,time_of_day(0)
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
  TwDefine("SWNT size='300 750'");

  // water
  TwAddVarRW(bar, "Sun Position X", TW_TYPE_FLOAT, &swnt.water.sun_pos[0], "min=-1000.0 max=1000.0 step=5.0 group='Water'");
  TwAddVarRW(bar, "Sun Position Y", TW_TYPE_FLOAT, &swnt.water.sun_pos[1], "min=-1000.0 max=1000.0 step=5.0 group='Water'");
  TwAddVarRW(bar, "Sun Position Z", TW_TYPE_FLOAT, &swnt.water.sun_pos[2], "min=-1000.0 max=1000.0 step=5.0 group='Water'");
  TwAddVarRW(bar, "Sun Color Red", TW_TYPE_FLOAT, &swnt.water.sun_color[0], "min=-5.0 max=5.0 step=0.1 group='Water'");
  TwAddVarRW(bar, "Sun Color Green", TW_TYPE_FLOAT, &swnt.water.sun_color[1], "min=-5.0 max=5.0 step=0.1 group='Water'");
  TwAddVarRW(bar, "Sun Color Blue", TW_TYPE_FLOAT, &swnt.water.sun_color[2], "min=-5.0 max=5.0 step=0.1 group='Water'");
  TwAddVarRW(bar, "Sun Shininess", TW_TYPE_FLOAT, &swnt.water.sun_shininess, "min=0.0 max=50.0 step=0.1 group='Water'");
  TwAddVarRW(bar, "Maximum Foam Depth", TW_TYPE_FLOAT, &swnt.water.foam_depth, "min=0.0 max=50.0 step=0.1 group='Water'");
  TwAddVarRW(bar, "Maximum Water Depth", TW_TYPE_FLOAT, &swnt.water.max_depth, "min=0.0 max=5.0 step=0.01 group='Water'");
  //  TwAddVarRW(bar, "Diffuse Intensity", TW_TYPE_FLOAT, &swnt.water.ads_intensities[1], "min=-10.0 max=10.0 step=0.01 group='Water'");
  //  TwAddVarRW(bar, "Specular Intensity", TW_TYPE_FLOAT, &swnt.water.ads_intensities[2], "min=-10.0 max=10.0 step=0.01 group='Water'");
  TwAddVarRW(bar, "Final Intensity", TW_TYPE_FLOAT, &swnt.water.ads_intensities[6], "min=0.0 max=2.0 step=0.01 group='Water'");
  TwAddVarRW(bar, "Sun Intensity", TW_TYPE_FLOAT, &swnt.water.ads_intensities[3], "min=-10.0 max=10.0 step=0.01 group='Water'");
  TwAddVarRW(bar, "Foam Intensity", TW_TYPE_FLOAT, &swnt.water.ads_intensities[4], "min=-10.0 max=10.0 step=0.01 group='Water'");
  TwAddVarRW(bar, "Diffuse Texture Intensity", TW_TYPE_FLOAT, &swnt.water.ads_intensities[5], "min=-10.0 max=10.0 step=0.01 group='Water'");
  TwAddVarRW(bar, "Ambient Intensity", TW_TYPE_FLOAT, &swnt.water.ads_intensities[0], "min=-10.0 max=10.0 step=0.01 group='Water'");
  TwAddVarRW(bar, "Ambient Color", TW_TYPE_COLOR3F, &swnt.water.ambient_color, "group='Water'");

  // splashes
  Splashes& sp = swnt.effects.splashes;
  TwAddVarRW(bar, "Minimum Lifetime", TW_TYPE_FLOAT, &sp.lifetime_min, "min=0.0 max=300.0 step=1.0 group='Splashes'");
  TwAddVarRW(bar, "Maximum Lifetime", TW_TYPE_FLOAT, &sp.lifetime_max, "min=0.0 max=300.0 step=1.0 group='Splashes'");
  TwAddVarRW(bar, "Minimum Width", TW_TYPE_FLOAT, &sp.size_x_min, "min=0.0 max=300.0 step=1.0 group='Splashes'");
  TwAddVarRW(bar, "Maximum Width", TW_TYPE_FLOAT, &sp.size_x_max, "min=0.0 max=300.0 step=1.0 group='Splashes'");
  TwAddVarRW(bar, "Minimum Height", TW_TYPE_FLOAT, &sp.size_y_min, "min=0.0 max=300.0 step=1.0 group='Splashes'");
  TwAddVarRW(bar, "Maximum Height", TW_TYPE_FLOAT, &sp.size_y_max, "min=0.0 max=300.0 step=1.0 group='Splashes'");
  TwAddVarRW(bar, "Minimum Rotation Speed", TW_TYPE_FLOAT, &sp.rotate_speed_min, "min=-10.0 max=10.0 step=0.2 group='Splashes'");
  TwAddVarRW(bar, "Maximum Rotation Speed", TW_TYPE_FLOAT, &sp.rotate_speed_max, "min=-10.0 max=10.0 step=0.2 group='Splashes'");
  TwAddVarRW(bar, "Minimum Move Speed", TW_TYPE_FLOAT, &sp.move_speed_min, "min=0.0 max=150.0 step=0.2 group='Splashes'");
  TwAddVarRW(bar, "Maximum Move Speed", TW_TYPE_FLOAT, &sp.move_speed_max, "min=0.0 max=150.0 step=0.2 group='Splashes'");
  TwAddVarRW(bar, "Minimum Splash Size", TW_TYPE_FLOAT, &sp.size_min, "min=0.0 max=300.0 step=1.0 group='Splashes'");
  TwAddVarRW(bar, "Maximum Splash Size", TW_TYPE_FLOAT, &sp.size_max, "min=0.0 max=300.0 step=1.0 group='Splashes'");
  TwAddVarRW(bar, "Animation Speed Of Texture", TW_TYPE_FLOAT, &sp.texture_anim_speed, "min=0.0 max=2.0 step=0.01 group='Splashes'");

  // rendering
  TwAddVarRW(bar, "Draw Flow Field", TW_TYPE_BOOLCPP, &swnt.draw_flow, "group='Rendering'");
  TwAddVarRW(bar, "Draw Water", TW_TYPE_BOOLCPP, &swnt.draw_water, "group='Rendering'");
  TwAddVarRW(bar, "Draw Debug Eddy", TW_TYPE_BOOLCPP, &swnt.draw_vortex, "group='Rendering'");
  TwAddVarRW(bar, "Draw Contours and Tangents", TW_TYPE_BOOLCPP, &swnt.draw_tracking, "group='Rendering'");
  TwAddVarRW(bar, "Override Values With GUI", TW_TYPE_BOOLCPP, &swnt.override_with_gui, "group='Rendering'");

  TwAddVarCB(bar, "Time Of Day", TW_TYPE_FLOAT, set_time_of_day, get_time_of_day, this, "min=0.25 max=0.75 step=0.001");


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
