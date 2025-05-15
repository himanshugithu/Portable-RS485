#include "ModbusManager.h"
#include "constant.h"
// Create a ModbusMaster object
#include <ModbusMaster.h>
ModbusMaster node;

void setupModbus(uint16_t baudRate, String serialConfig, uint8_t slaveID) 
{  
    pinMode(RS485_EN_PIN, OUTPUT);    // RS485 Control Pin
    digitalWrite(RS485_EN_PIN, LOW);
    if (serialConfig == "8N1")
    { 
      Serial1.begin(baudRate, SERIAL_8N1, RS485_RX_PIN, RS485_TX_PIN);
      Serial.println(serialConfig);
    }
    else if (serialConfig == "8E1")
    { 
    Serial1.begin(baudRate, SERIAL_8E1, RS485_RX_PIN, RS485_TX_PIN);
    Serial.println(serialConfig);
    }
    else if (serialConfig == "8O1")
    { 
      Serial1.begin(baudRate, SERIAL_8O1, RS485_RX_PIN, RS485_TX_PIN);
      Serial.println(serialConfig);
    }
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
  uint32_t combined = ((uint32_t)high << 16) | low;
  float result;
  memcpy(&result, &combined, sizeof(result));
  return result;
}

uint64_t convertToUint64(uint16_t high1, uint16_t high2, uint16_t low1, uint16_t low2) {
  return ((uint64_t)high1 << 48) | ((uint64_t)high2 << 32) | ((uint64_t)low1 << 16) | low2;
}

uint32_t convertToUint32(uint16_t high, uint16_t low) {
  return ((uint32_t)high << 16) | low;
}

void readModbusValues(int address, int count,float scale, String dataType) 
{
  uint16_t result;
  float data=0.0;
  if(count == 1)
    {
      result = node.readHoldingRegisters(address, count);
      if (result == node.ku8MBSuccess) 
        {
          uint16_t rawValue = node.getResponseBuffer(0);
          dataFromMeter = rawValue;  // Apply scaling factor
          Serial.println("Count : 1");
          Serial.println(dataFromMeter);
        }
      else 
        {
          dataFromMeter = 0.0;
          Serial.println("Count : 1");
          Serial.println("Failed to read data");
        }  
    }
  else if(count == 2)
    {
      result = node.readHoldingRegisters(address, count);  // Read from register 40141
      if (result == node.ku8MBSuccess) 
        {
          dataFromMeter = convertToFloat(node.getResponseBuffer(1), node.getResponseBuffer(0));
          Serial.println("Count : 2");
          Serial.println(dataFromMeter);
        } 
      else 
        {
          dataFromMeter = 0.0;
          Serial.println("Count : 2");
          Serial.println("Failed to read voltage.");
        }
    }
  delay(800);
}