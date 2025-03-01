# Energy Meter Monitoring System

An embedded system for monitoring energy meters using Modbus RTU protocol with OLED display interface.

## Features

- Menu-driven OLED interface
- Configurable communication parameters:
  - Baud rate (9600-115200)
  - Serial configuration (8N1/8E1/8O1)
  - Meter ID selection
  - Function Code (holding and input register)
- Register configuration:
  - Register Value adjustment
  - Step size configuration
  - Register count setting
- Real-time data display

## Hardware Requirements

- ESP32 microcontroller
- SH1106 128x64 OLED Display
- 6x Tactile buttons
- 10KΩ resistors (for button pull-ups)
- Rs485 ti TTL converter
- Jumper wires
- Battery * 2
- Zero PCB


## Connections

| Component    | ESP32 Pin |
|--------------|-----------|
| OLED SDA     | GPIO 21   |
| OLED SCL     | GPIO 22   |
| BTN_UP       | GPIO 32   |
| BTN_DOWN     | GPIO 33   |
| BTN_LEFT     | GPIO 34   |
| BTN_RIGHT    | GPIO 35   |
| BTN_SELECT   | GPIO 18   |
| BTN_BACK     | GPIO 19   |
| RS485_TX_PIN | GPIO 17   |
| RS485_RX_PIN | GPIO 16   |
| RS485_EN_PIN | GPIO 4    |


## Installation

1. **Prerequisites**
   - Arduino IDE 2.x
   - ESP32 Board Support Package
   - Required libraries:
     - `Adafruit_SH110X`
     - `Wire`
     - `Arduino.h`
     - `ModbusMaster.h`

2. **Setup**
   ```bash
   git clone https://github.com/yourusername/energy-meter-monitor.git
   cd portable_RS485