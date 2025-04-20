#include "NeoPixel.h"
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>

#define NEOPIXEL_PIN  D5
#define NUM_PIXELS    60

NeoPixel* NeoPixel::instance = nullptr;

NeoPixel::NeoPixel()
    : strip(NUM_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800), brightness(50), currentPattern(PATTERN_OFF), lastUpdate(0) {
}

NeoPixel* NeoPixel::getInstance() {
    if (!instance) {
        instance = new NeoPixel();
        instance->begin();
    }
    return instance;
}

void NeoPixel::begin() {
    strip.begin();
    strip.setBrightness(brightness);
    strip.show();
    for (int i = 0; i < NUM_PIXELS; ++i) {
        pixelColors[i] = strip.Color(0, 0, 0);
    }
    Serial.println("NeoPixel initialized with " + String(NUM_PIXELS) + " LEDs on pin " + String(NEOPIXEL_PIN));
}

void NeoPixel::setAllPixels(uint32_t color) {
    Serial.println("Setting all pixels to color: R=" + String((color >> 16) & 0xFF) + 
                   ", G=" + String((color >> 8) & 0xFF) + 
                   ", B=" + String(color & 0xFF));
    
    // Disable interrupts during pixel updates to prevent crashes
    noInterrupts();
    for (int i = 0; i < NUM_PIXELS; ++i) {
        pixelColors[i] = color;
        strip.setPixelColor(i, color);
    }
    interrupts();
    
    strip.show();
}

void NeoPixel::updatePixelColor(int idx, int r, int g, int b) {
    if (idx < 0 || idx >= NUM_PIXELS) {
        Serial.println("ERROR: Invalid pixel index: " + String(idx));
        return;
    }
    
    Serial.println("Setting pixel " + String(idx) + " to R=" + String(r) + ", G=" + String(g) + ", B=" + String(b));
    
    // Disable interrupts during pixel updates to prevent crashes
    noInterrupts();
    uint32_t color = strip.Color(r, g, b);
    pixelColors[idx] = color;
    strip.setPixelColor(idx, color);
    interrupts();
    
    strip.show();
}

void NeoPixel::setBrightness(int b) {
    Serial.println("Setting brightness to " + String(b));
    brightness = b;
    
    // Disable interrupts during strip updates
    noInterrupts();
    strip.setBrightness(brightness);
    interrupts();
    
    strip.show();
}

void NeoPixel::setPattern(PatternType pattern) {
    Serial.println("Setting pattern to " + String(pattern));
    currentPattern = pattern;
    
    // Disable interrupts during pattern setup
    noInterrupts();
    
    // Simple placeholder: pattern 0 = all off, 1 = all red, 2 = rainbow
    if (pattern == PATTERN_OFF) {
        // Turn off all LEDs
        setAllPixels(strip.Color(0,0,0));
    } else if (pattern == PATTERN_RED) {
        // All red
        setAllPixels(strip.Color(255,0,0));
    } else if (pattern == PATTERN_RAINBOW) {
        // Simple rainbow: each pixel a different color
        for (int i = 0; i < NUM_PIXELS; ++i) {
            uint32_t color = strip.Color((i*40)%255, (255-(i*40))%255, (i*80)%255);
            pixelColors[i] = color;
            strip.setPixelColor(i, color);
        }
        strip.show();
    } else if (pattern == PATTERN_CHASE) {
        // Set up for chase pattern - actual animation happens in update()
        // Just initialize with all pixels off
        setAllPixels(strip.Color(0,0,0));
    } else if (pattern == PATTERN_FADE) {
        // Set up for fade pattern - actual animation happens in update()
        setAllPixels(strip.Color(0,0,0));
    } else if (pattern == PATTERN_TWINKLE) {
        // Set up for twinkle pattern - actual animation happens in update()
        setAllPixels(strip.Color(0,0,0));
    } else if (pattern == PATTERN_FIRE) {
        // Set up for fire pattern - actual animation happens in update()
        setAllPixels(strip.Color(10,0,0)); // Start with dim red
    } else if (pattern == PATTERN_RAIN) {
        // Set up for rain pattern - actual animation happens in update()
        setAllPixels(strip.Color(0,0,0)); // Start with all off
    } else if (pattern == PATTERN_COLOR_WIPE) {
        // Set up for color wipe pattern - actual animation happens in update()
        setAllPixels(strip.Color(0,0,0)); // Start with all off
    }
    
    interrupts();
}

void NeoPixel::show() {
    // Disable interrupts during strip updates
    noInterrupts();
    for (int i = 0; i < NUM_PIXELS; ++i) {
        strip.setPixelColor(i, pixelColors[i]);
    }
    interrupts();
    
    strip.show();
    // Serial.println("NeoPixel strip updated"); // Commented out to reduce serial spam
}

void NeoPixel::update() {
    // Return early if no active animated pattern
    if (currentPattern < PATTERN_CHASE) return;
    
    // Throttle updates to not overwhelm the system
    unsigned long currentTime = millis();
    if (currentTime - lastUpdate < 50) return; // Update at max ~20fps
    
    lastUpdate = currentTime;
    
    // Handle different animated patterns
    switch (currentPattern) {
        case PATTERN_CHASE:
            updateChasePattern();
            break;
        case PATTERN_FADE:
            updateFadePattern();
            break;
        case PATTERN_TWINKLE:
            updateTwinklePattern();
            break;
        case PATTERN_FIRE:
            updateFirePattern();
            break;
        case PATTERN_RAIN:
            updateRainPattern();
            break;
        case PATTERN_COLOR_WIPE:
            updateColorWipePattern();
            break;
        default:
            // No animation for other patterns
            break;
    }
}

void NeoPixel::updateChasePattern() {
    static int chasePosition = 0;
    // Removed unused variable chaseColor
    
    // Clear previous position
    for (int i = 0; i < NUM_PIXELS; i++) {
        strip.setPixelColor(i, 0);
        pixelColors[i] = 0;
    }
    
    // Set the "chase" pixel and a few neighbors
    for (int i = 0; i < 3; i++) {
        int pos = (chasePosition + i) % NUM_PIXELS;
        uint32_t color = strip.Color(0, 0, 255 - (i * 60)); // Fading blue tail
        strip.setPixelColor(pos, color);
        pixelColors[pos] = color;
    }
    
    strip.show();
    
    // Move the chase position
    chasePosition = (chasePosition + 1) % NUM_PIXELS;
}

void NeoPixel::updateFadePattern() {
    static int fadeDirection = 1;
    static int fadeValue = 0;
    
    // Create a fading effect
    uint32_t color = strip.Color(fadeValue, 0, fadeValue);
    for (int i = 0; i < NUM_PIXELS; i++) {
        strip.setPixelColor(i, color);
        pixelColors[i] = color;
    }
    
    strip.show();
    
    // Update fade value and direction
    fadeValue += (fadeDirection * 5);
    if (fadeValue >= 255) {
        fadeValue = 255;
        fadeDirection = -1;
    } else if (fadeValue <= 0) {
        fadeValue = 0;
        fadeDirection = 1;
    }
}

void NeoPixel::updateTwinklePattern() {
    // Slowly return all LEDs to black
    for (int i = 0; i < NUM_PIXELS; i++) {
        uint32_t color = pixelColors[i];
        uint8_t r = (color >> 16) & 0xFF;
        uint8_t g = (color >> 8) & 0xFF;
        uint8_t b = color & 0xFF;
        
        // Fade out pixels with a smooth decay
        if (r > 0) r = (r * 95) / 100;
        if (g > 0) g = (g * 95) / 100;
        if (b > 0) b = (b * 95) / 100;
        
        pixelColors[i] = strip.Color(r, g, b);
        strip.setPixelColor(i, pixelColors[i]);
    }
    
    // Randomly light up new pixels
    static unsigned long lastTwinkle = 0;
    unsigned long now = millis();
    
    // Add new twinkles at a randomized rate
    if (now - lastTwinkle > (unsigned long)random(20, 150)) {
        lastTwinkle = now;
        
        // Pick random pixel positions
        int numTwinkles = random(1, 4); // 1-3 new twinkles at a time
        for (int i = 0; i < numTwinkles; i++) {
            int idx = random(NUM_PIXELS);
            
            // Initialize r, g, b to 0
            uint8_t r = 0, g = 0, b = 0;
            int colorChoice = random(3);
            switch (colorChoice) {
                case 0: // White
                    r = g = b = random(180, 255);
                    break;
                case 1: // Pale blue
                    r = random(20, 70);
                    g = random(150, 220);
                    b = random(200, 255);
                    break;
                case 2: // Pale gold/yellow
                    r = random(200, 255);
                    g = random(150, 220);
                    b = random(10, 40);
                    break;
            }
            
            pixelColors[idx] = strip.Color(r, g, b);
            strip.setPixelColor(idx, pixelColors[idx]); // Fixed: use idx instead of i
        }
    }
    
    strip.show();
}

void NeoPixel::updateFirePattern() {
    // Fire effect simulation - red/orange/yellow flicker
    for (int i = 0; i < NUM_PIXELS; i++) {
        // Get a random number in the range controlled by heat (higher = more intense fire)
        int flicker = random(80, 150);
        
        // Basic fire colors with randomized intensity
        uint8_t r = flicker; // red is always high
        uint8_t g = flicker * 0.4; // less green (orange-ish)
        uint8_t b = flicker * 0.1; // very little blue
        
        // Add some additional random variation
        if (random(100) < 30) {
            // Occasionally boost a pixel to make it flare
            r = min(255, r + (int)random(30, 80));
            g = min(255, g + (int)random(20, 50));
        }
        
        pixelColors[i] = strip.Color(r, g, b);
        strip.setPixelColor(i, pixelColors[i]);
    }
    
    strip.show();
}

void NeoPixel::updateRainPattern() {
    // Simulate rain falling - blue drops
    // First, move all existing colors down by one pixel
    for (int i = NUM_PIXELS - 1; i > 0; i--) {
        pixelColors[i] = pixelColors[i - 1];
        strip.setPixelColor(i, pixelColors[i]);
    }
    
    // New raindrops appear randomly at the top
    if (random(100) < 25) { // 25% chance of a new raindrop
        // Pick a blue color with some variation
        uint8_t b = random(180, 240);
        uint8_t g = b / 3; // Some cyan tint
        pixelColors[0] = strip.Color(0, g, b);
    } else {
        pixelColors[0] = strip.Color(0, 0, 0); // No drop, black
    }
    strip.setPixelColor(0, pixelColors[0]);
    
    // Add some random water puddle effects at the bottom
    if (random(100) < 10) {
        int puddleIdx = random(NUM_PIXELS - 5, NUM_PIXELS);
        uint8_t puddleBlue = random(50, 100);
        pixelColors[puddleIdx] = strip.Color(0, puddleBlue/2, puddleBlue);
        strip.setPixelColor(puddleIdx, pixelColors[puddleIdx]);
    }
    
    strip.show();
}

void NeoPixel::updateColorWipePattern() {
    static int wipePosition = 0;
    static int colorIndex = 0;
    static const uint32_t wipeColors[] = {
        strip.Color(255, 0, 0),     // Red
        strip.Color(0, 255, 0),     // Green
        strip.Color(0, 0, 255),     // Blue
        strip.Color(255, 255, 0),   // Yellow
        strip.Color(0, 255, 255),   // Cyan
        strip.Color(255, 0, 255)    // Magenta
    };
    static const int numColors = 6;
    
    // Set the current pixel to the current color
    pixelColors[wipePosition] = wipeColors[colorIndex];
    strip.setPixelColor(wipePosition, wipeColors[colorIndex]);
    
    strip.show();
    
    // Advance to the next position
    wipePosition++;
    
    // If we've filled the entire strip, move to the next color
    if (wipePosition >= NUM_PIXELS) {
        wipePosition = 0;
        colorIndex = (colorIndex + 1) % numColors;
        
        // Clear the strip when starting a new color
        for (int i = 0; i < NUM_PIXELS; i++) {
            pixelColors[i] = strip.Color(0, 0, 0);
            strip.setPixelColor(i, 0);
        }
    }
}

bool NeoPixel::isAnimationActive() {
    // Check if the current pattern is an animated one
    return (currentPattern == PATTERN_CHASE || 
            currentPattern == PATTERN_FADE || 
            currentPattern == PATTERN_TWINKLE ||
            currentPattern == PATTERN_FIRE ||
            currentPattern == PATTERN_RAIN ||
            currentPattern == PATTERN_COLOR_WIPE);
}

uint32_t NeoPixel::rgbToColor(int r, int g, int b) {
    return strip.Color(r, g, b);
}

String NeoPixel::getStatusJson() {
    JsonDocument doc;
    doc["brightness"] = brightness;
    doc["pattern"] = currentPattern;
    doc["pixels"] = JsonArray();
    JsonArray arr = doc["pixels"].to<JsonArray>();
    for (int i = 0; i < NUM_PIXELS; ++i) {
        uint32_t c = pixelColors[i];
        arr.add(c);
    }
    String out;
    serializeJson(doc, out);
    return out;
}
