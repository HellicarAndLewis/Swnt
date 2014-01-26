#version 150

const vec2 pos[] = vec2[4](
  vec2(-1.0, 1.0),
  vec2(-1.0, -1.0),
  vec2(1.0,   1.0),
  vec2(1.0,  -1.0)
);

const vec2 tex[4] = vec2[](
  vec2(0.0, 1.0),  
  vec2(0.0, 0.0), 
  vec2(1.0, 1.0), 
  vec2(1.0, 0.0)
);

out vec2 v_tex;
out vec2 v_pos;

void main() {

  gl_Position = vec4(pos[gl_VertexID], 0.0, 1.0);
  v_tex = tex[gl_VertexID];
  v_pos = pos[gl_VertexID];
}
