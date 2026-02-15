#include <Arduino.h>

#include <avr/wdt.h>
#include "Config.h"
#include "HeaterController.h"
#include "TemperatureSensors.h"
#include "FrequencyGenerator.h"
#include "Bluetooth.h"
#include "Eeprom_utils.h"


// Глобальные переменные для таймеров
unsigned long lastTempUpdate = 0;
unsigned long lastControlUpdate = 0;
unsigned long lastBluetoothCheck = 0;

void setup() {
    // Отключаем Watchdog при старте
    wdt_disable();
    
    Serial.begin(9600);
    
    Serial.println(F("========================================="));
    Serial.println(F("Arduino Mega 2560 Controller v" VERSION));
    Serial.println(F("========================================="));
    
    // Инициализация всех модулей
    initBluetooth();
    initHeaters();
    initTemperatureSensors();
    initFrequencyGenerators();
    
    // Загружаем сохраненные настройки
    loadAllSettings();

    // Устанавливаем сохранённые частоты
    setGenerator1(gen1.frequency);
    setGenerator2(gen2.frequency);
    
    Serial.print(F("Loaded target temperature 1: "));
    Serial.println(heater1.targetTemp, 1);
    Serial.print(F("Loaded target temperature 2: "));
    Serial.println(heater2.targetTemp, 1);
    Serial.print(F("Loaded target frequency: "));
    
    // Включаем Watchdog на 4 секунды
    wdt_enable(WDTO_4S);
    Serial.println(F("Watchdog enabled (4s)"));
    
    // Первый запрос температур
    sensorReactor1.requestTemperatures();
    sensorReactor2.requestTemperatures();
    
    Serial.println(F("System ready!"));

    wdt_enable(WDTO_4S);
}

void loop() {
    wdt_reset();
    
    // Обработка Bluetooth
    processBluetooth();
    checkBluetoothConnection();
    
    // Обновление температуры
    if (millis() - lastTempUpdate >= TEMP_UPDATE_INTERVAL) {
        updateTemperatures(heater1, heater2);
        lastTempUpdate = millis();
    }
    
    // Управление нагревателями
    if (millis() - lastControlUpdate >= CONTROL_UPDATE_INTERVAL) {
        updateHeaterControl(heater1, TEMP_HYSTERESIS_LOW, TEMP_HYSTERESIS_HIGH);
        updateHeaterControl(heater2, TEMP_HYSTERESIS_LOW, TEMP_HYSTERESIS_HIGH);
        lastControlUpdate = millis();
    }
    
    // Проверка состояния нагревателей
    if (millis() - heater1.lastStateCheckTime >= HEATER_STATE_CHECK_INTERVAL) {
        verifyHeaterState(heater1, 1);
        heater1.lastStateCheckTime = millis();
    }
    
    if (millis() - heater2.lastStateCheckTime >= HEATER_STATE_CHECK_INTERVAL) {
        verifyHeaterState(heater2, 2);
        heater2.lastStateCheckTime = millis();
    }
    
    // Вывод статуса (каждые 5 секунд)
    static unsigned long lastPrintTime = 0;
    if (millis() - lastPrintTime >= 5000) {
        lastPrintTime = millis();
        
        Serial.println(F("------------------------------------------------"));
        Serial.print(F("V1 R:"));
        Serial.print(heater1.currentTemp, 1);
        Serial.print(F("°C T:"));
        Serial.print(heater1.targetTemp, 1);
        Serial.print(F("°C H:"));
        Serial.print(heater1.state ? F("ON ") : F("OFF"));
        Serial.print(F(" A:"));
        Serial.print(heater1.alarm ? F("YES ") : F("NO  "));
        Serial.print(F(" P:"));
        Serial.print(heater1.permission ? F("ON ") : F("OFF"));
        
        Serial.print(F(" | V2 R:"));
        Serial.print(heater2.currentTemp, 1);
        Serial.print(F("°C T:"));
        Serial.print(heater2.targetTemp, 1);
        Serial.print(F("°C H:"));
        Serial.print(heater2.state ? F("ON ") : F("OFF"));
        Serial.print(F(" A:"));
        Serial.print(heater2.alarm ? F("YES ") : F("NO  "));
        Serial.print(F(" P:"));
        Serial.print(heater2.permission ? F("ON ") : F("OFF"));
        
        Serial.print(F(" | FREQ1:"));
        Serial.print(gen1.frequency);
        Serial.print(gen1.active ? F("*") : F(" "));
        Serial.print(F("Hz FREQ2:"));
        Serial.print(gen2.frequency);
        Serial.print(gen2.active ? F("*") : F(" "));
        Serial.print(F("Hz"));
        
        Serial.print(F(" | BT:"));
        Serial.print(millis() - lastBluetoothCheck < 10000 ? F("OK ") : F("WARN"));
        Serial.println();
    }
    
    delay(10);
}