#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <Arduino.h>
#include <vector>
#include <functional>
#include "CustomWiFiManager.h"
#include "Weather.h"
#include "WebServer.h"
#include "Config.h"
#include "NeoPixel.h"

/**
 * @class Protocol
 * @brief Task scheduler and system manager that follows the Singleton pattern
 * 
 * This class manages all system components and scheduled tasks, coordinating
 * their execution. It ensures there's only one instance of the system manager.
 */
class Protocol {
private:
    static Protocol* instance;         // Singleton instance pointer
    
    // System components
    CustomWiFiManager* wifiManager;    // WiFi manager 
    Weather* weatherService;           // Weather service
    WebServer* webServer;              // Web server
    NeoPixel* neoPixel;                // NeoPixel manager
    
    // System state
    bool* ledState;                    // LED state reference
    int* brightness;                   // LED brightness reference
    
    // Task Timing
    unsigned long lastWeatherUpdate;
    unsigned long lastSystemMonitor;
    unsigned long lastNeoPixelUpdate;

    /**
     * @brief Private constructor to enforce Singleton pattern
     */
    Protocol();

    // Helper to update LED based on weather ID
    void updateLedBasedOnWeather(int weatherId);

public:
    /**
     * @brief Gets or creates the singleton instance
     * @return Pointer to the Protocol instance
     */
    static Protocol* getInstance();
    
    /**
     * @brief Initializes all system components
     * @return true if all components initialized successfully
     */
    bool initializeSystem();
    
    /**
     * @brief Main loop update function. Call this from main loop().
     */
    void update();
    
    /**
     * @brief Task function to update weather information
     */
    void weatherUpdateTask();
    
    /**
     * @brief Task function to monitor system resources
     */
    void systemMonitorTask();
    
    /**
     * @brief Task function to update NeoPixel patterns
     */
    void neoPixelTask();
    
    /**
     * @brief Gets the WiFi manager instance
     * @return Pointer to the CustomWiFiManager
     */
    CustomWiFiManager* getWiFiManager() { return wifiManager; }
    
    /**
     * @brief Gets the WebServer instance
     * @return Pointer to the WebServer
     */
    WebServer* getWebServer() { return webServer; }
    
    /**
     * @brief Gets the Weather service instance
     * @return Pointer to the Weather service
     */
    Weather* getWeatherService() { return weatherService; }
};

#endif // PROTOCOL_H