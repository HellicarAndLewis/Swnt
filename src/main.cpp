/*

  BASIC GLFW + GLXW WINDOW AND OPENGL SETUP 
  ------------------------------------------
  See https://gist.github.com/roxlu/6698180 for the latest version of the example.

*/

#define ROXLU_USE_ALL
#define ROXLU_IMPLEMENTATION
#include <tinylib.h>

#include <swnt/Types.h>
#include <swnt/Settings.h>
#include <swnt/Swnt.h>

#include <iostream>

#if defined(__linux)
#  include <GLXW/glxw.h>
#endif

#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>

void cursor_callback(GLFWwindow* win, double x, double y);
void button_callback(GLFWwindow* win, int bt, int action, int mods);
void key_callback(GLFWwindow* win, int key, int scancode, int action, int mods);
void error_callback(int err, const char* desc);
void resize_callback(GLFWwindow* window, int width, int height);
void scroll_callback(GLFWwindow* window, double x, double y);

Settings settings;
Swnt swnt(settings);

bool draw_forces = false;
float force_x = 0.0f;
float force_y = 0.0f;

int main() {
  vec2 v(10,40);
  v = normalized(v);
  vec2 b(40,44);
  float d = dot(v,b);
  printf(">>>>>> %f\n", d);

  glfwSetErrorCallback(error_callback);

  if(!glfwInit()) {
    printf("error: cannot setup glfw.\n");
    return false;
  }

  ///  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  
  if(!settings.load(rx_get_exe_path() +"data/settings.xml")) {
    printf("Error: cannot find the settings.xml file in the datapath.\n");
    ::exit(EXIT_FAILURE);
  }

  GLFWwindow* win = NULL;
  //win = glfwCreateWindow(settings.win_w, settings.win_h, "Swnt", glfwGetPrimaryMonitor(), NULL);
  win = glfwCreateWindow(settings.win_w, settings.win_h, "Swnt", NULL, NULL);
  if(!win) {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glfwSetFramebufferSizeCallback(win, resize_callback);
  glfwSetKeyCallback(win, key_callback);
  glfwSetMouseButtonCallback(win, button_callback);
  glfwSetScrollCallback(win, scroll_callback);
  glfwSetCursorPosCallback(win, cursor_callback);
  glfwMakeContextCurrent(win);
  glfwSwapInterval(1);

#if defined(__linux) || defined(WIN32)
  if(glxwInit() != 0) {
    printf("error: cannot initialize glxw.\n");
    ::exit(EXIT_FAILURE);
  }
#endif

  int w,h = 0;
  glfwGetWindowSize(win, &w, &h);
  if(w != settings.win_w || h != settings.win_h) {
    printf("The created window does not have the asked dimensions!\nTry changing the width and height in the settings: %d x %d\n", w, h);
    ::exit(EXIT_FAILURE);
  }

  // ----------------------------------------------------------------
  // THIS IS WHERE YOU START CALLING OPENGL FUNCTIONS, NOT EARLIER!!
  // ----------------------------------------------------------------

  if(!swnt.setup()) {
    printf("Error: Swnt::setup() failed, see log for more info.\n");
    ::exit(EXIT_FAILURE);
  }

  uint64_t prev_time = rx_hrtime();
  float inv_ns = 1.0f / 1000000000.0f;
  
  float dt = 1.0f/60.0f;
  int64_t simulation_step = dt * 1000000ull * 1000ull ;
  int64_t simulation_time = 0;
  int64_t started = rx_hrtime();
  int64_t now = started;

  while(!glfwWindowShouldClose(win)) {
    glClearColor(0.094f, 0.074f, 0.184f, 1.0f);
    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // perform integration step for height field simulation.
    now = rx_hrtime() - started;
    while(simulation_time < now) {
      swnt.integrate(dt);
      simulation_time += simulation_step;
    }


#if USE_WATER
    if(draw_forces) { 
      swnt.height_field.beginDrawForces();
      swnt.height_field.drawForceTexture(swnt.water.force_tex, force_x, force_y, 0.4, 0.4);
      swnt.height_field.endDrawForces();
      draw_forces = false;
    }
#endif
    
    uint64_t now = rx_hrtime();
    uint64_t delta_ns = now - prev_time;
    float dt = delta_ns * inv_ns; 
    prev_time = now;
    swnt.update(dt);
    swnt.draw();

    glfwSwapBuffers(win);
    glfwPollEvents();
  }


  glfwTerminate();

  return EXIT_SUCCESS;
}


void error_callback(int err, const char* desc) {
  printf("glfw error: %s (%d)\n", desc, err);
}

void key_callback(GLFWwindow* win, int key, int scancode, int action, int mods) {

  if(action == GLFW_PRESS) {
#if USE_GUI
    swnt.gui.onKeyPressed(key, mods);
#endif
  }

  if(action != GLFW_PRESS) {
    return;
  }

  switch(key) {
#if USE_AUDIO
    case GLFW_KEY_I: {
      printf("audio.play(SOUND_WAVES_CRASHING)\n");
      swnt.audio.play(SOUND_WAVES_CRASHING);
      break;
    }
    case GLFW_KEY_O: {
      printf("audio.stop(SOUND_WAVES_CRASHING)\n");
      swnt.audio.stop(SOUND_WAVES_CRASHING);
      break;
    }
#endif
    case GLFW_KEY_1: {
      swnt.draw_water = !swnt.draw_water;
      break;
    }
    case GLFW_KEY_2: {
      swnt.draw_flow = !swnt.draw_flow;
      break;
    }
    case GLFW_KEY_3: {
      swnt.draw_threshold = !swnt.draw_threshold;
      break;
    }
    case GLFW_KEY_A: {
      swnt.state = STATE_RENDER_ALIGN;
      break;
    }
    case GLFW_KEY_B: {
      swnt.state = STATE_RENDER_SCENE;
      break;
    }
    case GLFW_KEY_G: { 
#if USE_GUI
      swnt.draw_gui = !swnt.draw_gui;
#endif      
      break;
    }
    case GLFW_KEY_SPACE: {
      break;
    }
    case GLFW_KEY_LEFT: {
      break;
    }
    case GLFW_KEY_RIGHT: {
      break;
    }
    case GLFW_KEY_ESCAPE: {
      glfwSetWindowShouldClose(win, GL_TRUE);
      break;
    }
    case GLFW_KEY_M: {
      static bool show = false;
      glfwSetInputMode(win, GLFW_CURSOR, (show) ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
      show = !show;
      break;
    }
    case GLFW_KEY_T: {
      int num = 5;
      vec3 dir(rx_random(-1.0,1.0), rx_random(-1.0f, 1.0f), 0.0);
      float angle = rx_random(0, TWO_PI);
      float total_angle = 40 * DEG_TO_RAD;
      float part_angle = total_angle / num;
      float radius = 150;
      vec3 pos(512.0f, 384.0, 0.0);
      for(int i = 0; i < num; ++i) {

        float c = cos(i*part_angle);
        float s = sin(i * part_angle);
        dir.set(c, s, 0.0);
        vec3 p(pos.x + c * radius, pos.y + s * radius, 0.0);
      //      dir.set(1.0, 0.0, 0.0);
#if USE_EFFECTS
        //        swnt.effects.splashes.createParticle(p, dir);
#endif
      }
      break;
    }
    case GLFW_KEY_R: {
      printf("Recompiled.\n");
#if USE_EFFECTS
      //      swnt.effects.splashes.prog.recompile();
#endif
      break;
    }
  };
  
}

void resize_callback(GLFWwindow* window, int width, int height) {
#if USE_GUI
  swnt.gui.onResize(width, height);
#endif
}

void button_callback(GLFWwindow* win, int bt, int action, int mods) {
  if(action == GLFW_PRESS) {
    draw_forces = true;
  }

#if USE_GUI
  swnt.gui.onMouseClicked(bt, action);
#endif
}

void cursor_callback(GLFWwindow* win, double x, double y) {

#if USE_GUI
  swnt.gui.onMouseMoved(x, y);
#endif

  force_x = x/1280.0;
  force_y = (720-y)/720.0;
  // printf("force_x: %f, force_y: %f\n", force_x, force_y);
}

void scroll_callback(GLFWwindow* window, double x, double y) {
#if USE_GUI
  swnt.gui.onMouseWheel(x, y);
#endif  
}
