#include <assert.h>
#include <ocean/WaterGraphics.h>


// ---------------------------------------------
GLint ufl(GLuint prog, const char* name) {
  GLint u = glGetUniformLocation(prog, name);
  if(u < 0) {
    printf("error: cannot find `%s` in prog `%d`\n", name, prog);
    ::exit(EXIT_FAILURE);
  }
  return u;
}


// ---------------------------------------------

WaterGraphics::WaterGraphics(OceanSettings& settings) 
  :settings(settings)
  ,win_w(0)
  ,win_h(0)
{
}

WaterGraphics::~WaterGraphics() {
}

bool WaterGraphics::setup(int winW, int winH) {
  win_w = winW;
  win_h = winH;

  // Fullscreen shader.
  fullscreen_vs = rx_create_shader(GL_VERTEX_SHADER, FULLSCREEN_VS);
  fullscreen_fs = rx_create_shader(GL_FRAGMENT_SHADER, FULLSCREEN_FS);
  fullscreen_prog = rx_create_program(fullscreen_vs, fullscreen_fs);
  glLinkProgram(fullscreen_prog);
  rx_print_shader_link_info(fullscreen_prog);

  // Horizontal Sub transform
  hor_subtrans_fs = rx_create_shader(GL_FRAGMENT_SHADER, SUBTRANSFORM_HOR_FS);
  hor_subtrans_prog = rx_create_program(fullscreen_vs, hor_subtrans_fs);
  glLinkProgram(hor_subtrans_prog);
  rx_print_shader_link_info(hor_subtrans_prog);
  glUseProgram(hor_subtrans_prog);
  glUniform1f(ufl(hor_subtrans_prog, "u_transform_size"), RESOLUTION);

  // Vertical Sub transform
  vert_subtrans_fs = rx_create_shader(GL_FRAGMENT_SHADER, SUBTRANSFORM_VERT_FS);
  vert_subtrans_prog = rx_create_program(fullscreen_vs, vert_subtrans_fs);
  glLinkProgram(vert_subtrans_prog);
  rx_print_shader_link_info(vert_subtrans_prog);
  glUseProgram(vert_subtrans_prog);
  glUniform1f(ufl(vert_subtrans_prog, "u_transform_size"), RESOLUTION);

  // Initial spectrum
  init_spectrum_fs = rx_create_shader(GL_FRAGMENT_SHADER, INIT_SPECTRUM_FS);
  init_spectrum_prog = rx_create_program(fullscreen_vs, init_spectrum_fs);
  glLinkProgram(init_spectrum_prog);
  rx_print_shader_link_info(init_spectrum_prog);
  glUseProgram(init_spectrum_prog);
  glUniform1f(ufl(init_spectrum_prog, "u_resolution"), RESOLUTION);

  // Phase 
  phase_fs = rx_create_shader(GL_FRAGMENT_SHADER, PHASE_FS);
  phase_prog = rx_create_program(fullscreen_vs, phase_fs);
  glLinkProgram(phase_prog);
  rx_print_shader_link_info(phase_prog);
  glUseProgram(phase_prog);
  glUniform1f(ufl(phase_prog, "u_resolution"), RESOLUTION);

  // Spectrum
  spectrum_fs = rx_create_shader(GL_FRAGMENT_SHADER, SPECTRUM_FS);
  spectrum_prog = rx_create_program(fullscreen_vs, spectrum_fs);
  glLinkProgram(spectrum_prog);
  rx_print_shader_link_info(spectrum_prog);
  glUseProgram(spectrum_prog);
  glUniform1f(ufl(spectrum_prog, "u_resolution"), RESOLUTION);
  glUniform1i(ufl(spectrum_prog, "u_initial_spectrum"), INITIAL_SPECTRUM_UNIT);

  // Normal map
  normal_map_fs = rx_create_shader(GL_FRAGMENT_SHADER, NORMAL_MAP_FS);
  normal_map_prog = rx_create_program(fullscreen_vs, normal_map_fs);
  glLinkProgram(normal_map_prog);
  rx_print_shader_link_info(normal_map_prog);
  glUseProgram(normal_map_prog);
  glUniform1f(ufl(normal_map_prog, "u_resolution"), RESOLUTION);
  glUniform1i(ufl(normal_map_prog, "u_displacement_map"), DISPLACEMENT_MAP_UNIT);

  // Ocean
  {
    ocean_vs = rx_create_shader(GL_VERTEX_SHADER, OCEAN_VS);
    ocean_fs = rx_create_shader(GL_FRAGMENT_SHADER, OCEAN_FS);
    ocean_prog = rx_create_program(ocean_vs, ocean_fs);
    glBindAttribLocation(ocean_prog, 0, "a_pos");
    glBindAttribLocation(ocean_prog, 1, "a_tex");
    glLinkProgram(ocean_prog);
    rx_print_shader_link_info(ocean_prog);


    float ocean_color[] = { 0.004f, 0.016f, 0.047f };
    float sky_color[] = { 3.2f, 12.6f, 12.8f } ;
    float sun_direction[] = {1.0f, 1.0f, 1.0f}; // {-1.0f, 1.0f, 1.0f};
    float exposure = 0.35f;

    glUseProgram(ocean_prog);
    glUniform1f(ufl(ocean_prog, "u_geometry_size"), GEOMETRY_SIZE);
    glUniform1i(ufl(ocean_prog, "u_displacement_map"), DISPLACEMENT_MAP_UNIT);
    glUniform1i(ufl(ocean_prog, "u_normal_map"), NORMAL_MAP_UNIT);
    glUniform3fv(ufl(ocean_prog, "u_ocean_color"), 1, ocean_color);
    glUniform3fv(ufl(ocean_prog, "u_sky_color"), 1, sky_color);
    glUniform3fv(ufl(ocean_prog, "u_sun_direction"), 1, sun_direction);
    glUniform1f(ufl(ocean_prog, "u_exposure"), exposure);
  }

  glGenVertexArrays(1, &fullscreen_vao);

  printf("--\n");
  printf("fullscreen_prog: %d\n", fullscreen_prog);
  printf("hor_subtrans_prog: %d\n", hor_subtrans_prog);
  printf("vert_subtrans_prog: %d\n", vert_subtrans_prog);
  printf("init_spectrum_prog: %d\n", init_spectrum_prog);
  printf("phase_prog: %d\n", phase_prog);
  printf("spectrum_prog: %d\n", spectrum_prog);
  printf("normal_map_prog: %d\n", normal_map_prog);
  printf("ocean_prog; %d\n", ocean_prog);  
  printf("--\n");

  return true;
}
