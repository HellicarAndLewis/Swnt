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
  ,color_tex(0)
  ,max_depth(9.5)
  ,sun_shininess(4.0)
  ,foam_depth(2.4)
{
  sun_pos[0] = 0.0;
  sun_pos[1] = 15.0;
  sun_pos[2] = -300.0;
  sun_color[0] = 4;
  sun_color[1] = 3;
  sun_color[2] = 1;
  ads_intensities[0] = 0.1; // ambient
  ads_intensities[1] = 0.4; // diffuse
  ads_intensities[2] = 0.4; // spec
  ads_intensities[3] = 1.0; // sun  
  ads_intensities[4] = 1.0; // foam
  ads_intensities[5] = 0.3; // texture
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
  glUniform1i(glGetUniformLocation(prog, "u_tex_tang"),      3);  // VS
  glUniform1i(glGetUniformLocation(prog, "u_noise_tex"),     4);  // FS
  glUniform1i(glGetUniformLocation(prog, "u_norm_tex"),      5);  // FS
  glUniform1i(glGetUniformLocation(prog, "u_flow_tex"),      6);  // FS
  glUniform1i(glGetUniformLocation(prog, "u_diffuse_tex"),   7);  // FS
  glUniform1i(glGetUniformLocation(prog, "u_foam_tex"),      8);  // FS
  glUniform1i(glGetUniformLocation(prog, "u_color_tex"),     9);  // FS

  glUniformMatrix4fv(glGetUniformLocation(prog, "u_pm"), 1, GL_FALSE, height_field.pm.ptr());
  glUniformMatrix4fv(glGetUniformLocation(prog, "u_vm"), 1, GL_FALSE, height_field.vm.ptr());

  // load textures
  normals_tex = createTexture("images/water_normals.png");
  flow_tex = createTexture("images/water_flow.png");
  noise_tex = createTexture("images/water_noise.png");
  diffuse_tex = createTexture("images/water_diffuse.png");
  foam_tex = createTexture("images/water_foam.png");
  force_tex0 = createTexture("images/force.png");

  // load color ramp
  unsigned char* img_pix = NULL;
  int img_w, img_h,img_channels = 0;
  if(!rx_load_png(rx_to_data_path("images/water_color.png"), &img_pix, img_w, img_h, img_channels)) {
    printf("Error: cannot load the water_color.png image.\n");
    return false;
  }

  glGenTextures(1, &color_tex);
  glBindTexture(GL_TEXTURE_1D, color_tex);
  glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, img_w, 0, GL_RGB, GL_UNSIGNED_BYTE, img_pix);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  delete[] img_pix;
  printf("water.color_tex: %d\n", color_tex);
  

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
    glBindTexture(GL_TEXTURE_2D, height_field.tex_tang);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, noise_tex);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, normals_tex);

    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, flow_tex);

    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, diffuse_tex);

    glActiveTexture(GL_TEXTURE8);
    glBindTexture(GL_TEXTURE_2D, foam_tex);

    glActiveTexture(GL_TEXTURE9);
    glBindTexture(GL_TEXTURE_1D, color_tex);
  }

  static float t = 0.0;
  float time0 = fmod(t, 1.0);
  float time1 = fmod(t + 0.5, 1.0);
  t += 0.002;
  
  glUniform1f(glGetUniformLocation(prog, "u_time"), t);
  glUniform1f(glGetUniformLocation(prog, "u_time0"), time0);
  glUniform1f(glGetUniformLocation(prog, "u_time1"), time1);
  glUniform3fv(glGetUniformLocation(prog, "u_sun_pos"), 1, sun_pos);
  glUniform3fv(glGetUniformLocation(prog, "u_sun_color"), 1, sun_color);
  glUniform1f(glGetUniformLocation(prog, "u_max_depth"), max_depth);
  glUniform1f(glGetUniformLocation(prog, "u_foam_depth"), foam_depth);
  glUniform1f(glGetUniformLocation(prog, "u_sun_shininess"), sun_shininess);
  glUniform1fv(glGetUniformLocation(prog, "u_ads_intensities"), 6, ads_intensities);
  glUniform3fv(glGetUniformLocation(prog, "u_ambient_color"), 1, ambient_color);

  //  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindVertexArray(height_field.vao);
  glDrawElements(GL_TRIANGLES, height_field.indices.size(), GL_UNSIGNED_INT, NULL);
}
