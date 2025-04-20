# LED Cloud - IoT Dashboard

## Overview
LED Cloud is an ESP8266-based IoT project that provides a web dashboard for controlling LED lights and displaying weather information. The project uses a responsive web interface that can be accessed from any device on the same network.

## Features

### LED Control
- Toggle LED on/off remotely
- Adjust LED brightness with a slider (0-255)
- Visual indicator of LED status
- Real-time feedback

### Weather Display
- Shows current temperature in Celsius
- Displays weather description (e.g., "Clear sky", "Light rain")
- Shows humidity percentage
- Weather icon visualization
- Last update timestamp
- API call counter to monitor usage
- Manual weather refresh option

### System Information
- Free heap memory monitoring
- Heap fragmentation percentage
- WiFi signal strength indicator
- System uptime tracker
- Refresh button for system stats

### Configuration
- Customizable weather settings (API key, location)
- Dark/light theme toggle with persistent storage
- Mobile-responsive design

## Technical Implementation

### Hardware Requirements
- ESP8266-based development board
- LED connected to a PWM-capable pin
- Power supply

### Software Components
- **CustomWiFiManager**: Handles WiFi connection and configuration
- **Weather**: Manages OpenWeatherMap API communication
- **WebServer**: Provides the web interface and API endpoints
- **Protocol**: Handles LED control functionality

### Technologies Used
- ESP8266 Arduino Core
- LittleFS for file storage
- ArduinoJSON for parsing API responses
- HTML/CSS/JavaScript for the frontend
- Ticker library for scheduling tasks

## Setup Instructions

1. Clone the repository
2. Configure your `Config.h` with your WiFi credentials and OpenWeatherMap API key
3. Upload the firmware using PlatformIO
4. Upload the web interface files to the ESP8266's filesystem
5. Access the dashboard by connecting to the ESP8266's IP address

## Future Enhancements
- Add support for multiple LEDs or RGB strips
- Implement scheduled lighting patterns
- Add additional sensors (temperature, motion, etc.)
- Create user profiles with preferences

## API Endpoints

- `/led/on` - Turn LED on
- `/led/off` - Turn LED off
- `/brightness?value=X` - Set LED brightness (0-255)
- `/weather` - Get current weather data
- `/weather/update` - Force weather update
- `/weather/settings` - Get or update weather settings
- `/system/info` - Get system information

## Notes
- Weather API is rate-limited to avoid exceeding the free tier limits
- Settings are stored in LittleFS and persist through reboots
- The interface uses client-side storage for theme preferences
