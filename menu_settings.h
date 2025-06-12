/*
  menu_settings.h - Settings Management (CORRECTED VERSION - FIXED DISPLAY TEXT POSITIONING)
  RC Transmitter for Arduino Mega
*/

#ifndef MENU_SETTINGS_H
#define MENU_SETTINGS_H

#include "config.h"
#include "display.h"
#include "menu_data.h"

// Constants
#define NAV_DEBOUNCE 200
#define RAPID_CHANGE_THRESHOLD 1500  // Hold for 1.5 seconds for rapid change
#define RAPID_CHANGE_INTERVAL 100    // Change every 100ms when rapid

// Settings variables
bool settingActive = false;
bool keyboardActive = false;
int keyboardCursorPos = 0;
int keyboardCharPos = 0;
String keyboardInput = "";
String keyboardChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

// Setting completion lockout variables
bool settingJustCompleted = false;
unsigned long settingCompletionTime = 0;
#define SETTING_LOCKOUT_PERIOD 1000  // 1 second lockout after completing a setting

// Cancel tracking to prevent saving on cancel
bool settingBeingCancelled = false;

// LED color setting variables
LEDColorMode currentLEDMode = LED_COLOR_ARMED;
int ledColorComponent = 0; // 0=R, 1=G, 2=B

// Rapid change variables
unsigned long buttonPressStartTime = 0;
bool rapidChangeActive = false;
unsigned long lastRapidChange = 0;
int lastNavDirection = 0;

// External variables from menu.h
extern MenuState currentMenu;
extern int menuSelection;
extern int menuOffset;
extern int maxMenuItems;
extern unsigned long lastNavigation;
extern unsigned long menuTimer;

// Forward declarations for external functions
extern ButtonStates buttons;
extern int getNavigationDirection();

// Function declarations
void initMenuSettings();
void updateMenuSettings();
void handleSettingNavigation();
void handleKeyboardNavigation();
void startSetting(String settingType);
void completeSetting();
void cancelSetting();
void exitMenuSettings();
void goBackSettings();
void handleLEDSettingsSelection(int selection);
void handleFailsafeSettingsSelection(int selection);
void resetAllSettings();
void drawMenuSettings();
void drawSettingScreen();
void drawFailsafeSettingScreen();
void drawKeyboardScreen();
void drawSettingSaveScreen();
bool isSettingActive();
bool* getCurrentLEDColorArray();
bool isInSettingLockout();

// Helper function - defined first so it can be used by other functions
bool* getCurrentLEDColorArray() {
  switch(currentLEDMode) {
    case LED_COLOR_ARMED: return settings.ledArmedColor;
    case LED_COLOR_DISARMED: return settings.ledDisarmedColor;
    case LED_COLOR_TRANSMITTING: return settings.ledTransmitColor;
    case LED_COLOR_ERROR: return settings.ledErrorColor;
    case LED_COLOR_MENU: return settings.ledMenuColor;
    default: return settings.ledArmedColor;
  }
}

void drawSettingSaveScreen() {
  // Fill entire screen with white background
  display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
  
  // Calculate progress (0 to 1000ms)
  unsigned long elapsed = millis() - settingCompletionTime;
  float progress = (float)elapsed / (float)SETTING_LOCKOUT_PERIOD;
  if (progress > 1.0) progress = 1.0;
  
  // Draw "Setting Being Saved" text in black (since background is white)
  display.setTextColor(SSD1306_BLACK);
  display.setTextSize(1);
  
  // Center the text
  String saveText = "Setting Being Saved";
  int textWidth = saveText.length() * 6; // Approximate character width
  int textX = (SCREEN_WIDTH - textWidth) / 2;
  int textY = 20;
  
  display.setCursor(textX, textY);
  display.println(saveText);
  
  // Draw progress bar
  int progressBarX = 20;
  int progressBarY = 35;
  int progressBarWidth = SCREEN_WIDTH - 40; // 20px margin on each side
  int progressBarHeight = 8;
  
  // Draw progress bar outline in black
  display.drawRect(progressBarX, progressBarY, progressBarWidth, progressBarHeight, SSD1306_BLACK);
  
  // Fill progress bar based on elapsed time
  int fillWidth = (int)(progress * (progressBarWidth - 2)); // -2 for border
  if (fillWidth > 0) {
    display.fillRect(progressBarX + 1, progressBarY + 1, fillWidth, progressBarHeight - 2, SSD1306_BLACK);
  }
  
  // Show percentage
  String percentText = String((int)(progress * 100)) + "%";
  int percentWidth = percentText.length() * 6;
  int percentX = (SCREEN_WIDTH - percentWidth) / 2;
  int percentY = 50;
  
  display.setCursor(percentX, percentY);
  display.println(percentText);
  
  // Reset text color back to white for other functions
  display.setTextColor(SSD1306_WHITE);
}

void initMenuSettings() {
  // Initialize settings subsystem
  settingActive = false;
  keyboardActive = false;
  rapidChangeActive = false;
  settingJustCompleted = false;
  settingCompletionTime = 0;
  settingBeingCancelled = false; // Initialize cancel flag
}

void updateMenuSettings() {
  // Check if we're in the lockout period after completing a setting
  if (settingJustCompleted) {
    if (millis() - settingCompletionTime > SETTING_LOCKOUT_PERIOD) {
      settingJustCompleted = false;
      Serial.println("Setting lockout period ended - menu navigation re-enabled");
    } else {
      // During lockout, ignore all input to prevent accidental menu selection
      return;
    }
  }
  
  if (keyboardActive) {
    handleKeyboardNavigation();
  } else {
    handleSettingNavigation();
  }
}

void handleSettingNavigation() {
  static bool lastOkState = false;
  bool currentOkState = buttons.btnOK;
  
  // Handle OK button with debounce to prevent double-entry
  if (currentOkState && !lastOkState) {
    // OK button just pressed
    if (millis() - lastNavigation > NAV_DEBOUNCE) {
      completeSetting();
      lastNavigation = millis();
      return; // Exit immediately to prevent further processing
    }
  }
  lastOkState = currentOkState;
  
  if (millis() - lastNavigation < NAV_DEBOUNCE && !rapidChangeActive) return;
  
  int navDirection = getNavigationDirection();
  
  // Handle rapid change for continuous button press
  if (navDirection != 0) {
    if (lastNavDirection == navDirection) {
      // Same direction - check for rapid change
      if (!rapidChangeActive && millis() - buttonPressStartTime > RAPID_CHANGE_THRESHOLD) {
        rapidChangeActive = true;
        Serial.println("Rapid change activated");
      }
    } else {
      // Direction changed - reset rapid change
      buttonPressStartTime = millis();
      rapidChangeActive = false;
      lastNavDirection = navDirection;
    }
  } else {
    // No input - reset rapid change
    rapidChangeActive = false;
    lastNavDirection = 0;
  }
  
  // Apply changes based on rapid mode
  bool shouldChange = false;
  if (!rapidChangeActive) {
    shouldChange = (navDirection != 0);
  } else {
    shouldChange = (navDirection != 0) && (millis() - lastRapidChange > RAPID_CHANGE_INTERVAL);
  }
  
  if (shouldChange) {
    menuTimer = millis();
    
    if (currentMenu == MENU_DEADZONE_SETTING) {
      if (navDirection == 2 || navDirection == 1) { // Right or Down - increase
        settings.joystickDeadzone = min(200, settings.joystickDeadzone + (rapidChangeActive ? 10 : 5));
      } else if (navDirection == -2 || navDirection == -1) { // Left or Up - decrease
        settings.joystickDeadzone = max(0, settings.joystickDeadzone - (rapidChangeActive ? 10 : 5));
      }
    } else if (currentMenu == MENU_BRIGHTNESS_SETTING) {
      if (navDirection == 2 || navDirection == 1) { // Right or Down - increase
        settings.displayBrightness = min(255, settings.displayBrightness + (rapidChangeActive ? 25 : 10));
        applyDisplayBrightness();
      } else if (navDirection == -2 || navDirection == -1) { // Left or Up - decrease
        settings.displayBrightness = max(50, settings.displayBrightness - (rapidChangeActive ? 25 : 10));
        applyDisplayBrightness();
      }
    } else if (currentMenu == MENU_LED_COLOR_SETTING) {
      if (navDirection == 1) { // Down - next component
        ledColorComponent = (ledColorComponent + 1) % 3;
      } else if (navDirection == -1) { // Up - previous component
        ledColorComponent = (ledColorComponent + 2) % 3;
      } else if (navDirection == 2) { // Right - toggle component
        bool* colorArray = getCurrentLEDColorArray();
        colorArray[ledColorComponent] = !colorArray[ledColorComponent];
        applyLEDSettings();  // Apply immediately when color changes
        Serial.print("LED color component ");
        Serial.print(ledColorComponent);
        Serial.print(" toggled to: ");
        Serial.println(colorArray[ledColorComponent] ? "ON" : "OFF");
      } else if (navDirection == -2) { // Left - toggle component
        bool* colorArray = getCurrentLEDColorArray();
        colorArray[ledColorComponent] = !colorArray[ledColorComponent];
        applyLEDSettings();  // Apply immediately when color changes
        Serial.print("LED color component ");
        Serial.print(ledColorComponent);
        Serial.print(" toggled to: ");
        Serial.println(colorArray[ledColorComponent] ? "ON" : "OFF");
      }
    } else if (currentMenu == MENU_CHANNEL_SETTINGS) {
      if (navDirection == 2 || navDirection == 1) { // Right or Down - increase
        settings.radioChannel = min(125, settings.radioChannel + (rapidChangeActive ? 5 : 1));
      } else if (navDirection == -2 || navDirection == -1) { // Left or Up - decrease
        settings.radioChannel = max(0, settings.radioChannel - (rapidChangeActive ? 5 : 1));
      }
    } else if (currentMenu == MENU_FAILSAFE_THROTTLE_SETTING) {
      if (navDirection == 2 || navDirection == 1) { // Right or Down - increase
        settings.failsafeThrottle = min(1000, settings.failsafeThrottle + (rapidChangeActive ? 50 : 10));
      } else if (navDirection == -2 || navDirection == -1) { // Left or Up - decrease
        settings.failsafeThrottle = max(-1000, settings.failsafeThrottle - (rapidChangeActive ? 50 : 10));
      }
    } else if (currentMenu == MENU_FAILSAFE_STEERING_SETTING) {
      if (navDirection == 2 || navDirection == 1) { // Right or Down - increase
        settings.failsafeSteering = min(1000, settings.failsafeSteering + (rapidChangeActive ? 50 : 10));
      } else if (navDirection == -2 || navDirection == -1) { // Left or Up - decrease
        settings.failsafeSteering = max(-1000, settings.failsafeSteering - (rapidChangeActive ? 50 : 10));
      }
    }
    
    if (rapidChangeActive) {
      lastRapidChange = millis();
    } else {
      lastNavigation = millis();
    }
  }
}

void handleKeyboardNavigation() {
  if (millis() - lastNavigation < NAV_DEBOUNCE) return;
  
  int navDirection = getNavigationDirection();
  if (navDirection != 0) {
    menuTimer = millis();
    
    if (navDirection == 2) { // Right - next character
      keyboardCharPos = (keyboardCharPos + 1) % keyboardChars.length();
    } else if (navDirection == -2) { // Left - previous character
      keyboardCharPos = (keyboardCharPos + keyboardChars.length() - 1) % keyboardChars.length();
    } else if (navDirection == 1) { // Down - next row (skip 9 chars for new layout)
      keyboardCharPos = (keyboardCharPos + 9) % keyboardChars.length();
    } else if (navDirection == -1) { // Up - previous row (skip 9 chars for new layout)
      keyboardCharPos = (keyboardCharPos + keyboardChars.length() - 9) % keyboardChars.length();
    }
    
    lastNavigation = millis();
  }
  
  // Check for OK to select character
  if (buttons.btnOK && millis() - lastNavigation > NAV_DEBOUNCE) {
    if (keyboardCursorPos < 5) { // Max 5 characters for radio address
      if (keyboardCursorPos >= keyboardInput.length()) {
        keyboardInput += keyboardChars[keyboardCharPos];
      } else {
        keyboardInput[keyboardCursorPos] = keyboardChars[keyboardCharPos];
      }
      keyboardCursorPos++;
    }
    lastNavigation = millis();
  }
  
  // Check for backspace (left joystick button)
  if (buttons.leftJoyBtn && millis() - lastNavigation > NAV_DEBOUNCE) {
    if (keyboardInput.length() > 0 && keyboardCursorPos > 0) {
      keyboardInput.remove(keyboardCursorPos - 1, 1);
      keyboardCursorPos--;
    }
    lastNavigation = millis();
  }
  
  // Check for SAVE (right joystick button - only in keyboard mode)
  if (buttons.rightJoyBtn && millis() - lastNavigation > NAV_DEBOUNCE) {
    completeSetting();
    lastNavigation = millis();
  }
}

void startSetting(String settingType) {
  Serial.print("Starting setting: ");
  Serial.println(settingType);
  
  settingActive = true;
  rapidChangeActive = false;
  
  if (settingType == "DEADZONE") {
    currentMenu = MENU_DEADZONE_SETTING;
  } else if (settingType == "BRIGHTNESS") {
    currentMenu = MENU_BRIGHTNESS_SETTING;
  } else if (settingType == "RADIO_ADDRESS") {
    currentMenu = MENU_RADIO_ADDRESS;
    keyboardActive = true;
    keyboardCursorPos = 0;
    keyboardCharPos = 0;
    keyboardInput = String(settings.radioAddress);
  } else if (settingType == "CHANNEL") {
    currentMenu = MENU_CHANNEL_SETTINGS;
  } else if (settingType == "FAILSAFE_THROTTLE") {
    currentMenu = MENU_FAILSAFE_THROTTLE_SETTING;
  } else if (settingType == "FAILSAFE_STEERING") {
    currentMenu = MENU_FAILSAFE_STEERING_SETTING;
  }
}

void completeSetting() {
  // Check if this is actually a cancel operation
  if (settingBeingCancelled) {
    cancelSetting();
    return;
  }
  
  Serial.println("Setting complete - SAVING");
  
  if (currentMenu == MENU_RADIO_ADDRESS) {
    // Copy keyboard input to settings
    keyboardInput.toCharArray(settings.radioAddress, 6);
    keyboardActive = false;
  }
  
  saveSettings();  // ONLY save when completing via OK button
  settingActive = false;
  rapidChangeActive = false;
  
  // Return to appropriate parent menu
  if (currentMenu == MENU_FAILSAFE_THROTTLE_SETTING || currentMenu == MENU_FAILSAFE_STEERING_SETTING) {
    currentMenu = MENU_FAILSAFE_SETTINGS;
    maxMenuItems = 4; // Updated to 4 since we removed test failsafe
  } else {
    currentMenu = MENU_SETTINGS;
    maxMenuItems = 8;
  }
  
  menuSelection = 0;
  menuOffset = 0;
  
  // CRITICAL FIX: Enable setting completion lockout
  settingJustCompleted = true;
  settingCompletionTime = millis();
  
  // Reset all timing and state variables
  lastNavigation = millis();
  menuTimer = millis();
  rapidChangeActive = false;
  buttonPressStartTime = 0;
  lastRapidChange = 0;
  lastNavDirection = 0;
  
  Serial.println("Setting lockout enabled - preventing menu actions for 1 second");
}

void cancelSetting() {
  Serial.println("Setting cancelled - NOT SAVING");
  
  // DON'T save settings - just reload from EEPROM to restore original values
  loadSettings();
  applyLEDSettings();     // Restore LED state
  applyDisplayBrightness(); // Restore display brightness
  
  settingActive = false;
  keyboardActive = false;
  rapidChangeActive = false;
  settingBeingCancelled = false; // Reset cancel flag
  
  // Return to appropriate parent menu
  if (currentMenu == MENU_FAILSAFE_THROTTLE_SETTING || currentMenu == MENU_FAILSAFE_STEERING_SETTING) {
    currentMenu = MENU_FAILSAFE_SETTINGS;
    maxMenuItems = 4;
  } else {
    currentMenu = MENU_SETTINGS;
    maxMenuItems = 8;
  }
  
  menuSelection = 0;
  menuOffset = 0;
  
  // Reset all timing and state variables
  lastNavigation = millis();
  menuTimer = millis();
  rapidChangeActive = false;
  buttonPressStartTime = 0;
  lastRapidChange = 0;
  lastNavDirection = 0;
}

void exitMenuSettings() {
  // This is called when forcefully exiting settings (like from cancel confirmation)
  // Set the cancel flag so completeSetting() knows this is a cancel operation
  settingBeingCancelled = true;
  
  Serial.println("Forcefully exiting settings - marking as cancelled");
  
  // Don't do the actual cancel here, let completeSetting() handle it
  // This ensures proper cleanup regardless of how the exit happens
}

void goBackSettings() {
  // This is called when cancel/back is used - DON'T save settings
  cancelSetting();
}

void handleLEDSettingsSelection(int selection) {
  switch (selection) {
    case 0: // Toggle LED enabled
      settings.ledEnabled = !settings.ledEnabled;
      applyLEDSettings();  // Apply immediately
      saveSettings();      // Save to EEPROM
      Serial.print("LED Enable toggled to: ");
      Serial.println(settings.ledEnabled ? "ON" : "OFF");
      break;
    case 1: currentLEDMode = LED_COLOR_ARMED; currentMenu = MENU_LED_COLOR_SETTING; settingActive = true; break;
    case 2: currentLEDMode = LED_COLOR_DISARMED; currentMenu = MENU_LED_COLOR_SETTING; settingActive = true; break;
    case 3: currentLEDMode = LED_COLOR_TRANSMITTING; currentMenu = MENU_LED_COLOR_SETTING; settingActive = true; break;
    case 4: currentLEDMode = LED_COLOR_ERROR; currentMenu = MENU_LED_COLOR_SETTING; settingActive = true; break;
    case 5: currentLEDMode = LED_COLOR_MENU; currentMenu = MENU_LED_COLOR_SETTING; settingActive = true; break;
  }
}

void handleFailsafeSettingsSelection(int selection) {
  switch (selection) {
    case 0: // Toggle failsafe enabled
      settings.failsafeEnabled = !settings.failsafeEnabled;
      saveSettings();
      break;
    case 1: startSetting("FAILSAFE_THROTTLE"); return;
    case 2: startSetting("FAILSAFE_STEERING"); return;
  }
}

void resetAllSettings() {
  resetSettings();
  resetCalibration();
  saveSettings();
  saveCalibration();
  applyLEDSettings();
  applyDisplayBrightness();
  Serial.println("All settings reset to defaults");
}

bool isSettingActive() {
  return settingActive;
}

bool isInSettingLockout() {
  return settingJustCompleted && (millis() - settingCompletionTime <= SETTING_LOCKOUT_PERIOD);
}

void drawMenuSettings() {
  // Check if we're in setting lockout and show saving screen
  if (settingJustCompleted && (millis() - settingCompletionTime <= SETTING_LOCKOUT_PERIOD)) {
    drawSettingSaveScreen();
    return;
  }
  
  if (keyboardActive) {
    drawKeyboardScreen();
  } else if (currentMenu == MENU_FAILSAFE_THROTTLE_SETTING || currentMenu == MENU_FAILSAFE_STEERING_SETTING) {
    drawFailsafeSettingScreen();
  } else {
    drawSettingScreen();
  }
}

void drawSettingScreen() {
  display.setTextSize(1);
  display.setCursor(0, 0);
  
  if (currentMenu == MENU_DEADZONE_SETTING) {
    display.println("Joystick Deadzone");
    display.setCursor(0, 16);
    display.print("Value: ");
    display.println(settings.joystickDeadzone);
    
    // Draw bar - positioned higher to fit on screen
    int barWidth = map(settings.joystickDeadzone, 0, 200, 0, 100);
    display.drawRect(10, 28, 102, 8, SSD1306_WHITE);
    display.fillRect(11, 29, barWidth, 6, SSD1306_WHITE);
    
    // Instructions positioned to fit on screen (y=40 and y=52)
    display.setCursor(0, 40);
    display.println("Arrows: Adjust");
    display.setCursor(0, 52);
    display.print("OK: Save");
    
  } else if (currentMenu == MENU_BRIGHTNESS_SETTING) {
    display.println("Display Brightness");
    display.setCursor(0, 16);
    display.print("Value: ");
    display.println(settings.displayBrightness);
    
    // Draw bar - positioned higher to fit on screen
    int barWidth = map(settings.displayBrightness, 50, 255, 0, 100);
    display.drawRect(10, 28, 102, 8, SSD1306_WHITE);
    display.fillRect(11, 29, barWidth, 6, SSD1306_WHITE);
    
    // Instructions positioned to fit on screen (y=40 and y=52)
    display.setCursor(0, 40);
    display.println("Arrows: Adjust");
    display.setCursor(0, 52);
    display.print("OK: Save");
    
  } else if (currentMenu == MENU_LED_COLOR_SETTING) {
    display.println("LED Color Setting");
    display.setCursor(0, 17);
    display.print("Mode: ");
    
    switch(currentLEDMode) {
      case LED_COLOR_ARMED: display.println("Armed"); break;
      case LED_COLOR_DISARMED: display.println("Disarmed"); break;
      case LED_COLOR_TRANSMITTING: display.println("Transmit"); break;
      case LED_COLOR_ERROR: display.println("Error"); break;
      case LED_COLOR_MENU: display.println("Menu"); break;
    }
    
    // Show RGB components - positioned at y=30
    display.setCursor(0, 30);
    String components[] = {"R", "G", "B"};
    bool* colorArray = getCurrentLEDColorArray();
    
    for (int i = 0; i < 3; i++) {
      if (i == ledColorComponent) display.print("[");
      display.print(components[i]);
      display.print(":");
      display.print(colorArray[i] ? "1" : "0");
      if (i == ledColorComponent) display.print("]");
      display.print(" ");
    }
    
    // Instructions positioned to fit on screen (y=42 and y=54)
    display.setCursor(0, 42);
    display.println("Up/Down: Component");
    display.setCursor(0, 54);
    display.print("Left/Right: Toggle");
    
  } else if (currentMenu == MENU_CHANNEL_SETTINGS) {
    display.println("Radio Channel");
    display.setCursor(0, 16);
    display.print("Channel: ");
    display.println(settings.radioChannel);
    
    display.setCursor(0, 28);
    display.print("Freq: ");
    display.print(2400 + settings.radioChannel);
    display.println(" MHz");
    
    // Instructions positioned to fit on screen (y=40 and y=52)
    display.setCursor(0, 40);
    display.println("Arrows: Adjust");
    display.setCursor(0, 52);
    display.print("Hold 1.5s: Rapid");
  }
}

void drawFailsafeSettingScreen() {
  display.setTextSize(1);
  display.setCursor(0, 0);
  
  if (currentMenu == MENU_FAILSAFE_THROTTLE_SETTING) {
    display.println("Failsafe Throttle");
    display.setCursor(0, 16);
    display.print("Value: ");
    display.println(settings.failsafeThrottle);
    
    // Draw bar - positioned higher to fit on screen
    int barWidth = map(settings.failsafeThrottle, -1000, 1000, 0, 100);
    display.drawRect(10, 28, 102, 8, SSD1306_WHITE);
    display.fillRect(11, 29, barWidth, 6, SSD1306_WHITE);
    
    // Center line
    display.drawLine(60, 28, 60, 36, SSD1306_WHITE);
    
    // Instructions positioned to fit on screen (y=40 and y=52)
    display.setCursor(0, 40);
    display.println("Range: -1000 to 1000");
    display.setCursor(0, 52);
    display.print("Arrows: Adjust, OK: Save");
    
  } else if (currentMenu == MENU_FAILSAFE_STEERING_SETTING) {
    display.println("Failsafe Steering");
    display.setCursor(0, 16);
    display.print("Value: ");
    display.println(settings.failsafeSteering);
    
    // Draw bar - positioned higher to fit on screen
    int barWidth = map(settings.failsafeSteering, -1000, 1000, 0, 100);
    display.drawRect(10, 28, 102, 8, SSD1306_WHITE);
    display.fillRect(11, 29, barWidth, 6, SSD1306_WHITE);
    
    // Center line
    display.drawLine(60, 28, 60, 36, SSD1306_WHITE);
    
    // Instructions positioned to fit on screen (y=40 and y=52)
    display.setCursor(0, 40);
    display.println("Range: -1000 to 1000");
    display.setCursor(0, 52);
    display.print("Arrows: Adjust, OK: Save");
  }
}

void drawKeyboardScreen() {
  display.setTextSize(1);
  
  // Show current input in header area with cursor
  display.setCursor(0, 0);
  display.print("Addr: ");
  for (int i = 0; i < 5; i++) {
    if (i < keyboardInput.length()) {
      if (i == keyboardCursorPos) {
        // Blinking cursor effect
        if ((millis() / 500) % 2 == 0) {
          display.print("[");
          display.print(keyboardInput[i]);
          display.print("]");
        } else {
          display.print(" ");
          display.print(keyboardInput[i]);
          display.print(" ");
        }
      } else {
        display.print(keyboardInput[i]);
      }
    } else {
      if (i == keyboardCursorPos) {
        // Blinking cursor for new position
        if ((millis() / 500) % 2 == 0) {
          display.print("[_]");
        } else {
          display.print(" _ ");
        }
      } else {
        display.print("_");
      }
    }
  }
  
  // Draw improved keyboard grid (4 rows x 9 columns) - moved up since we removed instructions
  int startY = 15;  // Moved up from 20 to 15
  int charWidth = 14;
  int charHeight = 11;
  
  for (int row = 0; row < 4; row++) {
    for (int col = 0; col < 9; col++) {
      int charIndex = row * 9 + col;
      if (charIndex >= keyboardChars.length()) break;
      
      int x = col * charWidth;
      int y = startY + row * charHeight;
      
      if (charIndex == keyboardCharPos) {
        display.fillRect(x, y, charWidth - 1, charHeight - 1, SSD1306_WHITE);
        display.setTextColor(SSD1306_BLACK);
      } else {
        display.setTextColor(SSD1306_WHITE);
      }
      
      display.setCursor(x + 4, y + 2);
      display.print(keyboardChars[charIndex]);
      display.setTextColor(SSD1306_WHITE);
    }
  }
}

#endif