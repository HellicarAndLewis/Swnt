#include <assert.h>
#include <math.h>
#include <sstream>
#include <string.h>
#include <swnt/Blur.h>
 
Blur::Blur()
  :fbo(0)
  ,depth(0)
  ,vao(0)
  ,prog0(0)
  ,prog1(0)
  ,vert(0)
  ,frag0(0)
  ,frag1(0)
  ,tex0(0)
  ,tex1(0)
  ,win_w(0)
  ,win_h(0)
  ,blur_amount(0)
  ,num_fetches(0)
{
}
 
Blur::~Blur() {
  shutdown();
}
 
bool Blur::setup(int winW, int winH, float blurAmount, int texFetches) {
  assert(winW);
  assert(winH);
  assert(blurAmount);
  assert(texFetches);
 
  win_w = winW;
  win_h = winH;
  blur_amount = blurAmount;
  num_fetches = texFetches;
 
  glGenVertexArrays(1, &vao);
 
  if(!setupFBO()) {
    shutdown();
    return false;
  }
 
  if(!setupShader()) {
    shutdown();
    return false;
  }
 
  return true;
}
 
bool Blur::setupFBO() { 
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
 
  glGenRenderbuffers(1, &depth);
  glBindRenderbuffer(GL_RENDERBUFFER, depth);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, win_w, win_h);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth);
 
  glGenTextures(1, &tex0);
  glBindTexture(GL_TEXTURE_2D, tex0);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, win_w, win_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex0, 0);
 
  glGenTextures(1, &tex1);
  glBindTexture(GL_TEXTURE_2D, tex1);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, win_w, win_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, tex1, 0);
 
  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if(status != GL_FRAMEBUFFER_COMPLETE) {
    printf("Framebuffer not complete");
    shutdown();
    return false;
  }
 
  return true;
}
 
bool Blur::setupShader() {
 
  // CREATE SHADER SOURCES
  // -----------------------------------------------------------------------
  int num_els = num_fetches;  /* kernel size */
  float* weights = new float[num_els]; 
  float sum = 0.0;
  float sigma2 = blur_amount;
 
  weights[0] = gauss(0,sigma2);
  sum = weights[0];
 
  for(int i = 1; i < num_els; i++) {
    weights[i] = gauss(i, sigma2);
    sum += 2 * weights[i];
  }
 
  for(int i = 0; i < num_els; ++i) {
    weights[i] = weights[i] / sum;
  }
 
  float dy = 1.0 / win_h;
  float dx = 1.0 / win_w;
 
  std::stringstream xblur;
  std::stringstream yblur;
  std::stringstream base_frag;
 
  base_frag << "#version 150\n"
            << "uniform sampler2D u_scene_tex;\n"
            << "in vec2 v_tex;\n"
            << "out vec4 fragcolor;\n";
 
  xblur << base_frag.str()
        << "void main() {\n"
        << "  fragcolor = texture(u_scene_tex, v_tex) * " << weights[0] << ";\n";
 
  yblur << base_frag.str()
        << "void main() {\n"
        << "  fragcolor = texture(u_scene_tex, v_tex) * " << weights[0] << ";\n";
 
  for(int i = 1 ; i < num_els; ++i) {
    yblur << "  fragcolor += texture(u_scene_tex, v_tex + vec2(0.0, " << float(i) * dy << ")) * " << weights[i] << ";\n" ;
    yblur << "  fragcolor += texture(u_scene_tex, v_tex - vec2(0.0, " << float(i) * dy << ")) * " << weights[i] << ";\n" ;
    xblur << "  fragcolor += texture(u_scene_tex, v_tex + vec2(" << float(i) * dx << ", 0.0)) * " << weights[i] << ";\n" ;
    xblur << "  fragcolor += texture(u_scene_tex, v_tex - vec2(" << float(i) * dx << ", 0.0)) * " << weights[i] << ";\n" ;
  }
 
  yblur << "}";
  xblur << "}";

  delete weights;
  weights = NULL;
 
  // -----------------------------------------------------------------------
  std::string yblur_s = yblur.str();
  std::string xblur_s = xblur.str();
  const char* yblur_vss = yblur_s.c_str();
  const char* xblur_vss = xblur_s.c_str();

  // y-blur
  vert = rx_create_shader(GL_VERTEX_SHADER, B_VS);
  frag0 = rx_create_shader(GL_FRAGMENT_SHADER, yblur_vss);
  prog0 = rx_create_program(vert, frag0);
  glLinkProgram(prog0);
  rx_print_shader_link_info(prog0);
 
  // x-blur
  frag1 = rx_create_shader(GL_FRAGMENT_SHADER, xblur_vss);
  prog1 = rx_create_program(vert, frag1);
  glLinkProgram(prog1);
  rx_print_shader_link_info(prog1);
 
  GLint u_scene_tex = 0;
 
  // set scene tex unit for 1st pass
  glUseProgram(prog0);
  u_scene_tex = glGetUniformLocation(prog0, "u_scene_tex");
  if(u_scene_tex < 0) {
    printf("Cannot find u_scene_tex (1)");
    shutdown();
    return false;
  }
  glUniform1i(u_scene_tex, 0);
 
  // set scene tex unit for 2nd pass
  glUseProgram(prog1);
  u_scene_tex = glGetUniformLocation(prog1, "u_scene_tex");
  if(u_scene_tex < 0) {
    printf("Cannot find u_scene_tex (2)");
    shutdown();
    return false;
  }
  glUniform1i(u_scene_tex, 0);
 
  return true;
}
 
 
void Blur::blur() {
  assert(fbo);
 
  // Blur the current read framebuffer into our first attachment (the source)
  GLenum draw_bufs0[] = { GL_COLOR_ATTACHMENT0 } ;
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDrawBuffers(1, draw_bufs0);
  glViewport(0,0,win_w, win_h);
  glBlitFramebuffer(0, 0, win_w, win_h, 0, 0, win_w, win_h, GL_COLOR_BUFFER_BIT, GL_LINEAR);
 
  glBindVertexArray(vao);
 
  {
    // vertical blur

    GLenum draw_bufs1[] = { GL_COLOR_ATTACHMENT1 } ;
    glDrawBuffers(1, draw_bufs1);
 
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex0);
 
    glUseProgram(prog0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  }
 
  {
    // horizontal blur
    GLenum draw_bufs1[] = { GL_COLOR_ATTACHMENT0 } ;
    glDrawBuffers(1, draw_bufs1);
 
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex1);
 
    glUseProgram(prog1);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  }
 
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}
 
void Blur::setAsReadBuffer() {
  assert(fbo);
 
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
  glReadBuffer(GL_COLOR_ATTACHMENT0);
}
 
float Blur::gauss(const float x, const float sigma2) {
  double coeff = 1.0 / (2.0 * PI * sigma2);
  double expon = -(x*x) / (2.0 * sigma2);
  return (float) (coeff*exp(expon));
}
 
void Blur::shutdown() {
  if(fbo) {
    glDeleteFramebuffers(1, &fbo);
  }
  fbo = 0;
 
  if(depth) {
    glDeleteRenderbuffers(1, &depth);
  }
  depth = 0;
 
  if(vao) {
    glDeleteVertexArrays(1, &vao);
  }
  vao = 0;
 
  if(prog0) {
    glDeleteProgram(prog0);
  }
  prog0 = 0;
 
  if(prog1) {
    glDeleteProgram(prog1);
  }
  prog1 = 0;
 
  if(vert) {
    glDeleteShader(vert);
  }
  vert = 0;
 
  if(frag0) {
    glDeleteShader(frag0);
  }
  frag0 = 0;
 
  if(frag1) {
    glDeleteShader(frag1);
  }
  frag1 = 0;
 
  if(tex0) {
    glDeleteTextures(1, &tex0);
  }
  tex0 = 0;
 
  if(tex1) {
    glDeleteTextures(1, &tex1);
  }
  tex1 = 0;
 
  win_w = 0;
  win_h = 0;
  blur_amount = 0.0f;
  num_fetches = 0;
}
