#include "BTManager.h"
#ifdef USE_BLE

#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// BLE: UUID для Nordic UART Service
const String ServiceUUID = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";
const String RxCharUUID = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E";
const String TxCharUUID = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E";


class BLEManager; // Manager forward declaration

/**
 *  Callbacks declaration
 * */
class BLECallbacksStatus : public BLEServerCallbacks {
    private:
        BLEManager* manager;
    public:
        BLECallbacksStatus(BLEManager* manager) : manager(manager) {}

        void onConnect(BLEServer *pServer) override;

        void onDisconnect(BLEServer *pServer) override;
};

class BLECallbacksData : public BLECharacteristicCallbacks {
    private:
        BLEManager* manager;
    public:
        BLECallbacksData(BLEManager* manager) : manager(manager) {}

        void onWrite(BLECharacteristic *pCharacteristic) override;
};

/**
 *  Manager declaration and implementation
 * */
class BLEManager: public BluetoothManager {
private:
    BLEServer *pServer;
    BLEService *pService;
    BLECharacteristic *rxCharacteristic;
    BLECharacteristic *txCharacteristic;
public:
    BLEManager() = default;

    ~BLEManager() override {
        delete pServer;
    };

    void begin(const String &deviceName) override {
        BLEDevice::init(deviceName.c_str());
        // https://github.com/nkolban/esp32-snippets/issues/325
        // Basically characteristic value read on ios only shows the first 23 bytes
        // ... apple has no MTU setting ...
        BLEDevice::setMTU(32); // default 23
        pServer = BLEDevice::createServer();

        BLECallbacksStatus* callbacksStatus = new BLECallbacksStatus(this);
        pServer->setCallbacks(callbacksStatus);

        pService = pServer->createService(ServiceUUID.c_str());
        rxCharacteristic = pService->createCharacteristic(RxCharUUID.c_str(), BLECharacteristic::PROPERTY_WRITE);

        txCharacteristic = pService->createCharacteristic(TxCharUUID.c_str(), BLECharacteristic::PROPERTY_NOTIFY);
        txCharacteristic->addDescriptor(new BLE2902());

        BLECallbacksData* callbacksData = new BLECallbacksData(this);
        rxCharacteristic->setCallbacks(callbacksData);

        pService->start();

        pServer->getAdvertising()->start();
        #ifdef LOG_VERBOSE
            Serial.println(BTLogPrefix + "initialized and advertising");
        #endif
    }

    void end() override {
        pServer->removeService(pService);
    }

private:
    bool sendToClient(const String &data) const override {
        if (!connected) return false;
        #ifdef LOG_VERBOSE
            Serial.printf("%ssendToClient: '%s'\n", BTLogPrefix, data.c_str());
        #endif
        txCharacteristic->setValue(data.c_str());
        txCharacteristic->notify();
        return true;
    }
    
    void onConnect(BLEServer *pServer) {
        #ifdef LOG_VERBOSE
            Serial.println(BTLogPrefix + "Device connected");
        #endif
        connected = true;
    }

    void onDisconnect(BLEServer *pServer) {
        #ifdef LOG_VERBOSE
            Serial.println(BTLogPrefix + "Device disconnected");
        #endif
        connected = false;
        pServer->startAdvertising();
    }

    void onDataReceived(BLECharacteristic *pCharacteristic) {
        String receivedData = pCharacteristic->getValue().c_str();
        #ifdef LOG_VERBOSE
            Serial.printf("%s received: '%s'\n", BTLogPrefix, receivedData);
        #endif
        commandQueue.push(receivedData);
    }

    friend class BLECallbacksStatus;
    friend class BLECallbacksData;
};

/**
 *  Callbacks implementation
 * */
void BLECallbacksStatus::onConnect(BLEServer *pServer) {
    manager->onConnect(pServer);
}

void BLECallbacksStatus::onDisconnect(BLEServer *pServer) {
    manager->onDisconnect(pServer);
}

void BLECallbacksData::onWrite(BLECharacteristic *pCharacteristic) {
    manager->onDataReceived(pCharacteristic);
}

#endif // BLE_MANAGER_H

#endif // USE_BLE
