#version 150

/* Used to draw extra forces which will be accumulated with the height field */
   
const vec2[] pos = vec2[4](
  vec2(-1.0, 1.0),
  vec2(-1.0, -1.0),
  vec2(1.0, 1.0),
  vec2(1.0, -1.0)
);

const vec2[] tex = vec2[4](
  vec2(0.0, 1.0),
  vec2(0.0, 0.0),
  vec2(1.0, 1.0),
  vec2(1.0, 0.0)
);

uniform mat4 u_pm;
uniform mat4 u_mm;

out vec2 v_tex;

void main() {
  vec4 vert = vec4(pos[gl_VertexID], 0.0, 1.0);
  gl_Position = u_pm * u_mm * vert;
  v_tex = tex[gl_VertexID];
}
