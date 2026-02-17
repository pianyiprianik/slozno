#ifndef TIMER_PIN_H
#define TIMER_PIN_H

#include <Arduino.h>
#include "Config.h"

// Структура для таймерного пина
struct TimerPin {
    const int pin;
    unsigned int lowSeconds;      // V50
    unsigned int highSeconds;     // V51
    bool enabled;                 // V52
    bool currentState;            
    unsigned long lastToggleTime; 
    bool initialized;
    
    // Конструктор с защитой - сразу устанавливаем значения по умолчанию
    TimerPin(int p) : pin(p), 
                      lowSeconds(DEFAULT_LOW_SECONDS), 
                      highSeconds(DEFAULT_HIGH_SECONDS), 
                      enabled(false), 
                      currentState(false), 
                      lastToggleTime(0), 
                      initialized(false) {}
    
    void init() {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
        currentState = false;
        initialized = true;
        Serial.print(F("Timer pin initialized on "));
        Serial.println(pin);
    }
    
    // Безопасная установка LOW секунд
    bool setLowSeconds(unsigned int sec) {
        if (sec >= MIN_TIMER_SECONDS && sec <= MAX_TIMER_SECONDS) {
            if (lowSeconds != sec) {
                lowSeconds = sec;
                Serial.print(F("Timer LOW seconds set to: "));
                Serial.println(sec);
                return true;
            }
        } else {
            Serial.print(F("ERROR: LOW seconds "));
            Serial.print(sec);
            Serial.print(F(" out of range ("));
            Serial.print(MIN_TIMER_SECONDS);
            Serial.print(F("-"));
            Serial.print(MAX_TIMER_SECONDS);
            Serial.println(F(")"));
        }
        return false;
    }
    
    // Безопасная установка HIGH секунд
    bool setHighSeconds(unsigned int sec) {
        if (sec >= MIN_TIMER_SECONDS && sec <= MAX_TIMER_SECONDS) {
            if (highSeconds != sec) {
                highSeconds = sec;
                Serial.print(F("Timer HIGH seconds set to: "));
                Serial.println(sec);
                return true;
            }
        } else {
            Serial.print(F("ERROR: HIGH seconds out of range"));
        }
        return false;
    }
    
    // Безопасное включение/выключение
    void setEnabled(bool en) {
        if (enabled != en) {
            enabled = en;
            if (enabled) {
                currentState = false;
                digitalWrite(pin, LOW);
                lastToggleTime = millis();
                Serial.println(F("Timer enabled"));
            } else {
                currentState = false;
                digitalWrite(pin, LOW);
                Serial.println(F("Timer disabled"));
            }
        }
    }
    
    void update() {
        if (!initialized || !enabled) return;
        
        unsigned long currentMillis = millis();
        unsigned long elapsedSeconds = (currentMillis - lastToggleTime) / 1000;
        
        if (currentState == false) {
            if (elapsedSeconds >= lowSeconds) {
                currentState = true;
                digitalWrite(pin, HIGH);
                lastToggleTime = currentMillis;
            }
        } else {
            if (elapsedSeconds >= highSeconds) {
                currentState = false;
                digitalWrite(pin, LOW);
                lastToggleTime = currentMillis;
            }
        }
    }
    
    bool getState() { return currentState; }
};

extern TimerPin timerPin;

#endif