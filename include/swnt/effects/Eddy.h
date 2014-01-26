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

/*
  Eddy
  ----
  The Eddy class is used to render extra flow into the water simulation. The
  Water class has a feature that allows us to draw flow colors that are used
  together with the flow texture to change the direction of the water.

 */
#ifndef SWNT_EDDY_EFFECT_H
#define SWNT_EDDY_EFFECT_H

#define USE_EDDY_DEBUG 0

#include <swnt/Types.h>
#include <swnt/effects/BaseEffect.h>
#include <vector>

static const char* EDDY_VS = ""
  "#version 150\n"
  "uniform mat4 u_pm;"
  "uniform mat4 u_mm;"
  "in vec4 a_pos;"
  "in vec3 a_tex;"
  "in float a_alpha;"
  "out vec3 v_tex;"
  "out float v_alpha;"
  "void main() {"
  "  gl_Position = u_pm * u_mm * a_pos; "
  "  v_tex = a_tex;"
  "  v_alpha = a_alpha;"
  "}"
  "";

static const char* EDDY_FS = ""
  "#version 150\n"
  "uniform sampler2D u_tex;"
  "uniform float u_time;"
  "out vec4 fragcolor;"
  "in vec3 v_tex;"
  "in float v_alpha;"
  "void main() {"
  "  vec2 tc = v_tex.xy / v_tex.z;"
  "  vec4 diffuse_color = texture(u_tex, vec2(tc.s - u_time, tc.t));"
  "  fragcolor.rgb = diffuse_color.rgb;"
  "  fragcolor.a = v_alpha;"

#if USE_EDDY_DEBUG
  " fragcolor = vec4(1.0, 0.0, 0.0, 1.0);"
#endif
  "}"
  "";

struct EddyShape {

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


struct EddyVertex {
  EddyVertex() {}
  EddyVertex(vec3 pos, vec3 tex, float alpha):pos(pos),tex(tex),alpha(alpha){}
  float* ptr() { return pos.ptr(); } 
  vec3 pos;
  vec3 tex;
  float alpha;
};

class Eddy : public BaseEffect {

 public:
  Eddy(Effects& effects);
  ~Eddy();
  bool setup();
  void update();
  void drawExtraFlow();

 private:
  void createRing(float x, float y, float radius, float width, float fromAlpha, float toAlpha);

 private:
  GLuint vert;
  GLuint frag;
  GLuint prog;
  GLuint vbo;
  GLuint vao;
  GLuint eddy_tex;
  GLint u_time;
  std::vector<EddyVertex> vertices;
  std::vector<GLsizei> counts;
  std::vector<GLint> offsets;
  std::vector<EddyShape> shapes;
  size_t bytes_allocated;
  bool needs_update;
};

#endif
