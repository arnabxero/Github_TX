/*
  menu_display.h - Display and UI Functions (FIXED VERSION)
  RC Transmitter for Arduino Mega
*/

#ifndef MENU_DISPLAY_H
#define MENU_DISPLAY_H

#include "config.h"
#include "display.h"
#include "menu_data.h"

// Display constants
#define MENU_ITEM_HEIGHT 12
#define MENU_START_Y 17
#define SCROLLBAR_WIDTH 4
#define SCROLLBAR_X 124

// External variables from menu.h
extern int menuSelection;
extern int menuOffset;
extern int maxMenuItems;
extern int maxVisibleItems;
extern MenuState currentMenu;
extern bool cancelConfirmActive;
extern int cancelSelection;

// Function declarations
void drawMainMenus();
void drawScrollableMenu(MenuItem* items, int itemCount, String header);
void drawScrollbar(int totalItems, int visibleItems, int offset);
void drawCancelConfirmation();

void drawMainMenus() {
  switch (currentMenu) {
    case MENU_MAIN: {
      MenuItem items[] = {
        {"Calibration", true, true},
        {"Settings", true, true},
        {"System Info", true, true},
        {"Radio Test", true, false},
        {"Display Test", true, false},
        {"Factory Reset", true, false},
        {"Exit", true, false}
      };
      drawScrollableMenu(items, 7, "RC TX MENU");
      break;
    }
    
    case MENU_CALIBRATION: {
      MenuItem items[] = {
        {"Joystick Cal", true, true},
        {"Potentiometer Cal", true, true},
        {"MPU6500 Cal " + getCalibrationStatus("MPU"), true, false},
        {"Back", true, false}
      };
      drawScrollableMenu(items, 4, "Calibration");
      break;
    }
    
    case MENU_JOYSTICK_CAL: {
      MenuItem items[] = {
        {"Right X " + getCalibrationStatus("RIGHT_X"), true, false},
        {"Right Y " + getCalibrationStatus("RIGHT_Y"), true, false},
        {"Left X " + getCalibrationStatus("LEFT_X"), true, false},
        {"Left Y " + getCalibrationStatus("LEFT_Y"), true, false},
        {"Back", true, false}
      };
      drawScrollableMenu(items, 5, "Joystick Cal");
      break;
    }
    
    case MENU_POTENTIOMETER_CAL: {
      MenuItem items[] = {
        {"Left Pot " + getCalibrationStatus("LEFT_POT"), true, false},
        {"Right Pot " + getCalibrationStatus("RIGHT_POT"), true, false},
        {"Back", true, false}
      };
      drawScrollableMenu(items, 3, "Potentiometer Cal");
      break;
    }
    
    case MENU_SETTINGS: {
      MenuItem items[] = {
        {"Joystick Deadzone", true, false},
        {"Display Brightness", true, false},
        {"LED Settings", true, true},
        {"Radio Address", true, false},
        {"Radio Channel", true, false},
        {"Failsafe Settings", true, true},
        {"Reset to Defaults", true, false},
        {"Back", true, false}
      };
      drawScrollableMenu(items, 8, "Settings");
      break;
    }
    
    case MENU_LED_SETTINGS: {
      MenuItem items[] = {
        {"LED Enable: " + String(settings.ledEnabled ? "ON" : "OFF"), true, false},
        {"Armed Color", true, false},
        {"Disarmed Color", true, false},
        {"Transmit Color", true, false},
        {"Error Color", true, false},
        {"Menu Color", true, false},
        {"Back", true, false}
      };
      drawScrollableMenu(items, 7, "LED Settings");
      break;
    }
    
    case MENU_FAILSAFE_SETTINGS: {
      MenuItem items[] = {
        {"Enable: " + String(settings.failsafeEnabled ? "ON" : "OFF"), true, false},
        {"Set Throttle: " + String(settings.failsafeThrottle), true, false},
        {"Set Steering: " + String(settings.failsafeSteering), true, false},
        {"Back", true, false}
      };
      drawScrollableMenu(items, 4, "Failsafe");
      break;
    }
    
    case MENU_INFO: {
      MenuItem items[] = {
        {"Firmware v3.0", false, false},
        {"Free Memory: " + String(freeMemory()), false, false},
        {"Back", true, false}
      };
      drawScrollableMenu(items, 3, "System Info");
      break;
    }
  }
}

void drawScrollableMenu(MenuItem* items, int itemCount, String header) {
  // Draw header
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(header);
  
  // Draw menu items
  int visibleItems = min(maxVisibleItems, itemCount);
  
  for (int i = 0; i < visibleItems; i++) {
    int itemIndex = menuOffset + i;
    if (itemIndex >= itemCount) break;
    
    int yPos = MENU_START_Y + (i * MENU_ITEM_HEIGHT);
    
    // Draw selection box
    if (itemIndex == menuSelection) {
      display.fillRect(0, yPos, 120, MENU_ITEM_HEIGHT, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
    } else {
      display.setTextColor(SSD1306_WHITE);
    }
    
    // Draw menu item text
    display.setCursor(2, yPos + 2);
    display.print(items[itemIndex].title);
    
    // Draw submenu indicator
    if (items[itemIndex].hasSubmenu) {
      display.setCursor(110, yPos + 2);
      display.print(">");
    }
    
    // Reset text color
    display.setTextColor(SSD1306_WHITE);
  }
  
  // Draw scrollbar if needed
  if (itemCount > maxVisibleItems) {
    drawScrollbar(itemCount, visibleItems, menuOffset);
  }
}

void drawScrollbar(int totalItems, int visibleItems, int offset) {
  // Calculate scrollbar dimensions
  int scrollbarHeight = (visibleItems * MENU_ITEM_HEIGHT * maxVisibleItems) / totalItems;
  int scrollbarY = MENU_START_Y + (offset * MENU_ITEM_HEIGHT * maxVisibleItems) / totalItems;
  
  // Draw scrollbar track
  display.drawRect(SCROLLBAR_X, MENU_START_Y, SCROLLBAR_WIDTH, maxVisibleItems * MENU_ITEM_HEIGHT, SSD1306_WHITE);
  
  // Draw scrollbar thumb
  display.fillRect(SCROLLBAR_X + 1, scrollbarY, SCROLLBAR_WIDTH - 2, scrollbarHeight, SSD1306_WHITE);
}

void drawCancelConfirmation() {
  // Draw background box
  display.fillRect(20, 20, 88, 24, SSD1306_WHITE);
  display.drawRect(20, 20, 88, 24, SSD1306_BLACK);
  
  // Draw text
  display.setTextColor(SSD1306_BLACK);
  display.setCursor(25, 25);
  display.println("Cancel Operation?");
  
  // Draw buttons
  display.setCursor(25, 35);
  if (cancelSelection == 0) {
    display.print("[Cancel]");
  } else {
    display.print("Cancel");
  }
  
  display.print("  ");
  
  if (cancelSelection == 1) {
    display.print("[OK]");
  } else {
    display.print("OK");
  }
  
  display.setTextColor(SSD1306_WHITE);
}

#endif