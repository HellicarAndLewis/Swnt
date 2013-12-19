#ifndef SWNT_WATER_H
#define SWNT_WATER_H

#define ROXLU_USE_OPENGL
#define ROXLU_USE_MATH
#include <tinylib.h>

#define USE_SIMPLE_SHADER 0

static const char* WATER_VS = ""
  "#version 150\n"
  "uniform sampler2D u_height_tex;"
  "uniform mat4 u_pm;"
  "uniform mat4 u_vm;"
  "uniform mat3 u_nm;"
  "in vec4 a_pos; "
  "const float size = 6.4;"
  "const float half_size = size * 0.5;"
  "const float tex_step_size = 1.0 / 64.0;"
  "const float step_size = size / 64.0;"
  "const float max_height = 3.0;"
  "out float v_height;"
#if !USE_SIMPLE_SHADER
  "out vec3 v_norm;"
  "out vec3 v_pos;"
#endif
  ""
  ""
  "void main() {"
  "  float s = (half_size + a_pos.x) / size;"
  "  float t = (half_size + a_pos.z) / size;"
  "  float height = texture(u_height_tex, vec2(s, t)).r * max_height;"
#if !USE_SIMPLE_SHADER 
  "  float height_bottom_right = texture(u_height_tex, vec2(s + tex_step_size, t)).r * max_height;"
  "  float height_top_right = texture(u_height_tex, vec2(s + tex_step_size, t + tex_step_size)).r * max_height;"
  "  vec3 a = vec3(a_pos.x, height, a_pos.z); " 
  "  vec3 b = vec3(a_pos.x + step_size, height_bottom_right, a_pos.z + step_size);"
  "  vec3 c = vec3(a_pos.x + step_size, height_top_right, a_pos.z - step_size);"
  "  vec3 ab = b-a;"
  "  vec3 ac = c-a;"
  //  "  v_norm =  u_nm * normalize(cross(ab, ac));"
  "  v_norm = transpose(inverse(mat3(u_vm)))  * normalize(cross(ab, ac));"
  "  v_pos = a;"
  "  gl_Position = u_pm * u_vm * vec4(a, 1.0);"
#else 
  "  gl_Position = u_pm * u_vm * vec4(a_pos.x, height, a_pos.z, 1.0);"
#endif

  "  v_height = (height / max_height);"
  "}"
  "";

static const char* WATER_FS = ""
  "#version 150\n"
  "out vec4 fragcolor;"
  "in float v_height;"
#if !USE_SIMPLE_SHADER
  "in vec3 v_norm;"
  "in vec3 v_pos;"
#endif
  "void main() {"
  "  fragcolor = vec4(1.0, 0.0, 0.0, 1.0);"
#if USE_SIMPLE_SHADER
  "  fragcolor.rgb = vec3(0.3, 0.3, 0.3);"
#else
  "  vec3 light_pos = vec3(0.0, 20.0, 0.0);"
  "  vec3 light_dir = normalize(v_pos - light_pos);"
  "  float ndl = max(dot(v_norm, light_dir), 0.0);"
  "  vec3 light_pos2 = vec3(100.0, 10.0, 0.0);"
  "  vec3 light_dir2 = normalize(v_pos - light_pos2);"
  "  float ndl2 = max(dot(v_norm, light_dir2), 0.0);"
  "  vec3 light_col2 = vec3(0.0, 0.1, 0.3) * ndl2;"
  "  vec3 diffuse = (light_col2 * 0.4) + vec3(0.0, 0.2, 0.8) + vec3(0.0, 0.1, 0.7) * ndl;"

  " vec3 r = normalize(-reflect(light_pos, v_norm));"
  " vec3 v = normalize(-v_pos);"
  " vec3 specular = pow(max(0.0, dot(r,v)), 4.0) * vec3(1.0,1.0,1.0);"

  //"  fragcolor.rgb = 0.5 + v_norm * 0.5 + diffuse;"
  "  fragcolor.rgb = diffuse + specular;"
  "  fragcolor.a = 0.8;"
#endif
  "}"
  "";

class Settings;
class Flow;

class Water {

 public:
  Water(Settings& settings, Flow& flow);
  bool setup();
  void update();
  void draw();

 public:
  Settings& settings;
  Flow& flow;

  /* GL state */
  GLuint vbo;                 /* the vertices */
  GLuint vao;                 /* vao used to render vertices */
  GLuint vert;                /* vertex shader */
  GLuint frag;                /* fragment shader */
  GLuint prog;                /* program */
  GLuint height_tex;          /* the height texture, based on the flow field */
  GLuint vbo_els;             /* we're using an index array to draw */
  mat4 pm;                  /* projection matrix */
  mat4 vm;                  /* view matrix */
  mat4 nm;                  /* normal matrix */

};
#endif
