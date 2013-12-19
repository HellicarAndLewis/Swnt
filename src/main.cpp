/*

  BASIC GLFW + GLXW WINDOW AND OPENGL SETUP 
  ------------------------------------------
  See https://gist.github.com/roxlu/6698180 for the latest version of the example.

*/

#include <swnt/Settings.h>
#include <swnt/Swnt.h>
#include <tinylib.h>
#include <iostream>

#if defined(__linux)
#  include <GLXW/glxw.h>
#endif

#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>

void key_callback(GLFWwindow* win, int key, int scancode, int action, int mods);
void error_callback(int err, const char* desc);
void resize_callback(GLFWwindow* window, int width, int height);

Settings settings;
Swnt swnt(settings);

int main() {

  glfwSetErrorCallback(error_callback);

  if(!glfwInit()) {
    printf("error: cannot setup glfw.\n");
    return false;
  }

  glfwWindowHint(GLFW_SAMPLES, 4);
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
  glfwMakeContextCurrent(win);
  glfwSwapInterval(1);

#if defined(__linux)
  if(glxwInit() != 0) {
    printf("error: cannot initialize glxw.\n");
    ::exit(EXIT_FAILURE);
  }
#endif

  int w,h = 0;
  glfwGetWindowSize(win, &w, &h);
  if(w != settings.win_w || h != settings.win_h) {
    printf("The created window does not have the asked dimensions!\nTry changing the width and height in the settings");
    ::exit(EXIT_FAILURE);
  }

  // ----------------------------------------------------------------
  // THIS IS WHERE YOU START CALLING OPENGL FUNCTIONS, NOT EARLIER!!
  // ----------------------------------------------------------------

  if(!swnt.setup()) {
    printf("Error: Swnt::setup() failed, see log for more info.\n");
    ::exit(EXIT_FAILURE);
  }

  while(!glfwWindowShouldClose(win)) {
    glClearColor(0.094f, 0.074f, 0.184f, 1.0f);
    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    swnt.update();
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
  
  if(action != GLFW_PRESS) {
    return;
  }

  switch(key) {
    case GLFW_KEY_1: {
      swnt.draw_water = !swnt.draw_water;
      break;
    }
    case GLFW_KEY_2: {
      swnt.draw_flow = !swnt.draw_flow;
      break;
    }
    case GLFW_KEY_3: {
      swnt.draw_spirals = !swnt.draw_spirals;
      break;
    }
    case GLFW_KEY_4: {
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
  };
  
}

void resize_callback(GLFWwindow* window, int width, int height) {

}
