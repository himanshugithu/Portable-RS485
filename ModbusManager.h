#ifndef MODBUS_MANAGER_H
#define MODBUS_MANAGER_H

#include <Arduino.h>

// Function prototypes
void setupModbus(uint16_t baudRate, const String& serialConfig, uint8_t slaveID);
void preTransmission();
void postTransmission();
void readModbusValues(int address, int count, float scale, const String& dataType);

// Template for data conversion
template <typename T>
T convertToType(uint16_t high, uint16_t low);

#endif // MODBUS_MANAGER_H
