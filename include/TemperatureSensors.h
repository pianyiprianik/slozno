#ifndef TEMPERATURE_H
#define TEMPERATURE_H

#include <OneWire.h>
#include <DallasTemperature.h>
#include "Config.h"
#include "HeaterController.h"

// Структура для хранения состояния датчика с защитой
struct TempSensorData {
    float rawValue;           // Сырое значение с датчика
    float filteredValue;      // Отфильтрованное значение (для отправки)
    bool valid;               // Валидность данных
    unsigned long lastGoodRead; // Время последнего хорошего чтения
    int errorCount;           // Счётчик ошибок подряд
    float lastGoodValue;      // Последнее хорошее значение
    
    TempSensorData() : rawValue(0.0), filteredValue(0.0), valid(false), 
                       lastGoodRead(0), errorCount(0), lastGoodValue(0.0) {}
    
    // Обновление с защитой
    void update(float newValue, unsigned long currentTime) {
        rawValue = newValue;
        
        // Проверяем валидность
        bool currentValid = isValidReading(newValue);
        
        if (currentValid) {
            // Хорошее чтение
            valid = true;
            filteredValue = newValue;
            lastGoodValue = newValue;
            lastGoodRead = currentTime;
            errorCount = 0;
        } else {
            // Плохое чтение
            errorCount++;
            
            // Если ошибок мало, используем последнее хорошее значение
            if (errorCount <= 3 && lastGoodValue > 0.1) {
                filteredValue = lastGoodValue;
                valid = true;  // Считаем валидным, но с пометкой
            } else {
                // Слишком много ошибок - помечаем как невалидное
                valid = false;
                filteredValue = -273.0; // Абсолютный ноль как маркер
            }
        }
    }
    
    // Проверка разумности показаний
    static bool isValidReading(float temp) {
        return (temp != DEVICE_DISCONNECTED_C && 
                temp != -127.0 && 
                temp > -50.0 &&    // Не ниже -50°C
                temp < 150.0);      // Не выше 150°C (для промышленных датчиков)
    }
    
    // Получение безопасного значения для отправки
    float getSafeValue() {
        if (valid && filteredValue > -50.0 && filteredValue < 150.0) {
            return filteredValue;
        }
        return 0.0;  // Возвращаем 0 вместо -999.9
    }
};

// Основные датчики (для управления)
extern OneWire oneWireReactor1;
extern OneWire oneWireReactor2;
extern DallasTemperature sensorReactor1;
extern DallasTemperature sensorReactor2;

// ДОПОЛНИТЕЛЬНЫЕ датчики (только мониторинг)
extern OneWire oneWireExtra1;
extern OneWire oneWireExtra2;
extern DallasTemperature sensorExtra1;
extern DallasTemperature sensorExtra2;

// Данные дополнительных датчиков с защитой
extern TempSensorData extraData1;  // V60 - датчик 3
extern TempSensorData extraData2;  // V61 - датчик 4

// Функции для работы с температурой
void initTemperatureSensors();
void requestTemperatures();
void updateTemperatures(Heater &heater1, Heater &heater2);
void updateExtraTemperatures();
bool isValidTemperature(float temp);

// Функции для безопасного получения значений
float getExtraTemp1();  // Возвращает 0 вместо мусора
float getExtraTemp2();

#endif // TEMPERATURE_H