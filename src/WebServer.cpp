#include "HttpConstants.h"  // Include HTTP constants first
#include "WebServer.h"

// Define the onboard LED pin for ESP8266
#define LED_BUILTIN_PIN LED_BUILTIN  // Use the predefined LED_BUILTIN

/**
 * @brief Constructor initializes web server and stores LED state pointers
 * @param port Server port number
 * @param ledStatePtr Pointer to the LED state variable
 * @param brightnessPtr Pointer to the brightness variable
 */
WebServer::WebServer(uint16_t port, bool* ledStatePtr, int* brightnessPtr)
    : server(port), ledState(ledStatePtr), brightness(brightnessPtr) {
    
    // Record start time for uptime calculations
    startTime = millis();
    
    // Note: LED pin is already initialized in main.cpp
}

/**
 * @brief Initialize the file system for serving web files
 * @return true if initialization successful, false otherwise
 */
bool WebServer::initFileSystem() {
    // Try up to 3 times to mount the filesystem
    for (int i = 0; i < 3; i++) {
        if (LittleFS.begin()) {
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
void WebServer::notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

/**
 * @brief Initialize and start the web server
 */
void WebServer::begin() {
    // Initialize file system first
    if (!initFileSystem()) {
        Serial.println("Warning: Proceeding without file system");
    }
    
    // Set up all routes
    setupRoutes();
    
    // Start server
    server.begin();
    Serial.println("HTTP server started on port 80");
}

/**
 * @brief Update the physical LED based on current state
 * This function uses safer methods to control the LED
 */
void WebServer::updateLED() {
    if (!ledState) {
        Serial.println("[ERROR] ledState pointer is null!");
        return;
    }
    if (*ledState) {
        // ESP8266 onboard LED is inverted (LOW = ON)
        digitalWrite(LED_BUILTIN_PIN, LOW);
    } else {
        // LED is off
        digitalWrite(LED_BUILTIN_PIN, HIGH);
    }
}

/**
 * @brief Set up all routes for the web server
 */
void WebServer::setupRoutes() {
    // Simple health check endpoint that doesn't require filesystem
    server.on("/health", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "Server is running");
    });

    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/index.html", "text/html");
    });

    // Route to serve static files (CSS, JS, images)
    server.serveStatic("/", LittleFS, "/");

    // Route to set LED ON
    server.on("/led/on", HTTP_GET, [this](AsyncWebServerRequest *request) {
        if (ledState) {
            *ledState = true;
            Serial.println("LED ON");
            updateLED();
        } else {
            Serial.println("[ERROR] ledState pointer is null in /led/on");
        }
        request->send(200, "text/plain", "LED turned ON");
    });

    // Route to set LED OFF
    server.on("/led/off", HTTP_GET, [this](AsyncWebServerRequest *request) {
        if (ledState) {
            *ledState = false;
            Serial.println("LED OFF");
            updateLED();
        } else {
            Serial.println("[ERROR] ledState pointer is null in /led/off");
        }
        request->send(200, "text/plain", "LED turned OFF");
    });

    // Route to set brightness (doesn't affect LED yet, just stores the value)
    server.on("/brightness", HTTP_GET, [this](AsyncWebServerRequest *request) {
        if (brightness) {
            if (request->hasParam("value")) {
                *brightness = request->getParam("value")->value().toInt();
                Serial.print("Brightness set to: ");
                Serial.println(*brightness);
                
                // Note: Not using brightness for LED control yet
                // Will be implemented once basic functionality is stable
            }
        } else {
            Serial.println("[ERROR] brightness pointer is null in /brightness");
        }
        request->send(200, "text/plain", "Brightness updated");
    });

    // Simplified system info endpoint
    server.on("/system-info", HTTP_GET, [this](AsyncWebServerRequest *request) {
        String json = "{";
        json += "\"freeHeap\":" + String(ESP.getFreeHeap()) + ",";
        json += "\"ledState\":" + String(ledState ? (*ledState) : 0) + ",";
        json += "\"brightness\":" + String(brightness ? (*brightness) : 0) + ",";
        json += "\"uptime\":" + String((millis() - startTime) / 1000);
        json += "}";
        
        request->send(200, "application/json", json);
    });

    // 404 handler
    server.onNotFound(notFound);
}