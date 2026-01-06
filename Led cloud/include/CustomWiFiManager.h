#ifndef CUSTOM_WIFI_MANAGER_H
#define CUSTOM_WIFI_MANAGER_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h> // Use the async WiFi manager
#include <DNSServer.h>
#include <ESP8266mDNS.h>

/**
 * @class CustomWiFiManager
 * @brief A wrapper class for the Async WiFiManager library that provides additional functionality
 * 
 * This class handles WiFi connection management, including automatic reconnection,
 * access point configuration, and mDNS setup for easy device discovery
 */
class CustomWiFiManager {
private:
    String hostname;         // Device hostname for mDNS and AP mode
    AsyncWebServer* server;  // Pointer to the AsyncWebServer instance
    DNSServer dns;           // DNS server for captive portal
    AsyncWiFiManager* wifiManager; // Pointer to the async WiFiManager instance

    // Polling control
    unsigned long lastCheck;

    void checkWiFiConnection();

public:
    CustomWiFiManager(const char* deviceName = "LEDcloud", AsyncWebServer* asyncServer = nullptr);
    ~CustomWiFiManager();

    // Replaces startTask/stopTask with a main-loop update function
    void update();

    bool begin();
    void reset();
    bool isConnected();
    String getIP();
    String getHostname();
    
    // New method for setting up client IP logging
    void setupClientIPLogging(AsyncWebServer* server);
};

#endif // CUSTOM_WIFI_MANAGER_H