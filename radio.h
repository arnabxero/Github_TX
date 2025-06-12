/*
  radio.h - NRF24 communication functions (FIXED LED VERSION)
  RC Transmitter for Arduino Mega
*/

#ifndef RADIO_H
#define RADIO_H

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "config.h"
#include "controls.h"

// Radio object
extern RF24 radio;
extern bool radioOK;

// Function declarations
void initRadio();
void transmitData();
bool isRadioOK();

// Radio implementation
RF24 radio(RADIO_CE, RADIO_CSN);
bool radioOK = false;

void initRadio() {
  Serial.print("Initializing radio... ");
  
  radioOK = radio.begin();
  if (radioOK) {
    radio.setDataRate(RF24_2MBPS);
    radio.setPALevel(RF24_PA_HIGH);
    radio.setChannel(RADIO_CHANNEL);
    radio.setAutoAck(false);
    radio.openWritingPipe((byte*)RADIO_ADDRESS);
    radio.stopListening(); // Transmitter mode
    
    Serial.println("SUCCESS!");
    // CRITICAL FIX: Use applyLEDSettings() instead of direct LED control
    extern void applyLEDSettings();
    applyLEDSettings();
  } else {
    Serial.println("FAILED!");
    // CRITICAL FIX: For radio failure, just use red LED directly
    // The main loop will override this with proper settings anyway
    setLED(true, false, false); // Red for error
    Serial.println("LED set to red due to radio failure");
  }
}

void transmitData() {
  data.counter++;
  bool result = radio.write(&data, sizeof(data));
  
  // CRITICAL FIX: Remove all LED feedback from radio transmission
  // The LED state should be controlled entirely by the menu system
  // based on armed/disarmed/menu state, not transmission status
  
  // Debug output every DEBUG_INTERVAL packets
  if (data.counter % DEBUG_INTERVAL == 0) {
    Serial.print("TX - T:");
    Serial.print(data.throttle);
    Serial.print(" S:");
    Serial.print(data.steering);
    Serial.print(" #");
    Serial.print(data.counter);
    Serial.print(" Result:");
    Serial.println(result ? "OK" : "FAIL");
    
    // Only show transmission status in debug, don't change LEDs
    if (!result) {
      Serial.println("Warning: Transmission failed");
    }
  }
}

bool isRadioOK() {
  return radioOK;
}

#endif