#include <Arduino_GFX_Library.h>
#include "grain.h"
#include "draw.h"
#include "source.h"
#include "world.h"

#define GFX_DEV_DEVICE LILYGO_T_DISPLAY_S3_AMOLED
Arduino_DataBus *bus = new Arduino_ESP32QSPI(
  6 /* cs */, 47 /* sck */, 18 /* d0 */, 7 /* d1 */, 48 /* d2 */, 5 /* d3 */);
Arduino_GFX *gfx = new Arduino_RM67162(bus, 17 /* RST */, 0 /* rotation */);

#ifdef ESP32
#undef F
#define F(s) (s)
#endif

#define GRAY RGB565(128, 128, 128)

// run params
int32_t w, h, cx, cy;
uint8_t tsa, tsb, tsc, ds;

// Next Steps
/*
  Move source
  Multi-source
  Gravity
  Obstacles
*/



World *world;



// Serial.println("init source");
Source source = Source(world);

class FrameData {
private:
  int lastTime = millis();
  int start = 0;
  int last = 0;
  uint32_t current = 1;

public:
  void Next(int now) {
    this->current++;
    if (now - this->lastTime > 1000) {
      this->lastTime = now;
      this->last = this->current - this->start;
      this->start = this->current;
    }
  }


  void Debug() {
    gfx->setCursor(0, 0);

    gfx->setTextSize(tsa);
    gfx->setTextColor(GREEN);
    // gfx->println(this->last);
    gfx->println(source.nextIndex);
  }
};

FrameData frames;

void setup() {
  /**
     * * The difference between the touch and non-touch versions is that the display 
     * * power supply of the touch version is controlled by IO38
    */
  pinMode(38, OUTPUT);
  digitalWrite(38, OUTPUT);

  Serial.begin(115200);
  Serial.setDebugOutput(true);
  
  // Wait for serial connection
  while (!Serial);

  Serial.println("Snow!");
  
  Draw* d = new Draw(gfx);

  // Init Display
  if (!gfx->begin()) {
    Serial.println("gfx->begin() failed!");
  }

  w = gfx->width();
  h = gfx->height();
  cx = w / 2;
  cy = h / 2;
  tsa = ((w <= 176) || (h <= 160)) ? 1 : (((w <= 240) || (h <= 240)) ? 2 : 3);  // text size A
  tsb = ((w <= 272) || (h <= 220)) ? 1 : 2;                                     // text size B
  tsc = ((w <= 220) || (h <= 220)) ? 1 : 2;                                     // text size C
  ds = (w <= 160) ? 9 : 12;                                                     // digit size

  gfx->fillScreen(BLACK);

  //  W: 240, H: 536
  Serial.printf("Screen - W: %d, H: %d\n", w, h);

  world = new World(gfx, w, h);

  source.x = cx;

  world->drawWorld();
}

void loop() {
  gfx->fillRect(0, 0, 60, 20, BLACK);
  // gfx->drawRect(1, 1, w - 1, h - 1, MAGENTA);

  frames.Next(millis());
  frames.Debug();

  moveSource();
  source.next(millis());
  source.updateGrains();
}

bool right = true;
void moveSource() {
  gfx->drawPixel(source.x, source.y, BLACK);
  int rb = cx + 50;
  int lb = cx - 50;

  if (right) {
    if (source.x < rb) {
      source.x += 1;
    } else {
      right = false;
    }
  } else {
    if (source.x > lb) {
      source.x -= 1;
    } else {
      right = true;
    }
  }
  gfx->drawPixel(source.x, source.y, RED);

}
