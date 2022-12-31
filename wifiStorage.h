#ifndef CREDENTIALS_STORAGE_H
#define CREDENTIALS_STORAGE_H

#include <EEPROM.h>
const int EEPROM_ADDR_SSID = 0; // address in the NVM to store the SSID
const int EEPROM_ADDR_PASSWORD = 32; // address in the NVM to store the password
const int EEPROM_ADDR_CITY = 64;
const int EEPROM_ADDR_COUNTRY = 96;
const int EEPROM_ADDR_APIKEY = 128;
const int EEPROM_SIZE = 192;

class WifiStorage {
  private:
  
    void writeString(int addr, String str, int max) {
      int i = 0;
      int len = str.length();
      while(i < len && i < max - 1) {
        EEPROM.write(addr + i, str[i]);
        i++;
      }
      EEPROM.write(addr + i + 1, 0); // null-terminate the string   
    }
    
    String readString(int addr, int max) {
      String str;
      for (int i = 0; i < max; i++) {
        char c = EEPROM.read(addr + i);
        if (c == 0) {
          break;
        }
        str += c;
      }
      return str;  
    }  
    
  public:
    WifiStorage() {
        //EEPROM.begin(EEPROM_SIZE);
    }

    void init() {
      EEPROM.begin(EEPROM_SIZE);
    }
    
    void clear() {
      for (int i = EEPROM_ADDR_SSID; i < EEPROM_ADDR_SSID + EEPROM_SIZE; i++) {
        EEPROM.write(i, 0);
      }
      EEPROM.commit();
    }

    void storeWifi(String ssid, String password) {
      writeString(EEPROM_ADDR_SSID, ssid, 32);
      writeString(EEPROM_ADDR_PASSWORD, password, 32);
      EEPROM.commit();
    }

    void storeData(String city, String country, String apiKey) {
      writeString(EEPROM_ADDR_CITY, city, 32);
      writeString(EEPROM_ADDR_COUNTRY, country, 32);
      writeString(EEPROM_ADDR_APIKEY, apiKey, 64);
      EEPROM.commit();
    }
    
    String getSSID() {
      return readString(EEPROM_ADDR_SSID, 32);
    }   
    String getPassword() {
      return readString(EEPROM_ADDR_PASSWORD, 32);
    }
    String getApiKey() {
      return readString(EEPROM_ADDR_APIKEY, 64);
    }
    String getCity() {
      return readString(EEPROM_ADDR_CITY, 32);
    }    
    String getCountry() {
      return readString(EEPROM_ADDR_COUNTRY, 32);
    }    
    bool hasStored() {
      return 
        EEPROM.read(EEPROM_ADDR_SSID) != 0 
        && EEPROM.read(EEPROM_ADDR_APIKEY) != 0
        && EEPROM.read(EEPROM_ADDR_CITY) != 0
        && EEPROM.read(EEPROM_ADDR_COUNTRY) != 0;
        //&& EEPROM.read(EEPROM_ADDR_PASSWORD) != 0;
    }
};

#endif
