#ifndef BTC_MANAGER_H
#define BTC_MANAGER_H

#include "consts.h"
#include "BluetoothSerial.h"
#include <queue>

class BTClassicManager {
private:
    BluetoothSerial SerialBT;
    bool connected = false;
    std::queue<String> commandQueue;

    static void onConnectCallback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
        if (event == ESP_SPP_SRV_OPEN_EVT) {
            instance->connected = true;
            Serial.println(LogPrefixBTClass + "Client connected.");
        } else if (event == ESP_SPP_CLOSE_EVT) {
            instance->connected = false;
            Serial.println(LogPrefixBTClass + "Client disconnected.");
        }
    }

    static void onDataReceived(const uint8_t *data, size_t length) {
        String receivedData;
        for (size_t i = 0; i < length; i++) {
            receivedData += (char)data[i];
        }
        Serial.printf("%sonDataReceived: '%s'\n", LogPrefixBTClass, receivedData);
        instance->commandQueue.push(receivedData);
    }

    static BTClassicManager *instance;

public:
    BTClassicManager() {
        instance = this;
    }

    void init() {
        SerialBT.begin(DeviceNameBTC);
        SerialBT.register_callback(onConnectCallback);
        SerialBT.onData(onDataReceived);
        Serial.println(LogPrefixBTClass + "Bluetooth initialized");
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
        Serial.printf("%ssendToClient: '%s'\n", LogPrefixBTClass, data.c_str());
        SerialBT.printf("%s", data.c_str());
        return true;
    }
};

BTClassicManager *BTClassicManager::instance = nullptr;

#endif // BTC_MANAGER_H