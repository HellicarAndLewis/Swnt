#ifndef MASK_H
#define MASK_H

#define ROXLU_USE_OPENGL
#define ROXLU_USE_MATH
#define ROXLU_USE_PNG
#include "tinylib.h"

#include <vector>
#include <swnt/Settings.h>
#include <swnt/Graphics.h>
#include <swnt/Blur.h>
#include <swnt/Threshold.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

static const char* MASK_VS = ""
  "#version 150\n"
  "uniform mat4 u_pm;"
  "in vec4 a_pos;"
  "void main() {"
  "  gl_Position = u_pm * a_pos; "
  "}"
  "";

static const char* MASK_FS = ""
  "#version 150\n"
  "out vec4 fragcolor; "
  "void main() {"
  "  fragcolor.rgb = vec3(1.0);"
  "  fragcolor.a = 1.0;"
  "}"
  "";

static const char* MASKED_OUT_FS = ""
  "#version 150\n"
  "uniform sampler2D u_depth_tex;"
  "uniform sampler2D u_mask_tex;"
  "in vec2 v_tex;"
  "out vec4 fragcolor;"
  "void main() {"
  "  vec4 mask_tc = texture(u_mask_tex, v_tex);"
  "  vec4 depth_tc = texture(u_depth_tex, v_tex);"
  "  fragcolor.a = 1.0;"
  "  fragcolor.rgb = vec3(depth_tc.r) * ceil(mask_tc.r * 1000);"
  "}"
  "";

static const char* MASKED_OUT_SCENE_FS = ""
  "#version 150\n"
  "uniform sampler2D u_scene_tex;"
  "uniform sampler2D u_mask_tex;"
  "uniform sampler2D u_thresh_tex;"
  "uniform vec3 u_hand_col;"
  "uniform int u_draw_hand;"
  "in vec2 v_tex;"
  "out vec4 fragcolor;"
  "void main() {"
  "  vec4 mask_tc = texture(u_mask_tex, v_tex);"
  "  vec4 scene_tc = texture(u_scene_tex, v_tex);"
  "  vec4 thresh_tc = texture(u_thresh_tex, vec2(v_tex.s, 1.0 - v_tex.t));"
  "  fragcolor.a = mask_tc.r * scene_tc.a; "
  "  fragcolor.rgb = scene_tc.rgb;"

  // @todo remove u_draw_hand from shader
#if 0
  "  if(u_draw_hand == 1 && thresh_tc.r > 0.0) {"
  "     fragcolor.rgb = u_hand_col;"
  "     fragcolor.a = 1.0;"
  "  }"
#endif

  "}"
  "";

/* Masks out a texture with the generated mask */
static const char* DRAW_MASKED_FS = ""
  "#version 150\n"
  "uniform sampler2D u_mask_tex;"
  "uniform sampler2D u_diffuse_tex;"
  "in vec2 v_tex;"
  "out vec4 fragcolor;"

  "void main() {"
  "  float mask_color = texture(u_mask_tex, v_tex).r;"
  "  vec3 diffuse_color = texture(u_diffuse_tex, v_tex).rgb;"
  "  fragcolor.rgb = diffuse_color * mask_color;"
  "  fragcolor.a = diffuse_color.r;"
  "}"
  "";

class Mask {

 public:
  Mask(Settings& settings, Graphics& gfx);
  ~Mask();
  bool setup();
  void update();
  void drawMask();                        /* draw the mask shape */
  void drawThresholded();                 /* draw the thresholded image (wich contains the blobs) */

  void beginMaskGrab();                   /* begin grabbing the mask that is used to make sure that only a part of the scene is drawn. call drawMask() after this */
  void beginSceneGrab();                  /* begin grabbing the scene that you want to "mask" (this will be the final result) */
  void beginDepthGrab();                  /* begin grabbing the depth image... after this you need to render the depth and the the fragcolor.r value */

  void endSceneGrab();                    /* end grabbing the scene */
  void endDepthGrab();                    /* end grabbing the depth texture (where you render the depth texture as a correct dept image like: http://i.imgur.com/IOZMexQ.png */
  void endMaskGrab();                     /* end grabbing the mask shape */

  void maskOutDepth();                    /* this function will mask out the grabbed depth buffer with the grabbed mask generating an image that can be used for image processing */
  void maskOutScene();                    /* this will mask out the captured scene using the captured mask */
  void maskOutTexture(GLuint tex);
  
  void drawHand();
  void setScale(float s);                 /* set the scale for the mask; this is used to change the radius of the mask slightly. works together with the scale range */

  void refresh();                         /* refresh the rendering of the mask based on the current settings (e.g. change colors) */
  void print();                           /* print some debug info */

 private:
  bool createShader();
  bool createFBO();
  bool createBuffers(); 
  void updateVertices();
  
 public:
  Settings& settings;
  Graphics& graphics;
  Blur blur;                              /* used to blur the masked_out_tex */
  Threshold thresh;                       /* after blurring we threshold the masked_out_tex */
  bool draw_hand;                         /* draw the "blob" or "hand" in maskOutScene */

  /* Render To Texture GL */
  GLuint scene_fbo;                        /* the scene fbo into which we render the and that we use to render the mask into a texture */
  GLuint scene_depth;                      /* depth buffer for our scene fbo */
  GLuint scene_tex;                        /* the captured scene */ 
  GLuint mask_tex;                         /* we render the mask into this texture */
  GLuint mask_vao;                         /* used to draw the mask shape */
  GLuint mask_vbo;                         /* "" */

  /* Rendering the mask  */
  GLuint mask_vert;                        /* we use our own simple mask vertex shader */
  GLuint mask_frag;                        /* mask fragment shader, used to draw the mask shape vertices */
  GLuint mask_prog;                        /* the shader program that we use to draw the mask shape vertices */

  GLuint masked_out_vao;                   /* vao that we use to draw the masked out textures */
  GLuint masked_out_fbo;                   /* the fbo that will hold the mask shape + the masked out result */
  GLuint masked_out_tex;                   /* the texture that holds the masked out pixels, which is used for contour/block tracking */
  GLuint masked_out_vert;                  /* vertex shader that is used to mask out a texture */
  GLuint masked_out_frag;                  /* fragment shader "" "" */
  GLuint masked_out_prog;                  /* shader program ... "" "" */
  unsigned char* masked_out_pixels;        /* the pixels, grayscale unsigned char*, that we read back from the GPU and that is used by the Tracker. */

  /* Masking out the captured scene texture */
  GLuint masked_scene_frag;
  GLuint masked_scene_prog;

  /* Rendering a texture and mask it out, we use the TEX_VS vertex shader of Graphics */
  GLuint mask_tex_frag;                    /* the fragment shader we use to mask out a texture with the generated mask */
  GLuint mask_tex_prog;                    /* the program to mask out another texture */

  /* Grabbing the depth */
  GLuint depth_tex;                        /* we render the depth image from the kinect into this texture. this only does a simple "blit" we assume the depth pixels are already converted to meters */
  GLuint depth_fbo;                        /* fbo we used to grab the depth texture/image */

  /* Mask vertices (the circular shape that we use to mask is drawn using vertices) */
  int resolution;
  vec2 center;
  std::vector<vec2> vertices;
  Perlin perlin;                            /* used to animate the radius of the mask */
  size_t bytes_allocated;                   /* number of bytes allocated in the vbo */
  float scale;                              /* scale of the mask that we use with "scale_range" to modify the radius of the mask. e.g. when scale_range is 20, the a scale value of 1.0 means that we add 20 to the radius */
  float scale_range;                        /* the length we change the radius. this works together with scale */
};

#endif
