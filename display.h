#ifndef DISPLAY_H
#define DISPLAY_H

#include <LiquidCrystal_I2C.h>

byte backslash[] = { 0x00, 0x10, 0x08, 0x04, 0x02, 0x01, 0x00, 0x00 }; 
byte pressure[] = { 0x04, 0x04, 0x04, 0x04, 0x04, 0x15, 0x0E, 0x04 };

class Display {
  private:
    LiquidCrystal_I2C lcd;
    bool backlightStatus = true;
  public:
    Display(byte addr) : lcd(addr, 16, 2) {
    }
    void init() {
      this->lcd.init();
      this->lcd.createChar(0, pressure);
      this->lcd.createChar(1, backslash);      
      this->lcd.clear();         
      this->lcd.backlight();
      this->backlightStatus = true;
    }
    void write(int x, int y, String str) {
      this->lcd.setCursor(x, y);
      this->lcd.print(str);
    }
    void writePressure(int x, int y) {
      this->lcd.setCursor(x, y);
      this->lcd.write(byte(0));
    }
    void writeBackslash(int x, int y) {
      this->lcd.setCursor(x, y);
      this->lcd.write(byte(1));
    }
    void write(int x, int y, char chr) {
      this->lcd.setCursor(x, y);
      this->lcd.print(chr);
    }
    void writeLine(int y, String str) {
      this->lcd.setCursor(0, y);
      String lineStr = str;
      for (int i = str.length(); i < 16; i++) {
        lineStr += " ";
      }
      this->lcd.print(lineStr);    
    }
    void clear() {
      this->lcd.clear();
    }
    void backlight(bool status) {
      if (status) {
        this->lcd.backlight();      
      } else {
        this->lcd.noBacklight();        
      }
      this->backlightStatus = status;
    }
    void toggleBacklight() {
      this->backlight(!this->backlightStatus);
    }
    bool getBacklight() {
      return this->backlightStatus;
    }
};

#endif
