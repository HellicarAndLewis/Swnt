#ifndef SWNT_GUI_H
#define SWNT_GUI_H

#include <stdio.h>
#include <AntTweakBar.h>

class Swnt;

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
};

#endif
