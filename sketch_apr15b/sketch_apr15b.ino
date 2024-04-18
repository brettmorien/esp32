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
uint8_t* world;

void setup() {
    /**
     * * The difference between the touch and non-touch versions is that the display 
     * * power supply of the touch version is controlled by IO38
    */
    pinMode(38, OUTPUT);
    digitalWrite(38, OUTPUT);

    Serial.begin(115200);
    Serial.setDebugOutput(true);
    while(!Serial);
    Serial.println("Snow!");

    // Init Display
    if (!gfx->begin())
    {
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
    tsa = ((w <= 176) || (h <= 160)) ? 1 : (((w <= 240) || (h <= 240)) ? 2 : 3); // text size A
    tsb = ((w <= 272) || (h <= 220)) ? 1 : 2;                                    // text size B
    tsc = ((w <= 220) || (h <= 220)) ? 1 : 2;                                    // text size C
    ds = (w <= 160) ? 9 : 12;                                                    // digit size

    worldSize = w * h / 8;
    world = new uint8_t[worldSize];
    for (int i=0; i < worldSize; i++) {
      world[i] = 0;
    }

    gfx->drawRect(1, 1, w-1, h-1, MAGENTA);
}

void loop() {
  // put your main code here, to run repeatedly:

    // gfx->fillScreen(BLUE);
//    gfx->drawLine(0, 0, cx, cy, RED);

//    gfx->setCursor(0, 0);

//    gfx->setTextSize(tsa);
//    gfx->setTextColor(MAGENTA);
//    gfx->println(F("Brett's Text"));

// #ifdef CANVAS
//     gfx->flush();
// #endif

    while (true) {
      int rx = random(0, worldSize);
      int rb = random(0, 7);


      if ((world[rx] & (1 << rb)) == 0) {
        world[rx] = world[rx] | 1 << rb;
        break;
      }
    }

    drawWorld();

    // delay(60 * 1000L);
} 

void drawWorld() {

  gfx->drawBitmap(0, 0, world, w, h, WHITE);

}

