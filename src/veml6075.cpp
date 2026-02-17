#include "veml6075.h"
#include <Wire.h>
#include "Config.h"

// Создаём объект сенсора
Adafruit_VEML6075 uvSensor = Adafruit_VEML6075();
UVData uvData = {0.0, 0.0, 0.0, 5.0, false, false, 0}; // Уставка по умолчанию 5.0

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
        }
        
        uvData.sensorOK = false;
        return;
    }
    
    uvData.sensorOK = true;
    Serial.println(F("OK"));
    Serial.print(F("UV comparator pin initialized on "));
    Serial.println(UV_COMPARATOR_PIN);
}

// Установка уставки UVB
void setUVBThreshold(float threshold) {
    if (threshold < 0) threshold = 0;
    if (threshold > 1000) threshold = 1000; // Разумный предел
    
    if (abs(uvData.uvbThreshold - threshold) > 0.01) {
        uvData.uvbThreshold = threshold;
        Serial.print(F("UVB threshold set to: "));
        Serial.println(threshold);
        
        // Сразу обновляем компаратор при изменении уставки
        updateUVComparator();
    }
}

// Обновление компаратора UVB
void updateUVComparator() {
    if (!uvData.sensorOK) {
        // Если датчик не работает - выключаем пин
        if (uvData.comparatorState) {
            digitalWrite(UV_COMPARATOR_PIN, LOW);
            uvData.comparatorState = false;
            Serial.println(F("UV comparator: OFF (sensor error)"));
        }
        return;
    }
    
    bool newState = (uvData.uvb < uvData.uvbThreshold); // 1 если ниже уставки
    
    // Добавляем небольшой гистерезис (0.1) чтобы избежать дребезга
    if (newState != uvData.comparatorState) {
        // Проверяем с гистерезисом
        if (newState && uvData.uvb < uvData.uvbThreshold - 0.1) {
            // Включаем: значение значительно ниже уставки
            digitalWrite(UV_COMPARATOR_PIN, HIGH);
            uvData.comparatorState = true;
            Serial.print(F("UV comparator: ON (UVB="));
            Serial.print(uvData.uvb);
            Serial.print(F(" < "));
            Serial.print(uvData.uvbThreshold);
            Serial.println(F(")"));
        } 
        else if (!newState && uvData.uvb > uvData.uvbThreshold + 0.1) {
            // Выключаем: значение значительно выше уставки
            digitalWrite(UV_COMPARATOR_PIN, LOW);
            uvData.comparatorState = false;
            Serial.print(F("UV comparator: OFF (UVB="));
            Serial.print(uvData.uvb);
            Serial.print(F(" >= "));
            Serial.print(uvData.uvbThreshold);
            Serial.println(F(")"));
        }
        // В зоне гистерезиса ничего не делаем
    }
}

// Обновление данных с датчика
void updateVEML6075() {
    if (!uvData.sensorOK) return;
    
    // Читаем данные
    uvData.uva = uvSensor.readUVA();
    uvData.uvb = uvSensor.readUVB();
    uvData.uvIndex = uvSensor.readUVI();
    uvData.lastReadTime = millis();
    
    // Проверка на валидность
    if (uvData.uva < 0) uvData.uva = 0;
    if (uvData.uvb < 0) uvData.uvb = 0;
    if (uvData.uvIndex < 0) uvData.uvIndex = 0;
    
    // Обновляем компаратор
    updateUVComparator();
}

// Проверка связи с датчиком
bool isVEML6075Connected() {
    return uvData.sensorOK;
}