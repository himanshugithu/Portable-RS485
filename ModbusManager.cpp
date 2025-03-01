#include "ModbusManager.h"
#include "constant.h"
// Create a ModbusMaster object
#include <ModbusMaster.h>
ModbusMaster node;

void setupModbus(uint16_t baudRate, String serialConfig, uint8_t slaveID) 
{
    
    pinMode(RS485_EN_PIN, OUTPUT);    // RS485 Control Pin
    digitalWrite(RS485_EN_PIN, LOW);
    if (serialConfig == "8N1")      Serial1.begin(baudRate, SERIAL_8N1, RS485_RX_PIN, RS485_TX_PIN);
    else if (serialConfig == "8E1") Serial1.begin(baudRate, SERIAL_8E1, RS485_RX_PIN, RS485_TX_PIN);
    else if (serialConfig == "8O1") Serial1.begin(baudRate, SERIAL_8O1, RS485_RX_PIN, RS485_TX_PIN);
    node.begin(slaveID, Serial1);
    node.preTransmission(preTransmission);
    node.postTransmission(postTransmission);
    Serial.println("complete initilize");

}



void preTransmission() // Enable RS485 transmit mode
{
  digitalWrite(RS485_EN_PIN, HIGH);
}

void postTransmission() // Enable RS485 receive mode
{
  digitalWrite(RS485_EN_PIN, LOW);
}

float convertToFloat(uint16_t high, uint16_t low) {
  Serial.println("ithh alo manje jhal");
  uint32_t combined = ((uint32_t)high << 16) | low;
  float result;
  memcpy(&result, &combined, sizeof(result));
  return result;
}

void readModbusValues(int address, int count) {
  uint8_t result;
  float data=0.0;
  Serial.println(address);
  Serial.println(count);
  result = node.readHoldingRegisters(address, count);  // Read from register 40141
  if (result == node.ku8MBSuccess) {
    dataFromMeter = convertToFloat(node.getResponseBuffer(1), node.getResponseBuffer(0));
    Serial.println(dataFromMeter);
  } else {
    dataFromMeter = 0.0;
    Serial.println("Failed to read voltage.");
  }
  delay(800);
}