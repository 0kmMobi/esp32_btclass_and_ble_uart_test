#include <Arduino.h>
#include "BluetoothManager.h"
#define DEV_NODEMCU32S 1
#define DEV_WEMOS_LOLIN32_LITE 2

/////////////////////////////////
//#define USE_BLE
#define DEV_ID DEV_WEMOS_LOLIN32_LITE
///////////////////////////////


#ifdef USE_BLE
  #include "BLEManager.h"
  BLEManager btManager;
#else
  #include "BTClassicManager.h"
  BTClassicManager btManager;
#endif

#if (DEV_ID == DEV_NODEMCU32S)
  #define LED  2 // LED_BUILTIN (nodemcu-32s)
  #define LED_INVERT false
#elif (DEV_ID == DEV_WEMOS_LOLIN32_LITE)
  #define LED 22 // lolin32_lite (Wemos Lolin32 Lite)
  #define LED_INVERT true
#else 
  #error "The device is not defined. Compilation stopped."
#endif

void setup() {
    Serial.begin(115200);
    pinMode(LED, OUTPUT);
    digitalWrite(LED, LED_INVERT?HIGH:LOW);

    btManager.init();
    Serial.println("Setup complete.");
}

unsigned long notifyTimerStart = 0;
uint32_t counter = 0;

void sendHearbeat() {
  if(notifyTimerStart + 2000L <= millis()) {
    Serial.printf("Counter: %d\n", counter);
    btManager.sendToClient(String(counter));
    notifyTimerStart = millis();
    counter ++;
  }
}

void loop() {
    if(btManager.isConnected()) {
      sendHearbeat();

      if (btManager.hasCommand()) {
          String command = btManager.getNextCommand();
          command.trim();
          Serial.println("Received command: " + command);

          if (command.equalsIgnoreCase("LED_ON")) {
              digitalWrite(LED, LED_INVERT?LOW:HIGH);
              String sLed = String("LED switched ON");
              btManager.sendToClient(sLed);
          } else if (command.equalsIgnoreCase("LED_OFF")) {
              digitalWrite(LED, LED_INVERT?HIGH:LOW);
              btManager.sendToClient("LED switched OFF");
          } else {
              btManager.sendToClient("Unknown command: " + command);
          }
      }
    }
    delay(10);
}
