#ifndef CONFIG_H
#define CONFIG_H

// *** DEFAULT CONFIGURATION VALUES ***
// These values are used only as defaults when no saved settings exist
// Most settings can be modified through the web interface

// Device Configuration
#define DEVICE_HOSTNAME "cloudled"  // Device name on the network
#define DEVICE_PASSWORD ""          // Optional WiFi AP password when in setup mode

// LED Configuration
#define DEFAULT_BRIGHTNESS 100      // Default LED brightness (0-255)
#define LED_PIN LED_BUILTIN         // Default LED pin

// Server Configuration
#define WEB_SERVER_PORT 80          // Web server port

// Weather Configuration
#define WEATHER_API_KEY "7f29f73f56a4c0e81cbdd4900b8886bb"  // OpenWeatherMap API key
#define LATITUDE 9.953397           // Default latitude 
#define LONGITUDE 76.353383         // Default longitude 
#define WEATHER_UPDATE_INTERVAL 1200000  // Weather update frequency (20 minutes in ms)

// System Configuration
#define WIFI_CHECK_INTERVAL 5000    // WiFi connection check interval (5 seconds)
#define HEAP_CHECK_INTERVAL 30000   // Memory check interval (30 seconds)

#endif // CONFIG_H