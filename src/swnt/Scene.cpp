#include <assert.h>
#include <swnt/Scene.h>

Scene::Scene() 
  :win_w(0)
  ,win_h(0)
  ,vao(0)
  ,vbo(0)
  ,vert(0)
  ,frag(0)
  ,prog(0)
{
}

bool Scene::setup(int w, int h) {
  assert(w && h);

  win_w = w;
  win_h = h;

  OBJ obj;

  if(!obj.load(rx_to_data_path("landscape.obj"))) {
    printf("Error: cannot load the scene obj file.\n");
    return false;
  }

  if(!obj.hasNormals()) {
    printf("Error: loaded .obj does not have normals. \n");
    return false;
  }

  obj.copy(vertices);
  assert(vertices.size());

  printf("Loaded: %ld vertices.\n", vertices.size());
  
  // shader
  vert = rx_create_shader(GL_VERTEX_SHADER, SCENE_VS);
  frag = rx_create_shader(GL_FRAGMENT_SHADER, SCENE_FS);
  prog = rx_create_program(vert, frag);
  glBindAttribLocation(prog, 0, "a_pos");
  glBindAttribLocation(prog, 1, "a_norm");
  glLinkProgram(prog);
  rx_print_shader_link_info(prog);

  // buffers
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  glEnableVertexAttribArray(0); // pos
  glEnableVertexAttribArray(1); // norm

  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(VertexPN) * vertices.size(), vertices[0].ptr(), GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPN), (GLvoid*)0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPN), (GLvoid*)12);
  
  return true;
}

void Scene::draw(float* pm, float* vm) {

  mm.identity();
  mm.scale(5.0);

  glBindVertexArray(vao);
  glUseProgram(prog);
  rx_uniform_mat4fv(prog, "u_pm", 1, GL_FALSE, pm);
  rx_uniform_mat4fv(prog, "u_vm", 1, GL_FALSE, vm);
  rx_uniform_mat4fv(prog, "u_mm", 1, GL_FALSE, mm.ptr());

  glDrawArrays(GL_TRIANGLES, 0, vertices.size());
}
