#version 150

#define DEBUG 0

uniform sampler2D u_normal_tex;
uniform sampler2D u_diffuse_tex;
uniform float u_age_perc;
uniform float u_time;
out vec4 fragcolor;
in vec2 v_tex;

void main() {
  vec2 texcoord = vec2(v_tex.t, v_tex.s - u_time * 0.1);
  vec4 normal = texture(u_normal_tex, texcoord);
  vec4 tc = texture(u_diffuse_tex, vec2(texcoord.s + normal.r, texcoord.t + normal.t));

  fragcolor.rgb = vec3(1.0) * tc.r ;
  //fragcolor.a = 0.2 * (1.0 - u_age_perc) * tc.r *  tc.a; //  * (pow(1.0 - v_tex.s, 5.0)) ;
  float k = 0.2 * (1.0 - u_age_perc) * tc.r *  tc.a; //  * (pow(1.0 - v_tex.s, 5.0)) ;
  fragcolor.a = (k > 0.1) ? 1.0 : 0.0;
  //  fragcolor.rgb = tc.rgb;
  //  fragcolor.a = v_tex.s * 0.4;
  //  fragcolor.a = 1.0;
#if DEBUG
  fragcolor.rgb = vec3(1.0, 0.0, 0.0);
  fragcolor.a = 1.0;
#endif

}
