#include <rapidxml.hpp>
#include <iostream>
#include <fstream>
#include <swnt/Settings.h>
#include <tinylib.h>

using namespace rapidxml;

#define CHECK_SETTING(node) { if (!node) { printf("Error: cannot get element from settings: " #node ". Do you have the latest settings.xml file?\n");  return false; } }

// -------------------------------------------------
void ColorSettings::print() {
}
// -------------------------------------------------

Settings::Settings()
  :win_w(0)
  ,win_h(0)
  ,image_processing_w(640)
  ,image_processing_h(480)
  ,kinect_near(1.3f)
  ,kinect_far(1.8f)
  ,color_dx(0)
  ,radius(350.0f)
{
}

bool Settings::load(std::string filepath) {


  std::ifstream ifs(filepath.c_str(), std::ios::in);
  if(!ifs.is_open()) {
    printf("Error: cannot open: `%s`\n", filepath.c_str());
    return false;
  }

  printf("settings.file: %s\n", filepath.c_str());

  std::string xml_str;
  xml_str.assign(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
  
  xml_document<> doc;
  try { 
    doc.parse<0>((char*)xml_str.c_str());
    xml_node<>* cfg = doc.first_node("settings");
    CHECK_SETTING(cfg);

    // window width
    xml_node<>* ww = cfg->first_node("window_width");
    CHECK_SETTING(ww);

    win_w = rx_to_int(ww->value());
    ocean.win_w = win_w;

    // window height
    xml_node<>* wh = cfg->first_node("window_height");
    CHECK_SETTING(wh);

    win_h = rx_to_int(wh->value());
    ocean.win_h = win_h;

    // ocean
    {
      xml_node<>* oc = cfg->first_node("ocean");
      CHECK_SETTING(oc);

      xml_node<>* wfx = oc->first_node("wind_force_x");
      CHECK_SETTING(wfx);
      
      xml_node<>* wfy = oc->first_node("wind_force_y");
      CHECK_SETTING(wfy);

      xml_node<>* size = oc->first_node("size");
      CHECK_SETTING(size);

      ocean.wind_x = rx_to_float(wfx->value());
      ocean.wind_y = rx_to_float(wfy->value());
      ocean.size = rx_to_float(size->value());
    }

    // kinect
    xml_node<>* k_near = cfg->first_node("kinect_near");
    CHECK_SETTING(k_near);

    kinect_near = rx_to_float(k_near->value());

    xml_node<>* k_far = cfg->first_node("kinect_far");
    CHECK_SETTING(k_far);

    kinect_far = rx_to_float(k_far->value());

    // colors
    xml_node<>* cols = cfg->first_node("colors")->first_node();
    CHECK_SETTING(cols);

    while(cols) {
      ColorSettings col_setting;
      extractColor(cols->first_node("hand"), col_setting.hand) ;
      extractColor(cols->first_node("spiral_from"), col_setting.spiral_from) ;
      extractColor(cols->first_node("spiral_to"), col_setting.spiral_to) ;
      extractColor(cols->first_node("flow_lines"), col_setting.flow_lines) ;
      extractColor(cols->first_node("water"), col_setting.water) ;
      colors.push_back(col_setting);
      cols = cols->next_sibling();
    }

    if(colors.size() != 4) {
      printf("Error: we did found enough colors. Make sure the settings file has colors for each season.\n");
      ::exit(EXIT_FAILURE);
    }

  }
  catch(...) {
    printf("Error: cannot open/parse the settings.xml.\n");
    return false;
  }

  printf("settings.win_w %d\n", win_w);
  printf("settings.win_h: %d\n", win_h);

  ortho_matrix.ortho(0, win_w, win_h, 0, -1.0f, 1000.0f);
  depth_ortho_matrix.ortho(0.0f, image_processing_w, image_processing_h, 0.0f, 0.0f, 100.0f);
  return true;
}

bool Settings::extractColor(xml_node<>* n, vec3& col) {
  CHECK_SETTING(n);

  xml_node<>* c = n->first_node();
  assert(c);

  int dx = 0;
  while(c) {
    col[dx] = rx_to_float(c->value()) / 255.0f;
    dx++;
    c = c->next_sibling();
  }

  return true;
}

void Settings::setTimeOfYear(float t) {
  getColorsForTimeOfYear(t, curr_colors);
}

void Settings::getColorsForTimeOfYear(float t, ColorSettings& result) {
  t = CLAMP(t, 0.0, 1.0);
  
  // get the season
  int day = t * 366;
  int seasons[] = { 80, 172, 264, 366 } ;
  int season;
  for(int i = 0; i < 3; ++i) {
    if(day < seasons[i]) {
      season = i; 
      break;
    }
  }

  if(season > colors.size()) {
    printf("Warning: trying to get colors for a seaons which doesnt exist: season: %d, day: %d\n", season, day);
    return;
  }

  Spline<float> hand_hues;
  Spline<float> hand_sats;
  Spline<float> hand_vals;

  Spline<float> water_hues;
  Spline<float> water_sats;
  Spline<float> water_vals;

  Spline<float> flow_hues;
  Spline<float> flow_sats;
  Spline<float> flow_vals;
  
  float hand_hsv[3];
  float water_hsv[3];
  float flow_hsv[3];

  for(int i = 0; i < colors.size(); ++i) {
    ColorSettings col = colors[i];

    rx_rgb_to_hsv(col.hand, hand_hsv);
    rx_rgb_to_hsv(col.water, water_hsv);
    rx_rgb_to_hsv(col.spiral_from, flow_hsv);

    hand_hues.push_back(hand_hsv[0]);
    water_hues.push_back(water_hsv[0]);
    flow_hues.push_back(flow_hsv[0]);

    hand_sats.push_back(hand_hsv[1]);
    water_sats.push_back(water_hsv[1]);
    flow_sats.push_back(flow_hsv[1]);

    hand_vals.push_back(hand_hsv[2]);
    water_vals.push_back(water_hsv[2]);
    flow_vals.push_back(flow_hsv[2]);
  }

  rx_hsv_to_rgb(hand_hues.at(t), hand_sats.at(t), hand_vals.at(t), result.hand.x, result.hand.y, result.hand.z);
  rx_hsv_to_rgb(water_hues.at(t), water_sats.at(t), water_vals.at(t), result.water.x, result.water.y, result.water.z);
  rx_hsv_to_rgb(flow_hues.at(t), flow_sats.at(t), flow_vals.at(t), result.flow_lines.x, result.flow_lines.y, result.flow_lines.z);
}
