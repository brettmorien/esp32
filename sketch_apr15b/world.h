#ifndef World_h
#define World_h

#include <Arduino.h>
#include <Arduino_GFX_Library.h>

class World {
private:
  uint8_t PROGMEM *surface;
  Arduino_GFX *gfx;
  
  //  gravity
public:
  int w;
  int h;

  World(Arduino_GFX *gfx, int w, int h);

  void drawGrain(int x, int y);
  void drawWorld();
  bool hit(int x, int y);
};

#endif
