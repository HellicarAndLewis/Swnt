#version 150

uniform mat4 u_pm;
in vec4 a_pos;
in float a_size;
out vec2 v_tex;

const vec2 pos[] = vec2[4](
  vec2(-0.5,  0.5),
  vec2(-0.5, -0.5),
  vec2(0.5,   0.5),
  vec2(0.5,  -0.5)
);

const vec2 tex[4] = vec2[](
  vec2(0.0, 1.0),  
  vec2(0.0, 0.0), 
  vec2(1.0, 1.0), 
  vec2(1.0, 0.0)
);

void main() {
  vec2 offset = pos[gl_VertexID];

  gl_Position = u_pm * vec4(a_pos.x + (offset.x * a_size) ,
                            a_pos.y + (offset.y * a_size) ,
                            0.0,
                            1.0);
  
  v_tex = tex[gl_VertexID];
}

