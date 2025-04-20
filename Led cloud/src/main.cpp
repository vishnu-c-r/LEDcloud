#include <Arduino.h>
#include "Protocol.h"

// Single global instance of the Protocol class
Protocol* protocol = nullptr;

void setup() {
    // Initialize serial communication
    Serial.begin(115200);
    Serial.println("Booting LEDcloud...");
    
    // Get Protocol instance and initialize the system
    protocol = Protocol::getInstance();
    bool initSuccess = protocol->initializeSystem();
    
    if (initSuccess) {
        // Set up and start all system tasks
        protocol->setupTasks();
        Serial.println("System initialization complete.");
    } else {
        Serial.println("System initialization failed!");
    }
}

void loop() {
    // Keep the loop minimal - all functionality is handled by tasks/callbacks
    delay(10);
}