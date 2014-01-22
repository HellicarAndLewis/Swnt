#version 150

uniform sampler2D u_height_tex;
uniform sampler2D u_vel_tex;
uniform sampler2D u_noise_tex;
uniform float u_time;
out vec3 out_pos;
in vec2 v_tex;

const float size_x = 35.0;
const float size_y = 35.0;
const float field_size = 128.0;
const float half_x = size_x * 0.5;
const float half_y = size_y * 0.5;

void main() {
  float u = texture(u_height_tex,v_tex).r;
  float noise = texture(u_noise_tex, vec2(v_tex.s + u_time, v_tex.t)).r;

  out_pos = vec3(-half_x + (v_tex.s * size_x), 
                 u + (noise * 3.3), 
                 -half_y + (v_tex.t * size_y));
}
