#include <Arduino_GFX_Library.h>

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

class World {
private:
  uint8_t PROGMEM *surface;
  int w;
  int h;
  //  gravity
public:

  World(int w, int h) {
    this->w = w;
    this->h = h;

    int worldSize = w * h / 8;
    this->surface = new uint8_t[worldSize];
    int start = (h - 5) * w / 8;
    for (int i = 0; i < worldSize; i++) {
      this->surface[i] = (i < start) ? 0 : 255;
    }
  }

  void drawGrain(int x, int y) {
    //  W: 240, H: 536
    int byte = y * (w / 8) + (x + 7) / 8;
    int bit = x % 8;

    gfx->drawPixel(x, y, WHITE);
    this->surface[byte] = this->surface[byte] | 1 << bit;
  }

  void drawWorld() {
    gfx->drawBitmap(0, 0, this->surface, this->w, this->h, WHITE);
  }

  bool hit(int x, int y) {
    //  W: 240, H: 536
    int byte = y * (w / 8) + (x + 7) / 8;
    int bit = x % 8;

    return (this->surface[byte] & 1 << (x % 8));
  }
};

World *world;

class Grain {
public:
  uint32_t x;
  uint32_t y;
  int32_t velo;
  bool active;

  void drop(int x, int y) {
    this->x = x;
    this->y = y;
    this->active = true;
  }

  void Update() {
    if (!this->active) {
      return;
    }

    if (!move()) {
      this->active = false;
    }
  }

private:
  bool move() {
    Grain *g = this;
    gfx->drawPixel(g->x, g->y, BLACK);
    int dist = g->findBelowDistance();

    if (dist < g->velo) {
      g->y = g->y + dist;
      if (g->slide()) {
        return true;
      }

      world->drawGrain(this->x, this->y);

      return false;
    }

    g->x = g->x;
    g->y = g->y + g->velo;
    gfx->drawPixel(g->x, g->y, BLUE);

    return true;
  }

  int findBelowDistance() {
    for (int i = 0; i < h - this->y; i++) {
      if (world->hit(this->x, this->y + i)) {
        return i - 1;
      }
    }
    return h - this->y - 1;
  }

  bool slide() {
    int left = random(0, 1) == 0;

    if (this->slip(left ? -1 : 1)) {
      return true;
    } else if (this->slip(left ? 1 : -11)) {
      return true;
    }

    return false;
  }

  bool slip(int dist) {
    if (!world->hit(this->x + dist, this->y + 1)) {
      this->x += dist;
      return true;
    }
    return false;
  }
};

class Source {
public:
  int numGrains = 300;
  int velo = 7;
  int dropWindow = 10;
  int x = cx;
  int y = 5;
  int interval;
  Grain *grains;
  int nextIndex = 0;

  Source() {
    int rate = 100;
    this->interval = 1000/rate;
    this->grains = new Grain[this->numGrains];
    for (int i = 0; i < this->numGrains; i++) {
      this->dropGrain(i);
    }
  }

  int lastDropTime;
  void next(int time) {
    int count = time / interval - lastDropTime / interval;

    if (count > 0) {
      this->lastDropTime = (time / interval) * interval;  // round down to interval
      for (int i=0; i < count; ++i) {
        this->dropGrain(this->nextIndex + i);
      }
      this->nextIndex = (this->nextIndex + count) % this->numGrains;
    }
  }

  void updateGrains() {
    for (int i = 0; i < this->numGrains; i++) {
      Grain *grain = &this->grains[i];
      grain->Update();
    }
  }

private:
  void dropGrain(int index) {
    Grain *grain = &this->grains[index];

    int x = random(this->x - this->dropWindow, this->x + this->dropWindow);
    int y = random(0, 50);

    grain->drop(x, y);
    grain->velo = this->velo;
  }
};

Source source;

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
  while (!Serial)
    ;
  Serial.println("Snow!");

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

  world = new World(w, h);

  source.x = cx;

  world->drawWorld();
}

void loop() {
  gfx->fillRect(0, 0, 60, 20, BLACK);
  gfx->drawRect(1, 1, w - 1, h - 1, MAGENTA);

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
  gfx->drawPixel(source.x, source.y, BLUE);

}
