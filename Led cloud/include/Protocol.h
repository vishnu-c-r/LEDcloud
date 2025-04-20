#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <Arduino.h>
#include <vector>
#include <functional>
#include <Ticker.h>
#include "CustomWiFiManager.h"
#include "Weather.h"
#include "WebServer.h"
#include "Config.h"
#include "NeoPixel.h"

/**
 * @struct TaskConfig
 * @brief Configuration structure for scheduled tasks
 * 
 * Defines all properties needed to manage periodic tasks including
 * name, timing, and the function to execute
 */
struct TaskConfig {
    const char* name;                  // Friendly name for the task
    Ticker ticker;                     // Timer that schedules task execution
    std::function<void(void)> callback; // Function to call when task runs
    uint32_t interval_ms;              // Interval between executions in milliseconds
};

/**
 * @class Protocol
 * @brief Task scheduler and system manager that follows the Singleton pattern
 * 
 * This class manages all system components and scheduled tasks, coordinating
 * their execution. It ensures there's only one instance of the system manager.
 */
class Protocol {
private:
    static Protocol* instance;         // Singleton instance pointer
    std::vector<TaskConfig> tasks;     // Container for all registered tasks
    
    // System components
    CustomWiFiManager* wifiManager;    // WiFi manager 
    Weather* weatherService;           // Weather service
    WebServer* webServer;              // Web server
    NeoPixel* neoPixel;                // NeoPixel manager
    
    // System state
    bool* ledState;                    // LED state reference
    int* brightness;                   // LED brightness reference
    
    /**
     * @brief Private constructor to enforce Singleton pattern
     */
    Protocol();

public:
    /**
     * @brief Gets or creates the singleton instance
     * @return Pointer to the Protocol instance
     */
    static Protocol* getInstance();
    
    /**
     * @brief Initializes all system components
     * @return true if all components initialized successfully
     */
    bool initializeSystem();
    
    /**
     * @brief Sets up all system tasks
     */
    void setupTasks();
    
    /**
     * @brief Clears all tasks, preparing for new configuration
     */
    void initializeTasks();
    
    /**
     * @brief Activates all registered tasks
     */
    void startAllTasks();
    
    /**
     * @brief Stops all running tasks and clears the task list
     */
    void stopAllTasks();
    
    /**
     * @brief Creates and registers a new task
     * @param name Friendly identifier for the task
     * @param taskFunction Function to execute when the task runs
     * @param interval_ms Time between executions in milliseconds
     */
    void createTask(const char* name, std::function<void(void)> taskFunction, 
                   uint32_t interval_ms);
                   
    /**
     * @brief Task function to update weather information
     */
    void weatherUpdateTask();
    
    /**
     * @brief Task function to monitor system resources
     */
    void systemMonitorTask();
    
    /**
     * @brief Task function to update NeoPixel patterns
     */
    void neoPixelTask();
    
    /**
     * @brief Gets the WiFi manager instance
     * @return Pointer to the CustomWiFiManager
     */
    CustomWiFiManager* getWiFiManager() { return wifiManager; }
    
    /**
     * @brief Gets the WebServer instance
     * @return Pointer to the WebServer
     */
    WebServer* getWebServer() { return webServer; }
    
    /**
     * @brief Gets the Weather service instance
     * @return Pointer to the Weather service
     */
    Weather* getWeatherService() { return weatherService; }
};

#endif // PROTOCOL_H