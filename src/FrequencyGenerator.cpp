#include "FrequencyGenerator.h"
#include <Arduino.h>
#include <EEPROM.h>

// Глобальные переменные
int targetFrequency = 0;
bool frequencyActive = false;
static unsigned long lastFrequencyChangeTime = 0;

// Инициализация генератора частоты
void initFrequencyGenerator() {
    pinMode(FREQ_PIN, OUTPUT);
    digitalWrite(FREQ_PIN, LOW);
    
    Timer1.initialize(1000);  // Начинаем с 1 мс = 1000 Гц
    Timer1.pwm(FREQ_PIN, 0);   // 0% заполнения (сигнала нет)
    
    Serial.print(F("Frequency generator initialized on pin "));
    Serial.println(FREQ_PIN);
}

// Установка частоты
void setFrequency(int freq) {
    if (freq == 0) {
        Timer1.setPwmDuty(FREQ_PIN, 0);
        digitalWrite(FREQ_PIN, LOW);
        frequencyActive = false;
        Serial.println(F("Frequency generator: OFF"));
    } else {
        float period = 1000000.0 / freq; // Период в микросекундах
        Timer1.setPeriod(period);
        Timer1.setPwmDuty(FREQ_PIN, PWM_DUTY_CYCLE); // 50% заполнения
        frequencyActive = true;
        
        Serial.print(F("Frequency generator set to: "));
        Serial.print(freq);
        Serial.println(F(" Hz"));
    }
}

// Сохранение частоты в EEPROM
void saveTargetFrequency() {
    EEPROM.put(EEPROM_TARGET_FREQUENCY, targetFrequency);
    int checksum = targetFrequency + EEPROM_MAGIC_NUMBER;
    EEPROM.put(EEPROM_FREQ_CHECKSUM, checksum);
}

// Загрузка частоты из EEPROM
void loadTargetFrequency() {
    int loadedFreq;
    int savedChecksum;
    
    EEPROM.get(EEPROM_TARGET_FREQUENCY, loadedFreq);
    EEPROM.get(EEPROM_FREQ_CHECKSUM, savedChecksum);
    int calculatedChecksum = loadedFreq + EEPROM_MAGIC_NUMBER;
    
    if (calculatedChecksum == savedChecksum && loadedFreq >= 0 && loadedFreq <= 1000) {
        targetFrequency = loadedFreq;
    } else {
        targetFrequency = 0; // По умолчанию выключено
    }
}

// безопасное обновление частоты с проверкой времени
bool updateFrequency(int newFreq) {
    if (newFreq < 0 || newFreq > 1000) return false;
    
    if (newFreq != targetFrequency) {
        if (millis() - lastFrequencyChangeTime >= FREQUENCY_CHANGE_DELAY) {
            targetFrequency = newFreq;
            setFrequency(targetFrequency);
            saveTargetFrequency();
            lastFrequencyChangeTime = millis();
            return true;
        }
    }
    return false;
}