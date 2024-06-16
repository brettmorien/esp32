#ifndef Grain_h
#define Grain_h

#ifdef ESP32
#undef F
#define F(s) (s)
#endif

#include <Arduino.h>

class Grain {
public:
  uint32_t x;
  uint32_t y;
  int32_t velo;
  bool active;

  void drop(int x, int y);
  void Update();

private:
  bool move();
  int findBelowDistance();
  bool slide();
  bool slip(int dist);
};

#endif