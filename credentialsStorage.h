#ifndef CREDENTIALS_STORAGE_H
#define CREDENTIALS_STORAGE_H

#include <EEPROM.h>
const int EEPROM_ADDR_SSID = 0; // address in the NVM to store the SSID
const int EEPROM_ADDR_PASSWORD = 32; // address in the NVM to store the password
const int EEPROM_SIZE = 64;

void clearWiFiCredentials() {
  // clear the SSID and password from the NVM
  for (int i = EEPROM_ADDR_SSID; i < EEPROM_ADDR_PASSWORD + 32; i++) {
    EEPROM.write(i, 0);
  }
  EEPROM.commit(); // save the data to the NVM
}

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

void saveWiFiCredentials(String ssid, String password) {
  writeString(EEPROM_ADDR_SSID, ssid, 32);
  writeString(EEPROM_ADDR_PASSWORD, password, 32);
  EEPROM.commit();
}

String getSavedSSID() {
  return readString(EEPROM_ADDR_SSID, 32);
}

String getSavedPassword() {
  return readString(EEPROM_ADDR_PASSWORD, 32);
}

bool hasSavedWiFiCredentials() {
  return EEPROM.read(EEPROM_ADDR_SSID) != 0 && EEPROM.read(EEPROM_ADDR_PASSWORD) != 0;
}

#endif
