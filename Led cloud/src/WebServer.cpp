#include "HttpConstants.h" // Include HTTP constants first
#include "WebServer.h"
#include "NeoPixel.h" // Include NeoPixel.h for NeoPixel class references

// Define the onboard LED pin for ESP8266
#define LED_BUILTIN_PIN LED_BUILTIN // Use the predefined LED_BUILTIN

/**
 * @brief Constructor initializes web server and stores LED state pointers
 * @param port Server port number
 * @param ledStatePtr Pointer to the LED state variable
 * @param brightnessPtr Pointer to the brightness variable
 */
WebServer::WebServer(uint16_t port, bool *ledStatePtr, int *brightnessPtr)
    : server(port), ledState(ledStatePtr), brightness(brightnessPtr), weatherService(nullptr)
{

    // Record start time for uptime calculations
    startTime = millis();

    // Note: LED pin is already initialized in main.cpp
}

/**
 * @brief Initialize the file system for serving web files
 * @return true if initialization successful, false otherwise
 */

bool WebServer::initFileSystem()
{
    // Try up to 3 times to mount the filesystem
    for (int i = 0; i < 3; i++)
    {
        if (LittleFS.begin())
        {
            Serial.println("LittleFS mounted successfully");
            return true;
        }
        delay(100);
    }

    Serial.println("An error occurred while mounting LittleFS");
    return false;
}

/**
 * @brief Handle 404 errors with a simple text response
 */
void WebServer::notFound(AsyncWebServerRequest *request)
{
    request->send(404, "text/plain", "Not found");
}

/**
 * @brief Initialize and start the web server
 */
void WebServer::begin()
{
    // Initialize file system first
    if (!initFileSystem())
    {
        Serial.println("Warning: Proceeding without file system");
    }

    // Set up all routes
    setupRoutes();

    // Start server
    server.begin();
    Serial.println("HTTP server started on port 80");
}

/**
 * @brief Update the physical LED based on current state and brightness
 * This function uses PWM to control the LED brightness
 */
void WebServer::updateLED()
{
    if (!ledState || !brightness)
    {
        Serial.println("[ERROR] LED state or brightness pointer is null!");
        return;
    }

    if (*ledState)
    {
        // ESP8266 onboard LED is inverted (LOW = ON)
        // Map brightness (0-255) to PWM range (0-1023) and invert for LED_BUILTIN
        // LED_BUILTIN is inverted (0 = full brightness, 1023 = off)
        int pwmValue = 1023 - map(*brightness, 0, 255, 0, 1023);
        analogWrite(LED_BUILTIN_PIN, pwmValue);
        Serial.printf("LED ON with brightness %d (PWM value: %d)\n", *brightness, pwmValue);
    }
    else
    {
        // LED is off (HIGH for ESP8266 built-in LED)
        digitalWrite(LED_BUILTIN_PIN, HIGH);
        Serial.println("LED OFF");
    }
}

/**
 * @brief Set up all routes for the web server
 */
void WebServer::setupRoutes()
{
    // Set up routes for different functionalities
    setupLEDRoutes();
    setupSystemRoutes();
    setupWeatherRoutes();
    setupNeoPixelRoutes();

    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/index.html", "text/html"); });

    // Route to serve static files (CSS, JS, images)
    server.serveStatic("/", LittleFS, "/");

    // 404 handler
    server.onNotFound(notFound);
}

/**
 * @brief Set up routes for LED control
 */
void WebServer::setupLEDRoutes()
{
    // Simple health check endpoint that doesn't require filesystem
    server.on("/health", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(200, "text/plain", "Server is running"); });

    // Route to set LED ON
    server.on("/led/on", HTTP_GET, [this](AsyncWebServerRequest *request)
              {
        if (ledState) {
            *ledState = true;
            Serial.println("LED ON");
            updateLED();
        } else {
            Serial.println("[ERROR] ledState pointer is null in /led/on");
        }
        request->send(200, "text/plain", "LED turned ON"); });

    // Route to set LED OFF
    server.on("/led/off", HTTP_GET, [this](AsyncWebServerRequest *request)
              {
        if (ledState) {
            *ledState = false;
            Serial.println("LED OFF");
            updateLED();
        } else {
            Serial.println("[ERROR] ledState pointer is null in /led/off");
        }
        request->send(200, "text/plain", "LED turned OFF"); });

    // Route to set brightness
    server.on("/brightness", HTTP_GET, [this](AsyncWebServerRequest *request)
              {
        if (brightness) {
            if (request->hasParam("value")) {
                *brightness = request->getParam("value")->value().toInt();
                Serial.print("Brightness set to: ");
                Serial.println(*brightness);
                
                // Apply the brightness if the LED is on
                if (ledState && *ledState) {
                    updateLED();
                }
            }
        } else {
            Serial.println("[ERROR] brightness pointer is null in /brightness");
        }
        request->send(200, "text/plain", "Brightness updated"); });
}

/**
 * @brief Set up routes for system information
 */
void WebServer::setupSystemRoutes()
{
    // Improved system info endpoint with heap fragmentation and WiFi signal
    server.on("/system-info", HTTP_GET, [this](AsyncWebServerRequest *request)
              {
        String json = "{";
        json += "\"freeHeap\":" + String(ESP.getFreeHeap()) + ",";
        json += "\"heapFragmentation\":" + String(ESP.getHeapFragmentation()) + ",";
        json += "\"wifiSignal\":" + String(WiFi.RSSI()) + ",";
        json += "\"ledState\":" + String(ledState ? (*ledState) : 0) + ",";
        json += "\"brightness\":" + String(brightness ? (*brightness) : 0) + ",";
        json += "\"uptime\":" + String((millis() - startTime) / 1000);
        json += "}";
        
        request->send(200, "application/json", json); });
}

/**
 * @brief Set up routes for weather data
 */
void WebServer::setupWeatherRoutes()
{
    // Weather data endpoint
    server.on("/weather", HTTP_GET, [this](AsyncWebServerRequest *request)
              {
        if (weatherService) {
            String weatherJson = weatherService->toJson();
            request->send(200, "application/json", weatherJson);
        } else {
            request->send(503, "application/json", "{\"error\":\"Weather service not available\"}");
        } });

    // Get weather settings endpoint
    server.on("/weather-settings", HTTP_GET, [this](AsyncWebServerRequest *request)
              {
        if (weatherService) {
            String settingsJson = weatherService->getSettingsJson();
            request->send(200, "application/json", settingsJson);
        } else {
            request->send(503, "application/json", "{\"error\":\"Weather service not available\"}");
        } });

    // Save weather settings endpoint
    server.on("/weather-settings", HTTP_POST, [](AsyncWebServerRequest *request)
              {
                  // POST request will be handled in the body handler
              },
              NULL, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
              {
        if (!weatherService) {
            request->send(503, "application/json", "{\"status\":\"error\",\"message\":\"Weather service not available\"}");
            return;
        }
        
        // Parse JSON data
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, data, len);
        
        if (error) {
            request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid JSON\"}");
            return;
        }
        
        // Extract values
        String apiKey = doc["apiKey"].as<String>();
        float latitude = doc["latitude"].as<float>();
        float longitude = doc["longitude"].as<float>();
        
        // Update settings
        bool success = weatherService->updateSettings(apiKey, latitude, longitude);
        
        if (success) {
            request->send(200, "application/json", "{\"status\":\"success\",\"message\":\"Settings updated\"}");
        } else {
            request->send(500, "application/json", "{\"status\":\"error\",\"message\":\"Failed to save settings\"}");
        } });
}

/**
 * @brief Set up routes for NeoPixel control
 */
void WebServer::setupNeoPixelRoutes() {
    // Set all LEDs to a color (POST: {"r":int, "g":int, "b":int})
    server.on("/neopixel/setAll", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL,
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, data, len);
            if (error) {
                request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
                return;
            }
            int r = doc["r"] | 0;
            int g = doc["g"] | 0;
            int b = doc["b"] | 0;
            NeoPixel::getInstance()->setAllPixels(NeoPixel::getInstance()->rgbToColor(r, g, b));
            NeoPixel::getInstance()->show();
            request->send(200, "application/json", "{\"status\":\"ok\"}");
        }
    );

    // Set a specific LED's color (POST: {"index":int, "r":int, "g":int, "b":int})
    server.on("/neopixel/setPixel", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL,
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, data, len);
            if (error) {
                request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
                return;
            }
            int idx = doc["index"] | 0;
            int r = doc["r"] | 0;
            int g = doc["g"] | 0;
            int b = doc["b"] | 0;
            NeoPixel::getInstance()->updatePixelColor(idx, r, g, b);
            request->send(200, "application/json", "{\"status\":\"ok\"}");
        }
    );

    // Set pattern (POST: {"pattern":int})
    server.on("/neopixel/setPattern", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL,
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, data, len);
            if (error) {
                request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
                return;
            }
            int pattern = doc["pattern"] | 0;
            NeoPixel::getInstance()->setPattern(static_cast<PatternType>(pattern));
            request->send(200, "application/json", "{\"status\":\"ok\"}");
        }
    );

    // Set brightness (POST: {"brightness":int})
    server.on("/neopixel/setBrightness", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL,
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, data, len);
            if (error) {
                request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
                return;
            }
            int brightness = doc["brightness"] | 0;
            NeoPixel::getInstance()->setBrightness(brightness);
            request->send(200, "application/json", "{\"status\":\"ok\"}");
        }
    );

    // Get NeoPixel status (GET)
    server.on("/neopixel/status", HTTP_GET, [](AsyncWebServerRequest *request) {
        String status = NeoPixel::getInstance()->getStatusJson();
        request->send(200, "application/json", status);
    });
}

/**
 * @brief Set the weather service instance
 * @param weather Pointer to the Weather instance
 */
void WebServer::setWeatherService(Weather *weather)
{
    this->weatherService = weather;
}