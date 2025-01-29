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
    const uint16_t headerReserve = 3; // Запас на BLE заголовки
    const String fragmentDelimiter = "\x0B"; // Символ конца не последней части сообщения
    String partialMsg = ""; // Буфер для частичного сообщения

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
        //BLEDevice::setMTU(32); // default 23
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
            uint16_t mtu = getPayloadSize();
            Serial.printf("%s Default payload size = %d\n", BTLogPrefix.c_str(), mtu);
        #endif
    }

    void end() override {
        pServer->removeService(pService);
    }

private:
    uint16_t getPayloadSize() const {
        return BLEDevice::getMTU() - headerReserve;
    }

    bool sendToClient(const String &data) const override {
        if (!connected) return false;

        const uint16_t payloadSize = getPayloadSize();
        const uint16_t dataLength = data.length();
        
        #ifdef LOG_VERBOSE
            Serial.printf("%ssendToClient: '%s'; MTU = %d; dataLength= %d \n", BTLogPrefix, data.c_str(), payloadSize, dataLength);
        #endif

        if (dataLength <= payloadSize) {
            txCharacteristic->setValue(data.c_str());
            txCharacteristic->notify();
        } else {
            uint16_t partLength = payloadSize - fragmentDelimiter.length();
            for (int i = 0; i < dataLength; i += partLength) {
                String fragment = "";
                // Если это не последняя часть, добавляем символ fragmentDelimiter
                if (i + partLength >= dataLength - 1) {
                    fragment = data.substring(i, dataLength);
                    i++;
                } else {
                    fragment = data.substring(i, i + partLength) + fragmentDelimiter;
                }
                txCharacteristic->setValue(fragment.c_str());
                txCharacteristic->notify();
            }
        }
        return true;
    }
    
    void onConnect(BLEServer *pServer) {
        #ifdef LOG_VERBOSE
            Serial.println(BTLogPrefix + "Device connected");
        #endif
        connected = true;
        #ifdef LOG_VERBOSE
            uint16_t mtu = getPayloadSize();
            Serial.printf("%s Payload size after connect = %d\n", BTLogPrefix.c_str(), mtu);
        #endif
        partialMsg = "";
    }

    void onDisconnect(BLEServer *pServer) {
        #ifdef LOG_VERBOSE
            Serial.println(BTLogPrefix + "Device disconnected");
        #endif
        connected = false;
        pServer->startAdvertising();
    }

    void onDataReceived(BLECharacteristic *pCharacteristic) {
        String msg = pCharacteristic->getValue().c_str();

        if (msg.endsWith(fragmentDelimiter)) { // Убираем символ Переноса и добавляем к буферу
            String msgPart = msg.substring(0, msg.length() - 1);
        #ifdef LOG_VERBOSE
            Serial.printf("%s received Part: '%s' (%d)\n", BTLogPrefix, msgPart, msgPart.length());
        #endif
            partialMsg += msgPart;
        } else { // Добавляем остаток к буферу и считаем сообщение полным
        #ifdef LOG_VERBOSE
            Serial.printf("%s received End: '%s' (%d)\n", BTLogPrefix, msg, msg.length());
        #endif
            commandQueue.push(partialMsg + msg);
            partialMsg = "";
        }
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
