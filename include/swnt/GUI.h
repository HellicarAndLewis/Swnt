#ifndef SWNT_GUI_H
#define SWNT_GUI_H

#include <stdio.h>
#include <AntTweakBar.h>

class Swnt;

void TW_CALL set_time_of_day(const void* value, void* user);
void TW_CALL get_time_of_day(void* value, void* user);

class GUI {
 public:
  GUI(Swnt& swnt);
  ~GUI();
  bool setup(int w, int h);
  void onMouseMoved(double x, double y);
  void onMouseClicked(int bt, int action);
  void onKeyPressed(int key, int mod);
  void onResize(int w, int h);
  void draw();
 public:
  Swnt& swnt;
  int win_w;
  int win_h;
  TwBar* bar;
  float time_of_day;  /* used to override the time of day, 0 = midnight at 00:00, 1 = 23:59:59... */
};

#endif
