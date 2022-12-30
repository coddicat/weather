#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>
//#include "backlight.h"

class Button
{
private:
  void (*clickHandler)();
  bool hasClick;
  void (*holdHandler)();
  bool hasHold;
  void (*upHandler)();
  bool hasUp;
  void (*downHandler)();
  bool hasDown;
  long holdDelay;
  int pin;
  long releaseTime;
  bool lastState;
  bool holdFlag;
  long holdReleaseTime;
  //Backlight *backlight;
  bool hasBacklight;

public:
  Button(int pin)
  {
    this->pin = pin;
    pinMode(pin, INPUT_PULLUP);
  }
  void setClick(void (*handler)())
  {
    this->clickHandler = handler;
    this->hasClick = true;
  }
  void setHold(long holdDelay, void (*handler)())
  {
    this->holdHandler = handler;
    this->holdDelay = holdDelay;
    this->hasHold = true;
  }
  void setDown(void (*handler)())
  {
    this->downHandler = handler;
    this->hasDown = true;
  }
  void setUp(void (*handler)())
  {
    this->upHandler = handler;
    this->hasUp = true;
  }
  // void setBacklight(Backlight *backlight) {
  //   this->backlight = backlight;
  //   this->hasBacklight = true;
  // }
  void tick()
  {
    bool state = !digitalRead(this->pin);
    long now = millis();

    if (state != this->lastState && this->releaseTime < now)
    {
      this->releaseTime = now + 200;
      this->lastState = state;
      if (state)
      {
        this->holdReleaseTime = now + 2000;
        this->holdFlag = false;
        if (this->hasDown)
        {
          this->downHandler();
        }
        //   if(this->hasBacklight) {
        //     this->backlight->highlight();
        //   }
      }
      else
      {
        if (this->hasUp)
        {
          this->upHandler();
        }
        //   if (this->hasBacklight) {
        //     this->backlight->release();
        //   }
        if (!this->holdFlag && this->hasClick)
        {
          this->clickHandler();
        }
      }
    }
    if (this->lastState && this->holdReleaseTime < now && this->releaseTime < now && !this->holdFlag && this->hasHold)
    {
      this->holdFlag = true;
      this->holdHandler();
    }
  }
};

#endif
