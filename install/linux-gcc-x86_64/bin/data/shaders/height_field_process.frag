/*

  - Uses the calculated position to calculate the height values.
  - I'm not sure is I need to calculate dx/dy using a stepsize of 2 or 1

 */
#version 150

uniform sampler2D u_pos_tex;
uniform sampler2D u_height_tex;
uniform sampler2D u_vel_tex;
out vec3 out_norm;
out vec2 out_tex;
in vec2 v_tex;

void main() {
  vec3 center = texture(u_pos_tex, v_tex).rgb;
  vec3 right  = textureOffset(u_pos_tex, v_tex, ivec2(1,  0)).rgb;
  vec3 top    = textureOffset(u_pos_tex, v_tex, ivec2(0, -1)).rgb;

#if 0
  vec3 dx = right - center;
  vec3 dy = top - center;
#else
  vec3 left   = textureOffset(u_pos_tex, v_tex, ivec2(-1, 0)).rgb;
  vec3 bottom = textureOffset(u_pos_tex, v_tex, ivec2( 0, 1)).rgb;
  vec3 dx = right - left;
  vec3 dy = top - bottom;
#endif

  out_norm = normalize(cross(dx, dy));
  out_tex = v_tex;
}

