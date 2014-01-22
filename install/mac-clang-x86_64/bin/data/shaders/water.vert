#version 150

in vec2 a_tex;

uniform mat4 u_pm;
uniform mat4 u_vm;
uniform sampler2D u_pos_tex;
uniform sampler2D u_norm_tex;

out vec2 v_tex;
out vec3 v_norm;
out vec3 v_eye_pos;
out vec3 v_world_pos;

void main() {

  vec4 world_pos = vec4(texelFetch(u_pos_tex, ivec2(a_tex), 0).rgb, 1.0);
  gl_Position = u_pm * u_vm * world_pos;

  v_norm = texelFetch(u_norm_tex, ivec2(a_tex), 0).rgb;
  v_tex = vec2(a_tex.x / 128.0, a_tex.y / 128.0);
  v_eye_pos = vec3(u_vm * world_pos);
  v_world_pos = world_pos.xyz;
}

