#include <swnt/HeightField.h>

HeightField::HeightField() 
  :vert(0)
  ,frag(0)
  ,prog(0)
  ,vao(0)
  ,vbo_els(0)
  ,vbo(0)
  ,fbo0(0)
  ,tex_u0(0)
  ,tex_u1(0)
  ,tex_v0(0)
  ,tex_v1(0)
  ,tex_norm(0)
  ,tex_pos(0)
  ,tex_texcoord(0)
  ,tex_gradient(0)
  ,tex_forces(0)
  ,tex_noise(0)
  ,state_diffuse(0)
  ,fbo1(0)
  ,tex_foam0(0)
  ,tex_foam1(0)
  ,state_foam(0)
  ,fbo_forces(0)
  ,frag_forces(0)
  ,prog_forces(0)
  ,vao_draw(0)
  ,vert_draw(0)
  ,frag_draw(0)
  ,prog_draw(0)
  ,vert_pos(0)
  ,frag_pos(0)
  ,prog_pos(0)
  ,prog_render(0)
  ,vert_render(0)
  ,frag_render(0)
  ,vert_norm(0)
  ,frag_norm(0)
  ,prog_norm(0)
{
}

bool HeightField::setup() {
  
  pm.perspective(60.0f, float(HEIGHT_FIELD_W)/HEIGHT_FIELD_H, 0.01, 100.0);

#if 0
  vm.lookAt(vec3(0.0, 20.0, 0.0), vec3(0.0, 0.0, 0.1), vec3(0.0, 1.0, 0.0));
#else
  vm.rotateX(DEG_TO_RAD * 25);
  vm.translate(0.0, -10.0, -14.0);
  //vm.translate(0.0, -4.0, -24.0);
#endif

  // initial data
  float init_v[HEIGHT_FIELD_NN];
  float init_u[HEIGHT_FIELD_NN];
  int upper = HEIGHT_FIELD_N * 0.60;
  int lower = HEIGHT_FIELD_N * 0.30;
  for(int i = 0; i < HEIGHT_FIELD_N; ++i) {
    for(int j = 0; j < HEIGHT_FIELD_N; ++j) {
      int dx = j * HEIGHT_FIELD_N + i;
      init_v[dx] = 0.0f;
      init_u[dx] = 0.0f;

      if(i > lower && i < upper && j > lower && j < upper) {
        init_u[dx] = 0.0f;
        init_v[dx] = 0.0f;
      }
    }
  }
  
  // create diffuse shader
  vert = rx_create_shader(GL_VERTEX_SHADER, HF_DIFFUSE_VERT);
  frag = rx_create_shader(GL_FRAGMENT_SHADER, HF_DIFFUSE_FRAG);
  prog = rx_create_program(vert, frag);
  glBindAttribLocation(prog, 0, "a_tex"); // tex
  glBindFragDataLocation(prog, 0, "u_out"); // new velocity value
  glBindFragDataLocation(prog, 1, "v_out"); // new height value
  glLinkProgram(prog);
  rx_print_shader_link_info(prog);
  glUseProgram(prog);
  glUniform1i(glGetUniformLocation(prog, "u_tex_u"), 0);
  glUniform1i(glGetUniformLocation(prog, "u_tex_v"), 1);
  glUniform1i(glGetUniformLocation(prog, "u_tex_forces"), 2);

  // create draw shader for debugging
  vert_draw = rx_create_shader(GL_VERTEX_SHADER, HF_TEXTURE_VS);
  frag_draw = rx_create_shader(GL_FRAGMENT_SHADER, HF_DEBUG_FLOAT_FS);
  prog_draw = rx_create_program(vert_draw, frag_draw);
  glLinkProgram(prog_draw);
  rx_print_shader_link_info(prog_draw);
  glUseProgram(prog_draw);
  glUniform1i(glGetUniformLocation(prog_draw, "u_tex"), 0);

  // create render shader for final result.
  vert_render = rx_create_shader(GL_VERTEX_SHADER, HF_RENDER_VS);
  frag_render = rx_create_shader(GL_FRAGMENT_SHADER, HF_RENDER_FS);
  prog_render = rx_create_program(vert_render, frag_render);
  glBindAttribLocation(prog_render, 0, "a_tex");
  glLinkProgram(prog_render);
  rx_print_shader_link_info(prog_render);
  glUseProgram(prog_render);
  glUniformMatrix4fv(glGetUniformLocation(prog_render, "u_pm"), 1, GL_FALSE, pm.ptr());
  glUniformMatrix4fv(glGetUniformLocation(prog_render, "u_vm"), 1, GL_FALSE, vm.ptr());
  glUniform1i(glGetUniformLocation(prog_render, "u_tex_u"), 0);
  glUniform1i(glGetUniformLocation(prog_render, "u_tex_norm"), 1);

  // create normal shader
  vert_norm = rx_create_shader(GL_VERTEX_SHADER, HF_NORMALS_VS);
  frag_norm = rx_create_shader(GL_FRAGMENT_SHADER, HF_NORMALS_FS);
  prog_norm = rx_create_program(vert_norm, frag_norm);
  glBindAttribLocation(prog_norm, 0, "a_tex");
  glBindFragDataLocation(prog_norm, 0, "norm_out");
  glBindFragDataLocation(prog_norm, 1, "grad_out");
  // glBindFragDataLocation(prog_norm, 1, "tang_out");
  glLinkProgram(prog_norm);
  rx_print_shader_link_info(prog_norm);
  glUseProgram(prog_norm);
  glUniform1i(glGetUniformLocation(prog_norm, "u_tex_u"), 0);
  glUniform1i(glGetUniformLocation(prog_norm, "u_tex_pos"), 1);

  // create position/texcoord shader
  vert_pos = rx_create_shader(GL_VERTEX_SHADER, HF_POSITION_VS);
  frag_pos = rx_create_shader(GL_FRAGMENT_SHADER, HF_POSITION_FS);
  prog_pos = rx_create_program(vert_pos, frag_pos);
  glBindFragDataLocation(prog_pos, 0, "pos_out");
  glBindFragDataLocation(prog_pos, 1, "tex_out");
  glBindAttribLocation(prog_pos, 0, "a_pos");
  glLinkProgram(prog_pos);
  rx_print_shader_link_info(prog_pos);
  glUseProgram(prog_pos);
  glUniform1i(glGetUniformLocation(prog_pos, "u_tex_u"), 0);
  glUniform1i(glGetUniformLocation(prog_pos, "u_tex_noise"), 1);

  // create shader that is used to draw "force textures"
  frag_forces = rx_create_shader(GL_FRAGMENT_SHADER, HF_FORCES_FS);
  prog_forces = rx_create_program(vert_draw, frag_forces);
  glLinkProgram(prog_forces);
  rx_print_shader_link_info(prog_forces);
  glUseProgram(prog_forces);
  glUniform1i(glGetUniformLocation(prog_forces, "u_tex"), 0);

  glGenVertexArrays(1, &vao_draw);
  
  // create textures that will hold the height field
  glGenTextures(1, &tex_u0);
  glBindTexture(GL_TEXTURE_2D, tex_u0);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, HEIGHT_FIELD_N, HEIGHT_FIELD_N, 0,  GL_RED, GL_FLOAT, init_u);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glGenTextures(1, &tex_u1);
  glBindTexture(GL_TEXTURE_2D, tex_u1);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, HEIGHT_FIELD_N, HEIGHT_FIELD_N, 0,  GL_RED, GL_FLOAT, init_u);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  float forces[HEIGHT_FIELD_NN * 2];
  memset(forces, 0, sizeof(forces));
  glGenTextures(1, &tex_forces);
  glBindTexture(GL_TEXTURE_2D, tex_forces);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, HEIGHT_FIELD_N, HEIGHT_FIELD_N, 0,  GL_RG, GL_FLOAT, forces);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glGenTextures(1, &tex_v0);
  glBindTexture(GL_TEXTURE_2D, tex_v0);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, HEIGHT_FIELD_N, HEIGHT_FIELD_N, 0,  GL_RED, GL_FLOAT, init_v);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glGenTextures(1, &tex_v1);
  glBindTexture(GL_TEXTURE_2D, tex_v1);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, HEIGHT_FIELD_N, HEIGHT_FIELD_N, 0,  GL_RED, GL_FLOAT, init_v);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glGenTextures(1, &tex_norm);
  glBindTexture(GL_TEXTURE_2D, tex_norm);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, HEIGHT_FIELD_N, HEIGHT_FIELD_N, 0, GL_RGB, GL_FLOAT, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glGenTextures(1, &tex_pos);
  glBindTexture(GL_TEXTURE_2D, tex_pos);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, HEIGHT_FIELD_N, HEIGHT_FIELD_N, 0, GL_RGB, GL_FLOAT, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glGenTextures(1, &tex_texcoord);
  glBindTexture(GL_TEXTURE_2D, tex_texcoord);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, HEIGHT_FIELD_N, HEIGHT_FIELD_N, 0, GL_RG, GL_FLOAT, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glGenTextures(1, &tex_gradient);
  glBindTexture(GL_TEXTURE_2D, tex_gradient);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, HEIGHT_FIELD_N, HEIGHT_FIELD_N, 0, GL_RGB, GL_FLOAT, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


  // fbo0 for the diffuse math, normals, positions, etc..
  glGenFramebuffers(1, &fbo0);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_u0,       0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, tex_u1,       0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, tex_v0,       0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, tex_v1,       0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, tex_norm,     0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT5, GL_TEXTURE_2D, tex_pos,      0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT6, GL_TEXTURE_2D, tex_texcoord, 0);
  //  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT7, GL_TEXTURE_2D, tex_tang,     0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT7, GL_TEXTURE_2D, tex_gradient, 0);
                
  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    printf("Error: framebuffer is not complete.\n");
    ::exit(EXIT_FAILURE);
  }

  // custom forces (fbo0 + tex)
  glGenFramebuffers(1, &fbo_forces);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo_forces);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_forces, 0);
  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    printf("Framebuffer for custom forces step not complete.\n");
    ::exit(EXIT_FAILURE);
  }


  // +++++++++++++++++++ FOAM +++++++++++++++++++++++++++++++++++++++
  glGenFramebuffers(1, &fbo1);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo1);

  glGenTextures(1, &tex_foam0);
  glBindTexture(GL_TEXTURE_2D, tex_foam0);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, HEIGHT_FIELD_N, HEIGHT_FIELD_N, 0, GL_RED, GL_FLOAT, init_u);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_foam0, 0);

  glGenTextures(1, &tex_foam1);
  glBindTexture(GL_TEXTURE_2D, tex_foam1); 
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, HEIGHT_FIELD_N, HEIGHT_FIELD_N, 0, GL_RED, GL_FLOAT, init_u);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, tex_foam1, 0);
  
  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    printf("Error: the foam framebuffer texture is not complete.\n");
    ::exit(EXIT_FAILURE);
  }

  const char* attr[] = { "a_tex" };
  foam_vert = rx_create_shader(GL_VERTEX_SHADER, HF_FOAM_VS);
  foam_frag = rx_create_shader(GL_FRAGMENT_SHADER, HF_FOAM_FS);
  foam_prog = rx_create_program_with_attribs(foam_vert, foam_frag, 1, attr);
  glUseProgram(foam_prog);
  glUniform1i(glGetUniformLocation(foam_prog, "u_prev_u_tex"), 0); // texture which hold the previous height fields
  glUniform1i(glGetUniformLocation(foam_prog, "u_curr_u_tex"), 1); // current height fields (curr - prev => delta)
  glUniform1i(glGetUniformLocation(foam_prog, "u_prev_foam_tex"), 2); // previous foam, to which we add the new foam
  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // create buffer
  std::vector<FieldVertex> field_vertices(HEIGHT_FIELD_NN, FieldVertex());
  for(int i = 0; i < HEIGHT_FIELD_N; ++i) {
    for(int j = 0; j < HEIGHT_FIELD_N; ++j) {
      int dx = j * HEIGHT_FIELD_N + i;
      field_vertices[dx].set(i, j);
    }
  }

  // triangle indices
  for(int i = 1; i < HEIGHT_FIELD_N - 1; ++i) {
    for(int j = 1; j < HEIGHT_FIELD_N - 1; ++j) {
      int a = (j + 0) * HEIGHT_FIELD_N + (i + 0); // bottom left
      int b = (j + 0) * HEIGHT_FIELD_N + (i + 1); // bottom right
      int c = (j + 1) * HEIGHT_FIELD_N + (i + 1); // top right
      int d = (j + 1) * HEIGHT_FIELD_N + (i + 0); // top left
      indices.push_back(a);
      indices.push_back(b);
      indices.push_back(c);

      indices.push_back(a);
      indices.push_back(c);
      indices.push_back(d);
    }
  }

  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(FieldVertex) * field_vertices.size(), &field_vertices[0].tex[0], GL_STATIC_DRAW);
  glEnableVertexAttribArray(0); // tex
  glVertexAttribIPointer(0, 2, GL_INT, sizeof(FieldVertex), (GLvoid*)0); // tex

  glGenBuffers(1, &vbo_els);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_els);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLint) * indices.size(), &indices[0], GL_STATIC_DRAW);

  tex_noise = rx_create_texture(rx_to_data_path("images/water_noise.png")); 

  printf("height_field.tex_u0: %d\n", tex_u0);
  printf("height_field.tex_u1: %d\n", tex_u1);
  printf("height_field.tex_v0: %d\n", tex_v0);
  printf("height_field.tex_v1: %d\n", tex_v1);
  printf("height_field.tex_norm: %d\n", tex_norm);
  printf("height_field.tex_noise: %d\n", tex_noise);
  printf("height_field.tex_gradient: %d\n", tex_gradient);
  printf("height_field.tex_foam0: %d\n", tex_foam0);
  printf("height_field.tex_foam1: %d\n", tex_foam1);
  //  printf("height_field.tex_tang: %d\n", tex_tang);
  
  return true;
}


void HeightField::drawTexture(GLuint tex, float x, float y, float w, float h) {

  mat4 lpm;
  mat4 lmm;
  float hw = w * 0.5;
  float hh = h * 0.5;
  lpm.ortho(0, HEIGHT_FIELD_W, HEIGHT_FIELD_H, 0, 0.0f, 100.0f);
  lmm.translate(x + hw, y + hh, 0.0f);
  lmm.scale(hw, hh, 1.0);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex);
  
  glBindVertexArray(vao_draw);
  glUseProgram(prog_draw);
  glUniformMatrix4fv(glGetUniformLocation(prog_draw, "u_pm"), 1, GL_FALSE, lpm.ptr());
  glUniformMatrix4fv(glGetUniformLocation(prog_draw, "u_mm"), 1, GL_FALSE, lmm.ptr());
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

}


// Renders the calculated heights/normls/etc.. to screens
void HeightField::render() {
  
  glUseProgram(prog_render);

  glEnable(GL_DEPTH_TEST);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex_u0);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, tex_norm);

  glBindVertexArray(vao);

  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, NULL);
  
}

// Diffuses the heights
void HeightField::calculateHeights() {
 
  glBindFramebuffer(GL_FRAMEBUFFER, fbo0);
  glViewport(0, 0, HEIGHT_FIELD_N, HEIGHT_FIELD_N);
  glUseProgram(prog);
  glBindVertexArray(vao);
  glDisable(GL_BLEND);

  state_diffuse = 1 - state_diffuse;
  
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, tex_forces);

  if(state_diffuse == 0) {
    // read from u0, write to u1
    // read from v0, write to v1
    GLenum draw_bufs[] = { GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT3 } ;
    glDrawBuffers(2, draw_bufs);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_u0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, tex_v0);
  }
  else {
    // read from u1, write to u0
    // read from v1, write to v0
    GLenum draw_bufs[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT2 } ;
    glDrawBuffers(2, draw_bufs);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_u1);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, tex_v1);
  }

  glDrawArrays(GL_POINTS, 0, HEIGHT_FIELD_NN);

  // clear the forces buffer.
  GLenum forces_bufs[] = { GL_COLOR_ATTACHMENT0 } ;
  glBindFramebuffer(GL_FRAMEBUFFER, fbo_forces);
  glDrawBuffers(1, forces_bufs);
  glClear(GL_COLOR_BUFFER_BIT);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, HEIGHT_FIELD_W, HEIGHT_FIELD_H);
  
}

void HeightField::calculatePositions() {
  
  static float t = 0.0;

  glViewport(0, 0, HEIGHT_FIELD_N, HEIGHT_FIELD_N);

  GLenum drawbufs[] = { GL_COLOR_ATTACHMENT5, GL_COLOR_ATTACHMENT6 } ;
  glBindFramebuffer(GL_FRAMEBUFFER, fbo0);
  glDrawBuffers(2, drawbufs);

  glUseProgram(prog_pos);
  glBindVertexArray(vao);

  glUniform1f(glGetUniformLocation(prog_pos, "u_time"), t);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex_u0);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, tex_noise);

  glDrawArrays(GL_POINTS, 0, HEIGHT_FIELD_NN);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, HEIGHT_FIELD_W, HEIGHT_FIELD_H);
  
  t += 0.001;
  
}

// This uses the current height values to produce a texture that will contain the normals
void HeightField::calculateNormals() {
 
  GLenum drawbufs[] = { GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT7 } ;

  glViewport(0, 0, HEIGHT_FIELD_N, HEIGHT_FIELD_N);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo0);
  glDrawBuffers(2, drawbufs);

  glUseProgram(prog_norm);
  glBindVertexArray(vao);
  
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex_u0);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, tex_pos);

  glDrawArrays(GL_POINTS, 0, HEIGHT_FIELD_NN);
  
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, HEIGHT_FIELD_W, HEIGHT_FIELD_H);
  
}

void HeightField::calculateFoam() {
 
  // GLenum drawbufs[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 } ;
  
  glBindFramebuffer(GL_FRAMEBUFFER, fbo1);
  glViewport(0, 0, HEIGHT_FIELD_N, HEIGHT_FIELD_N);
  glUseProgram(foam_prog);
  glBindVertexArray(vao);
  glDisable(GL_BLEND);
  
  if(state_foam == 0) {
    GLenum drawbufs[] = { GL_COLOR_ATTACHMENT0 } ;
    glDrawBuffers(1, drawbufs);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, tex_foam1);
  }
  else {
    GLenum drawbufs[] = { GL_COLOR_ATTACHMENT1 } ;
    glDrawBuffers(1, drawbufs);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, tex_foam0);
  }

  if(state_diffuse == 1) {
    glActiveTexture(GL_TEXTURE0);  // prev
    glBindTexture(GL_TEXTURE_2D, tex_u0);
    glActiveTexture(GL_TEXTURE1); // curr
    glBindTexture(GL_TEXTURE_2D, tex_u1);
  }
  else {
    glActiveTexture(GL_TEXTURE0);  // prev
    glBindTexture(GL_TEXTURE_2D, tex_u1);
    glActiveTexture(GL_TEXTURE1); // curr
    glBindTexture(GL_TEXTURE_2D, tex_u0);
  }

  //  glClear(GL_COLOR_BUFFER_BIT);
  glDrawArrays(GL_POINTS, 0, HEIGHT_FIELD_NN);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, HEIGHT_FIELD_W, HEIGHT_FIELD_H);

  state_foam = 1 - state_foam;
  
}


void HeightField::beginDrawForces() {
  
  GLenum drawbufs[] = { GL_COLOR_ATTACHMENT0 } ;
  glBindFramebuffer(GL_FRAMEBUFFER, fbo_forces);
  glDrawBuffers(1, drawbufs);
  glClear(GL_COLOR_BUFFER_BIT);
  glViewport(0, 0, HEIGHT_FIELD_N, HEIGHT_FIELD_N);
  
}

void HeightField::endDrawForces() {

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, HEIGHT_FIELD_W, HEIGHT_FIELD_H);

}

void HeightField::drawForceTexture(GLuint tex, float px, float py, float pw, float ph) {

  mat4 lpm;
  mat4 lmm;
  float hw = 0.5 * HEIGHT_FIELD_N * pw;
  float hh = 0.5 * HEIGHT_FIELD_N * ph;

#if 0
  lpm.ortho(0, HEIGHT_FIELD_W, HEIGHT_FIELD_H, 0, 0.0f, 100.0f);
  lmm.translate( (px * HEIGHT_FIELD_N) + hw, (py * HEIGHT_FIELD_N) + hh, 0.0f);
  lmm.scale(hw, hh, 1.0);
#else
  lpm.ortho(0, HEIGHT_FIELD_N, HEIGHT_FIELD_N, 0, 0.0f, 100.0f);
  lmm.translate(px * HEIGHT_FIELD_N, py * HEIGHT_FIELD_N, 0.0f);
  lmm.scale(pw * HEIGHT_FIELD_N, ph * HEIGHT_FIELD_N, 1.0);
#endif


  // glBindFramebuffer(GL_FRAMEBUFFER, 0);
  // glViewport(0, 0, N, N);
  glUseProgram(prog_forces);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex);

  glBindVertexArray(vao_draw);
  glUniformMatrix4fv(glGetUniformLocation(prog_forces, "u_pm"), 1, GL_FALSE, lpm.ptr());
  glUniformMatrix4fv(glGetUniformLocation(prog_forces, "u_mm"), 1, GL_FALSE, lmm.ptr());

  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  //glViewport(0, 0, W, H);

}


void HeightField::debugDraw() {

  // Draw the height field delta 
  glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo1);
  glReadBuffer(GL_COLOR_ATTACHMENT1);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  glBlitFramebuffer(0, 0, HEIGHT_FIELD_N, HEIGHT_FIELD_N, 0, 0, HEIGHT_FIELD_N, HEIGHT_FIELD_N, GL_COLOR_BUFFER_BIT, GL_LINEAR);

}
