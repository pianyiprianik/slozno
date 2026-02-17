#ifndef VEML6075_H
#define VEML6075_H

#include <Arduino.h>
#include <Adafruit_VEML6075.h>
#include "Config.h"

// Структура для хранения данных с датчика
struct UVData {
    float uva;          
    float uvb;          
    float uvIndex;      
    float uvbThreshold;  
    bool comparatorState; 
    bool sensorOK;      
    unsigned long lastReadTime;
    
    // Конструктор с явной инициализацией всех полей
    UVData() : 
        uva(0.0), 
        uvb(0.0), 
        uvIndex(0.0), 
        uvbThreshold(DEFAULT_UVB_THRESHOLD), 
        comparatorState(false), 
        sensorOK(false), 
        lastReadTime(0) {}
    
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
};

// Глобальный объект сенсора
extern Adafruit_VEML6075 uvSensor;
extern UVData uvData;

// Функции для работы с датчиком
void initVEML6075();
void updateVEML6075();
void updateUVComparator();  // Новая функция для сравнения
bool isVEML6075Connected();
void setUVBThreshold(float threshold);  // Установка уставки

#endif // VEML6075_H