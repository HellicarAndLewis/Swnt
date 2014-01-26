#include <assert.h>
#include <stdio.h>
#include <swnt/RGBShift.h>

RGBShift::RGBShift() 
  :fbo(0)
  ,depth(0)
  ,tex_scene(0)
  ,vert(0)
  ,frag(0)
  ,prog(0)
  ,vao(0)
{
}

RGBShift::~RGBShift() {
  printf("Error: needs to deallocate the GL objects created in RGBShift.\n");
}

bool RGBShift::setup() {

  if(!createFBO()) {
    return false;
  }

  if(!createShader()) {
    return false;
  }

  glGenVertexArrays(1, &vao);;
  return true;
}

bool RGBShift::createFBO() {

  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  int w = 1024;
  int h = 768;

  // depth.
  glGenRenderbuffers(1, &depth);
  glBindRenderbuffer(GL_RENDERBUFFER, depth);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, w, h);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth);

  // scene tex
  glGenTextures(1, &tex_scene);
  glBindTexture(GL_TEXTURE_2D, tex_scene);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_scene, 0);

  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if(status != GL_FRAMEBUFFER_COMPLETE) {
    printf("Error: framebuffer not complete in RGBShift.\n");
    ::exit(EXIT_FAILURE);
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindTexture(GL_TEXTURE_2D, 0);
  return true;
}

bool RGBShift::createShader() {
  vert = rx_create_shader(GL_VERTEX_SHADER, RGBSHIFT_VS);
  frag = rx_create_shader(GL_FRAGMENT_SHADER, RGBSHIFT_FS);

  prog = rx_create_program(vert, frag);
  glLinkProgram(prog);
  return true;
}

void RGBShift::apply() {
  int w = 1024;
  int h = 768;
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
  GLenum db[] = { GL_COLOR_ATTACHMENT0 } ;
  glDrawBuffers(1, db);

  glBlitFramebuffer(0,0, w, h,
                    0,0, w, h,
                    GL_COLOR_BUFFER_BIT, GL_LINEAR);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RGBShift::draw() {
  glUseProgram(prog);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex_scene);
  glUniform1i(glGetUniformLocation(prog, "u_tex"), 0);
  
  glBindVertexArray(vao);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
