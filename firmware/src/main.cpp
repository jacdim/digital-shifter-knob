#ifndef USE_SDL
#include <Arduino.h>
#include <Preferences.h>
#include "BLEManager.h"
#else
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
unsigned long millis() {
    auto now = std::chrono::system_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
}
void delay(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}
#endif

#include "DisplaySetup.h"

// --- Pin Definitions (Seeed XIAO ESP32S3) ---
const int PIN_HALL_X = 1; // A0
const int PIN_HALL_Y = 2; // A1

// --- Globals ---
#ifndef USE_SDL
Preferences preferences;
BLEManager ble;
#endif

enum GearState {
    GEAR_NEUTRAL = 0,
    GEAR_1 = 1,
    GEAR_2 = 2,
    GEAR_3 = 3,
    GEAR_4 = 4,
    GEAR_5 = 5,
    GEAR_6 = 6,
    GEAR_REVERSE = 7
};

GearState currentGear = GEAR_NEUTRAL;

struct GearZone {
    int minX; int maxX;
    int minY; int maxY;
};

// Default wide zones
GearZone zones[8];
const int ZONE_RADIUS = 300; // tolerance around center

// --- Function Prototypes ---
void saveCalibration(int gearIndex, int x, int y);
void loadCalibration();

#ifndef USE_SDL
// --- Calibration Handler ---
// Expected format from BLE: "gearIndex,x,y" e.g., "1,1500,2000"
void handleCalibrationCommand(String cmd) {
    int firstComma = cmd.indexOf(',');
    int secondComma = cmd.indexOf(',', firstComma + 1);
    
    if (firstComma > 0 && secondComma > 0) {
        int gIndex = cmd.substring(0, firstComma).toInt();
        int x = cmd.substring(firstComma + 1, secondComma).toInt();
        int y = cmd.substring(secondComma + 1).toInt();
        
        if (gIndex >= 0 && gIndex <= 7) {
            saveCalibration(gIndex, x, y);
            Serial.printf("Calibrated Gear %d to Center(%d, %d)\n", gIndex, x, y);
        }
    }
}

void saveCalibration(int gearIndex, int x, int y) {
    String keyX = "gx" + String(gearIndex);
    String keyY = "gy" + String(gearIndex);
    
    preferences.putInt(keyX.c_str(), x);
    preferences.putInt(keyY.c_str(), y);
    
    zones[gearIndex].minX = x - ZONE_RADIUS;
    zones[gearIndex].maxX = x + ZONE_RADIUS;
    zones[gearIndex].minY = y - ZONE_RADIUS;
    zones[gearIndex].maxY = y + ZONE_RADIUS;
}

void loadCalibration() {
    preferences.begin("shifter", false);
    for (int i = 0; i < 8; i++) {
        String keyX = "gx" + String(i);
        String keyY = "gy" + String(i);
        
        // Default center is 2048 (midpoint of 12-bit ADC)
        int cx = preferences.getInt(keyX.c_str(), 2048);
        int cy = preferences.getInt(keyY.c_str(), 2048);
        
        zones[i].minX = cx - ZONE_RADIUS;
        zones[i].maxX = cx + ZONE_RADIUS;
        zones[i].minY = cy - ZONE_RADIUS;
        zones[i].maxY = cy + ZONE_RADIUS;
    }
}

GearState determineGear(int x, int y) {
    for (int i = 1; i < 8; i++) { // Check gears 1-6 and R
        if (x >= zones[i].minX && x <= zones[i].maxX &&
            y >= zones[i].minY && y <= zones[i].maxY) {
            return (GearState)i;
        }
    }
    // If it doesn't match any specific gear zone, assume Neutral
    return GEAR_NEUTRAL; 
}

String getGearString(GearState gear) {
    switch(gear) {
        case GEAR_NEUTRAL: return "N";
        case GEAR_REVERSE: return "R";
        case GEAR_1: return "1";
        case GEAR_2: return "2";
        case GEAR_3: return "3";
        case GEAR_4: return "4";
        case GEAR_5: return "5";
        case GEAR_6: return "6";
        default: return "N";
    }
}

void setup() {
    Serial.begin(115200);
    analogReadResolution(12);
    
    // Init display
    initDisplay();
    
    // Load saved calibration points
    loadCalibration();
    
    // Init BLE
    ble.init();
    
    Serial.println("Digital Shifter Knob - Firmware Running");
}

unsigned long lastRawNotify = 0;

void loop() {
    lv_timer_handler(); // let the GUI do its work
    lv_tick_inc(5); // Update LVGL time by 5ms
    
    // EMA Filtering to smooth sensor noise
    static float smoothedX = 2048.0;
    static float smoothedY = 2048.0;
    const float alpha = 0.2; // Smoothing factor (0.0 to 1.0, lower is smoother)
    
    smoothedX = (alpha * analogRead(PIN_HALL_X)) + ((1.0 - alpha) * smoothedX);
    smoothedY = (alpha * analogRead(PIN_HALL_Y)) + ((1.0 - alpha) * smoothedY);
    
    int valX = (int)smoothedX;
    int valY = (int)smoothedY;

    GearState newGear = determineGear(valX, valY);

    if (newGear != currentGear) {
        currentGear = newGear;
        String gearStr = getGearString(currentGear);
        Serial.println("Gear changed: " + gearStr);
        updateDisplayGear(gearStr);
        ble.notifyGear(gearStr);
    }

    // Notify raw sensor data every 100ms for the Calibration App
    if (millis() - lastRawNotify > 100) {
        ble.notifyRawSensor(valX, valY);
        lastRawNotify = millis();
    }

    delay(5);
}
#else

std::string getGearString(GearState gear) {
    switch(gear) {
        case GEAR_NEUTRAL: return "N";
        case GEAR_REVERSE: return "R";
        case GEAR_1: return "1";
        case GEAR_2: return "2";
        case GEAR_3: return "3";
        case GEAR_4: return "4";
        case GEAR_5: return "5";
        case GEAR_6: return "6";
        default: return "N";
    }
}

void setup() {
    initDisplay();
    std::cout << "Digital Shifter Knob - Emulator Running" << std::endl;
}

unsigned long lastGearChange = 0;

void loop() {
    lv_timer_handler();
    lv_tick_inc(5);
    
    if (millis() - lastGearChange > 2000) {
        lastGearChange = millis();
        // Simulate gear change
        currentGear = (GearState)((currentGear + 1) % 8);
        std::string gearStr = getGearString(currentGear);
        std::cout << "Gear simulated: " << gearStr << std::endl;
        updateDisplayGear(gearStr);
    }
    
    delay(5);
}

int main(void) {
    setup();
    while (true) {
        loop();
    }
    return 0;
}

#endif
