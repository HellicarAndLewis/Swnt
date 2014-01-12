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
  "uniform sampler2D u_tex_gradient;" 
  //  "uniform sampler2D u_tex_tang;" // input texture which contains the tangents
  "in ivec3 a_tex;"
  "out vec3 v_norm;"
  "out vec3 v_pos;"
  "out vec2 v_tex;"
  "out vec3 v_gradient;"
  //  "out vec3 v_tang;"
  "out float v_fog;"

  "void main() {"
  "  v_norm = texelFetch(u_tex_norm, ivec2(a_tex), 0).rgb;"
  "  v_tex = texelFetch(u_tex_texcoord, ivec2(a_tex), 0).rg;"
  "  v_pos = texelFetch(u_tex_pos, ivec2(a_tex), 0).rgb;"
  //  "  v_tang = texelFetch(u_tex_tang, ivec2(a_tex), 0).rgb;"
  "  v_gradient = texelFetch(u_tex_gradient, ivec2(a_tex), 0).rgb;"
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
  "uniform sampler2D u_extra_flow_tex;" // contains customly rendered colors (e.g. custom streams)
  "uniform sampler2D u_foam_tex;"
  "uniform sampler1D u_color_tex;"  // used to colorize by depth 
  "uniform sampler2D u_foam_delta_tex;" // the foam delta, used to mix foam
  "uniform sampler2D u_foam_colors;" // the 3 levels of foam 
  "uniform sampler2D u_foam_ramp;" // used to mix the 3 foam levels

  "uniform vec3 u_sun_pos;"
  "uniform vec3 u_sun_color;"
  "uniform float u_sun_shininess;"  
  "uniform vec3 u_ambient_color;"
  "uniform float u_max_depth;" // used to offset into the depth color texture 
  "uniform float u_foam_depth;" // how much of the foam should be visible
  "uniform float u_ads_intensities[6];" // ambient, diffuse, specular, fake sun, moved_foam, texture intensities

  "in vec3 v_norm;"
  //  "in vec3 v_tang;"
  "in vec3 v_gradient;"
  "in vec3 v_pos;"
  "in vec2 v_tex;"
  "in float v_fog;"
  "out vec4 fragcolor;"
  
  "void main() {"
  " vec2 flipped_tex = vec2(1.0 - v_tex.s, v_tex.t);"

#if 0
 "  vec2 flow_color = texture(u_flow_tex, vec2(1.0 - v_tex.s, v_tex.t)).rg;"
#else
 "  vec2 flow_color = texture(u_flow_tex, v_tex).rg;"
#endif

  "  float foam_delta = texture(u_foam_delta_tex, v_tex).r;"
  "  vec3 normal_color = texture(u_norm_tex, v_tex).rgb;"   // bump mapping
  "  vec3 diffuse_color = texture(u_diffuse_tex, v_tex).rgb;"
  "  float noise_color = texture(u_noise_tex, v_tex).r;"
  "  vec3 depth_color = texture(u_color_tex, v_pos.y/u_max_depth).rgb;"
  "  vec4 extra_color = texture(u_extra_flow_tex, flipped_tex);"

#if 1
  // This is what really makes the difference between a waterlike rendering
  // or not. By moving the water into the direction of the normal. 
  "  flow_color = (v_norm.xz * 0.1 + flow_color);"  // move the texture in the direction of the normal
  "  flow_color -= extra_color.rg;"  // move the texture in the direction of the normal
  "  flow_color += v_gradient.xy;"
  "  float phase0 = noise_color * 0.2 + u_time0;"
  "  float phase1 =  noise_color * 0.2 + u_time1;"

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

  "  vec3 Ka = u_ambient_color;"
  "  vec3 Kd = vec3(0.0, 0.1, 0.2);"
  "  vec3 Ks = vec3(0.5, 0.5, 0.0);"
  "  vec3 spec = vec3(0.0);"
  "  vec3 n  = v_norm; "
     
  "  vec3 perturbed_normal = normalize((-1.0 + 2.0 * moved_normal) + v_norm);"
  "  n = perturbed_normal;"
     
  "  vec3 l     = vec3(-8.0, 15.0, 0.0);" 
  "  vec3 eye_l = mat3(u_vm) * l;"
  "  vec3 eye_p = mat3(u_vm) * v_pos;"
  "  vec3 eye_s = normalize(eye_l - eye_p);"
  "  vec3 eye_n = normalize(mat3(u_vm) * n);"
  "  float ndl  = max(dot(eye_n,eye_s), 0.0);"
     
  "  vec3 eye_sun = mat3(u_vm) * u_sun_pos;" 
  "  eye_s = normalize(eye_sun - eye_p);"
  "  vec3 eye_v = normalize(-eye_p);"
  "  vec3 eye_r = normalize(reflect(-eye_s, eye_n));"
  "  float sun_ndl = max(dot(eye_s, eye_n), 0.0);"
  "  if(sun_ndl > 0.0) {"
  "    spec = Ks * pow(max(dot(eye_r, eye_v), 0.0), 4.0);"
  "  } "


  //   " foam_delta = 1.0;"
  "  float foam_flow_k = 0.2;"
  "  vec2 foam_tc0 = vec2(flipped_tex.x + foam_flow_k * texcoord0.x, flipped_tex.y + foam_flow_k * texcoord0.y);"
  "  vec2 foam_tc1 = vec2(flipped_tex.x + foam_flow_k * texcoord1.x, flipped_tex.y + foam_flow_k * texcoord1.y);"
  "  vec3 foam_ramp_color = texture(u_foam_ramp, vec2(clamp(pow(foam_delta * 0.08, 4.0), 0.0, 0.99), 0.0)).rgb;"
  //  "  vec3 foam_color0 = texture(u_foam_colors, texcoord0).rgb;"
  //  "  vec3 foam_color1 = texture(u_foam_colors, texcoord1).rgb;"
  "  vec3 foam_color0 = texture(u_foam_colors, foam_tc0 * 5.0).rgb;"
  "  vec3 foam_color1 = texture(u_foam_colors, foam_tc1 * 5.0).rgb;"
  "  vec3 foam_color = mix(foam_color0, foam_color1, lerp);"
  //  "  foam_color = vec3(foam_color.b, foam_color.g, foam_color.b);"
  // "  float foam_m = dot(foam_ramp_color, foam_color); " 
   "  float foam_m = dot(foam_ramp_color, texture(u_foam_colors, flipped_tex * 5.0).rgb); " 

  //"  float foam_k = foam_delta + (v_pos.y / u_foam_depth);"
  
  // " foam = vec3(foam_k);"


  "  float foam_level = u_foam_depth;"
  "  float foam_k = v_pos.y / foam_level;"
  "  if(foam_k > 1.0) { foam_k = 1.0; } "//  else if (foam_k <= 0.2) { foam_k = 0.0; } "
  "  vec3 foam = foam_k * moved_foam;"
 "  foam += foam_m * 0.5;"

  // http://blog.elmindreda.org/2011/06/oceans-of-fun/ 
  "  vec3 fake_sun = u_sun_color * pow(clamp(dot(eye_r, eye_v), 0.0, 1.0), u_sun_shininess);"

  "  vec3 K = u_ads_intensities[0] * Ka;"
  "  K     += u_ads_intensities[1] * Kd * ndl ;"
  "  K     += u_ads_intensities[2] * spec ;"
  "  K     += u_ads_intensities[3] * fake_sun;"
  "  K     += depth_color;"
  "  K     += mix(u_ads_intensities[5] * moved_diffuse, u_ads_intensities[4] * foam, foam_k);"

  "  fragcolor.rgb = K;"
  "  fragcolor.a = 1.0;"

  //  "  fragcolor.rgb = vec3(foam_m);"
  //  "  foam_delta = 0.0; "


  //"  foam_delta = 0.9;"


  //  "  vec3 foam_color = texture(u_foam_colors, flipped_tex * 5.0).rgb;"
  //  "  fragcolor.rgb = texture(u_foam_colors, v_tex).rgb;"
  //"  fragcolor.rgb += 0.2 * vec3(dot(foam_ramp_color, foam_color));"
  //  " fragcolor.rgb = foam_ramp_color;"
#else 
  //  "  float speed = length(v_norm * 0.5 + 0.5);"
  //"  float speed = length(v_gradient);"
  /*
  "  float speed = length(vec2(v_norm.x, v_norm.z)) * 3.4;"
  "  fragcolor.rgb = v_norm * 0.5 + 0.5;"
  "  fragcolor.rgb = texture(u_color_tex, speed).rgb;"
  "  fragcolor.a = 1.0;"

  */
  //    "  fragcolor.rgb = abs(v_gradient.rgb);"
    "  fragcolor.rgb = vec3(v_gradient.x, v_gradient.y, 0.0);"

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
  void print();               /* prints some debug info */
  void beginGrabFlow();       /* start rendering extra flow colors which we mix with the diffuse water texture */
  void endGrabFlow();         /* end rendering the extra flow colors */
  void blitFlow(float x, float y, float w, float h);

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
  GLuint color_tex;           /* 1D color ramp to change the colors of the water */

  GLuint foam_colors_tex; /* R,G,B layers with foam */
  GLuint foam_ramp_tex;  /* used to mix the R/G/B layers */

  GLuint force_tex0;          /* used to apply a force to the height field */

  /* Rendering extra diffuse/flow colors */
  GLuint fbo;
  GLuint extra_flow_tex;      /* everything that gets rendered between "beginGrabFlow" and "endGrabFlow" is added to the flow forces */

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
