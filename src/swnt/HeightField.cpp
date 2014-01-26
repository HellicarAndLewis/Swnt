#include <swnt/HeightField.h>

HeightField::HeightField() 
  :field_fbo(0)
  ,tex_u0(0)
  ,tex_u1(0)
  ,tex_v0(0)
  ,tex_v1(0)
  ,tex_noise(0)
  ,field_size(128)
  ,field_vao(0)
  ,state_diffuse(0)
  ,win_w(0)
  ,win_h(0)
  ,force_fbo(0)
  ,force_tex(0)
  ,force_max(1.5f)
  ,u_force(0)
  ,process_fbo(0)
  ,tex_out_norm(0)
  ,tex_out_pos(0)
  ,tex_out_texcoord(0)
  ,vertices_vbo(0)
  ,vertices_vao(0)
{
}

bool HeightField::setup(int w, int h) {

  if(!w || !h) {
    printf("Error: invalid width/height: %d x %d\n", w, h);
    return false;
  }

  win_w = w;
  win_h = h;

  glGenVertexArrays(1, &field_vao);

  if(!setupDiffusing()) {
    printf("Error: cannot set GL state for the diffuse step.\n");
    return false;
  }

  if(!setupProcessing()) {
    printf("Error: cannot setup the GL state for the processing.\n");
    return false;
  }
  
  if(!setupVertices()) {
    printf("Error: cannot setup the vertices for the height field.\n");
    return false;
  }

  if(!setupDebug()) {
    printf("Error: cannot setup the GL state for debugging.\n");
    return false;
  }

  if(!setupExtraForces()) {
    printf("Error: cannot setup the extra forces.\n");
    return false;
  }

  return true;
}

bool HeightField::setupExtraForces() {

  // Shader
  force_prog.create(GL_VERTEX_SHADER, rx_to_data_path("shaders/height_field_forces.vert"));
  force_prog.create(GL_FRAGMENT_SHADER, rx_to_data_path("shaders/height_field_forces.frag"));
  force_prog.link();
  glUseProgram(force_prog.id);
  glUniform1i(glGetUniformLocation(force_prog.id, "u_tex"), 0);

  mat4 pm;
  pm.ortho(0, field_size, field_size, 0, 0.0f, 100.0);
  glUniformMatrix4fv(glGetUniformLocation(force_prog.id, "u_pm"), 1, GL_FALSE, pm.ptr());

  // Texture
  float* forces = new float[field_size * field_size * 2];
  memset((char*)forces, 0, sizeof(float) * field_size * field_size * 2);

  glGenTextures(1, &force_tex);
  glBindTexture(GL_TEXTURE_2D, force_tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, field_size, field_size, 0,  GL_RG, GL_FLOAT, forces);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  // FBO 
  glGenFramebuffers(1, &force_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, force_fbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, force_tex, 0);

  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    printf("Framebuffer for custom forces step not complete.\n");
    ::exit(EXIT_FAILURE);
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  delete[] forces;
  forces = NULL;
  
  return true;
}

bool HeightField::setupVertices() {
  glGenVertexArrays(1, &vertices_vao);
  glBindVertexArray(vertices_vao);

  glGenBuffers(1, &vertices_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vertices_vbo);

  std::vector<HeightFieldVertex> tmp(field_size * field_size, HeightFieldVertex());
  for(int j = 0; j < field_size; ++j) {
    for(int i = 0; i < field_size; ++i) {
      int dx = j * field_size + i;
      tmp[dx].tex.set(i, j);
    }
  }

  for(int j = 0; j < field_size-1; ++j) {
    for(int i = 0; i < field_size-1; ++i) {
      int a = (j + 0) * field_size + (i + 0);
      int b = (j + 0) * field_size + (i + 1);
      int c = (j + 1) * field_size + (i + 1);
      int d = (j + 1) * field_size + (i + 0);
      vertices.push_back(tmp[a]);
      vertices.push_back(tmp[b]);
      vertices.push_back(tmp[c]);

      vertices.push_back(tmp[a]);
      vertices.push_back(tmp[c]);
      vertices.push_back(tmp[d]);
    }
  }
    
  glBufferData(GL_ARRAY_BUFFER, sizeof(HeightFieldVertex) * vertices.size(), vertices[0].tex.ptr(), GL_STATIC_DRAW);

  glEnableVertexAttribArray(0); // tex
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(HeightFieldVertex), (GLvoid*)0);
  return true;
}

bool HeightField::setupDebug() {

  pm.perspective(60.0f, float(win_w)/win_h, 0.01f, 100.0f);
#if 1
  vm.lookAt(vec3(0.0f, 20.0f, 0.0f), vec3(0.0f, 0.0f, 0.1f), vec3(0.0f, 1.0f, 0.0f));
#else
  vm.translate(0.0f, 0.0f, -30.0f);
  vm.rotateX(30 * DEG_TO_RAD);
#endif

  const char* atts[] = { "a_tex" };
  debug_prog.create(GL_VERTEX_SHADER, rx_to_data_path("shaders/height_field_debug.vert"));
  debug_prog.create(GL_FRAGMENT_SHADER, rx_to_data_path("shaders/height_field_debug.frag"));
  debug_prog.link(1, atts);

  glUseProgram(debug_prog.id);
  glUniformMatrix4fv(glGetUniformLocation(debug_prog.id, "u_pm"), 1, GL_FALSE, pm.ptr());
  glUniformMatrix4fv(glGetUniformLocation(debug_prog.id, "u_vm"), 1, GL_FALSE, vm.ptr());
  glUniform1i(glGetUniformLocation(debug_prog.id, "u_pos_tex"), 0);

  return true;
}

bool HeightField::setupProcessing() {

  glGenTextures(1, &tex_out_norm);
  glBindTexture(GL_TEXTURE_2D, tex_out_norm);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, field_size, field_size, 0, GL_RGB, GL_FLOAT, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glGenTextures(1, &tex_out_pos);
  glBindTexture(GL_TEXTURE_2D, tex_out_pos);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F,field_size, field_size, 0, GL_RGB, GL_FLOAT, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glGenTextures(1, &tex_out_texcoord);
  glBindTexture(GL_TEXTURE_2D, tex_out_texcoord);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, field_size, field_size, 0, GL_RG, GL_FLOAT, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glGenFramebuffers(1, &process_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, process_fbo);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_out_pos, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, tex_out_norm, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, tex_out_texcoord, 0);

  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    printf("Error: process framebuffer not complete.\n");
    return false;
  }

  // Position processing
  const char* pos_frags[] = { "out_pos" };
  pos_prog.create(GL_VERTEX_SHADER, rx_to_data_path("shaders/height_field.vert"));
  pos_prog.create(GL_FRAGMENT_SHADER, rx_to_data_path("shaders/height_field_pos.frag"));
  pos_prog.link(0, NULL, 1, pos_frags);
  glUseProgram(pos_prog.id);
  glUniform1i(glGetUniformLocation(pos_prog.id, "u_height_tex"), 0);
  glUniform1i(glGetUniformLocation(pos_prog.id, "u_vel_tex"), 1);
  glUniform1i(glGetUniformLocation(pos_prog.id, "u_noise_tex"), 2);

  tex_noise = rx_create_texture(rx_to_data_path("images/water_noise.png"));

  // Extra processing
  const char* process_frags[] = { "out_norm", "out_tex" };
  process_prog.create(GL_VERTEX_SHADER, rx_to_data_path("shaders/height_field.vert"));
  process_prog.create(GL_FRAGMENT_SHADER, rx_to_data_path("shaders/height_field_process.frag"));
  process_prog.link(0, NULL, 2, process_frags);
  glUseProgram(process_prog.id);
  glUniform1i(glGetUniformLocation(process_prog.id, "u_height_tex"), 0);
  glUniform1i(glGetUniformLocation(process_prog.id, "u_vel_tex"), 1);
  glUniform1i(glGetUniformLocation(process_prog.id, "u_pos_tex"), 2);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  return true;
}

bool HeightField::setupDiffusing() {

  // some text data
  float* u = new float[field_size * field_size];
  float* v = new float[field_size * field_size];
  size_t nbytes = field_size * field_size * sizeof(float);
  memset((char*)u, 0x00, nbytes);
  memset((char*)v, 0x00, nbytes);

  int splash_size = 10;
  int upper = (field_size * 0.5) + splash_size;
  int lower = (field_size * 0.5) - splash_size;
  for(int j = 0; j < field_size; ++j) {
    for(int i = 0; i < field_size; ++i) {
      u[j * field_size + i] = 0.0f;
      v[j * field_size + i] = 0.0f;
      if(i > lower && i < upper && j > lower && j < upper) {
        u[j * field_size + i] = 0.0;
      }
    }
  }

  glGenTextures(1, &tex_u0);
  glBindTexture(GL_TEXTURE_2D, tex_u0);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, field_size, field_size, 0, GL_RED, GL_FLOAT, u);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glGenTextures(1, &tex_u1);
  glBindTexture(GL_TEXTURE_2D, tex_u1);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, field_size, field_size, 0, GL_RED, GL_FLOAT, u);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glGenTextures(1, &tex_v0);
  glBindTexture(GL_TEXTURE_2D, tex_v0);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, field_size, field_size, 0, GL_RED, GL_FLOAT, v);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glGenTextures(1, &tex_v1);
  glBindTexture(GL_TEXTURE_2D, tex_v1);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, field_size, field_size, 0, GL_RED, GL_FLOAT, v);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glGenFramebuffers(1, &field_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, field_fbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_u0, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, tex_u1, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, tex_v0, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, tex_v1, 0);

  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    printf("Error: diffuse framebuffer not complete.\n");
    return false;
  }

  GLenum drawbufs[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
  glDrawBuffers(4, drawbufs);
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT);

  const char* frags[] = { "out_height", "out_vel" } ;
  field_prog.create(GL_VERTEX_SHADER, rx_to_data_path("shaders/height_field.vert"));
  field_prog.create(GL_FRAGMENT_SHADER, rx_to_data_path("shaders/height_field.frag"));
  field_prog.link(0, NULL, 2, frags);

  glUseProgram(field_prog.id);
  glUniform1i(glGetUniformLocation(field_prog.id, "u_height_tex"), 0);
  glUniform1i(glGetUniformLocation(field_prog.id, "u_vel_tex"), 1);
  glUniform1i(glGetUniformLocation(field_prog.id, "u_force_tex"), 2);

  u_dt = glGetUniformLocation(field_prog.id, "u_dt");
  u_force = glGetUniformLocation(field_prog.id, "u_force");

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  delete[] v;
  delete[] u; 
  u = NULL;
  v = NULL;
  return true;
}

void HeightField::update(float dt) {
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_BLEND);

  glViewport(0, 0, field_size, field_size);
  glBindFramebuffer(GL_FRAMEBUFFER, field_fbo);
  glUseProgram(field_prog.id);
  glBindVertexArray(field_vao);
  glUniform1f(u_dt, dt);
  glUniform1f(u_force, force_max);

  state_diffuse = 1 - state_diffuse;

  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, force_tex);

  if(state_diffuse == 0) {
    // read from u0, write to u1
    // read from v0, write to v1
    GLenum drawbufs[] = { GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT3 };
    glDrawBuffers(2, drawbufs);

    glActiveTexture(GL_TEXTURE0);  glBindTexture(GL_TEXTURE_2D, tex_u0);
    glActiveTexture(GL_TEXTURE1);  glBindTexture(GL_TEXTURE_2D, tex_v0);

  }
  else {
    // read from u1, write to u0
    // read from v1, write to v0
    GLenum drawbufs[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(2, drawbufs);

    glActiveTexture(GL_TEXTURE0);  glBindTexture(GL_TEXTURE_2D, tex_u1);
    glActiveTexture(GL_TEXTURE1);  glBindTexture(GL_TEXTURE_2D, tex_v1);

  }
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);  
  
  // clear the forces buffer.
  GLenum forces_bufs[] = { GL_COLOR_ATTACHMENT0 } ;
  glBindFramebuffer(GL_FRAMEBUFFER, force_fbo);
  glDrawBuffers(1, forces_bufs);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, win_w, win_h);
}

void HeightField::process() {
  static float t = 0.0f;

  glBindFramebuffer(GL_FRAMEBUFFER, process_fbo);
  glViewport(0, 0, field_size, field_size);
  glBindVertexArray(field_vao);

  glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, tex_u0);
  glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, tex_v0);
  glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, tex_noise);

  {
    // Calculate positions.
    glUseProgram(pos_prog.id);
    glUniform1f(glGetUniformLocation(pos_prog.id, "u_time"), t);

    GLenum drawbufs[] = { GL_COLOR_ATTACHMENT0 } ;
    glDrawBuffers(1, drawbufs);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  }

  {
    // Use positions to calc normals, etc..
    glUseProgram(process_prog.id);
    GLenum drawbufs[] = { GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(2, drawbufs);

    glActiveTexture(GL_TEXTURE2); 
    glBindTexture(GL_TEXTURE_2D, tex_out_pos);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  t += 0.001;

  glViewport(0,0,win_w,win_h);
}

void HeightField::beginDrawForces() {
  GLenum drawbufs[] = { GL_COLOR_ATTACHMENT0 } ;
  glViewport(0, 0, field_size, field_size);
  glBindFramebuffer(GL_FRAMEBUFFER, force_fbo);
  glDrawBuffers(1, drawbufs);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT);
}

void HeightField::endDrawForces() {

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, win_w, win_h);
}

void HeightField::drawForceTexture(GLuint tex, float px, float py, float pw, float ph) {
  float hw = 0.5 * field_size * pw;
  float hh = 0.5 * field_size * ph;

  mat4 mm;
  mm.translate(px * field_size, py * field_size, 0.0f);
  mm.scale(pw * field_size, ph * field_size, 1.0);

  glUseProgram(force_prog.id);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex);

  glBindVertexArray(field_vao);
  glUniformMatrix4fv(glGetUniformLocation(force_prog.id, "u_mm"), 1, GL_FALSE, mm.ptr());

  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void HeightField::debugDraw() {
  glViewport(0, 0, win_w, win_h);
  glBindFramebuffer(GL_FRAMEBUFFER, 0); // tmp
  glDisable(GL_DEPTH_TEST);

  glBindVertexArray(vertices_vao);
  glUseProgram(debug_prog.id);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex_out_pos);

  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glDrawArrays(GL_POINTS, 0, vertices.size());
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  glBindFramebuffer(GL_READ_FRAMEBUFFER, field_fbo);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  glReadBuffer(GL_COLOR_ATTACHMENT0);
  glBlitFramebuffer(0, 0, field_size, field_size, 0, 0, field_size, field_size, GL_COLOR_BUFFER_BIT, GL_LINEAR);
  glBlitFramebuffer(0, 0, field_size, field_size, 0, 0, field_size, field_size, GL_COLOR_BUFFER_BIT, GL_LINEAR);

  glBindFramebuffer(GL_READ_FRAMEBUFFER, force_fbo);
  glReadBuffer(GL_COLOR_ATTACHMENT0);
  glBlitFramebuffer(0, 0, field_size, field_size, field_size, 0, field_size*2, field_size, GL_COLOR_BUFFER_BIT, GL_LINEAR);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void HeightField::print() {
  printf("heightfield.tex_u0: %d\n", tex_u0);
  printf("heightfield.tex_u1: %d\n", tex_u1);
  printf("heightfield.tex_v0: %d\n", tex_v0);
  printf("heightfield.tex_v1: %d\n", tex_v1);
  printf("heightfield.tex_out_norm: %d\n", tex_out_norm);
  printf("heightfield.tex_out_texcoord: %d\n", tex_out_texcoord);
  printf("heightfield.tex_out_pos: %d\n", tex_out_pos);
  printf("heightfield.force_tex: %d\n", force_tex);
}
