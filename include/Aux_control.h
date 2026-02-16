#ifndef AUX_CONTROL_H
#define AUX_CONTROL_H

#include <Arduino.h>
#include "Config.h"

struct AuxControl {
    const int pin;
    bool targetState;      // Целевое состояние из Virtuino
    bool currentState;      // Текущее физическое состояние
    bool initialized;       // Была ли инициализация
    
    AuxControl(int p) : pin(p), targetState(false), 
                        currentState(false), initialized(false) {}
    
    void init() {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
        currentState = false;
        initialized = true;
        Serial.print(F("Aux control initialized on pin "));
        Serial.println(pin);
    }
    
    void setTarget(bool state) {
        if (targetState != state) {
            targetState = state;
            Serial.print(F("Aux target changed to: "));
            Serial.println(targetState ? F("HIGH") : F("LOW"));
        }
    }
    
    void update() {
        if (!initialized) return;
        
        if (currentState != targetState) {
            digitalWrite(pin, targetState ? HIGH : LOW);
            currentState = targetState;
            Serial.print(F("Aux pin "));
            Serial.print(pin);
            Serial.println(targetState ? F(" ON") : F(" OFF"));
        }
    }
};

// Глобальный объект
extern AuxControl auxControl;

#endif // AUX_CONTROL_H