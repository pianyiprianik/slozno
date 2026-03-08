#include "TemperatureSensors.h"
#include "HeaterController.h"
#include <Arduino.h>

// 1-Wire шины
OneWire oneWireMain(ONE_WIRE_BUS_MAIN);      // Пин 6
OneWire oneWireExtra(ONE_WIRE_BUS_EXTRA);    // Пин 7

// Объекты DallasTemperature
DallasTemperature sensorMain(&oneWireMain);
DallasTemperature sensorExtra(&oneWireExtra);

// Данные датчиков
TempSensorData mainData1;   // V1
TempSensorData mainData2;   // V11
TempSensorData extraData1;  // V60
TempSensorData extraData2;  // V61

// Флаги для асинхронного чтения
static bool mainConversionStarted = false;
static bool extraConversionStarted = false;

static unsigned long conversionStartTime = 0;
static unsigned long lastMainRecoveryTime = 0;
static unsigned long lastExtraRecoveryTime = 0;

// Константа для времени конверсии
const unsigned long CONVERSION_TIME = 750; // 750ms для 12-bit
const unsigned long RECOVERY_INTERVAL = 5000; // 5 секунд

// Вспомогательная функция для вывода адреса
void printAddress(const uint8_t* addr) {
    for (uint8_t i = 0; i < 8; i++) {
        if (addr[i] < 16) Serial.print("0");
        Serial.print(addr[i], HEX);
    }
}

// Сканирование конкретной шины
int scanBus(OneWire &bus, const char* busName, TempSensorData* data1, TempSensorData* data2) {
    Serial.print(F("Scanning "));
    Serial.print(busName);
    Serial.println(F(" bus..."));
    
    uint8_t addr[8];
    int count = 0;
    
    bus.reset_search();
    while (bus.search(addr)) {
        if (OneWire::crc8(addr, 7) == addr[7]) {
            count++;
            Serial.print(F("  Device "));
            Serial.print(count);
            Serial.print(F(": "));
            printAddress(addr);
            
            // Определяем тип датчика
            if (addr[0] == 0x28) Serial.print(F(" (DS18B20)"));
            else if (addr[0] == 0x10) Serial.print(F(" (DS18S20)"));
            else if (addr[0] == 0x22) Serial.print(F(" (DS1822)"));
            Serial.println();
            
            // Сохраняем адрес в соответствующий датчик
            if (count == 1 && data1 != nullptr) {
                memcpy(data1->address, addr, 8);
            } else if (count == 2 && data2 != nullptr) {
                memcpy(data2->address, addr, 8);
            }
        }
    }
    
    Serial.print(F("Found "));
    Serial.print(count);
    Serial.print(F(" devices on "));
    Serial.println(busName);
    
    return count;
}

// Сканирование всех шин
void scanAllBuses() {
    Serial.println(F("=== Scanning all 1-Wire buses ==="));
    
    int mainCount = scanBus(oneWireMain, "MAIN (pin 6)", &mainData1, &mainData2);
    int extraCount = scanBus(oneWireExtra, "EXTRA (pin 7)", &extraData1, &extraData2);
    
    // Обновляем статус датчиков
    mainData1.initialized = (mainCount >= 1);
    mainData2.initialized = (mainCount >= 2);
    extraData1.initialized = (extraCount >= 1);
    extraData2.initialized = (extraCount >= 2);
    
    Serial.print(F("Main bus: "));
    Serial.print(mainCount);
    Serial.println(mainCount >= 2 ? F(" (both OK)") : F(" (incomplete)"));
    
    Serial.print(F("Extra bus: "));
    Serial.print(extraCount);
    Serial.println(extraCount >= 2 ? F(" (both OK)") : F(" (incomplete)"));
}

// Инициализация всех датчиков
void initTemperatureSensors() {
    Serial.println(F("Initializing temperature sensors..."));
    
    // Инициализация шин
    sensorMain.begin();
    sensorMain.setResolution(12);
    sensorMain.setWaitForConversion(false);
    
    sensorExtra.begin();
    sensorExtra.setResolution(12);
    sensorExtra.setWaitForConversion(false);
    
    // Первое сканирование
    delay(500);  // Даем время датчикам стабилизироваться
    scanAllBuses();
    
    Serial.println(F("Temperature sensors initialized"));
}

// Проверка и восстановление MAIN шины
void checkAndRecoverMainBus() {
    if (millis() - lastMainRecoveryTime < RECOVERY_INTERVAL) return;
    lastMainRecoveryTime = millis();
    
    int count = scanBus(oneWireMain, "MAIN (pin 6)", &mainData1, &mainData2);
    
    bool needReset = false;
    
    if (count >= 2) {
        if (!mainData1.initialized || !mainData2.initialized) {
            Serial.println(F("Main sensors recovered!"));
            mainData1.initialized = true;
            mainData2.initialized = true;
            mainData1.errorCount = 0;
            mainData2.errorCount = 0;
            needReset = true;
        }
    } else if (count == 1) {
        if (!mainData1.initialized) {
            Serial.println(F("First main sensor recovered"));
            mainData1.initialized = true;
            mainData1.errorCount = 0;
            needReset = true;
        }
        mainData2.initialized = false;
    } else {
        mainData1.initialized = false;
        mainData2.initialized = false;
    }
    
    if (needReset) {
        sensorMain.begin();  // Переинициализация
    }
}

// Проверка и восстановление EXTRA шины
void checkAndRecoverExtraBus() {
    if (millis() - lastExtraRecoveryTime < RECOVERY_INTERVAL) return;
    lastExtraRecoveryTime = millis();
    
    int count = scanBus(oneWireExtra, "EXTRA (pin 7)", &extraData1, &extraData2);
    
    bool needReset = false;
    
    if (count >= 2) {
        if (!extraData1.initialized || !extraData2.initialized) {
            Serial.println(F("Extra sensors recovered!"));
            extraData1.initialized = true;
            extraData2.initialized = true;
            extraData1.errorCount = 0;
            extraData2.errorCount = 0;
            needReset = true;
        }
    } else if (count == 1) {
        if (!extraData1.initialized) {
            Serial.println(F("First extra sensor recovered"));
            extraData1.initialized = true;
            extraData1.errorCount = 0;
            needReset = true;
        }
        extraData2.initialized = false;
    } else {
        extraData1.initialized = false;
        extraData2.initialized = false;
    }
    
    if (needReset) {
        sensorExtra.begin();  // Переинициализация
    }
}

// Запрос температур
void requestTemperatures() {
    unsigned long now = millis();
    
    if (!mainConversionStarted && (mainData1.initialized || mainData2.initialized)) {
        sensorMain.requestTemperatures();
        mainConversionStarted = true;
        conversionStartTime = now;
    }
    
    if (!extraConversionStarted && (extraData1.initialized || extraData2.initialized)) {
        sensorExtra.requestTemperatures();
        extraConversionStarted = true;
    }
}

// Обновление MAIN датчиков
void updateMainTemperatures() {
    unsigned long now = millis();
    
    if (!mainConversionStarted) return;
    if (now - conversionStartTime < CONVERSION_TIME) return;
    
    if (mainData1.initialized) {
        float newTemp1 = sensorMain.getTempCByIndex(0);
        mainData1.update(newTemp1, now);
    }
    
    if (mainData2.initialized) {
        float newTemp2 = sensorMain.getTempCByIndex(1);
        mainData2.update(newTemp2, now);
    }
    
    mainConversionStarted = false;
}

// Обновление EXTRA датчиков
void updateExtraTemperatures() {
    unsigned long now = millis();
    
    if (!extraConversionStarted) return;
    if (now - conversionStartTime < CONVERSION_TIME) return;
    
    if (extraData1.initialized) {
        float newTemp1 = sensorExtra.getTempCByIndex(0);
        extraData1.update(newTemp1, now);
    }
    
    if (extraData2.initialized) {
        float newTemp2 = sensorExtra.getTempCByIndex(1);
        extraData2.update(newTemp2, now);
    }
    
    extraConversionStarted = false;
}

// Обновление всех температур
void updateAllTemperatures(Heater &heater1, Heater &heater2) {
    requestTemperatures();
    
    updateMainTemperatures();
    updateExtraTemperatures();
    
    // Обновляем объекты ТЭНов из mainData
    heater1.currentTemp = getMainTemp1();
    heater1.alarm = !mainData1.valid;
    heater2.currentTemp = getMainTemp2();
    heater2.alarm = !mainData2.valid;
    
    // Периодическое восстановление
    checkAndRecoverMainBus();
    checkAndRecoverExtraBus();
}

bool isValidTemperature(float temp) {
    return TempSensorData::isValidReading(temp);
}

// Функции для получения значений
float getMainTemp1() {
    if (!mainData1.initialized) return 0.0;
    return mainData1.getSafeValue();
}

float getMainTemp2() {
    if (!mainData2.initialized) return 0.0;
    return mainData2.getSafeValue();
}

float getExtraTemp1() {
    if (!extraData1.initialized) return 0.0;
    return extraData1.getSafeValue();
}

float getExtraTemp2() {
    if (!extraData2.initialized) return 0.0;
    return extraData2.getSafeValue();
}