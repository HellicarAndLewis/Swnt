#include <ocean/Ocean.h>
#include <ocean/WaterGraphics.h>

Ocean::Ocean(OceanSettings& settings, WaterGraphics& gfx) 
  :settings(settings)
  ,gfx(gfx)
  ,data_vbo(0)
  ,indices_vbo(0)
  ,changed(true)
  ,ping_phase(true)
  ,wind_x(20.5f)
  ,wind_y(20.3f)
  ,size(250.0f)
{
}

Ocean::~Ocean() {
}

bool Ocean::setup() {

  if(!setupBuffers()) {
    printf("Error: cannot setup the ocean buffer.\n");
    return false;
  }

  return true;
}

bool Ocean::setupBuffers() {
  for(int z_dx = 0; z_dx < GEOMETRY_RESOLUTION - 1; ++z_dx) {
    for(int x_dx = 0; x_dx < GEOMETRY_RESOLUTION -1; ++x_dx) {
      int top_left = z_dx * GEOMETRY_RESOLUTION + x_dx;
      int top_right = top_left + 1;
      int bottom_left = top_left + GEOMETRY_RESOLUTION;
      int bottom_right = bottom_left + 1;

      // triangle a
      indices.push_back(top_left);
      indices.push_back(bottom_left);
      indices.push_back(bottom_right);

      // triangle b
      indices.push_back(bottom_right);
      indices.push_back(top_right);
      indices.push_back(top_left);
    }
  }

  std::vector<float> data;
  for(int z_dx = 0; z_dx < GEOMETRY_RESOLUTION; ++z_dx) {
    for(int x_dx = 0; x_dx < GEOMETRY_RESOLUTION; ++x_dx) {
      float x0 = (x_dx * GEOMETRY_SIZE) / (GEOMETRY_RESOLUTION - 1) + GEOMETRY_ORIG_X;
      float y0 = 0.0f;
      float z0 = (z_dx * GEOMETRY_SIZE) / (GEOMETRY_RESOLUTION - 1) + GEOMETRY_ORIG_Z;
      float x1 = float(x_dx) / (GEOMETRY_RESOLUTION - 1);
      float z1 = float(z_dx) / (GEOMETRY_RESOLUTION - 1);

      data.push_back(x0);
      data.push_back(y0);
      data.push_back(z0);
      data.push_back(x1);
      data.push_back(z1);

      #if 0
      if(z_dx == 1) {
        printf("x0: %f, z0: %f, x1: %f, z1: %f\n", x0, z0, x1, z1);
      }
      #endif
    }
  }
  // data - vao
  glGenVertexArrays(1, &data_vao);
  glBindVertexArray(data_vao);
  
  // data
  glGenBuffers(1, &data_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, data_vbo);
  glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);

  glEnableVertexAttribArray(0); // pos
  glEnableVertexAttribArray(1); // tex
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (GLvoid*) 0); // pos
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (GLvoid*)(sizeof(float) * 3)); // tex

  // indices
  glGenBuffers(1, &indices_vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_vbo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), &indices[0], GL_STATIC_DRAW);

  glBindVertexArray(0);

  int w = RESOLUTION;
  int h = RESOLUTION;
  init_spectrum_tex =    createTexture(INITIAL_SPECTRUM_UNIT, GL_RGBA32F, w, h, GL_RGBA, GL_FLOAT, NULL, GL_REPEAT, GL_NEAREST);
  pong_phase_tex =       createTexture(PONG_PHASE_UNIT,       GL_RGBA32F, w, h, GL_RGBA, GL_FLOAT, NULL, GL_CLAMP_TO_EDGE, GL_NEAREST);
  spectrum_tex =         createTexture(SPECTRUM_UNIT,         GL_RGBA32F, w, h, GL_RGBA, GL_FLOAT, NULL, GL_CLAMP_TO_EDGE, GL_NEAREST);
  displacement_map_tex = createTexture(DISPLACEMENT_MAP_UNIT, GL_RGBA32F, w, h, GL_RGBA, GL_FLOAT, NULL, GL_CLAMP_TO_EDGE, GL_LINEAR);
  normal_map_tex =       createTexture(NORMAL_MAP_UNIT,       GL_RGBA32F, w, h, GL_RGBA, GL_FLOAT, NULL, GL_CLAMP_TO_EDGE, GL_LINEAR);
  ping_transform_tex =   createTexture(PING_TRANSFORM_UNIT,   GL_RGBA32F, w, h, GL_RGBA, GL_FLOAT, NULL, GL_CLAMP_TO_EDGE, GL_NEAREST);
  pong_transform_tex =   createTexture(PONG_TRANSFORM_UNIT,   GL_RGBA32F, w, h, GL_RGBA, GL_FLOAT, NULL, GL_CLAMP_TO_EDGE, GL_NEAREST);

  printf("init_spectrum_tex: %d\n", init_spectrum_tex);
  printf("pong_phase_tex: %d\n", pong_phase_tex);
  printf("spectrum_tex: %d\n", spectrum_tex);
  printf("displacement_map_tex: %d\n", displacement_map_tex);
  printf("normal_map_tex: %d\n", normal_map_tex);
  printf("ping_transform_tex: %d\n", ping_transform_tex);
  printf("pong_transform_tex: %d\n", pong_transform_tex);

  // phase data
  std::vector<float> phase_data;
  phase_data.assign(RESOLUTION * RESOLUTION * 4, 0.0f);
  for(int i = 0; i < RESOLUTION; ++i) {
    for(int j = 0; j < RESOLUTION; ++j) {
      phase_data[i * RESOLUTION * 4 + j * 4 + 0] = rx_random(0.0f, 1.0f) * TWO_PI;
      phase_data[i * RESOLUTION * 4 + j * 4 + 1] = 0.0f;
      phase_data[i * RESOLUTION * 4 + j * 4 + 2] = 0.0f;
      phase_data[i * RESOLUTION * 4 + j * 4 + 3] = 0.0f;
    }
  }

  ping_phase_tex = createTexture(PING_PHASE_UNIT, GL_RGBA32F, w, h, GL_RGBA, GL_FLOAT, (GLvoid*) &phase_data[0], GL_CLAMP_TO_EDGE, GL_NEAREST);
  printf("ping_phase_tex: %d\n\n---\n", ping_phase_tex);

  // create FBOs
  init_spectrum_fbo = createFBO(init_spectrum_tex);
  ping_phase_fbo = createFBO(ping_phase_tex);
  pong_phase_fbo = createFBO(pong_phase_tex);
  spectrum_fbo = createFBO(spectrum_tex);
  displacement_map_fbo = createFBO(displacement_map_tex);
  normal_map_fbo = createFBO(normal_map_tex);
  ping_transform_fbo = createFBO(ping_transform_tex);
  pong_transform_fbo = createFBO(pong_transform_tex);
  
  printf("init_spectrum_fbo: %d\n", init_spectrum_fbo);
  printf("pong_phase_fbo: %d\n", pong_phase_fbo);
  printf("spectrum_fbo: %d\n", spectrum_fbo);
  printf("displacement_map_fbo: %d\n", displacement_map_fbo);
  printf("normal_map_fbo: %d\n", normal_map_fbo);
  printf("ping_transform_fbo: %d\n", ping_transform_fbo);
  printf("pong_transform_fbo: %d\n", pong_transform_fbo);


  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  return true;
}

GLuint Ocean::createTexture(GLuint unit, GLenum iformat, int w, int h, GLenum eformat, GLenum type, GLvoid* data, GLenum wrap, GLenum filter) {
  GLuint tex = 0;
  glGenTextures(1, &tex);
  glActiveTexture(GL_TEXTURE0 + unit);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexImage2D(GL_TEXTURE_2D, 0, iformat, w, h, 0, eformat, type, data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
  return tex;
}

GLuint Ocean::createFBO(GLuint tex) {
  GLuint fbo = 0;
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if(status != GL_FRAMEBUFFER_COMPLETE) {
    printf("Error framebuffer is not complete.\n");
    ::exit(EXIT_FAILURE);
  }

  return fbo;
}


void Ocean::update(float dt) {

  bindTexture(init_spectrum_tex, INITIAL_SPECTRUM_UNIT);
  bindTexture(pong_phase_tex, PONG_PHASE_UNIT);
  bindTexture(spectrum_tex, SPECTRUM_UNIT);
  bindTexture(displacement_map_tex, DISPLACEMENT_MAP_UNIT);
  bindTexture(normal_map_tex, NORMAL_MAP_UNIT);
  bindTexture(ping_transform_tex, PING_TRANSFORM_UNIT);
  bindTexture(pong_transform_tex, PONG_TRANSFORM_UNIT);

  glViewport(0, 0, RESOLUTION, RESOLUTION);
  glDisable(GL_DEPTH_TEST);

  glBindVertexArray(gfx.fullscreen_vao);

  if(changed) {

    printf("Init spectrum.\n");
    glBindFramebuffer(GL_FRAMEBUFFER, init_spectrum_fbo);
    #if 0
    glClear(GL_COLOR_BUFFER_BIT);
    GLenum db[] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, db);
    #endif

    glUseProgram(gfx.init_spectrum_prog);
    glUniform2f(ufl(gfx.init_spectrum_prog, "u_wind"), wind_x, wind_y);
    glUniform1f(ufl(gfx.init_spectrum_prog, "u_size"), size);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  }

  // phase
  glUseProgram(gfx.phase_prog);
  glBindFramebuffer(GL_FRAMEBUFFER, (ping_phase) ? pong_phase_fbo : ping_phase_fbo);
  glUniform1i(ufl(gfx.phase_prog, "u_phases"), (ping_phase) ? PING_PHASE_UNIT : PONG_PHASE_UNIT);
  ping_phase = !ping_phase;
  glUniform1f(ufl(gfx.phase_prog, "u_deltaTime"), dt);
  glUniform1f(ufl(gfx.phase_prog, "u_size"), size);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  // spectrum
  glUseProgram(gfx.spectrum_prog);
  glBindFramebuffer(GL_FRAMEBUFFER, spectrum_fbo);
  glUniform1i(ufl(gfx.spectrum_prog, "u_phases"), ping_phase ? PING_PHASE_UNIT : PONG_PHASE_UNIT);
  glUniform1f(ufl(gfx.spectrum_prog, "u_size"), size);
  glUniform1f(ufl(gfx.spectrum_prog, "u_choppiness"), 2.5);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  // GPU FFT using Stockham 
#if 1

  GLuint subtrans_prog = gfx.hor_subtrans_prog;  
  glUseProgram(subtrans_prog);
  
  int niter = log2(RESOLUTION) * 2;

  for(int i = 0; i < niter; ++i) {
    if(i == 0) {
      glBindFramebuffer(GL_FRAMEBUFFER, ping_transform_fbo);
      glUniform1i(ufl(subtrans_prog, "u_input"), SPECTRUM_UNIT);
    }
    else if(i == (niter - 1) ) {
      glBindFramebuffer(GL_FRAMEBUFFER, displacement_map_fbo);
      glUniform1i(ufl(subtrans_prog, "u_input"), (niter % 2 == 0) ? PING_TRANSFORM_UNIT : PONG_TRANSFORM_UNIT);
    }
    else if(i % 2 == 1) {
      glBindFramebuffer(GL_FRAMEBUFFER, pong_transform_fbo);
      glUniform1i(ufl(subtrans_prog, "u_input"), PING_TRANSFORM_UNIT);
    }
    else {
      glBindFramebuffer(GL_FRAMEBUFFER, ping_transform_fbo);
      glUniform1i(ufl(subtrans_prog, "u_input"), PONG_TRANSFORM_UNIT);
    }

    if(i == (niter / 2)) {
      subtrans_prog = gfx.vert_subtrans_prog;
      glUseProgram(subtrans_prog);
    }
    
    glUniform1f(ufl(subtrans_prog, "u_subtransform_size"), (float)std::pow(2.0f, (i % (niter / 2)) + 1.0f));
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  }

#endif

  // normal map
  glBindFramebuffer(GL_FRAMEBUFFER, normal_map_fbo);
  glUseProgram(gfx.normal_map_prog);
  if(changed) {
    glUniform1f(ufl(gfx.normal_map_prog, "u_size"), size);
  }
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);


  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0.0f, 0.0f, settings.win_w, settings.win_h);

}

void Ocean::draw() {
  // ocean shader

  {
    
    //glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //glViewport(0, 0, settings.win_w, settings.win_h);
    glEnable(GL_DEPTH_TEST);
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(data_vao);
    glUseProgram(gfx.ocean_prog);

    vec3 cam_pos(-500.0f, 819.0f, 0.0f);
    //vec3 cam_pos(-712.0f, 719.0f, 1412.0);
    mat4 pm;

    //pm.perspective(60, 16.0/9.0, 1, 10000);
    pm.perspective(60, 4.0/3.0, 1, 10000);
    mat4 vm;
    vm.lookAt(vec3(cam_pos.x, cam_pos.y, cam_pos.z), vec3(0.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0));
    //vm.scale(1.0, -1.0, 1.0);


    // using hardcoded view matrix and projection matrix 
    float tpm[16]; // projection matrix
    float tvm[16]; // view matrix 

    tvm[0] = 0.9210609793663025;   tvm[4] = 0;                   tvm[8] = 0.3894183337688446;    tvm[12] = -49.438804626464844;
    tvm[1] = 0.1866970956325531;   tvm[5] = 0.8775825500488281;  tvm[9] = -0.4415801763534546;   tvm[13] = 302.28753662109375;
    tvm[2] = -0.3417467474937439;  tvm[6] = 0.4794255495071411;  tvm[10] = 0.8083070516586304;   tvm[14] = -1253.33349609375;
    tvm[3] = 0;                    tvm[7] = 0;                   tvm[11] = 0;                    tvm[15] = 1; 


    tpm[0] = 0.5231773257255554; tpm[4] = 0;                    tpm[8] = 0;                      tpm[12] = 0;                  
    tpm[1] = 0;                  tpm[5] = 1.7320507764816284;   tpm[9] = 0;                      tpm[13] = 0;                  
    tpm[2] = 0;                  tpm[6] = 0;                    tpm[10] = -1.000100016593933;    tpm[14] = -1.000100016593933; 
    tpm[3] = 0;                  tpm[7] = 0;                    tpm[11] = -1;                    tpm[15] = 0;                  

    
#if 1
    glUniformMatrix4fv(ufl(gfx.ocean_prog, "u_pm"), 1, GL_FALSE, pm.ptr());
    glUniformMatrix4fv(ufl(gfx.ocean_prog, "u_vm"), 1, GL_FALSE, vm.ptr());
#else
    glUniformMatrix4fv(ufl(gfx.ocean_prog, "u_pm"), 1, GL_FALSE, tpm);
    glUniformMatrix4fv(ufl(gfx.ocean_prog, "u_vm"), 1, GL_FALSE, tvm);
#endif
    glUniform3fv(ufl(gfx.ocean_prog, "u_camera_position"), 1, cam_pos.ptr());

    
    if(changed) {
      glUniform1f(ufl(gfx.ocean_prog, "u_size"), size);
    }

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }

  glDisable(GL_DEPTH_TEST);
  changed = false;  
}
