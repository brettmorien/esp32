#ifndef Source_h
#define Source_h

#include <Arduino.h>
#include "grain.h"

class Source {
public:
  Source();

  int numGrains = 300;
  int velo = 3;
  int dropWindow = 10;
  // int x = cx;
  int x = 120;
  int y = 5;
  int interval;
  Grain *grains;
  int nextIndex = 0;

  int lastDropTime;

  void next(int time);
  void updateGrains();

private:
  void dropGrain(int index);
};

#endif
