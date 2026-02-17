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
#define AUX1_PIN 46  // Пин для переменной 0/1
#define AUX2_PIN 47  // Пин для переменной V31 (0/1)
#define UV_COMPARATOR_PIN 43 

#define TIMER_PIN 44

// Минимальные и максимальные значения для таймера (в секундах)
const unsigned int MIN_TIMER_SECONDS = 1;
const unsigned int MAX_TIMER_SECONDS = 7200; // 1 час

// Настройки времени интеграции (в миллисекундах)
#define UV_INTEGRATION_TIME 100  // 50, 100, 200, 400 мс

// Интервал чтения UV датчика (мс)
#define UV_READ_INTERVAL 2000

// Количество ошибок для отключения датчика
#define UV_MAX_ERRORS 5

// Параметры плавного пуска
const int FREQ_STEP_SIZE = 15;        // Шаг увеличения частоты (Гц)
const unsigned long FREQ_STEP_INTERVAL = 40;  // Интервал между шагами (мс)
const int FREQ_RESET_VALUE = 1;        // Значение при сбросе (1 Гц)

// ==================== ЗАЩИТА ПЕРЕМЕННЫХ ====================
// Минимальные и максимальные значения для всех переменных
const float MIN_UVB_THRESHOLD = 0.0;
const float MAX_UVB_THRESHOLD = 100.0;
const float DEFAULT_UVB_THRESHOLD = 5.0;

const unsigned int DEFAULT_LOW_SECONDS = 5;
const unsigned int DEFAULT_HIGH_SECONDS = 5;

// Магические числа для защиты EEPROM (усилим)
const uint16_t EEPROM_MAGIC_V44 = 0xA1B2;
const uint16_t EEPROM_MAGIC_V50 = 0xA3B4;
const uint16_t EEPROM_MAGIC_V51 = 0xA5B6;
const uint16_t EEPROM_MAGIC_V52 = 0xA7B8;

// ==================== ТАЙМЕР ДЛЯ V30 ====================
const unsigned long MIN_TIMER_INTERVAL = 1;      // Минимум 1 секунда
const unsigned long MAX_TIMER_INTERVAL = 3600;   // Максимум 1 час (3600 секунд)
const unsigned long DEFAULT_TIMER_INTERVAL = 60; // По умолчанию 60 секунд

// ==================== ПАРАМЕТРЫ ТЕМПЕРАТУРЫ ====================
const float MIN_TARGET_TEMP = 20.00;
const float MAX_TARGET_TEMP = 40.00;
const float HYSTERESIS = 1.0;
const float TEMP_TOLERANCE = 0.5;
const float TEMP_HYSTERESIS_LOW = 0.2;      //  Нижний порог гистерезиса
const float TEMP_HYSTERESIS_HIGH = 0.2;     //  Верхний порог гистерезиса

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
#define EEPROM_TARGET_FREQUENCY_2 50
#define EEPROM_FREQ2_CHECKSUM 54

// ==================== EEPROM ДЛЯ UV ДАТЧИКА (ЗАГОТОВКА) ====================
// Сохранять калибровочные значения, если понадобится
// #define EEPROM_UV_CALIB 60

#define EEPROM_MAGIC_NUMBER 0x5A    //0x5A = 90 в десятичной

#endif // CONFIG_