#include "Eeprom_utils.h"
#include <EEPROM.h>
#include "HeaterController.h"
#include "FrequencyGenerator.h"
#include "Timer_pin.h"
#include "veml6075.h"

// Новые адреса с защитой
#define EEPROM_V44_VALUE 80
#define EEPROM_V44_MAGIC 84
#define EEPROM_V50_VALUE 86
#define EEPROM_V50_MAGIC 90
#define EEPROM_V51_VALUE 92
#define EEPROM_V51_MAGIC 96
#define EEPROM_V52_VALUE 98
#define EEPROM_V52_MAGIC 102

// Внешние переменные
extern UVData uvData;
extern TimerPin timerPin;

void saveAllSettings() {
    // Сохраняем V44 с магическим числом
    EEPROM.put(EEPROM_V44_VALUE, uvData.uvbThreshold);
    EEPROM.put(EEPROM_V44_MAGIC, EEPROM_MAGIC_V44);
    
    // Сохраняем V50
    EEPROM.put(EEPROM_V50_VALUE, timerPin.lowSeconds);
    EEPROM.put(EEPROM_V50_MAGIC, EEPROM_MAGIC_V50);
    
    // Сохраняем V51
    EEPROM.put(EEPROM_V51_VALUE, timerPin.highSeconds);
    EEPROM.put(EEPROM_V51_MAGIC, EEPROM_MAGIC_V51);
    
    // Сохраняем V52 (включен/выключен)
    uint8_t enabledByte = timerPin.enabled ? 1 : 0;
    EEPROM.put(EEPROM_V52_VALUE, enabledByte);
    EEPROM.put(EEPROM_V52_MAGIC, EEPROM_MAGIC_V52);

    // Сохраняем V32 (AUX3)
    uint8_t aux3State = aux3.getState() ? 1 : 0;
    EEPROM.put(EEPROM_V34_VALUE, aux3State);
    EEPROM.put(EEPROM_V34_MAGIC, EEPROM_MAGIC_V34);
    
    Serial.println(F("All settings saved to EEPROM"));
}

// Сохранение целевых температур
void saveTargetTemps(float temp1, float temp2) {
    // Сохраняем температуру 1
    EEPROM.put(EEPROM_TARGET_TEMP_1, temp1);
    int checksum1 = (int)(temp1 * 10) + EEPROM_MAGIC_NUMBER;
    EEPROM.put(EEPROM_TEMP_CHECKSUM_1, checksum1);
    
    // Сохраняем температуру 2
    EEPROM.put(EEPROM_TARGET_TEMP_2, temp2);
    int checksum2 = (int)(temp2 * 10) + EEPROM_MAGIC_NUMBER;
    EEPROM.put(EEPROM_TEMP_CHECKSUM_2, checksum2);
}

// Загрузка целевых температур
void loadTargetTemps(float &temp1, float &temp2) {
    float loadedTemp;
    int savedChecksum;
    
    // Загружаем температуру 1
    EEPROM.get(EEPROM_TARGET_TEMP_1, loadedTemp);
    EEPROM.get(EEPROM_TEMP_CHECKSUM_1, savedChecksum);
    int calculatedChecksum = (int)(loadedTemp * 10) + EEPROM_MAGIC_NUMBER;
    
    if (calculatedChecksum == savedChecksum && 
        loadedTemp >= MIN_TARGET_TEMP && 
        loadedTemp <= MAX_TARGET_TEMP) {
        temp1 = loadedTemp;
    } else {
        temp1 = 30.0; // Значение по умолчанию
    }
    
    // Загружаем температуру 2
    EEPROM.get(EEPROM_TARGET_TEMP_2, loadedTemp);
    EEPROM.get(EEPROM_TEMP_CHECKSUM_2, savedChecksum);
    calculatedChecksum = (int)(loadedTemp * 10) + EEPROM_MAGIC_NUMBER;
    
    if (calculatedChecksum == savedChecksum && 
        loadedTemp >= MIN_TARGET_TEMP && 
        loadedTemp <= MAX_TARGET_TEMP) {
        temp2 = loadedTemp;
    } else {
        temp2 = 30.0; // Значение по умолчанию
    }
}

void saveTimerInterval() {
    extern unsigned int v30TimerInterval;
    EEPROM.put(EEPROM_TIMER_INTERVAL, v30TimerInterval);
    int checksum = v30TimerInterval + EEPROM_MAGIC_NUMBER;
    EEPROM.put(EEPROM_TIMER_CHECKSUM, checksum);
}

void loadTimerInterval() {
    extern unsigned int v30TimerInterval;
    unsigned int loadedInterval;
    int savedChecksum;
    
    EEPROM.get(EEPROM_TIMER_INTERVAL, loadedInterval);
    EEPROM.get(EEPROM_TIMER_CHECKSUM, savedChecksum);
    
    if (loadedInterval + EEPROM_MAGIC_NUMBER == savedChecksum) {
        v30TimerInterval = constrain(loadedInterval, MIN_TIMER_INTERVAL, MAX_TIMER_INTERVAL);
    }
}

// Загрузка всех настроек
void loadAllSettings() {
    extern Heater heater1;
    extern Heater heater2;

    uint16_t magic;
    float tempFloat;
    unsigned int tempInt;
    uint8_t tempByte;
    uint8_t savedState;
    
    loadTargetTemps(heater1.targetTemp, heater2.targetTemp);
    loadFrequencies();  // Загружаем оба генератора
    loadTimerInterval();

    EEPROM.get(EEPROM_V34_MAGIC, magic);
    if (magic == EEPROM_MAGIC_V34) {
        EEPROM.get(EEPROM_V34_VALUE, savedState);
        if (savedState == 0 || savedState == 1) {
            aux3.setTarget(savedState == 1);
            Serial.print(F("Loaded V32: "));
            Serial.println(savedState);
        }
    }

    // Загружаем V44
    EEPROM.get(EEPROM_V44_MAGIC, magic);
    if (magic == EEPROM_MAGIC_V44) {
        EEPROM.get(EEPROM_V44_VALUE, tempFloat);
        if (tempFloat >= MIN_UVB_THRESHOLD && tempFloat <= MAX_UVB_THRESHOLD) {
            uvData.uvbThreshold = tempFloat;
            Serial.print(F("Loaded V44: "));
            Serial.println(tempFloat);
        }
    }
    
    // Загружаем V50
    EEPROM.get(EEPROM_V50_MAGIC, magic);
    if (magic == EEPROM_MAGIC_V50) {
        EEPROM.get(EEPROM_V50_VALUE, tempInt);
        if (tempInt >= MIN_TIMER_SECONDS && tempInt <= MAX_TIMER_SECONDS) {
            timerPin.lowSeconds = tempInt;
            Serial.print(F("Loaded V50: "));
            Serial.println(tempInt);
        }
    }
    
    // Загружаем V51
    EEPROM.get(EEPROM_V51_MAGIC, magic);
    if (magic == EEPROM_MAGIC_V51) {
        EEPROM.get(EEPROM_V51_VALUE, tempInt);
        if (tempInt >= MIN_TIMER_SECONDS && tempInt <= MAX_TIMER_SECONDS) {
            timerPin.highSeconds = tempInt;
            Serial.print(F("Loaded V51: "));
            Serial.println(tempInt);
        }
    }
    
    // Загружаем V52
    EEPROM.get(EEPROM_V52_MAGIC, magic);
    if (magic == EEPROM_MAGIC_V52) {
        EEPROM.get(EEPROM_V52_VALUE, tempByte);
        if (tempByte == 0 || tempByte == 1) {
            timerPin.setEnabled(tempByte == 1);
            Serial.print(F("Loaded V52: "));
            Serial.println(tempByte);
        }
    }
}

