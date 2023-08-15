#pragma once
#ifndef _EEPROM_HPP_
#define _EEPROM_HPP_

#include <Arduino.h>

void beginEEPROM();
void saveToEEPROM(int address, const char* buffer, size_t size);
void saveToEEPROM(int address, int value);
void readFromEEPROM(int address, char* buffer, size_t size);
void readFromEEPROM(int address, int value);

#endif // _EEPROM_HPP_