/*

  Threshold shader

 */

#ifndef ROXLU_THRESHOLD_SHADER_H
#define ROXLU_THRESHOLD_SHADER_H

#define ROXLU_USE_OPENGL
#include <tinylib.h>

static const char* THRESHOLD_VS = ""
  "#version 150\n"
  "out vec2 v_tex;"
  ""
  "const vec2 verts[4] = vec2[]("
  "  vec2(-1.0, 1.0),  "
  "  vec2(-1.0, -1.0), "
  "  vec2(1.0, 1.0),   "
  "  vec2(1.0, -1.0)   "
  ");"
  ""
  "const vec2 tex[4] = vec2[]("
  "   vec2(0.0, 0.0), "
  "   vec2(0.0, 1.0), "
  "   vec2(1.0, 0.0), "
  "   vec2(1.0, 1.0)  "
  ");"
  ""
  "void main() {"
  "  gl_Position = vec4(verts[gl_VertexID], 0.0, 1.0);"
  "  v_tex = tex[gl_VertexID];"
  "}"
  "";

static const char* THRESHOLD_FS = ""
  "#version 150\n"
  "uniform sampler2D u_tex;"
  "in vec2 v_tex;"
  "out vec4 fragcolor;"
  "void main() {"
  "  vec4 tc = texture(u_tex, v_tex);"
  "  fragcolor = vec4(0.3, 0.1, 0.0, 1.0);"
  "  fragcolor.r = step(0.5, tc.r);"
  "}"
  "";

class Threshold {

 public:
  Threshold();
  bool setup(int winW, int winH);
  void threshold();
  void print();

 public:
  int win_w;
  int win_h;
  GLuint prog;
  GLuint vert;
  GLuint frag;
  GLuint input_tex; 
  GLuint output_tex;
  GLuint fbo;
  GLuint vao;
};
#endif
