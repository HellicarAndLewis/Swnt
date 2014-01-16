#include <assert.h>
#include <swnt/Water.h>
#include <swnt/HeightField.h>
#include <swnt/Settings.h>

Water::Water(HeightField& hf, Settings& settings)
  :height_field(hf)
  ,settings(settings)
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
  ,vortex_tex(0)
  ,color_tex(0)
  ,foam_colors_tex(0)
  ,foam_ramp_tex(0)
  ,max_depth(5.0)
  ,sun_shininess(4.6)
  ,foam_depth(2.4)
  ,fbo(0)
  ,extra_flow_tex(0)
  ,vortex_intensity(0.2)
{
  sun_pos[0] = 0.0;
  sun_pos[1] = 15.0;
  sun_pos[2] = -25.0;
  sun_color[0] = 4;
  sun_color[1] = 3;
  sun_color[2] = 1;
  ads_intensities[0] = 0.2f;  // ambient, the selected color
  ads_intensities[1] = 0.0f;  // diffuse
  ads_intensities[2] = 0.75f; // spec
  ads_intensities[3] = 0.36;  // sun  
  ads_intensities[4] = 0.39;  // foam
  ads_intensities[5] = 0.45;  // texture
  ads_intensities[6] = 1.0;   // overall intensity
  ambient_color[0] = 46.0/255.0f;
  ambient_color[1] = 72.0/255.0f;
  ambient_color[2] = 96.0/255.0f;


#if 0
  float r = 0.0/255.0f;
  float g = 125.0f/255.0f;
  float b = 155.0f/255.0f;
  float h,s,v = 0.0f;

  float step = 0.01;
  for(float i = 0; i < 1.0f; i += step) {
    for(float j = 0; j < 1.0f; j += step) {
      for(float k = 0; k < 1.0f; k += step) {
        r = i;
        g = j;
        b = k;
        rx_rgb_to_hsv(r,g,b, h,s,v);
        printf("> hsv(%f, %f, %f), rgb(%f, %f, %f)\n", h,s,v, r,g,b);
        float r1,g1,b1;
        rx_hsv_to_rgb(h,s,v,r1,g1,b1);
        printf("< hsv(%f, %f, %f), rgb(%f, %f, %f)\n", h,s,v, r,g,b);

        if(fabs(r1 - r) > 0.001) {
          //  printf("Red is wrong.\n");
          //::exit(0);
        }

        if(fabs(g1 - g) > 0.001) {
          //printf("Green is wrong.\n");
          /// ::exit(0);
        }
       
        if(fabs(b1 - b) > 0.001) {
          //printf("Blue is wrong.\n");
          //::exit(0);
        }
        printf("\n");
      }
    }
  }

  r = 0.0f;
  g = 0.0f;
  b = 0.2f;
  rx_rgb_to_hsv(r,g,b,h,s,v);
  printf("h: %f, s: %f, v: %f - r: %f, g: %f, b: %f\n", h,s, v, r, g, b);
    r = g = b = 0;
  rx_hsv_to_rgb(h,s,v,r,g,b);
  printf("h: %f, s: %f, v: %f - r: %f, g: %f, b: %f\n", h, s, v, r, g, b);
#endif
  
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
  glUniform1i(glGetUniformLocation(prog, "u_tex_pos"),            0);  // VS
  glUniform1i(glGetUniformLocation(prog, "u_tex_norm"),           1);  // VS
  glUniform1i(glGetUniformLocation(prog, "u_tex_texcoord"),       2);  // VS
  //  glUniform1i(glGetUniformLocation(prog, "u_tex_tang"),       3);  // VS
  glUniform1i(glGetUniformLocation(prog, "u_tex_gradient"),       3);  // VS
  glUniform1i(glGetUniformLocation(prog, "u_noise_tex"),          4);  // FS
  glUniform1i(glGetUniformLocation(prog, "u_norm_tex"),           5);  // FS
  glUniform1i(glGetUniformLocation(prog, "u_flow_tex"),           6);  // FS
  glUniform1i(glGetUniformLocation(prog, "u_diffuse_tex"),        7);  // FS
  glUniform1i(glGetUniformLocation(prog, "u_foam_tex"),           8);  // FS
  glUniform1i(glGetUniformLocation(prog, "u_color_tex"),          9);  // FS
  glUniform1i(glGetUniformLocation(prog, "u_extra_flow_tex"),     10);  // FS
  glUniform1i(glGetUniformLocation(prog, "u_foam_delta_tex"),     11);  // FS
  glUniform1i(glGetUniformLocation(prog, "u_foam_colors"),        12);  // FS
  glUniform1i(glGetUniformLocation(prog, "u_foam_ramp"),          13);  // FS
  glUniform1i(glGetUniformLocation(prog, "u_vortex_tex"),         14);  // FS

  //GLint texture_units = 0;
  //glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &texture_units);
  //printf("Max units: %d\n", texture_units);

  glUniformMatrix4fv(glGetUniformLocation(prog, "u_pm"), 1, GL_FALSE, height_field.pm.ptr());
  glUniformMatrix4fv(glGetUniformLocation(prog, "u_vm"), 1, GL_FALSE, height_field.vm.ptr());

  // load textures
  normals_tex = createTexture("images/water_normals.png");
  flow_tex = createTexture("images/water_flow.png");
  noise_tex = createTexture("images/water_noise.png");
  diffuse_tex = createTexture("images/water_diffuse.png");
  foam_tex = createTexture("images/water_foam.png");
  force_tex0 = createTexture("images/force.png");
  foam_colors_tex = createTexture("images/foam_densities.png");
  foam_ramp_tex = createTexture("images/foam_ramps.png");
  vortex_tex = createTexture("images/vortex.png");
  
  glBindTexture(GL_TEXTURE_2D, foam_colors_tex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glBindTexture(GL_TEXTURE_2D, foam_ramp_tex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  unsigned char* img_pix = NULL;
  int img_w, img_h,img_channels = 0;  

  // load diffuse color ramp
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


  // FBO we used for rendering extra diffuse colors to the water
  glGenFramebuffers(1, &fbo); 
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  glGenTextures(1, &extra_flow_tex);
  glBindTexture(GL_TEXTURE_2D, extra_flow_tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, win_w, win_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, extra_flow_tex, 0);

  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    printf("Cannot create the framebuffer for the water diffuse capture.\n");
    return false;
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

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

    //    glActiveTexture(GL_TEXTURE3);
    //    glBindTexture(GL_TEXTURE_2D, height_field.tex_tang);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, height_field.tex_gradient);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D,  noise_tex);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, normals_tex);

    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, flow_tex);

    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, diffuse_tex);

    glActiveTexture(GL_TEXTURE8);
    glBindTexture(GL_TEXTURE_2D, foam_tex);

    glActiveTexture(GL_TEXTURE10);
    glBindTexture(GL_TEXTURE_2D, extra_flow_tex);

    glActiveTexture(GL_TEXTURE9);
    glBindTexture(GL_TEXTURE_1D, color_tex);

    glActiveTexture(GL_TEXTURE11);
    glBindTexture(GL_TEXTURE_2D, height_field.tex_foam0);

    glActiveTexture(GL_TEXTURE12);
    glBindTexture(GL_TEXTURE_2D, foam_colors_tex);

    glActiveTexture(GL_TEXTURE13);
    glBindTexture(GL_TEXTURE_2D, foam_ramp_tex);

    glActiveTexture(GL_TEXTURE14);
    glBindTexture(GL_TEXTURE_2D, vortex_tex);
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
  glUniform1fv(glGetUniformLocation(prog, "u_ads_intensities"), 7, ads_intensities);
  glUniform3fv(glGetUniformLocation(prog, "u_ambient_color"), 1, ambient_color);
  glUniform1f(glGetUniformLocation(prog, "u_vortex_intensity"), vortex_intensity);

  //glEnable(GL_CULL_FACE);
  //  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  //glFrontFace(GL_CW);
  glBindVertexArray(height_field.vao);
  glDrawElements(GL_TRIANGLES, height_field.indices.size(), GL_UNSIGNED_INT, NULL);
  //glDisable(GL_CULL_FACE);
  //glFrontFace(GL_CCW);
  // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}


void Water::beginGrabFlow() {
  GLenum drawbufs[] = { GL_COLOR_ATTACHMENT0 } ;
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glDrawBuffers(1, drawbufs);
  glClear(GL_COLOR_BUFFER_BIT);
  glViewport(0, 0, win_w, win_h);
}

void Water::endGrabFlow() {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Water::blitFlow(float x, float y, float w, float h) {
  glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
  glReadBuffer(GL_COLOR_ATTACHMENT0);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  glBlitFramebuffer(0, 0, win_w, win_h, x, y, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void Water::setTimeOfDay(float t, float sun) {
  sun = CLAMP(sun, 0.0f, 1.0f);
  foam_depth = 2.0 + (t * 1.2);
  ads_intensities[6] = 1.0 + (sun * 0.6); // overall intensity

  // change the color of the water.
  /*
  vec3 hsv(sun * 0.410, 0.5, 0.5);
  rx_hsv_to_rgb(hsv, ambient_color);
  */
  return;

  ads_intensities[0] = -0.2 + (sun * 0.7); // ambient
  ads_intensities[3] = (sun * 0.5);  // sun 
  ads_intensities[4] = sun; // foam
}

void Water::setTimeOfYear(float t) {
  ambient_color[0] = settings.curr_colors.water.x;
  ambient_color[1] = settings.curr_colors.water.y;
  ambient_color[2] = settings.curr_colors.water.z;
}

void Water::setRoughness(float r) {
  r = CLAMP(r, 0.0f, 1.0f);
}


void Water::print() {
  printf("water.flow_tex: %d\n", flow_tex);
  printf("water.normals_tex: %d\n", normals_tex);
  printf("water.noise_tex: %d\n", noise_tex);
  printf("water.diffuse_tex: %d\n", diffuse_tex);
  printf("water.foam_tex: %d\n", foam_tex);
  printf("water.color_tex: %d\n", color_tex);
  printf("water.extra_flow_tex: %d\n", extra_flow_tex);
}
