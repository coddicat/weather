#ifndef SPINNER_H
#define SPINNER_H

#include "display.h"

class Spinner {
  private:
    const String spinner = "|/-\\";
    Display *display;
    int index = 0;   
    long lastTime = 0;
    long delay;
    int x;
    int y;
  public:
    Spinner(Display *display, long delay, int x, int y) {
      this->delay = delay;
      this->display = display;
      this->x = x;
      this->y = y;
    }
    void displaySpinner(long now) {
      char chr = this->spinner[this->index];
      this->display->write(this->x, this->y, chr);
      this->lastTime = now;
    }
    void tick() {
      long now = millis();
      if (now - this->lastTime > this->delay) {
        this->index++;
        if (index >= this->spinner.length()) {
          this->index = 0;
        }
        displaySpinner(now);
      }
    }
};

#endif
