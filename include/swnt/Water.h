/*

  Water
  ------

  This class renders the given height fied (using most of the 
  internal types of this HeightField class) using a flow map
  to simulate the flow of water.

 */

#ifndef WATER_H
#define WATER_H

#define ROXLU_USE_OPENGL
#define ROXLU_USE_MATH
#define ROXLU_USE_PNG
#include <tinylib.h>
#include <vector>
#include <string>

// ------------------------------------------------------

// ------------------------------------------------------
class Settings;
class HeightField;

class Water {

 public:
  Water(HeightField& heightField, Settings& settings);
  bool setup(int winW, int winH);
  void update(float dt);
  void draw();

  void beginGrabFlow();                            /* start rendering extra flow colors which we mix with the diffuse water texture */
  void endGrabFlow();                              /* end rendering the extra flow colors */

  void setWeatherInfo(float wind);                 /* Sets the weather info; wind = 0.0f, no vortex, 1.0 = max vortex */
  void setTimeOfYear(float t);                     /* 0 = 1 jan, 1 = 31 dec */
  void setVortexAmount(float v);

  void print();                                    /* print debug info */

 private:
  bool setupShaders();
  bool setupTextures();
  bool setupFBO();                                 /* we have an fbo that we use to allow extra flow level to be drawn */
  
 public:

  HeightField& height_field;
  Settings& settings;
  int win_w;
  int win_h;

  Program water_prog;                             /* used to render the shaded version of the water */
  GLuint diffuse_tex;                             /* the water texture */
  GLuint normal_tex;                              /* the normals for the water */
  GLuint noise_tex;                               /* noise color to offset the flow textures */
  GLuint foam_tex;                                /* the foam texture we mix with the diffuse */
  GLuint flow_tex;                                /* the flow texture for the direction */
  GLuint sand_tex;                                /* the sand / floow texture */
  GLuint depth_ramp_tex;                          /* colors that are selected by depth */
  GLuint extra_flow_tex;                          /* the capture flow texture */
  GLuint force_tex;                               /* this texture is used to apply an extra force to the height field */
  
  GLuint fbo;                                     /* used to render the extra flow */

  float ambient_color[3];                         /* ambient color which is added to the overal colors */
  float ads_intensities[7];                       /* ambient, diffuse, specular  intensity, sun, foam, texture, overall */
  float max_foam_depth;                           /* used to mix water + foam */
  float foam_depth;                               
  float sun_pos[3];                               /* position of the sun */
  float sun_color[3];                             /* color of the sun */
  float vortex_intensity;
  float sun_intensity;
  float ambient_intensity;
  float sun_shininess;
  float diffuse_intensity;
  float foam_intensity;
  float final_intensity;

  GLint u_max_foam_depth;                         /* points to the u_max_foam_depth uniform, used to influence the amount of foam */
  GLint u_vortex_intensity;                       /* points to u_vortex_intensity */
  GLint u_sun_color;                              /* points to the sun color uniform */
  GLint u_sun_intensity;                          /* points to the sun intensity uniform */
  GLint u_sun_shininess;
  GLint u_ambient_color;
  GLint u_ambient_intensity;
  GLint u_diffuse_intensity;
  GLint u_foam_intensity;
  GLint u_final_intensity;

  float wind_level;                              /* the wind level is set by setWeatherInfo() and is used to change the vortex intensity */

};

#endif
