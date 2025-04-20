#include <Arduino.h>
#include <LittleFS.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "CustomWiFiManager.h"
#include "Protocol.h"
#include "WebServer.h"

AsyncWebServer server(80);
CustomWiFiManager* wifiManager = nullptr;
Protocol* protocol = nullptr;
WebServer* webServer = nullptr;

bool ledState = false;
int brightness = 100; // 0-255 range for PWM

void setup() {
    Serial.begin(115200);
    Serial.println("Booting with AsyncWiFiManager...");
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);

    if (!LittleFS.begin()) {
        Serial.println("LittleFS mount failed!");
    } else {
        Serial.println("LittleFS mounted.");
    }

    // Initialize WiFi manager with hostname "cloudled"
    wifiManager = new CustomWiFiManager("cloudled", &server);
    Serial.println("CustomWiFiManager initialized.");
    wifiManager->begin();
    wifiManager->startTask(); // Start the WiFi checking task
    
    // Setup client IP logging
    wifiManager->setupClientIPLogging(&server);

    // Instantiate Protocol after WiFi is initialized
    protocol = Protocol::getInstance(*wifiManager);
    Serial.println("Protocol instance created.");

    // Instantiate WebServer after Protocol
    webServer = new WebServer(80, &ledState, &brightness);
    webServer->begin();
    Serial.println("WebServer instance created and started.");

    // Basic homepage route
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "Hello, world! (AsyncWiFiManager + Protocol + WebServer)");
    });
    server.begin();
    Serial.println("Web server started.");
}

void loop() {
    // Keep the loop minimal - most functionality is handled by tasks/callbacks
    delay(10);
}