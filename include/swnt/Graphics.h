/*
---------------------------------------------------------------------------------
 
                                               oooo
                                               `888
                oooo d8b  .ooooo.  oooo    ooo  888  oooo  oooo
                `888""8P d88' `88b  `88b..8P'   888  `888  `888
                 888     888   888    Y888'     888   888   888
                 888     888   888  .o8"'88b    888   888   888
                d888b    `Y8bod8P' o88'   888o o888o  `V88V"V8P'
 
                                                  www.roxlu.com
                                             www.apollomedia.nl
                                          www.twitter.com/roxlu
 
---------------------------------------------------------------------------------
*/

#ifndef GRAPHICS_H
#define GRAPHICS_H

#define ROXLU_USE_OPENGL
#define ROXLU_USE_MATH
#define ROXLU_USE_PNG
#include "tinylib.h"

#include <swnt/Settings.h>
#include <stdio.h>

static const char* TEX_VS = ""
  "#version 150\n"
  "uniform mat4 u_pm;"
  "uniform mat4 u_mm;"

  "const vec2 verts[4] = vec2[] ("
  "  vec2(-1.0, 1.0), " 
  "  vec2(-1.0, -1.0), "
  "  vec2(1.0, 1.0), "
  "  vec2(1.0, -1.0)"
  ");"

  "const vec2 tex[4] = vec2[] ("
  "  vec2(0.0, 1.0), " 
  "  vec2(0.0, 0.0), "
  "  vec2(1.0, 1.0), "
  "  vec2(1.0, 0.0)"
  ");"

  "out vec2 v_tex; " 
  "void main() {"
  "  vec4 vert = vec4(verts[gl_VertexID], 0.0, 1.0);"
  "  gl_Position = u_pm * u_mm * vert;"
  "  v_tex = tex[gl_VertexID];"
  "}"
  "";

static const char* TEX_FS = ""
  "#version 150\n"
  "uniform sampler2D u_tex;"
  "in vec2 v_tex;"
  "out vec4 fragcolor;"
  "void main() {"
  "  fragcolor = texture(u_tex, v_tex);"
  "}"
  "";

/* Used to draw plain vec3, vec2 vertices (nothing else) */
static const char* V_VS = ""
  "#version 150\n"
  "uniform mat4 u_pm;"
  "uniform mat4 u_mm;"
  "in vec4 a_pos;"
  "void main() { "
  "  gl_Position = u_pm * u_mm * a_pos; " 
  "}"
  "";

static const char* V_FS = ""
  "#version 150\n"
  "uniform vec3 u_color;"
  "out vec4 fragcolor; " 
  "void main() {"
  "  fragcolor = vec4(u_color, 1.0);"
  "}"
  "";

static const char* KINECT_FS = ""
  "#version 150\n"
  "uniform sampler2D u_tex;"
  "uniform vec2 u_dist;"  // get everything between u_dist.x (near) and u_dist.y (far)y
  "in vec2 v_tex;"
  "out vec4 fragcolor;"
  ""
  "void main() {"
  " float r = texture(u_tex, v_tex).r;"

#if 1
  " r = (r * 65535.0);"  // the incoming value is in a range of 0-1 using uint16 so we need to scale it (values will be 0-2048)
  " if(r > 2047) {"
  "    r = 0.0; "
  " }"
  " r = 1.0 / ((r * -0.0030711016) + 3.3309495161);" // get the value in meters, 1.0 = one meter 
  " if (r > u_dist.y || r < u_dist.x) { "
  "    r = 0.0; " 
  " }"
#endif

  " fragcolor.rgb = vec3(r, r, r);"
  " fragcolor.a = 1.0;"
  "}"
  "";

class Graphics {

 public:
  Graphics(Settings& settings);
  ~Graphics();
  bool setup();
  void drawTexture(GLuint tex, float x, float y, float w, float h);
  void drawDepth(GLuint tex, float x, float y, float w, float h);
  void drawCircle(float x, float y, float radius, vec3 color);
  GLuint createTexture(std::string filepath);                                  /* loads the PNG image and creates a texture object for it */

 public:

  /* drawing a texture */
  GLuint tex_vao;
  GLuint tex_vs;
  GLuint tex_fs;
  GLuint tex_prog;
  mat4 tex_pm;
  mat4 tex_mm; 

  /* used to debug draw vec2/3  */
  GLuint v_vs;
  GLuint v_fs;
  GLuint v_prog;
  GLuint circle_vao;
  GLuint circle_vbo;
  int circle_resolution;

  /* kinect */
  GLuint kinect_fs;
  GLuint kinect_prog;

  Settings& settings;
};

#endif
