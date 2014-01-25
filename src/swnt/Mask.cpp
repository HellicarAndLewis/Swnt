#include <assert.h>
#include <swnt/Mask.h>
#include <swnt/Graphics.h>

Mask::Mask(Settings& settings, Graphics& graphics) 
  :settings(settings)
  ,graphics(graphics)
  ,draw_hand(true)
  ,mask_vao(0)
  ,mask_vbo(0)
  ,scene_fbo(0)
  ,scene_depth(0)
  ,scene_tex(0)
  ,resolution(560)
  ,mask_vert(0)
  ,mask_frag(0)
  ,mask_prog(0)
  ,mask_diffuse_tex(0)
  ,masked_out_vao(0)
  ,masked_out_fbo(0)
  ,masked_out_tex(0)
  ,masked_out_vert(0)
  ,masked_out_frag(0)
  ,masked_out_prog(0)
  ,masked_out_pixels(NULL)
  ,masked_scene_frag(0)
  ,masked_scene_prog(0)
  ,mask_tex_frag(0)
  ,mask_tex_prog(0)
  ,hand_prog(0)
  ,hand_vert(0)
  ,hand_frag(0)
  ,hand_rotation(0)
  ,hand_flip_x(false)
  ,hand_flip_y(false)
  ,perlin(4, 4, 1, 34)
  ,bytes_allocated(0)
  ,scale(1.0)
  ,scale_range(50.0f)
{
}

Mask::~Mask() {
}

bool Mask::setup() {

  center.pos.x = settings.win_w * 0.5;
  center.pos.y = settings.win_h * 0.5;
  center.tex.set(0.0, 0.0);

  if(!createShader()) {
    printf("Error: cannot create mask shader.\n");
    return false;
  }

  if(!createFBO()) {
    printf("Error: cannot create mask fbo.\n");
    return false;
  }

  if(!createBuffers()) {
    printf("Error: cannot create mask buffers.\n");
    return false;
  }

  masked_out_pixels = new unsigned char[settings.image_processing_w * settings.image_processing_h];
  if(!masked_out_pixels) {
    printf("Error: cannot allocated the buffer for the image processing image. Using to big or invalid values? See settings.xml\n");
    return false;
  }

  if(!blur.setup(settings.image_processing_w, settings.image_processing_h, 4, 5)) {
    printf("Error: cannot setup the blur shader.\n");
    return false;
  }

  if(!thresh.setup(settings.image_processing_w, settings.image_processing_h)) {
    printf("Error: cannot setup the thresholder.\n");
    return false;
  }

  refresh();

  return true;
}


bool Mask::createBuffers() {

  // Mask shape GL state
  {
    glGenVertexArrays(1, &mask_vao);
    glBindVertexArray(mask_vao);
    glGenBuffers(1, &mask_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mask_vbo);
    glEnableVertexAttribArray(0); // pos
    glEnableVertexAttribArray(1); // tex
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPT), (GLvoid*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexPT), (GLvoid*)12);
  }
  
  // Masking out GL 
  glGenVertexArrays(1, &masked_out_vao);
  return true;
}

void Mask::update() {
  updateVertices();
}

// Update the vertices that create the mask shape
void Mask::updateVertices() {
  assert(resolution);
  assert(settings.radius > 0.0f);

  vertices.clear();
  vertices.push_back(center);

  static float t = 0.001f;
  float a = TWO_PI / resolution;
  float first_change = 0;
  float radius = settings.radius - scale_range + (scale_range * scale);

  for(int i = 0; i <= resolution; ++i) {
    float perc = float(i) / (resolution-1);
#if 0
    float change = sin(perc * TWO_PI * 10) * 5 ;
#else
    float change = 0.0f;
#endif
    VertexPT pt;
    pt.pos.set(center.pos.x + cos(a * i) * (radius + change), 
               center.pos.y + sin(a * i) * (radius + change),
               0.0f);
    pt.tex.set(perc, 0.95f);
    vertices.push_back(pt);
    //vec2 p(center.x + cos(a * i) * (radius + change), center.y + sin(a * i) * (radius + change));
    //vertices.push_back(p);
  }

  t += 0.001;

  glBindBuffer(GL_ARRAY_BUFFER, mask_vbo);
  size_t needed = sizeof(VertexPT) * vertices.size();
  if(needed > bytes_allocated) {
    glBufferData(GL_ARRAY_BUFFER, needed, vertices[0].ptr(), GL_STREAM_DRAW);
    bytes_allocated = needed;
  }
  else {
    glBufferSubData(GL_ARRAY_BUFFER, 0, needed, vertices[0].ptr());
  }

}

bool Mask::createShader() {

  // Basic shader for rendering the actuall mask shape
  mask_vert = rx_create_shader(GL_VERTEX_SHADER, MASK_VS);
  mask_frag = rx_create_shader(GL_FRAGMENT_SHADER, MASK_FS);
  mask_prog = rx_create_program(mask_vert, mask_frag);
  glBindAttribLocation(mask_prog, 0, "a_pos");
  glBindAttribLocation(mask_prog, 1, "a_tex");
  glLinkProgram(mask_prog);
  rx_print_shader_link_info(mask_prog);
  glUseProgram(mask_prog);
  glUniform1i(glGetUniformLocation(mask_prog, "u_mask_tex"), 0);

  mask_diffuse_tex = rx_create_texture(rx_to_data_path("images/mask_diffuse.png"));

  // Shader that masks out the shape of the mask in the depth image of the kinect
  masked_out_vert = rx_create_shader(GL_VERTEX_SHADER, TEX_VS);
  masked_out_frag = rx_create_shader(GL_FRAGMENT_SHADER, MASKED_OUT_FS);
  masked_out_prog = rx_create_program(masked_out_vert, masked_out_frag);
  glBindAttribLocation(masked_out_prog, 0, "a_pos");
  glBindAttribLocation(masked_out_prog, 1, "a_tex");
  glLinkProgram(masked_out_prog);
  rx_print_shader_link_info(masked_out_prog);

  // Shader that masks out the scene texture using the mask texture
  masked_scene_frag = rx_create_shader(GL_FRAGMENT_SHADER, MASKED_OUT_SCENE_FS);
  masked_scene_prog = rx_create_program(masked_out_vert, masked_scene_frag);
  glBindAttribLocation(masked_scene_prog, 0, "a_pos");
  glBindAttribLocation(masked_scene_prog, 1, "a_tex");
  glLinkProgram(masked_scene_prog);
  rx_print_shader_link_info(masked_scene_prog);

  // Shader that will mask out a generic texture
  mask_tex_frag = rx_create_shader(GL_FRAGMENT_SHADER, DRAW_MASKED_FS);
  mask_tex_prog = rx_create_program(graphics.tex_vs, mask_tex_frag);
  glBindAttribLocation(mask_tex_prog, 0, "a_pos"); // @todo hmm we don't have this
  glBindAttribLocation(mask_tex_prog, 1, "a_tex"); // @todo and this one...
  glLinkProgram(mask_tex_prog);
  rx_print_shader_link_info(mask_tex_prog);
  glUseProgram(mask_tex_prog);
  glUniform1i(glGetUniformLocation(mask_tex_prog, "u_mask_tex"), 0);
  glUniform1i(glGetUniformLocation(mask_tex_prog, "u_diffuse_tex"), 1);
  glUniformMatrix4fv(glGetUniformLocation(mask_tex_prog, "u_pm"), 1, GL_FALSE, graphics.tex_pm.ptr());

  mat4 mm;
  mm.translate(settings.win_w * 0.5, settings.win_h * 0.5, 0.0f);
  mm.scale(settings.win_w * 0.5, settings.win_h * 0.5, 1.0);
  glUniformMatrix4fv(glGetUniformLocation(mask_tex_prog, "u_mm"), 1, GL_FALSE, mm.ptr());

  // Last minute change - hand prog
  hand_vert = rx_create_shader(GL_VERTEX_SHADER, TEX_VS);
  hand_frag = rx_create_shader(GL_FRAGMENT_SHADER, MASK_HAND_FS);
  hand_prog = rx_create_program(hand_vert, hand_frag);
  glLinkProgram(hand_prog);
  rx_print_shader_link_info(hand_prog);
  glUseProgram(hand_prog);
  glUniformMatrix4fv(glGetUniformLocation(hand_prog, "u_pm"), 1, GL_FALSE, graphics.tex_pm.ptr());
  glUniform1i(glGetUniformLocation(hand_prog, "u_hand_tex"), 0);
  glUniform1i(glGetUniformLocation(hand_prog, "u_mask_tex"), 1);
  return true;
}

bool Mask::createFBO() {

  assert(settings.win_w > 0 && settings.win_h > 0);
  assert(settings.image_processing_w > 0 && settings.image_processing_h > 0);

  // Setup the render scene to texture 
  {

    glGenFramebuffers(1, &scene_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, scene_fbo);

    glGenRenderbuffers(1, &scene_depth);
    glBindRenderbuffer(GL_RENDERBUFFER, scene_depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, settings.win_w, settings.win_h);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, scene_depth);

    glGenTextures(1, &scene_tex);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, scene_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, settings.win_w, settings.win_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, scene_tex, 0);

    glGenTextures(1, &mask_tex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mask_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, settings.win_w, settings.win_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, mask_tex, 0);

    glBindTexture(GL_TEXTURE_2D, 0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE) {
      printf("Error: fbo not complete.\n");
      return false;
    }
  }

  // Setup the fbo that we use to mask out everything outside the 'mask' 
  {
    glGenFramebuffers(1, &masked_out_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, masked_out_fbo);

    glGenTextures(1, &masked_out_tex);
    glBindTexture(GL_TEXTURE_2D, masked_out_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, settings.image_processing_w, settings.image_processing_h, 0,  GL_RED, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, masked_out_tex, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      printf("Error: cannot create the masked out fbo.\n");
    }
  }

  // Setup the fbo into which we render the depth image
  {
    glGenFramebuffers(1, &depth_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, depth_fbo);
    glGenTextures(1, &depth_tex);
    glBindTexture(GL_TEXTURE_2D, depth_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, settings.image_processing_w, settings.image_processing_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, depth_tex, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  return true;
}

void Mask::beginSceneGrab() {
  GLenum drawbufs[] = { GL_COLOR_ATTACHMENT0 } ;
  glBindFramebuffer(GL_FRAMEBUFFER, scene_fbo);
  glDrawBuffers(1, drawbufs);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glViewport(0.0f, 0.0f, settings.win_w, settings.win_h);
}

void Mask::endSceneGrab() {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0.0f, 0.0f, settings.win_w, settings.win_h);
}

void Mask::beginDepthGrab() { 
  GLenum drawbufs[] = { GL_COLOR_ATTACHMENT0 } ;
  glBindFramebuffer(GL_FRAMEBUFFER, depth_fbo);
  glDrawBuffers(1, drawbufs);
  glViewport(0.0f, 0.0f, settings.image_processing_w, settings.image_processing_h);
}

void Mask::endDepthGrab() {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0.0f, 0.0f, settings.win_w, settings.win_h);
}

void Mask::beginMaskGrab() {
  GLenum drawbufs[] = { GL_COLOR_ATTACHMENT1 };
  glBindFramebuffer(GL_FRAMEBUFFER, scene_fbo);
  glDrawBuffers(1, drawbufs);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  glViewport(0.0f, 0.0f, settings.win_w, settings.win_h);
}

void Mask::endMaskGrab() {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0.0f, 0.0f, settings.win_w, settings.win_h);
}

void Mask::drawMask() {
  glBindVertexArray(mask_vao);
  glUseProgram(mask_prog);
  glUniformMatrix4fv(glGetUniformLocation(mask_prog, "u_pm"), 1, GL_FALSE, settings.ortho_matrix.ptr());

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, mask_diffuse_tex);

  glDrawArrays(GL_TRIANGLE_FAN, 0, vertices.size());
}

// Use the mask to remove all the depth pixels. we also blur + threshold the masked_out image
void Mask::maskOutDepth() {

  assert(masked_out_pixels);

  mat4 mm;
  mat4 pm;

  GLenum drawbufs[] = { GL_COLOR_ATTACHMENT0 } ;
  glBindFramebuffer(GL_FRAMEBUFFER, masked_out_fbo);
  glDrawBuffers(1, drawbufs);
  glClear(GL_COLOR_BUFFER_BIT);
  glViewport(0.0f, 0.0f, settings.image_processing_w, settings.image_processing_h);
  
  glUseProgram(masked_out_prog);
  glUniformMatrix4fv(glGetUniformLocation(masked_out_prog, "u_pm"), 1, GL_FALSE, pm.ptr());
  glUniformMatrix4fv(glGetUniformLocation(masked_out_prog, "u_mm"), 1, GL_FALSE, mm.ptr());
  glUniform1i(glGetUniformLocation(masked_out_prog, "u_mask_tex"), 0);
  glUniform1i(glGetUniformLocation(masked_out_prog, "u_depth_tex"), 1);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, mask_tex);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, depth_tex);

  glBindVertexArray(masked_out_vao);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  // apply blur + threshold
  glBindFramebuffer(GL_READ_FRAMEBUFFER, masked_out_fbo);
  glReadBuffer(GL_COLOR_ATTACHMENT0);
  blur.blur();
  blur.setAsReadBuffer();
  thresh.threshold();

  glViewport(0.0f, 0.0f, settings.win_w, settings.win_h);

#if 1
  //  glReadBuffer(GL_COLOR_ATTACHMENT0);
  // Download the pixels e.g.
  glBindTexture(GL_TEXTURE_2D, thresh.output_tex);
  glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE, masked_out_pixels);
#endif

  #if 0
  static int c = 0;
  if(c < 100) {
    char n[512];
    sprintf(n, "%04d.png", c++);
    rx_save_png(n, masked_out_pixels, 640, 480, 1);
  }
  #endif
}

void Mask::drawThresholded() {
  graphics.drawTexture(thresh.output_tex, 0.0, 0.0, settings.win_w, settings.win_h);
}

// Draws the masked out scene (this will be the final result) 
void Mask::maskOutScene() {

  mat4 mm;
  mat4 pm;

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glBindVertexArray(masked_out_vao);
  glUseProgram(masked_scene_prog);
  glUniformMatrix4fv(glGetUniformLocation(masked_scene_prog, "u_pm"), 1, GL_FALSE, pm.ptr());
  glUniformMatrix4fv(glGetUniformLocation(masked_scene_prog, "u_mm"), 1, GL_FALSE, mm.ptr());
  glUniform1i(glGetUniformLocation(masked_scene_prog, "u_mask_tex"), 0);
  glUniform1i(glGetUniformLocation(masked_scene_prog, "u_scene_tex"), 1);
  glUniform1i(glGetUniformLocation(masked_scene_prog, "u_thresh_tex"), 2);
  glUniform1i(glGetUniformLocation(masked_scene_prog, "u_draw_hand"), draw_hand);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, mask_tex);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, scene_tex);

  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, thresh.output_tex);

  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glDisable(GL_BLEND);
}

void Mask::maskOutTexture(GLuint tex) {

  glBindVertexArray(graphics.tex_vao);
  glUseProgram(mask_tex_prog);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, mask_tex);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, tex);

  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void Mask::print() {
  printf("mask.scene_tex: %d\n", scene_tex);
  printf("mask.mask_tex: %d\n", mask_tex);
  printf("mask.masked_out_tex: %d\n", masked_out_tex);
  printf("mask.depth_tex: %d\n", depth_tex);
  printf("mask.thresh.output_tex: %d\n", thresh.output_tex);
  printf("--\n");
}


void Mask::refresh() {
  assert(settings.color_dx < settings.colors.size());
  glUseProgram(masked_scene_prog);
  glUniform3fv(glGetUniformLocation(masked_scene_prog, "u_hand_col"), 1, settings.colors[settings.color_dx].hand.ptr());
}

void Mask::drawHand() {

#if 0 
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  maskOutTexture(thresh.output_tex);
#else 

  // last minute changes to the hand.
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  glBindVertexArray(graphics.tex_vao);
  glUseProgram(hand_prog);

  mat4 hmm;
  float hw = settings.win_w * 0.5;
  float hh = settings.win_h * 0.5;
  int hand_scale_x = ((hand_flip_x) ? -1 : 1);
  int hand_scale_y = ((hand_flip_y) ? -1 : 1);
  hmm.identity();
  hmm.translate(hw, hh, 0.0);
  hmm.rotateZ(hand_rotation * DEG_TO_RAD);
  hmm.scale(hw * hand_scale_x, hh * hand_scale_y, 1.0f);

  glUniformMatrix4fv(glGetUniformLocation(hand_prog, "u_mm"), 1, GL_FALSE, hmm.ptr());
  glUniform3fv(glGetUniformLocation(hand_prog, "u_hand_color"), 1, settings.curr_colors.hand.ptr());

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, thresh.output_tex);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, mask_tex);

  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glDisable(GL_BLEND);

#endif
}

void Mask::setScale(float s) {
  s = CLAMP(s, 0.0f, 1.0f);
  scale = s;
}
