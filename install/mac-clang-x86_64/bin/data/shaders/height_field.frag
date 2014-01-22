/*

  Height Field Water Simulation
  -----------------------------
  - See http://www.matthiasmueller.info/talks/gdc2008.pdf
  - The value of c must be limited to: 1.0/dt, so 30fps, 1.0/0.033 = 33.33.

 */

#version 150

uniform sampler2D u_force_tex;        /* texture which will contain the extra force that we add to the height field */
uniform sampler2D u_height_tex;       /* the current height values */
uniform sampler2D u_vel_tex;          /* the current velocity values; that we use to diffuse */
uniform float u_dt;
out vec4 out_height;
out vec4 out_vel;
in vec2 v_tex;

float get_force(float f0, vec2 f1) {
  // float k = 0.001;
  //return f0 + max((f1.r * k),0.0) - min((f1.g * k), 0.0);
  return f0 + (f1.r * 5.2) - (f1.g * 5.2);
  //return f0;
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

#if 0
  u1 = vec2(0.0);
  u_right1 = vec2(0.0);
  u_left1 = vec2(0.0);
  u_top1 = vec2(0.0);
  u_bottom1 = vec2(0.0);
#endif

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
  } else if(f < -mv) {
    f = -mv; 
  }

 
  float v = texture(u_vel_tex, v_tex).r;
  float v_new = v + f * u_dt;
  float k = 0.0;

#if 0
  if(u1.r > 0.0 || u1.g > 0.0) {
    float km = 2.5;
    k = min(km * u1.r, km) + max(km * u1.g, -km);
  }
#endif

  out_height = vec4(k + u * 0.99 + v_new * u_dt);
  out_vel = vec4(v_new * 0.99);
}
