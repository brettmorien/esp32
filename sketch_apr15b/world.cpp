#include "world.h"

World::World(Arduino_GFX *gfx, int w, int h) {
  this->gfx = gfx;
  this->w = w;
  this->h = h;

  int worldSize = w * h / 8;
  this->surface = new uint8_t[worldSize];
  int start = (h - 5) * w / 8;
  for (int i = 0; i < worldSize; i++) {
    this->surface[i] = (i < start) ? 0 : 255;
  }
}

void World::drawGrain(int x, int y) {
  //  W: 240, H: 536
  int byte = y * (w / 8) + (x + 7) / 8;
  int bit = x % 8;

  this->gfx->drawPixel(x, y, WHITE);
  this->surface[byte] = this->surface[byte] | 1 << bit;
}

void World::drawWorld() {
  this->gfx->drawBitmap(0, 0, this->surface, this->w, this->h, WHITE);
}

bool World::hit(int x, int y) {
  //  W: 240, H: 536
  int byte = y * (w / 8) + (x + 7) / 8;
  int bit = x % 8;

  return (this->surface[byte] & 1 << (x % 8));
}
