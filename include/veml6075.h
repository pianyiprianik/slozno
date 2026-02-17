#ifndef VEML6075_H
#define VEML6075_H

#include <Arduino.h>
#include <Adafruit_VEML6075.h>

// Структура для хранения данных с датчика
struct UVData {
    float uva;          // Значение UVA (V40)
    float uvb;          // Значение UVB (V41)
    float uvIndex;      // Индекс UVI (V42)
    float uvbThreshold;  // Уставка сравнения UVB (V44)
    bool comparatorState; // Состояние пина 43
    bool sensorOK;      // Флаг исправности датчика (V43)
    unsigned long lastReadTime;
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