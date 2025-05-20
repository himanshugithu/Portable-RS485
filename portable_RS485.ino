#include <Wire.h>
#include <Adafruit_SH110X.h>
#include "ModbusManager.h"
#include "constant.h"

// OLED display dimensions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// OLED display object
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

// Button pins
#define BTN_UP 32
#define BTN_DOWN 33
#define BTN_LEFT 34
#define BTN_RIGHT 35
#define BTN_SELECT 19
#define BTN_BACK 18


// Menu states
enum MenuState {
  MAIN_MENU,
  BAUD_RATE_MENU,
  SERIAL_CONFIG_MENU,
  METER_ID_MENU,
  FUNCTION_CODE_MENU,
  FINAL_SELECTION_MENU,
  DATA_TYPE_MENU,       // Moved before register menu
  REGISTER_VALUE_MENU,  // Now comes after data type
  INITIALIZE_SERIAL_MENU
};
MenuState currentMenu = MAIN_MENU;

// Change the main menu options
const char* mainMenu[] = { "Portable RS485" }; // Changed from Energy Meter/Solar Inverter
int mainMenuLength = sizeof(mainMenu) / sizeof(mainMenu[0]); // This will now be 1
int currentMainMenuIndex = 0;

// Baud rate options
const int baudRates[] = { 9600, 19200, 38400, 57600, 115200 };
int baudRateLength = sizeof(baudRates) / sizeof(baudRates[0]);
int currentBaudIndex = 0;
int baudRate = 0;
// Serial configurations
const char* serialConfigs[] = { "8N1","8O1","8E1" };
int serialConfigLength = sizeof(serialConfigs) / sizeof(serialConfigs[0]);
int currentSerialConfigIndex = 0;
// Add these variables at the top
int scaleIndex = 0;
const float scaleValues[] = {0.1, 0.01, 0.001};
const int scaleValuesCount = sizeof(scaleValues) / sizeof(scaleValues[0]);

const char* functionCodes[] = { "0x03", "0x04" };
int functionCodeLength = sizeof(functionCodes) / sizeof(functionCodes[0]);
int currentFunctionCodeIndex = 0;

const char* dataTypes[] = { "Float", "UINT", "Long" };
int currentDataTypeIndex = 0;
const int dataTypeLength = sizeof(dataTypes) / sizeof(dataTypes[0]);

int meterId = 1;
int registerValue = 1;
int registerStep = 1;
int registerCount = 1;            // Added count variable
int registerConfigSelection = 0;  // 0: Value, 1: Step, 2: Count (changed to integer)
float dataFromMeter = 0.0;
float registerScale = 1.0;
int startIndexRegister = 0;
const int itemsPerPageRegister = 3;
String selectedDataType;


// Function prototypes
void showMainMenu();
void showBaudRateSelection();
void showSerialConfigSelection();
void showMeterIdSelection();
void showFunctionCodeSelection();
void showRegisterValueSelection();
void showFinalSelection();

void setup() {
  Serial.begin(115200);

  // Initialize OLED
  if (!display.begin(0x3C, true)) {
    Serial.println(F("SH110X allocation failed"));
    while (true)
      ;
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.display();

  // Configure buttons as inputs
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_SELECT, INPUT_PULLUP);
  pinMode(BTN_BACK, INPUT_PULLUP);

  // Show the main menu
  showMainMenu();
}

void loop() {
  if (!digitalRead(BTN_UP)) {
    Serial.println("BTN_UP Pressed");
    if (currentMenu == MAIN_MENU) {
      // currentMainMenuIndex = (currentMainMenuIndex - 1 + mainMenuLength) % mainMenuLength;
      // showMainMenu();
    } else if (currentMenu == REGISTER_VALUE_MENU) {
      // Wrap selection upward
      registerConfigSelection = (registerConfigSelection - 1 + 4) % 4;  // 4 items total
      // Adjust start index to keep selection visible
      startIndexRegister = (registerConfigSelection >= (itemsPerPageRegister - 1))
                             ? (registerConfigSelection - (itemsPerPageRegister - 1))
                             : 0;
      showRegisterValueSelection();
    }
    delay(200);
  }

if (!digitalRead(BTN_DOWN)) {
  Serial.println("BTN_DOWN Pressed");
  if (currentMenu == MAIN_MENU) {
    // currentMainMenuIndex = (currentMainMenuIndex + 1) % mainMenuLength;
    // showMainMenu();
  } else if (currentMenu == REGISTER_VALUE_MENU) {
    // Wrap selection downward
    registerConfigSelection = (registerConfigSelection + 1) % 4; // 4 items total
    // Adjust start index to keep selection visible
    startIndexRegister = (registerConfigSelection >= (itemsPerPageRegister - 1)) 
                          ? (registerConfigSelection - (itemsPerPageRegister - 1)) 
                          : 0;
    showRegisterValueSelection();
  }
  delay(200);
}

  if (!digitalRead(BTN_LEFT)) {
    Serial.println("BTN_LEFT Pressed");
    if (currentMenu == BAUD_RATE_MENU) {
      currentBaudIndex = (currentBaudIndex - 1 + baudRateLength) % baudRateLength;
      showBaudRateSelection();
    } else if (currentMenu == SERIAL_CONFIG_MENU) {
      currentSerialConfigIndex = (currentSerialConfigIndex - 1 + serialConfigLength) % serialConfigLength;
      showSerialConfigSelection();
    } else if (currentMenu == METER_ID_MENU) {
      meterId++;
      showMeterIdSelection();
    } else if (currentMenu == FUNCTION_CODE_MENU) {
      currentFunctionCodeIndex = (currentFunctionCodeIndex - 1 + functionCodeLength) % functionCodeLength;
      showFunctionCodeSelection();
    } else if (currentMenu == REGISTER_VALUE_MENU) {
      if (registerConfigSelection == 0) {
        registerValue += registerStep;
        registerValue = max(0, registerValue);
      } else if (registerConfigSelection == 1) {
        registerStep = max(1, registerStep + 1);
      } else if (registerConfigSelection == 2) {
        registerCount = min(4, max(1, registerCount + 1));
      } else if (registerConfigSelection == 3) {  // Scale
        scaleIndex = (scaleIndex + 1) % scaleValuesCount;
      }
      showRegisterValueSelection();
    }
    delay(200);
  }

  if (!digitalRead(BTN_RIGHT)) {
    Serial.println("BTN_RIGHT Pressed");
    if (currentMenu == BAUD_RATE_MENU) {
      currentBaudIndex = (currentBaudIndex + 1) % baudRateLength;
      showBaudRateSelection();
    } else if (currentMenu == SERIAL_CONFIG_MENU) {
      currentSerialConfigIndex = (currentSerialConfigIndex + 1) % serialConfigLength;
      showSerialConfigSelection();
    } else if (currentMenu == METER_ID_MENU) {
      meterId--;
      if (meterId < 1) meterId = 1;
      showMeterIdSelection();
    } else if (currentMenu == FUNCTION_CODE_MENU) {
      currentFunctionCodeIndex = (currentFunctionCodeIndex + 1) % functionCodeLength;
      showFunctionCodeSelection();
    } else if (currentMenu == REGISTER_VALUE_MENU) {
      if (registerConfigSelection == 0) {
        registerValue = max(0, registerValue - registerStep);
      } else if (registerConfigSelection == 1) {
        registerStep = max(1, registerStep - 1);
      } else if (registerConfigSelection == 2) {
        registerCount = max(1, registerCount - 1);
      } else if (registerConfigSelection == 3) {  // Scale
        scaleIndex = (scaleIndex - 1 + scaleValuesCount) % scaleValuesCount;
      }
      showRegisterValueSelection();
    }
    delay(200);
  }

  if (!digitalRead(BTN_SELECT)) {
    Serial.println("BTN_SELECT Pressed");
    if (currentMenu == MAIN_MENU) {
      currentMenu = BAUD_RATE_MENU;
      showBaudRateSelection();
    } else if (currentMenu == BAUD_RATE_MENU) {
      Serial.print("Baud Rate Selected: ");
      Serial.println(baudRates[currentBaudIndex]);
      currentMenu = SERIAL_CONFIG_MENU;
      showSerialConfigSelection();
    } else if (currentMenu == SERIAL_CONFIG_MENU) {
      Serial.print("Serial Config Selected: ");
      Serial.println(serialConfigs[currentSerialConfigIndex]);
      currentMenu = METER_ID_MENU;
      showMeterIdSelection();
    } else if (currentMenu == METER_ID_MENU) {
      Serial.print("Meter ID Selected: ");
      Serial.println(meterId);
      currentMenu = FUNCTION_CODE_MENU;
      showFunctionCodeSelection();
    } else if (currentMenu == FUNCTION_CODE_MENU) {
      Serial.print("Function code: ");
      Serial.println(functionCodes[currentFunctionCodeIndex]);
      currentMenu = FINAL_SELECTION_MENU;  // New transition
      showFinalSelection();
    } else if (currentMenu == FINAL_SELECTION_MENU) {
      currentMenu = DATA_TYPE_MENU;  // Go to data type first
      showDataTypeSelection();
    } else if (currentMenu == DATA_TYPE_MENU) {
      Serial.print("Data Type: ");
      Serial.println(dataTypes[currentDataTypeIndex]);
      currentMenu = REGISTER_VALUE_MENU;  // Then to register config
      showRegisterValueSelection();
    } else if (currentMenu == REGISTER_VALUE_MENU) {
      currentMenu = INITIALIZE_SERIAL_MENU;
      showInitializeSerial();
    }

    delay(200);
  }

  if (!digitalRead(BTN_BACK)) {
    Serial.println("BTN_BACK Pressed");
    if (currentMenu == BAUD_RATE_MENU) {
      currentMenu = MAIN_MENU;
      showMainMenu();
    } else if (currentMenu == SERIAL_CONFIG_MENU) {
      currentMenu = BAUD_RATE_MENU;
      showBaudRateSelection();
    } else if (currentMenu == METER_ID_MENU) {
      currentMenu = SERIAL_CONFIG_MENU;
      showSerialConfigSelection();
    } else if (currentMenu == FUNCTION_CODE_MENU) {
      currentMenu = METER_ID_MENU;
      showMeterIdSelection();
    } else if (currentMenu == FINAL_SELECTION_MENU) {
      currentMenu = FUNCTION_CODE_MENU;
      showFunctionCodeSelection();
    } else if (currentMenu == DATA_TYPE_MENU) {
      currentMenu = FINAL_SELECTION_MENU;
      showFinalSelection();
    } else if (currentMenu == REGISTER_VALUE_MENU) {
      currentMenu = DATA_TYPE_MENU;
      showDataTypeSelection();
    } else if (currentMenu == INITIALIZE_SERIAL_MENU) {
      currentMenu = REGISTER_VALUE_MENU;
      showRegisterValueSelection();
    }
    delay(200);
  }
  if (!digitalRead(BTN_UP) || !digitalRead(BTN_DOWN)) {
    if (currentMenu == DATA_TYPE_MENU) {
      if (!digitalRead(BTN_DOWN)) {
        currentDataTypeIndex = (currentDataTypeIndex + 1) % dataTypeLength;
      } else {
        currentDataTypeIndex = (currentDataTypeIndex - 1 + dataTypeLength) % dataTypeLength;
      }
      showDataTypeSelection();
      delay(200);
    }
  }
}

void showMainMenu() {
  display.clearDisplay();
  
  // Display "Portable" with text size 2
  display.setTextSize(2);
  int16_t x1, y1;
  uint16_t w, h;
  
  // Calculate position for "Portable"
  display.getTextBounds("Portable", 0, 0, &x1, &y1, &w, &h);
  int xPortable = (SCREEN_WIDTH - w) / 2;
  int yPortable = 10;  // Starting Y position
  display.setCursor(xPortable, yPortable);
  display.print("Portable");

  // Display "RS485" with text size 1
  display.setTextSize(2);
  
  // Calculate position for "RS485"
  display.getTextBounds("RS485", 0, 0, &x1, &y1, &w, &h);
  int xRS485 = (SCREEN_WIDTH - w) / 2;
  int yRS485 = yPortable + 30;  // 20 pixels below "Portable"
  display.setCursor(xRS485, yRS485);
  display.print("RS485");

  display.display();
  display.setTextSize(1);
}

void showBaudRateSelection() {
  display.clearDisplay();
  display.setTextSize(1);
  
  // Calculate positions dynamically
  int16_t x1, y1;
  uint16_t w, h;
  
  // Center "Baud Rate:" header
  String header = "Baud Rate:";
  display.getTextBounds(header, 0, 0, &x1, &y1, &w, &h);
  int headerX = (SCREEN_WIDTH - w) / 2;
  display.setCursor(headerX, 10);
  display.print(header);

  // Get current baud rate string dimensions
  String baudStr = String(baudRates[currentBaudIndex]);
  display.getTextBounds(baudStr, 0, 0, &x1, &y1, &w, &h);
  
  // Calculate positions for arrows and value
  int arrowSpacing = 6; // Space between arrows and value
  int totalWidth = 6 + w + 6; // 6px per arrow + value width
  int startX = (SCREEN_WIDTH - totalWidth) / 2;

  // Draw elements
  display.setCursor(startX, 30);
  display.print("<");
  
  display.setCursor(startX + 6 + arrowSpacing, 30);
  display.print(baudStr);
  
  display.setCursor(startX + 6 + arrowSpacing + w + arrowSpacing, 30);
  display.print(">");

  display.display();
}

void showSerialConfigSelection() {
  display.clearDisplay();
  display.setTextSize(1);
  
  // Center header
  String header = "Serial Config:";
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(header, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w)/2, 10);
  display.print(header);

  // Get config string dimensions
  String configStr = serialConfigs[currentSerialConfigIndex];
  display.getTextBounds(configStr, 0, 0, &x1, &y1, &w, &h);
  
  // Calculate positions
  int arrowSpacing = 6;
  int totalWidth = 6 + w + 6;
  int startX = (SCREEN_WIDTH - totalWidth)/2;

  // Draw elements
  display.setCursor(startX, 30);
  display.print("<");
  display.setCursor(startX + 6 + arrowSpacing, 30);
  display.print(configStr);
  display.setCursor(startX + 6 + arrowSpacing + w + arrowSpacing, 30);
  display.print(">");

  display.display();
}


void showMeterIdSelection() {
  display.clearDisplay();
  display.setTextSize(1);
  
  // Center header
  String header = "Meter ID:";
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(header, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w)/2, 10);
  display.print(header);

  // Get meter ID string dimensions
  String idStr = String(meterId);
  display.getTextBounds(idStr, 0, 0, &x1, &y1, &w, &h);
  
  // Calculate positions
  int arrowSpacing = 6;
  int totalWidth = 6 + w + 6;
  int startX = (SCREEN_WIDTH - totalWidth)/2;

  // Draw elements
  display.setCursor(startX, 30);
  display.print("<");
  display.setCursor(startX + 6 + arrowSpacing, 30);
  display.print(idStr);
  display.setCursor(startX + 6 + arrowSpacing + w + arrowSpacing, 30);
  display.print(">");

  display.display();
}

void showFunctionCodeSelection() {
  display.clearDisplay();
  display.setTextSize(1);
  
  // Center header
  String header = "Function Code:";
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(header, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w)/2, 10);
  display.print(header);

  // Get function code dimensions
  String funcStr = functionCodes[currentFunctionCodeIndex];
  display.getTextBounds(funcStr, 0, 0, &x1, &y1, &w, &h);
  
  // Calculate positions
  int arrowSpacing = 6;
  int totalWidth = 6 + w + 6;
  int startX = (SCREEN_WIDTH - totalWidth)/2;

  // Draw elements
  display.setCursor(startX, 30);
  display.print("<");
  display.setCursor(startX + 6 + arrowSpacing, 30);
  display.print(funcStr);
  display.setCursor(startX + 6 + arrowSpacing + w + arrowSpacing, 30);
  display.print(">");

  display.display();
}
void showRegisterValueSelection() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Modify:");

  int yPos = 20;
  for (int i = 0; i < itemsPerPageRegister; i++) {
    int itemIndex = startIndexRegister + i;
    if (itemIndex >= 4) break;

    display.setCursor(20, yPos + (i * 15));

    if (itemIndex == registerConfigSelection) display.print("> ");
    else display.print(" ");

    switch (itemIndex) {
      case 0:
        display.print("Value: ");
        display.print(registerValue);
        break;
      case 1:
        display.print("Step: ");
        display.print(registerStep);
        break;
      case 2:
        display.print("Count: ");
        display.print(registerCount);
        break;
      case 3:
        display.print("Scale: ");
        display.print(scaleValues[scaleIndex], 3);
        break;
    }

    if (itemIndex == registerConfigSelection) display.print(" <");
  }

  // Scroll indicators
  if (startIndexRegister > 0) display.drawTriangle(110, 20, 115, 15, 120, 20, SH110X_WHITE);
  if (startIndexRegister + itemsPerPageRegister < 4) display.drawTriangle(110, 57, 115, 62, 120, 57, SH110X_WHITE);

  display.display();
}


void showInitializeSerial() {
  display.clearDisplay();
  
  // Show loading message
  display.setTextSize(1);
  String loadingMsg = "Collecting Data...";
  int16_t x1, y1;
  uint16_t w, h;
  
  // Center loading message
  display.getTextBounds(loadingMsg, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w)/2, (SCREEN_HEIGHT - h)/2);
  display.print(loadingMsg);
  display.display();

  // Modbus setup and read
  setupModbus(baudRates[currentBaudIndex], serialConfigs[currentSerialConfigIndex], meterId);
  delay(1000);
  readModbusValues(registerValue, registerCount, scaleValues[scaleIndex], selectedDataType,functionCodes[currentFunctionCodeIndex]);

  // Prepare value display
  display.clearDisplay();
  String valueStr;
  
  // Format based on data type
  if (selectedDataType == "Float") {
    valueStr = String(dataFromMeter, 2); // 2 decimal places for float
  } else {
    valueStr = String((int)(dataFromMeter)); // Integer for UINT/Long
  }

  // Determine optimal text size
  int textSize = 2;
  display.setTextSize(textSize);
  display.getTextBounds(valueStr, 0, 0, &x1, &y1, &w, &h);
  
  // Switch to smaller text if needed
  if (w > SCREEN_WIDTH - 4) { // -4 for slight padding
    textSize = 1;
    display.setTextSize(textSize);
    display.getTextBounds(valueStr, 0, 0, &x1, &y1, &w, &h);
  }

  // Calculate centered position
  int xPos = (SCREEN_WIDTH - w) / 2;
  int yPos = (SCREEN_HEIGHT - h) / 2;

  // Draw value
  display.setCursor(xPos, yPos);
  display.print(valueStr);
  display.display();
}

void showFinalSelection() {
  display.clearDisplay();
  display.setCursor(10, 10);
  display.print("Baud: ");
  display.print(baudRates[currentBaudIndex]);

  display.setCursor(10, 25);
  display.print("Config: ");
  display.print(serialConfigs[currentSerialConfigIndex]);

  display.setCursor(10, 40);
  display.print("Meter ID: ");
  display.print(meterId);

  display.setCursor(10, 55);
  display.print("Function: ");
  display.print(functionCodes[currentFunctionCodeIndex]);

  display.display();
}

void showDataTypeSelection() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(10, 7);
  display.print("Data Type:");

  for (int i = 0; i < dataTypeLength; i++) {
      display.setCursor(30, 23 + i * 12);
      if (i == currentDataTypeIndex) {
          display.print("> ");
          selectedDataType = dataTypes[i];
      } else {
          display.print("  ");
      }
      display.print(dataTypes[i]);
}
  display.display();
}
