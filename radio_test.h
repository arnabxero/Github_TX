// ===== 1. radio_test.h (NEW FILE) =====

/*
  radio_test.h - NRF24 Radio Testing and Diagnostics
  RC Transmitter for Arduino Mega
*/

#ifndef RADIO_TEST_H
#define RADIO_TEST_H

#include "config.h"
#include "display.h"
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// Radio test variables
bool radioTestActive = false;
unsigned long radioTestStartTime = 0;
bool radioTestCompleted = false;

// Register values structure
struct RadioRegisters {
  uint8_t reg_EN_AA;      // Expected: 0x3f
  uint8_t reg_EN_RXADDR;  // Expected: 0x02  
  uint8_t reg_RF_CH;      // Expected: 0x4c
  uint8_t reg_RF_SETUP;   // Expected: 0x03
  uint8_t reg_CONFIG;     // Expected: 0x0f
};

RadioRegisters testResults;
RadioRegisters expectedValues = {0x3f, 0x02, 0x4c, 0x03, 0x0f};

// External variable declarations
extern RF24 radio;
extern ButtonStates buttons;
extern unsigned long lastNavigation;
extern MenuState currentMenu;
extern int maxMenuItems;
extern int menuSelection;
extern int menuOffset;
extern Adafruit_SSD1306 display;

// Function declarations
void initRadioTest();
void updateRadioTest();
void startRadioTest();
void exitRadioTest();
bool isRadioTestActive();
void drawRadioTest();
void readRadioRegisters();
uint8_t readRegister(uint8_t reg);

void initRadioTest() {
  radioTestActive = false;
  radioTestCompleted = false;
  Serial.println("Radio test system initialized");
}

void updateRadioTest() {
  if (!radioTestActive) return;
  
  // Check for back button (any navigation button or joystick buttons)
  if (buttons.btnLeft || buttons.btnRight || buttons.btnUp || buttons.btnDown || 
      buttons.leftJoyBtn || buttons.rightJoyBtn || buttons.btnOK) {
    
    if (millis() - lastNavigation > 200) { // Debounce
      exitRadioTest();
      lastNavigation = millis();
    }
  }
  
  // Auto-read registers when test starts
  if (!radioTestCompleted && millis() - radioTestStartTime > 500) {
    readRadioRegisters();
    radioTestCompleted = true;
  }
}

void startRadioTest() {
  Serial.println("Starting Radio Test...");
  radioTestActive = true;
  radioTestCompleted = false;
  radioTestStartTime = millis();
  
  currentMenu = MENU_RADIO_TEST;
  
  // Reset test results
  testResults.reg_EN_AA = 0x00;
  testResults.reg_EN_RXADDR = 0x00;
  testResults.reg_RF_CH = 0x00;
  testResults.reg_RF_SETUP = 0x00;
  testResults.reg_CONFIG = 0x00;
}

void exitRadioTest() {
  Serial.println("Exiting Radio Test");
  radioTestActive = false;
  radioTestCompleted = false;
  
  // Return to main menu
  currentMenu = MENU_MAIN;
  maxMenuItems = 7;
  menuSelection = 0;
  menuOffset = 0;
}

bool isRadioTestActive() {
  return radioTestActive;
}

void readRadioRegisters() {
  Serial.println("Reading NRF24 registers...");
  
  // Configure radio for testing
  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  
  byte addresses[][6] = {"1Node", "2Node"};
  radio.openWritingPipe(addresses[0]);
  radio.openReadingPipe(1, addresses[1]); 
  radio.startListening();
  
  // Read the specific registers
  testResults.reg_EN_AA = readRegister(0x01);      // EN_AA register
  testResults.reg_EN_RXADDR = readRegister(0x02);  // EN_RXADDR register  
  testResults.reg_RF_CH = readRegister(0x05);      // RF_CH register
  testResults.reg_RF_SETUP = readRegister(0x06);   // RF_SETUP register
  testResults.reg_CONFIG = readRegister(0x00);     // CONFIG register
  
  // Print results to serial for debugging
  Serial.println("Register Test Results:");
  Serial.print("EN_AA (Expected 0x3f): 0x"); 
  if (testResults.reg_EN_AA < 0x10) Serial.print("0");
  Serial.println(testResults.reg_EN_AA, HEX);
  
  Serial.print("EN_RXADDR (Expected 0x02): 0x"); 
  if (testResults.reg_EN_RXADDR < 0x10) Serial.print("0");
  Serial.println(testResults.reg_EN_RXADDR, HEX);
  
  Serial.print("RF_CH (Expected 0x4c): 0x"); 
  if (testResults.reg_RF_CH < 0x10) Serial.print("0");
  Serial.println(testResults.reg_RF_CH, HEX);
  
  Serial.print("RF_SETUP (Expected 0x03): 0x"); 
  if (testResults.reg_RF_SETUP < 0x10) Serial.print("0");
  Serial.println(testResults.reg_RF_SETUP, HEX);
  
  Serial.print("CONFIG (Expected 0x0f): 0x"); 
  if (testResults.reg_CONFIG < 0x10) Serial.print("0");
  Serial.println(testResults.reg_CONFIG, HEX);
}

uint8_t readRegister(uint8_t reg) {
  uint8_t result;
  
  digitalWrite(RADIO_CSN, LOW);
  SPI.transfer(0x00 | (reg & 0x1F)); // Read command
  result = SPI.transfer(0xFF);       // Read the register value
  digitalWrite(RADIO_CSN, HIGH);
  
  return result;
}

void drawRadioTest() {
  display.setTextSize(1);
  
  // Header (first 16 pixels)
  display.setCursor(0, 0);
  display.println("NRF24 Radio Test");
  display.setCursor(0, 8);
  display.print("# Ideal Yours Stat");
  
  // Table data (remaining 48 pixels, starting at y=16)
  int startY = 16;
  int rowHeight = 9;
  
  // Define register names and their data
  String regNames[] = {"AA", "RX", "CH", "RF", "CF"};
  uint8_t expectedVals[] = {expectedValues.reg_EN_AA, expectedValues.reg_EN_RXADDR, 
                           expectedValues.reg_RF_CH, expectedValues.reg_RF_SETUP, expectedValues.reg_CONFIG};
  uint8_t actualVals[] = {testResults.reg_EN_AA, testResults.reg_EN_RXADDR,
                         testResults.reg_RF_CH, testResults.reg_RF_SETUP, testResults.reg_CONFIG};
  
  for (int i = 0; i < 5; i++) {
    int yPos = startY + (i * rowHeight);
    
    // Register name (2 chars)
    display.setCursor(0, yPos);
    display.print(regNames[i]);
    
    // Expected value (0xXX format)
    display.setCursor(20, yPos);
    if (expectedVals[i] < 0x10) display.print("0");
    display.print(expectedVals[i], HEX);
    
    // Actual value (0xXX format) 
    display.setCursor(54, yPos);
    if (radioTestCompleted) {
      if (actualVals[i] < 0x10) display.print("0");
      display.print(actualVals[i], HEX);
      
      // Status indicator
      display.setCursor(90, yPos);
      if (actualVals[i] == expectedVals[i]) {
        display.print("OK");
      } else {
        display.print("ERR");
      }
    } else {
      display.print("--");
    }
  }
  
  // Instructions at bottom
  display.setCursor(0, 56);
  if (!radioTestCompleted) {
    display.print("Testing...");
  }
}

#endif