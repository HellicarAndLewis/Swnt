/*

  Height Field Water Simulation
  -----------------------------
  - See http://www.matthiasmueller.info/talks/gdc2008.pdf
  - The value of c must be limited to: 1.0/dt, so 30fps, 1.0/0.033 = 33.33.

 */

#version 150

uniform float u_force;                /* the force we apply using the force texture */
uniform sampler2D u_force_tex;        /* texture which will contain the extra force that we add to the height field */
uniform sampler2D u_height_tex;       /* the current height values */
uniform sampler2D u_vel_tex;          /* the current velocity values; that we use to diffuse */

uniform float u_dt;
out float out_height;
out float out_vel;
in vec2 v_tex;

float get_force(float f0, vec2 f1) {
  float k = u_force;
  return f0 + (f1.r * k) - (f1.g * k);
}

void main() {

  float u0        = texture(u_height_tex, v_tex).r;
  vec2 u1         = texture(u_force_tex, v_tex).rg;

  float u_right0  = textureOffset(u_height_tex, v_tex, ivec2( 1,  0)).r;
  float u_left0   = textureOffset(u_height_tex, v_tex, ivec2(-1,  0)).r;
  float u_top0    = textureOffset(u_height_tex, v_tex, ivec2( 0, -1)).r;
  float u_bottom0 = textureOffset(u_height_tex, v_tex, ivec2( 0,  1)).r;

  vec2 u_right1  = textureOffset(u_force_tex, v_tex, ivec2( 1,  0)).rg;
  vec2 u_left1   = textureOffset(u_force_tex, v_tex, ivec2(-1,  0)).rg;
  vec2 u_top1    = textureOffset(u_force_tex, v_tex, ivec2( 0, -1)).rg;
  vec2 u_bottom1 = textureOffset(u_force_tex, v_tex, ivec2( 0,  1)).rg;

  float u = get_force(u0, u1);
  float u_right = get_force(u_right0, u_right1);
  float u_left = get_force(u_left0, u_left1);
  float u_top = get_force(u_top0, u_top1);
  float u_bottom = get_force(u_bottom0, u_bottom1);

  float c = 50.0;
  float f = ((c*c) * ((u_right + u_left + u_top + u_bottom) - (4.0 * u)) ) / 4.0;
  
  // damping
  float mv = 40.0;
  if(f > mv) {
    f = mv; 
  } 
  else if(f < -mv) {
    f = -mv; 
  }
 
  float v = texture(u_vel_tex, v_tex).r;
  float v_new = v + f * u_dt;

  out_height = u * 0.99 + v_new * u_dt;
  out_vel = v_new * 0.99;
}
