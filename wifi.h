#ifndef WEATHER_WIFI_H
#define WEATHER_WIFI_H

#include <ESP8266WiFi.h>
#include <WiFiClient.h>

enum WifiStatus {
  DISCONNECTED,
  CONNECTING,
  CONNECTED,
  FAILED
};

class WeatherWifi {
  private:
    WifiStatus status = DISCONNECTED;
    int attempts = 0;
    long lastAttempt = 0;

    void handleConnected() {
      if (this->status == CONNECTED) return;    
      WiFi.setAutoReconnect(true);
      this->status = CONNECTED;
      this->attempts = 0;
      Serial.println("Connected");
    }
    
    void handleFailed() {
      if (this->status == FAILED) return;  
      this->status = FAILED;
    }
    
    void handleConnecting() {
      Serial.println("Connecting");
      if (this->status != CONNECTING) {
        this->status = CONNECTING;
      }
      this->attempts ++;      
    }
    
    
    
  public:
    WeatherWifi() {
      WiFi.mode(WIFI_AP);
    }
    String startAp() {      
      WiFi.softAP("WeatherHotspot", "");
      IPAddress myIP = WiFi.softAPIP();          
      return myIP.toString();
    }
    void stopAp() {
      WiFi.softAPdisconnect(true); 
    }
    void disconnect() {
      WiFi.disconnect();
      this->status = DISCONNECTED;
    }
    void connect(String ssid, String password) {
      Serial.println("connect:" + ssid + " " +password);      
      WiFi.disconnect();
      WiFi.begin(ssid.c_str(), password.c_str());
      this->status = CONNECTING;
      this->attempts = 0;
    }  
    
    void tick() {
      if (this->status == DISCONNECTED) return;
      
      long now = millis();
      if (now - this->lastAttempt < 500) return;
      
      if (WiFi.status() == WL_CONNECTED) {
        this->handleConnected();
      } else {
        if (this->attempts > 20) {
          this->handleFailed();
        } else {
          this->handleConnecting();
        }
      }
      this->lastAttempt = now;
    } 

    WifiStatus getStatus() {
      return this->status;
    }
};

#endif
