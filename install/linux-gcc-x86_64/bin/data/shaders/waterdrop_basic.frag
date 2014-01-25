#version 150

uniform sampler2D u_normals_tex;
uniform sampler2D u_alpha_tex;

in vec2 v_tex;
out vec4 fragcolor;
out vec4 out_alpha;
out vec4 out_normals;

void main() {
  out_alpha = texture(u_alpha_tex, v_tex);
  out_normals = texture(u_normals_tex, v_tex) * out_alpha.r ; // * 0.4;
}
