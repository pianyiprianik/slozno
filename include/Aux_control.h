#ifndef AUX_CONTROL_H
#define AUX_CONTROL_H

#include <Arduino.h>
#include "Config.h"

// Класс для дополнительного пина (более защищённый, чем struct)
class AuxPin {
private:
    const int _pin;
    bool _targetState;      // Целевое состояние из Virtuino
    bool _currentState;      // Текущее физическое состояние
    bool _initialized;
    const char* _name;
    
public:
    // Конструктор
    AuxPin(int p, const char* n) : 
        _pin(p), 
        _targetState(false), 
        _currentState(false), 
        _initialized(false), 
        _name(n) {}
    
    // Инициализация
    void init() {
        pinMode(_pin, OUTPUT);
        digitalWrite(_pin, LOW);
        _currentState = false;
        _targetState = false;  // Важно: начинаем с известного состояния
        _initialized = true;
        
        Serial.print(F("Aux pin "));
        Serial.print(_name);
        Serial.print(F(" initialized on pin "));
        Serial.println(_pin);
    }
    
    // Установка целевого состояния (вызывается из Bluetooth)
    void setTarget(bool state) {
        // Защита от дребезга: только если реально изменилось
        if (_targetState != state) {
            _targetState = state;
            
            Serial.print(F("Aux "));
            Serial.print(_name);
            Serial.print(F(" target changed to: "));
            Serial.println(_targetState ? F("HIGH") : F("LOW"));
            
            // НЕ применяем сразу - дожидаемся update() в главном цикле
        }
    }
    
    // Применение состояния к пину (вызывается из loop)
    void update() {
        if (!_initialized) return;
        
        // Если целевое состояние отличается от текущего
        if (_currentState != _targetState) {
            // Применяем новое состояние к пину
            digitalWrite(_pin, _targetState ? HIGH : LOW);
            _currentState = _targetState;
            
            Serial.print(F("Aux "));
            Serial.print(_name);
            Serial.print(F(" pin "));
            Serial.print(_pin);
            Serial.println(_targetState ? F(" ON") : F(" OFF"));
        }
    }
    
    // Получить текущее состояние
    bool getState() const {
        return _currentState;
    }
    
    // Получить целевое состояние
    bool getTargetState() const {
        return _targetState;
    }
    
    // Проверить, изменилось ли состояние (для внешних триггеров)
    bool hasChanged() const {
        return _currentState != _targetState;
    }
};

// Глобальные объекты - объявляем как extern
extern AuxPin aux1;  // Пин 48 (V30)
extern AuxPin aux2;  // Пин 47 (V31)

#endif // AUX_CONTROL_H