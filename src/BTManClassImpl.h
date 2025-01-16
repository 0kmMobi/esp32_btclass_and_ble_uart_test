#include "BTManager.h"
#ifndef USE_BLE

#ifndef BTC_MANAGER_H
#define BTC_MANAGER_H

#include "BluetoothSerial.h"

class BTClassicManager: public BluetoothManager {
private:
    BluetoothSerial *SerialBTC;

    static void onConnectCallback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
        if (event == ESP_SPP_SRV_OPEN_EVT) {
            instance->connected = true;
            #ifdef LOG_VERBOSE
                Serial.println(BTLogPrefix + "Client connected.");
            #endif
        } else if (event == ESP_SPP_CLOSE_EVT) {
            instance->connected = false;
            #ifdef LOG_VERBOSE
                Serial.println(BTLogPrefix + "Client disconnected.");
            #endif
        }
    }

    static void onDataReceived(const uint8_t *data, size_t length) {
        String receivedData;
        for (size_t i = 0; i < length; i++) {
            receivedData += (char)data[i];
        }
        #ifdef LOG_VERBOSE
            Serial.printf("%s received: '%s'\n", BTLogPrefix, receivedData);
        #endif
        instance->commandQueue.push(receivedData);
    }

    static BTClassicManager *instance;

public:
    BTClassicManager() {
        instance = this;
    }
    ~BTClassicManager() override {
        delete SerialBTC;
        instance = nullptr;
    };

    void begin(const String &deviceName) override {
        SerialBTC = new BluetoothSerial();
        SerialBTC->begin(deviceName);
        SerialBTC->register_callback(onConnectCallback);
        SerialBTC->onData(onDataReceived);
        #ifdef LOG_VERBOSE
            Serial.println(BTLogPrefix + "Bluetooth initialized");
        #endif
    }

    void end() override {
        SerialBTC->end();
        connected = false;
    }

private:
    bool sendToClient(const String &data) const override {
        if (!connected) return false;
        #ifdef LOG_VERBOSE
            Serial.printf("%ssendToClient: '%s'\n", BTLogPrefix, data.c_str());
        #endif
        SerialBTC->printf("%s", data.c_str());
        int count = 0;
        SerialBTC->print("" + count);
        return true;
    }


};

BTClassicManager *BTClassicManager::instance = nullptr;

#endif // BTC_MANAGER_H

#endif // USE_BLE