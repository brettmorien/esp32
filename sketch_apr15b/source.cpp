#include "source.h"

Source::Source() {
  int rate = 5;
  this->interval = 1000 / rate;
  this->grains = new Grain[this->numGrains];
  // for (int i = 0; i < numGrains; i++) {
  //   this->dropGrain(i);
  // }

  Serial.println("Done creating source");
}

void Source::next(int time) {
  int count = time / interval - lastDropTime / interval;

  if (count > 0) {
    this->lastDropTime = (time / interval) * interval;  // round down to interval
    for (int i = 0; i < count; ++i) {
      this->dropGrain(this->nextIndex + i);
    }
    this->nextIndex = (this->nextIndex + count) % this->numGrains;
  }
}

void Source::updateGrains() {
  for (int i = 0; i < this->numGrains; i++) {
    Grain grain = this->grains[i];
    grain.Update();
  }
}


void Source::dropGrain(int index) {
  Grain *grain = &this->grains[index];

  int x = random(this->x - this->dropWindow, this->x + this->dropWindow);
  int y = random(0, 50);

  grain->drop(x, y);
  grain->velo = this->velo;
}
