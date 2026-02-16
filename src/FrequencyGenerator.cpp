#include "FrequencyGenerator.h"
#include <Arduino.h>
#include <EEPROM.h>

// Создаём генераторы с разными максимальными частотами
FrequencyGenerator gen1(FREQ1_PIN, 1, FREQ1_MAX);  // 1000 Гц
FrequencyGenerator gen2(FREQ2_PIN, 5, FREQ2_MAX);  // 5000 Гц

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
            Serial.print(F("Gen1 (0-1000Hz) on pin "));
            break;
        case 5:
            Timer5.initialize(200);
            Timer5.pwm(pin, 0);
            Serial.print(F("Gen2 (0-5000Hz) on pin "));
            break;
    }
    Serial.println(pin);
}

// Применить частоту физически
void FrequencyGenerator::applyFrequency(int freq) {
    if (freq == 0) {
        switch(timerId) {
            case 1: Timer1.setPwmDuty(pin, 0); break;
            case 5: Timer5.setPwmDuty(pin, 0); break;
        }
        digitalWrite(pin, LOW);
        active = false;
    } else {
        float period = 1000000.0 / freq;
        
        if (period < 2) {
            Serial.println(F("Warning: Frequency too high!"));
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
        active = true;
    }
}

// Установка целевой частоты
void FrequencyGenerator::setTarget(int freq) {
    if (!isValidFreq(freq)) {
        Serial.print(F("Error: Frequency out of range"));
        return;
    }
    
    if (targetFrequency == freq) return;
    
    targetFrequency = freq;
    Serial.print(F("Target freq set to: "));
    Serial.print(freq);
    Serial.println(F(" Hz"));
    
    // Если aux не активен, начинаем плавный пуск
    if (freq > 0 && !needReset) {
        Serial.println(F("Starting soft start..."));
        currentFrequency = FREQ_RESET_VALUE;  // Начинаем с 1 Гц
        applyFrequency(currentFrequency);
        lastStepTime = millis();
    }
}

// Сброс до 1 Гц (вызывается при изменении aux)
void FrequencyGenerator::reset() {
    if (targetFrequency > 0) {
        needReset = true;
        currentFrequency = FREQ_RESET_VALUE;
        applyFrequency(currentFrequency);
        lastStepTime = millis();
        Serial.println(F("Generator reset to 1 Hz"));
    }
}

// Плавное увеличение частоты (вызывать в loop)
void FrequencyGenerator::update() {
    // Если нет цели или цель уже достигнута
    if (targetFrequency == 0 || currentFrequency >= targetFrequency) {
        return;
    }
    
    // Проверяем время для следующего шага
    if (millis() - lastStepTime >= FREQ_STEP_INTERVAL) {
        // Увеличиваем частоту
        currentFrequency += FREQ_STEP_SIZE;
        
        // Не превышаем целевую
        if (currentFrequency > targetFrequency) {
            currentFrequency = targetFrequency;
        }
        
        // Применяем новую частоту
        applyFrequency(currentFrequency);
        
        Serial.print(F("Freq step: "));
        Serial.print(currentFrequency);
        Serial.print(F(" / "));
        Serial.println(targetFrequency);
        
        lastStepTime = millis();
        
        // Если достигли цели
        if (currentFrequency >= targetFrequency) {
            Serial.println(F("Target frequency reached"));
            needReset = false;
        }
    }
}

// Инициализация всех генераторов
void initFrequencyGenerators() {
    gen1.init();
    gen2.init();
    Serial.println(F("All frequency generators initialized"));
}

// Обновление всех генераторов (вызывать в loop)
void updateAllGenerators() {
    gen1.update();
    gen2.update();
}

// Установка частоты первого генератора
void setGenerator1(int freq) {
    gen1.setTarget(freq);
}

// Установка частоты второго генератора
void setGenerator2(int freq) {
    gen2.setTarget(freq);
}

// Сохранение частот в EEPROM
void saveFrequencies() {
    EEPROM.put(EEPROM_TARGET_FREQUENCY, gen1.targetFrequency);
    int checksum1 = gen1.targetFrequency + EEPROM_MAGIC_NUMBER;
    EEPROM.put(EEPROM_FREQ_CHECKSUM, checksum1);
    
    EEPROM.put(EEPROM_FREQ2, gen2.targetFrequency);
    int checksum2 = gen2.targetFrequency + EEPROM_MAGIC_NUMBER;
    EEPROM.put(EEPROM_FREQ2_CHECKSUM, checksum2);
}

// Загрузка частот из EEPROM
void loadFrequencies() {
    int loadedFreq;
    int savedChecksum;
    
    EEPROM.get(EEPROM_TARGET_FREQUENCY, loadedFreq);
    EEPROM.get(EEPROM_FREQ_CHECKSUM, savedChecksum);
    if (loadedFreq + EEPROM_MAGIC_NUMBER == savedChecksum && 
        loadedFreq >= 0 && loadedFreq <= FREQ1_MAX) {
        gen1.targetFrequency = loadedFreq;
    }
    
    EEPROM.get(EEPROM_FREQ2, loadedFreq);
    EEPROM.get(EEPROM_FREQ2_CHECKSUM, savedChecksum);
    if (loadedFreq + EEPROM_MAGIC_NUMBER == savedChecksum && 
        loadedFreq >= 0 && loadedFreq <= FREQ2_MAX) {
        gen2.targetFrequency = loadedFreq;
    }
}