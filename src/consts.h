#include "Arduino.h"

const String DeviceNameBLE = "UART-Test-LE";
const String DeviceNameBTC = "UART-Test-Classic";

const String LogPrefixBLE = "BLE: ";
const String LogPrefixBTClass = "BTClass: ";

// BLE: UUID для Nordic UART Service
#define SERVICE_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define RX_CHAR_UUID "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define TX_CHAR_UUID "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

