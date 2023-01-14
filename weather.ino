#include "server.h"
#include "wifi.h"
#include "wifiStorage.h"
#include "button.h"
#include "display.h"
#include "spinner.h"
//#include "scrollLine.h"

#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

Display disp(0x27);
Button butt(D3);
WifiStorage wifiStorage;
WeatherWifi wifi;
WeatherServer server(80, &wifi, &wifiStorage, &disp);

String password;
String ssid;

const long dsec = 15;
long lastButton = 0;
long lastObtain = -dsec*60*1000;
long lastTime = -1000;
bool settingsMode = false;
WifiStatus status = DISCONNECTED;
bool hasCredentials;

void holdHandler() {
  lastButton = millis();
  
  if (settingsMode) return;

  if (status == DISCONNECTED && hasCredentials) {
    wifi.connect(ssid, password);
  }
  
  server.start();
  settingsMode = true;  
  disp.backlight(true);
}

void clickHandler() {
  lastButton = millis();
  if (!settingsMode) {
    disp.toggleBacklight();
    return;
  }
  disp.backlight(true);
  server.stop();
  getStoredData();
  lastObtain = -dsec*60*1000;
  settingsMode = false; 
  disp.writeLine(0, "Wait...");
  disp.writeLine(1, "");   
  displayStatus(); 
}

void displayStatus() {  
  char chr;
  switch (status) {
    case CONNECTED: chr = '~'; break;
    case DISCONNECTED:
      return;
    case FAILED: 
      chr = '!'; break;
  }
  disp.write(15, 1, chr);
}

//https://api.ipgeolocation.io/ipgeo?apiKey=
const String timeUrl = "https://worldtimeapi.org/api/ip";
const String apiUrl = "https://api.openweathermap.org/data/2.5/weather?q=";
String city;
String country;
String apiKey;
//ScrollLine scroll(&disp, 500, 1, "-  -  -  -  -  -");

long unixTime = 0;
long timeOn = -1;

void obtainTime() {
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient https;
  if (https.begin(client, timeUrl)) {
      int httpCode = https.GET();
      Serial.println("Response code: " + String(httpCode));
      if (httpCode > 0) {
        StaticJsonDocument<1024> doc;
        String response = https.getString();
        DeserializationError error = deserializeJson(doc, response);
        if (error) {
          Serial.println("Error parsing JSON");
        } else {
          long dt = doc["unixtime"];
          int tz = doc["raw_offset"];
          unixTime = dt + tz;          
        }
      }
      https.end();
  }
  else {
    disp.writeLine(0, "Unable to connect");   
  }   
}

void obtainWeather() {
  Serial.println("obtainWeather...");
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
           
            String line = tempStr + degree + humiStr + "% "; 
            disp.writeLine(0, line);
            disp.writeLine(1, main + " " + String(fixSpeed) + "km/h");
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
  Serial.println("Begin");
  butt.setHold(2000, holdHandler);
  butt.setClick(clickHandler);
  disp.init();
  server.init();
  wifiStorage.init();
  
  hasCredentials = wifiStorage.hasStored();
  Serial.println(hasCredentials ? "Has wifi credentials:" : "No wifi credentials");  
  
  if (hasCredentials) {
    getStoredData();
    password = wifiStorage.getPassword();
    ssid = wifiStorage.getSSID();
    wifi.connect(ssid, password);    
    settingsMode = false;
    disp.writeLine(0, "Wait...");
    displayStatus();
  } else {
    server.start();
    settingsMode = true;
  }

  lastButton = millis();
}

Spinner spinner(&disp, 300, 15, 1);

String separate = ":";
void showTime() {
  long diff = (millis() - timeOn) / 1000;
  long now = unixTime + diff;
  int hours = now / 3600 % 24;
  int minutes = now / 60 % 60;
  String hoursStr = String(hours);
  String minutesStr = String(minutes);
  if (hoursStr.length() == 1) hoursStr = "0" + hoursStr;
  if (minutesStr.length() == 1) minutesStr = "0" + minutesStr;
  disp.write(11, 0, hoursStr + separate + minutesStr);  
  separate = separate == ":" ? " " : ":";
}

void loop() {
  if (settingsMode) {
    server.tick();    
  } else {
    WifiStatus newStatus = wifi.getStatus();

    long now = millis();
    if (status == CONNECTED) {
      //scroll.tick();

      if (timeOn == -1) {
        obtainTime();
        timeOn = now;
      }
      
      //15 minutes
      if (now - lastObtain > dsec*60*1000) {        
        obtainWeather();
        lastObtain = now;
        lastButton = now;
        //ESP.deepSleep(2e7);
        wifi.disconnect();
      }
    }

    if (now - lastTime > 1000) {
      showTime();
      lastTime = now;
    }      

    if (status == CONNECTING) {
      spinner.tick();     
    }

    if (status != newStatus) {
      status = newStatus;
      displayStatus();
    }   

    if (status == DISCONNECTED && now - lastObtain > dsec*60*1000) {
        wifi.connect(ssid, password);
    }

    if (now - lastButton > 30*1000 && disp.getBacklight()) {
      disp.backlight(false);
    }
  }
  wifi.tick();
  butt.tick(); 
}
