/*
  controls.h - Input handling functions with Calibration Support (FIXED LED VERSION)
  RC Transmitter for Arduino Mega
*/

#ifndef CONTROLS_H
#define CONTROLS_H

#include "config.h"

// Forward declare calibration functions
extern int getCalibratedSteering();
extern int getCalibratedThrottle();

// Function declarations
void initControls();
void readJoysticks();
void checkButtons();
void setLED(bool red, bool green, bool blue);
bool getArmedStatus();

// Button state variables
struct ButtonStates {
  bool leftTriggerDown;
  bool leftTriggerUp;
  bool rightTriggerDown;
  bool rightTriggerUp;
  bool rightJoyBtn;
  bool leftJoyBtn;
  bool btnLeft;
  bool btnRight;
  bool btnUp;
  bool btnDown;
  bool btnOK;
};

ButtonStates buttons;

// Arming system
bool isArmed = false;
bool lastLeftTriggerDown = false;

// Potentiometer values
int leftPotValue = 0;
int rightPotValue = 0;

void initControls() {
  Serial.println("Initializing controls...");
  
  // Setup LED pins
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  
  // Setup button pins with pull-up resistors
  pinMode(LEFT_TRIGGER_DOWN, INPUT_PULLUP);   // LOW = pressed
  pinMode(LEFT_TRIGGER_UP, INPUT_PULLUP);     // LOW = pressed
  pinMode(RIGHT_TRIGGER_DOWN, INPUT_PULLUP);  // LOW = pressed
  pinMode(RIGHT_TRIGGER_UP, INPUT_PULLUP);    // LOW = pressed
  pinMode(RIGHT_JOY_BTN, INPUT_PULLUP);       // LOW = pressed
  pinMode(LEFT_JOY_BTN, INPUT_PULLUP);        // LOW = pressed
  pinMode(BUTTON_LEFT, INPUT_PULLUP);         // LOW = pressed
  pinMode(BUTTON_OK, INPUT_PULLUP);           // LOW = pressed
  pinMode(BUTTON_DOWN, INPUT_PULLUP);         // LOW = pressed
  pinMode(BUTTON_UP, INPUT_PULLUP);           // LOW = pressed
  pinMode(BUTTON_RIGHT, INPUT_PULLUP);        // LOW = pressed
  
  // REMOVED: Set LED to indicate initialization - let menu system handle this
  // The LED will be controlled by applyLEDSettings() instead
  
  Serial.println("Controls initialized!");
}

void readJoysticks() {
  // Only process joystick inputs if ARMED
  if (isArmed) {
    // Use calibrated values if available, otherwise use default mapping
    data.steering = getCalibratedSteering();
    data.throttle = getCalibratedThrottle();
    
    // Apply deadzone to prevent drift
    if (abs(data.steering) < DEADZONE_THRESHOLD) data.steering = 0;
    if (abs(data.throttle) < DEADZONE_THRESHOLD) data.throttle = 0;
  } else {
    // DISARMED - force neutral values
    data.steering = 0;
    data.throttle = 0;
  }
  
  // Read potentiometers (always active)
  leftPotValue = analogRead(LEFT_POT);
  rightPotValue = analogRead(RIGHT_POT);
}

void checkButtons() {
  // Store previous trigger state
  lastLeftTriggerDown = buttons.leftTriggerDown;
  
  // Read all button states (LOW = pressed due to INPUT_PULLUP)
  buttons.leftTriggerDown = !digitalRead(LEFT_TRIGGER_DOWN);     // Invert: LOW input = true pressed
  buttons.leftTriggerUp = !digitalRead(LEFT_TRIGGER_UP);
  buttons.rightTriggerDown = !digitalRead(RIGHT_TRIGGER_DOWN);
  buttons.rightTriggerUp = !digitalRead(RIGHT_TRIGGER_UP);
  buttons.rightJoyBtn = !digitalRead(RIGHT_JOY_BTN);
  buttons.leftJoyBtn = !digitalRead(LEFT_JOY_BTN);
  buttons.btnLeft = !digitalRead(BUTTON_LEFT);
  buttons.btnRight = !digitalRead(BUTTON_RIGHT);
  buttons.btnUp = !digitalRead(BUTTON_UP);
  buttons.btnDown = !digitalRead(BUTTON_DOWN);
  buttons.btnOK = !digitalRead(BUTTON_OK);
  
  // ARMING LOGIC: Left trigger down = ARMED
  if (buttons.leftTriggerDown && !lastLeftTriggerDown) {
    // Trigger just pressed - ARM the system
    isArmed = true;
    Serial.println("SYSTEM ARMED!");
    // CRITICAL FIX: Use applyLEDSettings() instead of direct setLED() call
    extern void applyLEDSettings();
    applyLEDSettings();
  } else if (!buttons.leftTriggerDown && lastLeftTriggerDown) {
    // Trigger just released - DISARM the system
    isArmed = false;
    Serial.println("SYSTEM DISARMED!");
    // CRITICAL FIX: Use applyLEDSettings() instead of direct setLED() call
    extern void applyLEDSettings();
    applyLEDSettings();
  }
  
  // REMOVED: All the direct LED control code that was overriding settings
  // The LED state is now fully controlled by the menu system via applyLEDSettings()
  
  // Other button actions can be added here as needed
  if (buttons.rightTriggerUp) {
    // Right trigger up action - could trigger a special LED mode if needed
    // But we'll let the menu system handle LED colors
  }
}

void setLED(bool red, bool green, bool blue) {
  // LEDs are active LOW (pin LOW = LED ON)
  digitalWrite(LED_RED, !red);
  digitalWrite(LED_GREEN, !green);
  digitalWrite(LED_BLUE, !blue);
  
  // Debug output to track LED changes
  static bool lastRed = false, lastGreen = false, lastBlue = false;
  if (red != lastRed || green != lastGreen || blue != lastBlue) {
    Serial.print("LED Physical Set: R:");
    Serial.print(red ? "ON" : "OFF");
    Serial.print(" G:");
    Serial.print(green ? "ON" : "OFF");
    Serial.print(" B:");
    Serial.println(blue ? "ON" : "OFF");
    lastRed = red; lastGreen = green; lastBlue = blue;
  }
}

// Getter functions for other modules
int getLeftPot() {
  return leftPotValue;
}

int getRightPot() {
  return rightPotValue;
}

ButtonStates getButtonStates() {
  return buttons;
}

// Arming system functions
bool getArmedStatus() {
  return isArmed;
}

#endif