#ifndef TEMPERATURE_H
#define TEMPERATURE_H

#include <OneWire.h>
#include <DallasTemperature.h>
#include "Config.h"
#include "HeaterController.h"

// Глобальные объекты датчиков
extern OneWire oneWireReactor1;
extern OneWire oneWireReactor2;
extern DallasTemperature sensorReactor1;
extern DallasTemperature sensorReactor2;

extern OneWire oneWireExtra1;
extern OneWire oneWireExtra2;
extern DallasTemperature sensorExtra1;
extern DallasTemperature sensorExtra2;

// Температуры дополнительных датчиков
extern float extraTemp1;  // V60 - температура датчика 3
extern float extraTemp2;  // V61 - температура датчика 4

// Функции для работы с температурой
void initTemperatureSensors();
void requestTemperatures();
void updateTemperatures(Heater &heater1, Heater &heater2);
void updateExtraTemperatures();  // Новая функция для дополнительных датчиков
bool isValidTemperature(float temp);

#endif // TEMPERATURE_H