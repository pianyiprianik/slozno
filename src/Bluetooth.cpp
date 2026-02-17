#include "Bluetooth.h"
#include "HeaterController.h"
#include "FrequencyGenerator.h"
#include "Aux_control.h"
#include "Eeprom_utils.h"
#include <Arduino.h>
#include "veml6075.h"
#include "Timer_pin.h"

unsigned int v30TimerInterval = DEFAULT_TIMER_INTERVAL;

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
    static unsigned long lastWarningTime = 0;
    
    if (millis() - lastBluetoothCheck > BLUETOOTH_TIMEOUT) {
        if (millis() - lastWarningTime > 30000) {  // Предупреждение раз в 30 сек
            lastWarningTime = millis();
            Serial.println(F("WARNING: No Bluetooth data received"));
        }
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
    
    for (unsigned int i = 0; i < str.length(); i++) {  // unsigned int
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

        // Дополнительная защита: проверяем разумность значения
        if (abs(floatValue) > 10000) {  // Защита от космических чисел
            Serial.print(F("Value too large: "));
            Serial.println(floatValue);
            return;
        }
        
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
                if (gen1.targetFrequency != intValue) {
                    if (millis() - gen1.lastChangeTime >= FREQUENCY_CHANGE_DELAY) {
                        setGenerator1(intValue);
                        gen1.lastChangeTime = millis();
                        saveFrequencies();
                    }
                }
                break;

            case 21: // V21 - Частота второго генератора
                intValue = constrain(intValue, 0, FREQ2_MAX);
                if (gen2.targetFrequency != intValue) {
                    if (millis() - gen2.lastChangeTime >= FREQUENCY_CHANGE_DELAY) {
                        setGenerator2(intValue);
                        gen2.lastChangeTime = millis();
                        saveFrequencies();
                    }
                }
                break;
            
            // ===== ДОПОЛНИТЕЛЬНЫЙ ПИН 46 =====
            case 30: // V30 - Управление пином 46 (0 или 1)
                                // Проверка: должно быть 0 или 1
                if (intValue == 0 || intValue == 1) {
                    bool newState = (intValue == 1);
                    
                    // Сохраняем предыдущее состояние для проверки
                    static bool lastAux1State = false;
                    
                    // Устанавливаем целевое состояние
                    aux1.setTarget(newState);
                    
                    // Если состояние ИЗМЕНИЛОСЬ, сбрасываем генератор
                    if (lastAux1State != newState) {
                        lastAux1State = newState;
                        
                        Serial.print(F("AUX1 changed to "));
                        Serial.println(newState ? F("ON") : F("OFF"));
                        
                        // Сбрасываем второй генератор
                        if (gen2.targetFrequency > 0) {
                            gen2.reset();
                            Serial.println(F("Generator 2 reset due to AUX1 change"));
                        }
                    }
                } else {
                    Serial.print(F("Error: V30 value must be 0 or 1, received: "));
                    Serial.println(intValue);
                }
                break;

                case 31: // V31 - Интервал таймера V30 (в секундах)
                    intValue = constrain(intValue, MIN_TIMER_INTERVAL, MAX_TIMER_INTERVAL);
                    if (v30TimerInterval != (unsigned int)intValue) {
                        v30TimerInterval = intValue;
                        Serial.print(F("V30 timer interval set to: "));
                        Serial.print(v30TimerInterval);
                        Serial.println(F(" seconds"));
            
                        // Сохраняем в EEPROM (если нужно)
                        saveTimerInterval();
                    }
                    break;

                case 32: // V30 - Управление пином 46 (0 или 1)
                    // Проверка: должно быть 0 или 1
                    if (intValue == 0 || intValue == 1) {
                        bool newState = (intValue == 1);
                    
                        // Сохраняем предыдущее состояние для проверки
                        static bool lastAux2State = false;
                    
                        // Устанавливаем целевое состояние
                        aux2.setTarget(newState);
                    
                        // Если состояние ИЗМЕНИЛОСЬ, сбрасываем генератор
                        if (lastAux2State != newState) {
                            lastAux2State = newState;
                        
                            Serial.print(F("AUX2 changed to "));
                            Serial.println(newState ? F("ON") : F("OFF"));
                        
                            // Сбрасываем второй генератор
                            if (gen2.targetFrequency > 0) {
                                gen2.reset();
                                Serial.println(F("Generator 2 reset due to AUX2 change"));
                            }
                        }
                        } else {
                            Serial.print(F("Error: V31 value must be 0 or 1, received: "));
                            Serial.println(intValue);
                            }
                    break;

                case 44: // V44 - Уставка UVB
                    if (!uvData.setThreshold(floatValue)) {
                        Serial.print(F("V44 out of range: "));
                        Serial.println(floatValue);
                    } 
                    break;

                case 50:
                    if (!timerPin.setLowSeconds(intValue)) {
                        Serial.print(F("V50 out of range: "));
                        Serial.println(intValue);
                    }
                    break;
                
                // ===== ТАЙМЕР V51 =====
                case 51:
                    if (!timerPin.setHighSeconds(intValue)) {
                        Serial.print(F("V51 out of range: "));
                        Serial.println(intValue);
                    }
                    break;
                
                // ===== ТАЙМЕР V52 =====
                case 52:
                    if (intValue == 0 || intValue == 1) {
                        timerPin.setEnabled(intValue == 1);
                    } else {
                        Serial.print(F("V52 must be 0 or 1, got: "));
                        Serial.println(intValue);
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
            case 20: return String(gen1.targetFrequency);
            case 21: return String(gen2.targetFrequency);
            case 22: return String(gen2.currentFrequency);  // Текущая частота
            case 30: return String(aux1.getState() ? 1 : 0);
            case 31: return String(v30TimerInterval);  // Текущий интервал
            case 32: return String(aux2.getState() ? 1 : 0);
            case 33: return String(millis() / 1000);   // Прошедшее время (для отладки)
            
            // ===== UV ДАТЧИК =====
            case 40: return String(uvData.uva, 2);  
            case 41: return String(uvData.uvb, 2);
            case 42: return String(uvData.uvIndex, 2);
            case 43: return String(uvData.sensorOK ? 1 : 0);
            case 44: return String(uvData.uvbThreshold, 2);  
            case 45: return String(uvData.comparatorState ? 1 : 0);

            // ===== ТАЙМЕРНЫЙ ПИН =====
            case 50: return String(timerPin.lowSeconds);     // Текущее LOW время
            case 51: return String(timerPin.highSeconds);    // Текущее HIGH время
            case 52: return String(timerPin.enabled ? 1 : 0); // Состояние таймера
            case 53: return String(timerPin.getState() ? 1 : 0); // Текущее состояние пина
        }
    }
    return "";
}