#pragma once

#include <Arduino.h>


#define   DISPLAYTYPE    0    // Either 0 = SURE or 1 = EA  
#define   DEVICETYPE     1    // How the device works. If 0: Show Power & Energy  Mode 1: Show Times (for timers and count downs)                                             
#define   DEBUG_LOCAL    0    // If =1 then show debug values, if =0 then switch them off

// These are the pin wirings

#define   LED           9   // This is a blink LED o n the PCB

#define   WR_CLK        7   // This is the Write clock for a sure LED matrix
#define   DATA          8   // This is the Data for a sure LED matrix
#define   PIN_CS        5   // This is a pin to output the data for the SURE display - not needed when CS and CS_CLK are used.

#define   TX_EA         11    // Tx for the EA display
#define   RX_EA         12    // Rx for the EA Display

// Button parameters
#define RESET_BUTTON_PIN                3
#define DISPLAY_BUTTON_PIN              2

// Button feel settings
#define DEBOUNCED_BUTTON_THRESHOLD      5
#define DEBOUNCED_BUTTON_DELAY          5
#define DEBOUNCED_BUTTON_HELD_MS        300
#define DEBOUNCED_BUTTON_RPT_INITIAL_MS 500
#define DEBOUNCED_BUTTON_RPT_MS         300

#define DISPLAYUPDATEMS                 500 // mS between each display and button press check update

// Serial Read Functions
#define MAX_STRING                      25  // Sets the maximum length of the serial string (probably could be lower)
