#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <Arduino.h>
#include <vector>
#include <functional>
#include <Ticker.h>
#include "CustomWiFiManager.h"

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
 * @brief Task scheduler and manager that follows the Singleton pattern
 * 
 * This class manages all scheduled tasks in the system and coordinates
 * their execution. It ensures there's only one instance of the task manager.
 */
class Protocol {
private:
    static Protocol* instance;         // Singleton instance pointer
    std::vector<TaskConfig> tasks;     // Container for all registered tasks
    CustomWiFiManager& wifiManager;    // Reference to WiFi manager

    /**
     * @brief Private constructor to enforce Singleton pattern
     * @param wifiMgr Reference to the WiFi manager instance
     */
    Protocol(CustomWiFiManager& wifiMgr);

public:
    /**
     * @brief Gets or creates the singleton instance
     * @param wifiMgr Reference to the WiFi manager
     * @return Pointer to the Protocol instance
     */
    static Protocol* getInstance(CustomWiFiManager& wifiMgr);
    
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
     * @brief Gets the WiFi manager instance
     * @return Reference to the CustomWiFiManager
     */
    CustomWiFiManager& getWiFiManager() { return wifiManager; }
};

#endif // PROTOCOL_H