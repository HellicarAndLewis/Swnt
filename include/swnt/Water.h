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
  "uniform sampler2D u_tex_tang;" // input texture which contains the tangents
  "in ivec3 a_tex;"
  "out vec3 v_norm;"
  "out vec3 v_pos;"
  "out vec2 v_tex;"
  "out vec3 v_tang;"
  "out float v_fog;"

  "void main() {"
  "  v_norm = texelFetch(u_tex_norm, ivec2(a_tex), 0).rgb;"
  "  v_tex = texelFetch(u_tex_texcoord, ivec2(a_tex), 0).rg;"
  "  v_pos = texelFetch(u_tex_pos, ivec2(a_tex), 0).rgb;"
  "  v_tang = texelFetch(u_tex_tang, ivec2(a_tex), 0).rgb;"
  "  float max_dist = 20.0;"
  "  float dist = abs(v_pos.z);"
  "  v_fog = clamp((max_dist - dist) / (max_dist), 0.0, 1.0);"
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
  "uniform sampler1D u_color_tex;"  // used to colorize by depth 

  "uniform vec3 u_sun_pos;"
  "uniform vec3 u_sun_color;"
  "uniform float u_sun_shininess;"  
  "uniform vec3 u_ambient_color;"
  "uniform float u_max_depth;" // used to offset into the depth color texture 
  "uniform float u_foam_depth;" // how much of the foam should be visible
  "uniform float u_ads_intensities[6];" // ambient, diffuse, specular, fake sun, moved_foam, texture intensities

  "in vec3 v_norm;"
  "in vec3 v_tang;"
  "in vec3 v_pos;"
  "in vec2 v_tex;"
  "in float v_fog;"
  "out vec4 fragcolor;"
  
  "void main() {"
  // "  vec2 flow_color = texture(u_flow_tex, vec2(1.0 - v_tex.s, v_tex.t)).rg;"
 "  vec2 flow_color = texture(u_flow_tex, v_tex).rg;"

  "  vec3 normal_color = texture(u_norm_tex, v_tex).rgb;"   // bump mapping
  "  vec3 diffuse_color = texture(u_diffuse_tex, v_tex).rgb;"
  "  float noise_color = texture(u_noise_tex, v_tex).r;"
  "  vec3 depth_color = texture(u_color_tex, v_pos.y/u_max_depth).rgb;"
  // "  vec3 depth_color = texture(u_color_tex, 0.9).rgb;"

  "  flow_color = -1.0 + 2.0 * flow_color;"
  "  float gradient = 1.0 - dot(v_norm, vec3(0,1,0));"

  "  float phase0 = (noise_color * 0.4 + u_time0);"
  "  float phase1 = (noise_color * 0.4 + u_time1);"

  "  float tex_scale = 1.0;"
  "  float flow_k = 0.3;"
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

  " vec3 Ka = u_ambient_color;"
  " vec3 Kd = vec3(0.0, 0.1, 0.2);"
  " vec3 Ks = vec3(0.5, 0.5, 0.0);"
  " vec3 spec = vec3(0.0);"
  " vec3 n  = v_norm; "

  " vec3 perturbed_normal = normalize((-1.0 + 2.0 * moved_normal) + v_norm);"
  " n = perturbed_normal;"

  " vec3 l     = vec3(-8.0, 15.0, 0.0);" 
  " vec3 eye_l = mat3(u_vm) * l;"
  " vec3 eye_p = mat3(u_vm) * v_pos;"
  " vec3 eye_s = normalize(eye_l - eye_p);"
  " vec3 eye_n = normalize(mat3(u_vm) * n);"
  " float ndl  = max(dot(eye_n,eye_s), 0.0);"

  " vec3 eye_sun = mat3(u_vm) * u_sun_pos;" 
  " eye_s = normalize(eye_sun - eye_p);"
  " vec3 eye_v = normalize(-eye_p);"
  " vec3 eye_r = normalize(reflect(-eye_s, eye_n));"
  " float sun_ndl = max(dot(eye_s, eye_n), 0.0);"
  " if(sun_ndl > 0.0) {"
  "   spec = Ks * pow(max(dot(eye_r, eye_v), 0.0), 4.0);"
  " } "

#if 1
  " vec3 b = (cross(v_tang, n));"
  " vec3 eye_b = (mat3(u_vm) * b);"
  " vec3 eye_t = (mat3(u_vm) * v_tang);"
  " mat3 to_object = mat3("
  "      eye_t.x, eye_b.x, eye_n.x, "
  "      eye_t.y, eye_b.y, eye_n.y, "
  "      eye_t.z, eye_b.z, eye_n.z  " 
  " );"

  " vec3 object_p = (to_object * eye_p);"
  " vec3 object_l = (to_object * eye_sun);"
  " vec3 object_s = normalize(object_l - object_p);"
  " sun_ndl = max(dot(object_s, perturbed_normal), 0.0);"
  " if(sun_ndl > 0.0) {"
  "   spec = Ks * pow(max(dot(eye_r, eye_v), 0.0), 4.0);"
  " } "
  " object_l = to_object * eye_l;"
  " object_s = normalize(object_l - object_s);"
  " ndl = max(dot(perturbed_normal, object_s), 0.0);"
#endif

  " float foam_level = u_foam_depth;"
  " float foam_k = v_pos.y / foam_level;"
  " if(foam_k > 1.0) { foam_k = 1.0; } "//  else if (foam_k <= 0.2) { foam_k = 0.0; } "
  " vec3 foam = foam_k * moved_foam;"

  // http://blog.elmindreda.org/2011/06/oceans-of-fun/ 
  " vec3 fake_sun = u_sun_color * pow(clamp(dot(eye_r, eye_v), 0.0, 1.0), u_sun_shininess);"

  //   " vec3 fake_sun = vec3(4.0, 4.0, 4.0) * pow(clamp(dot(eye_r, eye_v), 0.0, 1.0), 4.0);"

  " vec3 K = u_ads_intensities[0] * Ka;"
  " K     += u_ads_intensities[1] * Kd * ndl ;"
  " K     += u_ads_intensities[2] * spec ;"
  " K     += u_ads_intensities[3] * fake_sun;"
  " K     += depth_color;"
  " K     += mix(u_ads_intensities[5] * moved_diffuse, u_ads_intensities[4] * foam, foam_k);"

  // "  fragcolor.rgb = mix(K, (1.0 - foam_k) * (mix(moved_diffuse,depth_color, 0.3)) + moved_foam, 0.4);"  
  "fragcolor.rgb = K;"
  //  " fragcolor.rgb = mix(vec3(0.0),(1.0 - foam_k) * moved_diffuse + moved_foam * foam_k + fake_sun * 0.1, v_fog);" // NICE
  //  " fragcolor.rgb = vec3(v_fog);"
  //  " fragcolor.rgb = mix(K, (1.0 - foam_k) * (mix(moved_diffuse, depth_color, 0.9)) + foam, 0.6);"  
  //  " fragcolor.rgb = mix(K, (1.0 - foam_k) * depth_color + foam, 0.8) ;"  
  //  " fragcolor.rgb = K;"
  //  " fragcolor.rgb = depth_color;"
  //  " fragcolor.rgb = foam;"
  //  " fragcolor.rgb = mix(moved_diffuse, moved_foam, foam_k) * 0.5 + Ka +  Kd * ndl + fake_sun;" // NICE!
  //  " fragcolor.rgb = max(dot(eye_n, eye_s), 0.0) * vec3(1);"
  //  " vec3 n = normalize((-1.0 + 2.0 * normal_color) + v_norm);"
  //  " vec3 N = normalize((-1.0 + 2.0 * moved_normal) + v_norm);"    // @todo - is this correct
  //  " vec3 N = normalize(-1.0 + 2.0 * normal_color);"

#if 0
  "  fragcolor = vec4(v_norm * 0.5 + 0.5, 1.0);"
  "  fragcolor.rgb = moved_normal;"
  "  fragcolor.rgb = diffuse_color;"
  "  vec3 spec = vec3(0.0);"
  "  float height = v_pos.y / 0.1;"
  "  foam_k = (height + gradient) * 0.4; "
  "  foam_k = v_pos.y;"
  "  if(foam_k > 1.0) { foam_k = 1.0; } else if (foam_k < 0.2) { foam_k = 0.2; } "
  "  fragcolor.rgb = spec + moved_foam * foam_k  + (1.0 - foam_k) *  moved_diffuse  + 0.001 * (ndl * vec3(1.0));"
  "  fragcolor.rgb = 0.5 + 0.5 * n;"
  "  fragcolor.rgb = vec3(1.0, 0.0, 0.0) * max(dot(n, s), 0.0);"
#endif
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
  GLuint color_tex; /* 1D color ramp. */

  GLuint force_tex0;  /* used to apply a force to the height field */

  /* uniforms */
  float sun_pos[3];
  float sun_color[3];
  float sun_shininess;
  float max_depth;
  float foam_depth;
  float ads_intensities[6]; /* ambient, diffuse, specular  intensity, sun, foam, texture */
  float ambient_color[3];
};

#endif
