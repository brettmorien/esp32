#include <Arduino.h>
#include <Arduino_GFX_Library.h>

class Draw {
  Arduino_GFX *gfx;

public:
  Draw(Arduino_GFX *gfx) {
    Serial.println("Init draw");
    this.gfx = gfx;
  }


};
