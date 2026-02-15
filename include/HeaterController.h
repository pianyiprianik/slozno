#ifndef HEATERS_H
#define HEATERS_H

#include <Arduino.h>
#include "Config.h"

// Структура для хранения состояния одного ТЭНа
struct Heater {
    const int pin;      // Пин управления
    float currentTemp;  // Текущая температура
    float targetTemp;   // Целевая температура
    
    bool state;         // Состояние (вкл/выкл)
    bool alarm;         // Авария
    bool permission;    // Разрешение на нагрев
    bool enabled;       // Разрешение от системы
    
    unsigned long lastSwitchTime;       // Время последнего переключения
    unsigned long lastStateCheckTime;   // Время последней проверки
    
    // Конструктор
    Heater(int p) : pin(p), currentTemp(0.0), targetTemp(30.0), 
                    state(false), alarm(false), permission(true),
                    enabled(false), lastSwitchTime(0), lastStateCheckTime(0) {}
    
    // Инициализация пина
    void init() {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
    }
};

// Глобальные объекты (объявляем как extern)
extern Heater heater1;
extern Heater heater2;

// Функции управления
void initHeaters();
void updateHeaterControl(Heater &heater, float lowThreshold, float highThreshold);
void setHeater(Heater &heater, bool newState, int heaterNumber);
void verifyHeaterState(Heater &heater, int heaterNumber);
void safeDigitalWrite(int pin, bool state, int heaterNumber);

#endif // HEATERS_H