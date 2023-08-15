#include "eeprom.hpp"
#include <EEPROM.h>

void beginEEPROM() {
    // the EEPROM on esp8266 has 512 available bytes
    EEPROM.begin(512);
}

void saveToEEPROM(int address, const char* buffer, size_t size) {
    // the last byte is ignored as it is used as the null terminator
    for (int i=0; i<size-1; ++i, ++address) {
        EEPROM.write(address, buffer[i]);
    }
    EEPROM.commit();
}

void saveToEEPROM(int address, int value) {
    char* buffer = (char*)&value;
    for (int i=0; i<sizeof(int); ++i, ++address) {
        EEPROM.write(address, buffer[i]);
    }
    EEPROM.commit();
}

void readFromEEPROM(int address, char* buffer, size_t size) {
    for (int i=0; i<size-1; ++i, ++address) {
        buffer[i] = EEPROM.read(address);
    }
    buffer[size-1] = '\0'; // set the null terminator as last byte
}

void readFromEEPROM(int address, int value) {
    char* buffer = (char*)&value;
    for (int i=0; i<sizeof(int); ++i, ++address) {
        buffer[i] = EEPROM.read(address);
    }
}
