#ifndef MODBUS_MANAGER_H
#define MODBUS_MANAGER_H
#include <Arduino.h> 
// #include <ModbusMaster.h>

// ModbusMaster node;

// Declare functions and variables
void setupModbus(uint16_t baudRate,String serialConfig, uint8_t slaveID);

void preTransmission();
void postTransmission();

float convertToFloat(uint16_t , uint16_t );
void readModbusValues(int,int);
#endif // MODBUS_MANAGER_H
