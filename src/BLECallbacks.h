#ifndef BLECALLBACKS
#define BLECALLBACKS


#include <BLEServer.h>

class BLEManager;

class MyServerCallbacks : public BLEServerCallbacks {
    private:
        BLEManager* manager;
    public:
        MyServerCallbacks(BLEManager* manager) : manager(manager) {}

        void onConnect(BLEServer *pServer) override;

        void onDisconnect(BLEServer *pServer) override;
};

class MyCallbacks : public BLECharacteristicCallbacks {
    private:
        BLEManager* manager;
    public:
        MyCallbacks(BLEManager* manager) : manager(manager) {}

        void onWrite(BLECharacteristic *pCharacteristic) override;
};

#endif // BLECALLBACKS