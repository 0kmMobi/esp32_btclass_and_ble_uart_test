#include "BLECallbacks.h"
#include "BLEManager.h"


void MyServerCallbacks::onConnect(BLEServer *pServer) {
    manager->onConnect(pServer);
}

void MyServerCallbacks::onDisconnect(BLEServer *pServer) {
    manager->onDisconnect(pServer);
}

void MyCallbacks::onWrite(BLECharacteristic *pCharacteristic) {
    manager->onDataReceived(pCharacteristic);
}
