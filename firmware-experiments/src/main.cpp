#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

// ⚠️ Replace with your actual Wi-Fi credentials
const char* ssid = "OPTUS_A28891M";
const char* password = "halts32249um";

// The built-in orange LED on the XIAO ESP32S3 is on GPIO 21 (active-low)
#define LED_PIN 21

// LED state
bool ledOn = false;
bool blinkMode = false;
int blinkRate = 500; // milliseconds

// For non-blocking blink
unsigned long lastBlinkTime = 0;
bool blinkState = false;

// Create an HTTP server on port 80
AsyncWebServer server(80);

// ─── Helper: Add CORS headers to every response ───
void addCorsHeaders(AsyncWebServerResponse* response) {
  response->addHeader("Access-Control-Allow-Origin", "*");
  response->addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  response->addHeader("Access-Control-Allow-Headers", "Content-Type");
}

// ─── Helper: Send the current LED state as JSON ───
void sendLedState(AsyncWebServerRequest* request) {
  JsonDocument doc;
  doc["on"] = ledOn;
  doc["blink"] = blinkMode;
  doc["blinkRate"] = blinkRate;
  doc["temperature"] = temperatureRead();
  doc["rssi"] = WiFi.RSSI();
  doc["freeHeap"] = ESP.getFreeHeap();
  doc["uptime"] = millis() / 1000;

  String json;
  serializeJson(doc, json);

  AsyncWebServerResponse* response = request->beginResponse(200, "application/json", json);
  addCorsHeaders(response);
  request->send(response);
}

// ─── Apply the LED state ───
void applyLed() {
  if (!blinkMode) {
    // Active-low: LOW = ON, HIGH = OFF
    digitalWrite(LED_PIN, ledOn ? LOW : HIGH);
  }
}

void setup() {
  Serial.begin(115200);
  delay(3000); // Wait for USB serial to connect

  // Set up the LED pin
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH); // Start with LED off (active-low)

  // Mount LittleFS (holds the built React app)
  if (!LittleFS.begin(true)) {
    Serial.println("ERROR: Failed to mount LittleFS!");
  } else {
    Serial.println("LittleFS mounted successfully.");
  }

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.println("\nConnecting to WiFi...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("\nConnected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // ─── CORS preflight handler (browsers send OPTIONS before POST) ───
  server.on("/api/led", HTTP_OPTIONS, [](AsyncWebServerRequest* request) {
    AsyncWebServerResponse* response = request->beginResponse(204);
    addCorsHeaders(response);
    request->send(response);
  });

  // ─── GET /api/led — Return current LED state ───
  server.on("/api/led", HTTP_GET, [](AsyncWebServerRequest* request) {
    sendLedState(request);
  });

  // ─── POST /api/led — Update LED state ───
  // We collect the body manually since AsyncCallbackJsonWebHandler
  // is not available in all library forks.
  server.on("/api/led", HTTP_POST, 
    // 1) Called when the request is complete (headers received, body collected)
    [](AsyncWebServerRequest* request) {
      // The body was already parsed in the onBody callback below.
      // Just send back the current state.
      sendLedState(request);
    },
    NULL, // no upload handler
    // 2) onBody callback — receives the raw body data
    [](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
      // Parse the JSON body
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, data, len);
      if (error) {
        Serial.printf("JSON parse error: %s\n", error.c_str());
        return;
      }

      if (doc["on"].is<bool>()) {
        ledOn = doc["on"].as<bool>();
      }
      if (doc["blink"].is<bool>()) {
        blinkMode = doc["blink"].as<bool>();
      }
      if (doc["blinkRate"].is<int>()) {
        blinkRate = doc["blinkRate"].as<int>();
        if (blinkRate < 50) blinkRate = 50;
        if (blinkRate > 5000) blinkRate = 5000;
      }

      applyLed();
      Serial.printf("LED: on=%d, blink=%d, rate=%d\n", ledOn, blinkMode, blinkRate);
    }
  );

  // ─── Serve the React app from LittleFS ───
  server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

  server.begin();
  Serial.println("Server started!");
}

void loop() {
  // Handle blink mode with non-blocking timing
  if (ledOn && blinkMode) {
    unsigned long now = millis();
    if (now - lastBlinkTime >= (unsigned long)blinkRate) {
      lastBlinkTime = now;
      blinkState = !blinkState;
      digitalWrite(LED_PIN, blinkState ? LOW : HIGH);
    }
  }
}
