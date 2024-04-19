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
uint8_t *world;

uint32_t frame = 1;

typedef struct Grain {
  uint32_t x;
  uint32_t y;
} Grain;

Grain grain;

const int numGrains = 5;
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

  for (int i=0; i < 20; i++) {
    world[20000 + i] = 255;
  }

  dropGrain(cx, 30);
}

void loop() {
  gfx->startWrite();

  gfx->fillRect(0, 0, cx, 20, BLACK);
  gfx->drawRect(1, 1, w - 1, h - 1, MAGENTA);
  drawFrameData();

  updateGrains();
  drawWorld();

  gfx->endWrite();
  frame++;
}

void drawRandomDot() {
  int rx = random(0, w);
  int ry = random(0, h);

  gfx->drawPixel(rx, ry, WHITE);
}

void dropGrain(int x, int y) {
  grain.x = x;
  grain.y = y;
}

bool go = true;

void updateGrains() {
  if (go) {
    if (!moveGrain(&grain)) {
      drawToWorld(grain);
      go = false;
    }
  }
}

bool moveGrain(Grain *g) {

  if (g->y % 40 == 0) {
    drawToWorld(*g);
  }

  if (g->y >= h) {
    return false;
  }

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

void drawToWorld(Grain grain) {
  //  W: 240, H: 536
  int x = (grain.x + 7) / 8;

  int byte = grain.y * (w / 8) + x;
  int bit = grain.y % 8;

  world[byte] = world[byte] | 1 << ((x + 3) % 8);
}

void drawWorld() {

  gfx->drawBitmap(0, 0, world, w, h, WHITE);
}
