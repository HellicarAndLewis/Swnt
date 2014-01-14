#ifndef HEIGHTFIELD_H
#define HEIGHTFIELD_H


#define HEIGHT_FIELD_W 1024
#define HEIGHT_FIELD_H 768

#define HEIGHT_FIELD_N 128
#define HEIGHT_FIELD_NN (HEIGHT_FIELD_N * HEIGHT_FIELD_N)

#define HEIGHT_FIELD_DRAW_WIDTH 35.0
#define HEIGHT_FIELD_DRAW_HEIGHT 35.0
#define HEIGHT_FIELD_QUOTE_E(x) # x
#define HEIGHT_FIELD_QUOTE(x) HEIGHT_FIELD_QUOTE_E(x)

#define ROXLU_USE_OPENGL
#define ROXLU_USE_MATH
#define ROXLU_USE_PNG
#include <tinylib.h>
#include <vector>


/*

  Versions: 
  
  v.0.0.0.4   - added position buffer    - https://gist.github.com/roxlu/2c8a964700cb806df0e0
  v.0.0.0.3.3 - rendering/normals        - https://gist.github.com/roxlu/e85690f04be0968f316b
  v.0.0.0.3.2 - rendering                - https://gist.github.com/roxlu/aabcf9decfd6a1f30d9b
  v.0.0.0.3.1 - diffuse + normals        - https://gist.github.com/roxlu/ddbc776af57e98c5b6a0
  v.0.0.0.3   - diffuse working          - https://gist.github.com/roxlu/8b49a9932c158ac9bd16
  v.0.0.0.2   - first diffuse test       - https://gist.github.com/roxlu/904865ede5d32e10ff24
  v.0.0.0.1   - initial version          - https://gist.github.com/roxlu/b0cf42bfdad3550565c1
 */

// Awesome WebGL fluid sim: http://www.youtube.com/watch?v=f_6aTwP2lMg
// ""      ""     demo:     http://skeelogy.github.io/skunami.js/examples/skunami_twoWayCoupling.html


// Diffuses the height field
static const char* HF_DIFFUSE_VERT = ""
  "#version 150\n"
  "in ivec2 a_tex;                "
  "uniform sampler2D u_tex_u;     "
  "uniform sampler2D u_tex_forces;"
  "uniform sampler2D u_tex_v;     "
  "const float dt = 0.16;         "
  "out float v_new_u_value;       "
  "out float v_new_v_value;       "

  "float get_force(int i, int j) {"
  "  float f0 = texelFetch(u_tex_u,      ivec2(a_tex.s + i, a_tex.t + j), 0).r;"
  "  vec2  f1 = texelFetch(u_tex_forces, ivec2(a_tex.s + i, a_tex.t + j), 0).rg;"
  "  return f0 + (f1.r  * 5.2) - (f1.g * 9.2);"
  "}"

  "void main() {                  "
  "  gl_Position = vec4(-1.0 + float(a_tex.x) * (1.0 / " HEIGHT_FIELD_QUOTE(HEIGHT_FIELD_N) ") * 2.0, -1.0 + float(a_tex.y) * (1.0 / " HEIGHT_FIELD_QUOTE(HEIGHT_FIELD_N) ") * 2.0, 0.0, 1.0);"

  "  float u_center = get_force( 0,  0);"
  "  float u_left   = get_force(-1,  0);"
  "  float u_right  = get_force( 1,  0);"
  "  float u_top    = get_force( 0, -1);"
  "  float u_bottom = get_force( 0,  1);"

  "  float f = 4.4 * ((u_right + u_left + u_bottom + u_top) - (4.0 * u_center));"
  "  { "
  "    float max = 4.5;" // set to 0.5 for slow/stable water
  "    if(f > max) { f = max; } else if( f < -max) { f = -max; } "
  "  } "

  "  float v = texelFetch(u_tex_v, a_tex, 0).r;"
  "  v_new_v_value = (v + f * dt); "
  "  v_new_u_value = u_center + v_new_v_value * dt;"
  "  v_new_v_value = v_new_v_value * 0.995;" // set to 0.975 when simulation explodes

#if 0
  "  int low = 50; "
  "  int high = 60;"
  "  if(a_tex.s >= low && a_tex.s <= high && a_tex.t >= low && a_tex.t <= high){ "
  "    v_new_u_value = 6.0; "
  "  }"
#endif

  "}"
  "";

static const char* HF_DIFFUSE_FRAG = ""
  "#version 150\n"
  "out vec4 v_out; "
  "out vec4 u_out; "
  "in float v_new_u_value; "
  "in float v_new_v_value; "
  "void main() {"
  "  v_out = vec4(v_new_v_value);"
  "  u_out = vec4(v_new_u_value);"
  "}"
  "";

// Positions: renders the positions (later gradients too?)
// -----------------------------------------------------------
static const char* HF_POSITION_VS = ""
  "#version 150\n"
  "uniform float u_time;" 
  "uniform sampler2D u_tex_noise;"
  "uniform sampler2D u_tex_u;"
  "in ivec2 a_tex;"
  "out vec3 v_pos;" 
  "out vec2 v_tex;"

  "const float size_y = " HEIGHT_FIELD_QUOTE(HEIGHT_FIELD_DRAW_WIDTH) ";"
  "const float size_x = " HEIGHT_FIELD_QUOTE(HEIGHT_FIELD_DRAW_HEIGHT) ";"
  "const float step_y = size_y / " HEIGHT_FIELD_QUOTE(HEIGHT_FIELD_N) ";"
  "const float step_x = size_x / " HEIGHT_FIELD_QUOTE(HEIGHT_FIELD_N) ";"
  "const float hx = size_x * 0.5;"
  "const float hy = size_y * 0.5;"
  "const float step_size = 2.0 * (1.0 / " HEIGHT_FIELD_QUOTE(HEIGHT_FIELD_N) ");"

  "void main() {"
  "  gl_Position = vec4(-1.0 + float(a_tex.s) * step_size, -1.0 + a_tex.t * step_size, 0.0, 1.0);"
  "  float current_height = texelFetch(u_tex_u, ivec2(a_tex.s + 0, a_tex.t + 0), 0).r;"
  "  v_tex = vec2(a_tex.s * (1.0 / " HEIGHT_FIELD_QUOTE(HEIGHT_FIELD_N) "), a_tex.t * (1.0 / " HEIGHT_FIELD_QUOTE(HEIGHT_FIELD_N) "));"
  "  float noise_height = texture(u_tex_noise, vec2(v_tex.s, v_tex.t + u_time)).r * 3.2;"
  "  v_pos = vec3(-hx + (a_tex.s + 0) * step_x, current_height + noise_height, -hy + a_tex.t * step_y + 0);"
  "}"
  "";

static const char* HF_POSITION_FS = ""
  "#version 150\n"
  "in vec3 v_pos;"
  "in vec2 v_tex;"
  "out vec4 pos_out;"
  "out vec4 tex_out;"
  
  "void main() {"
  "  pos_out = vec4(v_pos, 1.0);"
  "  tex_out = vec4(v_tex, 0.0, 1.0);"
  "}"
  "";

// Normals
// -----------------------------------------------------------
static const char* HF_NORMALS_VS = ""
  "#version 150\n"
  "uniform sampler2D u_tex_u;"
  "uniform sampler2D u_tex_pos;"
  "in ivec2 a_tex;"
  "out vec3 v_norm;"
  "out vec3 v_grad;"
  "const float size_y = " HEIGHT_FIELD_QUOTE(HEIGHT_FIELD_DRAW_WIDTH) ";"
  "const float size_x = " HEIGHT_FIELD_QUOTE(HEIGHT_FIELD_DRAW_HEIGHT) ";"
  "const float step_y = size_y / " HEIGHT_FIELD_QUOTE(HEIGHT_FIELD_N) ";"
  "const float step_x = size_x / " HEIGHT_FIELD_QUOTE(HEIGHT_FIELD_N) ";"
  "const float hx = size_x * 0.5;"
  "const float hy = size_y * 0.5;"
  "const float step_size = 2.0 * (1.0 / " HEIGHT_FIELD_QUOTE(HEIGHT_FIELD_N) ");"

  "float grid(int i, int j) { "
  "  return texelFetch(u_tex_u, ivec2(a_tex.s + i, a_tex.t + j), 0).r; "
  "}"

  "void main() {"
  "  gl_Position = vec4(-1.0 + float(a_tex.s) * step_size, -1.0 + a_tex.t * step_size, 0.0, 1.0);"

  "  vec3 current_pos     = texelFetch(u_tex_pos, ivec2(a_tex.s + 0, a_tex.t + 0), 0).rgb;"
  "  vec3 right_pos       = texelFetch(u_tex_pos, ivec2(a_tex.s + 1, a_tex.t + 0), 0).rgb;"
  "  vec3 top_pos         = texelFetch(u_tex_pos, ivec2(a_tex.s + 0, a_tex.t - 1), 0).rgb;"
  "  vec3 to_right        = right_pos - current_pos; "
  "  vec3 to_top          = top_pos - current_pos;"
  "  v_norm = normalize(cross(to_right, to_top));"
  //  "  v_norm = (cross(to_right, to_top));"

  "  float center = grid(0, 0);"
  "  float left   = grid(-1, 0);"
  "  float right  = grid(1, 0);"
  "  float top    = grid(0, -1);"
  "  float bottom = grid(0, 1);"
  //  "  vec3 diff = vec3( grid(1,0) - grid(0,0),  grid(0,1) - grid(0,0), 0.0);"
  //"  vec3 diff = vec3( right - center, bottom - center, 0.0);"
  "vec3 diff = vec3(right - left, top - bottom, 0.0);"
  // "  v_grad = 0.5 + 0.5 * normalize(diff);"
  "  v_grad = diff;"
  "}"
  "";

static const char* HF_NORMALS_FS = ""
  "#version 150\n"
  "in vec3 v_norm;"
  "in vec3 v_grad;"
  "out vec4 norm_out;"
  "out vec4 grad_out;"

  "void main() {"
  "  norm_out = vec4(v_norm, 1.0);"
  "  grad_out = vec4(0.5 + 0.5 * v_grad, 1.0);"
  "}"
  "";

// Renders the height field
// -----------------------------------------------------------

static const char* HF_RENDER_VS = ""
  "#version 150\n"
  "uniform sampler2D u_tex_u;"  
  "uniform sampler2D u_tex_norm;"
  "uniform mat4 u_pm;"
  "uniform mat4 u_vm;"

  "in ivec2 a_tex;"
  "out vec3 v_norm;"

  "const float size_y = " HEIGHT_FIELD_QUOTE(HEIGHT_FIELD_DRAW_WIDTH) ";"
  "const float size_x = " HEIGHT_FIELD_QUOTE(HEIGHT_FIELD_DRAW_HEIGHT) ";"
  "const float step_y = size_y / " HEIGHT_FIELD_QUOTE(HEIGHT_FIELD_N) ";"
  "const float step_x = size_x / " HEIGHT_FIELD_QUOTE(HEIGHT_FIELD_N) ";"
  "const float hx = size_x * 0.5;"
  "const float hy = size_y * 0.5;"
  
  "void main() {"
  "  float height = texelFetch(u_tex_u, a_tex, 0).r;"
  "  vec4 v = vec4(-hx + a_tex.s * step_x, height, -hy + a_tex.t * step_y, 1.0);"
  "  gl_Position = u_pm * u_vm * v;"
  "  v_norm = texelFetch(u_tex_norm, a_tex, 0).rgb;"
  "}"
  "";

static const char* HF_RENDER_FS = ""
  "#version 150\n"

  "in vec3 v_norm;"
  "out vec4 fragcolor;"
  
  "void main() {"
  "  fragcolor = vec4(1.0, 0.0, 0.0, 1.0);"
  "  fragcolor.rgb = 0.5 + 0.5 * v_norm;"
  "}"
  "";

// Debug drawing
// -----------------------------------------------------------

static const char* HF_TEXTURE_VS = "" // also used for the forces program
  "#version 150\n"
  "uniform mat4 u_pm;"
  "uniform mat4 u_mm;"

  "const vec2 verts[4] = vec2[]("
  "  vec2(-1.0,  1.0),   "
  "  vec2(-1.0, -1.0),   "
  "  vec2( 1.0,  1.0),   "
  "  vec2( 1.0, -1.0)    "
  ");"
  
  "const vec2 tex[4] = vec2[]("
  "  vec2(0.0, 1.0), " 
  "  vec2(0.0, 0.0), "
  "  vec2(1.0, 1.0), "
  "  vec2(1.0, 0.0)"
  ");"

  "out vec2 v_tex;"

  "void main() {"
  "  vec4 vert = vec4(verts[gl_VertexID], 0.0, 1.0);"
  "  gl_Position = u_pm * u_mm * vert;"
  "  v_tex = tex[gl_VertexID];"
  "}"
  "";

static const char* HF_DEBUG_FLOAT_FS = ""
  "#version 150\n"
  "uniform sampler2D u_tex;"
  "in vec2 v_tex;"
  "out vec4 fragcolor;"
  "void main() {"
  "  fragcolor.rgb = texture(u_tex, v_tex).rgb;"
  "  fragcolor.a = 0.0;"
  "}"
  "";

// Shader that is used to draw custom forces, these forces are added to the height field
// -------------------------------------------------------------------------------------

static const char* HF_FORCES_FS = ""
  "#version 150\n"
  "uniform sampler2D u_tex;"
  "in vec2 v_tex;"
  "out vec4 fragcolor;"
  "void main() {"
  "  fragcolor = vec4(texture(u_tex, v_tex).rg, 0.0, 1.0);"
  "}"
  "";


static const char* HF_FOAM_VS = ""
  "#version 150\n"
  "uniform sampler2D u_prev_u_tex;"
  "uniform sampler2D u_curr_u_tex;"
  "uniform sampler2D u_prev_foam_tex;"
  "in ivec2 a_tex;"
  "out float v_curr_u; "
  "out float v_prev_u; "
  "out float v_prev_foam;"
  "const float step_size = 2.0 * (1.0 / " HEIGHT_FIELD_QUOTE(HEIGHT_FIELD_N) ");"
  "void main() {"
  "  gl_Position = vec4(-1.0 + float(a_tex.s) * step_size, -1.0 + a_tex.t * step_size, 0.0, 1.0);"
  "  v_curr_u = texelFetch(u_prev_u_tex, a_tex, 0).r;"
  "  v_prev_foam = texelFetch(u_prev_foam_tex, a_tex, 0).r; "
  "  v_prev_u = texture(u_prev_u_tex, a_tex).r;"
  "}"
  "";

static const char* HF_FOAM_FS = ""
  "#version 150\n"
  "out vec4 foam_h;"
  "in float v_curr_u;"
  "in float v_prev_u;"
  "in float v_prev_foam;"
  "void main() {"
  "  float dh = max(v_curr_u - v_prev_u, 0.0);"
  // "  dh = pow(max(0, dh-0.1) * 2.0, 6.0);"
  "  foam_h = vec4((v_prev_foam * 0.97 + pow(dh, 4.0) ) , 0.0, 0.0, 1.0); " 
  "}"
  "";



struct FieldVertex {
  FieldVertex(){ tex[0] = 0; tex[1] = 0; }
  FieldVertex(int i, int j) { tex[0] = i; tex[1] = j; }
  void set(int i, int j) { tex[0] = i; tex[1] = j; }
  GLint tex[2];
};


class HeightField {

 public:
  HeightField();

  bool setup();
  void calculateHeights();          /* diffuse step */
  void calculatePositions();        /* after the new heights have been diffused we can update the position buffer */
  void calculateNormals();          /* after calling diffuse() you need to diffuse the normals */
  void calculateFoam();             /* after all above calculation are done, we do one more step that calculates the foam */
  void debugDraw();                 /* used while debugging; blits some buffers to screen */
  void drawTexture(GLuint tex, float x, float y, float w, float h);
  void render();

  void beginDrawForces();
  void drawForceTexture(GLuint tex, float px, float py, float pw, float ph);  // all in percentages 
  void endDrawForces();


 public:

  /* Shared */
  GLuint vao;                        /* generic VAO that is used to render FieldVertices */
  GLuint vbo_els;                    /* element array buffer */
  std::vector<GLint> indices;        /* indices to render triangles */

  /*  Program diffuse_prog; */

  /* Diffuse the height field */
  GLuint vert;                       /* vertex shader which performns the diffuse step */
  GLuint frag;                       /* fragment shader which writes/sets the diffused values + velocity */
  GLuint prog;                       /* program which does the diffuse/velocity updates */
  GLuint vbo;                        /* contains the FieldVertex data that is used to sample from the correct location in the shader */

  GLuint fbo0;                        /* we use a FBO with a couple of destination/source texture into which we write normals, u0, u1, velocities etc.. */
  GLuint tex_u0;                     /* contains the height values for state 0 */
  GLuint tex_u1;                     /* contains the height values for state 1 */
  GLuint tex_v0;                     /* contains the velocity values for state 0 */
  GLuint tex_v1;                     /* contains the velocity values for state 1 */
  GLuint tex_norm;                   /* contains the normals of the height field */
  //  GLuint tex_tang;               /* contains the tangents of the height field */
  GLuint tex_pos;                    /* contains the positions in world space of the vertices */
  GLuint tex_texcoord;               /* contains the texture coords in a range from 0-1 for the final render, see the position shader */
  GLuint tex_gradient;               /* contains the gradients for the current positions */
  GLuint tex_noise;                  /* we load a simple grayscale image with some perlin noise that is used to offset the height values a little bit.. based on time */
  int state_diffuse;                 /* toggles between 0 and 1 to ping/pong the read/write buffers */

  /* Foam */
  GLuint fbo1;
  GLuint tex_foam0;                    /* contains the foam intensity, ping ponged with tex_foam1 */
  GLuint tex_foam1;                    /* contains the foam intensity, ping ponged with tex_foam0 */
  int state_foam;                      /* toggles between 0 and 1 to ping/pong the read/write buffers for the foam */
  GLuint foam_prog;
  GLuint foam_frag;
  GLuint foam_vert;
                     
  /* Custom forces (testing) */
  GLuint fbo_forces;
  GLuint tex_forces;                 /* texture that is used to add custom forces; this works by drawing something into the force texture */
  GLuint frag_forces;                /* fragment shader for the step where we add custom forces, we use the vert_draw vertex shader */
  GLuint prog_forces;

  /* Debug drawing */                
  GLuint vao_draw;                   /* vao used to draw attribute less vertices for a texture */
  GLuint vert_draw;                  /* vertex shader to draw the u/v textures */
  GLuint frag_draw;                  /* fragment shader to draw the u/v textures */
  GLuint prog_draw;                  /* program to draw the u/v textures */

  /* Position (info) shader */
  GLuint vert_pos;                   /* vertex shader used to write the (world) positions of the grid */
  GLuint frag_pos;                   /* fragment shader to write the (world) positions */
  GLuint prog_pos;                   /* program to write positions (later maybe more info) */
                                       
  /* Normal shader */                
  GLuint vert_norm;                  /* vertex shader that calculates the normals */
  GLuint frag_norm;                  /* fragment shader that writes the normals */
  GLuint prog_norm;                  /* the program that writes the normals */
                                     
  /* Draw the height field */        
  GLuint prog_render;                /* program that is used to render/show the result of the height field */
  GLuint vert_render;                /* vertex shader that is used to render the height field */
  GLuint frag_render;                /* fragment shader that is used to render the height field */
                                     
  mat4 pm;                           /* projection matrix */
  mat4 vm;                           /* view matrix */
};

#endif
