#include "TemperatureSensors.h"
#include "HeaterController.h"
#include <Arduino.h>

// Основные датчики
OneWire oneWireReactor1(ONE_WIRE_BUS_REACTOR_1);
OneWire oneWireReactor2(ONE_WIRE_BUS_REACTOR_2);
DallasTemperature sensorReactor1(&oneWireReactor1);
DallasTemperature sensorReactor2(&oneWireReactor2);

// ДОПОЛНИТЕЛЬНЫЕ датчики
OneWire oneWireExtra1(ONE_WIRE_BUS_EXTRA_1);
OneWire oneWireExtra2(ONE_WIRE_BUS_EXTRA_2);
DallasTemperature sensorExtra1(&oneWireExtra1);
DallasTemperature sensorExtra2(&oneWireExtra2);

// Данные дополнительных датчиков с защитой
TempSensorData extraData1;
TempSensorData extraData2;

// Флаги для асинхронного чтения
static bool conversionStarted1 = false;
static bool conversionStarted2 = false;
static bool extraConversionStarted1 = false;
static bool extraConversionStarted2 = false;
static unsigned long conversionStartTime = 0;

// Константа для времени конверсии
const unsigned long CONVERSION_TIME = 750; // 750ms для 12-bit

// Инициализация всех датчиков
void initTemperatureSensors() {
    // Основные датчики
    sensorReactor1.begin();
    sensorReactor1.setResolution(12);
    sensorReactor1.setWaitForConversion(false);
    
    sensorReactor2.begin();
    sensorReactor2.setResolution(12);
    sensorReactor2.setWaitForConversion(false);
    
    // Дополнительные датчики
    sensorExtra1.begin();
    sensorExtra1.setResolution(12);
    sensorExtra1.setWaitForConversion(false);
    
    sensorExtra2.begin();
    sensorExtra2.setResolution(12);
    sensorExtra2.setWaitForConversion(false);
    
    Serial.println(F("Temperature sensors initialized (2 main + 2 extra)"));
    
    // Инициализируем данные датчиков
    extraData1 = TempSensorData();
    extraData2 = TempSensorData();
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
    if (!extraConversionStarted1) {
        sensorExtra1.requestTemperatures();
        extraConversionStarted1 = true;
    }
    
    if (!extraConversionStarted2) {
        sensorExtra2.requestTemperatures();
        extraConversionStarted2 = true;
    }
}

// Обновление только дополнительных датчиков с защитой
void updateExtraTemperatures() {
    unsigned long now = millis();
    
    // Проверяем, завершилось ли преобразование
    if (extraConversionStarted1 && extraConversionStarted2 && 
        now - conversionStartTime > CONVERSION_TIME) {
        
        // Читаем дополнительный датчик 1
        float newTemp1 = sensorExtra1.getTempCByIndex(0);
        extraData1.update(newTemp1, now);
        
        // Читаем дополнительный датчик 2
        float newTemp2 = sensorExtra2.getTempCByIndex(0);
        extraData2.update(newTemp2, now);
        
        // Вывод отладки при ошибках
        if (!extraData1.valid && extraData1.errorCount > 3) {
            Serial.print(F("EXT1 sensor error: raw="));
            Serial.println(newTemp1);
        }
        if (!extraData2.valid && extraData2.errorCount > 3) {
            Serial.print(F("EXT2 sensor error: raw="));
            Serial.println(newTemp2);
        }
        
        // Сбрасываем флаги
        extraConversionStarted1 = false;
        extraConversionStarted2 = false;
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

// Безопасное получение значений для отправки в Virtuino
float getExtraTemp1() {
    return extraData1.getSafeValue();
}

float getExtraTemp2() {
    return extraData2.getSafeValue();
}