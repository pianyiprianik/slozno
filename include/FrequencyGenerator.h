#ifndef FREQUENCY_H
#define FREQUENCY_H

#include <Arduino.h>
#include <TimerOne.h>
#include "Config.h"

// Глобальная переменная целевой частоты
extern int targetFrequency;         //  переменная объявлена здесь, но определена (создана) в другом файле (frequency.cpp)
extern bool frequencyActive;        //  Флаг, показывающий, активен ли генератор частоты в данный момент

// Функции для управления частотой
void initFrequencyGenerator();          //  Инициализация генератора частоты
void setFrequency(int freq);            //  Установка новой частоты
bool updateFrequency(int newFreq);      //  Обновление частоты с проверкой изменений (дополнительная функция, которой у нас нет в коде)
void saveTargetFrequency();             //  Сохранение текущей частоты в EEPROM
void loadTargetFrequency();             //  Загрузка сохранённой частоты из EEPROM

#endif // FREQUENCY_H