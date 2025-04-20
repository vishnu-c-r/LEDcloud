#include "Protocol.h"
#include "Config.h"
#include <LittleFS.h>

// Initialize static member
Protocol *Protocol::instance = nullptr;

/**
 * @brief Private constructor initializes members with default values
 */
Protocol::Protocol()
    : wifiManager(nullptr),
      weatherService(nullptr),
      webServer(nullptr)
{

    // Allocate memory for LED state and brightness
    ledState = new bool(false);
    brightness = new int(DEFAULT_BRIGHTNESS);
}

/**
 * @brief Gets or creates the singleton instance of Protocol
 *
 * This ensures only one system manager exists in the application
 *
 * @return Pointer to the Protocol singleton instance
 */
Protocol *Protocol::getInstance()
{
    if (instance == nullptr)
    {
        instance = new Protocol();
    }
    return instance;
}

/**
 * @brief Initializes all system components
 *
 * Sets up the file system, WiFi, weather service, and web server
 *
 * @return true if all components initialized successfully
 */
bool Protocol::initializeSystem()
{
    Serial.println("Initializing system components...");

    // Initialize built-in LED for PWM control
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH); // LED off initially
    analogWriteRange(1023); // Set PWM range to 0-1023 (default for ESP8266)

    // Initialize file system
    if (!LittleFS.begin())
    {
        Serial.println("LittleFS mount failed!");
        return false;
    }
    Serial.println("LittleFS mounted successfully");

    // Initialize WiFi manager
    wifiManager = new CustomWiFiManager(DEVICE_HOSTNAME, new AsyncWebServer(WEB_SERVER_PORT));
    wifiManager->begin();
    Serial.println("WiFi manager initialized");

    // Initialize Weather service
    weatherService = Weather::getInstance();
    weatherService->beginWithSavedSettings();
    Serial.println("Weather service initialized");

    // Initialize Web server
    webServer = new WebServer(WEB_SERVER_PORT, ledState, brightness);
    webServer->setWeatherService(weatherService);
    webServer->begin();
    Serial.println("Web server initialized");

    return true;
}

/**
 * @brief Sets up all system tasks
 *
 * Registers weather updates, system monitoring, and other periodic tasks
 */
void Protocol::setupTasks()
{
    // Clear any existing tasks
    initializeTasks();

    // Create weather update task - use Config.h value and ensure it's not too frequent
    unsigned long weatherInterval = WEATHER_UPDATE_INTERVAL;
    // Ensure minimum 10 minute interval for weather updates to prevent API rate limiting
    if (weatherInterval < 600000)
    {
        Serial.println("WARNING: Weather update interval too short, increasing to 10 minutes minimum");
        weatherInterval = 600000; // 10 minutes in milliseconds
    }

    createTask("WeatherUpdate", [this]()
               { weatherUpdateTask(); }, weatherInterval);

    // Create system monitor task
    createTask("SystemMonitor", [this]()
               { systemMonitorTask(); }, HEAP_CHECK_INTERVAL);

    // Start all tasks including WiFi monitoring
    startAllTasks();

    Serial.println("All system tasks configured and started");
    Serial.printf("Weather updates scheduled every %lu minutes\n", weatherInterval / 60000);
}

/**
 * @brief Task function to update weather information
 */
void Protocol::weatherUpdateTask()
{
    if (weatherService)
    {
        weatherService->fetchWeatherData();
    }
}

/**
 * @brief Task function to monitor system resources
 */
void Protocol::systemMonitorTask()
{
    uint32_t freeHeap = ESP.getFreeHeap();
    uint32_t maxFreeBlockSize = ESP.getMaxFreeBlockSize();
    uint32_t heapFragmentation = ESP.getHeapFragmentation();
    uint8_t cpuFreqMHz = ESP.getCpuFreqMHz();
    float heapPercent = 100.0 * freeHeap / ESP.getFreeContStack();

    Serial.println("=== System Status ===");
    Serial.printf("Free heap: %u bytes (%.1f%%)\n", freeHeap, heapPercent);
    Serial.printf("Max free block: %u bytes\n", maxFreeBlockSize);
    Serial.printf("Heap fragmentation: %u%%\n", heapFragmentation);
    Serial.printf("CPU frequency: %u MHz\n", cpuFreqMHz);
    Serial.printf("LED state: %s\n", *ledState ? "ON" : "OFF");
    Serial.printf("Brightness: %d\n", *brightness);

    // WiFi information
    if (wifiManager && WiFi.status() == WL_CONNECTED)
    {
        Serial.printf("WiFi SSID: %s (RSSI: %d dBm)\n", WiFi.SSID().c_str(), WiFi.RSSI());
        Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());
    }

    // Weather information
    if (weatherService)
    {
        Serial.printf("Weather: %.1f°C, %s\n",
                      weatherService->getTemperature(),
                      weatherService->getDescription().c_str());
        Serial.printf("Weather API calls: %lu\n", 
                      Weather::getApiCallCount());
    }

    Serial.printf("Uptime: %lu seconds\n", millis() / 1000);
    Serial.println("====================");
}

/**
 * @brief Clears all tasks, preparing for new task registration
 */
void Protocol::initializeTasks()
{
    tasks.clear();
}

/**
 * @brief Starts the WiFi monitoring task and all registered application tasks
 *
 * Each task will run at its specified interval using the Ticker library
 */
void Protocol::startAllTasks()
{
    if (wifiManager)
    {
        wifiManager->startTask();
    }

    for (auto &task : tasks)
    {
        task.ticker.attach_ms(task.interval_ms, task.callback);
    }
}

/**
 * @brief Stops all running tasks and clears the task list
 *
 * This provides a clean shutdown of all background activities
 */
void Protocol::stopAllTasks()
{
    if (wifiManager)
    {
        wifiManager->stopTask();
    }

    for (auto &task : tasks)
    {
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
void Protocol::createTask(const char *name, std::function<void(void)> taskFunction,
                          uint32_t interval_ms)
{
    TaskConfig newTask = {
        .name = name,
        .ticker = Ticker(),
        .callback = taskFunction,
        .interval_ms = interval_ms};

    tasks.push_back(newTask);
    Serial.printf("Task '%s' created successfully\n", name);
}