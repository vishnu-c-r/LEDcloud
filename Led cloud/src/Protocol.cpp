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
      webServer(nullptr),
      neoPixel(nullptr),
      lastWeatherUpdate(0),
      lastSystemMonitor(0),
      lastNeoPixelUpdate(0)
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

    // Initialize NeoPixel
    neoPixel = NeoPixel::getInstance();
    neoPixel->begin();
    Serial.println("NeoPixel initialized");

    // Update LED based on initial weather (if available)
    if (weatherService->getWeatherId() != 0) {
        updateLedBasedOnWeather(weatherService->getWeatherId());
    }

    return true;
}

/**
 * @brief Main loop update function
 */
void Protocol::update()
{
    unsigned long currentMillis = millis();

    // WiFi Manager Task (keep connectivity alive)
    // Note: CustomWiFiManager uses its own Ticker internally in startTask,
    // but we can also poll it if needed. For now we assume it handles itself
    // or we might need to verify if it uses Ticker correctly.
    // Assuming CustomWiFiManager is safe or handled elsewhere.

    // Weather Update Task
    if (currentMillis - lastWeatherUpdate >= WEATHER_UPDATE_INTERVAL) {
        lastWeatherUpdate = currentMillis;
        weatherUpdateTask();
    }

    // System Monitor Task
    if (currentMillis - lastSystemMonitor >= HEAP_CHECK_INTERVAL) {
        lastSystemMonitor = currentMillis;
        systemMonitorTask();
    }

    // NeoPixel Update Task (Animation)
    if (currentMillis - lastNeoPixelUpdate >= 50) { // 20fps approx
        lastNeoPixelUpdate = currentMillis;
        neoPixelTask();
    }
}


/**
 * @brief Task function to update weather information
 */
void Protocol::weatherUpdateTask()
{
    if (weatherService)
    {
        weatherService->fetchWeatherData();
        int weatherId = weatherService->getWeatherId();
        if (weatherId != 0) {
            updateLedBasedOnWeather(weatherId);
        }
    }
}

void Protocol::updateLedBasedOnWeather(int weatherId) {
    if (!neoPixel) return;

    // OpenWeatherMap Condition Codes: https://openweathermap.org/weather-conditions
    PatternType newPattern = PATTERN_OFF;

    if (weatherId >= 200 && weatherId < 300) {
        // Thunderstorm
        newPattern = PATTERN_CHASE; // Or maybe a custom lightning effect
    } else if (weatherId >= 300 && weatherId < 400) {
        // Drizzle
        newPattern = PATTERN_RAIN;
    } else if (weatherId >= 500 && weatherId < 600) {
        // Rain
        newPattern = PATTERN_RAIN;
    } else if (weatherId >= 600 && weatherId < 700) {
        // Snow
        newPattern = PATTERN_TWINKLE; // White sparkles
    } else if (weatherId >= 700 && weatherId < 800) {
        // Atmosphere (Mist, Smoke, Haze, Dust, Fog, Sand, Dust, Ash, Squall, Tornado)
        newPattern = PATTERN_FADE; // Eerie fade
    } else if (weatherId == 800) {
        // Clear
        newPattern = PATTERN_FIRE; // Warm sun
    } else if (weatherId > 800 && weatherId < 900) {
        // Clouds
        newPattern = PATTERN_FADE; // Gentle clouds
    } else {
        newPattern = PATTERN_RAINBOW; // Default
    }

    Serial.printf("Weather ID: %d -> Setting Pattern: %d\n", weatherId, newPattern);
    neoPixel->setPattern(newPattern);
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
        Serial.printf("Weather ID: %d\n", weatherService->getWeatherId());
        Serial.printf("Weather API calls: %lu\n", 
                      Weather::getApiCallCount());
    }

    Serial.printf("Uptime: %lu seconds\n", millis() / 1000);
    Serial.println("====================");
}

/**
 * @brief Task function to update NeoPixel pattern
 */
void Protocol::neoPixelTask()
{
    if (neoPixel) {
        // Update the current animation pattern
        neoPixel->update();
        
        // Only call show if we're in an active animation mode
        if (neoPixel->isAnimationActive()) {
            neoPixel->show();
        }
    }
}