#pragma once
#include <Adafruit_NeoPixel.h>
#include <Arduino.h>

// Define your pattern types here
enum PatternType {
    PATTERN_OFF = 0,
    PATTERN_RED = 1,
    PATTERN_RAINBOW = 2,
    PATTERN_CHASE = 3,    // New animation: chase effect
    PATTERN_FADE = 4,     // New animation: fade in/out
    PATTERN_TWINKLE = 5,  // New animation: random twinkling
    PATTERN_FIRE = 6,     // Fire effect simulation
    PATTERN_RAIN = 7,     // Blue rain effect
    PATTERN_COLOR_WIPE = 8 // Color wipe animation
};

class NeoPixel {
public:
    static NeoPixel* getInstance();
    void begin();
    void setAllPixels(uint32_t color);
    void updatePixelColor(int idx, int r, int g, int b);
    void setBrightness(int b);
    void setPattern(PatternType pattern);
    void show();
    void update();    // Method to update animations
    bool isAnimationActive(); // Method to check if an animation is currently running
    uint32_t rgbToColor(int r, int g, int b);
    String getStatusJson();

private:
    NeoPixel();
    static NeoPixel* instance;
    Adafruit_NeoPixel strip;
    int brightness;
    PatternType currentPattern;
    uint32_t pixelColors[60]; // Updated to support 60 LEDs
    unsigned long lastUpdate;  // Timestamp of last animation update
    
    // Animation update methods
    void updateChasePattern();
    void updateFadePattern();
    void updateTwinklePattern();
    void updateFirePattern();
    void updateRainPattern();
    void updateColorWipePattern();
};
