#include "server.h"
#include "wifi.h"
#include "wifiStorage.h"
#include "button.h"
#include "display.h"
#include "spinner.h"
#include "scrollLine.h"

#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

Display disp(0x27);
Button butt(D3);
WifiStorage wifiStorage;
WeatherWifi wifi;
WeatherServer server(80, &wifi, &wifiStorage, &disp);

long lastObtain = -10*60*1000;
bool settingsMode = false;

void holdHandler() {
  if (settingsMode) return;
  server.start();
  settingsMode = true;  
  disp.backlight(true);
}
void clickHandler() {
  if (!settingsMode) {
    disp.toggleBacklight();
    return;
  }
  disp.backlight(true);
  server.stop();
  getStoredData();
  lastObtain = -10*60*1000;
  settingsMode = false; 
  disp.writeLine(0, "Wait...");
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

//https://api.ipgeolocation.io/ipgeo?apiKey=

const String apiUrl = "https://api.openweathermap.org/data/2.5/weather?q=";
String city;
String country;
String apiKey;

ScrollLine scroll(&disp, 500, 1, "-  -  -  -  -  -");
void obtainWeather() {
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient https;
  String url = apiUrl + city + ',' + country + "&appid=" + apiKey + "&units=metric";
  if (https.begin(client, url))
  {
      int httpCode = https.GET();
      Serial.println("Response code: " + String(httpCode));
      if (httpCode > 0)
      {
        StaticJsonDocument<1024> doc;
        String response = https.getString();
        DeserializationError error = deserializeJson(doc, response);
        if (error) {
          Serial.println("Error parsing JSON");
        } else {

          disp.clear();
          if (httpCode == 200) {
            int temp = doc["main"]["temp"];
            int humi = doc["main"]["humidity"];
            int press = doc["main"]["pressure"];
            String main = doc["weather"][0]["main"];
            String desc = doc["weather"][0]["description"];
            float speed = doc["wind"]["speed"];
            int fixSpeed = speed * 3.6;
            String tempStr = String(temp);
            String humiStr = String(humi);
            String pressStr = String(press);

            char degree[] = { 0xDF, 'C', ' ', 0x00 };
            String line = tempStr + degree + humiStr + "% " + pressStr;
            disp.writeLine(0, line);
            disp.writePressure(line.length(), 0);            
            scroll.setText(main + ": " + desc + ", wind:" + String(fixSpeed) + "km/h, " + city);            
          } else {
            disp.writeLine(0, "Something wrong");
          }          
        }
      }
      https.end();
  }
  else
  {
    disp.writeLine(0, "Unable to connect");
  }   
}

void getStoredData() {
  city = wifiStorage.getCity();
  country = wifiStorage.getCountry();
  apiKey = wifiStorage.getApiKey();
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
    getStoredData();
    String password = wifiStorage.getPassword();
    String ssid = wifiStorage.getSSID();
    wifi.connect(ssid, password);    
    settingsMode = false;
    disp.writeLine(0, "Wait...");
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

    long now = millis();
    if (status == CONNECTED) {
      scroll.tick();
  
      //5 minutes
      if (now - lastObtain > 5*60*1000) {
        obtainWeather();
        lastObtain = now;        
      }
    }

    if (status == CONNECTING) {
      spinner.tick();     
    }

  }
  wifi.tick();
  butt.tick(); 
}
