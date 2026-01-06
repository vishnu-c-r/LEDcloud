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
        // We don't use setupTasks anymore, the update loop handles it.
        Serial.println("System initialization complete.");
    } else {
        Serial.println("System initialization failed!");
    }
}

void loop() {
    // Run the main system update loop
    if (protocol) {
        protocol->update();
    }

    // Allow for background WiFi/System tasks
    delay(1);
}