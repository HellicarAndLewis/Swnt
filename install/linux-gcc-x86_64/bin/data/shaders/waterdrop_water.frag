#version 150

#define RENDER_SHADED 1
#define RENDER_NORMALS 0
#define RENDER_BACKGROUND 0
#define RENDER_GRAYSCALE 0

uniform float u_time;
uniform sampler2D u_normals_tex;
uniform sampler2D u_alpha_tex;
uniform sampler2D u_background_tex;

in vec2 v_tex;
in vec2 v_pos;
out vec4 fragcolor;

void main() {

  vec3 normal = texture(u_normals_tex, v_tex).rgb;
  vec3 alpha = texture(u_alpha_tex, v_tex).rgb;

  fragcolor.a = alpha.r;

#if RENDER_NORMALS
  fragcolor.rgb = normalize(normal);
#endif  

#if RENDER_BACKGROUND
  vec3 background = texture(u_background_tex, v_tex).rgb;
  fragcolor.rgb = background;
#endif

#if RENDER_SHADED

#if 1 
#if 1
  normal = normalize(normal);
  float influence = 0.1;
  vec2 offset_uv = vec2(v_tex.s + normal.r * influence + u_time * 0.0, 
                        v_tex.t + normal.g * influence);
  vec3 background = texture(u_background_tex, offset_uv).rgb;
  //float k = pow(alpha.r, 5.0);
  float k = alpha.r;
#if 0
  //k = (k >= 0.7) ? 1.0 : 0.0;
  fragcolor.rgb = background * k;
#else
  k = (k >= 0.7) ? 1.0 : 0.0;
  fragcolor.rgb = vec3(k);
#endif

  fragcolor.a = k;

#else
  // based on skype chat with carlos
  float influence = 0.0;
  vec2 coords = normalize(normal * 2 - 1).rg;
  vec3 background = texture(u_background_tex, 
                            vec2(clamp(v_tex.s + coords.x * influence, 0.0, 1.0), 
                                 clamp(v_tex.t + coords.y * influence, 0.0, 1.0))).rgb;

  float k = alpha.r;
  k = (k >= 0.7) ? 1.0 : 0.0;
  fragcolor.rgb = background * k;
  fragcolor.a = k;

#endif
#endif


#endif


#if RENDER_GRAYSCALE
  float k = (alpha.r >= 0.7) ? 1.0 : 0.0;
  fragcolor.rgb = vec3(k);
  //fragcolor.rgb = alpha;
  fragcolor.a = k;
#endif
}

