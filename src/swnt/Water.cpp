#include <swnt/Water.h>
#include <swnt/Settings.h>
#include <swnt/Flow.h>

Water::Water(Settings& settings, Flow& flow)
  :settings(settings)
  ,flow(flow)
  ,vbo(0)
  ,vao(0)
  ,vert(0)
  ,frag(0)
  ,prog(0)
  ,height_tex(0)
{
}

bool Water::setup() {
  assert(flow.field_size);
  
  // matrices
  pm.perspective(45.0, float(settings.win_w)/settings.win_h, 0.01, 100.0f);
  vm.lookAt(vec3(0.0f, 7.0f, 0.0f), vec3(0.01f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));

  nm[0] = vm[0];
  nm[1] = vm[4];
  nm[2] = vm[8];

  nm[3] = vm[1];
  nm[4] = vm[5];
  nm[5] = vm[9];

  nm[6] = vm[2];
  nm[7] = vm[6];
  nm[8] = vm[10];

  // shaders
  vert = rx_create_shader(GL_VERTEX_SHADER, WATER_VS);
  frag = rx_create_shader(GL_FRAGMENT_SHADER, WATER_FS);
  prog = rx_create_program(vert, frag);
  glBindAttribLocation(prog, 0, "a_pos");
  glLinkProgram(prog);
  rx_print_shader_link_info(prog);
  glUseProgram(prog);
  glUniform1i(glGetUniformLocation(prog, "u_height_tex"), 0);
  
  // data
  std::vector<vec3> vertices;
  std::vector<GLuint> indices;
  vertices.assign(flow.field_size * flow.field_size, vec3());
  float size = 0.1; 
  float hs = (flow.field_size * size) * 0.5;

  for(int j = 0; j < flow.field_size; ++j) {
    for(int i = 0; i < flow.field_size; ++i) {
      int dx = (j * flow.field_size) + i;
      vertices[dx].set(-hs + (i * size), 0.0, -hs + (j * size));
    }
  }

  for(int j = 0; j < flow.field_size-1; ++j) {
    for(int i = 0; i < flow.field_size-1; ++i) {
      GLuint a = ((j + 0) * flow.field_size) + (i + 0);  // bottom left
      GLuint b = ((j + 0) * flow.field_size) + (i + 1);  // bottom right
      GLuint c = ((j + 1) * flow.field_size) + (i + 1);  // top right 
      GLuint d = ((j + 1) * flow.field_size) + (i + 0);  // top left

      indices.push_back(a);
      indices.push_back(b);
      indices.push_back(c);
      
      indices.push_back(a);
      indices.push_back(c);
      indices.push_back(d);
    }
  }

  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  glGenBuffers(1, &vbo_els);
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_els);

  glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * vertices.size(), vertices[0].ptr(), GL_STATIC_DRAW);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), &indices[0], GL_STATIC_DRAW);


  glEnableVertexAttribArray(0); // pos
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (GLvoid*)0);

  // texture for the height values.
  glGenTextures(1, &height_tex);
  glBindTexture(GL_TEXTURE_2D, height_tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, flow.field_size, flow.field_size, 0, GL_RED, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  return true;
}

void Water::update() {
  assert(flow.heights.size());

  glBindTexture(GL_TEXTURE_2D, height_tex);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, flow.field_size, flow.field_size, GL_RED, GL_FLOAT, (GLvoid*)&flow.heights[0]);
}

void Water::draw() {

  glUseProgram(prog);
  glUniformMatrix4fv(glGetUniformLocation(prog, "u_pm"), 1, GL_FALSE, pm.ptr());
  glUniformMatrix4fv(glGetUniformLocation(prog, "u_vm"), 1, GL_FALSE, vm.ptr());
  glUniformMatrix3fv(glGetUniformLocation(prog, "u_nm"), 1, GL_FALSE, nm.ptr());

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, height_tex);

  glBindVertexArray(vao);
#if USE_SIMPLE_SHADER
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glDrawElements(GL_TRIANGLES, flow.field_size * flow.field_size * 6, GL_UNSIGNED_INT, 0);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

#else
#  if 1
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glDrawElements(GL_TRIANGLES, flow.field_size * flow.field_size * 6, GL_UNSIGNED_INT, 0);

  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glDrawElements(GL_TRIANGLES, flow.field_size * flow.field_size * 6, GL_UNSIGNED_INT, 0);

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glDisable(GL_BLEND);

#  else
  glDrawElements(GL_TRIANGLES, flow.field_size * flow.field_size * 6, GL_UNSIGNED_INT, 0);
#  endif
#endif

}
