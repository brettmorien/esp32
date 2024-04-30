#include <Arduino_GFX_Library.h>

#define GFX_DEV_DEVICE LILYGO_T_DISPLAY_S3_AMOLED
Arduino_DataBus *bus = new Arduino_ESP32QSPI(
  6 /* cs */, 47 /* sck */, 18 /* d0 */, 7 /* d1 */, 48 /* d2 */, 5 /* d3 */);
Arduino_GFX *gfx = new Arduino_RM67162(bus, 17 /* RST */, 0 /* rotation */);

#ifdef ESP32
#undef F
#define F(s) (s)
#endif

// run params

int32_t w, h, n, n1, cx, cy, cx1, cy1, cn, cn1;
uint8_t tsa, tsb, tsc, ds;

int32_t worldSize;
uint8_t PROGMEM *world;

class Source {
public:
  int numGrains = 100;
  int velo = 7;
  int dropWindow = 10;
};

Source source;

class FrameData {
public:
  uint32_t current = 1;
  int lastTime = millis();
  int start = 0;
  int last = 0;
  void Frame(int time) {
    if (now - this->lastTime > 1000) {
      this->lastTime = now;
      this->last = this->current - this->start;
      this->start = this->current;
    }
  }
};

FrameData frames;

class Grain {
public:
  uint32_t x;
  uint32_t y;
  int32_t velo;
};

Grain *grains;

void setup() {
  /**
     * * The difference between the touch and non-touch versions is that the display 
     * * power supply of the touch version is controlled by IO38
    */
  pinMode(38, OUTPUT);
  digitalWrite(38, OUTPUT);

  Serial.begin(115200);
  Serial.setDebugOutput(true);
  while (!Serial)
    ;
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

  //  W: 240, H: 536
  Serial.printf("Screen - W: %d, H: %d\n", w, h);

  int start = (h - 5) * w / 8;
  int end = start + 3 * w / 8;

  for (int i = start; i < end; i++) {
    world[i] = 255;
  }

  grains = new Grain[source.numGrains];
  for (int i = 0; i < source.numGrains; i++) {
    dropGrain(source, i);
  }
  drawWorld();
}

// Next Steps
/*
  Multi-pixel drop rate
  Variable speed drop
  Collide and slide left/right
  Obstacles
*/

void loop() {
  gfx->fillRect(0, 0, 60, 20, BLACK);
  gfx->drawRect(1, 1, w - 1, h - 1, MAGENTA);
  drawFrameData();

  updateGrains();

  frames.current++;
}



void dropGrain(Source s, int index) {
  Grain *grain = &grains[index];

  grain->x = random(cx - s.dropWindow, cx + s.dropWindow);
  grain->y = random(0, 50);
  grain->velo = s.velo;
}

void updateGrains() {
  for (int i = 0; i < source.numGrains; i++) {
    Grain *grain = &grains[i];
    if (!moveGrain(grain)) {
      dropGrain(source, i);
    }
  }
}

bool moveGrain(Grain *g) {
  gfx->drawPixel(g->x, g->y, BLACK);
  int dist = findBelowDistance(*g);

  if (dist < g->velo) {
    g->y = g->y + dist;
    if (slide(g)) {
      return true;
    }

    drawToWorld(*g);

    return false;
  }

  g->x = g->x;
  g->y = g->y + g->velo;
  gfx->drawPixel(g->x, g->y, WHITE);

  return true;
}

bool slide(Grain *g) {
  int left = random(0, 1) == 0;

  if (slip(g, left ? -1 : 1)) {
    return true;
  } else if (slip(g, left ? 1 : -11)) {
    return true;
  }

  return false;
}

bool slip(Grain *g, int dist) {
  if (!hit(g->x + dist, g->y + 1)) {
    g->x += dist;
    return true;
  }
  return false;
}

void drawFrameData() {
  int now = millis();
  

  // if (now - frames.lastTime > 1000) {
  //   frames.lastTime = now;
  //   frames.last = frames.current - frames.start;
  //   frames.start = frames.current;
  // }

  gfx->setCursor(0, 0);

  gfx->setTextSize(tsa);
  gfx->setTextColor(GREEN);
  // gfx->println(frame);
  gfx->println(frames.last);
}

int findBelowDistance(Grain g) {
  for (int i = 0; i < h - g.y; i++) {
    if (hit(g.x, g.y + i)) {
      return i - 1;
    }
  }
  return h - g.y - 1;
}

void drawToWorld(Grain g) {
  //  W: 240, H: 536
  int byte = g.y * (w / 8) + (g.x + 7) / 8;
  int bit = g.x % 8;

  gfx->drawPixel(g.x, g.y, WHITE);
  world[byte] = world[byte] | 1 << bit;
}

void drawWorld() {
  gfx->drawBitmap(0, 0, world, w, h, WHITE);
}

bool hit(int x, int y) {
  //  W: 240, H: 536
  int byte = y * (w / 8) + (x + 7) / 8;
  int bit = x % 8;

  return (world[byte] & 1 << (x % 8));
}
