Демо-проект, реализующий текстовый протокол обмена данными между ESP32-устройством (сервер) и каким-либо клиентом (например, с приложением на Android).

Поддерживается как **Bluetooth Classic** (по уполчанию) или **BLE** (Включается по _#define USE_BLE_ в файле _BTManager.h_).

После подключения клиентского устройства, ESP32 начинает каждые 2 сек отправлять (_Notify_) значение инкрементируемого счетчика клиенту (функция _tryToSendCounter()_).

Для примера работы на BT-сервере реализована поддержка следующих команд:

- *LED_ON*/*LED_OFF* для управления включением встроенного в nodemcu плату светодиода. На такие команды BT-сервер отвечает клиенту текстом со статусом светодиода;

- *STOP*/*START* прерывает и вновь запускает периодическую отправку инкрементируемого счетчика клиенту;

- *RESET* сбрасывает значение инкрементируемого счетчика, отправляемого клиенту, в 0.

При получении иных команд BT-сервер ответчает текстом: "_Unknown command:_".

---------------------
Известные проблемы:
1. Для BLE нужна поддержка разбиения и сборки длинных сообщений, превышающих установленный MTU
