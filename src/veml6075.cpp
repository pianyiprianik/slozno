#include "veml6075.h"
#include <Wire.h>
#include "Config.h"

// Создаём объект сенсора
Adafruit_VEML6075 uvSensor = Adafruit_VEML6075();
UVData uvData;

// Таймер для восстановления
static unsigned long lastRecoveryTime = 0;
const unsigned long UV_RECOVERY_INTERVAL = 10000; // 10 секунд

// Инициализация датчика
void initVEML6075() {
    Serial.print(F("Initializing VEML6075 UV sensor... "));
    
    Wire.begin();
    delay(100);
    
    // Инициализация пина компаратора
    pinMode(UV_COMPARATOR_PIN, OUTPUT);
    digitalWrite(UV_COMPARATOR_PIN, LOW);
    uvData.comparatorState = false;
    
    if (!uvSensor.begin()) {
        Serial.println(F("FAILED! Check wiring."));
        
        // Сканирование I2C шины для диагностики
        Serial.println(F("Scanning I2C bus..."));
        byte error, address;
        int nDevices = 0;
        
        for(address = 1; address < 127; address++ ) {
            Wire.beginTransmission(address);
            error = Wire.endTransmission();
            
            if (error == 0) {
                Serial.print(F("Found device at 0x"));
                if (address < 16) Serial.print("0");
                Serial.print(address, HEX);
                Serial.println();
                nDevices++;
            }
        }
        
        if (nDevices == 0) {
            Serial.println(F("No I2C devices found"));
        } else {
            // Если есть устройства, но не VEML6075, пробуем другой адрес
            Serial.println(F("Trying alternative init..."));
            delay(500);
            if (uvSensor.begin()) {
                Serial.println(F("OK on second attempt!"));
                uvData.sensorOK = true;
            } else {
                uvData.sensorOK = false;
            }
        }
    } else {
        uvData.sensorOK = true;
        Serial.println(F("OK"));
    }
    
    Serial.print(F("UV comparator pin initialized on "));
    Serial.println(UV_COMPARATOR_PIN);
}

// Принудительный сброс датчика
void resetUVSensor() {
    Serial.println(F("Resetting UV sensor..."));
    uvData.sensorOK = false;
    uvData.valid = false;
    
    // Переинициализируем I2C
    Wire.end();
    delay(100);
    Wire.begin();
    delay(100);
    
    if (uvSensor.begin()) {
        uvData.sensorOK = true;
        uvData.errorCount = 0;
        Serial.println(F("UV sensor recovered"));
    } else {
        Serial.println(F("UV sensor recovery failed"));
    }
}

// Установка уставки UVB
void setUVBThreshold(float threshold) {
    if (threshold < 0) threshold = 0;
    if (threshold > 100) threshold = 100;
    
    if (abs(uvData.uvbThreshold - threshold) > 0.01) {
        uvData.uvbThreshold = threshold;
        Serial.print(F("UVB threshold set to: "));
        Serial.println(threshold);
        updateUVComparator();
    }
}

// Обновление компаратора UVB
void updateUVComparator() {
    if (!uvData.sensorOK || !uvData.valid) {
        if (uvData.comparatorState) {
            digitalWrite(UV_COMPARATOR_PIN, LOW);
            uvData.comparatorState = false;
            Serial.println(F("UV comparator: OFF (sensor error)"));
        }
        return;
    }
    
    bool newState = (uvData.uvb < uvData.uvbThreshold);
    
    if (newState != uvData.comparatorState) {
        if (newState && uvData.uvb < uvData.uvbThreshold - 0.1) {
            digitalWrite(UV_COMPARATOR_PIN, HIGH);
            uvData.comparatorState = true;
            Serial.print(F("UV comparator: ON (UVB="));
            Serial.print(uvData.uvb, 2);
            Serial.print(F(" < "));
            Serial.print(uvData.uvbThreshold, 2);
            Serial.println(F(")"));
        } 
        else if (!newState && uvData.uvb > uvData.uvbThreshold + 0.1) {
            digitalWrite(UV_COMPARATOR_PIN, LOW);
            uvData.comparatorState = false;
            Serial.print(F("UV comparator: OFF (UVB="));
            Serial.print(uvData.uvb, 2);
            Serial.print(F(" >= "));
            Serial.print(uvData.uvbThreshold, 2);
            Serial.println(F(")"));
        }
    }
}

// Обновление данных с датчика
void updateVEML6075() {
    unsigned long now = millis();
    
    if (!uvData.sensorOK) {
        // Пробуем восстановить каждые 10 секунд
        if (now - lastRecoveryTime > UV_RECOVERY_INTERVAL) {
            lastRecoveryTime = now;
            resetUVSensor();
        }
        return;
    }
    
    // Читаем данные
    float newUVA = uvSensor.readUVA();
    float newUVB = uvSensor.readUVB();
    float newUVI = uvSensor.readUVI();
    
    // Обновляем с защитой
    uvData.update(newUVA, newUVB, newUVI, now);
    
    // Отладка при ошибках
    if (!uvData.valid && uvData.errorCount > 3) {
        Serial.print(F("UV sensor error: UVA="));
        Serial.print(newUVA);
        Serial.print(F(" UVB="));
        Serial.print(newUVB);
        Serial.print(F(" UVI="));
        Serial.println(newUVI);
    } else if (uvData.valid && uvData.errorCount == 0) {
        // Периодический вывод для отладки (раз в 30 секунд)
        static unsigned long lastPrint = 0;
        if (now - lastPrint > 30000) {
            lastPrint = now;
            Serial.print(F("UV: UVA="));
            Serial.print(uvData.uva, 2);
            Serial.print(F(" UVB="));
            Serial.print(uvData.uvb, 2);
            Serial.print(F(" UVI="));
            Serial.println(uvData.uvIndex, 2);
        }
    }
    
    // Обновляем компаратор
    updateUVComparator();
}

// Проверка связи с датчиком
bool isVEML6075Connected() {
    return uvData.sensorOK && uvData.valid;
}