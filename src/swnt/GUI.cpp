#include <assert.h>
#include <swnt/GUI.h>
#include <GLFW/glfw3.h>
#include <swnt/Swnt.h>
#include <swnt/Types.h>
#include <swnt/Spirals.h>

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
void TW_CALL set_time_of_year(const void* value, void* user) {
  GUI* g = static_cast<GUI*>(user);
  g->time_of_year = *(float*)value;
  if(g->swnt.override_with_gui) {
    g->swnt.setTimeOfYear(g->time_of_year);
  }
}

void TW_CALL get_time_of_year(void* value, void* user) {
  GUI* g = static_cast<GUI*>(user);
  if(g->swnt.override_with_gui) {
    *(float*)value = g->time_of_year;
  }
  else {
    *(float*)value = g->swnt.time_of_year;
  }
}

// -------------------------------------------------------------------------------

GUI::GUI(Swnt& swnt)
  :swnt(swnt)
  ,win_w(0)
  ,win_h(0)
  ,scroll_y(0)
  ,bar(NULL)
  ,time_of_day(0)
{
}

GUI::~GUI() {
  //TwTerminate();
}

#if USE_GUI
bool GUI::setup(int w, int h) {
  assert(w);
  assert(h);
  win_w = w;
  win_h = h;

  TwInit(TW_OPENGL_CORE, NULL);
  bar = TwNewBar("SWNT");
  TwDefine("SWNT size='300 750'");

  // water
  /*
  TwAddVarRW(bar, "Sun Position X", TW_TYPE_FLOAT, &swnt.water.sun_pos[0], "min=-1000.0 max=1000.0 step=5.0 group='Water'");
  TwAddVarRW(bar, "Sun Position Y", TW_TYPE_FLOAT, &swnt.water.sun_pos[1], "min=-1000.0 max=1000.0 step=5.0 group='Water'");
  TwAddVarRW(bar, "Sun Position Z", TW_TYPE_FLOAT, &swnt.water.sun_pos[2], "min=-1000.0 max=1000.0 step=5.0 group='Water'");
  */
  TwAddVarRW(bar, "Sun Color Red", TW_TYPE_FLOAT, &swnt.water.sun_color[0], "min=-5.0 max=5.0 step=0.1 group='Water'");
  TwAddVarRW(bar, "Sun Color Green", TW_TYPE_FLOAT, &swnt.water.sun_color[1], "min=-5.0 max=5.0 step=0.1 group='Water'");
  TwAddVarRW(bar, "Sun Color Blue", TW_TYPE_FLOAT, &swnt.water.sun_color[2], "min=-5.0 max=5.0 step=0.1 group='Water'");
  TwAddVarRW(bar, "Sun Shininess", TW_TYPE_FLOAT, &swnt.water.sun_shininess, "min=0.0 max=50.0 step=0.1 group='Water'");
  TwAddVarRW(bar, "Sun Intensity", TW_TYPE_FLOAT, &swnt.water.sun_intensity, "min=-10.0 max=10.0 step=0.01 group='Water'");
  TwAddVarRW(bar, "Maximum Foam Depth", TW_TYPE_FLOAT, &swnt.water.max_foam_depth, "min=0.0 max=50.0 step=0.1 group='Water'");
  // TwAddVarRW(bar, "Maximum Water Depth", TW_TYPE_FLOAT, &swnt.water.max_depth, "min=0.0 max=50.0 step=0.01 group='Water'");
  //  TwAddVarRW(bar, "Diffuse Intensity", TW_TYPE_FLOAT, &swnt.water.ads_intensities[1], "min=-10.0 max=10.0 step=0.01 group='Water'");
  //  TwAddVarRW(bar, "Specular Intensity", TW_TYPE_FLOAT, &swnt.water.ads_intensities[2], "min=-10.0 max=10.0 step=0.01 group='Water'");
  TwAddVarRW(bar, "Final Intensity", TW_TYPE_FLOAT, &swnt.water.final_intensity, "min=0.0 max=2.0 step=0.01 group='Water'");

  TwAddVarRW(bar, "Foam Intensity", TW_TYPE_FLOAT, &swnt.water.foam_intensity, "min=-10.0 max=10.0 step=0.01 group='Water'");
  TwAddVarRW(bar, "Diffuse Texture Intensity", TW_TYPE_FLOAT, &swnt.water.diffuse_intensity, "min=-10.0 max=10.0 step=0.01 group='Water'");
  TwAddVarRW(bar, "Ambient Intensity", TW_TYPE_FLOAT, &swnt.water.ambient_intensity, "min=-10.0 max=10.0 step=0.01 group='Water'");
  TwAddVarRW(bar, "Ambient Color", TW_TYPE_COLOR3F, &swnt.water.ambient_color, "group='Water'");
  TwAddVarRW(bar, "Vortex Amount", TW_TYPE_FLOAT, &swnt.water.vortex_intensity, "group='Water' min=0.0 max=5.0 step=0.01");
  TwDefine("SWNT/Water opened=false");

  // splashes
#if USE_EFFECTS  
#if HF_FIXED
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
  TwDefine("SWNT/Splashes opened=false");
#endif
#endif

  // rendering
  TwAddVarRW(bar, "Draw Flow Field", TW_TYPE_BOOLCPP, &swnt.draw_flow, "group='Rendering'");
  TwAddVarRW(bar, "Draw Water", TW_TYPE_BOOLCPP, &swnt.draw_water, "group='Rendering'");
  //TwAddVarRW(bar, "Draw Debug Eddy", TW_TYPE_BOOLCPP, &swnt.draw_vortex, "group='Rendering'");
#if USE_TRIANGULATION
  TwAddVarRW(bar, "Draw Tracking (Blobs, Points, Contours) ", TW_TYPE_BOOLCPP, &swnt.draw_tracking, "group='Rendering'");
  TwAddVarRW(bar, "Draw Triangulated Blobs", TW_TYPE_BOOLCPP, &swnt.tracking.draw_triangulated_blobs, "group='Rendering'");
  TwAddVarRW(bar, "Draw Points of Tracked Blobs", TW_TYPE_BOOLCPP, &swnt.tracking.draw_tracking_points, "group='Rendering'");
  TwAddVarRW(bar, "Draw Blob Contours", TW_TYPE_BOOLCPP, &swnt.tracking.draw_contours, "group='Rendering'");
#endif
  TwDefine("SWNT/Rendering opened=false");

  // general
  TwAddVarRW(bar, "Override Values With GUI", TW_TYPE_BOOLCPP, &swnt.override_with_gui, "group='Colors And Time'");
  TwAddVarCB(bar, "Time Of Day", TW_TYPE_FLOAT, set_time_of_day, get_time_of_day, this, "group='Colors And Time' min=0.25 max=0.75 step=0.001");
  TwAddVarCB(bar, "Time Of Year", TW_TYPE_FLOAT, set_time_of_year, get_time_of_year, this, "group='Colors And Time' min=0.0 max=1.0 step=0.01");

  // colors
  if(swnt.settings.colors.size() == 4) {
    TwAddVarRW(bar, "Hand Winter", TW_TYPE_COLOR3F, swnt.settings.colors[0].hand.ptr(), "group='Colors And Time'");
    TwAddVarRW(bar, "Hand Spring", TW_TYPE_COLOR3F, swnt.settings.colors[1].hand.ptr(), "group='Colors And Time'");
    TwAddVarRW(bar, "Hand Summer", TW_TYPE_COLOR3F, swnt.settings.colors[2].hand.ptr(), "group='Colors And Time'");
    TwAddVarRW(bar, "Hand Autumn", TW_TYPE_COLOR3F, swnt.settings.colors[3].hand.ptr(), "group='Colors And Time'");

    TwAddVarRW(bar, "Spiral Winter", TW_TYPE_COLOR3F, swnt.settings.colors[0].spiral_from.ptr(), "group='Colors And Time'");
    TwAddVarRW(bar, "Spiral Spring", TW_TYPE_COLOR3F, swnt.settings.colors[1].spiral_from.ptr(), "group='Colors And Time'");
    TwAddVarRW(bar, "Spiral Summer", TW_TYPE_COLOR3F, swnt.settings.colors[2].spiral_from.ptr(), "group='Colors And Time'");
    TwAddVarRW(bar, "Spiral Autumn", TW_TYPE_COLOR3F, swnt.settings.colors[3].spiral_from.ptr(), "group='Colors And Time'");

    TwAddVarRW(bar, "Water Winter", TW_TYPE_COLOR3F, swnt.settings.colors[0].water.ptr(), "group='Colors And Time'");
    TwAddVarRW(bar, "Water Spring", TW_TYPE_COLOR3F, swnt.settings.colors[1].water.ptr(), "group='Colors And Time'");
    TwAddVarRW(bar, "Water Summer", TW_TYPE_COLOR3F, swnt.settings.colors[2].water.ptr(), "group='Colors And Time'");
    TwAddVarRW(bar, "Water Autumn", TW_TYPE_COLOR3F, swnt.settings.colors[3].water.ptr(), "group='Colors And Time'");
  }

  TwDefine("SWNT/'Colors And Time' opened=false");

  // kinect
  TwAddVarRW(bar, "Kinect Far", TW_TYPE_FLOAT, &swnt.settings.kinect_far, "group='Kinect' min=0.00 max=5.00 step=0.01");
  TwAddVarRW(bar, "Kinect Near", TW_TYPE_FLOAT, &swnt.settings.kinect_near, "group='Kinect' min=0.00 max=5.00 step=0.01");
  TwDefine("SWNT/Kinect opened=false");

#if USE_SPIRALS
  Spirals& spirals = swnt.spirals;
  TwAddVarRW(bar, "Min Particle Lifetime", TW_TYPE_FLOAT, &spirals.min_lifetime, "min=0.0 max=300.0 step=1.0 group='Spirals'");
  TwAddVarRW(bar, "Max Particle Lifetime", TW_TYPE_FLOAT, &spirals.max_lifetime, "min=0.0 max=300.0 step=1.0 group='Spirals'");
  TwAddVarRW(bar, "Min Particle Mass", TW_TYPE_FLOAT, &spirals.min_mass, "min=0.0 max=15.0 step=0.001 group='Spirals'");
  TwAddVarRW(bar, "Max Particle Mass", TW_TYPE_FLOAT, &spirals.max_mass, "min=0.0 max=15.0 step=0.001 group='Spirals'");
  TwAddVarRW(bar, "Force Towards Center", TW_TYPE_FLOAT, &spirals.center_force, "min=0.0 max=400.0 step=0.1 group='Spirals'");
  TwAddVarRW(bar, "Velocity Field Force", TW_TYPE_FLOAT, &spirals.field_force, "min=0.0 max=5.0 step=0.01 group='Spirals'");
  TwAddVarRW(bar, "Min Width", TW_TYPE_FLOAT, &spirals.min_strip_width, "min=0.0 max=50.0 step=0.1 group='Spirals'");
  TwAddVarRW(bar, "Max Width", TW_TYPE_FLOAT, &spirals.max_strip_width, "min=0.0 max=50.0 step=0.1 group='Spirals'");
  TwAddVarRW(bar, "Min Tail Size", TW_TYPE_UINT32, &spirals.min_tail_size, "min=2 max=100 step=1 group='Spirals'");
  TwAddVarRW(bar, "Max Tail Size", TW_TYPE_UINT32, &spirals.max_tail_size, "min=2 max=100 step=1 group='Spirals'");
  TwAddVarRW(bar, "Number of Particles To Spawn", TW_TYPE_UINT32, &spirals.spawn_per_tracked, "min=1 max=200 step=1 group='Spirals'");
  TwDefine("SWNT/Spirals opened=false");
#endif

#if USE_TRIANGULATION
  TwAddVarRW(bar, "Hand Scale", TW_TYPE_FLOAT, &swnt.tracking.blob_scale, "min=1.0 max=3.0 step=0.01");
  TwAddVarRW(bar, "Hand Offset", TW_TYPE_FLOAT, &swnt.tracking.blob_offset, "min=-300.0 max=300.0 step=0.5");
#endif

  TwWindowSize(win_w, win_h);
  return true;
}
#endif // USE_GUI

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

void GUI::onMouseWheel(double x, double y) {
  if(y > 0.0) {
    scroll_y += 1;
  }
  else {
    scroll_y -= 1;
  }
  TwMouseWheel(scroll_y);
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
