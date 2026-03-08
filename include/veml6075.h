#ifndef VEML6075_H
#define VEML6075_H

#include <Arduino.h>
#include <Adafruit_VEML6075.h>
#include "Config.h"

// Структура для хранения данных с датчика с защитой
struct UVData {
    float uva;          
    float uvb;          
    float uvIndex;      
    float uvbThreshold;  
    bool comparatorState; 
    bool sensorOK;      
    unsigned long lastReadTime;
    
    // Добавляем поля для защиты от залипания
    float lastGoodUVA;
    float lastGoodUVB;
    float lastGoodUVI;
    int errorCount;
    unsigned long lastGoodReadTime;
    bool valid;  // Флаг валидности текущих данных
    
    // Конструктор
    UVData() : 
        uva(0.0), uvb(0.0), uvIndex(0.0), 
        uvbThreshold(DEFAULT_UVB_THRESHOLD), 
        comparatorState(false), sensorOK(false), lastReadTime(0),
        lastGoodUVA(0.0), lastGoodUVB(0.0), lastGoodUVI(0.0),
        errorCount(0), lastGoodReadTime(0), valid(false) {}
    
    // Проверка валидности показаний UV датчика
    static bool isValidUVReading(float uva, float uvb, float uvi) {
        // UV датчик может показывать 0, если нет излучения
        // Но отрицательные значения - это ошибка
        return (uva >= 0 && uva < 2000 &&  // Разумный максимум UVA
                uvb >= 0 && uvb < 2000 &&  // Разумный максимум UVB
                uvi >= 0 && uvi < 20);     // UV индекс обычно до 11-12
    }
    
    // Обновление с защитой
    void update(float newUVA, float newUVB, float newUVI, unsigned long currentTime) {
        bool currentValid = isValidUVReading(newUVA, newUVB, newUVI);
        
        if (currentValid) {
            // Хорошее чтение
            uva = newUVA;
            uvb = newUVB;
            uvIndex = newUVI;
            lastGoodUVA = newUVA;
            lastGoodUVB = newUVB;
            lastGoodUVI = newUVI;
            lastGoodReadTime = currentTime;
            errorCount = 0;
            valid = true;
            sensorOK = true;
        } else {
            // Плохое чтение
            errorCount++;
            
            if (errorCount <= 3 && lastGoodReadTime > 0) {
                // Используем последнее хорошее значение (до 3 ошибок подряд)
                uva = lastGoodUVA;
                uvb = lastGoodUVB;
                uvIndex = lastGoodUVI;
                valid = true;  // Считаем валидным, но с пометкой
            } else {
                // Слишком много ошибок - помечаем как невалидное
                uva = 0;
                uvb = 0;
                uvIndex = 0;
                valid = false;
                sensorOK = false;
            }
        }
        lastReadTime = currentTime;
    }
    
    // Безопасное обновление уставки
    bool setThreshold(float newThreshold) {
        if (newThreshold >= MIN_UVB_THRESHOLD && newThreshold <= MAX_UVB_THRESHOLD) {
            if (abs(uvbThreshold - newThreshold) > 0.01) {
                uvbThreshold = newThreshold;
                return true;
            }
        }
        return false;
    }
    
    // Получение безопасных значений
    float getUVA() { return valid ? uva : 0; }
    float getUVB() { return valid ? uvb : 0; }
    float getUVI() { return valid ? uvIndex : 0; }
};

// Глобальный объект сенсора
extern Adafruit_VEML6075 uvSensor;
extern UVData uvData;

// Функции для работы с датчиком
void initVEML6075();
void updateVEML6075();
void updateUVComparator();
bool isVEML6075Connected();
void setUVBThreshold(float threshold);
void resetUVSensor();  // Новая функция для принудительного сброса

#endif // VEML6075_H