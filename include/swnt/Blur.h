/*
 
  # Blur
 
  Simple blur shader which performs 2 passes; a vertical and horizontal blur.
  This class assumes you set the read framebuffer and then call blur(). We read
  the current GL_READ_FRAMEBUFFER into our intermediate scene texture and then 
  perfom 2 passes. This means we might have one extra blit which might be considered
  a performance issue; thouh on modern gpus you won't notice anything.
 
  To use the blurred result, call setAsReadBuffer() and either blit it to the
  default framebuffer or do something else with it.
 
*/
 
#ifndef ROXLU_BLUR_H
#define ROXLU_BLUR_H

#define ROXLU_USE_OPENGL
#include <tinylib.h>
 
static const char* B_VS = ""
  "#version 150\n"
 
  "const float w = 1.0;"
  "const float h = 1.0;"
 
  "const vec2 vertices[4] = vec2[]("
  "     vec2(-w, h), "
  "     vec2(-w, -h), "
  "     vec2( w,  h), "
  "     vec2( w, -h) "
  ");"
 
  "const vec2 texcoords[4] = vec2[] ("
  "     vec2(0.0, 0.0), "
  "     vec2(0.0, 1.0), "
  "     vec2(1.0, 0.0), "
  "     vec2(1.0, 1.0) "
  ");"
 
  "out vec2 v_tex;"
 
  "void main() {"
  "  gl_Position = vec4(vertices[gl_VertexID], 0.0, 1.0);"
  "  v_tex = texcoords[gl_VertexID]; "
  "}"
  "";
 
class Blur {
 public:
  Blur();
  ~Blur();
  bool setup(int winW, int winH, float blurAmount = 8.0f, int texFetches = 5);
  void blur();                   /* applies the blur */
  void setAsReadBuffer();        /* sets the result to the current read buffer */
 private:
  bool setupFBO();
  bool setupShader();
  float gauss(const float x, const float sigma2);
  void shutdown();               /* resets this class; destroys all allocated objects */
 public:
  GLuint fbo;                    /* the framebuffer for rtt */
  GLuint depth;                  /* depth buffer; not 100% we actually need one */
  GLuint vao;                    /* we use attribute-less rendering; but we need a vao as GL core 3 does not allow drawing with the default VAO */
  GLuint prog0;                  /* program for the vertical blur */
  GLuint prog1;                  /* program for the horizontal blur */
  GLuint vert;                   /* the above vertex shader */
  GLuint frag0;                  /* vertical fragment shader */
  GLuint frag1;                  /* horizontal fragment shader */
  GLuint tex0;                   /* first pass (scene capture) + combined blur */
  GLuint tex1;                   /* intermedia texture */
  int win_w;                     /* width of the window/fbo */
  int win_h;                     /* height of the window/fbo */
  float blur_amount;             /* the blur amount, 5-8 normal, 8+ heavy */
  int num_fetches;               /* how many texel fetches (half), the more the heavier for the gpu but more blur  */
};
 
#endif
