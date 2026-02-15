#ifndef EEPROM_UTILS_H
#define EEPROM_UTILS_H

#include <Arduino.h>
#include "Config.h"

// Функции для работы с EEPROM
void saveTargetTemps(float temp1, float temp2);         //  Сохраняет целевые температуры ёмкостей в EEPROM
void loadTargetTemps(float &temp1, float &temp2);       //  Загружает целевые температуры из EEPROM
void saveFrequencies();                             //  Сохраняет частоту генератора в EEPROM
void loadFrequencies();                             //  Загружает частоту генератора из EEPROM
void loadAllSettings();                                 //  Загружает ВСЕ настройки системы из EEPROM

#endif // EEPROM_UTILS_H