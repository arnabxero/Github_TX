/*
  menu.h - Core Menu System (FIXED VERSION WITH SETTING LOCKOUT)
  RC Transmitter for Arduino Mega
*/

#ifndef MENU_H
#define MENU_H

#include "config.h"
#include "display.h"
#include "controls.h"
#include "menu_data.h"
#include "menu_display.h"
#include "menu_settings.h"
#include "menu_calibration.h"

// Menu navigation variables - declare extern where used in other files
MenuState currentMenu = MENU_HIDDEN;
MenuState previousMenu = MENU_HIDDEN;
int menuSelection = 0;
int menuOffset = 0;
int maxMenuItems = 0;
int maxVisibleItems = 4;
bool menuActive = false;
unsigned long menuTimer = 0;
unsigned long okButtonPressTime = 0;
bool okButtonLongPressed = false;
bool lastOkButtonState = false;

// Navigation timing
unsigned long lastNavigation = 0;
#define NAV_DEBOUNCE 200

// Cancel confirmation variables
bool cancelConfirmActive = false;
int cancelSelection = 0; // 0 = Cancel, 1 = OK

// Function declarations
void initMenu();
void updateMenu();
void handleMenuNavigation();
void enterMenu();
void exitMenu();
void selectMenuItem();
void goBack();
void showCancelConfirm();
void handleCancelConfirmation();
int getNavigationDirection();
bool isMenuActive();
void drawMenu();

// Forward declaration for the lockout check
extern bool isInSettingLockout();
extern void drawSettingSaveScreen();

void initMenu() {
  Serial.println("Initializing menu system...");
  
  // Initialize all subsystems
  initMenuData();
  initMenuSettings();
  initMenuCalibration();
  
  Serial.println("Menu system initialized!");
}

void updateMenu() {
  // Handle cancel confirmation first
  if (cancelConfirmActive) {
    handleCancelConfirmation();
    return;
  }
  
  // Check for right joystick button press (cancel function)
  if (buttons.rightJoyBtn && millis() - lastNavigation > NAV_DEBOUNCE) {
    if (currentMenu != MENU_MAIN && currentMenu != MENU_HIDDEN) {
      showCancelConfirm();
      return;
    }
  }
  
  // Check for OK button long press (2 seconds)
  bool currentOkState = buttons.btnOK;
  
  if (currentOkState && !lastOkButtonState) {
    okButtonPressTime = millis();
    okButtonLongPressed = false;
  } else if (currentOkState && !okButtonLongPressed) {
    if (millis() - okButtonPressTime >= 2000) {
      okButtonLongPressed = true;
      if (currentMenu == MENU_HIDDEN) {
        enterMenu();
      }
    }
  } else if (!currentOkState && lastOkButtonState) {
    if (!okButtonLongPressed && currentMenu != MENU_HIDDEN) {
      // CRITICAL FIX: Check if we're in setting lockout before allowing menu selection
      if (!isInSettingLockout()) {
        selectMenuItem();
      } else {
        Serial.println("Menu selection blocked - setting lockout active");
      }
    }
    okButtonLongPressed = false;
  }
  
  lastOkButtonState = currentOkState;
  
  // Handle different subsystem updates
  if (currentMenu != MENU_HIDDEN) {
    // Update appropriate subsystem
    if (isCalibrationActive()) {
      updateMenuCalibration();
    } else if (isSettingActive()) {
      updateMenuSettings();
    } else {
      // CRITICAL FIX: Only handle navigation if not in setting lockout
      if (!isInSettingLockout()) {
        handleMenuNavigation();
      }
    }
    
    // Auto-exit menu after 30 seconds of inactivity
    if (millis() - menuTimer > 30000) {
      exitMenu();
    }
  }
}

void handleMenuNavigation() {
  if (millis() - lastNavigation < NAV_DEBOUNCE) return;
  
  int navDirection = getNavigationDirection();
  if (navDirection != 0) {
    menuTimer = millis();
    
    if (navDirection == 1) { // Down
      menuSelection++;
      if (menuSelection >= maxMenuItems) {
        menuSelection = 0;
        menuOffset = 0;
      } else if (menuSelection >= menuOffset + maxVisibleItems) {
        menuOffset++;
      }
    } else if (navDirection == -1) { // Up
      menuSelection--;
      if (menuSelection < 0) {
        menuSelection = maxMenuItems - 1;
        menuOffset = max(0, maxMenuItems - maxVisibleItems);
      } else if (menuSelection < menuOffset) {
        menuOffset--;
      }
    } else if (navDirection == 2) { // Right/Select
      // CRITICAL FIX: Double-check lockout before selecting
      if (!isInSettingLockout()) {
        selectMenuItem();
      }
    } else if (navDirection == -2) { // Left/Back
      goBack();
    }
    
    lastNavigation = millis();
  }
}

void handleCancelConfirmation() {
  if (millis() - lastNavigation < NAV_DEBOUNCE) return;
  
  int navDirection = getNavigationDirection();
  if (navDirection != 0) {
    if (navDirection == 2 || navDirection == -2) { // Left or Right
      cancelSelection = 1 - cancelSelection; // Toggle between 0 and 1
    }
    lastNavigation = millis();
  }
  
  // Check for OK to confirm selection
  if (buttons.btnOK && millis() - lastNavigation > NAV_DEBOUNCE) {
    if (cancelSelection == 1) { // OK selected - cancel operation
      exitMenu();
    }
    cancelConfirmActive = false;
    lastNavigation = millis();
  }
}

int getNavigationDirection() {
  // Always allow arrow button navigation
  if (buttons.btnDown) return 1;
  if (buttons.btnUp) return -1;
  if (buttons.btnRight) return 2;
  if (buttons.btnLeft) return -2;
  
  // Allow joystick navigation only when not in special modes
  if (!isSettingActive() && !isCalibrationActive()) {
    int rightJoyY = analogRead(RIGHT_JOY_Y);
    int leftJoyY = analogRead(LEFT_JOY_Y);
    int rightJoyX = analogRead(RIGHT_JOY_X);
    int leftJoyX = analogRead(LEFT_JOY_X);
    
    if (rightJoyY < 200 || leftJoyY > 800) return -1; // Up
    if (rightJoyY > 800 || leftJoyY < 200) return 1;  // Down
    if (rightJoyX < 200 || leftJoyX > 800) return -2; // Left
    if (rightJoyX > 800 || leftJoyX < 200) return 2;  // Right
  }
  
  return 0;
}

void enterMenu() {
  Serial.println("Entering menu...");
  currentMenu = MENU_MAIN;
  maxMenuItems = 7;
  menuSelection = 0;
  menuOffset = 0;
  menuTimer = millis();
  menuActive = true;
  applyLEDSettings(); // Show menu LED color
}

void exitMenu() {
  Serial.println("Exiting menu...");
  currentMenu = MENU_HIDDEN;
  menuActive = false;
  exitMenuCalibration();
  exitMenuSettings();
  cancelConfirmActive = false;
  menuSelection = 0;
  menuOffset = 0;
  applyLEDSettings(); // Return to normal LED state
}

void showCancelConfirm() {
  cancelConfirmActive = true;
  cancelSelection = 0; // Default to "Cancel"
}

void goBack() {
  switch (currentMenu) {
    case MENU_MAIN:
      exitMenu();
      break;
    case MENU_CALIBRATION:
      currentMenu = MENU_MAIN;
      maxMenuItems = 7;
      break;
    case MENU_SETTINGS:
      currentMenu = MENU_MAIN;
      maxMenuItems = 7;
      break;
    case MENU_INFO:
      currentMenu = MENU_MAIN;
      maxMenuItems = 7;
      break;
    case MENU_JOYSTICK_CAL:
    case MENU_POTENTIOMETER_CAL:
    case MENU_MPU6500_CAL:
      currentMenu = MENU_CALIBRATION;
      maxMenuItems = 4;
      break;
    case MENU_LED_SETTINGS:
    case MENU_FAILSAFE_SETTINGS:
      currentMenu = MENU_SETTINGS;
      maxMenuItems = 8;
      break;
    default:
      // Let subsystems handle their own back navigation
      if (isSettingActive()) {
        goBackSettings();
      } else if (isCalibrationActive()) {
        goBackCalibration();
      } else {
        currentMenu = MENU_MAIN;
        maxMenuItems = 7;
      }
      break;
  }
  menuSelection = 0;
  menuOffset = 0;
}

void selectMenuItem() {
  // CRITICAL FIX: Final safety check before any menu action
  if (isInSettingLockout()) {
    Serial.println("selectMenuItem() blocked - setting lockout active");
    return;
  }
  
  switch (currentMenu) {
    case MENU_MAIN:
      switch (menuSelection) {
        case 0: // Calibration
          currentMenu = MENU_CALIBRATION;
          maxMenuItems = 4;
          break;
        case 1: // Settings
          currentMenu = MENU_SETTINGS;
          maxMenuItems = 8;
          break;
        case 2: // System Info
          currentMenu = MENU_INFO;
          maxMenuItems = 3;
          break;
        case 6: // Exit
          exitMenu();
          return;
      }
      break;
      
    case MENU_CALIBRATION:
      switch (menuSelection) {
        case 0: 
          currentMenu = MENU_JOYSTICK_CAL; 
          maxMenuItems = 5; 
          break;
        case 1: 
          currentMenu = MENU_POTENTIOMETER_CAL; 
          maxMenuItems = 3; 
          break;
        case 2: 
          startCalibration("MPU6500", ""); 
          return;
        case 3: 
          goBack(); 
          return;
      }
      break;
      
    case MENU_JOYSTICK_CAL:
      switch (menuSelection) {
        case 0: startCalibration("JOYSTICK", "RIGHT_X"); return;
        case 1: startCalibration("JOYSTICK", "RIGHT_Y"); return;
        case 2: startCalibration("JOYSTICK", "LEFT_X"); return;
        case 3: startCalibration("JOYSTICK", "LEFT_Y"); return;
        case 4: goBack(); return;
      }
      break;
      
    case MENU_POTENTIOMETER_CAL:
      switch (menuSelection) {
        case 0: startCalibration("POTENTIOMETER", "LEFT"); return;
        case 1: startCalibration("POTENTIOMETER", "RIGHT"); return;
        case 2: goBack(); return;
      }
      break;
      
    case MENU_SETTINGS:
      switch (menuSelection) {
        case 0: startSetting("DEADZONE"); return;
        case 1: startSetting("BRIGHTNESS"); return;
        case 2: 
          currentMenu = MENU_LED_SETTINGS; 
          maxMenuItems = 7; 
          break;
        case 3: startSetting("RADIO_ADDRESS"); return;
        case 4: startSetting("CHANNEL"); return;
        case 5: 
          currentMenu = MENU_FAILSAFE_SETTINGS; 
          maxMenuItems = 4; // Updated to 4 since we removed test failsafe
          break;
        case 6: resetAllSettings(); break;
        case 7: goBack(); return;
      }
      break;
      
    case MENU_LED_SETTINGS:
      handleLEDSettingsSelection(menuSelection);
      if (menuSelection == 6) goBack(); // Back option
      return;
      
    case MENU_FAILSAFE_SETTINGS:
      handleFailsafeSettingsSelection(menuSelection);
      if (menuSelection == 3) goBack(); // Back option (now index 3 instead of 4)
      return;
      
    case MENU_INFO:
      if (menuSelection == maxMenuItems - 1) {
        goBack();
        return;
      }
      break;
  }
  
  menuSelection = 0;
  menuOffset = 0;
  menuTimer = millis();
}

bool isMenuActive() {
  return menuActive;
}

// Main display function - delegates to appropriate subsystem
void drawMenu() {
  if (currentMenu == MENU_HIDDEN) return;
  
  display.clearDisplay();
  
  // Check if we're in setting lockout and show saving screen
  if (isInSettingLockout()) {
    drawSettingSaveScreen();
  } else if (cancelConfirmActive) {
    drawCancelConfirmation();
  } else if (isCalibrationActive()) {
    drawMenuCalibration();
  } else if (isSettingActive()) {
    drawMenuSettings();
  } else {
    drawMainMenus();
  }
  
  display.display();
}

#endif