/*

  Water
  ------

  This class renders the given height fied (using most of the 
  internal types of this HeightField class) using a flow map
  to simulate the flow of water.

 */

#ifndef WATER_H
#define WATER_H

#define ROXLU_USE_OPENGL
#define ROXLU_USE_MATH
#define ROXLU_USE_PNG
#include <tinylib.h>
#include <vector>
#include <string>


// ------------------------------------------------------

static const char* WATER_VS = ""
  "#version 150\n"
  "uniform mat4 u_pm;"
  "uniform mat4 u_vm;"
  "uniform sampler2D u_tex_pos;"
  "uniform sampler2D u_tex_norm;"
  "uniform sampler2D u_tex_texcoord;"
  "in ivec3 a_tex;"
  "out vec3 v_norm;"
  "out vec3 v_pos;"
  "out vec2 v_tex;"

  "void main() {"
  "  v_norm = texelFetch(u_tex_norm, ivec2(a_tex), 0).rgb;"
  "  v_tex = texelFetch(u_tex_texcoord, ivec2(a_tex), 0).rg;"
  "  v_pos = texelFetch(u_tex_pos, ivec2(a_tex), 0).rgb;"
  "  gl_Position = u_pm * u_vm * vec4(v_pos, 1.0);"

  "}"
  "";

static const char* WATER_FS = ""
  "#version 150\n"
  "uniform mat4 u_vm;"
  "uniform float u_time;"
  "uniform float u_time0;"
  "uniform float u_time1;"
  "uniform sampler2D u_noise_tex;"
  "uniform sampler2D u_flow_tex;"
  "uniform sampler2D u_norm_tex;" // not the same as the norm in WATER_VS 
  "uniform sampler2D u_diffuse_tex;"
  "uniform sampler2D u_foam_tex;"

  "in vec3 v_norm;"
  "in vec3 v_pos;"
  "in vec2 v_tex;"
  "out vec4 fragcolor;"
  
  "void main() {"
  //"  vec2 flow_color = texture(u_flow_tex, vec2(gl_FragCoord.x / 1024.0,gl_FragCoord.y / 768.0)).rg;"
  //" vec2 flow_color = texture(u_flow_tex, v_tex).rg;"
  "  vec2 flow_color = texture(u_flow_tex, vec2(1.0 - v_tex.s, v_tex.t)).rg;"
  "  vec3 normal_color = texture(u_norm_tex, v_tex).rgb;"   // bump mapping
  "  vec3 diffuse_color = texture(u_diffuse_tex, v_tex).rgb;"
  "  float noise_color = texture(u_noise_tex, v_tex).r;"

  "  float gradient = 1.0 - dot(v_norm, vec3(0,1,0));"

  "  float phase0 = (noise_color * 0.4 + u_time0);"
  "  float phase1 = (noise_color * 0.4 + u_time1);"
  //"  flow_color = vec2(flow_color.g, flow_color.r);"
  //  "  flow_color = normalize(v_norm.xz + flow_color);"  // move the texture in the direction of the normal
  //  "  flow_color = (v_norm.xz * 0.4 + flow_color * 0.7);"  // move the texture in the direction of the normal
  "  flow_color = -1.0 + 2.0 * flow_color;"

  "  float tex_scale = 1.0;"
   " float flow_k = 0.2;"
  "  vec2 texcoord0 = (v_tex * tex_scale) + (flow_color * phase0 * flow_k);"
  "  vec2 texcoord1 = (v_tex * tex_scale) + (flow_color * phase1 * flow_k);"

  "  vec3 normal0 = texture(u_norm_tex, texcoord0).rgb;"
  "  vec3 normal1 = texture(u_norm_tex, texcoord1).rgb;" 
  "  float lerp = abs(0.5 - u_time0) / 0.5;"
  "  vec3 moved_normal = mix(normal0, normal1, lerp);"

  "  vec3 diffuse0 = texture(u_diffuse_tex, texcoord0).rgb;"
  "  vec3 diffuse1 = texture(u_diffuse_tex, texcoord1).rgb;"
  "  vec3 moved_diffuse = mix(diffuse0, diffuse1, lerp);"

  "  vec3 foam0 = texture(u_foam_tex, texcoord0 * 4.0).rgb;"
  "  vec3 foam1 = texture(u_foam_tex, texcoord1 * 4.0).rgb;"
  "  vec3 moved_foam = mix(foam0, foam1, lerp);"

  "  vec3 N = normalize((-1.0 + 2.0 * moved_normal) + v_norm);"    // @todo - is this correct
  "  vec3 L = vec3(-60,1,0);"
  "  float ndl = max(dot(N, L), 0.0);"

  "  fragcolor = vec4(v_norm * 0.5 + 0.5, 1.0);"
  "  fragcolor.rgb = moved_normal;"
  "  fragcolor.rgb = diffuse_color;"
  
  "  vec3 spec = vec3(0.0);"


  // test normal mapping
  #if 1
  "  vec3 tang = vec3(1.0, 0.0, 0.0);"
  "  vec3 binorm = cross(tang, N);"
  "  mat3 to_object = mat3(tang.x, binorm.x, N.x,"
  "                        tang.y, binorm.y, N.y, "
  "                        tang.z, binorm.z, N.z);"
  "  vec3 L_object = to_object * L;"
  "  vec3 V_object = to_object * normalize(-v_pos);"
  "  ndl = max(dot(N, L_object), 0.0);"

  #endif


#if 0
  // specular (it looks like my 
  "  vec3 P_eye = normalize(mat3(u_vm) * v_pos);"
  "  vec3 V_eye = normalize(-P_eye);"
  "  vec3 L_eye = normalize(mat3(u_vm) * L);"
  "  vec3 S_eye = normalize(L_eye - P_eye);"
  "  vec3 N_eye = normalize(mat3(u_vm) * N);"
  "  vec3 R = reflect(-S_eye, N_eye);"
  "  if(ndl > 0.0) { "
  "    spec = pow(max(dot(R, V_eye), 0.0), 4.0) * vec3(1.0);"
  "  }"
#endif

#if 1
  // normal mapping
  // @todo
#endif
  "  float height = v_pos.y / 0.1;"
  "  float foam_k = (height + gradient) * 0.4; "
  "  foam_k = v_pos.y;"
  "  if(foam_k > 1.0) { foam_k = 1.0; } else if (foam_k < 0.2) { foam_k = 0.2; } "
  "  fragcolor.rgb = spec + moved_foam * foam_k  + (1.0 - foam_k) *  moved_diffuse  + 0.001 * (ndl * vec3(1.0));"
  // "  fragcolor.rgb = vec3(0.0)  +  (ndl * vec3(0.0, 0.0, 0.43));"
#if 0
  "  if(gl_FragCoord.x < 640) {"
  "     fragcolor.rgb = vec3(ndl);"
  "     fragcolor.rgb = v_norm * 0.5 + 0.5;"
  "     fragcolor.rgb = vec3(gradient);"
  "     fragcolor.rgb = 0.5 * N_eye + 0.5;"
  "     fragcolor.rgb = 0.5 + 0.5 * N;"
  "     fragcolor.rgb = spec;"
  "  }"
#endif

  //"fragcolor = vec4(0.5 + 0.5 * flow_color, 0.0, 1.0);"
  // " fragcolor = vec4(v_tex, 0.0, 1.0);"
  // " fragcolor = vec4(v_tex.r,0.0, 0.0, 1.0);"
  //" fragcolor = vec4(gl_FragCoord.x / 1024.0,0.0, 0.0, 1.0);"
  //"fragcolor = vec4(flow_color, 0, 1) ;"
  "}"

  "";

// ------------------------------------------------------

class HeightField;

class Water {

 public:
  Water(HeightField& heightField);
  bool setup(int winW, int winH);
  void update(float dt);
  void draw();

 private:
  GLuint createTexture(std::string filename);
  
 public:

  HeightField& height_field;
  int win_w;
  int win_h;

  GLuint prog;  
  GLuint vert;
  GLuint frag;
  GLuint flow_tex;
  GLuint normals_tex;
  GLuint noise_tex;
  GLuint diffuse_tex;
  GLuint foam_tex;

  GLuint force_tex0; 
};

#endif
