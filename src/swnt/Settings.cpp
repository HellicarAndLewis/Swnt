#include <rapidxml.hpp>
#include <iostream>
#include <fstream>
#include <swnt/Settings.h>
#include <tinylib.h>

using namespace rapidxml;

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
  printf("--\n");
  printf("settings.file: %s\n", filepath.c_str());

  std::string xml_str;
  xml_str.assign(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
  
  xml_document<> doc;
  try { 
    doc.parse<0>((char*)xml_str.c_str());
    xml_node<>* cfg = doc.first_node("settings");
    assert(cfg);

    // window width
    xml_node<>* ww = cfg->first_node("window_width");
    assert(ww);
    win_w = rx_to_int(ww->value());
    ocean.win_w = win_w;

    // window height
    xml_node<>* wh = cfg->first_node("window_height");
    assert(wh);
    win_h = rx_to_int(wh->value());
    ocean.win_h = win_h;

    // ocean
    {
      xml_node<>* oc = cfg->first_node("ocean");
      assert(oc);

      xml_node<>* wfx = oc->first_node("wind_force_x");
      assert(wfx);
      
      xml_node<>* wfy = oc->first_node("wind_force_y");
      assert(wfy);

      xml_node<>* size = oc->first_node("size");
      assert(size);

      ocean.wind_x = rx_to_float(wfx->value());
      ocean.wind_y = rx_to_float(wfy->value());
      ocean.size = rx_to_float(size->value());
    }

    // kinect
    xml_node<>* k_near = cfg->first_node("kinect_near");
    assert(k_near);
    kinect_near = rx_to_float(k_near->value());

    xml_node<>* k_far = cfg->first_node("kinect_far");
    assert(k_far);
    kinect_far = rx_to_float(k_far->value());

    // colors
    xml_node<>* cols = cfg->first_node("colors")->first_node();
    assert(cols);
    while(cols) {
      ColorSettings col_setting;
      extractColor(cols->first_node("hand"), col_setting.hand) ;
      extractColor(cols->first_node("spiral_from"), col_setting.spiral_from) ;
      extractColor(cols->first_node("spiral_to"), col_setting.spiral_to) ;
      extractColor(cols->first_node("flow_lines"), col_setting.flow_lines) ;
      colors.push_back(col_setting);
      cols = cols->next_sibling();
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

void Settings::extractColor(xml_node<>* n, vec3& col) {
  assert(n);

  xml_node<>* c = n->first_node();
  assert(c);

  int dx = 0;
  while(c) {
    col[dx] = rx_to_float(c->value()) / 255.0f;
    dx++;
    c = c->next_sibling();
  }
}
