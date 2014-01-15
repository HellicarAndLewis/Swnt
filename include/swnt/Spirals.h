/*

  Spirals
  -------
  Renders and simulates the spirals coming from the blobs
  which have been detected using a kinect.

 */

#ifndef SWNT_SPIRALS_H
#define SWNT_SPIRALS_H

#define ROXLU_USE_OPENGL
#define ROXLU_USE_MATH
#include <tinylib.h>
#include <swnt/Particles.h>

class Settings;
class Tracking;
class Graphics;
class Flow;

static const char* SPIRAL_VS = ""
  "#version 150\n"
  "uniform mat4 u_pm;"
  "uniform mat4 u_vm;"
  "in vec4 a_pos;"
  "in vec2 a_tex;"
  "in float a_age_perc;"
  "in float a_pos_perc;"
  "out float v_age_perc;"
  "out float v_pos_perc;"
  "out vec2 v_tex;"
  "void main() {"
  "  gl_Position = u_pm * u_vm * a_pos;"
  "  v_age_perc = a_age_perc;"
  "  v_pos_perc = a_pos_perc;"
  "  v_tex = a_tex;"
  "}"
  "";

static const char* SPIRAL_FS = ""
  "#version 150\n"
  "uniform sampler2D u_diffuse_tex;"
  "uniform vec3 u_col_from;"
  "uniform vec3 u_col_to;"
  "uniform float u_alpha;"
  "out vec4 fragcolor; "
  "in float v_age_perc;"
  "in float v_pos_perc;"
  "in vec2 v_tex;"
  "void main() {"
  "  vec4 diffuse_tc = texture(u_diffuse_tex, v_tex);"
  "  fragcolor.a = diffuse_tc.r * (1.0 - (pow(v_age_perc, 4.0))) * u_alpha;"
  "  fragcolor.rgb = mix(u_col_from, u_col_to, v_tex.s);"
  "}"
  "";

static const char* SPIRAL_DISPLACEMENT_FS = ""
  "#version 150\n"
  "uniform sampler2D u_diffuse_tex;"
  "uniform vec3 u_col_from;"
  "uniform vec3 u_col_to;"
  "uniform float u_alpha;"
  "out vec4 fragcolor; "
  "in float v_age_perc;"
  "in float v_pos_perc;"
  "in vec2 v_tex;"
  "void main() {"
  "  vec4 diffuse_tc = texture(u_diffuse_tex, v_tex);"
  "  fragcolor.a = diffuse_tc.r * (1.0 - (pow(v_age_perc, 4.0))) * u_alpha;"
  "  fragcolor.rgb = diffuse_tc.rgb;"
  "  fragcolor.a = 1.0;"
  "}"
  "";

struct SpiralVertex {
  float* ptr() {  return pos.ptr(); } 
  vec3 pos;
  vec2 tex;
  float age_perc;
  float pos_perc;
};

class Spirals {

 public:
  Spirals(Settings& settings, Tracking& tracker, Graphics& graphics, Flow& flow);
  bool setup();
  void update(float dt);
  void draw();
  void drawDisplacement();
  void refresh();                            /* will change to the current setting values */

 private: 
  void draw(float alpha);
  void spawnParticles();                     /* spawn particles around blobs and tracked objects */
  void updateVertices();                     /* update the vertices of the spirals, triangle strips */
  bool setupGraphics();                      /* setup opengl state */
  void applyVelocityField();                 /* apply the velocity field of the flow object to the particles */
  void applyCenterForce();                   /* make sure that all particles are attracted to the center */
  void applyPerlinToField();                 /* use perlin noise to influence the vector field; we' use the perlin value as angle */
  void applyVortexToField();                 /* creates a vortex force where we found something that we can track */

 public:
  Settings& settings;
  Tracking& tracker;
  Graphics& graphics;
  Flow& flow;
  Particles particles;

  /* Particle behavior  */
  float min_lifetime;
  float max_lifetime;
  float min_strip_width;
  float max_strip_width;
  float min_mass;
  float max_mass;

  /* Vertices */
  size_t bytes_allocated;                      /* how many bytes we have allocated for our vbo that hold the spiral vertices */
  std::vector<SpiralVertex> vertices;          /* the spiral vertices */
  std::vector<GLint> vertex_offsets;           /* used with glMultiDraw() */
  std::vector<GLsizei> vertex_counts;          /* used "" " */

  /* GL */
  GLuint vao;                                  /* vao used to render spirals */
  GLuint vbo;                                  /* vbo that holds the spiral vertices */
  GLuint vert;                                 /* vertex shader */
  GLuint frag;                                 /* fragment shader */
  GLuint prog;                                 /* program, used when drawing the spirals */
  GLuint diffuse_tex;                          /* the diffuse texture used for the spirals */
  mat4 pm;                                     /* projection matrix .. ortho graphic projection */
  mat4 vm;                                     /* view matrix */

  /* Displacement test */
  GLuint displacement_frag;
  GLuint displacement_prog;
  GLuint displacement_fbo;
  GLuint displacement_tex;

  /* Physics */
  Particle* center_particle;
};
#endif
