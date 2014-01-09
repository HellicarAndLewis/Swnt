#include <swnt/Effects.h>
#include <swnt/Settings.h>
#include <swnt/effects/Eddy.h>

void EddyShape::reset() {
  scale_speed = rx_random(1.05f, 1.1f);
  rotate_speed = rx_random(0.1, 0.15f);
  lifetime = rx_random(3.5f, 7.5f);
  die_time = rx_millis() + lifetime;
}

Eddy::Eddy(Effects& effects)
  :BaseEffect(effects, EFFECT_EDDY)
  ,bytes_allocated(0)
  ,vert(0)
  ,frag(0)
  ,prog(0)
  ,vbo(0)
  ,vao(0)
  ,eddy_tex(0)
  ,u_time(0)
   //  ,u_perc(0)
  ,needs_update(false)
{
}

Eddy::~Eddy() {
}

bool Eddy::setup() {
  vert = rx_create_shader(GL_VERTEX_SHADER, EDDY_VS);
  frag = rx_create_shader(GL_FRAGMENT_SHADER, EDDY_FS);
  prog = rx_create_program(vert, frag);
  glBindAttribLocation(prog, 0, "a_pos");
  glBindAttribLocation(prog, 1, "a_tex");
  glBindAttribLocation(prog, 2, "a_alpha");
  glLinkProgram(prog);
  rx_print_shader_link_info(prog);

  glUseProgram(prog);
  glUniform1i(glGetUniformLocation(prog, "u_tex"), 0);
  u_time = glGetUniformLocation(prog, "u_time");
  //  u_perc = glGetUniformLocation(prog, "u_perc");

  assert(u_time >= 0);
  //  assert(u_perc >= 0);

  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);

  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(EddyVertex), (GLvoid*)0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(EddyVertex), (GLvoid*)12);
  glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(EddyVertex), (GLvoid*)24);

  createRing(effects.settings.win_w * 0.5, effects.settings.win_h * 0.5, 60.0f, 450.0, 0.0f, 1.0f);

  eddy_tex = rx_create_texture(rx_to_data_path("images/eddy.png")); // , GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
  if(!eddy_tex) {
    printf("Error: cannot create the eddy texture.\n");
    return false;
  }
  printf("eddy.eddy_tex: %d\n", eddy_tex);
  return true;
}

void Eddy::update() {

  if(!needs_update) {
    return;
  }

  size_t needed = sizeof(EddyVertex) * vertices.size();
  if(!needed) {
    return;
  }

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  if(needed > bytes_allocated) {
    glBufferData(GL_ARRAY_BUFFER, needed, vertices[0].ptr(), GL_DYNAMIC_DRAW);
    bytes_allocated = needed;
  }
  else {
    glBufferSubData(GL_ARRAY_BUFFER, 0, needed, vertices[0].ptr());
  }

  needs_update = false;
}

void Eddy::drawExtraFlow() {

#if USE_EDDY_DEBUG
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
#endif

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, eddy_tex);

  glUseProgram(prog);
  glBindVertexArray(vao);
  glUniformMatrix4fv(glGetUniformLocation(prog, "u_pm"), 1, GL_FALSE, effects.ortho_pm.ptr());

#define EDDY_ROTATE_SHAPES 1
#define EDDY_MULTI_DRAW 0

#if EDDY_ROTATE_SHAPES

  for(std::vector<EddyShape>::iterator it = shapes.begin(); it != shapes.end(); ++it) {
    EddyShape& shape = *it;
    float perc = 1.0f;
    glUniform1f(u_time, rx_millis() * shape.rotate_speed);
    //    glUniform1f(u_perc, perc);
    shape.mm.identity();
    shape.mm.translate(shape.x, shape.y, 0.0f);

    glUniformMatrix4fv(glGetUniformLocation(prog, "u_mm"), 1, GL_FALSE, shape.mm.ptr());
    glDrawArrays(GL_TRIANGLES, shape.offset, shape.count);
  }
#elif EDDY_MULTI_DRAW
  mat4 mm;
  glUniformMatrix4fv(glGetUniformLocation(prog, "u_mm"), 1, GL_FALSE, mm.ptr());
  glUniform1f(u_time, rx_millis() * 0.1);
  glMultiDrawArrays(GL_TRIANGLE_STRIP, &offsets.front(), &counts.front(), counts.size());
#else
  mat4 mm;
  glUniformMatrix4fv(glGetUniformLocation(prog, "u_mm"), 1, GL_FALSE, mm.ptr());
  for(size_t i = 0; i < counts.size(); ++i) {
    glUniform1f(u_time, rx_millis() * 0.1 * i);
    glDrawArrays(GL_TRIANGLE_STRIP, offsets[i], counts[i]);
  }
#endif

  glDisable(GL_BLEND);
}

void Eddy::createRing(float x, float y, float radius, float width, float fromAlpha, float toAlpha) {
  offsets.push_back(vertices.size());

  float resolution = 60.0;
  float angle = TWO_PI/resolution;
  float outer_radius = radius + width;

  for(int i = 0; i < resolution; ++i) {
    float c0 = cos( (i + 0) * angle);
    float s0 = sin( (i + 0) * angle);
    float c1 = cos( (i + 1) * angle);
    float s1 = sin( (i + 1) * angle);
    
    // positions
    vec3 pa(c0 * radius, s0 * radius, 0.0f);
    vec3 pb(c1 * radius, s1 * radius, 0.0f);
    vec3 pc(c1 * outer_radius, s1 * outer_radius, 0.0f);
    vec3 pd(c0 * outer_radius, s0 * outer_radius, 0.0f);

    // texcoords
    float u0 = float(i+0)/resolution;
    float u1 = float(i+1)/resolution;
    vec3 ta(u0, 0.0f, 1.0f);
    vec3 tb(u1, 0.0f, 1.0f);
    vec3 tc(u1, 1.0f, 1.0f);
    vec3 td(u0, 1.0f, 1.0f);

    // calculate distances from the corners to the centers
    vec3 intersection;
    if(!intersect(pa, pc, pb, pd, intersection)) {
      printf("The vertices of the dist do not intersect. Error.\n");
      ::exit(EXIT_FAILURE);
    }

    float d0 = length(pa - intersection);
    float d1 = length(pb - intersection);
    float d2 = length(pc - intersection);
    float d3 = length(pd - intersection);

    ta = ta * ((d0 + d2)/d2);
    tb = tb * ((d1 + d3)/d3);
    tc = tc * ((d2 + d0)/d0);
    td = td * ((d3 + d1)/d1);

    // store the vertices
    EddyVertex a(pa,ta, fromAlpha);
    EddyVertex b(pb,tb, fromAlpha);
    EddyVertex c(pc,tc, toAlpha);
    EddyVertex d(pd,td, toAlpha);

    vertices.push_back(a);
    vertices.push_back(b);
    vertices.push_back(c);

    vertices.push_back(a);
    vertices.push_back(c);
    vertices.push_back(d);
  }

  counts.push_back(vertices.size()-offsets.back());
  needs_update = true;

  EddyShape shape;
  shape.reset();
  shape.offset = offsets.back();
  shape.count = counts.back();
  shape.x = effects.settings.win_w * 0.5;
  shape.y = effects.settings.win_h * 0.5;
  shapes.push_back(shape);
}
