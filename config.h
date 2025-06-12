/*
  config.h - Pin definitions and constants
  RC Transmitter for Arduino Mega
*/

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Data structure - MUST match receiver exactly
struct RCData {
  int16_t throttle;  // -1000 to +1000
  int16_t steering;  // -1000 to +1000
  uint32_t counter;  // Packet counter
};

// External data variable
extern RCData data;

// Pin definitions - Joysticks
#define RIGHT_JOY_X A1      // Steering control
#define RIGHT_JOY_Y A0      // Not used for boat
#define RIGHT_JOY_BTN 13
#define LEFT_JOY_X A3       // Not used for boat  
#define LEFT_JOY_Y A2       // Throttle control
#define LEFT_JOY_BTN 12

// Pin definitions - Potentiometers
#define LEFT_POT A8
#define RIGHT_POT A9

// Pin definitions - Triggers
#define LEFT_TRIGGER_DOWN 6
#define LEFT_TRIGGER_UP 7
#define RIGHT_TRIGGER_DOWN 4
#define RIGHT_TRIGGER_UP 5

// Pin definitions - Buttons
#define BUTTON_LEFT 30
#define BUTTON_OK 32
#define BUTTON_DOWN 34
#define BUTTON_UP 36
#define BUTTON_RIGHT 38

// Pin definitions - LEDs
#define LED_RED 24
#define LED_GREEN 26
#define LED_BLUE 28

// Pin definitions - Radio
#define RADIO_CE 9
#define RADIO_CSN 10

// Pin definitions - Display (I2C)
#define DISPLAY_SDA 20  // I2C SDA
#define DISPLAY_SCL 21  // I2C SCL

// Display constants
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCREEN_ADDRESS 0x3C
#define YELLOW_AREA_HEIGHT 16  // Top 16 pixels are yellow
#define BLUE_AREA_HEIGHT 48    // Bottom 48 pixels are blue
#define BLUE_AREA_START 16     // Blue area starts at pixel 16

// Radio constants
#define RADIO_CHANNEL 76
#define RADIO_ADDRESS "BOAT1"

// Timing constants
#define TRANSMIT_INTERVAL 20    // 50Hz transmission
#define DISPLAY_INTERVAL 50     // 20Hz display update
#define DEADZONE_THRESHOLD 50   // Joystick deadzone

// Debug constants
#define DEBUG_INTERVAL 100      // Print debug every 100 packets

#endif