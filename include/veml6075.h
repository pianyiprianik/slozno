#ifndef VEML6075_H
#define VEML6075_H

#include <Arduino.h>
#include <Adafruit_VEML6075.h>

// Структура для хранения данных с датчика
struct UVData {
    float uva;          // Значение UVA
    float uvb;          // Значение UVB
    float uvIndex;      // Индекс UVI
    bool sensorOK;      // Флаг исправности датчика
    unsigned long lastReadTime; // Время последнего успешного чтения
};

// Глобальный объект сенсора
extern Adafruit_VEML6075 uvSensor;
extern UVData uvData;

// Функции для работы с датчиком
void initVEML6075();
void updateVEML6075();
bool isVEML6075Connected();

#endif // VEML6075_H