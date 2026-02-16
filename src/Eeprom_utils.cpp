#include "Eeprom_utils.h"
#include <EEPROM.h>
#include "HeaterController.h"
#include "FrequencyGenerator.h"

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
    
    loadTargetTemps(heater1.targetTemp, heater2.targetTemp);
    loadFrequencies();  // Загружаем оба генератора
    loadTimerInterval();
}

