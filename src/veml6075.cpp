#include "veml6075.h"
#include <Wire.h>

// Создаём объект сенсора
Adafruit_VEML6075 uvSensor = Adafruit_VEML6075();
UVData uvData = {0.0, 0.0, 0.0, false, 0};

// Инициализация датчика
void initVEML6075() {
    Serial.print(F("Initializing VEML6075 UV sensor on pins 20(SDA), 21(SCL)... "));
    
    // Инициализируем I2C шину
    Wire.begin();
    delay(100);  // Даем шине стабилизироваться
    
    if (!uvSensor.begin()) {
        Serial.println(F("FAILED! Check wiring."));
        
        // Дополнительная диагностика - сканирование I2C шины
        Serial.println(F("Scanning I2C bus for devices..."));
        byte error, address;
        int nDevices = 0;
        
        for(address = 1; address < 127; address++ ) {
            Wire.beginTransmission(address);
            error = Wire.endTransmission();
            
            if (error == 0) {
                Serial.print(F("Found device at address 0x"));
                if (address < 16) Serial.print("0");
                Serial.print(address, HEX);
                Serial.println(" !");
                nDevices++;
            }
        }
        
        if (nDevices == 0) {
            Serial.println(F("No I2C devices found. Check wiring and pull-up resistors."));
        } else {
            Serial.println(F("I2C scan complete. Device found, but VEML6075 not responding."));
        }
        
        uvData.sensorOK = false;
        return;
    }
    
    // Настройка датчика
    uvData.sensorOK = true;
    Serial.println(F("OK"));
}

// Обновление данных с датчика
void updateVEML6075() {
    if (!uvData.sensorOK) return;
    
    // Читаем данные
    uvData.uva = uvSensor.readUVA();
    uvData.uvb = uvSensor.readUVB();
    uvData.uvIndex = uvSensor.readUVI();
    uvData.lastReadTime = millis();
    
    // Простая проверка на валидность (неотрицательные значения)
    if (uvData.uva < 0) uvData.uva = 0;
    if (uvData.uvb < 0) uvData.uvb = 0;
    if (uvData.uvIndex < 0) uvData.uvIndex = 0;
}

// Проверка связи с датчиком
bool isVEML6075Connected() {
    return uvData.sensorOK;
}