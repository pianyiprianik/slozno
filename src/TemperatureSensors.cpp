#include "TemperatureSensors.h"
#include "HeaterController.h"
#include <Arduino.h>

// Создаем объекты для работы с датчиками
OneWire oneWireReactor1(ONE_WIRE_BUS_REACTOR_1);
OneWire oneWireReactor2(ONE_WIRE_BUS_REACTOR_2);

DallasTemperature sensorReactor1(&oneWireReactor1);
DallasTemperature sensorReactor2(&oneWireReactor2);

OneWire oneWireExtra1(ONE_WIRE_BUS_EXTRA_1);
OneWire oneWireExtra2(ONE_WIRE_BUS_EXTRA_2);
DallasTemperature sensorExtra1(&oneWireExtra1);
DallasTemperature sensorExtra2(&oneWireExtra2);

// Температуры дополнительных датчиков
float extraTemp1 = 0.00;
float extraTemp2 = 0.00;

// Флаги для асинхронного чтения
static bool conversionStarted1 = false;
static bool conversionStarted2 = false;
static unsigned long conversionStartTime = 0;

// Флаги для дополнительных датчиков
static bool extraConversionStarted1 = false;
static bool extraConversionStarted2 = false;

// Инициализация датчиков
void initTemperatureSensors() {
    sensorReactor1.begin();
    sensorReactor1.setResolution(12);
    sensorReactor1.setWaitForConversion(false);
    
    sensorReactor2.begin();
    sensorReactor2.setResolution(12);
    sensorReactor2.setWaitForConversion(false);

    sensorExtra1.begin();
    sensorExtra1.setResolution(12);
    sensorExtra1.setWaitForConversion(false);
    
    sensorExtra2.begin();
    sensorExtra2.setResolution(12);
    sensorExtra2.setWaitForConversion(false);
    
    Serial.println(F("Temperature sensors initialized"));
}

// Запрос температур (асинхронный)
void requestTemperatures() {
    if (!conversionStarted1) {
        sensorReactor1.requestTemperatures();
        conversionStarted1 = true;
        conversionStartTime = millis();
    }
    
    if (!conversionStarted2) {
        sensorReactor2.requestTemperatures();
        conversionStarted2 = true;
    }

    // Дополнительные датчики (запускаем асинхронно)
    if (!extraConversionStarted1) {
        sensorExtra1.requestTemperatures();
        extraConversionStarted1 = true;
    }
    
    if (!extraConversionStarted2) {
        sensorExtra2.requestTemperatures();
        extraConversionStarted2 = true;
    }
}

// Проверка валидности температуры
bool isValidTemperature(float temp) {
    return (temp != DEVICE_DISCONNECTED_C && 
            temp != -127.0 && 
            temp > -50 && 
            temp < 100);
}

// Обновление только дополнительных датчиков
void updateExtraTemperatures() {
    // Проверяем, завершилось ли преобразование для дополнительных датчиков
    if (extraConversionStarted1 && extraConversionStarted2 && 
        millis() - conversionStartTime > 750) {
        
        // Читаем дополнительный датчик 1
        float newTemp1 = sensorExtra1.getTempCByIndex(0);
        if (isValidTemperature(newTemp1)) {
            extraTemp1 = newTemp1;
        } else {
            extraTemp1 = -999.99;
        }
        
        // Читаем дополнительный датчик 2
        float newTemp2 = sensorExtra2.getTempCByIndex(0);
        if (isValidTemperature(newTemp2)) {
            extraTemp2 = newTemp2;
        } else {
            extraTemp2 = -999.99;
        }
        
        // Сбрасываем флаги
        extraConversionStarted1 = false;
        extraConversionStarted2 = false;
    }
}

// Обновление температур
void updateTemperatures(Heater &heater1, Heater &heater2) {
    requestTemperatures();
    
    // Проверяем, завершилось ли преобразование (750ms для 12-bit)
    if (conversionStarted1 && conversionStarted2 && 
        millis() - conversionStartTime > 750) {
        
        // Читаем температуру 1
        float newTemp1 = sensorReactor1.getTempCByIndex(0);
        if (isValidTemperature(newTemp1)) {
            heater1.currentTemp = newTemp1;
            heater1.alarm = false;
        } else {
            heater1.currentTemp = -999.99;
            heater1.alarm = true;
            Serial.println(F("ALARM: Reactor 1 sensor disconnected or faulty!"));
        }
        
        // Читаем температуру 2
        float newTemp2 = sensorReactor2.getTempCByIndex(0);
        if (isValidTemperature(newTemp2)) {
            heater2.currentTemp = newTemp2;
            heater2.alarm = false;
        } else {
            heater2.currentTemp = -999.99;
            heater2.alarm = true;
            Serial.println(F("ALARM: Reactor 2 sensor disconnected or faulty!"));
        }
        
        conversionStarted1 = false;
        conversionStarted2 = false;

        }
    
    // Обновляем дополнительные датчики
    updateExtraTemperatures();
}