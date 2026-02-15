#include "Bluetooth.h"
#include "HeaterController.h"
#include "FrequencyGenerator.h"
#include "Eeprom_utils.h"
#include <Arduino.h>

// Создаем объекты
SoftwareSerial bluetooth(BT_RX_PIN, BT_TX_PIN);
VirtuinoCM virtuino;

extern Heater heater1;
extern Heater heater2;

// Инициализация Bluetooth
void initBluetooth() {
    bluetooth.begin(9600);
    virtuino.begin(onReceived, onRequested, 256);
    
    Serial.print(F("Bluetooth initialized on pins RX="));
    Serial.print(BT_RX_PIN);
    Serial.print(F(", TX="));
    Serial.println(BT_TX_PIN);
}

// Обработка входящих данных
void processBluetooth() {
    unsigned long startTime = millis();
    
    while (bluetooth.available()) {
        if (millis() - startTime > 5) break;
        
        char tempChar = bluetooth.read();
        if (tempChar == CM_START_CHAR) {
            String data = bluetooth.readStringUntil(CM_END_CHAR);
            if (data.length() > 0 && data.length() < 50) {
                virtuino.readBuffer = CM_START_CHAR;
                virtuino.readBuffer += data;
                virtuino.readBuffer += CM_END_CHAR;
                
                String* response = virtuino.getResponse();
                if (response != nullptr && response->length() > 0) {
                    bluetooth.print(*response);
                    extern unsigned long lastBluetoothCheck;
                    lastBluetoothCheck = millis();
                }
            }
            break;
        }
    }
}

// Проверка наличия связи с Bluetooth
extern unsigned long lastBluetoothCheck;

void checkBluetoothConnection() {
    if (millis() - lastBluetoothCheck > BLUETOOTH_TIMEOUT) {
        static bool warningShown = false;
        if (!warningShown) {
            Serial.println(F("WARNING: No Bluetooth data received for 30 seconds"));
            warningShown = true;
        }
    } else {
        static bool warningShown = false;
        warningShown = false;
    }
}

// Проверка команд Virtuino
bool isValidVirtuinoCommand(String str) {
    if (str.startsWith("!Q") && str.indexOf("=?") > 0) {
        return false;
    }
    return true;
}

// Проверка валидности данных
bool isValidData(String str) {
    if (str.length() == 0 || str.length() > 7) return false;
    
    bool hasDecimal = false;
    bool hasDigit = false;
    
    for (int i = 0; i < str.length(); i++) {
        if (str[i] == '-' && i == 0) continue;
        if (str[i] == '.' && !hasDecimal) {
            hasDecimal = true;
            continue;
        }
        if (isDigit(str[i])) {
            hasDigit = true;
            continue;
        }
        return false;
    }
    
    return hasDigit;
}

// Обработка полученных данных от Virtuino
void onReceived(char variableType, uint8_t variableIndex, String valueAsText) {
    if (variableType == 'V') {
        
        if (!isValidVirtuinoCommand(valueAsText)) {
            Serial.print(F("Virtuino debug: "));
            Serial.println(valueAsText);
            return;
        }
        
        if (!isValidData(valueAsText)) {
            Serial.print(F("Invalid data received: "));
            Serial.println(valueAsText);
            return;
        }
        
        float floatValue = valueAsText.toFloat();
        int intValue = valueAsText.toInt();
        
        switch (variableIndex) {
            // Емкость 1
            case 0:
                floatValue = constrain(floatValue, MIN_TARGET_TEMP, MAX_TARGET_TEMP);
                if (abs(heater1.targetTemp - floatValue) > 0.01) {
                    heater1.targetTemp = floatValue;
                    Serial.print(F("Target temperature 1 set to: "));
                    Serial.print(heater1.targetTemp, 1);
                    Serial.println(F("°C"));
                    saveTargetTemps(heater1.targetTemp, heater2.targetTemp);
                }
                break;
                
            case 7:
                heater1.permission = (intValue == 1);
                Serial.print(F("Heating permission 1: "));
                Serial.println(heater1.permission ? F("ON") : F("OFF"));
                break;
                
            // Емкость 2
            case 10:
                floatValue = constrain(floatValue, MIN_TARGET_TEMP, MAX_TARGET_TEMP);
                if (abs(heater2.targetTemp - floatValue) > 0.01) {
                    heater2.targetTemp = floatValue;
                    Serial.print(F("Target temperature 2 set to: "));
                    Serial.print(heater2.targetTemp, 1);
                    Serial.println(F("°C"));
                    saveTargetTemps(heater1.targetTemp, heater2.targetTemp);
                }
                break;
                
            case 17:
                heater2.permission = (intValue == 1);
                Serial.print(F("Heating permission 2: "));
                Serial.println(heater2.permission ? F("ON") : F("OFF"));
                break;
                
            // Частота
            case 20: // V20 - Частота первого генератора
                intValue = constrain(intValue, 0, FREQ1_MAX);
                if (gen1.frequency != intValue) {
                    if (millis() - gen1.lastChangeTime >= FREQUENCY_CHANGE_DELAY) {
                        setGenerator1(intValue);
                        gen1.lastChangeTime = millis();
                        saveFrequencies();
                    }
                }
                break;

            case 21: // V21 - Частота второго генератора
                intValue = constrain(intValue, 0, FREQ2_MAX);
                if (gen2.frequency != intValue) {
                    if (millis() - gen2.lastChangeTime >= FREQUENCY_CHANGE_DELAY) {
                        setGenerator2(intValue);
                        gen2.lastChangeTime = millis();
                        saveFrequencies();
                    }
                }
                break;
        }
    }
}

// Отправка данных в Virtuino
String onRequested(char variableType, uint8_t variableIndex) {
    if (variableType == 'V') {
        switch (variableIndex) {
            case 0: return String(heater1.targetTemp, 1);
            case 1: return String(heater1.currentTemp, 1);
            case 6: return String(heater1.alarm ? 1 : 0);
            case 7: return String(heater1.permission ? 1 : 0);
            case 8: return String(heater1.state ? 1 : 0);
            case 10: return String(heater2.targetTemp, 1);
            case 11: return String(heater2.currentTemp, 1);
            case 16: return String(heater2.alarm ? 1 : 0);
            case 17: return String(heater2.permission ? 1 : 0);
            case 18: return String(heater2.state ? 1 : 0);
            //case 20: return String(targetFrequency);
            case 21: return String(gen2.frequency);  // V21 - частота gen2
            case 22: return String(gen2.active ? 1 : 0);  // V22 - состояние gen2
        }
    }
    return "";
}