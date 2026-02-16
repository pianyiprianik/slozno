// Общие константы

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ==================== ВЕРСИЯ ====================
#define VERSION "2.0.0"

// ==================== ПИНЫ ====================
// Bluetooth
#define BT_RX_PIN 10        //  RX подключаем на 11 пин
#define BT_TX_PIN 11        //  TX подключаем на 10 пин

// Датчики температуры
#define ONE_WIRE_BUS_REACTOR_1 6
#define ONE_WIRE_BUS_REACTOR_2 8

// ТЭНы
#define HEATER_PIN_1 4
#define HEATER_PIN_2 3

// Генератор частоты
#define FREQ1_PIN 12
#define FREQ1_MAX 10000
#define FREQ1_TIMER Timer1

#define FREQ2_PIN 45  
#define FREQ2_MAX 5000  
#define FREQ2_TIMER Timer5

// ТРИГГЕР (новый выход)
#define AUX_PIN 46  // Пин для переменной 0/1

// Параметры плавного пуска
const int FREQ_STEP_SIZE = 8;        // Шаг увеличения частоты (Гц)
const unsigned long FREQ_STEP_INTERVAL = 60;  // Интервал между шагами (мс)
const int FREQ_RESET_VALUE = 1;        // Значение при сбросе (1 Гц)

// ==================== ТАЙМЕР ДЛЯ V30 ====================
const unsigned long MIN_TIMER_INTERVAL = 1;      // Минимум 1 секунда
const unsigned long MAX_TIMER_INTERVAL = 3600;   // Максимум 1 час (3600 секунд)
const unsigned long DEFAULT_TIMER_INTERVAL = 60; // По умолчанию 60 секунд

// ==================== ПАРАМЕТРЫ ТЕМПЕРАТУРЫ ====================
const float MIN_TARGET_TEMP = 20.00;
const float MAX_TARGET_TEMP = 40.00;
const float HYSTERESIS = 1.0;
const float TEMP_TOLERANCE = 0.5;
const float TEMP_HYSTERESIS_LOW = 0.3;      //  Нижний порог гистерезиса
const float TEMP_HYSTERESIS_HIGH = 0.7;     //  Верхний порог гистерезиса

// ==================== ТАЙМЕРЫ (в миллисекундах) ====================
const unsigned long TEMP_UPDATE_INTERVAL = 2000;        //  Обновление температуры
const unsigned long CONTROL_UPDATE_INTERVAL = 1000;     //  Управление нагревателями
const unsigned long BLUETOOTH_TIMEOUT = 30000;          //  Таймаут Bluetooth
const unsigned long HEATER_STATE_CHECK_INTERVAL = 5000; //  Проверка состояния
const unsigned long HEATER_SWITCH_DELAY = 2000;         //  Задержка переключения
const unsigned long FREQUENCY_CHANGE_DELAY = 500;       //  Задержка смены частоты

// ==================== ПАРАМЕТРЫ ГЕНЕРАТОРА ====================
const int PWM_DUTY_CYCLE = 512;     //  50% заполнения (1024/2)

// ==================== АДРЕСА EEPROM ====================
#define EEPROM_TARGET_TEMP_1 4
#define EEPROM_TEMP_CHECKSUM_1 8
#define EEPROM_TARGET_TEMP_2 20
#define EEPROM_TEMP_CHECKSUM_2 24
#define EEPROM_TARGET_FREQUENCY 40
#define EEPROM_FREQ_CHECKSUM 44
#define EEPROM_MAGIC_NUMBER 0x5A    //0x5A = 90 в десятичной

#endif // CONFIG_H