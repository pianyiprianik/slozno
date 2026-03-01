#ifndef EEPROM_UTILS_H
#define EEPROM_UTILS_H

#include <Arduino.h>
#include "Config.h"

#define EEPROM_TIMER_INTERVAL 60
#define EEPROM_TIMER_CHECKSUM 64

#define EEPROM_V34_VALUE 104
#define EEPROM_V34_MAGIC 108

#define EEPROM_V36_VALUE 116
#define EEPROM_V36_MAGIC 120
#define EEPROM_V37_VALUE 122
#define EEPROM_V37_MAGIC 126

// Функции для работы с EEPROM
void saveTargetTemps(float temp1, float temp2);         //  Сохраняет целевые температуры ёмкостей в EEPROM
void loadTargetTemps(float &temp1, float &temp2);       //  Загружает целевые температуры из EEPROM
void saveFrequencies();                             //  Сохраняет частоту генератора в EEPROM
void loadFrequencies();                             //  Загружает частоту генератора из EEPROM
void saveTimerInterval();
void loadTimerInterval();
void loadAllSettings();                                 //  Загружает ВСЕ настройки системы из EEPROM
void saveAllSettings();
void loadAllSettings();
#endif // EEPROM_UTILS_H