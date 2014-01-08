#ifndef SWNT_MIST_EFFECT_H
#define SWNT_MIST_EFFECT_H

#define USE_MIST_DEBUG 0

#include <swnt/Types.h>
#include <swnt/effects/BaseEffect.h>
#include <vector>

static const char* MIST_VS = ""
  "#version 150\n"
  "uniform mat4 u_pm;"
  "uniform mat4 u_mm;"
  "in vec4 a_pos;"
  "in vec3 a_tex;"
  "out vec3 v_tex;"
  "void main() {"
  "  gl_Position = u_pm * u_mm * a_pos; "
// "  gl_Position = vec4(2.0 * a_tex - vec2(1.0), 0.0, 1.0);" // this will draw the texcoords; make sure to set polymode to line
  "  v_tex = a_tex;"
  "}"
  "";

static const char* MIST_FS = ""
  "#version 150\n"
  "uniform sampler2D u_tex;"
  "uniform float u_time;"
  "uniform float u_perc;"
  "out vec4 fragcolor;"
  "in vec3 v_tex;"
  "void main() {"
  "  vec2 tc = v_tex.xy / v_tex.z;"
  "  vec4 diffuse_color = texture(u_tex, vec2(tc.s - u_time, tc.t));"
  "  fragcolor.rgba = vec4(0.0, 0.2, 1.0, 1.0);"
  "  fragcolor.rgb = diffuse_color.rgb;"
  ///  "  fragcolor.a = sin(3.1415 * 0.5 * u_perc);"
  "  fragcolor.a = tc.t;"
  "  fragcolor.a = pow(length((gl_FragCoord.xy - vec2(512.0, 384.0)) / vec2(512.0, 384.0)), 2.0) ;"

#if USE_MIST_DEBUG
  " fragcolor = vec4(1.0, 0.0, 0.0, 1.0);"
#endif
  "}"
  "";

struct MistShape {

  void reset();

  size_t offset;
  size_t count;
  mat4 mm;
  float scale_speed;
  float rotate_speed;
  float die_time;
  float lifetime;
  float x;
  float y;
};

class Mist : public BaseEffect {

 public:
  Mist(Effects& effects);
  ~Mist();
  bool setup();
  void update();
  void draw();

 private:
  void createRing(float x, float y, float radius, float width);

 private:
  GLuint vert;
  GLuint frag;
  GLuint prog;
  GLuint vbo;
  GLuint vao;
  GLuint mist_tex;
  GLint u_time;
  GLint u_perc;
  std::vector<VertexPT3> vertices;
  std::vector<GLsizei> counts;
  std::vector<GLint> offsets;
  std::vector<MistShape> shapes;
  size_t bytes_allocated;
  bool needs_update;
};

#endif
