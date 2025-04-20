#ifndef CUSTOM_WIFI_MANAGER_H  // Changed header guard name
#define CUSTOM_WIFI_MANAGER_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <Ticker.h>
#include <WiFiManager.h>  // Third-party WiFiManager library

class CustomWiFiManager {
private:
    String hostname;
    ::WiFiManager* wifiManager;  // Global namespace to reference the library class
    Ticker wifiTicker;
    bool shouldRun;
    void checkWiFiConnection();

public:
    CustomWiFiManager(const char* deviceName = "LEDcloud");
    ~CustomWiFiManager();  // Add destructor to clean up pointer
    void startTask();
    void stopTask();
    bool begin();
    void reset();
    bool isConnected();
    String getIP();
    String getHostname();
};

#endif // CUSTOM_WIFI_MANAGER_H