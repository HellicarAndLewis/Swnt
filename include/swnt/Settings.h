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

#ifndef SWNT_SETTINGS_H
#define SWNT_SETTINGS_H

#define ROXLU_USE_OPENGL
#define ROXLU_USE_MATH
#include <tinylib.h>

#include <ocean/OceanSettings.h>
#include <string>
#include <rapidxml.hpp>

struct ColorSettings {
  void print();
  vec3 hand;
  vec3 spiral_from;
  vec3 spiral_to;
  vec3 flow_lines;     
  vec3 water;
};

class Settings {
 public:
  Settings();
  bool load(std::string filepath);
  void setTimeOfYear(float t);

 private:
  void getColorsForTimeOfYear(float t, ColorSettings& result); 
  bool extractColor(rapidxml::xml_node<>* n, vec3& col);

 public:
  int win_w;                           /* width of the window we render */
  int win_h;                           /* height of the window we render */ 
  int image_processing_w;              /* the width of the image that is used to detect the hands, the smaller the faster */
  int image_processing_h;              /* the height of the image "" .. */  
  OceanSettings ocean;                 /* ocean settings */
  vec3 cam_pos;                        /* cam pos .. I think only used to render the ocean @todo - cleanup if not used*/
  mat4 ortho_matrix;                   /* orthographic matrix we use to render "flat" drawings */
  mat4 depth_ortho_matrix;             /* when we render the depth image we use this ortho matrix */
  float kinect_near;                   /* mask out everything between kinect_near and kinect_far */
  float kinect_far;                    /* "" */
  float max_wind;                /* we use the wind speed from the weather info to influence the vortex */
  std::vector<ColorSettings> colors;   /* the colors used when rendering */
  size_t color_dx;                     /* the current color palette to use */
  float radius;                        /* radius of the overlay */
  ColorSettings curr_colors;           /* the colors for the current time of year, only valid when setTimeOfYear() has been called once */
};

#endif
