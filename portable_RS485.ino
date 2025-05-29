#include <Wire.h>
#include <Adafruit_SH110X.h>
#include "ModbusManager.h"
#include "constant.h"
// OLED display dimensions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// OLED display object
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

#include <FluxGarage_RoboEyes.h>
roboEyes roboEyes;
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
  DATA_TYPE_MENU,
  REGISTER_VALUE_MENU,
  INITIALIZE_SERIAL_MENU,
  IDLE_MENU
};
MenuState currentMenu = MAIN_MENU;
MenuState lastActiveMenu = MAIN_MENU;  // Store last active menu

// Change the main menu options
const char* mainMenu[] = { "Portable RS485" };
int mainMenuLength = sizeof(mainMenu) / sizeof(mainMenu[0]);
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

// Scale options
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
int registerCount = 1;
int registerConfigSelection = 0;
float dataFromMeter = 0.0;
float registerScale = 1.0;
int startIndexRegister = 0;
const int itemsPerPageRegister = 3;
String selectedDataType;

// Idle timeout variables
unsigned long lastInteractionTime = 0;
const unsigned long IDLE_TIMEOUT = 10000; // 10 seconds

// Function prototypes
void showMainMenu();
void showBaudRateSelection();
void showSerialConfigSelection();
void showMeterIdSelection();
void showFunctionCodeSelection();
void showRegisterValueSelection();
void showFinalSelection();
void showDataTypeSelection();
void showInitializeSerial();
void showIdleScreen();
bool anyButtonPressed();
void showCurrentMenu();  // Helper to show current menu

void setup() {
  Serial.begin(115200);

  // Initialize OLED
  if (!display.begin(0x3C, true)) {
    Serial.println(F("SH110X allocation failed"));
    while (true);
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
  
  // Initialize RoboEyes
  roboEyes.begin(SCREEN_WIDTH, SCREEN_HEIGHT, 100);
  roboEyes.setAutoblinker(ON, 3, 2);
  roboEyes.setIdleMode(ON, 2, 2);

  // Initialize idle timer
  lastInteractionTime = millis();

  // Show the main menu
  showMainMenu();
}

void loop() {
  unsigned long currentTime = millis();
  
  // Check for idle timeout
  if (currentMenu != IDLE_MENU && currentTime - lastInteractionTime > IDLE_TIMEOUT) {
    lastActiveMenu = currentMenu;  // Remember last active menu
    currentMenu = IDLE_MENU;
    showIdleScreen();
  }
  
  // Handle idle mode
  if (currentMenu == IDLE_MENU) {
    // Update the animation
    roboEyes.update();
    
    // Check for button presses to exit idle mode
    if (anyButtonPressed()) {
      lastInteractionTime = currentTime;  // Reset idle timer
      currentMenu = lastActiveMenu;       // Return to last active menu
      showCurrentMenu();                  // Show the previous menu
      delay(200);
      return; // Skip the rest of this loop iteration
    }
    
    // Skip button processing for other states since we're in idle mode
    return;
  }
  
  // Handle button presses for all other states
  if (anyButtonPressed()) {
    lastInteractionTime = currentTime;  // Reset idle timer
    
    // Process button presses
    if (!digitalRead(BTN_UP)) {
      Serial.println("BTN_UP Pressed");
      if (currentMenu == REGISTER_VALUE_MENU) {
        registerConfigSelection = (registerConfigSelection - 1 + 4) % 4;
        startIndexRegister = (registerConfigSelection >= (itemsPerPageRegister - 1))
                             ? (registerConfigSelection - (itemsPerPageRegister - 1))
                             : 0;
        showRegisterValueSelection();
      } else if (currentMenu == DATA_TYPE_MENU) {
        currentDataTypeIndex = (currentDataTypeIndex - 1 + dataTypeLength) % dataTypeLength;
        showDataTypeSelection();
      }
      delay(200);
    }

    if (!digitalRead(BTN_DOWN)) {
      Serial.println("BTN_DOWN Pressed");
      if (currentMenu == REGISTER_VALUE_MENU) {
        registerConfigSelection = (registerConfigSelection + 1) % 4;
        startIndexRegister = (registerConfigSelection >= (itemsPerPageRegister - 1)) 
                              ? (registerConfigSelection - (itemsPerPageRegister - 1)) 
                              : 0;
        showRegisterValueSelection();
      } else if (currentMenu == DATA_TYPE_MENU) {
        currentDataTypeIndex = (currentDataTypeIndex + 1) % dataTypeLength;
        showDataTypeSelection();
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
        } else if (registerConfigSelection == 3) {
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
        } else if (registerConfigSelection == 3) {
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
        currentMenu = SERIAL_CONFIG_MENU;
        showSerialConfigSelection();
      } else if (currentMenu == SERIAL_CONFIG_MENU) {
        currentMenu = METER_ID_MENU;
        showMeterIdSelection();
      } else if (currentMenu == METER_ID_MENU) {
        currentMenu = FUNCTION_CODE_MENU;
        showFunctionCodeSelection();
      } else if (currentMenu == FUNCTION_CODE_MENU) {
        currentMenu = FINAL_SELECTION_MENU;
        showFinalSelection();
      } else if (currentMenu == FINAL_SELECTION_MENU) {
        currentMenu = DATA_TYPE_MENU;
        showDataTypeSelection();
      } else if (currentMenu == DATA_TYPE_MENU) {
        currentMenu = REGISTER_VALUE_MENU;
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
  }
}

// Check if any button is pressed
bool anyButtonPressed() {
  return !digitalRead(BTN_UP) || !digitalRead(BTN_DOWN) || 
         !digitalRead(BTN_LEFT) || !digitalRead(BTN_RIGHT) || 
         !digitalRead(BTN_SELECT) || !digitalRead(BTN_BACK);
}

// Show idle screen
void showIdleScreen() {
  Serial.println("Enter in idle mode");
  // Clear display and let RoboEyes take over
  display.clearDisplay();
  display.display();
}

// Helper to show current menu
void showCurrentMenu() {
  switch (currentMenu) {
    case MAIN_MENU:
      showMainMenu();
      break;
    case BAUD_RATE_MENU:
      showBaudRateSelection();
      break;
    case SERIAL_CONFIG_MENU:
      showSerialConfigSelection();
      break;
    case METER_ID_MENU:
      showMeterIdSelection();
      break;
    case FUNCTION_CODE_MENU:
      showFunctionCodeSelection();
      break;
    case FINAL_SELECTION_MENU:
      showFinalSelection();
      break;
    case DATA_TYPE_MENU:
      showDataTypeSelection();
      break;
    case REGISTER_VALUE_MENU:
      showRegisterValueSelection();
      break;
    case INITIALIZE_SERIAL_MENU:
      showInitializeSerial();
      break;
    case IDLE_MENU:
      showIdleScreen();
      break;
  }
}

void showMainMenu() {
  display.clearDisplay();
  display.setTextSize(2);
  
  int16_t x1, y1;
  uint16_t w, h;
  
  // "Portable"
  display.getTextBounds("Portable", 0, 0, &x1, &y1, &w, &h);
  int xPortable = (SCREEN_WIDTH - w) / 2;
  display.setCursor(xPortable, 10);
  display.print("Portable");

  // "RS485"
  display.getTextBounds("RS485", 0, 0, &x1, &y1, &w, &h);
  int xRS485 = (SCREEN_WIDTH - w) / 2;
  display.setCursor(xRS485, 30);
  display.print("RS485");

  display.display();
  display.setTextSize(1);
}

void showBaudRateSelection() {
  display.clearDisplay();
  display.setTextSize(1);
  
  int16_t x1, y1;
  uint16_t w, h;
  
  String header = "Baud Rate:";
  display.getTextBounds(header, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w)/2, 10);
  display.print(header);

  String baudStr = String(baudRates[currentBaudIndex]);
  display.getTextBounds(baudStr, 0, 0, &x1, &y1, &w, &h);
  
  int arrowSpacing = 6;
  int totalWidth = 6 + w + 6;
  int startX = (SCREEN_WIDTH - totalWidth) / 2;

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
  
  String header = "Serial Config:";
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(header, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w)/2, 10);
  display.print(header);

  String configStr = serialConfigs[currentSerialConfigIndex];
  display.getTextBounds(configStr, 0, 0, &x1, &y1, &w, &h);
  
  int arrowSpacing = 6;
  int totalWidth = 6 + w + 6;
  int startX = (SCREEN_WIDTH - totalWidth)/2;

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
  
  String header = "Meter ID:";
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(header, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w)/2, 10);
  display.print(header);

  String idStr = String(meterId);
  display.getTextBounds(idStr, 0, 0, &x1, &y1, &w, &h);
  
  int arrowSpacing = 6;
  int totalWidth = 6 + w + 6;
  int startX = (SCREEN_WIDTH - totalWidth)/2;

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
  
  String header = "Function Code:";
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(header, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w)/2, 10);
  display.print(header);

  String funcStr = functionCodes[currentFunctionCodeIndex];
  display.getTextBounds(funcStr, 0, 0, &x1, &y1, &w, &h);
  
  int arrowSpacing = 6;
  int totalWidth = 6 + w + 6;
  int startX = (SCREEN_WIDTH - totalWidth)/2;

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
  display.setTextSize(1);
  
  String loadingMsg = "Collecting Data...";
  int16_t x1, y1;
  uint16_t w, h;
  
  display.getTextBounds(loadingMsg, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w)/2, (SCREEN_HEIGHT - h)/2);
  display.print(loadingMsg);
  display.display();

  // Modbus setup and read
  setupModbus(baudRates[currentBaudIndex], serialConfigs[currentSerialConfigIndex], meterId);
  delay(1000);
  readModbusValues(registerValue, registerCount, scaleValues[scaleIndex], selectedDataType, functionCodes[currentFunctionCodeIndex]);

  // Display value
  display.clearDisplay();
  String valueStr;
  
  if (selectedDataType == "Float") {
    valueStr = String(dataFromMeter, 2);
  } else {
    valueStr = String((int)(dataFromMeter));
  }

  int textSize = 2;
  display.setTextSize(textSize);
  display.getTextBounds(valueStr, 0, 0, &x1, &y1, &w, &h);
  
  if (w > SCREEN_WIDTH - 4) {
    textSize = 1;
    display.setTextSize(textSize);
    display.getTextBounds(valueStr, 0, 0, &x1, &y1, &w, &h);
  }

  int xPos = (SCREEN_WIDTH - w) / 2;
  int yPos = (SCREEN_HEIGHT - h) / 2;

  display.setCursor(xPos, yPos);
  display.print(valueStr);
  display.display();
  display.setTextSize(1);
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