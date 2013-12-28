#include <assert.h>
#include <swnt/Water.h>
#include <swnt/HeightField.h>

Water::Water(HeightField& hf)
  :height_field(hf)
  ,win_w(0)
  ,win_h(0)
  ,prog(0)
  ,vert(0)
  ,frag(0)
  ,flow_tex(0)
  ,normals_tex(0)
  ,noise_tex(0)
  ,diffuse_tex(0)
  ,foam_tex(0)
  ,force_tex0(0)
{

}

GLuint Water::createTexture(std::string filename) {
 
  int w, h, n;
  unsigned char* pix;
 
  if(!rx_load_png(rx_to_data_path(filename), &pix, w, h, n)) {
    printf("Error: cannot find: %s\n", filename.c_str());
    return 0;
  }
 
  GLuint tex;
  GLenum format = GL_RGB;
 
  if(n == 4) {
    format = GL_RGBA;
  }
 
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, format, GL_UNSIGNED_BYTE, pix);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  printf("Texture, w: %d, h: %d, n: %d\n", w, h, n);
 
  delete[] pix;
  pix = NULL;
  return tex;
}


bool Water::setup(int w, int h) {
  assert(w && h);

  win_w = w;
  win_h = h;

  // create shader
  vert = rx_create_shader(GL_VERTEX_SHADER, WATER_VS);
  frag = rx_create_shader(GL_FRAGMENT_SHADER, WATER_FS);
  prog = rx_create_program(vert, frag);
  glBindAttribLocation(prog, 0, "a_tex");
  glLinkProgram(prog);
  rx_print_shader_link_info(prog);
  glUseProgram(prog);
  glUniform1i(glGetUniformLocation(prog, "u_tex_pos"),       0);  // VS
  glUniform1i(glGetUniformLocation(prog, "u_tex_norm"),      1);  // VS
  glUniform1i(glGetUniformLocation(prog, "u_tex_texcoord"),  2);  // VS
  glUniform1i(glGetUniformLocation(prog, "u_noise_tex"),     3);  // FS
  glUniform1i(glGetUniformLocation(prog, "u_norm_tex"),      4);  // FS
  glUniform1i(glGetUniformLocation(prog, "u_flow_tex"),      5);  // FS
  glUniform1i(glGetUniformLocation(prog, "u_diffuse_tex"),   6);  // FS
  glUniform1i(glGetUniformLocation(prog, "u_foam_tex"),      7);  // FS

  glUniformMatrix4fv(glGetUniformLocation(prog, "u_pm"), 1, GL_FALSE, height_field.pm.ptr());
  glUniformMatrix4fv(glGetUniformLocation(prog, "u_vm"), 1, GL_FALSE, height_field.vm.ptr());

  // load textures
  normals_tex = createTexture("images/water_normals.png");
  flow_tex = createTexture("images/water_flow.png");
  noise_tex = createTexture("images/water_noise.png");
  diffuse_tex = createTexture("images/water_diffuse.png");
  foam_tex = createTexture("images/water_foam.png");
  force_tex0 = createTexture("images/force.png");

  glBindTexture(GL_TEXTURE_2D, flow_tex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  return true;
}

void Water::update(float dt) {
}

void Water::draw() {

  glEnable(GL_DEPTH_TEST);

  glUseProgram(prog);

  {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, height_field.tex_pos);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, height_field.tex_norm);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, height_field.tex_texcoord);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, noise_tex);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, normals_tex);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, flow_tex);

    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, diffuse_tex);

    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, foam_tex);
  }

  static float t = 0.0;
  float time0 = fmod(t, 1.0);
  float time1 = fmod(t + 0.5, 1.0);
  t += 0.002;
  
  glUniform1f(glGetUniformLocation(prog, "u_time"), t);
  glUniform1f(glGetUniformLocation(prog, "u_time0"), time0);
  glUniform1f(glGetUniformLocation(prog, "u_time1"), time1);

  //  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindVertexArray(height_field.vao);
  glDrawElements(GL_TRIANGLES, height_field.indices.size(), GL_UNSIGNED_INT, NULL);
}
