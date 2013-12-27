#include <swnt/Effects.h>
#include <swnt/Settings.h>
#include <swnt/Graphics.h>
#include <swnt/Spirals.h>

Effects::Effects(Settings& settings, Graphics& graphics, Spirals& spirals)
  :settings(settings)
  ,graphics(graphics)
  ,spirals(spirals)
  ,pt_vert(0)
  ,floor_vbo(0)
  ,floor_vao(0)
  ,floor_frag(0)
  ,floor_prog(0)
{
}

Effects::~Effects() {
  printf("Error: cleanup Effects in d'tor\n");

  for(std::vector<FloorItem*>::iterator it = floor_items.begin(); it != floor_items.end(); ++it) {
    delete *it;
  }
  floor_items.clear();
}

bool Effects::setup() {

  if(!setupShaders()) {
    printf("Error while trying to create the shaders for the effect.\n");
    return false;
  }

  if(!setupFloor()) {
    printf("Error: cannot setup the floor effect.\n");
    return false;
  }

  if(!setupDisplacement()) {
    printf("Error: cannot setup the displacement in effects.\n");
    return false;
  }

  if(!blur.setup(settings.win_w, settings.win_h)) {
    printf("Cannot setup blur");
    return false;
  }

  pm.perspective(45.0f, float(settings.win_w)/settings.win_h, 0.0f, 100.0f);
  vm.lookAt(vec3(0.0f, 10.0f, 0.0f), vec3(0.01, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));

  return true;
}

bool Effects::setupShaders() {

  // Floor effect.
  pt_vert = rx_create_shader(GL_VERTEX_SHADER, EFFECT_PT_VS);
  floor_frag = rx_create_shader(GL_FRAGMENT_SHADER, EFFECT_FLOOR_FS);
  floor_prog = rx_create_program(pt_vert, floor_frag);
  glBindAttribLocation(floor_prog, 0, "a_pos");
  glBindAttribLocation(floor_prog, 1, "a_tex");
  glLinkProgram(floor_prog);
  rx_print_shader_link_info(floor_prog);
  glUseProgram(floor_prog);
  glUniform1i(glGetUniformLocation(floor_prog, "u_tex"), 0);

  // Displacement effect
  disp_frag = rx_create_shader(GL_FRAGMENT_SHADER, DISP_FRAG_FS);
  disp_prog = rx_create_program(graphics.tex_vs, disp_frag);
  glLinkProgram(disp_prog);
  glUseProgram(disp_prog);
  glUniform1i(glGetUniformLocation(disp_prog, "u_displacement_tex"), 0);
  glUniform1i(glGetUniformLocation(disp_prog, "u_scene_tex"), 1);

  return true;
}

bool Effects::setupFloor() {
  
  glGenVertexArrays(1, &floor_vao);
  glBindVertexArray(floor_vao);
  glGenBuffers(1, &floor_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, floor_vbo);
  glEnableVertexAttribArray(0); // pos;
  glEnableVertexAttribArray(1); // tex;
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPT), (GLvoid*)0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexPT), (GLvoid*)12);

  float s = 4.5;
  VertexPT a(vec3(-s, 0.0f,  s), vec2(0.0f, 0.0f));
  VertexPT b(vec3( s, 0.0f,  s), vec2(1.0f, 0.0f));
  VertexPT c(vec3( s, 0.0f, -s), vec2(1.0f, 1.0f));
  VertexPT d(vec3(-s, 0.0f, -s), vec2(0.0f, 1.0f));

  std::vector<VertexPT> vertices;
  vertices.push_back(a);
  vertices.push_back(b);
  vertices.push_back(c);
  vertices.push_back(a);
  vertices.push_back(c);
  vertices.push_back(d);
  
  glBufferData(GL_ARRAY_BUFFER, sizeof(VertexPT) * vertices.size(), vertices[0].ptr(), GL_STATIC_DRAW);
  floor_tex = graphics.createTexture(rx_to_data_path("images/floor.png"));
  
  int num = 1;
  for(int i = 0; i < num; ++i) {
    FloorItem* fi = new FloorItem();
    floor_items.push_back(fi);
  }
  return true;
}

bool Effects::setupDisplacement() {
  glGenVertexArrays(1, &disp_vao);
  return true;
}

void Effects::displace(GLuint dispTex, GLuint sceneTex) {
  mat4 ident;

  glUseProgram(disp_prog);
  glBindVertexArray(disp_vao);

  glUniformMatrix4fv(glGetUniformLocation(disp_prog, "u_pm"), 1, GL_FALSE, ident.ptr());
  glUniformMatrix4fv(glGetUniformLocation(disp_prog, "u_mm"), 1, GL_FALSE, ident.ptr());

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, dispTex);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, sceneTex);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

}

void Effects::drawFloor() {
  return;
  glBindVertexArray(floor_vao);
  glUseProgram(floor_prog);
  glEnable(GL_BLEND);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, floor_tex);

  glUniformMatrix4fv(glGetUniformLocation(floor_prog, "u_pm"), 1, GL_FALSE, pm.ptr());
  glUniformMatrix4fv(glGetUniformLocation(floor_prog, "u_vm"), 1, GL_FALSE, vm.ptr());

  GLint u_mm = glGetUniformLocation(floor_prog, "u_mm");
  for(size_t i = 0; i < floor_items.size(); ++i) {
    FloorItem* fi = floor_items[i];
    fi->mm.rotateY(0.01);
    glUniformMatrix4fv(u_mm, 1, GL_FALSE, fi->mm.ptr());
    glDrawArrays(GL_TRIANGLES, 0, 6);
  }
}


