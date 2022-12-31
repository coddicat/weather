#ifndef WEATHER_SERVER_H
#define WEATHER_SERVER_H

#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include "wifiStorage.h"
#include "wifi.h"
#include "display.h"
#include "html.h"
#include "spinner.h"

class WeatherServer {
  private:
    ESP8266WebServer server;
    WeatherWifi *wifi;
    WifiStorage *storage;
    Display *display;
    Spinner *spinner;
    
    String ssid;
    String password;
    String city;
    String country;
    String apiKey;
    WifiStatus status = DISCONNECTED;
    bool newCredentials = false;

    void ok(String response) {
      this->server.send(200, "text/html", response);      
    }

    void notAllowed(String response) {
      this->server.send(405, "text/plain", response);
    }

    void onStatus() {
      String status = getStatusString();
      this->ok(status);
    }

    String getStatusString() {
      String statusStr;
      WifiStatus status = this->wifi->getStatus();
      switch (status) {
        case DISCONNECTED: statusStr = "DISCONNECTED"; break;
        case CONNECTING: statusStr = "CONNECTING..."; break;
        case CONNECTED: statusStr = "CONNECTED"; break;
        case FAILED: statusStr = "FAILED"; break;
      }
      return statusStr;     
    }

    void onData() {
      String response = "{\"wifi\":{\"status\":\"\",ssid:\"\"},\"weather\":{\"city\":\"\",\"country\":\"\",\"apiKey\":\"\"}}";
      DynamicJsonDocument doc(1024);

      //return function is error
      if (deserializeJson(doc, response)) {
        Serial.println("Error parsing JSON");
        this->ok(response);
        return;
      }

      JsonObject obj = doc.as<JsonObject>();
      obj["wifi"]["status"]=getStatusString();
      obj["wifi"]["ssid"]=ssid;
      obj["weather"]["city"]=city;
      obj["weather"]["country"]=country;
      obj["weather"]["apiKey"]=apiKey;
      String output;
      serializeJson(doc, output);
      this->ok(output);
    }    
      
    void onIndex() {
      this->ok(html);
    }

    void onDisconnect() {
      this->wifi->disconnect();
      this->storage->storeWifi(ssid,"");
      this->display->writeLine(1, "Disconnected");
      this->ok("Disconnected to Wifi");
    }

    void onConnect() {
      if (this->server.method() == HTTP_POST) {
        this->newCredentials = true;
        this->ssid = server.arg("ssid");
        this->password = server.arg("password");
        this->wifi->connect(ssid, password);   
        this->ok("Connecting to WiFi");
      } else {
        this->notAllowed("Method Not Allowed");
      }
    }    

    void onSave() {
      if (this->server.method() == HTTP_POST) {
        this->city = server.arg("city");
        this->country = server.arg("country");
        this->apiKey = server.arg("apiKey");
        this->storage->storeData(this->city, this->country, this->apiKey);
        this->ok("Data saved");
      } else {
        this->notAllowed("Method Not Allowed");
      }      
    }

    void displayStatus() {
      WifiStatus status = this->wifi->getStatus();
      String str;
      switch (status) {
        case DISCONNECTED: str = "DISCONNECTED"; break;
        case CONNECTING: str = "CONNECTING..."; break;
        case CONNECTED: str = "CONNECTED"; break;
        case FAILED: str = "FAILED"; break;
      }
      this->display->writeLine(1, str);     
    }
    
  public:
    WeatherServer(int port, WeatherWifi *wifi, WifiStorage *storage, Display *display): server(port) {   
      this->wifi = wifi;
      this->storage = storage;
      this->display = display;
      this->spinner = new Spinner(display, 300, 15, 1);
    }
    void init() {
      this->server.on("/data", std::bind(&WeatherServer::onData, this));
      this->server.on("/status", std::bind(&WeatherServer::onStatus, this));
      this->server.on("/", std::bind(&WeatherServer::onIndex, this));
      this->server.on("/connect", std::bind(&WeatherServer::onConnect, this));
      this->server.on("/save", std::bind(&WeatherServer::onSave, this));
      this->server.on("/disconnect", std::bind(&WeatherServer::onDisconnect, this));       
    }
    void start() {
      this->city = this->storage->getCity();
      this->country = this->storage->getCountry();
      this->apiKey = this->storage->getApiKey();
      this->ssid = this->storage->getSSID();
      this->server.begin(); 
      String ip = this->wifi->startAp();
      this->display->writeLine(0, ip);
      this->displayStatus();        
    }
    void stop() {
      this->server.stop(); 
      this->wifi->stopAp();
    }
    void tick() {
      WifiStatus status = this->wifi->getStatus();
      if (this->status != status) {
        this->displayStatus();
        this->status = status;
        if (status == CONNECTED && this->newCredentials) {
          this->storage->storeWifi(this->ssid, this->password);
        }
      }
      
      if (status == CONNECTING) {
        this->spinner->tick();
      }
      
      this->server.handleClient();
    }
};

#endif
