#include "TemperatureSensors.h"
#include <Arduino.h>

// Создаем объекты для работы с датчиками
OneWire oneWireReactor1(ONE_WIRE_BUS_REACTOR_1);
OneWire oneWireReactor2(ONE_WIRE_BUS_REACTOR_2);
DallasTemperature sensorReactor1(&oneWireReactor1);
DallasTemperature sensorReactor2(&oneWireReactor2);

// Флаги для асинхронного чтения
static bool conversionStarted1 = false;
static bool conversionStarted2 = false;
static unsigned long conversionStartTime = 0;

// Инициализация датчиков
void initTemperatureSensors() {
    sensorReactor1.begin();
    sensorReactor1.setResolution(12);
    sensorReactor1.setWaitForConversion(false);
    
    sensorReactor2.begin();
    sensorReactor2.setResolution(12);
    sensorReactor2.setWaitForConversion(false);
    
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
}

// Проверка валидности температуры
bool isValidTemperature(float temp) {
    return (temp != DEVICE_DISCONNECTED_C && 
            temp != -127.0 && 
            temp > -50 && 
            temp < 100);
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
            heater1.currentTemp = -999.9;
            heater1.alarm = true;
            Serial.println(F("ALARM: Reactor 1 sensor disconnected or faulty!"));
        }
        
        // Читаем температуру 2
        float newTemp2 = sensorReactor2.getTempCByIndex(0);
        if (isValidTemperature(newTemp2)) {
            heater2.currentTemp = newTemp2;
            heater2.alarm = false;
        } else {
            heater2.currentTemp = -999.9;
            heater2.alarm = true;
            Serial.println(F("ALARM: Reactor 2 sensor disconnected or faulty!"));
        }
        
        conversionStarted1 = false;
        conversionStarted2 = false;
    }
}