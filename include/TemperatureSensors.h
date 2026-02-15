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

// Функции для работы с температурой
void initTemperatureSensors();
void requestTemperatures();
void updateTemperatures(Heater &heater1, Heater &heater2);
bool isValidTemperature(float temp);

#endif // TEMPERATURE_H