#ifndef WEATHER_H
#define WEATHER_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <Ticker.h>

class Weather {
private:
    // Private constructor for singleton pattern
    Weather();
    
    // Static instance for singleton pattern
    static Weather* instance;
    
    // API configuration
    String apiKey;
    String city;
    String country;
    
    // Weather data
    float temperature;
    float humidity;
    float pressure;
    String weatherDescription;
    String weatherIcon;
    
    // Update tracking
    unsigned long lastUpdateTime;
    bool shouldRun;
    
    // Ticker for scheduling updates
    Ticker weatherTicker;

public:
    // API call limit management - setting to update every 2 hours (in ms)
    // This allows for 12 calls per day, well below the 900 limit
    static const unsigned long UPDATE_INTERVAL = 7200000; // 2 hours
    
    // Delete copy constructor and assignment operator
    Weather(const Weather&) = delete;
    void operator=(const Weather&) = delete;
    
    // Get the singleton instance
    static Weather* getInstance();
    
    // Initialize with API key and location
    void begin(const String& apiKey, const String& city, const String& country = "");
    
    // Start the weather update task
    void startTask();
    
    // Stop the weather update task
    void stopTask();
    
    // Fetch weather data from API
    void fetchWeatherData();
    
    // Get weather data as JSON string
    String toJson() const;
    
    // Getters for weather data
    float getTemperature() const { return temperature; }
    float getHumidity() const { return humidity; }
    float getPressure() const { return pressure; }
    String getDescription() const { return weatherDescription; }
    String getIcon() const { return weatherIcon; }
};

#endif // WEATHER_H