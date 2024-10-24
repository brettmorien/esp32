#include "grain.h"
#include <Arduino_GFX_Library.h>

Grain::Grain(World *world) {
  this->world = world;
}

void Grain::drop(int x, int y) {
  this->x = x;
  this->y = y;
  this->active = true;
}

void Grain::Update() {
  if (!this->active) {
    return;
  }

  if (!move()) {
    this->active = false;
  }
}

bool Grain::move() {
  this->gfx->drawPixel(this->x, this->y, BLACK);
  int dist = this->findBelowDistance();

  if (dist < this->velo) {
    this->y = this->y + dist;
    if (this->slide()) {
      return true;
    }

    this->world->drawGrain(this->x, this->y);

    return false;
  }

  this->x = this->x;
  this->y = this->y + this->velo;
  this->gfx->drawPixel(this->x, this->y, WHITE);

  return true;
}

int Grain::findBelowDistance() {
  int h = this->world->h;
  for (int i = 0; i < h - this->y; i++) {
    if (this->world->hit(this->x, this->y + i)) {
      return i - 1;
    }
  }
  return h - this->y - 1;
}

bool Grain::slide() {
  int left = random(0, 1) == 0;

  if (this->slip(left ? -1 : 1)) {
    return true;
  } else if (this->slip(left ? 1 : -11)) {
    return true;
  }

  return false;
}

bool Grain::slip(int dist) {
  if (!this->world->hit(this->x + dist, this->y + 1)) {
    this->x += dist;
    return true;
  }
  return false;
}
