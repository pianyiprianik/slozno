#ifndef FREQUENCY_H
#define FREQUENCY_H

#include <Arduino.h>
#include <TimerOne.h>
#include <TimerThree.h> 
#include <TimerFive.h>
#include "Config.h"
#include "Aux_control.h"

// Структура для генератора частоты
struct FrequencyGenerator {
    const int pin;
    const int timerId;  // 1 или 5
    const int maxFreq;

    int targetFrequency;      // Целевая частота (из Virtuino)
    int currentFrequency;      // Текущая частота (плавно растёт)
    bool active;
    bool needReset;           // Флаг сброса при изменении aux
    unsigned long lastStepTime;
    unsigned long lastChangeTime;
    
    // Конструктор с максимальной частотой
    FrequencyGenerator(int p, int timer, int maxF) : 
        pin(p), timerId(timer), maxFreq(maxF),
        targetFrequency(0), currentFrequency(0), 
        active(false), needReset(false),
        lastStepTime(0), lastChangeTime(0) {}
    
    void init();
    void setTarget(int freq);           // Установка целевой частоты
    void update();                       // Плавное изменение (вызывать в loop)
    void reset();                         // Сброс до 1 Гц
    void applyFrequency(int freq);        // Физическая установка частоты
    bool isValidFreq(int freq);
};

// Глобальные объекты
extern FrequencyGenerator gen1;  // Timer1 на пине 12
extern FrequencyGenerator gen2;  // Timer5 на пине 45

// Функции
void initFrequencyGenerators();
void setGenerator1(int freq);
void setGenerator2(int freq);
void updateAllGenerators();
void saveFrequencies();
void loadFrequencies();

#endif // FREQUENCY_H