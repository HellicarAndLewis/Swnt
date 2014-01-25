#include <swnt/Graphics.h>

Graphics::Graphics(Settings& settings)
  :settings(settings)
  ,v_vs(0)
  ,v_fs(0)
  ,v_prog(0)
  ,circle_resolution(30)
{
}

Graphics::~Graphics() {
  printf("@todo cleanup GL in Graphics.\n");
}

bool Graphics::setup() {

  glGenVertexArrays(1, &tex_vao);
  tex_pm.ortho(0.0f, settings.win_w, settings.win_h, 0.0f, 0.0f, 100.0f);

  // Texture shader.
  tex_vs = rx_create_shader(GL_VERTEX_SHADER, TEX_VS);
  tex_fs = rx_create_shader(GL_FRAGMENT_SHADER, TEX_FS);
  tex_prog = rx_create_program(tex_vs, tex_fs);
  glLinkProgram(tex_prog);
  rx_print_shader_link_info(tex_prog);
  glUseProgram(tex_prog);
  glUniformMatrix4fv(glGetUniformLocation(tex_prog, "u_pm"), 1, GL_FALSE, tex_pm.ptr());
  glUniform1i(glGetUniformLocation(tex_prog, "u_tex"), 0);

  // Kinect depth
  kinect_fs = rx_create_shader(GL_FRAGMENT_SHADER, KINECT_FS);
  kinect_prog = rx_create_program(tex_vs, kinect_fs);
  glLinkProgram(kinect_prog);
  rx_print_shader_link_info(kinect_prog);
  glUseProgram(kinect_prog);
  glUniform1i(glGetUniformLocation(kinect_prog, "u_tex"), 0);
  glUniform2f(glGetUniformLocation(kinect_prog, "u_dist"), settings.kinect_near, settings.kinect_far);

  // Debug shader
  v_vs = rx_create_shader(GL_VERTEX_SHADER, V_VS);
  v_fs = rx_create_shader(GL_FRAGMENT_SHADER, V_FS);
  v_prog = rx_create_program(v_vs, v_fs);
  glBindAttribLocation(v_prog, 0, "a_pos");
  glLinkProgram(v_prog);
  rx_print_shader_link_info(v_prog);
  glUseProgram(v_prog);

  float col[3] = {1.0, 0.0, 0.0};
  glUniform3fv(glGetUniformLocation(v_prog, "u_color"), 1, col);

  // Create circle
  {
    glGenVertexArrays(1, &circle_vao);
    glBindVertexArray(circle_vao);
    glGenBuffers(1, &circle_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, circle_vbo);
    glEnableVertexAttribArray(0); // pos
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (GLvoid*)0);

    std::vector<vec2> circle;
    float a = TWO_PI / (circle_resolution-1);
    circle.push_back(vec2());
    for(int i = 0; i <= circle_resolution; ++i) {
      circle.push_back(vec2(cos(a * i), sin(a * i)));
    }

    glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * circle.size(), circle[0].ptr(), GL_STATIC_DRAW);
  }
  return true;
}

void Graphics::drawTexture(GLuint tex, float x, float y, float w, float h) {

  glUseProgram(tex_prog);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex);
  glBindVertexArray(tex_vao);

  float hw = w * 0.5;
  float hh = h * 0.5;
  tex_mm.identity();
  tex_mm.translate(x + hw, y + hh, 0.0f);
  tex_mm.scale(hw, hh, 1.0f);

  glUniformMatrix4fv(glGetUniformLocation(tex_prog, "u_mm"), 1, GL_FALSE, tex_mm.ptr());
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void Graphics::drawDepth(GLuint tex, float x, float y, float w, float h) {
  glUseProgram(kinect_prog);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex);
  glBindVertexArray(tex_vao);

  float hw = w * 0.5;
  float hh = h * 0.5;
  tex_mm.identity();
  tex_mm.translate(x + hw, y + hh, 0.0f);
  tex_mm.scale(hw, hh, 1.0f);

  glUniformMatrix4fv(glGetUniformLocation(kinect_prog, "u_pm"), 1, GL_FALSE, settings.depth_ortho_matrix.ptr());
  glUniformMatrix4fv(glGetUniformLocation(kinect_prog, "u_mm"), 1, GL_FALSE, tex_mm.ptr());
  glUniform2f(glGetUniformLocation(kinect_prog, "u_dist"), settings.kinect_near, settings.kinect_far);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void Graphics::drawCircle(float x, float y, float radius, vec3 color) {
  mat4 mm;
  mm.translate(x, y, 0);
  mm.scale(radius, radius, radius);

  glBindVertexArray(circle_vao);
  glUseProgram(v_prog);
  glUniform3fv(glGetUniformLocation(v_prog, "u_color"), 1, color.ptr());
  glUniformMatrix4fv(glGetUniformLocation(v_prog, "u_mm"), 1, GL_FALSE, mm.ptr());
  glUniformMatrix4fv(glGetUniformLocation(v_prog, "u_pm"), 1, GL_FALSE, settings.ortho_matrix.ptr());
  glDrawArrays(GL_TRIANGLE_FAN, 0, circle_resolution + 1);
}

GLuint Graphics::createTexture(std::string filepath) {
  unsigned char* pixels = NULL;
  int w, h, nchannels = 0;

  if(!rx_load_png(filepath, &pixels, w, h, nchannels)) {
    if(pixels) {
      printf("Error: pixels was allocated... this shouldn't be!\n");
      ::exit(EXIT_FAILURE);
    }
    printf("Error: cannot load the PNG file: %s\n", filepath.c_str());
    return false;
  }

  GLenum format;
  switch(nchannels) {
    case 1:  { format = GL_RED;  break; } 
    case 2:  { format = GL_RG;   break; } 
    case 3:  { format = GL_RGB;  break; } 
    case 4:  { format = GL_RGBA; break; } 
    default: { printf("Error: unhandled format for channels: %d\n", nchannels); ::exit(EXIT_FAILURE); } 
  }

  GLuint t = 0;
  glGenTextures(1, &t);
  glBindTexture(GL_TEXTURE_2D, t);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, format, GL_UNSIGNED_BYTE, pixels);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  delete[] pixels;
  pixels = NULL;

#if !defined(NDEBUG)
  printf("image.width: %d\n", w);
  printf("image.height: %d\n", h);
  printf("image.nchannels: %d\n", nchannels);
#endif

  return t;
}
