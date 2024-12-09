#ifndef Grain_h
#define Grain_h

#ifdef ESP32
#undef F
#define F(s) (s)
#endif

#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include "world.h"

class Grain {
public:
  Grain();
  Grain(World *world, Arduino_GFX* gfx);

  uint32_t x;
  uint32_t y;
  int32_t velo;
  bool active;
  World *world;
  Arduino_GFX *gfx;

  void drop(int x, int y);
  void Update();

private:
  bool move();
  int findBelowDistance();
  bool slide();
  bool slip(int dist);
};

#endif
