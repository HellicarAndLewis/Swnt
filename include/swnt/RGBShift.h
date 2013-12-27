#ifndef RGBSHIFT_H
#define RGBSHIFT_H

#define ROXLU_USE_OPENGL
#define ROXLU_USE_MATH
#include <tinylib.h>

static const char* RGBSHIFT_VS = ""
  "#version 150\n"

  "const vec2 verts[4] = vec2[] ("
  "  vec2(-1.0, 1.0), " 
  "  vec2(-1.0, -1.0), "
  "  vec2(1.0, 1.0), "
  "  vec2(1.0, -1.0)"
  ");"

  "const vec2 tex[4] = vec2[] ("
  "  vec2(0.0, 1.0), " 
  "  vec2(0.0, 0.0), "
  "  vec2(1.0, 1.0), "
  "  vec2(1.0, 0.0)"
  ");"

  "out vec2 v_tex; " 
  
  "void main() {"
  "  gl_Position = vec4(verts[gl_VertexID], 0.0, 1.0); "
  "  v_tex = tex[gl_VertexID];"
  "}"

  "";

static const char* RGBSHIFT_FS = ""
  "#version 150\n"
  "uniform sampler2D u_tex;"
  "out vec4 fragcolor;"
  "in vec2 v_tex;"
  "void main() {"
  "  vec4 texcol = texture(u_tex, v_tex);"
  "  fragcolor = vec4(1.0, 0.0, 0.0, 1.0);"
  "  fragcolor.rgb += texcol.rgb;"
  "  fragcolor.r += 0.2;"

  "  float s = 0.003;"
  "  texcol.r = texture(u_tex, vec2(v_tex.s + s, v_tex.t)).r;"
  "  texcol.g = texture(u_tex, vec2(v_tex.s , v_tex.t)).g;"
  "  texcol.b = texture(u_tex, vec2(v_tex.s - s, v_tex.t)).b;"
  "  fragcolor.rgb = mix(texture(u_tex, v_tex).rgb, texcol.rgb, 0.6);"
  "}"
  "";

class RGBShift {
 public:
  RGBShift();
  ~RGBShift();
  bool setup();
  void apply(); /* captures the current read framebuffer into the scene texture and applies a rgb shift */
  void draw();
 private:
  bool createFBO();
  bool createShader();
 private:
  GLuint fbo;
  GLuint depth;
  GLuint tex_scene;
  GLuint vao;
  GLuint prog;
  GLuint frag;
  GLuint vert;
};

#endif
