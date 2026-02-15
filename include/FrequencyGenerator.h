#ifndef FREQUENCY_H
#define FREQUENCY_H

#include <Arduino.h>
#include <TimerOne.h>
#include <TimerThree.h> 
#include <TimerFive.h>
#include "Config.h"

// Структура для генератора частоты
struct FrequencyGenerator {
    const int pin;
    const int timerId;  // 1 или 5
    const int maxFreq;
    int frequency;
    bool active;
    unsigned long lastChangeTime;
    
    // Конструктор с максимальной частотой
    FrequencyGenerator(int p, int timer, int maxF) : 
        pin(p), timerId(timer), maxFreq(maxF),
        frequency(0), active(false), lastChangeTime(0) {}
    
    void init();
    void setFreq(int freq);
    bool isValidFreq(int freq);
};

// Глобальные объекты
extern FrequencyGenerator gen1;  // Timer1 на пине 12
extern FrequencyGenerator gen2;  // Timer5 на пине 45

// Функции
void initFrequencyGenerators();
void setGenerator1(int freq);
void setGenerator2(int freq);
void saveFrequencies();
void loadFrequencies();

#endif // FREQUENCY_H