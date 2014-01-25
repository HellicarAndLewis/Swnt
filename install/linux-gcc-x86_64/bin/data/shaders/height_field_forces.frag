#version 150

uniform sampler2D u_tex;
in vec2 v_tex;
out vec4 fragcolor;

void main() {
  fragcolor = vec4(texture(u_tex, v_tex).rg, 0.0, 1.0);
}

