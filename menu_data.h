// ===== 2. menu_data.h (MODIFIED - Add MENU_RADIO_TEST to enum) =====

/*
  menu_data.h - Data Structures and EEPROM Storage (UPDATED WITH RADIO TEST)
  RC Transmitter for Arduino Mega
*/

#ifndef MENU_DATA_H
#define MENU_DATA_H

#include <EEPROM.h>
#include "config.h"

// Menu states - UPDATED: Added MENU_RADIO_TEST
enum MenuState {
  MENU_HIDDEN,
  MENU_MAIN,
  MENU_CALIBRATION,
  MENU_JOYSTICK_CAL,
  MENU_POTENTIOMETER_CAL,
  MENU_MPU6500_CAL,
  MENU_SETTINGS,
  MENU_DEADZONE_SETTING,
  MENU_BRIGHTNESS_SETTING,
  MENU_LED_SETTINGS,
  MENU_LED_COLOR_SETTING,
  MENU_RADIO_ADDRESS,
  MENU_FAILSAFE_SETTINGS,
  MENU_FAILSAFE_THROTTLE_SETTING,
  MENU_FAILSAFE_STEERING_SETTING,
  MENU_CHANNEL_SETTINGS,
  MENU_INFO,
  MENU_CAL_IN_PROGRESS,
  MENU_CANCEL_CONFIRM,
  MENU_RADIO_TEST  // ADDED: New menu state for radio test
};

// LED Color modes
enum LEDColorMode {
  LED_COLOR_ARMED,
  LED_COLOR_DISARMED,
  LED_COLOR_TRANSMITTING,
  LED_COLOR_ERROR,
  LED_COLOR_MENU
};

// Calibration states
enum CalibrationState {
  CAL_IDLE,
  CAL_NEUTRAL,
  CAL_MAX,
  CAL_MIN,
  CAL_LEVEL,
  CAL_FORWARD,
  CAL_BACKWARD,
  CAL_LEFT,
  CAL_RIGHT,
  CAL_COMPLETE
};

// Menu item structure
struct MenuItem {
  String title;
  bool enabled;
  bool hasSubmenu;
};

// Settings data structure
struct SettingsData {
  // Joystick settings
  int joystickDeadzone;  // 0-200
  
  // Display settings
  int displayBrightness;  // 0-255 (contrast)
  
  // LED settings
  bool ledEnabled;
  bool ledArmedColor[3];      // RGB for armed state
  bool ledDisarmedColor[3];   // RGB for disarmed state
  bool ledTransmitColor[3];   // RGB for transmitting
  bool ledErrorColor[3];      // RGB for error state
  bool ledMenuColor[3];       // RGB for menu mode
  
  // Radio settings
  char radioAddress[6];       // 5 characters + null terminator
  int radioChannel;           // 0-125
  
  // Failsafe settings
  int failsafeThrottle;       // -1000 to 1000
  int failsafeSteering;       // -1000 to 1000
  bool failsafeEnabled;
  
  // EEPROM signature
  uint16_t signature;
};

// Calibration data structure
struct CalibrationData {
  // Individual joystick axis calibration
  int rightJoyX_min, rightJoyX_neutral, rightJoyX_max;
  int rightJoyY_min, rightJoyY_neutral, rightJoyY_max;
  int leftJoyX_min, leftJoyX_neutral, leftJoyX_max;
  int leftJoyY_min, leftJoyY_neutral, leftJoyY_max;
  
  // Individual potentiometer calibration
  int leftPot_min, leftPot_neutral, leftPot_max;
  int rightPot_min, rightPot_neutral, rightPot_max;
  
  // MPU6500 calibration
  float mpu_level_roll, mpu_level_pitch;
  float mpu_forward_pitch, mpu_backward_pitch;
  float mpu_left_roll, mpu_right_roll;
  
  // Individual calibration validity flags
  bool rightJoyX_calibrated, rightJoyY_calibrated;
  bool leftJoyX_calibrated, leftJoyY_calibrated;
  bool leftPot_calibrated, rightPot_calibrated;
  bool mpu_calibrated;
  
  // EEPROM signature
  uint16_t signature;
};

// Global data instances
SettingsData settings;
CalibrationData calData;

// EEPROM addresses
#define EEPROM_CAL_ADDRESS 0
#define EEPROM_SETTINGS_ADDRESS 512
#define EEPROM_SIGNATURE 0xCAFE

// Function declarations
void initMenuData();
void saveSettings();
void loadSettings();
void resetSettings();
void saveCalibration();
void loadCalibration();
void resetCalibration();
void applyLEDSettings();
void applyDisplayBrightness();
int getCurrentDeadzone();
String getCalibrationStatus(String axis);
int getCalibratedValue(int rawValue, int minVal, int neutralVal, int maxVal);
int getCalibratedSteering();
int getCalibratedThrottle();
int getCalibratedRightJoyY();
int getCalibratedLeftJoyX();
int getCalibratedLeftPot();
int getCalibratedRightPot();
int freeMemory();

// Forward declarations for external functions
extern bool getArmedStatus();
extern void setLED(bool red, bool green, bool blue);
extern bool menuActive;
extern Adafruit_SSD1306 display;

void initMenuData() {
  Serial.println("Loading data from EEPROM...");
  loadCalibration();
  loadSettings();
  applyDisplayBrightness();
  applyLEDSettings();
}

void saveSettings() {
  settings.signature = EEPROM_SIGNATURE;
  EEPROM.put(EEPROM_SETTINGS_ADDRESS, settings);
  Serial.println("Settings saved to EEPROM");
  
  // Apply settings immediately after saving
  applyLEDSettings();
  applyDisplayBrightness();
}

void loadSettings() {
  EEPROM.get(EEPROM_SETTINGS_ADDRESS, settings);
  
  if (settings.signature != EEPROM_SIGNATURE) {
    Serial.println("No valid settings found, using defaults");
    resetSettings();
  } else {
    Serial.println("Settings loaded from EEPROM");
  }
}

void resetSettings() {
  // Default settings
  settings.joystickDeadzone = 50;
  settings.displayBrightness = 150;
  settings.ledEnabled = true;
  
  // Default LED colors (RGB)
  settings.ledArmedColor[0] = false; settings.ledArmedColor[1] = true; settings.ledArmedColor[2] = false; // Green
  settings.ledDisarmedColor[0] = true; settings.ledDisarmedColor[1] = false; settings.ledDisarmedColor[2] = false; // Red
  settings.ledTransmitColor[0] = false; settings.ledTransmitColor[1] = false; settings.ledTransmitColor[2] = true; // Blue
  settings.ledErrorColor[0] = true; settings.ledErrorColor[1] = true; settings.ledErrorColor[2] = false; // Yellow
  settings.ledMenuColor[0] = true; settings.ledMenuColor[1] = false; settings.ledMenuColor[2] = true; // Magenta
  
  // Default radio settings
  strcpy(settings.radioAddress, "BOAT1");
  settings.radioChannel = 76;
  
  // Default failsafe settings
  settings.failsafeThrottle = 0;
  settings.failsafeSteering = 0;
  settings.failsafeEnabled = true;
  
  settings.signature = EEPROM_SIGNATURE;
}

void saveCalibration() {
  calData.signature = EEPROM_SIGNATURE;
  EEPROM.put(EEPROM_CAL_ADDRESS, calData);
  Serial.println("Calibration saved to EEPROM");
}

void loadCalibration() {
  EEPROM.get(EEPROM_CAL_ADDRESS, calData);
  
  if (calData.signature != EEPROM_SIGNATURE) {
    Serial.println("No valid calibration found, using defaults");
    resetCalibration();
  } else {
    Serial.println("Calibration loaded from EEPROM");
  }
}

void resetCalibration() {
  // Set default values for all axes
  calData.rightJoyX_min = 0; calData.rightJoyX_neutral = 512; calData.rightJoyX_max = 1023;
  calData.rightJoyY_min = 0; calData.rightJoyY_neutral = 512; calData.rightJoyY_max = 1023;
  calData.leftJoyX_min = 0; calData.leftJoyX_neutral = 512; calData.leftJoyX_max = 1023;
  calData.leftJoyY_min = 0; calData.leftJoyY_neutral = 512; calData.leftJoyY_max = 1023;
  
  calData.leftPot_min = 0; calData.leftPot_neutral = 512; calData.leftPot_max = 1023;
  calData.rightPot_min = 0; calData.rightPot_neutral = 512; calData.rightPot_max = 1023;
  
  calData.mpu_level_roll = 0; calData.mpu_level_pitch = 0;
  calData.mpu_forward_pitch = 30; calData.mpu_backward_pitch = -30;
  calData.mpu_left_roll = -30; calData.mpu_right_roll = 30;
  
  // Reset all calibration flags
  calData.rightJoyX_calibrated = false; calData.rightJoyY_calibrated = false;
  calData.leftJoyX_calibrated = false; calData.leftJoyY_calibrated = false;
  calData.leftPot_calibrated = false; calData.rightPot_calibrated = false;
  calData.mpu_calibrated = false;
  calData.signature = EEPROM_SIGNATURE;
}

void applyLEDSettings() {
  // CRITICAL FIX: Check if LEDs are disabled first
  if (!settings.ledEnabled) {
    setLED(false, false, false);  // Turn off all LEDs
    Serial.println("LEDs disabled - all LEDs turned off");
    return;
  }
  
  bool* color;
  
  // Determine which color to use based on context
  if (menuActive) {
    color = settings.ledMenuColor;
    Serial.print("LED: Menu mode - ");
  } else if (getArmedStatus()) {
    color = settings.ledArmedColor;
    Serial.print("LED: Armed mode - ");
  } else {
    color = settings.ledDisarmedColor;
    Serial.print("LED: Disarmed mode - ");
  }
  
  // Apply the selected color
  setLED(color[0], color[1], color[2]);
  
  // Enhanced debug output
  Serial.print("R:");
  Serial.print(color[0] ? "ON" : "OFF");
  Serial.print(" G:");
  Serial.print(color[1] ? "ON" : "OFF");
  Serial.print(" B:");
  Serial.print(color[2] ? "ON" : "OFF");
  Serial.print(" (Enabled: ");
  Serial.print(settings.ledEnabled ? "YES" : "NO");
  Serial.print(", Armed: ");
  Serial.print(getArmedStatus() ? "YES" : "NO");
  Serial.print(", Menu: ");
  Serial.print(menuActive ? "YES" : "NO");
  Serial.println(")");
}

void applyDisplayBrightness() {
  // Apply contrast setting to the display
  // This actually works with SSD1306 displays
  display.ssd1306_command(0x81); // Set contrast command
  display.ssd1306_command(settings.displayBrightness); // Contrast value (0-255)
  Serial.print("Display brightness set to: ");
  Serial.println(settings.displayBrightness);
}

int getCurrentDeadzone() {
  return settings.joystickDeadzone;
}

String getCalibrationStatus(String axis) {
  if (axis == "RIGHT_X") return calData.rightJoyX_calibrated ? "[OK]" : "[--]";
  if (axis == "RIGHT_Y") return calData.rightJoyY_calibrated ? "[OK]" : "[--]";
  if (axis == "LEFT_X") return calData.leftJoyX_calibrated ? "[OK]" : "[--]";
  if (axis == "LEFT_Y") return calData.leftJoyY_calibrated ? "[OK]" : "[--]";
  if (axis == "LEFT_POT") return calData.leftPot_calibrated ? "[OK]" : "[--]";
  if (axis == "RIGHT_POT") return calData.rightPot_calibrated ? "[OK]" : "[--]";
  if (axis == "MPU") return calData.mpu_calibrated ? "[OK]" : "[--]";
  return "[--]";
}

// Calibrated value functions
int getCalibratedValue(int rawValue, int minVal, int neutralVal, int maxVal) {
  if (rawValue <= neutralVal) {
    return map(rawValue, minVal, neutralVal, -1000, 0);
  } else {
    return map(rawValue, neutralVal, maxVal, 0, 1000);
  }
}

int getCalibratedSteering() {
  if (!calData.rightJoyX_calibrated) {
    return map(analogRead(RIGHT_JOY_X), 0, 1023, 1000, -1000);
  }
  int value = getCalibratedValue(analogRead(RIGHT_JOY_X), 
                                calData.rightJoyX_min, 
                                calData.rightJoyX_neutral, 
                                calData.rightJoyX_max);
  // Apply deadzone
  if (abs(value) < settings.joystickDeadzone) value = 0;
  return value;
}

int getCalibratedThrottle() {
  if (!calData.leftJoyY_calibrated) {
    return map(analogRead(LEFT_JOY_Y), 0, 1023, -1000, 1000);
  }
  int value = getCalibratedValue(analogRead(LEFT_JOY_Y), 
                                calData.leftJoyY_min, 
                                calData.leftJoyY_neutral, 
                                calData.leftJoyY_max);
  // Apply deadzone
  if (abs(value) < settings.joystickDeadzone) value = 0;
  return value;
}

// Additional calibrated functions for future use
int getCalibratedRightJoyY() {
  if (!calData.rightJoyY_calibrated) {
    return map(analogRead(RIGHT_JOY_Y), 0, 1023, -1000, 1000);
  }
  return getCalibratedValue(analogRead(RIGHT_JOY_Y), 
                           calData.rightJoyY_min, 
                           calData.rightJoyY_neutral, 
                           calData.rightJoyY_max);
}

int getCalibratedLeftJoyX() {
  if (!calData.leftJoyX_calibrated) {
    return map(analogRead(LEFT_JOY_X), 0, 1023, -1000, 1000);
  }
  return getCalibratedValue(analogRead(LEFT_JOY_X), 
                           calData.leftJoyX_min, 
                           calData.leftJoyX_neutral, 
                           calData.leftJoyX_max);
}

int getCalibratedLeftPot() {
  if (!calData.leftPot_calibrated) {
    return map(analogRead(LEFT_POT), 0, 1023, -1000, 1000);
  }
  return getCalibratedValue(analogRead(LEFT_POT), 
                           calData.leftPot_min, 
                           calData.leftPot_neutral, 
                           calData.leftPot_max);
}

int getCalibratedRightPot() {
  if (!calData.rightPot_calibrated) {
    return map(analogRead(RIGHT_POT), 0, 1023, -1000, 1000);
  }
  return getCalibratedValue(analogRead(RIGHT_POT), 
                           calData.rightPot_min, 
                           calData.rightPot_neutral, 
                           calData.rightPot_max);
}

// Utility function to get free memory
int freeMemory() {
  char top;
  return &top - reinterpret_cast<char*>(malloc(4));
}

#endif