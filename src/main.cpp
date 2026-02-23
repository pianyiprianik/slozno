#include <Arduino.h>

#include <avr/wdt.h>
#include "Config.h"
#include "HeaterController.h"
#include "TemperatureSensors.h"
#include "FrequencyGenerator.h"
#include "Bluetooth.h"
#include "Eeprom_utils.h"
#include "Aux_control.h"
#include "veml6075.h"
#include "Timer_pin.h"

// Глобальные переменные для таймеров
unsigned long lastTempUpdate = 0;
unsigned long lastControlUpdate = 0;
unsigned long lastBluetoothCheck = 0;
unsigned long lastV30ToggleTime = 0;
unsigned long lastUVUpdate = 0;
bool lastV30State = false;
const unsigned long UV_UPDATE_INTERVAL = 2000;

// Для отсчёта времени (из bluetooth)
extern unsigned int v30TimerInterval;
extern void saveAllSettings();

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
    aux1.init();
    aux2.init();
    initVEML6075();
    timerPin.init();


    // Загружаем сохраненные настройки
    loadAllSettings();

    // Устанавливаем сохранённые частоты
    setGenerator1(gen1.targetFrequency);
    setGenerator2(gen2.targetFrequency);
    
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

    // ===== ТАЙМЕР ДЛЯ V30 =====
    // Проверяем каждую секунду (чтобы не нагружать процессор)
    static unsigned long lastTimerCheck = 0;
    static unsigned long lastSaveTime = 0;
    const unsigned long SAVE_INTERVAL = 60000; // Сохранять каждую минуту

    if (millis() - lastSaveTime >= SAVE_INTERVAL) {
        saveAllSettings();
        lastSaveTime = millis();
    }

    if (millis() - lastTimerCheck >= 1000) {
        lastTimerCheck = millis();
        
        // Переводим интервал из секунд в миллисекунды
        unsigned long intervalMs = v30TimerInterval * 1000UL;
        
        if (millis() - lastV30ToggleTime >= intervalMs) {
            lastV30ToggleTime = millis();
            
            // Инвертируем текущее состояние
            bool newState = !aux1.getState();
            //bool newState = !aux2.getState();
            
            // Устанавливаем новое состояние
            aux1.setTarget(newState);
            //aux2.setTarget(newState);
            
            // Сбрасываем второй генератор
            if (gen2.targetFrequency > 0) {
                gen2.reset();
                Serial.println(F("Generator 2 reset due to V30 timer"));
            }
            
            Serial.print(F("V30 timer (interval: "));
            Serial.print(v30TimerInterval);
            Serial.print(F("s): set to "));
            Serial.println(newState ? F("ON") : F("OFF"));
        }
    }
    
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

    // Обновление всех генераторов (плавный пуск)
    updateAllGenerators();
    
    // Обновление дополнительного пина
    aux1.update();
    aux2.update();
    timerPin.update();

    // Обновление UV датчика (добавить в loop)
    if (millis() - lastUVUpdate >= UV_UPDATE_INTERVAL) {
        updateVEML6075();
        lastUVUpdate = millis();
    }

    // Автосохранение настроек (раз в минуту)
    //static unsigned long lastSaveTime = 0;  // Перенести сюда
    if (millis() - lastSaveTime >= 60000) {
        saveAllSettings();
        lastSaveTime = millis();
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
        Serial.print(gen1.currentFrequency);
        Serial.print(gen1.active ? F("*") : F(" "));
        Serial.print(F("Hz FREQ2:"));
        Serial.print(gen2.currentFrequency);
        Serial.print(gen2.active ? F("*") : F(" "));
        Serial.print(F("Hz"));

        Serial.print(F(" | AUX1:"));
        Serial.print(aux1.getState() ? F("ON ") : F("OFF"));
        Serial.print(F(" | AUX2:"));
        Serial.print(aux2.getState() ? F("ON ") : F("OFF"));
        
        Serial.print(F(" | BT:"));
        Serial.print(millis() - lastBluetoothCheck < 10000 ? F("OK ") : F("WARN"));
        Serial.println();

        if (uvData.sensorOK) {
            Serial.print(F(" | UVB:"));
            Serial.print(uvData.uvb, 2);
            Serial.print(F("/"));
            Serial.print(uvData.uvbThreshold, 2);
            Serial.print(F(" "));
            Serial.print(uvData.comparatorState ? F("PIN43:ON") : F("PIN43:OFF"));
        }
        
        if (timerPin.enabled) {
            Serial.print(F(" | TIMER:"));
            Serial.print(timerPin.getState() ? F("HIGH ") : F("LOW  "));
            Serial.print(timerPin.lowSeconds);
            Serial.print(F("/"));
            Serial.print(timerPin.highSeconds);
            Serial.print(F("s"));
        } else {
            Serial.print(F(" | TIMER:OFF"));
        }

        Serial.print(F(" | EXT1:"));
        if (extraData1.valid) {
            Serial.print(extraData1.filteredValue, 1);
            Serial.print(F("°C"));
            if (extraData1.errorCount > 0) Serial.print(F("*"));  // * если были ошибки
        } else {
            Serial.print(F("OFF"));
        }
    
        Serial.print(F(" EXT2:"));
        if (extraData2.valid) {
            Serial.print(extraData2.filteredValue, 1);
            Serial.print(F("°C"));
            if (extraData2.errorCount > 0) Serial.print(F("*"));
        } else {
            Serial.print(F("OFF"));
        }

    }
    
    delay(10);
}