
#ifndef BLUETOOTH_MANAGER_H
#define BLUETOOTH_MANAGER_H

#include "Arduino.h"

class BluetoothManager {
public:
    virtual void init() const = 0;
    virtual bool isConnected() const = 0;
    virtual bool hasCommand() const = 0;
    virtual String getNextCommand() = 0;
    virtual bool sendToClient(const String &data) = 0;
    virtual ~BluetoothManager() = default;
};

#endif // BLUETOOTH_MANAGER_H
