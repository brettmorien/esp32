#include <Arduino_GFX_Library.h>

#define GFX_DEV_DEVICE LILYGO_T_DISPLAY_S3_AMOLED
Arduino_DataBus *bus = new Arduino_ESP32QSPI(
  6 /* cs */, 47 /* sck */, 18 /* d0 */, 7 /* d1 */, 48 /* d2 */, 5 /* d3 */);
Arduino_GFX *gfx = new Arduino_RM67162(bus, 17 /* RST */, 0 /* rotation */);

#ifdef ESP32
#undef F
#define F(s) (s)
#endif

int32_t w, h, n, n1, cx, cy, cx1, cy1, cn, cn1;
uint8_t tsa, tsb, tsc, ds;

int32_t worldSize;
uint8_t PROGMEM *world;

uint32_t frame = 1;

typedef struct Grain {
  uint32_t x;
  uint32_t y;
} Grain;

const int numGrains = 100;
Grain grains[numGrains];

void setup() {
  /**
     * * The difference between the touch and non-touch versions is that the display 
     * * power supply of the touch version is controlled by IO38
    */
  pinMode(38, OUTPUT);
  digitalWrite(38, OUTPUT);

  Serial.begin(115200);
  Serial.setDebugOutput(true);
  while (!Serial);
  Serial.println("Snow!");

  // Init Display
  if (!gfx->begin()) {
    Serial.println("gfx->begin() failed!");
  }

  w = gfx->width();
  h = gfx->height();
  n = min(w, h);
  n1 = n - 1;
  cx = w / 2;
  cy = h / 2;
  cx1 = cx - 1;
  cy1 = cy - 1;
  cn = min(cx1, cy1);
  cn1 = cn - 1;
  tsa = ((w <= 176) || (h <= 160)) ? 1 : (((w <= 240) || (h <= 240)) ? 2 : 3);  // text size A
  tsb = ((w <= 272) || (h <= 220)) ? 1 : 2;                                     // text size B
  tsc = ((w <= 220) || (h <= 220)) ? 1 : 2;                                     // text size C
  ds = (w <= 160) ? 9 : 12;                                                     // digit size

  worldSize = w * h / 8;
  world = new uint8_t[worldSize];
  for (int i = 0; i < worldSize; i++) {
    world[i] = 0;
  }
  gfx->fillScreen(BLACK);

  Serial.printf("Screen - W: %d, H: %d", w, h);

  int start = (h - 5) * w / 8;
  int end = start + 3 * w / 8;

  for (int i=start; i < end; i++) {
    world[i] = 255;
  }
  gfx->fillRect(0, 0, cx, 20, BLACK);
  gfx->drawRect(1, 1, w - 1, h - 1, MAGENTA);

  for (int i=0; i < numGrains; i++) {
    dropGrain(i);
  }
}

void loop() {
  gfx->fillRect(0, 0, cx, 20, BLACK);
  gfx->drawRect(1, 1, w - 1, h - 1, MAGENTA);
  drawFrameData();

  updateGrains();
  drawWorld();

  frame++;
}

void drawRandomDot() {
  int rx = random(0, w);
  int ry = random(0, h);

  gfx->drawPixel(rx, ry, WHITE);
}

void dropGrain(int index) {
  Grain* grain = &grains[index];
  
  grain->x = random(2, w - 2);
  grain->y = random(100, 350);
}

void updateGrains() {
  for (int i=0; i < numGrains; i++) {
    Grain* grain = &grains[i];
    if (!moveGrain(grain)) {
      drawToWorld(*grain);
      dropGrain(i);
    }
  }
}

bool moveGrain(Grain *g) {
  if (filledInWorld(g->x, g->y + 1)) {
    return false;
  }
  
  // if (g->y >= h - 5) {
  //   return false;
  // }

  gfx->drawPixel(g->x, g->y, BLACK);
  g->x = g->x;
  g->y = g->y + 1;
  gfx->drawPixel(g->x, g->y, WHITE);

  return true;
}

void drawFrameData() {
  gfx->setCursor(0, 0);

  gfx->setTextSize(tsa);
  gfx->setTextColor(GREEN);
  gfx->println(frame);
}

bool filledInWorld(int x, int y) {
    //  W: 240, H: 536
  int byte = y * (w / 8) + (x + 7) / 8;
  int bit = x % 8;

  return (world[byte] & 1 << (x % 8));
}

void drawToWorld(Grain grain) {
  //  W: 240, H: 536
  int x = (grain.x + 7) / 8;

  int byte = grain.y * (w / 8) + x;
  int bit = grain.x % 8;

  world[byte] = world[byte] | 1 << (x % 8);
}

void drawWorld() {

  gfx->drawBitmap(0, 0, world, w, h, WHITE);
}
