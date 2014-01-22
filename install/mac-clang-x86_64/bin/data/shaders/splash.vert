#version 150
uniform mat4 u_pm;
uniform mat4 u_mm;
in vec4 a_pos;
in vec2 a_tex;
out vec2 v_tex;

void main() {
  gl_Position = u_pm * u_mm * a_pos;
  v_tex = a_tex;
}
