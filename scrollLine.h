#ifndef SCROLL_LINE_H
#define SCROLL_LINE_H

#include "display.h"

class ScrollLine {
  private:
    String text;
    Display *display;
    int x = 0;   
    long lastTime = 0;
    long delay;
    int y;

    void displayLine() {      
      int i = -this->x;
      int l = this->text.length();

      for(int x = 0; x < 16; x++) {
        if (i >= 0 && i < l) {
          display->write(x, this->y, this->text[i]);        
        } else {
          display->write(x, this->y, ' ');
        }
        i++;
      }      
    }
  public:
    ScrollLine(Display *display, long delay, int y, String text) {
      this->delay = delay;
      this->display = display;
      this->y = y;
      this->text = text;
    }
    void setText(String text) {
      this->text = text; 
    }
    void tick() {
      long now = millis();
      if (now - this->lastTime > this->delay) {
        this->x-=1;
        int len = this->text.length();
        if (this->x < -len) {
          this->x = 16;
        }
        displayLine();
        this->lastTime = now;
      }
    }
};

#endif
