#include "FrequencyGenerator.h"
#include <Arduino.h>
#include <EEPROM.h>

// Создаём генераторы
FrequencyGenerator gen1(FREQ1_PIN, 1, FREQ1_MAX);
FrequencyGenerator gen2(FREQ2_PIN, 5, FREQ2_MAX);

// Адреса в EEPROM для второго генератора
#define EEPROM_FREQ2 50
#define EEPROM_FREQ2_CHECKSUM 54

// Проверка допустимости частоты
bool FrequencyGenerator::isValidFreq(int freq) {
    return (freq >= 0 && freq <= maxFreq);
}

// Инициализация конкретного генератора
void FrequencyGenerator::init() {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    
    switch(timerId) {
        case 1:
            Timer1.initialize(1000);
            Timer1.pwm(pin, 0);
            Serial.print(F("Gen1 on pin "));
            break;
        case 5:
            Timer5.initialize(200);
            Timer5.pwm(pin, 0);
            Serial.print(F("Gen2 on pin "));
            break;
    }
    Serial.println(pin);
}

// Установка частоты для конкретного генератора
void FrequencyGenerator::setFreq(int freq) {
    if (!isValidFreq(freq)) {
        Serial.print(F("Error: Frequency "));
        Serial.print(freq);
        Serial.print(F(" out of range (0-"));
        Serial.print(maxFreq);
        Serial.println(F(")"));
        return;
    }
    
    if (freq == this->frequency) return;  // Нет изменений
    
    this->frequency = freq;
    
    if (freq == 0) {
        // Выключаем
        switch(timerId) {
            case 1:
                Timer1.setPwmDuty(pin, 0);
                break;
            case 5:
                Timer5.setPwmDuty(pin, 0);
                break;
        }
        digitalWrite(pin, LOW);
        this->active = false;
        Serial.print(F("Generator on pin "));
        Serial.print(pin);
        Serial.println(F(": OFF"));
    } else {
        // Включаем с заданной частотой
        float period = 1000000.0 / freq;
        
        // Проверка минимального периода для таймера
        if (period < 2) {  // Минимум 2 мкс для таймера на 16MHz
            Serial.print(F("Warning: Frequency too high! Period: "));
            Serial.print(period);
            Serial.println(F(" us"));
            return;
        }

        switch(timerId) {
            case 1:
                Timer1.setPeriod(period);
                Timer1.setPwmDuty(pin, PWM_DUTY_CYCLE);
                break;
            case 5:
                Timer5.setPeriod(period);
                Timer5.setPwmDuty(pin, PWM_DUTY_CYCLE);
                break;
        }
        this->active = true;
        
        Serial.print(F("Generator on pin "));
        Serial.print(pin);
        Serial.print(F(" set to: "));
        Serial.print(freq);
        Serial.println(F(" Hz"));
    }
}

// Инициализация всех генераторов
void initFrequencyGenerators() {
    gen1.init();
    gen2.init();
    Serial.println(F("All frequency generators initialized"));
}

// Установка частоты первого генератора
void setGenerator1(int freq) {
    gen1.setFreq(freq);
}

// Установка частоты второго генератора
void setGenerator2(int freq) {
    gen2.setFreq(freq);
}

// Сохранение частот в EEPROM
void saveFrequencies() {
    // Сохраняем gen1
    EEPROM.put(EEPROM_TARGET_FREQUENCY, gen1.frequency);
    int checksum1 = gen1.frequency + EEPROM_MAGIC_NUMBER;
    EEPROM.put(EEPROM_FREQ_CHECKSUM, checksum1);
    
    // Сохраняем gen2
    EEPROM.put(EEPROM_FREQ2, gen2.frequency);
    int checksum2 = gen2.frequency + EEPROM_MAGIC_NUMBER;
    EEPROM.put(EEPROM_FREQ2_CHECKSUM, checksum2);
}

// Загрузка частот из EEPROM
void loadFrequencies() {
    int loadedFreq;
    int savedChecksum;
    
    // Загружаем gen1
    EEPROM.get(EEPROM_TARGET_FREQUENCY, loadedFreq);
    EEPROM.get(EEPROM_FREQ_CHECKSUM, savedChecksum);
    if (loadedFreq + EEPROM_MAGIC_NUMBER == savedChecksum && 
        loadedFreq >= 0 && loadedFreq <= FREQ1_MAX) {
        gen1.frequency = loadedFreq;
    }
    
    // Загружаем gen2
    EEPROM.get(EEPROM_FREQ2, loadedFreq);
    EEPROM.get(EEPROM_FREQ2_CHECKSUM, savedChecksum);
    if (loadedFreq + EEPROM_MAGIC_NUMBER == savedChecksum && 
        loadedFreq >= 0 && loadedFreq <= FREQ2_MAX) {
        gen2.frequency = loadedFreq;
    }
}