/*
  display.h - OLED display functions with Menu Integration
  RC Transmitter for Arduino Mega
*/

#ifndef DISPLAY_H
#define DISPLAY_H

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"
#include "radio.h"
#include "controls.h"

// Forward declare menu functions
extern bool isMenuActive();
extern void drawMenu();

// Display object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Table position and size variables - adjust these to move/resize the table
int table_start_x = 10;
int table_start_y = 16;
int table_length = 118;
int table_width = 38;

// Table text positioning variables
int table_text_offset_x = 4;  // X offset for text inside table cells
int table_text_offset_y = 3;  // Y offset for text inside table cells

// Throttle bar position and size variables
int throttle_bar_x = 0;
int throttle_bar_y = 16;
int throttle_bar_width = 8;
int throttle_bar_length = 48;

// Steering bar position and size variables
int steer_bar_x = 10;
int steer_bar_y = 56;
int steer_bar_width = 8;
int steer_bar_length = 118;

// Function declarations
void initDisplay();
void updateDisplay();
void displayReady();
void drawThrottleBar();
void drawSteeringBar();
void drawValuesTable();
void displayError(const char* message);
void drawMainDisplay();
void drawMenuHint();

void initDisplay() {
  Serial.print("Initializing display... ");
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("FAILED!");
    Serial.println("SSD1306 allocation failed");
    // Continue anyway - maybe display will work later
    return;
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("RC Transmitter");
  display.println("Initializing...");
  display.display();
  
  Serial.println("SUCCESS!");
}

void updateDisplay() {
  // Check if menu is active
  if (isMenuActive()) {
    drawMenu();
    return;
  }
  
  // Draw normal operating display
  drawMainDisplay();
}

void drawMainDisplay() {
  display.clearDisplay();
  
  // === YELLOW AREA (0-15 pixels) ===
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("RC TX - ");
  display.print(isRadioOK() ? "ONLINE" : "OFFLINE");
  
  // Armed status and packet counter
  display.setCursor(0, 8);
  display.print(getArmedStatus() ? "ARMED" : "DISARMED");
  display.print(" PKT:");
  display.print(data.counter);
  
  // === BLUE AREA (16-63 pixels) ===
  
  // Draw control bars
  drawThrottleBar();
  drawSteeringBar();
  
  // Values table - organized layout
  drawValuesTable();
  
  // Draw menu hint at bottom
  drawMenuHint();
  
  display.display();
}

void drawMenuHint() {
  // Show menu access hint at the bottom right
  display.setTextSize(1);
  display.setCursor(85, 56);
  display.print("Hold OK");
}

void drawThrottleBar() {
  // Vertical bar positioned and sized using adjustable variables
  int barX = throttle_bar_x;
  int barY = throttle_bar_y;
  int barWidth = throttle_bar_width;
  int barHeight = throttle_bar_length;
  
  // Draw outline
  display.drawRect(barX, barY, barWidth, barHeight, SSD1306_WHITE);
  
  // Calculate fill height based on throttle value
  int fillHeight = map(abs(data.throttle), 0, 1000, 0, barHeight / 2 - 1);
  
  if (data.throttle > 0) {
    // Forward - fill from center up
    int fillY = barY + (barHeight / 2) - fillHeight;
    display.fillRect(barX + 1, fillY, barWidth - 2, fillHeight, SSD1306_WHITE);
  } else if (data.throttle < 0) {
    // Reverse - fill from center down
    int fillY = barY + (barHeight / 2);
    display.fillRect(barX + 1, fillY, barWidth - 2, fillHeight, SSD1306_WHITE);
  }
  
  // Center line
  int centerY = barY + barHeight / 2;
  display.drawLine(barX, centerY, barX + barWidth, centerY, SSD1306_WHITE);
}

void drawSteeringBar() {
  // Horizontal bar positioned and sized using adjustable variables
  int barX = steer_bar_x;
  int barY = steer_bar_y;
  int barWidth = steer_bar_length;
  int barHeight = steer_bar_width;
  
  // Draw outline
  display.drawRect(barX, barY, barWidth, barHeight, SSD1306_WHITE);
  
  // Calculate fill width based on steering value
  int fillWidth = map(abs(data.steering), 0, 1000, 0, barWidth / 2 - 1);
  
  if (data.steering > 0) {
    // Right - fill from center right
    int fillX = barX + (barWidth / 2);
    display.fillRect(fillX, barY + 1, fillWidth, barHeight - 2, SSD1306_WHITE);
  } else if (data.steering < 0) {
    // Left - fill from center left
    int fillX = barX + (barWidth / 2) - fillWidth;
    display.fillRect(fillX, barY + 1, fillWidth, barHeight - 2, SSD1306_WHITE);
  }
  
  // Center line
  int centerX = barX + barWidth / 2;
  display.drawLine(centerX, barY, centerX, barY + barHeight, SSD1306_WHITE);
}

void displayReady() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("RC Transmitter");
  display.println("Ready!");
  display.println("");
  display.print("Radio: ");
  display.println(isRadioOK() ? "OK" : "FAIL");
  display.println("");
  display.println("Right Joy X = Steer");
  display.println("Left Joy Y = Throttle");
  display.println("");
  display.println("Hold OK for menu");
  display.display();
  delay(2000); // Show for 2 seconds
}

void drawValuesTable() {
  // Table positioned and sized using adjustable variables
  int tableX = table_start_x;
  int tableY = table_start_y;
  int tableWidth = table_length;
  int tableHeight = table_width;
  
  // Draw the outer table outline
  display.drawRect(tableX, tableY, tableWidth, tableHeight, SSD1306_WHITE);
  
  // Column and row dimensions (for positioning text and lines)
  int col1Width = tableWidth * 0.26;  // ~31px for "THR"/"STR"
  int col2Width = tableWidth * 0.38;  // ~45px for values
  int col3Width = tableWidth * 0.36;  // ~42px for raw values
  int headerHeight = tableHeight * 0.33;  // ~13px
  int rowHeight = tableHeight * 0.33;     // ~13px
  
  // Draw vertical column separators
  display.drawLine(tableX + col1Width, tableY, tableX + col1Width, tableY + tableHeight - 1, SSD1306_WHITE);
  display.drawLine(tableX + col1Width + col2Width, tableY, tableX + col1Width + col2Width, tableY + tableHeight - 1, SSD1306_WHITE);
  
  // Set text size
  display.setTextSize(1);
  
  // Header row
  display.setCursor(tableX + table_text_offset_x, tableY + table_text_offset_y);
  display.print("#");
  display.setCursor(tableX + col1Width + table_text_offset_x + 3, tableY + table_text_offset_y);
  display.print("VAL");
  display.setCursor(tableX + col1Width + col2Width + table_text_offset_x + 2, tableY + table_text_offset_y);
  display.print("RAW");
  
  // THR row
  display.setCursor(tableX + table_text_offset_x, tableY + headerHeight + table_text_offset_y);
  display.print("THR");
  display.setCursor(tableX + col1Width + table_text_offset_x, tableY + headerHeight + table_text_offset_y);
  display.print(data.throttle);
  display.setCursor(tableX + col1Width + col2Width + table_text_offset_x, tableY + headerHeight + table_text_offset_y);
  display.print(analogRead(LEFT_JOY_Y));
  
  // STR row
  display.setCursor(tableX + table_text_offset_x, tableY + headerHeight + rowHeight + table_text_offset_y);
  display.print("STR");
  display.setCursor(tableX + col1Width + table_text_offset_x, tableY + headerHeight + rowHeight + table_text_offset_y);
  display.print(data.steering);
  display.setCursor(tableX + col1Width + col2Width + table_text_offset_x, tableY + headerHeight + rowHeight + table_text_offset_y);
  display.print(analogRead(RIGHT_JOY_X));
}

void displayError(const char* message) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("ERROR:");
  display.println(message);
  display.display();
}

#endif