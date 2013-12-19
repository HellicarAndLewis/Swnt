#include <assert.h>
#include <swnt/Threshold.h>

Threshold::Threshold()
  :win_w(0)
  ,win_h(0)
  ,prog(0)
  ,vert(0)
  ,frag(0)
  ,input_tex(0)
  ,output_tex(0)
  ,fbo(0)
  ,vao(0)
{
}

bool Threshold::setup(int w, int h) {
  assert(w > 0 && h > 0);

  win_w = w;
  win_h = h;

  // create shader.
  vert = rx_create_shader(GL_VERTEX_SHADER, THRESHOLD_VS);
  frag = rx_create_shader(GL_FRAGMENT_SHADER, THRESHOLD_FS);
  prog = rx_create_program(vert, frag);
  glLinkProgram(prog);
  rx_print_shader_link_info(prog);
  glUseProgram(prog);
  glUniform1i(glGetUniformLocation(prog, "u_tex"), 0);

  glGenVertexArrays(1, &vao);

  // create fbo
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  glGenTextures(1, &input_tex);
  glBindTexture(GL_TEXTURE_2D, input_tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, win_w, win_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, input_tex, 0);

  glGenTextures(1, &output_tex);
  glBindTexture(GL_TEXTURE_2D, output_tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, win_w, win_h, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, output_tex, 0);

  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    printf("Error: FBO is not complete.\n");
    return false;
  }
  return true;
}

void Threshold::threshold() {

  // blit the currently bound GL_READ_FRAMEBUFFER into out attachment.
  GLenum bufs[] = { GL_COLOR_ATTACHMENT0 };
  glViewport(0.0f, 0.0f, win_w, win_h);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
  glDrawBuffers(1, bufs);
  glBlitFramebuffer(0, 0, win_w, win_h, 0, 0, win_w, win_h, GL_COLOR_BUFFER_BIT, GL_LINEAR);

  // apply the threshold
  bufs[0] = GL_COLOR_ATTACHMENT1;
  glDrawBuffers(1, bufs);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, input_tex);

  glUseProgram(prog);
  glBindVertexArray(vao);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Threshold::print() {
  printf("threshold.input_tex: %d\n", input_tex);
  printf("threshold.output_tex: %d\n", output_tex);
}
