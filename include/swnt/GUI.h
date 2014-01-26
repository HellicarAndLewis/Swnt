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

#ifndef SWNT_GUI_H
#define SWNT_GUI_H

#include <stdio.h>
#if defined(__linux)
#  include <GL/glcorearb.h>
#endif

#include <AntTweakBar.h>
#include <swnt/Types.h>

class Swnt;

void TW_CALL set_time_of_day(const void* value, void* user);
void TW_CALL get_time_of_day(void* value, void* user);
void TW_CALL set_time_of_year(const void* value, void* user);
void TW_CALL get_time_of_year(void* value, void* user);

class GUI {
 public:
  GUI(Swnt& swnt);
  ~GUI();
#if USE_GUI
  bool setup(int w, int h);
#endif
  void onMouseMoved(double x, double y);
  void onMouseClicked(int bt, int action);
  void onKeyPressed(int key, int mod);
  void onResize(int w, int h);
  void onMouseWheel(double x, double y);
  void draw();
 public:
  Swnt& swnt;
  int win_w;
  int win_h;
  double scroll_y;
  TwBar* bar;
  float time_of_day;  /* used to override the time of day, 0 = midnight at 00:00, 1 = 23:59:59... */
  float time_of_year; /* used to override the time of year, 0 = 1 jan, 1 = 31 dec. */
};

#endif
