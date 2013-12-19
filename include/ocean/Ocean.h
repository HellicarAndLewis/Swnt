#ifndef OCEAN_H
#define OCEAN_H

#include <ocean/OceanSettings.h>

class WaterGraphics;

class Ocean {
 public:
  Ocean(OceanSettings& settings, WaterGraphics& w);
  ~Ocean();
  bool setup();
  void update(float dt);
  void draw();

 private:
  bool setupBuffers();
  GLuint createTexture(GLuint unit, GLenum iformat, int w, int h, GLenum eformat, GLenum type, GLvoid* data, GLenum wrap, GLenum filter);
  GLuint createFBO(GLuint tex);
  void bindTexture(GLuint tex, int unit);

 public:
  OceanSettings& settings;
  WaterGraphics& gfx;
  bool changed;                /* wether any of the uniform values changed */
  bool ping_phase;
  float wind_x;                /* wind x direction  @TODO - use settings.wind_y */
  float wind_y;                /* wind y direction  @TODO - use settings.wind_x */
  float size;                  /* size (in meters) of the ocean we're simulation  @TODO - use settings.size */
  float* pm;                   /* the projection matrix we use to render */
  float* vm;                   /* the view matrix we use to render */

  std::vector<int> indices;
  GLuint data_vao;
  GLuint data_vbo;
  GLuint indices_vbo;

  GLuint init_spectrum_tex;
  GLuint spectrum_tex;
  GLuint ping_phase_tex;
  GLuint pong_phase_tex;
  GLuint ping_transform_tex;
  GLuint pong_transform_tex;
  GLuint normal_map_tex;
  GLuint displacement_map_tex;
  
  GLuint init_spectrum_fbo;
  GLuint spectrum_fbo;
  GLuint ping_phase_fbo;
  GLuint pong_phase_fbo;
  GLuint ping_transform_fbo;
  GLuint pong_transform_fbo;
  GLuint normal_map_fbo;
  GLuint displacement_map_fbo;

};

inline void Ocean::bindTexture(GLuint tex, int unit) {
  glActiveTexture(GL_TEXTURE0 + unit);
  glBindTexture(GL_TEXTURE_2D, tex);
}

#endif
