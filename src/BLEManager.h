#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#include "consts.h"
#include "BluetoothManager.h"
#include "BLECallbacks.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <queue>

class MyServerCallbacks;
class MyCallbacks;

class BLEManager {
private:
    BLEServer *pServer;
    BLEService *pService;
    BLECharacteristic *rxCharacteristic;
    BLECharacteristic *txCharacteristic;

    bool connected = false;
    std::queue<String> commandQueue;

public:
    BLEManager() = default;

    void onConnect(BLEServer *pServer) {
        Serial.println(LogPrefixBLE + "Device connected");
        connected = true;
    }

    void onDisconnect(BLEServer *pServer) {
        Serial.println(LogPrefixBLE + "Device disconnected");
        connected = false;
        pServer->startAdvertising();
    }

    void onDataReceived(BLECharacteristic *pCharacteristic) {
        String receivedData = pCharacteristic->getValue().c_str();
        Serial.printf("%s onDataReceived: '%s'\n", LogPrefixBLE, receivedData);
        commandQueue.push(receivedData);
    }

    void init() {
        BLEDevice::init(DeviceNameBLE.c_str());
        pServer = BLEDevice::createServer();

        MyServerCallbacks* serverCallbacks = new MyServerCallbacks(this);
        pServer->setCallbacks(serverCallbacks);

        pService = pServer->createService(SERVICE_UUID);
        rxCharacteristic = pService->createCharacteristic(RX_CHAR_UUID, BLECharacteristic::PROPERTY_WRITE);

        txCharacteristic = pService->createCharacteristic(TX_CHAR_UUID, BLECharacteristic::PROPERTY_NOTIFY);
        txCharacteristic->addDescriptor(new BLE2902());

        MyCallbacks* myCallbacks = new MyCallbacks(this);
        rxCharacteristic->setCallbacks(myCallbacks);

        pService->start();

        pServer->getAdvertising()->start();
        Serial.println(LogPrefixBLE + "initialized and advertising");
    }

    bool isConnected() const {
        return connected;
    }

    bool hasCommand() const {
        return !commandQueue.empty();
    }

    String getNextCommand() {
        if (commandQueue.empty()) return "";
        String command = commandQueue.front();
        commandQueue.pop();
        return command;
    }

    bool sendToClient(const String &data) {
        if (!connected) return false;
        Serial.printf("%ssendToClient: '%s'\n", LogPrefixBLE, data.c_str());
        txCharacteristic->setValue(data.c_str());
        txCharacteristic->notify();
        return true;
    }
};

#endif // BLE_MANAGER_H
