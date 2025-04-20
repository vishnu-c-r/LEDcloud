#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <Arduino.h>
// Include our HTTP constants first before ESPAsyncWebServer.h
#include "HttpConstants.h"
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "Weather.h"

/**
 * @class WebServer
 * @brief Handles web server functionality for the LEDcloud project
 * 
 * This class encapsulates all web server functionality including
 * setting up routes, handling API requests, and serving static files.
 */
class WebServer {
private:
    AsyncWebServer server;
    
    // LED control variables
    bool* ledState;
    int* brightness;
    unsigned long startTime;
    
    // Weather instance
    Weather* weatherService;
    
    /**
     * @brief Initialize the LittleFS file system
     * @return true if successful, false otherwise
     */
    bool initFileSystem();
    
    /**
     * @brief Handle requests that were not found (404 error)
     */
    static void notFound(AsyncWebServerRequest *request);
    
public:
    /**
     * @brief Constructor
     * @param port Server port number
     * @param ledStatePtr Pointer to the LED state variable
     * @param brightnessPtr Pointer to the brightness variable
     */
    WebServer(uint16_t port, bool* ledStatePtr, int* brightnessPtr);
    
    /**
     * @brief Initialize and start the web server
     */
    void begin();
    
    /**
     * @brief Set up all routes for the web server
     */
    void setupRoutes();
    
    /**
     * @brief Update the physical LED based on current state and brightness
     */
    void updateLED();
    
    /**
     * @brief Initialize weather service with API key and location
     * @param apiKey OpenWeatherMap API key
     * @param city City name for weather data
     * @param country Optional country code
     */
    void initWeather(const String& apiKey, const String& city, const String& country = "");
};

#endif // WEBSERVER_H