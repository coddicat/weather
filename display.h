#ifndef DISPLAY_H
#define DISPLAY_H

#include <LiquidCrystal_I2C.h>

//byte backslash[8] = { 0x00, 0x10, 0x08, 0x04, 0x02, 0x01, 0x00, 0x00 }; 

class Display {
  private:
    LiquidCrystal_I2C lcd;
  public:
    Display(byte addr) : lcd(addr, 16, 2) {
    }
    void init() {
      this->lcd.init();
    //  this->lcd.createChar(1, backslash);
      this->lcd.clear();         
      this->lcd.backlight();      // Make sure backlight is on  
    }
    void write(int x, int y, String str) {
      this->lcd.setCursor(x, y);
      this->lcd.print(str);
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
};

#endif
