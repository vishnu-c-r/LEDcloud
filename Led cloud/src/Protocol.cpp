#include "Protocol.h"

// Initialize static member
Protocol* Protocol::instance = nullptr;

/**
 * @brief Private constructor enforces singleton pattern
 * @param wifiMgr Reference to the WiFi manager instance
 */
Protocol::Protocol(CustomWiFiManager& wifiMgr) : wifiManager(wifiMgr) {}

/**
 * @brief Gets or creates the singleton instance of Protocol
 * 
 * This ensures only one task manager exists in the system
 * 
 * @param wifiMgr Reference to the WiFi manager to use
 * @return Pointer to the Protocol singleton instance
 */
Protocol* Protocol::getInstance(CustomWiFiManager& wifiMgr) {
    if (instance == nullptr) {
        instance = new Protocol(wifiMgr);
    }
    return instance;
}

/**
 * @brief Clears all tasks, preparing for new task registration
 */
void Protocol::initializeTasks() {
    tasks.clear();
}

/**
 * @brief Starts the WiFi monitoring task and all registered application tasks
 * 
 * Each task will run at its specified interval using the Ticker library
 */
void Protocol::startAllTasks() {
    wifiManager.startTask();
    for (auto& task : tasks) {
        task.ticker.attach_ms(task.interval_ms, task.callback);
    }
}

/**
 * @brief Stops all running tasks and clears the task list
 * 
 * This provides a clean shutdown of all background activities
 */
void Protocol::stopAllTasks() {
    wifiManager.stopTask();
    for (auto& task : tasks) {
        task.ticker.detach();
    }
    tasks.clear();
}

/**
 * @brief Creates and registers a new task in the system
 * 
 * @param name Friendly identifier for the task
 * @param taskFunction Function to execute when task runs 
 * @param interval_ms Time between task executions in milliseconds
 */
void Protocol::createTask(const char* name, std::function<void(void)> taskFunction, 
                         uint32_t interval_ms) {
    TaskConfig newTask = {
        .name = name,
        .ticker = Ticker(),
        .callback = taskFunction,
        .interval_ms = interval_ms
    };
    
    tasks.push_back(newTask);
    Serial.printf("Task '%s' created successfully\n", name);
}