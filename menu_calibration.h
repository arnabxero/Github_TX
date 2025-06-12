/*
  menu_calibration.h - Calibration Management (UPDATED WITH BACK BUTTON)
  RC Transmitter for Arduino Mega
*/

#ifndef MENU_CALIBRATION_H
#define MENU_CALIBRATION_H

#include <Wire.h>
#include "config.h"
#include "display.h"
#include "menu_data.h"

// Calibration variables
bool calibrationActive = false;
String currentCalType = "";
String currentCalAxis = "";
int calStep = 0;
int maxCalSteps = 0;
bool waitingForOK = false;
CalibrationState calState = CAL_IDLE;

// External variables from menu.h
extern MenuState currentMenu;
extern int menuSelection;
extern int menuOffset;
extern int maxMenuItems;
extern unsigned long lastNavigation;

// Function declarations
void initMenuCalibration();
void updateMenuCalibration();
void startCalibration(String calType, String axis);
void completeCalibration();
void exitMenuCalibration();
void goBackCalibration();
void drawMenuCalibration();
void drawCalibrationScreen();
String getCalibrationStepText();
bool isCalibrationActive();
void initMPU6500();
void readMPU6500(float &roll, float &pitch);

void initMenuCalibration() {
  // Initialize MPU6500 for calibration
  initMPU6500();
  calibrationActive = false;
}

void updateMenuCalibration() {
  if (!waitingForOK) return;
  
  // During calibration, check for both OK button and left joystick button
  static bool lastOKState = false;
  static bool lastLeftJoyState = false;
  bool currentOKState = buttons.btnOK;
  bool currentLeftJoyState = buttons.leftJoyBtn;
  
  // ADDED: Check for left joystick button press (back/cancel functionality)
  if (currentLeftJoyState && !lastLeftJoyState) {
    Serial.println("Left joystick pressed during calibration - going back");
    
    // Cancel current calibration and go back to appropriate menu
    calibrationActive = false;
    waitingForOK = false;
    calState = CAL_IDLE;
    
    // Return to the appropriate parent menu
    if (currentCalType == "JOYSTICK") {
      currentMenu = MENU_JOYSTICK_CAL;
      maxMenuItems = 5;
    } else if (currentCalType == "POTENTIOMETER") {
      currentMenu = MENU_POTENTIOMETER_CAL;
      maxMenuItems = 3;
    } else if (currentCalType == "MPU6500") {
      currentMenu = MENU_CALIBRATION;
      maxMenuItems = 4;
    } else {
      // Default fallback
      currentMenu = MENU_CALIBRATION;
      maxMenuItems = 4;
    }
    
    menuSelection = 0;
    menuOffset = 0;
    
    Serial.println("Calibration cancelled - returned to menu");
    lastLeftJoyState = currentLeftJoyState;
    return;
  }
  
  // Check for OK button press (rising edge detection) - ORIGINAL FUNCTIONALITY
  if (currentOKState && !lastOKState) {
    Serial.println("OK pressed during calibration");
    
    if (currentCalType == "JOYSTICK") {
      int rawValue = 0;
      
      // Read the specific axis
      if (currentCalAxis == "RIGHT_X") rawValue = analogRead(RIGHT_JOY_X);
      else if (currentCalAxis == "RIGHT_Y") rawValue = analogRead(RIGHT_JOY_Y);
      else if (currentCalAxis == "LEFT_X") rawValue = analogRead(LEFT_JOY_X);
      else if (currentCalAxis == "LEFT_Y") rawValue = analogRead(LEFT_JOY_Y);
      
      // Store calibration values for specific axis
      switch (calState) {
        case CAL_NEUTRAL:
          if (currentCalAxis == "RIGHT_X") calData.rightJoyX_neutral = rawValue;
          else if (currentCalAxis == "RIGHT_Y") calData.rightJoyY_neutral = rawValue;
          else if (currentCalAxis == "LEFT_X") calData.leftJoyX_neutral = rawValue;
          else if (currentCalAxis == "LEFT_Y") calData.leftJoyY_neutral = rawValue;
          calState = CAL_MAX;
          break;
        case CAL_MAX:
          if (currentCalAxis == "RIGHT_X") calData.rightJoyX_max = rawValue;
          else if (currentCalAxis == "RIGHT_Y") calData.rightJoyY_max = rawValue;
          else if (currentCalAxis == "LEFT_X") calData.leftJoyX_max = rawValue;
          else if (currentCalAxis == "LEFT_Y") calData.leftJoyY_max = rawValue;
          calState = CAL_MIN;
          break;
        case CAL_MIN:
          if (currentCalAxis == "RIGHT_X") {
            calData.rightJoyX_min = rawValue;
            calData.rightJoyX_calibrated = true;
          } else if (currentCalAxis == "RIGHT_Y") {
            calData.rightJoyY_min = rawValue;
            calData.rightJoyY_calibrated = true;
          } else if (currentCalAxis == "LEFT_X") {
            calData.leftJoyX_min = rawValue;
            calData.leftJoyX_calibrated = true;
          } else if (currentCalAxis == "LEFT_Y") {
            calData.leftJoyY_min = rawValue;
            calData.leftJoyY_calibrated = true;
          }
          completeCalibration();
          break;
      }
    } else if (currentCalType == "POTENTIOMETER") {
      int rawValue = 0;
      
      if (currentCalAxis == "LEFT") rawValue = analogRead(LEFT_POT);
      else if (currentCalAxis == "RIGHT") rawValue = analogRead(RIGHT_POT);
      
      switch (calState) {
        case CAL_NEUTRAL:
          if (currentCalAxis == "LEFT") calData.leftPot_neutral = rawValue;
          else if (currentCalAxis == "RIGHT") calData.rightPot_neutral = rawValue;
          calState = CAL_MAX;
          break;
        case CAL_MAX:
          if (currentCalAxis == "LEFT") calData.leftPot_max = rawValue;
          else if (currentCalAxis == "RIGHT") calData.rightPot_max = rawValue;
          calState = CAL_MIN;
          break;
        case CAL_MIN:
          if (currentCalAxis == "LEFT") {
            calData.leftPot_min = rawValue;
            calData.leftPot_calibrated = true;
          } else if (currentCalAxis == "RIGHT") {
            calData.rightPot_min = rawValue;
            calData.rightPot_calibrated = true;
          }
          completeCalibration();
          break;
      }
    } else if (currentCalType == "MPU6500") {
      float roll, pitch;
      readMPU6500(roll, pitch);
      
      switch (calState) {
        case CAL_LEVEL:
          calData.mpu_level_roll = roll;
          calData.mpu_level_pitch = pitch;
          calState = CAL_FORWARD;
          break;
        case CAL_FORWARD:
          calData.mpu_forward_pitch = pitch;
          calState = CAL_BACKWARD;
          break;
        case CAL_BACKWARD:
          calData.mpu_backward_pitch = pitch;
          calState = CAL_LEFT;
          break;
        case CAL_LEFT:
          calData.mpu_left_roll = roll;
          calState = CAL_RIGHT;
          break;
        case CAL_RIGHT:
          calData.mpu_right_roll = roll;
          calData.mpu_calibrated = true;
          completeCalibration();
          break;
      }
    }
    
    calStep++;
  }
  
  lastOKState = currentOKState;
  lastLeftJoyState = currentLeftJoyState;
}

void startCalibration(String calType, String axis) {
  Serial.print("Starting calibration: ");
  Serial.print(calType);
  if (axis != "") {
    Serial.print(" - ");
    Serial.print(axis);
  }
  Serial.println();
  
  calibrationActive = true;
  currentCalType = calType;
  currentCalAxis = axis;
  calStep = 0;
  waitingForOK = true;
  currentMenu = MENU_CAL_IN_PROGRESS;
  
  if (calType == "JOYSTICK" || calType == "POTENTIOMETER") {
    maxCalSteps = 3; // Neutral, Max, Min
    calState = CAL_NEUTRAL;
  } else if (calType == "MPU6500") {
    maxCalSteps = 5; // Level, Forward, Backward, Left, Right
    calState = CAL_LEVEL;
  }
}

void completeCalibration() {
  Serial.print("Calibration complete: ");
  Serial.print(currentCalType);
  if (currentCalAxis != "") {
    Serial.print(" - ");
    Serial.print(currentCalAxis);
  }
  Serial.println();
  
  saveCalibration();
  calibrationActive = false;
  
  // Return to appropriate menu
  if (currentCalType == "JOYSTICK") {
    currentMenu = MENU_JOYSTICK_CAL;
    maxMenuItems = 5;
  } else if (currentCalType == "POTENTIOMETER") {
    currentMenu = MENU_POTENTIOMETER_CAL;
    maxMenuItems = 3;
  } else {
    currentMenu = MENU_CALIBRATION;
    maxMenuItems = 4;
  }
  
  menuSelection = 0;
  menuOffset = 0;
  calState = CAL_IDLE;
  
  delay(1000);
}

void exitMenuCalibration() {
  calibrationActive = false;
  calState = CAL_IDLE;
}

void goBackCalibration() {
  // Return to calibration menu if in progress
  if (calibrationActive) {
    calibrationActive = false;
    currentMenu = MENU_CALIBRATION;
    maxMenuItems = 4;
    menuSelection = 0;
    menuOffset = 0;
  }
}

bool isCalibrationActive() {
  return calibrationActive;
}

void drawMenuCalibration() {
  drawCalibrationScreen();
}

void drawCalibrationScreen() {
  // Header
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Cal: ");
  display.print(currentCalType);
  if (currentCalAxis != "") {
    display.print(" ");
    display.print(currentCalAxis);
  }
  
  // Progress
  display.setCursor(0, 16);
  display.print("Step ");
  display.print(calStep + 1);
  display.print("/");
  display.print(maxCalSteps);
  
  // Instructions
  display.setCursor(0, 28);
  display.println(getCalibrationStepText());
  
  // Current value display
  if (currentCalType == "JOYSTICK") {
    display.setCursor(0, 42);
    if (currentCalAxis == "RIGHT_X") {
      display.print("Value: ");
      display.print(analogRead(RIGHT_JOY_X));
    } else if (currentCalAxis == "RIGHT_Y") {
      display.print("Value: ");
      display.print(analogRead(RIGHT_JOY_Y));
    } else if (currentCalAxis == "LEFT_X") {
      display.print("Value: ");
      display.print(analogRead(LEFT_JOY_X));
    } else if (currentCalAxis == "LEFT_Y") {
      display.print("Value: ");
      display.print(analogRead(LEFT_JOY_Y));
    }
  } else if (currentCalType == "POTENTIOMETER") {
    display.setCursor(0, 42);
    if (currentCalAxis == "LEFT") {
      display.print("Value: ");
      display.print(analogRead(LEFT_POT));
    } else if (currentCalAxis == "RIGHT") {
      display.print("Value: ");
      display.print(analogRead(RIGHT_POT));
    }
  }
  
  // UPDATED: Show both OK and Back instructions
  display.setCursor(0, 52);
  display.print("OK: Continue");
}

String getCalibrationStepText() {
  if (currentCalType == "JOYSTICK" || currentCalType == "POTENTIOMETER") {
    switch (calState) {
      case CAL_NEUTRAL: return "Move to CENTER";
      case CAL_MAX: return "Move to MAXIMUM";
      case CAL_MIN: return "Move to MINIMUM";
      default: return "Unknown";
    }
  } else if (currentCalType == "MPU6500") {
    switch (calState) {
      case CAL_LEVEL: return "Hold LEVEL";
      case CAL_FORWARD: return "Tilt FORWARD";
      case CAL_BACKWARD: return "Tilt BACKWARD";
      case CAL_LEFT: return "Tilt LEFT";
      case CAL_RIGHT: return "Tilt RIGHT";
      default: return "Unknown";
    }
  }
  return "Unknown";
}

// Simplified MPU6500 functions for calibration
void initMPU6500() {
  Wire.beginTransmission(0x68);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission(true);
  delay(100);
}

void readMPU6500(float &roll, float &pitch) {
  // Read accelerometer data
  Wire.beginTransmission(0x68);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(0x68, 6, true);
  
  int16_t accelX = Wire.read() << 8 | Wire.read();
  int16_t accelY = Wire.read() << 8 | Wire.read();
  int16_t accelZ = Wire.read() << 8 | Wire.read();
  
  // Convert to g
  float ax = accelX / 16384.0;
  float ay = accelY / 16384.0;
  float az = accelZ / 16384.0;
  
  // Calculate angles
  roll = atan2(ay, sqrt(ax * ax + az * az)) * 180.0 / PI;
  pitch = atan2(-ax, sqrt(ay * ay + az * az)) * 180.0 / PI;
}

#endif