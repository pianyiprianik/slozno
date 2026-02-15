#include "HeaterController.h"
#include <Arduino.h>

// Создаем объекты ТЭНов
Heater heater1(HEATER_PIN_1);
Heater heater2(HEATER_PIN_2);

// Инициализация всех ТЭНов
void initHeaters() {
    heater1.init();
    heater2.init();
    Serial.println(F("Heaters initialized"));
}

// Обновление управления ТЭНом
void updateHeaterControl(Heater &heater, float lowThreshold, float highThreshold) {
    bool shouldHeat = false;
    
    if (!heater.alarm && heater.permission && heater.currentTemp > -50) {
        if (heater.currentTemp < (heater.targetTemp - highThreshold)) {
            shouldHeat = true;
        } else if (heater.currentTemp > (heater.targetTemp - lowThreshold)) {
            shouldHeat = false;
        } else {
            shouldHeat = heater.state;
        }
    }
    
    if (heater.alarm || !heater.permission || heater.currentTemp < -50) {
        shouldHeat = false;
    }
    
    if (shouldHeat != heater.state) {
        if (millis() - heater.lastSwitchTime >= HEATER_SWITCH_DELAY) {
            setHeater(heater, shouldHeat, 
                     (&heater == &heater1) ? 1 : 2);
            heater.lastSwitchTime = millis();
        }
    }
}

// Установка состояния ТЭНа
void setHeater(Heater &heater, bool newState, int heaterNumber) {
    if (heater.state != newState) {
        heater.state = newState;
        safeDigitalWrite(heater.pin, heater.state, heaterNumber);
        
        Serial.print(F("Heater "));
        Serial.print(heaterNumber);
        Serial.println(heater.state ? F(" ON") : F(" OFF"));
    }
}

// Проверка соответствия физического состояния
void verifyHeaterState(Heater &heater, int heaterNumber) {
    bool physicalState = digitalRead(heater.pin);
    bool expectedState = heater.state ? HIGH : LOW;
    
    if (physicalState != expectedState) {
        Serial.print(F("WARNING: Heater "));
        Serial.print(heaterNumber);
        Serial.print(F(" state mismatch! Expected: "));
        Serial.print(heater.state ? F("ON") : F("OFF"));
        Serial.print(F(", Actual: "));
        Serial.println(physicalState == HIGH ? F("ON") : F("OFF"));
        
        digitalWrite(heater.pin, expectedState);
        Serial.println(F("Heater state corrected"));
    }
}

// Безопасная запись с верификацией
void safeDigitalWrite(int pin, bool state, int heaterNumber) {
    digitalWrite(pin, state ? HIGH : LOW);
    delay(1);
    
    bool readState = digitalRead(pin);
    if (readState != (state ? HIGH : LOW)) {
        Serial.print(F("ERROR: Heater "));
        Serial.print(heaterNumber);
        Serial.println(F(" pin state verification failed!"));
        
        digitalWrite(pin, state ? HIGH : LOW);
        delay(1);
    }
}