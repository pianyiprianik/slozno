#include "TemperatureSensors.h"
#include "HeaterController.h"
#include <Arduino.h>

// Основные датчики
OneWire oneWireReactor1(ONE_WIRE_BUS_REACTOR_1);
OneWire oneWireReactor2(ONE_WIRE_BUS_REACTOR_2);
DallasTemperature sensorReactor1(&oneWireReactor1);
DallasTemperature sensorReactor2(&oneWireReactor2);

// ОБЩАЯ шина для дополнительных датчиков (пин 7)
OneWire oneWireExtra(ONE_WIRE_BUS_EXTRA);
DallasTemperature sensorExtra(&oneWireExtra);

// Данные дополнительных датчиков
TempSensorData extraData1;  // Индекс 0 на шине
TempSensorData extraData2;  // Индекс 1 на шине

// Флаги для асинхронного чтения
static bool mainSensor1Init = true;
static bool mainSensor2Init = true;

static bool conversionStarted1 = false;
static bool conversionStarted2 = false;
static bool extraConversionStarted = false;
static unsigned long conversionStartTime = 0;
static unsigned long lastRecoveryTime = 0;
static int lastDeviceCount = 0;

// Константа для времени конверсии
const unsigned long CONVERSION_TIME = 750; // 750ms для 12-bit
const unsigned long RECOVERY_INTERVAL = 5000; // 5 секунд
const unsigned long MAIN_RECOVERY_INTERVAL = 10000; // 10 секунд

// Функция для вывода адресов датчиков (диагностика)
void printDeviceAddress(DeviceAddress deviceAddress) {
    for (uint8_t i = 0; i < 8; i++) {
        if (deviceAddress[i] < 16) Serial.print("0");
        Serial.print(deviceAddress[i], HEX);
    }
}

// Поиск и вывод всех устройств на шине
int scanExtraBus() {
    Serial.println(F("Scanning extra bus..."));
    
    DeviceAddress tempAddr;
    int count = 0;
    
    oneWireExtra.reset_search();
    while (oneWireExtra.search(tempAddr)) {
        if (OneWire::crc8(tempAddr, 7) == tempAddr[7]) {
            count++;
            Serial.print(F("  Device "));
            Serial.print(count);
            Serial.print(F(": "));
            printDeviceAddress(tempAddr);
            
            // Определяем тип датчика
            if (tempAddr[0] == 0x28) Serial.print(F(" (DS18B20)"));
            else if (tempAddr[0] == 0x10) Serial.print(F(" (DS18S20)"));
            else if (tempAddr[0] == 0x22) Serial.print(F(" (DS1822)"));
            Serial.println();
        }
    }
    
    Serial.print(F("Found "));
    Serial.print(count);
    Serial.println(F(" devices on extra bus"));
    
    return count;
}

// Инициализация всех датчиков
void initTemperatureSensors() {
    // Основные датчики
    sensorReactor1.begin();
    sensorReactor1.setResolution(12);
    sensorReactor1.setWaitForConversion(false);
    
    sensorReactor2.begin();
    sensorReactor2.setResolution(12);
    sensorReactor2.setWaitForConversion(false);
    
    // Дополнительные датчики на общей шине
    sensorExtra.begin();
    sensorExtra.setResolution(12);
    sensorExtra.setWaitForConversion(false);

    // Сканируем шину
    lastDeviceCount = scanExtraBus();
    
    if (lastDeviceCount >= 2) {
        extraData1.initialized = true;
        extraData2.initialized = true;
        Serial.println(F("Both extra sensors OK"));
    } else if (lastDeviceCount == 1) {
        extraData1.initialized = true;
        extraData2.initialized = false;
        Serial.println(F("Warning: Only one extra sensor found"));
    } else {
        extraData1.initialized = false;
        extraData2.initialized = false;
        Serial.println(F("Warning: No extra sensors found"));
    }
    
    Serial.println(F("Temperature sensors initialized"));
}

// Проверка и восстановление дополнительных датчиков
void checkAndRecoverExtraSensors() {
        if (millis() - lastRecoveryTime < RECOVERY_INTERVAL) return;
    
    lastRecoveryTime = millis();
    
    // Сканируем шину
    int currentCount = scanExtraBus();
    
    if (currentCount != lastDeviceCount) {
        Serial.print(F("Device count changed: "));
        Serial.print(lastDeviceCount);
        Serial.print(F(" -> "));
        Serial.println(currentCount);
        lastDeviceCount = currentCount;
    }
    
    // Обновляем состояние датчиков
    if (currentCount >= 2) {
        if (!extraData1.initialized || !extraData2.initialized) {
            Serial.println(F("Both extra sensors recovered!"));
            extraData1.initialized = true;
            extraData2.initialized = true;
            extraData1.errorCount = 0;
            extraData2.errorCount = 0;
        }
    } else if (currentCount == 1) {
        if (!extraData1.initialized) {
            Serial.println(F("First extra sensor recovered"));
            extraData1.initialized = true;
            extraData1.errorCount = 0;
        }
        extraData2.initialized = false;
    } else {
        extraData1.initialized = false;
        extraData2.initialized = false;
    }
}

// Запрос температур у всех датчиков
void requestTemperatures() {
    unsigned long now = millis();
    
    // Основные датчики
    if (!conversionStarted1) {
        sensorReactor1.requestTemperatures();
        conversionStarted1 = true;
        conversionStartTime = now;
    }
    
    if (!conversionStarted2) {
        sensorReactor2.requestTemperatures();
        conversionStarted2 = true;
    }
    
    // Дополнительные датчики
    if (!extraConversionStarted && (extraData1.initialized || extraData2.initialized)) {
        sensorExtra.requestTemperatures();
        extraConversionStarted = true;
    }
}

// Обновление только дополнительных датчиков с защитой
void updateExtraTemperatures() {
        unsigned long now = millis();
    
    if (!extraConversionStarted) return;
    
    if (now - conversionStartTime > CONVERSION_TIME) {
        
        if (extraData1.initialized) {
            float newTemp1 = sensorExtra.getTempCByIndex(0);
            extraData1.update(newTemp1, now);
            
            if (!extraData1.valid && extraData1.errorCount > 3) {
                Serial.print(F("EXT1 sensor error: raw="));
                Serial.print(newTemp1);
                Serial.print(F(", errors="));
                Serial.println(extraData1.errorCount);
            } else if (extraData1.valid) {
                // Отладка: показываем когда恢复正常
                static int lastValid1 = 0;
                if (lastValid1 == 0) {
                    Serial.print(F("EXT1 OK: "));
                    Serial.println(newTemp1);
                    lastValid1 = 1;
                }
            }
        }
        
        if (extraData2.initialized) {
            float newTemp2 = sensorExtra.getTempCByIndex(1);
            extraData2.update(newTemp2, now);
            
            if (!extraData2.valid && extraData2.errorCount > 3) {
                Serial.print(F("EXT2 sensor error: raw="));
                Serial.print(newTemp2);
                Serial.print(F(", errors="));
                Serial.println(extraData2.errorCount);
            } else if (extraData2.valid) {
                static int lastValid2 = 0;
                if (lastValid2 == 0) {
                    Serial.print(F("EXT2 OK: "));
                    Serial.println(newTemp2);
                    lastValid2 = 1;
                }
            }
        }
        
        extraConversionStarted = false;
    }
}

// Обновление основных температур
void updateTemperatures(Heater &heater1, Heater &heater2) {
    unsigned long now = millis();
    requestTemperatures();
    
    if (conversionStarted1 && conversionStarted2 && 
        now - conversionStartTime > CONVERSION_TIME) {
        
        // Читаем температуру 1
        float newTemp1 = sensorReactor1.getTempCByIndex(0);
        if (isValidTemperature(newTemp1)) {
            heater1.currentTemp = newTemp1;
            heater1.alarm = false;
        } else {
            heater1.currentTemp = -999.9;
            heater1.alarm = true;
            Serial.print(F("Main sensor 1 error: "));
            Serial.println(newTemp1);
        }
        
        // Читаем температуру 2
        float newTemp2 = sensorReactor2.getTempCByIndex(0);
        if (isValidTemperature(newTemp2)) {
            heater2.currentTemp = newTemp2;
            heater2.alarm = false;
        } else {
            heater2.currentTemp = -999.9;
            heater2.alarm = true;
            Serial.print(F("Main sensor 2 error: "));
            Serial.println(newTemp2);
        }
        
        conversionStarted1 = false;
        conversionStarted2 = false;
    }
    
    // Обновляем дополнительные датчики
    updateExtraTemperatures();
}

// Проверка валидности температуры
bool isValidTemperature(float temp) {
    return TempSensorData::isValidReading(temp);
}

float getExtraTemp1() {
    if (!extraData1.initialized) return 0.0;
    return extraData1.getSafeValue();
}

float getExtraTemp2() {
    if (!extraData2.initialized) return 0.0;
    return extraData2.getSafeValue();
}