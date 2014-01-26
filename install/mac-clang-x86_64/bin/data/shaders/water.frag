#version 150

uniform mat4 u_vm;                     /* view matrix */
uniform float u_time0;                 /* the first time ramp to mix the flow textures */
uniform float u_time1;                 /* the second time ramp to mix the flow textures */
uniform float u_max_foam_depth;        /* a value around 4.0 which indicates the max depth of the water; used to calculate the foam level */
uniform float u_vortex_intensity;      /* extra flow color, vortex intensity */
uniform vec3 u_sun_color;              /* the sun color */
uniform float u_sun_intensity;         /* intensity of the sun */
uniform float u_sun_shininess;         /* specularity level */
uniform vec3 u_ambient_color;          /* overall color */
uniform float u_ambient_intensity; 
uniform float u_diffuse_intensity;
uniform float u_foam_intensity;
uniform float u_final_intensity;

uniform sampler2D u_diffuse_tex;        /* water texture */
uniform sampler2D u_normal_tex;         /* water normals from texture */
uniform sampler2D u_noise_tex;          /* noise to offset the flow texture */
uniform sampler2D u_foam_tex;           /* the foam we mix with diffuse */
uniform sampler2D u_flow_tex;           /* the texture that holds the flow directions */
uniform sampler2D u_sand_tex;           /* the bottom sand texture */
uniform sampler2D u_depth_ramp_tex;     /* a color ramp based on depth */
uniform sampler2D u_extra_flow_tex;     /* flow that you can add with beginGrabFlow() */
uniform sampler2D u_vortex_tex;         /* the dark vortex in the center */

out vec4 fragcolor;

in vec2 v_tex;
in vec3 v_norm;
in vec3 v_eye_pos;                      /* position in eye space */
in vec3 v_world_pos;                    /* world/object  position */

void main() {

  fragcolor = vec4(1.0, 0.0, 0.0, 1.0);

  // Get flowed colors
  // -------------------------------------------------------------
  vec3 extra_flow_color = texture(u_extra_flow_tex, v_tex).rgb;
  vec2 flow_color = texture(u_flow_tex, v_tex).rg;
  float noise_color = texture(u_noise_tex, v_tex).r;

  flow_color = (v_norm.xz * 0.1 + flow_color);
  flow_color -= (extra_flow_color.rg * u_vortex_intensity);

  float lerp = abs(0.5 - u_time0) / 0.5;
  float phase0 = noise_color * 0.2 + u_time0;
  float phase1 = noise_color * 0.2 + u_time1;
  vec2 texcoord0 = (v_tex * 1.0) + (flow_color * phase0 * 0.5);
  vec2 texcoord1 = (v_tex * 1.0) + (flow_color * phase1 * 0.5);

  vec3 diffuse0 = texture(u_diffuse_tex, texcoord0).rgb;
  vec3 diffuse1 = texture(u_diffuse_tex, texcoord1).rgb;
  vec3 moved_diffuse = mix(diffuse0, diffuse1, lerp);
  
  vec3 normal0 = texture(u_normal_tex, texcoord0).rgb;
  vec3 normal1 = texture(u_normal_tex, texcoord1).rgb;
  vec3 moved_normal = -1.0 + 2.0 * mix(normal0, normal1, lerp);
  vec3 peturped_normal = normalize(moved_normal + v_norm);
  //vec3 peturped_normal = v_norm;

  vec3 foam0 = texture(u_foam_tex, texcoord0 * 4.0).rgb;
  vec3 foam1 = texture(u_foam_tex, texcoord1 * 4.0).rgb;
  vec3 moved_foam = mix(foam0, foam1, lerp);

  // Diffuse Lighting
  // -------------------------------------------------------------
  mat3 nm = mat3(u_vm);
  vec3 light_position = nm * vec3(-100.0, 0.0, 0.0);
  vec3 s = normalize(light_position - v_eye_pos);
  vec3 n = normalize(nm * peturped_normal);
  float sdn = max(dot(s, n), 0.0);
  
  // Specular Lighting
  // -------------------------------------------------------------
  vec3 v;
  vec3 r;
  vec3 sun_dir = vec3(0.0, -1.0, 0.0);
  s = normalize(sun_dir);
  v = normalize(-v_eye_pos);
  r = normalize(reflect(s, n));
  vec3 spec = sdn * u_sun_color * pow(clamp(dot(r, v), 0.0, 1.0), u_sun_shininess); 

  // Vortex
  // -------------------------------------------------------------
  float vortex_v = 1.0 - texture(u_vortex_tex, v_tex).r;


  // Refraction + depth based coloring
  // -------------------------------------------------------------
  vec3 sand_color = texture(u_sand_tex, v_tex + n.xz * 0.1).rgb;
  float depth_perc = 0.1 + clamp(v_world_pos.y/u_max_foam_depth, 0.0, 1.0);
  vec4 depth_color = texture(u_depth_ramp_tex, vec2(depth_perc));

  fragcolor.rgb = moved_diffuse * (1.0 - depth_perc) * u_diffuse_intensity;
  fragcolor.rgb += u_ambient_color * sdn * u_ambient_intensity;
  fragcolor.rgb += (depth_perc * moved_foam * u_foam_intensity);
  fragcolor.rgb += (spec * max(u_sun_intensity, 0.0));
  fragcolor.rgb = mix(fragcolor.rgb, sand_color * depth_color.rgb, depth_color.a);
  fragcolor.rgb = fragcolor.rgb * u_final_intensity;
  fragcolor.rgb *= vortex_v;
  
#if 0
  fragcolor.rgb = texture(u_diffuse_tex, v_tex).rgb;
  fragcolor.rgb = texture(u_normal_tex, v_tex).rgb;
  fragcolor.rgb = texture(u_noise_tex, v_tex).rgb;
  fragcolor.rgb = texture(u_foam_tex, v_tex).rgb;
  fragcolor.rgb = texture(u_flow_tex, v_tex).rgb;
  fragcolor.rgb = texture(u_sand_tex, v_tex).rgb;
  fragcolor.rgb = texture(u_depth_ramp_tex, v_tex).rgb;
  fragcolor.rgb = texture(u_extra_flow_tex, v_tex).rgb;
#endif


}
