#ifndef BLUETOOTH_MANAGER_H
#define BLUETOOTH_MANAGER_H

/////////////////////////
#define USE_BLE
#define LOG_VERBOSE
///////////////////////////////

#include "Arduino.h"
#include <queue>

class BluetoothManager {
protected:
    bool connected = false;
    std::queue<String> commandQueue;
    virtual bool sendToClient(const String &data) const = 0;
    virtual ~BluetoothManager() = default;
public:
    virtual void begin(const String &deviceName) = 0;
    virtual void end() = 0;

    void print(const String&data) {
        sendToClient(data);
    }

    void println(const String&data) {
        sendToClient(data + "\n");
    }

    bool hasClient() const {
        return connected;
    }

    bool hasNextCommand() const {
        return !commandQueue.empty();
    }

    String getNextCommandLowCase() {
        if (commandQueue.empty()) return "";
        String command = commandQueue.front();
        command.toLowerCase();
        commandQueue.pop();
        return command;
    }
};


#ifdef USE_BLE
    const String DeviceNameDefault = "UART-Test-LE";
    const String BTLogPrefix = "BLE: ";
    #include "BTManLowEnImpl.h"
    BLEManager btManager;
#else
    const String DeviceNameDefault = "UART-Test-Classic";
    const String BTLogPrefix = "BTClass: ";
    #include "BTManClassImpl.h"
    BTClassicManager btManager;
#endif

#endif // BLUETOOTH_MANAGER_H
