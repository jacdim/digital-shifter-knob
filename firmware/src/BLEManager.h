#pragma once
#include <NimBLEDevice.h>
#include <Arduino.h>

#define SERVICE_UUID           "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHAR_UUID_GEAR         "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CHAR_UUID_RAW_SENSOR   "8c1df81c-d7d8-4f8e-bd71-c0068dc26a87"
#define CHAR_UUID_CALIBRATE    "59c62376-74fc-487b-83ee-bf35a4d468eb"

extern void handleCalibrationCommand(String cmd);

class CalibrationCallbacks : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pCharacteristic) {
        String value = String(pCharacteristic->getValue().c_str());
        if (value.length() > 0) {
            Serial.print("Received Calibration Data: ");
            Serial.println(value);
            handleCalibrationCommand(value);
        }
    }
};

class BLEManager {
private:
    NimBLEServer* pServer = nullptr;
    NimBLECharacteristic* pGearChar = nullptr;
    NimBLECharacteristic* pRawChar = nullptr;
    NimBLECharacteristic* pCalibrateChar = nullptr;
    bool deviceConnected = false;

    class ServerCallbacks : public NimBLEServerCallbacks {
        BLEManager* manager;
    public:
        ServerCallbacks(BLEManager* m) : manager(m) {}
        void onConnect(NimBLEServer* pServer) {
            manager->deviceConnected = true;
            Serial.println("BLE Client Connected");
        }
        void onDisconnect(NimBLEServer* pServer) {
            manager->deviceConnected = false;
            Serial.println("BLE Client Disconnected");
            NimBLEDevice::startAdvertising();
        }
    };

public:
    void init() {
        NimBLEDevice::init("ShiftKnob_R53");
        pServer = NimBLEDevice::createServer();
        pServer->setCallbacks(new ServerCallbacks(this));

        NimBLEService* pService = pServer->createService(SERVICE_UUID);

        pGearChar = pService->createCharacteristic(
            CHAR_UUID_GEAR,
            NIMBLE_PROPERTY::NOTIFY
        );

        pRawChar = pService->createCharacteristic(
            CHAR_UUID_RAW_SENSOR,
            NIMBLE_PROPERTY::NOTIFY
        );

        pCalibrateChar = pService->createCharacteristic(
            CHAR_UUID_CALIBRATE,
            NIMBLE_PROPERTY::WRITE
        );
        pCalibrateChar->setCallbacks(new CalibrationCallbacks());

        pService->start();

        NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
        pAdvertising->addServiceUUID(SERVICE_UUID);
        pAdvertising->setScanResponse(true);
        pAdvertising->start();
        Serial.println("BLE Advertising Started");
    }

    void notifyGear(String gearStr) {
        if (deviceConnected && pGearChar) {
            pGearChar->setValue(gearStr.c_str());
            pGearChar->notify();
        }
    }

    void notifyRawSensor(int x, int y) {
        if (deviceConnected && pRawChar) {
            char buffer[32];
            snprintf(buffer, sizeof(buffer), "%d,%d", x, y);
            pRawChar->setValue(buffer);
            pRawChar->notify();
        }
    }
    
    bool isConnected() {
        return deviceConnected;
    }
};
