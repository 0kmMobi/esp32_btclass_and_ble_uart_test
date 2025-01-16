#include <Arduino.h>
#include "BTManager.h"

  #define DEV_NODEMCU32S 1
  #define DEV_WEMOS_LOLIN32_LITE 2

  /////////////////////////////////
  #define DEV_MODEL DEV_NODEMCU32S
  ///////////////////////////////

  #if (DEV_MODEL == DEV_NODEMCU32S)
      #define LED_PIN  2 // LED_BUILTIN (nodemcu-32s)
      #define LED_PIN_INVERT false
  #elif (DEV_MODEL == DEV_WEMOS_LOLIN32_LITE)
      #define LED_PIN 22 // lolin32_lite (Wemos Lolin32 Lite)
      #define LED_PIN_INVERT true
  #else 
      #error "The device is not defined. Compilation stopped."
  #endif


  void setup() {
      Serial.begin(115200);
      pinMode(LED_PIN, OUTPUT);
      digitalWrite(LED_PIN, LED_PIN_INVERT?HIGH:LOW);

      btManager.begin(DeviceNameDefault);
      Serial.println("Setup complete.");
  }

  unsigned long notifyTimerStart = 0;
  uint32_t counter = 0;
  bool shouldSendCounter = true;

  void tryToSendCounter() {
    if(notifyTimerStart + 2000L <= millis()) {
      notifyTimerStart = millis();
      if(shouldSendCounter) {
        Serial.printf("Counter: %d\n", counter);
        btManager.print(String(counter));
        counter ++;
      } else {
        Serial.printf("Counter sending stopped");
      }
    }
  }

  void loop() {
      if(btManager.hasClient()) {
        tryToSendCounter();

        if (btManager.hasNextCommand()) {
            String command = btManager.getNextCommandLowCase();
            command.trim();
            Serial.println("Received command: " + command);

            if (command.equals("led_on")) {
                digitalWrite(LED_PIN, LED_PIN_INVERT?LOW:HIGH);
                String sLed = String("LED switched ON");
                btManager.print(sLed);
            } else if (command.equals("led_off")) {
                digitalWrite(LED_PIN, LED_PIN_INVERT?HIGH:LOW);
                btManager.print("LED switched OFF");
            } else if (command.equals("reset")) {
                counter = 0;
                btManager.print("Counter reset");
            } else if (command.equals("stop")) {
                shouldSendCounter = false;
                btManager.print("Counter sending stopped");
            } else if (command.equals("start")) {
                shouldSendCounter = true;
                btManager.print("Counter sending STARTED");
            } else {
                btManager.print("Unknown command: " + command);
            }
        }
      }
      delay(10);
  }
