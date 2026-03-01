#include "Eeprom_utils.h"
#include <EEPROM.h>
#include "HeaterController.h"
#include "FrequencyGenerator.h"
#include "Timer_pin.h"
#include "veml6075.h"

// Внешние переменные
extern UVData uvData;
extern TimerPin timerPin;
extern AuxPin aux3;
extern AuxPin aux4;
extern AuxPin aux5;
extern Heater heater1;
extern Heater heater2;
extern unsigned int v30TimerInterval;
extern FrequencyGenerator gen1;
extern FrequencyGenerator gen2;
extern FrequencyGenerator gen3;

// Структура заголовка EEPROM
struct EEPROMHeader {
    uint8_t version;
    uint16_t magic;
    uint32_t size;
    uint16_t checksum;
} header;

// Глобальный счётчик записей (для отладки)
static int eepromWriteCount = 0;
static unsigned long lastWriteTime = 0;

// ==================== ПРОВЕРКА ЦЕЛОСТНОСТИ EEPROM ====================

// Проверка EEPROM при запуске (БЕЗ автоматического форматирования!)
bool checkEEPROM() {
    EEPROM.get(EEPROM_HEADER_ADDR, header);
    
    // Проверяем валидность заголовка
    if (header.version == EEPROM_VERSION && 
        header.magic == 0x55AA && 
        header.checksum == (uint16_t)(header.version + header.magic + header.size)) {
        Serial.println(F("EEPROM header valid"));
        return true;
    } else {
        Serial.println(F("EEPROM header invalid - but will try to read data anyway"));
        return false;
    }
}

// Форматирование EEPROM (только по команде!)
void formatEEPROM() {
    Serial.println(F("Formatting EEPROM..."));
    
    // Записываем заголовок
    header.version = EEPROM_VERSION;
    header.magic = 0x55AA;
    header.size = 4096;
    header.checksum = (uint16_t)(header.version + header.magic + header.size);
    EEPROM.put(EEPROM_HEADER_ADDR, header);
    
    // Сбрасываем все значения по умолчанию
    saveTargetTemps(30.0, 30.0);
    
    // Сбрасываем генераторы
    int zero = 0;
    EEPROM.put(EEPROM_TARGET_FREQUENCY, zero);
    EEPROM.put(EEPROM_FREQ_CHECKSUM, zero + EEPROM_MAGIC_NUMBER);
    EEPROM.put(EEPROM_TARGET_FREQUENCY_2, zero);
    EEPROM.put(EEPROM_FREQ2_CHECKSUM, zero + EEPROM_MAGIC_NUMBER);
    EEPROM.put(EEPROM_FREQ3, zero);
    EEPROM.put(EEPROM_FREQ3_CHECKSUM, zero + EEPROM_MAGIC_NUMBER);
    
    // Сбрасываем UV
    float defaultUv = DEFAULT_UVB_THRESHOLD;
    EEPROM.put(EEPROM_V44_VALUE, defaultUv);
    EEPROM.put(EEPROM_V44_MAGIC, EEPROM_MAGIC_V44);
    
    // Сбрасываем таймер
    unsigned int defaultLow = DEFAULT_LOW_SECONDS;
    unsigned int defaultHigh = DEFAULT_HIGH_SECONDS;
    EEPROM.put(EEPROM_V50_VALUE, defaultLow);
    EEPROM.put(EEPROM_V50_MAGIC, EEPROM_MAGIC_V50);
    EEPROM.put(EEPROM_V51_VALUE, defaultHigh);
    EEPROM.put(EEPROM_V51_MAGIC, EEPROM_MAGIC_V51);
    uint8_t enabledZero = 0;
    EEPROM.put(EEPROM_V52_VALUE, enabledZero);
    EEPROM.put(EEPROM_V52_MAGIC, EEPROM_MAGIC_V52);
    
    // Сбрасываем AUX
    EEPROM.put(EEPROM_V32_VALUE, enabledZero);
    EEPROM.put(EEPROM_V32_MAGIC, EEPROM_MAGIC_V32);
    EEPROM.put(EEPROM_V34_VALUE, enabledZero);
    EEPROM.put(EEPROM_V34_MAGIC, EEPROM_MAGIC_V34);
    EEPROM.put(EEPROM_V36_VALUE, enabledZero);
    EEPROM.put(EEPROM_V36_MAGIC, EEPROM_MAGIC_V36);
    EEPROM.put(EEPROM_V37_VALUE, enabledZero);
    EEPROM.put(EEPROM_V37_MAGIC, EEPROM_MAGIC_V37);
    
    // Сбрасываем таймер V30
    unsigned long defaultTimer = DEFAULT_TIMER_INTERVAL;
    EEPROM.put(EEPROM_V30_INTERVAL, defaultTimer);
    EEPROM.put(EEPROM_V30_CHECKSUM, defaultTimer + EEPROM_MAGIC_NUMBER);
    
    Serial.println(F("EEPROM formatted"));
}

// ==================== ФУНКЦИИ СОХРАНЕНИЯ ====================

void saveAllSettings() {
    // Защита от слишком частых записей (не чаще 1 раза в 2 секунды)
    if (millis() - lastWriteTime < 2000) {
        return;  // Слишком часто, пропускаем
    }
    
    lastWriteTime = millis();
    eepromWriteCount++;

    // Сохраняем V7 (heatingPermission1)
    uint8_t v7State = heatingPermission1 ? 1 : 0;
    EEPROM.put(EEPROM_V7_VALUE, v7State);
    EEPROM.put(EEPROM_V7_MAGIC, EEPROM_MAGIC_V7);
    
    // Сохраняем V17 (heatingPermission2)
    uint8_t v17State = heatingPermission2 ? 1 : 0;
    EEPROM.put(EEPROM_V17_VALUE, v17State);
    EEPROM.put(EEPROM_V17_MAGIC, EEPROM_MAGIC_V17);

    // V44
    EEPROM.put(EEPROM_V44_VALUE, uvData.uvbThreshold);
    EEPROM.put(EEPROM_V44_MAGIC, EEPROM_MAGIC_V44);
    
    // V50, V51, V52
    EEPROM.put(EEPROM_V50_VALUE, timerPin.lowSeconds);
    EEPROM.put(EEPROM_V50_MAGIC, EEPROM_MAGIC_V50);
    EEPROM.put(EEPROM_V51_VALUE, timerPin.highSeconds);
    EEPROM.put(EEPROM_V51_MAGIC, EEPROM_MAGIC_V51);
    
    uint8_t enabledByte = timerPin.enabled ? 1 : 0;
    EEPROM.put(EEPROM_V52_VALUE, enabledByte);
    EEPROM.put(EEPROM_V52_MAGIC, EEPROM_MAGIC_V52);

    // AUX3 (V32)
    uint8_t aux3State = aux3.getState() ? 1 : 0;
    EEPROM.put(EEPROM_V32_VALUE, aux3State);
    EEPROM.put(EEPROM_V32_MAGIC, EEPROM_MAGIC_V32);
    
    // Для совместимости со старым кодом
    EEPROM.put(EEPROM_V34_VALUE, aux3State);
    EEPROM.put(EEPROM_V34_MAGIC, EEPROM_MAGIC_V34);

    // AUX4 (V36)
    uint8_t aux4State = aux4.getState() ? 1 : 0;
    EEPROM.put(EEPROM_V36_VALUE, aux4State);
    EEPROM.put(EEPROM_V36_MAGIC, EEPROM_MAGIC_V36);
    
    // AUX5 (V37)
    uint8_t aux5State = aux5.getState() ? 1 : 0;
    EEPROM.put(EEPROM_V37_VALUE, aux5State);
    EEPROM.put(EEPROM_V37_MAGIC, EEPROM_MAGIC_V37);
    
    // Сохраняем таймер V30
    EEPROM.put(EEPROM_V30_INTERVAL, v30TimerInterval);
    EEPROM.put(EEPROM_V30_CHECKSUM, v30TimerInterval + EEPROM_MAGIC_NUMBER);
    
    // Сохраняем температуры
    saveTargetTemps(heater1.targetTemp, heater2.targetTemp);
    
    // Сохраняем частоты
    saveFrequencies();

    Serial.println(F("All settings saved to EEPROM"));
}

// ==================== ФУНКЦИИ ЗАГРУЗКИ ====================

void loadAllSettings() {
    // Проверяем EEPROM, но НЕ форматируем автоматически
    checkEEPROM();

    uint16_t magic;
    float tempFloat;
    unsigned int tempInt;
    uint8_t tempByte;
    uint8_t savedState;
    bool loadedAny = false;

    // Загружаем температуры
    loadTargetTemps(heater1.targetTemp, heater2.targetTemp);
    loadedAny = true;
    
    // Загружаем частоты
    loadFrequencies();
    
    // Загружаем таймер V30
    loadTimerInterval();

    // ===== ЗАГРУЗКА V7 =====
    EEPROM.get(EEPROM_V7_MAGIC, magic);
    if (magic == EEPROM_MAGIC_V7) {
        EEPROM.get(EEPROM_V7_VALUE, savedState);
        if (savedState == 0 || savedState == 1) {
            heatingPermission1 = (savedState == 1);
            Serial.print(F("Loaded V7: "));
            Serial.println(heatingPermission1 ? F("ON") : F("OFF"));
            loadedAny = true;
        }
    }
    
    // ===== ЗАГРУЗКА V17 =====
    EEPROM.get(EEPROM_V17_MAGIC, magic);
    if (magic == EEPROM_MAGIC_V17) {
        EEPROM.get(EEPROM_V17_VALUE, savedState);
        if (savedState == 0 || savedState == 1) {
            heatingPermission2 = (savedState == 1);
            Serial.print(F("Loaded V17: "));
            Serial.println(heatingPermission2 ? F("ON") : F("OFF"));
            loadedAny = true;
        }
    }

    // ===== ЗАГРУЗКА С ЗАЩИТОЙ =====
    
    // V32 (AUX3) - новый адрес
    EEPROM.get(EEPROM_V32_MAGIC, magic);
    if (magic == EEPROM_MAGIC_V32) {
        EEPROM.get(EEPROM_V32_VALUE, savedState);
        if (savedState == 0 || savedState == 1) {
            aux3.setTarget(savedState == 1);
            Serial.print(F("Loaded V32: "));
            Serial.println(savedState);
            loadedAny = true;
        }
    } 
    // Если нет, пробуем старый адрес V34
    else {
        EEPROM.get(EEPROM_V34_MAGIC, magic);
        if (magic == EEPROM_MAGIC_V34) {
            EEPROM.get(EEPROM_V34_VALUE, savedState);
            if (savedState == 0 || savedState == 1) {
                aux3.setTarget(savedState == 1);
                Serial.print(F("Loaded V32 (compat): "));
                Serial.println(savedState);
                loadedAny = true;
            }
        }
    }

    // V44
    EEPROM.get(EEPROM_V44_MAGIC, magic);
    if (magic == EEPROM_MAGIC_V44) {
        EEPROM.get(EEPROM_V44_VALUE, tempFloat);
        if (tempFloat >= MIN_UVB_THRESHOLD && tempFloat <= MAX_UVB_THRESHOLD) {
            uvData.uvbThreshold = tempFloat;
            Serial.print(F("Loaded V44: "));
            Serial.println(tempFloat);
            loadedAny = true;
        }
    }
    
    // V50
    EEPROM.get(EEPROM_V50_MAGIC, magic);
    if (magic == EEPROM_MAGIC_V50) {
        EEPROM.get(EEPROM_V50_VALUE, tempInt);
        if (tempInt >= MIN_TIMER_SECONDS && tempInt <= MAX_TIMER_SECONDS) {
            timerPin.lowSeconds = tempInt;
            Serial.print(F("Loaded V50: "));
            Serial.println(tempInt);
            loadedAny = true;
        }
    }
    
    // V51
    EEPROM.get(EEPROM_V51_MAGIC, magic);
    if (magic == EEPROM_MAGIC_V51) {
        EEPROM.get(EEPROM_V51_VALUE, tempInt);
        if (tempInt >= MIN_TIMER_SECONDS && tempInt <= MAX_TIMER_SECONDS) {
            timerPin.highSeconds = tempInt;
            Serial.print(F("Loaded V51: "));
            Serial.println(tempInt);
            loadedAny = true;
        }
    }
    
    // V52
    EEPROM.get(EEPROM_V52_MAGIC, magic);
    if (magic == EEPROM_MAGIC_V52) {
        EEPROM.get(EEPROM_V52_VALUE, tempByte);
        if (tempByte == 0 || tempByte == 1) {
            timerPin.setEnabled(tempByte == 1);
            Serial.print(F("Loaded V52: "));
            Serial.println(tempByte);
            loadedAny = true;
        }
    }

    // V36 (AUX4)
    EEPROM.get(EEPROM_V36_MAGIC, magic);
    if (magic == EEPROM_MAGIC_V36) {
        EEPROM.get(EEPROM_V36_VALUE, savedState);
        if (savedState == 0 || savedState == 1) {
            aux4.setTarget(savedState == 1);
            Serial.print(F("Loaded V36: "));
            Serial.println(savedState);
            loadedAny = true;
        }
    }
    
    // V37 (AUX5)
    EEPROM.get(EEPROM_V37_MAGIC, magic);
    if (magic == EEPROM_MAGIC_V37) {
        EEPROM.get(EEPROM_V37_VALUE, savedState);
        if (savedState == 0 || savedState == 1) {
            aux5.setTarget(savedState == 1);
            Serial.print(F("Loaded V37: "));
            Serial.println(savedState);
            loadedAny = true;
        }
    }
    
    // Если ничего не загрузилось - только тогда ставим defaults
    if (!loadedAny) {
        Serial.println(F("No valid data found in EEPROM - using defaults"));
        setDefaultsInMemory();
    } else {
        Serial.println(F("Settings loaded successfully"));
        // Проверяем на валидность и исправляем если надо
        validateAndFixSettings();
    }
    
    // Всегда сохраняем заголовок, если его нет
    if (!checkEEPROM()) {
        Serial.println(F("Writing EEPROM header..."));
        header.version = EEPROM_VERSION;
        header.magic = 0x55AA;
        header.size = 4096;
        header.checksum = (uint16_t)(header.version + header.magic + header.size);
        EEPROM.put(EEPROM_HEADER_ADDR, header);
    }
}

// ==================== ФУНКЦИИ УСТАНОВКИ ЗНАЧЕНИЙ ПО УМОЛЧАНИЮ ====================

void setDefaultsInMemory() {
    // Устанавливаем значения по умолчанию ТОЛЬКО в оперативной памяти
    heater1.targetTemp = 30.0;
    heater2.targetTemp = 30.0;

    heatingPermission1 = true;  // V7 по умолчанию ON
    heatingPermission2 = true;  // V17 по умолчанию ON

    // Синхронизируем с объектами ТЭНов
    heater1.permission = heatingPermission1;
    heater2.permission = heatingPermission2;
    
    gen1.targetFrequency = 0;
    gen2.targetFrequency = 0;
    gen3.targetFrequency = 0;
    
    uvData.uvbThreshold = DEFAULT_UVB_THRESHOLD;
    
    timerPin.lowSeconds = DEFAULT_LOW_SECONDS;
    timerPin.highSeconds = DEFAULT_HIGH_SECONDS;
    timerPin.setEnabled(false);
    
    aux3.setTarget(false);
    aux4.setTarget(false);
    aux5.setTarget(false);
    
    v30TimerInterval = DEFAULT_TIMER_INTERVAL;
    
    Serial.println(F("Default settings loaded in memory"));
}

// ==================== СУЩЕСТВУЮЩИЕ ФУНКЦИИ ====================

void saveTargetTemps(float temp1, float temp2) {
    EEPROM.put(EEPROM_TARGET_TEMP_1, temp1);
    int checksum1 = (int)(temp1 * 10) + EEPROM_MAGIC_NUMBER;
    EEPROM.put(EEPROM_TEMP_CHECKSUM_1, checksum1);
    
    EEPROM.put(EEPROM_TARGET_TEMP_2, temp2);
    int checksum2 = (int)(temp2 * 10) + EEPROM_MAGIC_NUMBER;
    EEPROM.put(EEPROM_TEMP_CHECKSUM_2, checksum2);
}

void loadTargetTemps(float &temp1, float &temp2) {
    float loadedTemp;
    int savedChecksum;
    
    EEPROM.get(EEPROM_TARGET_TEMP_1, loadedTemp);
    EEPROM.get(EEPROM_TEMP_CHECKSUM_1, savedChecksum);
    int calculatedChecksum = (int)(loadedTemp * 10) + EEPROM_MAGIC_NUMBER;
    
    if (calculatedChecksum == savedChecksum && 
        loadedTemp >= MIN_TARGET_TEMP && 
        loadedTemp <= MAX_TARGET_TEMP) {
        temp1 = loadedTemp;
    } else {
        temp1 = 30.0;
    }
    
    EEPROM.get(EEPROM_TARGET_TEMP_2, loadedTemp);
    EEPROM.get(EEPROM_TEMP_CHECKSUM_2, savedChecksum);
    calculatedChecksum = (int)(loadedTemp * 10) + EEPROM_MAGIC_NUMBER;
    
    if (calculatedChecksum == savedChecksum && 
        loadedTemp >= MIN_TARGET_TEMP && 
        loadedTemp <= MAX_TARGET_TEMP) {
        temp2 = loadedTemp;
    } else {
        temp2 = 30.0;
    }
}

void saveFrequencies() {
    EEPROM.put(EEPROM_TARGET_FREQUENCY, gen1.targetFrequency);
    EEPROM.put(EEPROM_FREQ_CHECKSUM, gen1.targetFrequency + EEPROM_MAGIC_NUMBER);
    
    EEPROM.put(EEPROM_TARGET_FREQUENCY_2, gen2.targetFrequency);
    EEPROM.put(EEPROM_FREQ2_CHECKSUM, gen2.targetFrequency + EEPROM_MAGIC_NUMBER);
    
    EEPROM.put(EEPROM_FREQ3, gen3.targetFrequency);
    EEPROM.put(EEPROM_FREQ3_CHECKSUM, gen3.targetFrequency + EEPROM_MAGIC_NUMBER);
}

void loadFrequencies() {
    int loadedFreq;
    int savedChecksum;
    
    EEPROM.get(EEPROM_TARGET_FREQUENCY, loadedFreq);
    EEPROM.get(EEPROM_FREQ_CHECKSUM, savedChecksum);
    if (loadedFreq + EEPROM_MAGIC_NUMBER == savedChecksum && 
        loadedFreq >= 0 && loadedFreq <= FREQ1_MAX) {
        gen1.targetFrequency = loadedFreq;
    }
    
    EEPROM.get(EEPROM_TARGET_FREQUENCY_2, loadedFreq);
    EEPROM.get(EEPROM_FREQ2_CHECKSUM, savedChecksum);
    if (loadedFreq + EEPROM_MAGIC_NUMBER == savedChecksum && 
        loadedFreq >= 0 && loadedFreq <= FREQ2_MAX) {
        gen2.targetFrequency = loadedFreq;
    }
    
    EEPROM.get(EEPROM_FREQ3, loadedFreq);
    EEPROM.get(EEPROM_FREQ3_CHECKSUM, savedChecksum);
    if (loadedFreq + EEPROM_MAGIC_NUMBER == savedChecksum && 
        loadedFreq >= 0 && loadedFreq <= FREQ3_MAX) {
        gen3.targetFrequency = loadedFreq;
    }
}

void saveTimerInterval() {
    EEPROM.put(EEPROM_V30_INTERVAL, v30TimerInterval);
    EEPROM.put(EEPROM_V30_CHECKSUM, v30TimerInterval + EEPROM_MAGIC_NUMBER);
}

void loadTimerInterval() {
    unsigned int loadedInterval;
    int savedChecksum;
    
    EEPROM.get(EEPROM_V30_INTERVAL, loadedInterval);
    EEPROM.get(EEPROM_V30_CHECKSUM, savedChecksum);
    
    if ((int)(loadedInterval + EEPROM_MAGIC_NUMBER) == savedChecksum) {
        unsigned long constrained = constrain((unsigned long)loadedInterval, 
                                              (unsigned long)MIN_TIMER_INTERVAL, 
                                              (unsigned long)MAX_TIMER_INTERVAL);
        v30TimerInterval = constrained;
    }
}

// ==================== ФУНКЦИЯ ВАЛИДАЦИИ ====================

void validateAndFixSettings() {
    bool changed = false;
    
    // Проверяем температуры
    if (heater1.targetTemp < MIN_TARGET_TEMP || heater1.targetTemp > MAX_TARGET_TEMP) {
        heater1.targetTemp = 30.0;
        changed = true;
    }
    if (heater2.targetTemp < MIN_TARGET_TEMP || heater2.targetTemp > MAX_TARGET_TEMP) {
        heater2.targetTemp = 30.0;
        changed = true;
    }
    
    // Проверяем частоты
    if (gen1.targetFrequency < 0 || gen1.targetFrequency > FREQ1_MAX) {
        gen1.targetFrequency = 0;
        changed = true;
    }
    if (gen2.targetFrequency < 0 || gen2.targetFrequency > FREQ2_MAX) {
        gen2.targetFrequency = 0;
        changed = true;
    }
    if (gen3.targetFrequency < 0 || gen3.targetFrequency > FREQ3_MAX) {
        gen3.targetFrequency = 0;
        changed = true;
    }
    
    // Проверяем UV
    if (uvData.uvbThreshold < MIN_UVB_THRESHOLD || uvData.uvbThreshold > MAX_UVB_THRESHOLD) {
        uvData.uvbThreshold = DEFAULT_UVB_THRESHOLD;
        changed = true;
    }
    
    // Проверяем таймер
    if (timerPin.lowSeconds < MIN_TIMER_SECONDS || timerPin.lowSeconds > MAX_TIMER_SECONDS) {
        timerPin.lowSeconds = DEFAULT_LOW_SECONDS;
        changed = true;
    }
    if (timerPin.highSeconds < MIN_TIMER_SECONDS || timerPin.highSeconds > MAX_TIMER_SECONDS) {
        timerPin.highSeconds = DEFAULT_HIGH_SECONDS;
        changed = true;
    }
    
    // Проверяем V30
    if (v30TimerInterval < MIN_TIMER_INTERVAL || v30TimerInterval > MAX_TIMER_INTERVAL) {
        v30TimerInterval = DEFAULT_TIMER_INTERVAL;
        changed = true;
    }
    
    // Если были изменения, сохраняем
    if (changed) {
        Serial.println(F("Settings validation: fixed invalid values"));
        saveAllSettings();
    }
}