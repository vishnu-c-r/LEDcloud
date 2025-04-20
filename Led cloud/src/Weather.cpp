#include "Weather.h"
#include "Config.h"

// Initialize static instance pointer
Weather* Weather::instance = nullptr;

// Define the settings file path
const char* Weather::SETTINGS_FILE = "/weather_settings.json";

// Rate limiting - track the last API call time
unsigned long Weather::lastApiCallTime = 0;
const unsigned long Weather::MIN_API_CALL_INTERVAL = 600000; // 10 minutes minimum between API calls

// Initialize API call counter
unsigned long Weather::apiCallCount = 0;

Weather::Weather() 
    : temperature(0),
      humidity(0),  // Initialize humidity
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

void Weather::begin(const String& apiKey, float latitude, float longitude) {
    this->apiKey = apiKey;
    this->latitude = latitude;
    this->longitude = longitude;
    
    Serial.println("Weather service initialized");
    
    // Fetch initial weather data
    fetchWeatherData();
}

void Weather::beginWithDefaults() {
    // Use values from Config.h
    this->apiKey = WEATHER_API_KEY;
    this->latitude = LATITUDE;
    this->longitude = LONGITUDE;
    
    Serial.println("Weather service initialized with defaults from Config.h");
    Serial.print("Location: ");
    Serial.print(latitude, 6);
    Serial.print(", ");
    Serial.println(longitude, 6);
    
    // Fetch initial weather data if API key is not the placeholder
    if (apiKey != "YOUR_API_KEY_HERE") {
        fetchWeatherData();
    } else {
        Serial.println("Weather updates disabled: No API key configured");
    }
}

bool Weather::beginWithSavedSettings() {
    String savedApiKey;
    float savedLatitude = 0.0;
    float savedLongitude = 0.0;
    
    bool settingsLoaded = loadSettings(savedApiKey, savedLatitude, savedLongitude);
    
    if (settingsLoaded && savedApiKey.length() > 0) {
        // Use saved settings
        this->apiKey = savedApiKey;
        this->latitude = savedLatitude;
        this->longitude = savedLongitude;
        
        Serial.println("Weather service initialized with saved settings");
        Serial.print("Location: ");
        Serial.print(latitude, 6);
        Serial.print(", ");
        Serial.println(longitude, 6);
        
        // Fetch initial weather data
        fetchWeatherData();
        return true;
    } else {
        // Fall back to defaults
        beginWithDefaults();
        return false;
    }
}

bool Weather::saveSettings(const String& apiKey, float latitude, float longitude) {
    // Create a JSON document
    JsonDocument doc;
    
    doc["apiKey"] = apiKey;
    doc["latitude"] = latitude;
    doc["longitude"] = longitude;
    
    // Open file for writing
    File file = LittleFS.open(SETTINGS_FILE, "w");
    if (!file) {
        Serial.println("Failed to open weather settings file for writing");
        return false;
    }
    
    // Serialize JSON to file
    if (serializeJson(doc, file) == 0) {
        Serial.println("Failed to write weather settings to file");
        file.close();
        return false;
    }
    
    file.close();
    Serial.println("Weather settings saved successfully");
    return true;
}

bool Weather::loadSettings(String& apiKey, float& latitude, float& longitude) {
    // Check if file exists
    if (!LittleFS.exists(SETTINGS_FILE)) {
        Serial.println("Weather settings file not found");
        return false;
    }
    
    // Open file for reading
    File file = LittleFS.open(SETTINGS_FILE, "r");
    if (!file) {
        Serial.println("Failed to open weather settings file for reading");
        return false;
    }
    
    // Parse JSON from file
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    
    if (error) {
        Serial.print("Failed to parse weather settings: ");
        Serial.println(error.c_str());
        return false;
    }
    
    // Extract values
    apiKey = doc["apiKey"].as<String>();
    latitude = doc["latitude"].as<float>();
    longitude = doc["longitude"].as<float>();
    
    Serial.println("Weather settings loaded successfully");
    return true;
}

bool Weather::updateSettings(const String& apiKey, float latitude, float longitude) {
    // Save new settings
    if (!saveSettings(apiKey, latitude, longitude)) {
        return false;
    }
    
    // Update current settings
    this->apiKey = apiKey;
    this->latitude = latitude;
    this->longitude = longitude;
    
    // Fetch weather with new settings
    updateNow();
    
    return true;
}

String Weather::getSettingsJson() const {
    // Create a JSON document
    JsonDocument doc;
    
    // Mask API key for security (show only last 4 characters)
    String maskedApiKey = apiKey;
    if (maskedApiKey.length() > 4) {
        for (size_t i = 0; i < maskedApiKey.length() - 4; i++) {
            maskedApiKey[i] = '*';
        }
    }
    
    doc["apiKey"] = maskedApiKey;
    doc["latitude"] = latitude;
    doc["longitude"] = longitude;
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    return jsonString;
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

void Weather::updateNow() {
    // Force an immediate weather update regardless of the schedule
    fetchWeatherData();
    Serial.println("Manual weather update triggered");
}

void Weather::fetchWeatherData() {
    // Only fetch if WiFi is connected
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected. Skipping weather update.");
        return;
    }
    
    // Check if API key is set
    if (apiKey.length() == 0 || apiKey == "YOUR_API_KEY_HERE") {
        Serial.println("API key not set. Skipping weather update.");
        return;
    }
    
    // Rate limiting - check if enough time has passed since the last API call
    unsigned long currentTime = millis();
    if (lastApiCallTime > 0 && (currentTime - lastApiCallTime < MIN_API_CALL_INTERVAL)) {
        Serial.println("API call rate limited. Too frequent calls to OpenWeatherMap API.");
        Serial.printf("Next API call allowed in %lu seconds\n", 
                     (MIN_API_CALL_INTERVAL - (currentTime - lastApiCallTime)) / 1000);
        return;
    }
    
    // HTTP client and WiFi client
    WiFiClient client;
    HTTPClient http;
    
    // Build the URL for OpenWeatherMap Current Weather API (free tier compatible)
    String url = "http://api.openweathermap.org/data/2.5/weather?";
    
    // Add latitude and longitude
    url += "lat=" + String(latitude, 6); // 6 decimal places for accuracy
    url += "&lon=" + String(longitude, 6);
    
    // Add API key
    url += "&appid=" + apiKey;
    
    // Add units parameter (metric for Celsius)
    url += "&units=metric";
    
    Serial.println("Fetching weather data from: " + url);
    
    // Start the HTTP request
    http.begin(client, url);
    
    // Update last API call time before making the call
    lastApiCallTime = currentTime;
    
    // Increment API call counter
    apiCallCount++;
    Serial.printf("Making API call #%lu to OpenWeatherMap\n", apiCallCount);
    
    // Send HTTP GET request
    int httpResponseCode = http.GET();
    
    if (httpResponseCode > 0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        
        String payload = http.getString();
        Serial.println("Response: " + payload);
        
        // Use Arduino JSON to parse response
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);
        
        if (error) {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.c_str());
        } else {
            // Extract current weather data from JSON
            temperature = doc["main"]["temp"];
            
            // Extract humidity from the response
            if (!doc["main"]["humidity"].isNull()) {
                humidity = doc["main"]["humidity"].as<int>();
                Serial.print("Humidity: ");
                Serial.print(humidity);
                Serial.println("%");
            }
            
            // Weather description
            if (doc["weather"][0]["description"].isNull() == false) {
                weatherDescription = doc["weather"][0]["description"].as<String>();
                // Capitalize first letter of description
                weatherDescription.setCharAt(0, toupper(weatherDescription.charAt(0)));
            }
            
            // Weather icon code
            if (doc["weather"][0]["icon"].isNull() == false) {
                weatherIcon = doc["weather"][0]["icon"].as<String>();
            }
            
            Serial.println("Weather data updated successfully");
            Serial.print("Temperature: ");
            Serial.print(temperature);
            Serial.println("°C");
            Serial.print("Description: ");
            Serial.println(weatherDescription);
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

String Weather::getLastUpdateTime() const {
    // Calculate minutes since the last update
    unsigned long currentTime = millis();
    unsigned long timeDiff = (currentTime - lastUpdateTime) / 60000; // convert to minutes
    
    if (timeDiff < 60) {
        return String(timeDiff) + " min ago";
    } else if (timeDiff < 1440) { // less than 24 hours
        return String(timeDiff / 60) + " hours ago";
    } else {
        return String(timeDiff / 1440) + " days ago";
    }
}

String Weather::toJson() const {
    // Create a JSON document
    JsonDocument doc;
    
    doc["temperature"] = temperature;
    doc["humidity"] = humidity;  // Include humidity in the JSON response
    doc["description"] = weatherDescription;
    doc["icon"] = weatherIcon;
    doc["lastUpdate"] = getLastUpdateTime();
    doc["apiCallCount"] = apiCallCount;
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    return jsonString;
}