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

 Resources:

 - [Real-Time Water Simulation, perlin noise with code + grid method](http://www.cse.iitd.ac.in/~cs5080212/MiniP.pdf)
 - [Simulation Ocean Water, Jerry Tessendorf](http://graphics.ucsd.edu/courses/rendering/2005/jdewall/tessendorf.pdf)
 - [Sundog, Triton commercial software](http://sundog-soft.com/sds/evaluate/gallery/?nggpage=2)
 */

#ifndef WATER_GRAPHICS_H
#define WATER_GRAPHICS_H

#include <ocean/OceanSettings.h>

GLint ufl(GLuint prog, const char* name); /* wrapper that returns a uniform location + adds some checks */

static const char* FULLSCREEN_VS = ""
  "#version 150\n"
  ""
  "const vec2 verts[4] = vec2[] ("
  "  vec2(-1.0, 1.0), "
  "  vec2(-1.0, -1.0), "
  "  vec2(1.0,  1.0), "
  "  vec2(1.0, -1.0)"
  ");"
  ""
  "const vec2 tex[4] = vec2[] ("
  "  vec2(0.0, 1.0), " 
  "  vec2(0.0, 0.0), "
  "  vec2(1.0, 1.0), "
  "  vec2(1.0, 0.0)"
  ");"
  ""
  "out vec2 v_tex;"
  ""
  "void main() {"
  "  gl_Position = vec4(verts[gl_VertexID], 0.0, 1.0);"
  "  v_tex = tex[gl_VertexID];"
  "}"
  "";

static const char* FULLSCREEN_FS = ""
  "#version 150\n"
  "out vec4 fragcolor; "
  "in vec2 v_tex;"
  "void main() {"
  "  fragcolor = vec4(v_tex.x, v_tex.y, 0.0, 1.0);"
  "}";

static const char* SUBTRANSFORM_HOR_FS = ""
  "#version 150\n"
  "uniform sampler2D u_input;"
  "uniform float u_transform_size;"
  "uniform float u_subtransform_size;"
  ""
  "in vec2 v_tex;"
  "out vec4 fragcolor; " 
  ""
  "const float PI = 3.14159265359;"
  ""
  "vec2 multiply_complex(vec2 a, vec2 b) { " 
  "  return vec2(a[0] * b[0] - a[1] * b[1], a[1] * b[0] + a[0] * b[1]);"
  "}"
  ""
  "void main() {"
  "  float index = v_tex.x * u_transform_size - 0.5;"
  "  float even_index = floor(index / u_subtransform_size) * (u_subtransform_size * 0.5) + mod(index, u_subtransform_size * 0.5);"
  "  vec4 even = texture(u_input, vec2(even_index + 0.5, gl_FragCoord.y) / u_transform_size);"
  "  vec4 odd = texture(u_input, vec2(even_index + u_transform_size * 0.5 + 0.5, gl_FragCoord.y) / u_transform_size);"
  "  float twiddle_arg = -2.0 * PI * (index / u_subtransform_size);"
  "  vec2 twiddle = vec2(cos(twiddle_arg), sin(twiddle_arg));"
  "  vec2 output_a = even.xy + multiply_complex(twiddle, odd.xy);"
  "  vec2 output_b = even.zw + multiply_complex(twiddle, odd.zw);"
  "  fragcolor = vec4(output_a, output_b);"
  "}"
  "";

static const char* SUBTRANSFORM_VERT_FS = ""
  "#version 150\n"
  "uniform sampler2D u_input;"
  "uniform float u_transform_size;"
  "uniform float u_subtransform_size;"
  ""
  "in vec2 v_tex;"
  "out vec4 fragcolor; " 
  ""
  "const float PI = 3.14159265359;"
  ""
  "vec2 multiply_complex(vec2 a, vec2 b) { " 
  "  return vec2(a[0] * b[0] - a[1] * b[1], a[1] * b[0] + a[0] * b[1]);"
  "}"
  ""
  "void main() {"
  "  float index = v_tex.y * u_transform_size - 0.5;"
  "  float even_index = floor(index / u_subtransform_size) * (u_subtransform_size * 0.5) + mod(index, u_subtransform_size * 0.5);"
  "  vec4 even = texture(u_input, vec2(gl_FragCoord.x, even_index + 0.5) / u_transform_size);"
  "  vec4 odd = texture(u_input, vec2(gl_FragCoord.x, even_index + u_transform_size * 0.5 + 0.5) / u_transform_size);"
  "  float twiddle_arg = -2.0 * PI * (index / u_subtransform_size);"
  "  vec2 twiddle = vec2(cos(twiddle_arg), sin(twiddle_arg));"
  "  vec2 output_a = even.xy + multiply_complex(twiddle, odd.xy);"
  "  vec2 output_b = even.zw + multiply_complex(twiddle, odd.zw);"
  "  fragcolor = vec4(output_a, output_b);"
  "}"
  "";

static const char* INIT_SPECTRUM_FS = ""
  "#version 150\n"
  "const float PI = 3.14159265359;"
  "const float G = 9.81;"
  "const float KM = 370.0;"
  "const float CM = 0.23;"

  "uniform vec2 u_wind;"
  "uniform float u_resolution;"
  "uniform float u_size;"

  "out vec4 fragcolor;"

  "float square(float x) {"
  "  return x * x;"
  "}"

  "float omega(float k) {"
  "  return sqrt(G * k * (1.0 + square(k / KM)));"
  "}"

  "float tanh(float x) {"
  "  return (1.0 - exp(-2.0 * x)) / (1.0 + exp(-2.0 * x));"
  "}"

  "void main (void) {"
  "  vec2 coordinates = gl_FragCoord.xy - 0.5;"
  "  float n = (coordinates.x < u_resolution * 0.5) ? coordinates.x : coordinates.x - u_resolution;"
  "  float m = (coordinates.y < u_resolution * 0.5) ? coordinates.y : coordinates.y - u_resolution;"
  "  vec2 wave_vector = (2.0 * PI * vec2(n, m)) / u_size;"
  "  float k = length(wave_vector);"

  "  float U10 = length(u_wind);"

  "  float Omega = 0.84;"
  "  float kp = G * square(Omega / U10);"

  "  float c = omega(k) / k;"
  "  float cp = omega(kp) / kp;"

  "  float Lpm = exp(-1.25 * square(kp / k));"
  "  float gamma = 1.7;"
  "  float sigma = 0.08 * (1.0 + 4.0 * pow(Omega, -3.0));"
  "  float Gamma = exp(-square(sqrt(k / kp) - 1.0) / 2.0 * square(sigma));"
  "  float Jp = pow(gamma, Gamma);"
  "  float Fp = Lpm * Jp * exp(-Omega / sqrt(10.0) * (sqrt(k / kp) - 1.0));"
  "  float alphap = 0.006 * sqrt(Omega);"
  "  float Bl = 0.5 * alphap * cp / c * Fp;"

  "  float z0 = 0.000037 * square(U10) / G * pow(U10 / cp, 0.9);"
  "  float uStar = 0.41 * U10 / log(10.0 / z0);"
  "  float alpham = 0.01 * ((uStar < CM) ? (1.0 + log(uStar / CM)) : (1.0 + 3.0 * log(uStar / CM)));"
  "  float Fm = exp(-0.25 * square(k / KM - 1.0));"
  "  float Bh = 0.5 * alpham * CM / c * Fm * Lpm;"

  "  float a0 = log(2.0) / 4.0;"
  "  float am = 0.13 * uStar / CM;"
  "  float Delta = tanh(a0 + 4.0 * pow(c / cp, 2.5) + am * pow(CM / c, 2.5));"

  "  float cosPhi = dot(normalize(u_wind), normalize(wave_vector));"

  "  float S = (1.0 / (2.0 * PI)) * pow(k, -4.0) * (Bl + Bh) * (1.0 + Delta * (2.0 * cosPhi * cosPhi - 1.0));"

  "  float dk = 2.0 * PI / u_size;"
  "  float h = sqrt(S / 2.0) * dk;"

  "  if(wave_vector.x == 0.0 && wave_vector.y == 0.0){"
  "    h = 0.0;" //no DC term
  "  }"

  "  fragcolor = vec4(h, 0.0, 0.0, 0.0);"
  "}"
  "";

static const char* PHASE_FS = ""
  "#version 150\n"
  "const float PI = 3.14159265359;"
  "const float G = 9.81;"
  "const float KM = 370.0;"

  "in vec2 v_tex;"
  "out vec4 fragcolor;"

  "uniform sampler2D u_phases;"

  "uniform float u_deltaTime;"
  "uniform float u_resolution;"
  "uniform float u_size;"

  "float omega (float k) {"
  "  return sqrt(G * k * (1.0 + k * k / KM * KM));"
  "}"

  "void main (void) {"
  "  float deltaTime = 1.0 / 60.0;"
  "  vec2 coordinates = gl_FragCoord.xy - 0.5;"
  "  float n = (coordinates.x < u_resolution * 0.5) ? coordinates.x : coordinates.x - u_resolution;"
  "  float m = (coordinates.y < u_resolution * 0.5) ? coordinates.y : coordinates.y - u_resolution;"
  "  vec2 waveVector = (2.0 * PI * vec2(n, m)) / u_size;"

  "  float phase = texture(u_phases, v_tex).r;"
  "  float deltaPhase = omega(length(waveVector)) * u_deltaTime;"
  "  phase = mod(phase + deltaPhase, 2.0 * PI);"

  "  fragcolor = vec4(phase, 0.0, 0.0, 0.0);"
  "}"
  "";

static const char* SPECTRUM_FS = ""
  "#version 150\n"
  "const float PI = 3.14159265359;"
  "const float G = 9.81;"
  "const float KM = 370.0;"

  "in vec2 v_tex;"
  "out vec4 fragcolor;"

  "uniform float u_size;"
  "uniform float u_resolution;"

  "uniform sampler2D u_phases;"
  "uniform sampler2D u_initial_spectrum;"

  "uniform float u_choppiness;"

  "vec2 multiplyComplex (vec2 a, vec2 b) {"
  "  return vec2(a[0] * b[0] - a[1] * b[1], a[1] * b[0] + a[0] * b[1]);"
  "}"

  "vec2 multiplyByI (vec2 z) {"
  "  return vec2(-z[1], z[0]);"
  "}"

  "float omega (float k) {"
  "  return sqrt(G * k * (1.0 + k * k / KM * KM));"
  "}"

  "void main (void) {"
  "  vec2 coordinates = gl_FragCoord.xy - 0.5;"
  "  float n = (coordinates.x < u_resolution * 0.5) ? coordinates.x : coordinates.x - u_resolution;"
  "  float m = (coordinates.y < u_resolution * 0.5) ? coordinates.y : coordinates.y - u_resolution;"
  "  vec2 waveVector = (2.0 * PI * vec2(n, m)) / u_size;"
     
  "  float phase = texture(u_phases, v_tex).r;"
  "  vec2 phaseVector = vec2(cos(phase), sin(phase));"
     
  "  vec2 h0 = texture(u_initial_spectrum, v_tex).rg;"
  "  vec2 h0Star = texture(u_initial_spectrum, vec2(1.0 - v_tex + 1.0 / u_resolution)).rg;"
  "  h0Star.y *= -1.0;"
     
  "  vec2 h = multiplyComplex(h0, phaseVector) + multiplyComplex(h0Star, vec2(phaseVector.x, -phaseVector.y));"
     
  "  vec2 hX = -multiplyByI(h * (waveVector.x / length(waveVector))) * u_choppiness;"
  "  vec2 hZ = -multiplyByI(h * (waveVector.y / length(waveVector))) * u_choppiness;"

  //no DC term
  "  if (waveVector.x == 0.0 && waveVector.y == 0.0) {"
  "    h = vec2(0.0);"
  "    hX = vec2(0.0);"
  "    hZ = vec2(0.0);"
  "  }"

  "  fragcolor = vec4(hX + multiplyByI(h), hZ);"
  "}"
  "";

static const char* NORMAL_MAP_FS = ""
  "#version 150\n"
  "in vec2 v_tex;"
  "out vec4 fragcolor;"

  "uniform sampler2D u_displacement_map;"
  "uniform float u_resolution;"
  "uniform float u_size;"

  "void main (void) {"
  "  float texel = 1.0 / u_resolution;"
  "  float texelSize = u_size / u_resolution;"
     
  "  vec3 center = texture(u_displacement_map, v_tex).rgb;"
  "  vec3 right = vec3(texelSize, 0.0, 0.0) + texture(u_displacement_map, v_tex + vec2(texel, 0.0)).rgb - center;"
  "  vec3 left = vec3(-texelSize, 0.0, 0.0) + texture(u_displacement_map, v_tex + vec2(-texel, 0.0)).rgb - center;"
  "  vec3 top = vec3(0.0, 0.0, -texelSize) + texture(u_displacement_map, v_tex + vec2(0.0, -texel)).rgb - center;"
  "  vec3 bottom = vec3(0.0, 0.0, texelSize) + texture(u_displacement_map, v_tex + vec2(0.0, texel)).rgb - center;"
     
  "  vec3 topRight = cross(right, top);"
  "  vec3 topLeft = cross(top, left);"
  "  vec3 bottomLeft = cross(left, bottom);"
  "  vec3 bottomRight = cross(bottom, right);"

  "  fragcolor = vec4(normalize(topRight + topLeft + bottomLeft + bottomRight), 1.0);"
  "}"
  "";

static const char* OCEAN_VS = ""
  "#version 150\n"
  "in vec3 a_pos;"
  "in vec2 a_tex;"

  "out vec3 v_pos;"
  "out vec2 v_tex;"

  "uniform mat4 u_pm;"
  "uniform mat4 u_vm;"

  "uniform float u_size;"
  "uniform float u_geometry_size;"

  "uniform sampler2D u_displacement_map;"

  "void main (void) {"
  "  vec3 position = a_pos + texture(u_displacement_map, a_tex).rgb * (u_geometry_size / u_size);"
  "  v_pos = position;"
  "  v_tex = a_tex;"
  "  gl_Position = u_pm * u_vm * vec4(position, 1.0);"
  "}"
  "";


static const char* OCEAN_FS = ""
  "#version 150\n"
  "in vec2 v_tex;"
  "in vec3 v_pos;"
  "out vec4 fragcolor;"

  "uniform sampler2D u_displacement_map;"
  "uniform sampler2D u_normal_map;"

  "uniform vec3 u_camera_position;"

  "uniform vec3 u_ocean_color;"
  "uniform vec3 u_sky_color;"
  "uniform float u_exposure;"

  "uniform vec3 u_sun_direction;"

  "vec3 hdr(vec3 color, float exposure) {"
  "  return 1.0 - exp(-color * exposure);"
  "}"

  "void main (void) {"
  "  vec3 normal = texture(u_normal_map, v_tex).rgb;"
#if 1
  "  vec3 view = normalize(u_camera_position - v_pos);"
#else
  "  vec3 campos = vec3(-500.0, 419.0, 0.0);"
  "  vec3 view = normalize(campos - v_pos);"
#endif
  "  float fresnel = 0.05 + 0.95 * pow(1.0 - dot(normal, view), 4.0);"
  "  vec3 sky = fresnel * u_sky_color;"
  "  float diffuse = clamp(dot(normal, normalize(u_sun_direction)), 0.0, 1.0);"
  "  vec3 water = (1.0 - fresnel) * u_ocean_color * u_sky_color * diffuse;"
  "  vec3 color = sky + water;"
  "  fragcolor = vec4(hdr(color, u_exposure), 1.0);"
  "}"
  "";

class WaterGraphics {

 public:
  WaterGraphics(OceanSettings& settings);
  ~WaterGraphics();
  bool setup(int winW, int winH);

 public:
  OceanSettings& settings;
  int win_w;
  int win_h;

  // Fullscreen 
  GLuint fullscreen_vao;
  GLuint fullscreen_vs;
  GLuint fullscreen_fs;
  GLuint fullscreen_prog;

  /* Sub transform */
  GLuint hor_subtrans_fs;
  GLuint hor_subtrans_prog;
  GLuint vert_subtrans_fs;
  GLuint vert_subtrans_prog;

  /* Initial spectrum */
  GLuint init_spectrum_fs;
  GLuint init_spectrum_prog;
  
  /* Phase */
  GLuint phase_fs;
  GLuint phase_prog;

  /* Spectrum */
  GLuint spectrum_fs;
  GLuint spectrum_prog;

  /* Normal map */
  GLuint normal_map_fs;
  GLuint normal_map_prog;

  /* Ocean */
  GLuint ocean_vs;
  GLuint ocean_fs;
  GLuint ocean_prog;


  mat4 pm; /* perspective matrix */
};

#endif
