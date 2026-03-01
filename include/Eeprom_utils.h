#ifndef EEPROM_UTILS_H
#define EEPROM_UTILS_H

#include <Arduino.h>
#include "Config.h"

// ==================== СИСТЕМА ВЕРСИОНИРОВАНИЯ EEPROM ====================
#define EEPROM_VERSION 0x07        // Увеличивайте при изменении структуры
#define EEPROM_HEADER_ADDR 0        // Адрес заголовка

// ==================== АДРЕСА ДАННЫХ ====================
// Используем адреса из Config.h, но определяем новые
#ifndef EEPROM_TARGET_TEMP_1
#define EEPROM_TARGET_TEMP_1 4      // Должно совпадать с Config.h
#endif

#ifndef EEPROM_TEMP_CHECKSUM_1
#define EEPROM_TEMP_CHECKSUM_1 8
#endif

#ifndef EEPROM_TARGET_TEMP_2
#define EEPROM_TARGET_TEMP_2 20
#endif

#ifndef EEPROM_TEMP_CHECKSUM_2
#define EEPROM_TEMP_CHECKSUM_2 24
#endif

#ifndef EEPROM_TARGET_FREQUENCY
#define EEPROM_TARGET_FREQUENCY 40
#endif

#ifndef EEPROM_FREQ_CHECKSUM
#define EEPROM_FREQ_CHECKSUM 44
#endif

#ifndef EEPROM_TARGET_FREQUENCY_2
#define EEPROM_TARGET_FREQUENCY_2 50
#endif

#ifndef EEPROM_FREQ2_CHECKSUM
#define EEPROM_FREQ2_CHECKSUM 54
#endif

// Новые адреса для третьего генератора
#ifndef EEPROM_FREQ3
#define EEPROM_FREQ3 60
#endif

#ifndef EEPROM_FREQ3_CHECKSUM
#define EEPROM_FREQ3_CHECKSUM 64
#endif

// UV датчик
#ifndef EEPROM_V44_VALUE
#define EEPROM_V44_VALUE 70
#endif

#ifndef EEPROM_V44_MAGIC
#define EEPROM_V44_MAGIC 74
#endif

// Таймерный пин
#ifndef EEPROM_V50_VALUE
#define EEPROM_V50_VALUE 80
#endif

#ifndef EEPROM_V50_MAGIC
#define EEPROM_V50_MAGIC 84
#endif

#ifndef EEPROM_V51_VALUE
#define EEPROM_V51_VALUE 86
#endif

#ifndef EEPROM_V51_MAGIC
#define EEPROM_V51_MAGIC 90
#endif

#ifndef EEPROM_V52_VALUE
#define EEPROM_V52_VALUE 92
#endif

#ifndef EEPROM_V52_MAGIC
#define EEPROM_V52_MAGIC 96
#endif

// AUX пины
#ifndef EEPROM_V32_VALUE
#define EEPROM_V32_VALUE 100  // AUX3
#endif

#ifndef EEPROM_V32_MAGIC
#define EEPROM_V32_MAGIC 104
#endif

#ifndef EEPROM_V34_VALUE
#define EEPROM_V34_VALUE 106  // AUX3 (старый, для совместимости)
#endif

#ifndef EEPROM_V34_MAGIC
#define EEPROM_V34_MAGIC 110
#endif

#ifndef EEPROM_V36_VALUE
#define EEPROM_V36_VALUE 112  // AUX4
#endif

#ifndef EEPROM_V36_MAGIC
#define EEPROM_V36_MAGIC 116
#endif

#ifndef EEPROM_V37_VALUE
#define EEPROM_V37_VALUE 118  // AUX5
#endif

#ifndef EEPROM_V37_MAGIC
#define EEPROM_V37_MAGIC 122
#endif

// Таймер V30
#ifndef EEPROM_V30_INTERVAL
#define EEPROM_V30_INTERVAL 124
#endif

#ifndef EEPROM_V30_CHECKSUM
#define EEPROM_V30_CHECKSUM 128
#endif

// ==================== ФУНКЦИИ ====================
bool checkEEPROM();                                   // Проверка EEPROM без форматирования
void formatEEPROM();                                   // Принудительное форматирование
void setDefaultsInMemory();                            // Установка значений по умолчанию в RAM

void saveTargetTemps(float temp1, float temp2);     
void loadTargetTemps(float &temp1, float &temp2);   

void saveFrequencies();                             
void loadFrequencies();     

void saveTimerInterval();
void loadTimerInterval();

void saveAllSettings();                             
void loadAllSettings();
void validateAndFixSettings();                       // Проверка всех настроек

#endif // EEPROM_UTILS_H