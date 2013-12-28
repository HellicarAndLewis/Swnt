/*

  Some experimental effects to enhance the interactivity

  - nice haze effect: http://forums.epicgames.com/threads/708297-UDK-Video-Tutorials-Effects
 */ 

#ifndef SWNT_EFFECTS_H
#define SWNT_EFFECTS_H

#define ROXLU_USE_OPENGL
#define ROXLU_USE_MATH
#include <tinylib.h>
#include <swnt/Particles.h>
#include <swnt/Blur.h>

// ------------------------------------------

static const char* EFFECT_PT_VS = ""
  "#version 150\n"
  "uniform mat4 u_pm;"
  "uniform mat4 u_vm;"
  "uniform mat4 u_mm;"

  "in vec4 a_pos;"
  "in vec2 a_tex;"
  "out vec2 v_tex;"
  
  "void main() {"
  "  gl_Position = u_pm * u_vm * u_mm * a_pos;"
  "  v_tex = a_tex;"
  "}"
  "";

// used to render the floor effect
static const char* EFFECT_FLOOR_FS = ""
  "#version 150\n"
  "uniform sampler2D u_tex;"
  "out vec4 fragcolor;"
  "in vec2 v_tex;"

  "void main() {"
  "  vec4 tc = texture(u_tex, v_tex);"
  "  fragcolor = vec4(1.0, 0.0, 0.0, 1.0);"
  "  fragcolor = tc;"
  //  "  fragcolor.a = 1.0;"
  "}"
  "";


static const char* DISP_FRAG_FS = ""
  "#version 150\n"
  "uniform sampler2D u_displacement_tex;"
  "uniform sampler2D u_scene_tex;"
  "in vec2 v_tex;"
  "out vec4 fragcolor;"
  "void main() {"
  "  vec4 disp_col = texture(u_displacement_tex, v_tex);"
  "  vec4 scene_col = texture(u_scene_tex, vec2(v_tex.s + disp_col.r * 0.1, v_tex.t + disp_col.g * 0.1));"
  "  fragcolor = scene_col;"
  "  fragcolor.a = 1.0;"
  //  "  fragcolor.r = disp_col.r;"
  "}"
  "";

// ------------------------------------------
/*
struct VertexPT {
  VertexPT(){}
  VertexPT(vec3 pos, vec2 tex):pos(pos),tex(tex) {};
  float* ptr() { return pos.ptr(); } 
  vec3 pos;
  vec2 tex;
};

*/

// ------------------------------------------

struct FloorItem {
  mat4 mm;
};

// ------------------------------------------

class Graphics;
class Settings;
class Spirals;

class Effects { 

 public:
  Effects(Settings& settings, Graphics& graphics, Spirals& spiral);
  ~Effects();
  bool setup();
  void drawFloor();
  void displace(GLuint dispTex, GLuint sceneTex);

 private:
  bool setupShaders();
  bool setupFloor();
  bool setupDisplacement();
   
 public:
  Settings& settings;
  Graphics& graphics;
  Spirals& spirals;

  /* generic GL */
  GLuint pt_vert; /* position + texcoord vertex shader */
  mat4 pm;
  mat4 vm;

  /* floor effect */
  GLuint floor_vbo;
  GLuint floor_vao;
  GLuint floor_frag;
  GLuint floor_prog;
  GLuint floor_tex;
  std::vector<FloorItem*> floor_items;


  /* displacement effect */
  GLuint disp_fbo;
  GLuint disp_frag;
  GLuint disp_vao;
  GLuint disp_prog;

  /* test blur */
  Blur blur;
};

#endif
