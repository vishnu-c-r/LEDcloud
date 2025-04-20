#include "Weather.h"

// Initialize static instance pointer
Weather* Weather::instance = nullptr;

Weather::Weather() 
    : temperature(0), 
      humidity(0), 
      pressure(0), 
      weatherDescription("Unknown"), 
      weatherIcon(""),
      lastUpdateTime(0),
      shouldRun(false) {
}

Weather* Weather::getInstance() {
    if (instance == nullptr) {
        instance = new Weather();
    }
    return instance;
}

void Weather::begin(const String& apiKey, const String& city, const String& country) {
    this->apiKey = apiKey;
    this->city = city;
    this->country = country;
    
    Serial.println("Weather service initialized");
    
    // Fetch initial weather data
    fetchWeatherData();
}

void Weather::startTask() {
    shouldRun = true;
    
    // Using a ticker to schedule periodic updates
    weatherTicker.attach_ms(UPDATE_INTERVAL, []() {
        Weather::getInstance()->fetchWeatherData();
    });
    
    Serial.println("Weather update task started");
}

void Weather::stopTask() {
    shouldRun = false;
    weatherTicker.detach();
    Serial.println("Weather update task stopped");
}

void Weather::fetchWeatherData() {
    if (!shouldRun) {
        return;
    }
    
    // Only fetch if WiFi is connected
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected. Skipping weather update.");
        return;
    }
    
    // HTTP client and WiFi client
    WiFiClient client;
    HTTPClient http;
    
    // Base URL for OpenWeatherMap API
    String url = "http://api.openweathermap.org/data/2.5/weather?";
    
    // Add city parameter
    url += "q=" + city;
    
    // Add country code if provided
    if (country.length() > 0) {
        url += "," + country;
    }
    
    // Add API key
    url += "&appid=" + apiKey;
    
    // Add units parameter (metric for Celsius)
    url += "&units=metric";
    
    Serial.println("Fetching weather data from: " + url);
    
    // Start the HTTP request
    http.begin(client, url);
    
    // Send HTTP GET request
    int httpResponseCode = http.GET();
    
    if (httpResponseCode > 0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        
        String payload = http.getString();
        
        // Use Arduino JSON to parse response
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, payload);
        
        if (error) {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
        } else {
            // Extract weather data from JSON
            temperature = doc["main"]["temp"];
            humidity = doc["main"]["humidity"];
            pressure = doc["main"]["pressure"];
            
            // Weather description (first item in the "weather" array)
            if (doc["weather"][0]["description"].isNull() == false) {
                weatherDescription = doc["weather"][0]["description"].as<String>();
            }
            
            // Weather icon code
            if (doc["weather"][0]["icon"].isNull() == false) {
                weatherIcon = doc["weather"][0]["icon"].as<String>();
            }
            
            Serial.println("Weather data updated successfully");
            Serial.print("Temperature: ");
            Serial.print(temperature);
            Serial.println("°C");
        }
    } else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
    }
    
    // Free resources
    http.end();
    
    // Update timestamp
    lastUpdateTime = millis();
}

String Weather::toJson() const {
    // Create a JSON document
    DynamicJsonDocument doc(256);
    
    doc["temperature"] = temperature;
    doc["humidity"] = humidity;
    doc["pressure"] = pressure;
    doc["description"] = weatherDescription;
    doc["icon"] = weatherIcon;
    doc["lastUpdate"] = lastUpdateTime;
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    return jsonString;
}