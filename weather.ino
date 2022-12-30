//#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include "html.h"
#include "credentialsStorage.h"
//#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "button.h"

Button butt(D3);

enum WifiStatus {
  OFF,
  CONNECTING,
  CONNECTED,
  FAILED
};

LiquidCrystal_I2C lcd(0x27, 16, 2);
ESP8266WebServer server(80); // create a web server on port 80
enum WifiStatus wifiStatus = OFF;
long lastAttempt = millis();
int attempts = 0;
String ssid;
String password;
bool saveToNvm = false; 

const String spinner = "|/-\\";
int spinnerIndex = 0;

void handleStatus() {
  String status;
  switch (wifiStatus) {
    case OFF: status = "OFF"; break;
    case CONNECTING: status = "CONNECTING..."; break;
    case CONNECTED: status = "CONNECTED " + ssid; break;
    case FAILED: status = "FAILED"; break;
  }
  server.send(200, "text/plain", status);
}

void handleIndex() {
  server.send(200, "text/html", html);
}

void handleDisconnect() {
  wifiStatus = OFF;
  WiFi.disconnect();
  clearWiFiCredentials();
  lcd.setCursor(0,1);
  lcd.print("Disconnected    ");
  server.send(200, "text/html", "Disconnected to Wifi");
}

void attemptToConnect() {
  if (wifiStatus == OFF || wifiStatus == FAILED) return;
  
  long now = millis();
  if (now - lastAttempt < 500) return;
  
  if (WiFi.status() == WL_CONNECTED) {
    if (wifiStatus == CONNECTED) return;
    Serial.println("Connected!");
    lcd.setCursor(0,1);
    lcd.print("Connected       ");
    
    WiFi.setAutoReconnect(true);
    wifiStatus = CONNECTED;
    attempts = 0;
    if (saveToNvm) {
      saveWiFiCredentials(ssid, password);
      saveToNvm = false;
    }
  } else {
    Serial.print(".");
    if (attempts > 20) {
      wifiStatus = FAILED;
      clearWiFiCredentials();
      attempts = 0;      
      Serial.println("Failed!");
      lcd.setCursor(0,1);
      lcd.print("Failed          ");
    } else {
      wifiStatus = CONNECTING;
      attempts ++;
      lcd.setCursor(14,1);
      lcd.print(spinner[spinnerIndex]); 
      spinnerIndex ++;
      if (spinnerIndex >= spinner.length()) spinnerIndex = 0;
    }
  }
  lastAttempt = now;
}

void connectWifi(String ssid, String password) {
    WiFi.disconnect();
    WiFi.begin(ssid.c_str(), password.c_str());
    wifiStatus = CONNECTING;

    Serial.println("Connecting...");
    lcd.setCursor(0,1);
    lcd.print("Connecting...   ");
}

void handleConnect() {
  if (server.method() == HTTP_POST) {
    Serial.println("handleConnect");
    ssid = server.arg("ssid");
    password = server.arg("password");
    saveToNvm = true;
    connectWifi(ssid, password);
    server.send(200, "text/plain", "Connecting to WiFi");
  } else {
    server.send(405, "text/plain", "Method Not Allowed");
  }
}

//byte backslash[8] = { 0x00, 0x10, 0x08, 0x04, 0x02, 0x01, 0x00, 0x00 }; 

void holdHandler() {
  Serial.println("click");
  lcd.setCursor(15, 1);
  lcd.print("&");
}
void clickHandler() {
  Serial.println("click");
  lcd.setCursor(15, 1);
  lcd.print(" ");  
}
void setup() {
  Serial.begin(9600);
  lcd.init();
//  lcd.createChar(1, backslash);
  lcd.clear();         
  lcd.backlight();      // Make sure backlight is on
    
  //Wire.begin();
  EEPROM.begin(EEPROM_SIZE);
  server.on("/status", handleStatus);
  server.on("/", handleIndex);
  server.on("/connect", handleConnect);
  server.on("/disconnect", handleDisconnect);
  WiFi.mode(WIFI_AP);
  WiFi.softAP("WeatherHotspot", "");
  IPAddress myIP = WiFi.softAPIP();
  server.begin(); // start the web server
  Serial.print("Server started:");
  Serial.println(myIP.toString());

  lcd.setCursor(0,0);
  lcd.print(myIP.toString());
  lcd.setCursor(0,1);
  lcd.print("Disconnected    ");


  bool hasSavedCredentials = hasSavedWiFiCredentials();
  Serial.println(hasSavedCredentials ? "Has credentials" : "No credentials");
  
  if (hasSavedCredentials) {
    password = getSavedPassword();
    ssid = getSavedSSID();
    saveToNvm = false;
    Serial.println(ssid);
    Serial.println(password);
    connectWifi(ssid, password);
  }

  butt.setHold(2000, holdHandler);
  butt.setClick(clickHandler);
}

void loop() {
  server.handleClient(); // handle incoming HTTP requests
  attemptToConnect();
  butt.tick();
}
