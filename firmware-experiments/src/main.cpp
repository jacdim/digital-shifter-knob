#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_GC9A01A.h>
#include <SPI.h>

// ─── Onboard LED ───
#define LED_PIN 21

// ─── Pin definitions (ALL on LEFT side of XIAO) ───
#define TFT_MOSI 2   // D1 = row 1 -> connects to display SDA
#define TFT_SCK  3   // D2 = row 2 -> connects to display SCL
#define TFT_CS   4   // D3 = row 3 -> connects to display CS
#define TFT_DC   5   // D4 = row 4 -> connects to display DC
#define TFT_RST  6   // D5 = row 5 -> connects to display RST

// Software SPI with explicit pins
Adafruit_GC9A01A tft(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCK, TFT_RST);

void setup() {
    Serial.begin(115200);
    delay(3000);

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);

    Serial.println("=== GC9A01A TEST ===");

    tft.begin();
    tft.setRotation(0);

    Serial.println("Filling RED...");
    tft.fillScreen(GC9A01A_RED);
    delay(1000);

    Serial.println("Filling GREEN...");
    tft.fillScreen(GC9A01A_GREEN);
    delay(1000);

    Serial.println("Filling BLUE...");
    tft.fillScreen(GC9A01A_BLUE);
    delay(1000);

    Serial.println("Drawing text...");
    tft.fillScreen(GC9A01A_BLACK);
    tft.setTextColor(GC9A01A_WHITE);
    tft.setTextSize(4);
    tft.setCursor(85, 100);
    tft.println("Hi!");
}

int colorIndex = 0;
unsigned long lastSwitch = 0;

void loop() {
    digitalWrite(LED_PIN, LOW);
    delay(500);
    digitalWrite(LED_PIN, HIGH);
    delay(500);

    if (millis() - lastSwitch > 2000) {
        lastSwitch = millis();
        switch (colorIndex % 5) {
            case 0: tft.fillScreen(GC9A01A_RED);    break;
            case 1: tft.fillScreen(GC9A01A_GREEN);  break;
            case 2: tft.fillScreen(GC9A01A_BLUE);   break;
            case 3: tft.fillScreen(GC9A01A_YELLOW); break;
            case 4: tft.fillScreen(GC9A01A_WHITE);  break;
        }
        colorIndex++;
    }
}
