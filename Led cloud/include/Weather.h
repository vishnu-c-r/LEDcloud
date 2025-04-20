#ifndef WEATHER_H
#define WEATHER_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <Ticker.h>
#include <LittleFS.h>
#include "Config.h" // Include the configuration file

class Weather {
private:
    // Private constructor for singleton pattern
    Weather();
    
    // Static instance for singleton pattern
    static Weather* instance;
    
    // API configuration
    String apiKey;
    float latitude;
    float longitude;
    
    // Weather data
    float temperature;
    int humidity;    // Added humidity field
    String weatherDescription;
    String weatherIcon;
    
    // Update tracking
    unsigned long lastUpdateTime;
    bool shouldRun;
    
    // Ticker for scheduling updates
    Ticker weatherTicker;
    
    // Settings file path
    static const char* SETTINGS_FILE;
    
    // Rate limiting
    static unsigned long lastApiCallTime;
    static const unsigned long MIN_API_CALL_INTERVAL; // Minimum time between API calls
    
    // API call counter
    static unsigned long apiCallCount;

public:
    // API call limit management - using interval from Config.h
    static const unsigned long UPDATE_INTERVAL = WEATHER_UPDATE_INTERVAL;
    
    // Delete copy constructor and assignment operator
    Weather(const Weather&) = delete;
    void operator=(const Weather&) = delete;
    
    // Get the singleton instance
    static Weather* getInstance();
    
    // Initialize with API key and location
    void begin(const String& apiKey, float latitude, float longitude);
    
    // Initialize with config defaults
    void beginWithDefaults();
    
    // Initialize with saved settings or defaults
    bool beginWithSavedSettings();
    
    // Save settings to file system
    bool saveSettings(const String& apiKey, float latitude, float longitude);
    
    // Load settings from file system
    bool loadSettings(String& apiKey, float& latitude, float& longitude);
    
    // Update weather data with new settings
    bool updateSettings(const String& apiKey, float latitude, float longitude);
    
    // Start the weather update task
    void startTask();
    
    // Stop the weather update task
    void stopTask();
    
    // Fetch weather data from API
    void fetchWeatherData();
    
    // Get weather data as JSON string
    String toJson() const;
    
    // Get settings as JSON string
    String getSettingsJson() const;
    
    // Manual trigger to update weather immediately
    void updateNow();
    
    // Getters for weather data
    float getTemperature() const { return temperature; }
    int getHumidity() const { return humidity; }    // Added getter for humidity
    String getDescription() const { return weatherDescription; }
    String getIcon() const { return weatherIcon; }
    String getLastUpdateTime() const;
    
    // Getters for settings
    String getApiKey() const { return apiKey; }
    float getLatitude() const { return latitude; }
    float getLongitude() const { return longitude; }
    
    // Get API call count
    static unsigned long getApiCallCount() { return apiCallCount; }
};

#endif // WEATHER_H