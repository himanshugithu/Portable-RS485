#include <Wire.h>
#include <Adafruit_SH110X.h>

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
#define BTN_SELECT 18
#define BTN_BACK 19

// Menu states
enum MenuState { MAIN_MENU,
                 BAUD_RATE_MENU,
                 SERIAL_CONFIG_MENU,
                 METER_ID_MENU,
                 FUNCTION_CODE_MENU,
                 SHOW_CONFIG,
                 REGISTER_VALUE_MENU };
MenuState currentMenu = MAIN_MENU;

// Main menu options
const char* mainMenu[] = { "Energy Meter", "Solar Inverter" };
int mainMenuLength = sizeof(mainMenu) / sizeof(mainMenu[0]);
int currentMainMenuIndex = 0;

// Baud rate options
const int baudRates[] = { 9600, 19200, 38400, 57600, 115200 };
int baudRateLength = sizeof(baudRates) / sizeof(baudRates[0]);
int currentBaudIndex = 0;

// Serial configurations
const char* serialConfigs[] = { "8N1", "8E1", "8O1" };
int serialConfigLength = sizeof(serialConfigs) / sizeof(serialConfigs[0]);
int currentSerialConfigIndex = 0;

int meterId = 1;
int registerValue = 1;
int registerStep = 1;
int registerCount = 1;            // Added count variable
int registerConfigSelection = 0;  // 0: Value, 1: Step, 2: Count (changed to integer)
// const int functionCodes[] = { 0x03, 0x04, 0x06, 0x10 };
const char* functionCodes[] = { "0x03", "0x04" };
int functionCodeLength = sizeof(functionCodes) / sizeof(functionCodes[0]);
int currentFunctionCodeIndex = 0;
// bool registerConfigSelection = true;
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
      currentMainMenuIndex = (currentMainMenuIndex - 1 + mainMenuLength) % mainMenuLength;
      showMainMenu();
    } else if (currentMenu == REGISTER_VALUE_MENU) {
      registerConfigSelection = (registerConfigSelection - 1 + 3) % 3;  // Toggle between value and step
      showRegisterValueSelection();
    }
    delay(200);
  }

  if (!digitalRead(BTN_DOWN)) {
    Serial.println("BTN_DOWN Pressed");
    if (currentMenu == MAIN_MENU) {
      currentMainMenuIndex = (currentMainMenuIndex + 1) % mainMenuLength;
      showMainMenu();
    } else if (currentMenu == REGISTER_VALUE_MENU) {
      registerConfigSelection = (registerConfigSelection + 1) % 3;  // Toggle between value and step
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
      if (registerConfigSelection == 0) {  // Value
        registerValue += registerStep;
        registerValue = max(0, registerValue);
      } else if (registerConfigSelection == 1) {  // Step
        registerStep = max(1, registerStep + 1);
      } else if (registerConfigSelection == 2) {  // Count
        registerCount = min(4, max(1, registerCount + 1));
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
      if (registerConfigSelection == 0) {  // Value
        registerValue = max(0, registerValue - registerStep);
      } else if (registerConfigSelection == 1) {  // Step
        registerStep = max(1, registerStep - 1);
      } else if (registerConfigSelection == 2) {  // Count
        registerCount = max(1, registerCount - 1);

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
      Serial.print("Func: ");
      Serial.println(functionCodes[currentFunctionCodeIndex]);
      currentMenu = REGISTER_VALUE_MENU;
      showFinalSelection();
    } else if (currentMenu == REGISTER_VALUE_MENU) {
      Serial.print("register value: ");
      Serial.println(registerValue);
      showRegisterValueSelection();
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
      showFunctionCodeSelection();
    } else if (currentMenu == REGISTER_VALUE_MENU) {
      currentMenu = FUNCTION_CODE_MENU;
      showFunctionCodeSelection();
    }
    delay(200);
  }
}

// Display Functions

void showMainMenu() {
  display.clearDisplay();
  display.setTextSize(1);
  for (int i = 0; i < mainMenuLength; i++) {
    display.setCursor(10, 20 + i * 20);
    display.print(i == currentMainMenuIndex ? " > " : "   ");
    display.println(mainMenu[i]);
  }
  display.display();
}

void showBaudRateSelection() {
  display.clearDisplay();
  display.setCursor(10, 20);
  display.print("Baud Rate:");
  display.setCursor(60, 40);
  display.print(baudRates[currentBaudIndex]);
  display.display();
}

void showSerialConfigSelection() {
  display.clearDisplay();
  display.setCursor(10, 20);
  display.print("Serial Config:");
  display.setCursor(60, 40);
  display.print(serialConfigs[currentSerialConfigIndex]);
  display.display();
}

void showMeterIdSelection() {
  display.clearDisplay();
  display.setCursor(10, 20);
  display.print("Meter ID:");
  display.setCursor(60, 40);
  display.print(meterId);
  display.display();
}

void showFunctionCodeSelection() {
  display.clearDisplay();
  display.setCursor(10, 20);
  display.print("Function Code:");
  display.setCursor(20, 40);
  display.print(functionCodes[currentFunctionCodeIndex]);
  display.display();
}

void showRegisterValueSelection() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Modify:");

  // Value
  display.setCursor(20, 20);
  if (registerConfigSelection == 0) display.print(">");
  else display.print(" ");
  display.print(" Value: ");
  display.print(registerValue);
  if (registerConfigSelection == 0) display.print(" <");

  // Step
  display.setCursor(20, 35);
  if (registerConfigSelection == 1) display.print(">");
  else display.print(" ");
  display.print(" Step: ");
  display.print(registerStep);
  if (registerConfigSelection == 1) display.print(" <");

  // Count
  display.setCursor(20, 50);
  if (registerConfigSelection == 2) display.print(">");
  else display.print(" ");
  display.print(" Count: ");
  display.print(registerCount);
  if (registerConfigSelection == 2) display.print(" <");

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
