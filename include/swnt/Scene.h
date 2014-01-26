/*
---------------------------------------------------------------------------------
 
                                               oooo
                                               `888
                oooo d8b  .ooooo.  oooo    ooo  888  oooo  oooo
                `888""8P d88' `88b  `88b..8P'   888  `888  `888
                 888     888   888    Y888'     888   888   888
                 888     888   888  .o8"'88b    888   888   888
                d888b    `Y8bod8P' o88'   888o o888o  `V88V"V8P'
 
                                                  www.roxlu.com
                                             www.apollomedia.nl
                                          www.twitter.com/roxlu
 
---------------------------------------------------------------------------------
*/

#ifndef SWNT_SCENE_H
#define SWNT_SCENE_H

#define ROXLU_USE_ALL
#include <tinylib.h>

static const char* SCENE_VS = ""
  "#version 150\n"
  "uniform mat4 u_pm;"
  "uniform mat4 u_mm;"
  "uniform mat4 u_vm;"
  "in vec4 a_pos;"
  "in vec3 a_norm;"
  "out vec3 v_norm;"
  "out vec3 v_pos;"
  "void main() {"
  "  gl_Position = u_pm * u_vm * u_mm * a_pos;"
  "  v_norm = a_norm;"
  "  v_pos = vec3(u_vm * u_mm * a_pos);"
  "}"
  "";

static const char* SCENE_FS = ""
  "#version 150\n"
  "out vec4 fragcolor;"
  "in vec3 v_norm;"
  "in vec3 v_pos;"
  "uniform mat4 u_vm;"
  "uniform mat4 u_mm;"
  "void main() { "
  "  fragcolor = vec4(1.0, 0.0, 0.0, 1.0);"

  "  vec3 Kd = vec3(0.1, 0.4, 0.6);"
  "  vec3 l = vec3(0.0, 10.0, 0.0);"
  "  vec3 eye_l = (mat3(u_vm) * l);"

  "  vec3 eye_p = v_pos;"
  "  vec3 eye_s = normalize(eye_l - eye_p);"
  "  vec3 eye_n = normalize(mat3(u_vm) * v_norm);"
  "  float sdn = max(dot(eye_s, eye_n), 0.0);"
  "  fragcolor.rgb = Kd * sdn;"
  "}"
  "";

class Scene {
 public:
  Scene();
  bool setup(int w, int h);
  void draw(float* pm, float* vm);

 public:
  int win_w;
  int win_h;
  GLuint vao;
  GLuint vbo;
  GLuint vert;
  GLuint frag;
  GLuint prog;
  mat4 mm;
  std::vector<VertexPN> vertices;
};

#endif
