#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <SoftwareSerial.h>
#include "VirtuinoCM.h"
#include "Config.h"

// Глобальные объекты
extern SoftwareSerial bluetooth;    //  Объявляем, что объект bluetooth существует, но создан в другом файле (.cpp)
extern VirtuinoCM virtuino;         //  Объявляем, что объект virtuino существует, но создан в другом файле (.cpp)

// Функции для работы с Bluetooth
void initBluetooth();                       //  Инициализация Bluetooth модуля при старте
void processBluetooth();                    //  Основная обработка входящих данных (вызывается в loop)
void checkBluetoothConnection();            //  Проверка, есть ли связь (таймаут)

// Фильтрация данных от Virtuino
bool isValidVirtuinoCommand(String str);    //  Проверяет, является ли строка служебной командой
bool isValidData(String str);               //  Проверяет, что данные - корректное число

// Callback функции для Virtuino
void onReceived(char variableType,      //  Тип переменной (всегда 'V' для нас)
                uint8_t variableIndex,  //  Номер виртуального пина (0,1,2...)
                String valueAsText);    //  Значение, которое прислали (как текст)

String onRequested(char variableType,   //  Тип переменной ('V')
                uint8_t variableIndex); //  Номер виртуального пина

#endif // BLUETOOTH_H