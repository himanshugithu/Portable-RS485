#include "ModbusManager.h"
#include "constant.h"
#include <ModbusMaster.h>

ModbusMaster node;

void setupModbus(uint16_t baudRate, const String& serialConfig, uint8_t slaveID) {
  pinMode(RS485_EN_PIN, OUTPUT);
  digitalWrite(RS485_EN_PIN, LOW);

  // Configure Serial1 based on provided serial configuration
  struct SerialConfig {
    const char* config;
    uint32_t mode;
  };

  const SerialConfig serialModes[] = {
    { "8N1", SERIAL_8N1 },
    { "8E1", SERIAL_8E1 },
    { "8O1", SERIAL_8O1 }
  };

  for (const auto& mode : serialModes) {
    if (serialConfig == mode.config) {
      Serial1.begin(baudRate, mode.mode, RS485_RX_PIN, RS485_TX_PIN);
      Serial.println("Serial Config: " + serialConfig);
      break;
    }
  }

  node.begin(slaveID, Serial1);
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);
  Serial.println("Modbus Initialized");
}

void preTransmission() {
  digitalWrite(RS485_EN_PIN, HIGH);
}

void postTransmission() {
  digitalWrite(RS485_EN_PIN, LOW);
}

template<typename T>
T convertToType(uint16_t high, uint16_t low) {
  uint32_t combined = ((uint32_t)high << 16) | low;
  T result;
  memcpy(&result, &combined, sizeof(result));
  return result;
}

void readModbusValues(int address, int count, float scale, const String& dataType, const String& functionCode) {
  Serial.println("Address: " + String(address) + ", Count: " + String(count) + ", Scale : " + String(scale) + ", Data Type : " + String(dataType) + ", Function Code : " + String(functionCode));
  uint16_t result;
  if (functionCode == "0x03") {
    result = node.readHoldingRegisters(address, count);
    Serial.println("Holding Register select");
  } else {
    result = node.readInputRegisters(address, count);
    Serial.println("Input Register select");
  }
  if (result != node.ku8MBSuccess) {
    Serial.println("Error: Failed to read data");
    dataFromMeter = 0.0;
    return;
  }

  if (count == 1) {
    dataFromMeter = node.getResponseBuffer(0) * scale;
  } else if (count == 2) {
    uint16_t high = node.getResponseBuffer(1);
    uint16_t low = node.getResponseBuffer(0);

    if (dataType == "Float") {
      dataFromMeter = convertToType<float>(high, low);
    } else if (dataType == "UINT") {
      dataFromMeter = convertToType<uint32_t>(high, low);
    } else if (dataType == "Long") {
      dataFromMeter = convertToType<long>(high, low);
    } else {
      Serial.println("Error: Invalid data type");
      return;
    }
  } else {
    Serial.println("Error: Unsupported count value");
    return;
  }

  Serial.println("Data: " + String(dataFromMeter));
  delay(800);
}
