#include "server.h"
#include "wifi.h"
#include "wifiStorage.h"
#include "button.h"
#include "display.h"
#include "spinner.h"

Display disp(0x27);
Button butt(D3);
WifiStorage wifiStorage;
WeatherWifi wifi;
WeatherServer server(80, &wifi, &wifiStorage, &disp);

bool settingsMode = false;
void holdHandler() {
  if (settingsMode) return;
  server.start();
  settingsMode = true;  
}
void clickHandler() {
  if (!settingsMode) return;
  server.stop();
  settingsMode = false; 
  disp.writeLine(0, "Ready");
  disp.writeLine(1, "");   
  displayStatus(); 
}

WifiStatus status = DISCONNECTED;
void displayStatus() {  
  char chr;
  switch (status) {
    case CONNECTED: chr = '~'; break;
    case DISCONNECTED:
    case FAILED: 
      chr = '!'; break;
  }
  disp.write(15, 0, chr);
}

void setup() {
  Serial.begin(9600);
  butt.setHold(2000, holdHandler);
  butt.setClick(clickHandler);
  disp.init();
  server.init();
  wifiStorage.init();
  
  bool hasCredentials = wifiStorage.hasStored();
  Serial.println(hasCredentials ? "Has wifi credentials:" : "No wifi credentials");  
  
  if (hasCredentials) {
    String password = wifiStorage.getPassword();
    String ssid = wifiStorage.getSSID();
    Serial.println(ssid);
    Serial.println(password);
    wifi.connect(ssid, password);
    
    settingsMode = false;
    disp.writeLine(0, "Ready!");
    displayStatus();
  } else {
    server.start();
    settingsMode = true;
  }
}

Spinner spinner(&disp, 300, 15, 0);

void loop() {
  if (settingsMode) {
    server.tick();    
  } else {
    WifiStatus newStatus = wifi.getStatus();
    if (status != newStatus) {
      status = newStatus;
      displayStatus();
    }   
  }
  wifi.tick();
  butt.tick(); 
  if (status == CONNECTING) {
    spinner.tick();     
  }
}
