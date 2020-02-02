/********************************************************
  /****** LED Power Meter Display CODE ********************
  /****** by Matt Little **********************************
  /****** Date: 20/01/2020 ********************************
  /****** info@re-innovation.co.uk ************************
  /****** www.re-innovation.co.uk *************************
  /********************************************************

  This is a power and energy display program for serial-streamed data from our pedal power meters.
  It reads serial data (on D0/D1, Tx/Rx).
  It parses that data and displays the power on an LED display.

  This code is for an Arduino Nano

  It is designed to work with Embedded Adventures Dsiplays (with appropriate driver)
  OR it can use an HT1632C module - usually 2 pcs of 32 x 08 LED matrixes.

  Display information:


  Code for Sure 8 x 32 LED matrix HT1632 controller board 12V Type: DE-DP131112V200
  These have a CS and a CS CLOCK to control up to 8 boards.
  The HT1632 code was obtained from here:
  https://github.com/gauravmm/HT1632-for-Arduino
  Additional notes for http://store3.sure-electronics.com/de-dp13119
  Using this info
  http://cdn2.boxtec.ch/pub/sure/LED_Dot_Matrix-Display_Users-Guide.pdf
  Need to also clock through the CS
  Need to clock through 8 digital HIGH to get the correct CS as low to all the boards
  In future this will need clocking in correctly
  This has been added to this HT1632C arduino library included in this repository, but probably not in a very nice way!
  https://github.com/re-innovation/HT1632C_DEDP131112

  Code for the Embedded Adventures LED display unit with information here:
  https://www.embeddedadventures.com/datasheets/PLT-1001_hw_v4_doc_v3.pdf


  D0: Tx Serial Data (NOT used)
  D1: Rx Serial Data - reads information from external unit
  D2: Display Mode button (not usually added - done in set-up)
  D3: Button A (reset for User input)
  D4: CS pin - SURE LED board
  D5: Do NOT use!
  D6:
  D7: WR - SURE LED board
  B8: DATA  - SURE LED board
  B9: LED - Blink!
  D10: CS_CLK - SURE LED board
  D11:
  D12: Tx EA Controller
  D13: Rx EA Controller


  /************ External Libraries*****************************/
// General configuration
#include "config.h"

#include <stdlib.h>
#include <avr/pgmspace.h>  // Library for putting data into program memory
#include <EEPROM.h>        // For writing values to the EEPROM
#include <avr/eeprom.h>    // For writing values to EEPROM
#include <SoftwareSerial.h>
#include <Arduino.h>
#include <MutilaDebug.h>
#include <Millis.h>
#include <DebouncedButton.h>

#include "HT1632.h"
#include "font_5x4.h"
#include "font_8x4.h"
#include "images.h"

/************User variables and hardware allocation**********************************************/
//  Check config.h file

// Useful variables
char deviceID[3]; // A buffer to hold the device ID

float graphDisplay[64];         // A buffer to hold the graphical bar graph display
int maxGraphPower;              // Power in Watts which is max power to display ( =16 on output for EA display =8 for SURE display)
String maxGraphPowerStr;
String outputString;            // This is the holder for the data as a string. Start as blank
int displaySize;

// ****** Serial Data Read***********
// Variables for the serial data read
char inByte;                    // incoming serial char
String str_buffer = "";         // This is the holder for the string which we will display

char stringBuffer[MAX_STRING];  // A buffer to hold the string when pulled from program memory

// Count down values
int countdownSeconds = 999;   // Holds the value from the serial read
int oldCountdownSeconds;  // holds the previous value
String countdownSecondsStr;   // Holds the value from the serial read
int winner;             // This holds the winning team
String winnerStr;       // Holds the wining team as string
int countdownTimer;      // holds the time for the countdown
bool winnerFlag = false;
int offset;           // For offset of the data buffer display
int timer;            // For timer display
String timerStr;      // For timner display


String displayStr1;
String displayStr2;
bool displayFlag = false;
char bufDisplay1[10];
char bufDisplay2[10];

// Varibales for writing to EEPROM
int hiByte;      // These are used to store longer variables into EERPRPROM
int loByte;
long int number;

long int oldTime = 0;  // This is for outputting the data every 1S or so
unsigned long int time = 0;  // This holds the averaging
int timeValue = 0;  // Holds the time in 100's milliseconds. Aldo holds the Count Down value
String timeValueStr;

int displayOffset = 0;      // This is for putting the power data in the middle of the display.
int displayOffsetWh = 0;    // This is for putting the energy data in the middle of the display.
int displayMode = 0;        // This decides how to display the information.
int oldDisplayMode;         // For deciding upon write to EEPROM

// Commands for writing to serial port (put into const to save RAM)

int powerAve = 0;  // DC power in
String powerAveStr; // Holds the power value as a string
char powerAveChar[6]; // Holds the power value in a char array
float powerFloat = 0.0;


float energyWs = 0;  // DC power in Watt-Seconds
float energyWh = 0;  // DC power in in Watt-hours
char energyStr[5] = ""; // Holds the power value as a string


// These parameters are used for the EA display unit
String hello = "HELLO";
String paint = "paint\r";
String clearLED = "clear\r";

// Sort out buttons
DebouncedButton ResetButton(RESET_BUTTON_PIN, true);
DebouncedButton DisplayButton(DISPLAY_BUTTON_PIN, true);

boolean checkStringOK(String dataString)
{
  boolean fail = true; // Holds the check
  // Check the string is all numbers, not characters
  for (int n = 0; n < dataString.length(); n++)
  {
    if (isDigit(dataString[n]) == false)
    {
      fail = false;
    }
  }
  return (fail);
}


// ******* END Variables *************

//#if DISPLAYTYPE == 1
// This is an EA display so set up the serial port and other parameters
SoftwareSerial mySerial(RX_EA, TX_EA); // RX, TX
//#endif

void setup() {

  Serial.begin(115200);    // Set up a serial output for data display and changing parameters
  Serial.flush();
  pinMode(LED, OUTPUT);

  // Initialize button objects
  ResetButton.begin();
  DisplayButton.begin();

  if (DISPLAYTYPE == 0)
  {
    // This is the SURE display set-up
    //  This has been changed for these displays to: CS, CS_CLK,WR_CLK,DATA
    //  HT1632.begin(pinCS, pinWR, pinDATA);
    // In this example the pinCS is not used - CS and CS_CLK are set in HT1632.h.
    HT1632.begin(PIN_CS, WR_CLK, DATA);
    HT1632.clear(); // This zeroes out the internal memory.
    HT1632.render(); // This updates the screen display.
    HT1632.renderTarget(1);
    HT1632.drawText("HELLO", 5, 2, FONT_5X4, FONT_5X4_END, FONT_5X4_HEIGHT);
    HT1632.render(); // This updates the screen display.

  }
  else if (DISPLAYTYPE == 1)
  {
    // This is the EA display set-up
    // set the data rate for the SoftwareSerial port
    mySerial.begin(115200);
    // Clear the display
    mySerial.print(clearLED);
    mySerial.print(clearLED);
    delay(20);
    mySerial.print(paint);
    delay(20);
    mySerial.print("font 6\r");
    mySerial.print("text 2 3 24 ");
    mySerial.print('"');
    mySerial.print(hello);  // Start message
    mySerial.print('"');
    mySerial.print("\r");
    // Then we display it all
    mySerial.print(paint);
  }

  // Read in the values from EEPROM
  // Read the device ID
  deviceID[0] = char(EEPROM.read(0));
  deviceID[1] = char(EEPROM.read(1));
  Serial.print("Device ID is: ");
  Serial.print(deviceID[0]);
  Serial.println(deviceID[1]);

  // Read in the max Graph Power
  hiByte = EEPROM.read(9);
  loByte = EEPROM.read(10);
  maxGraphPower = (hiByte << 8) + loByte;
  Serial.print("Max Graph Power: ");
  Serial.println(maxGraphPower);

  displayMode = EEPROM.read(11);
  Serial.print("Display Mode is: ");
  Serial.println(displayMode);
  oldDisplayMode = displayMode; // Sets this to stop write to EEPROM

  for (int z = 0; z < 64; z++)
  {
    // Ensure the graphDisplay buffer is clear
    graphDisplay[z] = 0;
  }
  digitalWrite(LED , HIGH);
  delay(1000); // Show this message for a little bit at least
  oldCountdownSeconds = countdownSeconds; // sets the countdown to not display
}

void loop ()
{
  getData();                // Check the serial port for data

  ResetButton.update();     // Check Buttons
  DisplayButton.update();     // Check Buttons

  if (ResetButton.pushed() == true)
  {
    energyWs = 0;  // Reset the energy value
    //Serial.println("Pressed");
  }
  // Could try using ResetButton.tapped()==true  - then can use same button for both functions?

  if (DisplayButton.pushed() == true)
  {
    // Increase the displaytype
    displayMode++;
    if (displayMode >= 3)
    {
      displayMode = 0;
    }
    if (DEBUG_LOCAL == 1)
    {
      Serial.print("DisplayMode: ");
      Serial.println(displayMode);
    }
  }

  if (displayMode != oldDisplayMode)
  {
    Serial.println(F("DisplayMode->EEPROM"));
    EEPROM.write(11, displayMode);
    oldDisplayMode = displayMode;
  }


  //Decide which mode depending up Time or Power data being sent
  switch (DEVICETYPE)
  {
    case 0:
      // ********** POWER MODE *********************************
      // ***********Display data********************************
      // Here we keep a count of the milliseconds
      if ((millis()) >= (oldTime + DISPLAYUPDATEMS))
      {
        digitalWrite(LED, !digitalRead(LED));   // Toogle the LED to show running

        sortGraphBuffer();    // This ensures the correct data is in the graphBuffer

        // Depending on the displayMode this is where the layout of the display is configured:
        switch (displayMode)
        {
          case 0:
            displayPowerSure(1);
            displayEnergySure(0);
            //displayGraphSure(0);
            break;
          case 1:
            displayPowerSure(1);
            //displayEnergySure(1);
            displayGraphSure(0);
            break;
          case 2:
            //displayPowerSure(1);
            displayEnergySure(1);
            displayGraphSure(0);
            break;
          default:
            Serial.println(F("BAD Display Mode"));
            break;
        }
        oldTime = millis();  // Save the time ready for the next output display
      }
      break;

    case 1:
      // *********** TIME MODE *********************************
      // In this mode we have the following commands:
      // aXXCD?--------
      // Where ? = 3,2,1,0 if ? = 0 then we show GO!
      // aXXWN?--------
      // Where ? = 1 or 2 or 1=2 for draw
      // Here we keep a count of the milliseconds
      if ((millis()) >= (oldTime + DISPLAYUPDATEMS))
      {
        digitalWrite(LED, !digitalRead(LED));   // Toogle the LED to show running
        // If countdownSeconds has changed then change the display
        // if not then don't do anything
        //  Write Countdown
        if (countdownSeconds != oldCountdownSeconds && winnerFlag == false)
        {
          //Display countdown
          // Here we can display the values to the SURE displays:
          HT1632.renderTarget(1);
          HT1632.clear(); // This zeroes out the internal memory.
          HT1632.drawText("READY", 3, 0, FONT_8X4, FONT_8X4_END, FONT_8X4_HEIGHT);
          HT1632.render(); // This updates the screen display.
          HT1632.renderTarget(0);
          HT1632.clear(); // This zeroes out the internal memory.
          char buf[2];
          countdownSecondsStr.toCharArray(buf, 2);
          HT1632.drawText(buf, 14, 0, FONT_8X4, FONT_8X4_END, FONT_8X4_HEIGHT);
          HT1632.render(); // This updates the screen display.
          oldCountdownSeconds = countdownSeconds; // reset ready for the next one
          if (countdownSeconds == 0)
          {
            HT1632.renderTarget(1);
            HT1632.clear(); // This zeroes out the internal memory.
            HT1632.drawText("RACE!", 3, 0, FONT_8X4, FONT_8X4_END, FONT_8X4_HEIGHT);
            HT1632.render(); // This updates the screen display.
            HT1632.renderTarget(0);
            HT1632.clear(); // This zeroes out the internal memory.
            HT1632.render(); // This updates the screen display.
          }
        }
        if (winnerFlag == true)
        {
          // Display winner on screen
          // Here we can display the values to the SURE displays:
          HT1632.renderTarget(1);
          HT1632.clear(); // This zeroes out the internal memory.
          HT1632.drawText("WINNER", 0, 0, FONT_8X4, FONT_8X4_END, FONT_8X4_HEIGHT);
          HT1632.render(); // This updates the screen display.
          HT1632.renderTarget(0);
          HT1632.clear(); // This zeroes out the internal memory.
          if (winner == 0)
          {
            HT1632.drawText("DRAW", 7, 0, FONT_8X4, FONT_8X4_END, FONT_8X4_HEIGHT);
          }
          else
          {
            char buf[2];
            winnerStr.toCharArray(buf, 2);
            HT1632.drawText(buf, 14, 0, FONT_8X4, FONT_8X4_END, FONT_8X4_HEIGHT);
          }
          HT1632.render(); // This updates the screen display.
          winnerFlag = false;
        }
        if (displayFlag == true)
        {
          HT1632.renderTarget(1);
          HT1632.clear(); // This zeroes out the internal memory.
          HT1632.drawText(bufDisplay1, 0, 0, FONT_8X4, FONT_8X4_END, FONT_8X4_HEIGHT);
          HT1632.render(); // This updates the screen display.
          HT1632.renderTarget(0);
          HT1632.clear(); // This zeroes out the internal memory.
          HT1632.drawText(bufDisplay2, offset, 0, FONT_8X4, FONT_8X4_END, FONT_8X4_HEIGHT);
          HT1632.render(); // This updates the screen display.
          displayFlag = false;
        }
        oldTime = millis();  // Save the time ready for the next output display
      }
      break;

    default:
      Serial.println("Bad display mode"); // Error
      break;
  }
  delay(20);   // Short pause to stop everything running too quick
}


// This routine pulls the string stored in program memory so we can use it
// It is temporaily stored in the stringBuffer
char* getString(const char* str) {
  strcpy_P(stringBuffer, (char*)str);
  return stringBuffer;
}

void sortGraphBuffer()
{
  // We want to add the powerAve data into the power value buffer.
  // This is 64 bytes wide to cope with both Sure and EA displays

  // We enter this point every DISPLAYUPDATEMS (approx, it might be out by a bit (+/-5mS))
  // Sort out the graphDisplay with the power values
  // First move all the power values along
  // Then fit the new powerAve into the buffer
  if (DISPLAYTYPE == 0)
  {
    displaySize = 32;
  }
  else if (DISPLAYTYPE == 1)
  {
    displaySize = 64;
  }
  for (int z = 1; z < displaySize; z++)
  {
    // Shift all the graph display values along
    graphDisplay[z - 1] = graphDisplay[z];
  }
  graphDisplay[(displaySize - 1)] = powerFloat;

  if (DEBUG_LOCAL == 1)
  {
    // Show the graphBuffer for DEBUG
    Serial.println("Graph Buffer:");
    for (int n = 0; n < displaySize; n++)
    {
      Serial.print(graphDisplay[n]);
      Serial.print(" ");
    }
    Serial.println();
  }
}

void displayPowerEA()
{

  // The EA display will show everything on one screen so does not need adjustment options.

  //        // Output things to the Embedded Adventures display unit (serial)
  //        // More info can be found here: http://www.embeddedadventures.com/datasheets/PLT-1001_hw_v4_doc_v3.pdf
  //
  //        // Here we calculate the offset for the power text so that it stays to one side
  //        // The display is 64 px wide.
  //        // Each number is 10+1(space) px wide
  //        // So if power is 0-9W, Offset is 38
  //        // So if power is 10-99W, Offset is 27
  //        // So if power is 100-999W, Offset is 16
  //        // So if power is 1000-9999W, Offset is 5
  //
  //        if (powerAve >= 0 && powerAve < 10)
  //        {
  //          displayOffset = 38;
  //        }
  //        else if (powerAve >= 10 && powerAve < 20)
  //        {
  //          displayOffset = 29;
  //        }
  //        else if (powerAve >= 20 && powerAve < 100)
  //        {
  //          displayOffset = 27;
  //        }
  //        else if (powerAve >= 100 && powerAve < 200)
  //        {
  //          displayOffset = 18;
  //        }
  //        else if (powerAve >= 200 && powerAve < 1000)
  //        {
  //          displayOffset = 16;
  //        }
  //        else if (powerAve >= 1000 && powerAve < 2000)
  //        {
  //          displayOffset = 7;
  //        }
  //        else if (powerAve >= 2000 && powerAve < 10000)
  //        {
  //          displayOffset = 5;
  //        }
  //
  //        // Clear the LED display (just in case)
  //        mySerial.print(clearLED);
  //        // Then also want to display a scrolling bar graph of power along the bottom
  //        // This takes the values in graphDisplay and creates a line from 0 to value all along the bottom
  //
  //        // To draw a line we need to write the command:
  //        //  "line colour x1 y1 x2 y2" for every data point
  //
  //        // Power values is converted into a graph.
  //        // The power value is also displayed on top of the graph.
  //
  //        for (int y = 0; y < 64; y++)
  //        {
  //          if (graphDisplay[y] > 0)
  //          {
  //            mySerial.print("line 1 ");
  //            mySerial.print(y);
  //            mySerial.print(" 31 ");
  //            //mySerial.print(graphDisplay[y]);
  //            mySerial.print(y);
  //            mySerial.print(' ');
  //            // This is where we draw the bar graph section:
  //            // We have maxGraphPower = 32 LEDs lit.
  //            // So to convert from power reading to LEDs to light we use the calculation:
  //            // (power now / maxGraphPower) *32
  //            int LEDtoLight = (((float)graphDisplay[y] / (float)maxGraphPower) * 32);
  //            mySerial.print(31 - LEDtoLight);
  //            mySerial.print('\r');
  //            mySerial.print('\r');
  //          }
  //        }
  //        // Write the power
  //        mySerial.print("font 5\r");
  //        mySerial.print("text 2 ");
  //        mySerial.print(displayOffset);  // This ensure the numbers are in the middle of the display
  //        mySerial.print(" 16 ");
  //        mySerial.print('"');
  //        // Want to display the power here:
  //        mySerial.print(powerAve);
  //        mySerial.print('w');
  //        mySerial.print('"');
  //        mySerial.print('\r');
  //        delay(2);
  //

  //        //Write the energy
  //        mySerial.print("font 4\r");
  //        mySerial.print("text 2 ");
  //        mySerial.print(displayOffsetWh);  // This ensure the numbers are in the middle of the display
  //        mySerial.print(" 27 ");
  //        mySerial.print('"');
  //        // Want to display the power here:
  //        mySerial.print(energyStr);
  //        mySerial.print("Wh");
  //        mySerial.print('"');
  //        mySerial.print('\r');
  //        delay(2);
  //        // Then we display it all
  //        mySerial.print(paint);
  //
  //
}

void displayGraphSure(int displayNumber)
{
  // Here we want to display the power data on a graph of size: 32 wide by 8 high
  // Here we can display the values to the SURE displays:
  HT1632.renderTarget(displayNumber);
  HT1632.clear(); // This zeroes out the internal memory.

  // Need to take power data in Buffer
  // Number of lights in each section = (powerFloat / maxGraphPower) * 8.0
  int displayByte = 0;

  for (int n = 0; n < displaySize; n++)
  {
    if (graphDisplay[n] < maxGraphPower)
    {
      displayByte = (int)(((graphDisplay[n]) / (maxGraphPower)) * 8.0);
    }
    else
    {
      displayByte = 8;
    }
    for (int y = 0; y < 8; y++)
    {
      // Go through each pixel in y direction
      // if displayByte < y then light that pixel.
      // Otherwise leave if off
      if (y < displayByte)
      {
        HT1632.setPixel(n, (7 - y));
      }
    }
  }
  HT1632.render(); // This updates the screen display.
}

void displayPowerSure(int displayNumber)
{
  // The power data comes in as: powerAveStr
  // Want to process this to show 0.0 to 999.9 depending upon the number
  if (DEBUG_LOCAL == 1)
  {
    Serial.println(powerFloat);
  }

  if (powerFloat >= 0 && powerFloat < 1)
  {
    displayOffset = 6;
    // Convert float to char array. Needs stdio.h
    // dtostrf(float, minimum width, precision, character array);
    dtostrf(powerFloat, 2, 1, powerAveChar);
  }
  else if (powerFloat >= 1 && powerFloat < 2)
  {
    displayOffset = 7;
    // Convert float to char array. Needs stdio.h
    // dtostrf(float, minimum width, precision, character array);
    dtostrf(powerFloat, 2, 1, powerAveChar);
  }
  else if (powerFloat >= 2 && powerFloat < 10)
  {
    displayOffset = 6;
    // Convert float to char array. Needs stdio.h
    // dtostrf(float, minimum width, precision, character array);
    dtostrf(powerFloat, 2, 1, powerAveChar);
  }
  else if (powerFloat >= 10 && powerFloat < 20)
  {
    displayOffset = 2;
    // Convert float to char array. Needs stdio.h
    // dtostrf(float, minimum width, precision, character array);
    dtostrf(powerFloat, 2, 1, powerAveChar);
  }
  else if (powerFloat >= 20 && powerFloat < 100)
  {
    displayOffset = 1;
    // Convert float to char array. Needs stdio.h
    // dtostrf(float, minimum width, precision, character array);
    dtostrf(powerFloat, 2, 1, powerAveChar);
  }
  else if (powerFloat >= 100 && powerFloat < 200)
  {
    displayOffset = 4;
    // Convert float to char array. Needs stdio.h
    // dtostrf(float, minimum width, precision, character array);
    dtostrf(powerFloat, 3, 0, powerAveChar);
  }
  else if (powerFloat >= 200 && powerFloat < 999)
  {
    displayOffset = 3;
    // Convert float to char array. Needs stdio.h
    // dtostrf(float, minimum width, precision, character array);
    dtostrf(powerFloat, 3, 0, powerAveChar);
  }
  else if (powerFloat >= 999 && powerFloat < 10000)
  {

    displayOffset = 3;
    // Convert float to char array. Needs stdio.h
    // dtostrf(float, minimum width, precision, character array);
    powerFloat = 999.0;
    // Convert float to char array. Needs stdio.h
    // dtostrf(float, minimum width, precision, character array);
    dtostrf(powerFloat, 3, 0, powerAveChar);

  }
  // Here we can display the values to the SURE displays:
  HT1632.renderTarget(displayNumber);
  HT1632.clear(); // This zeroes out the internal memory.
  //powerAveStr.toCharArray(powerAveChar, 6);
  HT1632.drawText(powerAveChar, displayOffset, 0, FONT_8X4, FONT_8X4_END, FONT_8X4_HEIGHT);
  HT1632.drawText("W", 22, 0, FONT_8X4, FONT_8X4_END, FONT_8X4_HEIGHT);
  HT1632.render(); // This updates the screen display.
}

void displayEnergySure(int displayNumber)
{
  // Sort out the energy value
  // This starts at zero and calculated by adding on the power multiplied by time segment
  energyWs = energyWs + (((float)powerAve / 10.0) * ((millis() - (float)oldTime)) / 1000.0); // This is in Ws
  energyWh = energyWs / 3600.0;

  // Here we calculate the offset for the  text so that it stays to one side
  // The display is 64 px wide.
  // Each number is 10+1(space) px wide, except for 1 which is 8+1 wide!!!
  // So if energyWh is 0.00 -9.99 , Offset is 16
  // So if energyWh is 9.99 - 99.99, change decimal place Offset is 16
  // So if energyWh is 99.99 - 999.9 , change decimal place Offset is 16
  // So if energyWhis 999.9 to 9999.9, change decimal place Offset is 16
  // ******* TO SORT, IF NEEDED *****************************************

  if (energyWh >= 0 && energyWh < 10)
  {
    displayOffsetWh = 16;
    dtostrf(energyWh, 3, 2, energyStr );
  }
  else if (energyWh >= 10 && energyWh < 20)
  {
    displayOffsetWh = 16;
    dtostrf(energyWh, 3, 1, energyStr );
  }
  else if (energyWh >= 20 && energyWh < 100)
  {
    displayOffsetWh = 16;
    dtostrf(energyWh, 3, 1, energyStr );
  }
  else if (energyWh >= 100 && energyWh < 200)
  {
    displayOffsetWh = 16;
    dtostrf(energyWh, 3, 0, energyStr );
  }
  else if (energyWh >= 200 && energyWh < 1000)
  {
    displayOffsetWh = 16;
    dtostrf(energyWh, 3, 0, energyStr );
  }
  else if (energyWh >= 1000 && energyWh < 2000)
  {
    displayOffsetWh = 16;
    dtostrf(energyWh, 4, 0, energyStr );
  }
  else if (energyWh >= 2000 && energyWh < 10000)
  {
    dtostrf(energyWh, 4, 0, energyStr );
    displayOffsetWh = 16;
  }
  // Here we can display the values to the SURE displays:
  HT1632.renderTarget(displayNumber);
  HT1632.clear(); // This zeroes out the internal memory.
  HT1632.drawText(energyStr, 1, 0, FONT_8X4, FONT_8X4_END, FONT_8X4_HEIGHT);
  HT1632.drawText("Wh", 22, 0, FONT_8X4, FONT_8X4_END, FONT_8X4_HEIGHT);
  HT1632.render(); // This updates the screen display.
}


// **********************GET DATA SUBROUTINE*****************************************
// This sub-routine picks up and serial string sent to the device and sorts out a power string if there is one
// All values are global, hence nothing is sent/returned

void getData()
{
  // **********GET DATA*******************************************
  // We want to find the bit of interesting data in the serial data stream
  // As mentioned above, we are using LLAP for the data.
  // All the data arrives as serial commands via the serial interface.
  // All data is in format aXXDDDDDDDDD where XX is the device ID
  while (Serial.available())
  {
    inByte = (char)Serial.read(); // Read whatever is happening on the serial port
    str_buffer += inByte;
    if ((inByte == '-') || (inByte == '\r') || (inByte == '\n'))
    {
      break;
    }
  }
  if (str_buffer[0] == 'a' && ((str_buffer[str_buffer.length() - 1] == '-') || (str_buffer[str_buffer.length() - 1] == '\r') || (str_buffer[str_buffer.length() - 1] == '\n')))
  {
    Serial.println(str_buffer);  // TEST - print the str_buffer data (if it has arrived)
    sortData();
    str_buffer = ""; // Reset the buffer to be filled again
  }
  else
  {
    str_buffer = ""; // Reset the buffer to be filled again
    //Serial.println("Not OK");
  }
  Serial.flush();
}

// **********************SORT DATA SUBROUTINE*****************************************
// This sub-routine takes the read-in data string (12 char, starting with a) and does what is required with it
// The str-buffer is global so we do not need to send it to the routine
void sortData()
{
  // We first want to check if the device ID matches.
  // If it does not then we disregard the command (as it was not meant for this device
  if (str_buffer.substring(1, 3) == deviceID)
  {
    // If yes then we can do further checks on ths data
    // This is where we do all of the checks on the incomming serial command:

    //Serial.println("ID OK");  // TEST - got into this routine

    // Change device ID:
    // Device ID
    // “aXXCHIDXXE--“
    // Where the last two values (XX) are the new device ID (from AA to ZZ).
    if (str_buffer.substring(3, 7) == "CHID")
    {
      // First check if the NEW device ID is within the allowable range (AA-ZZ)
      // to do this we can convert to an int and check if it is within the OK levels
      // A -> int is 65, Z -. int is 90
      // So our new device ID as an int must be between 65 and 90 for it to be valid
      if (65 <= int(str_buffer[7]) && int(str_buffer[7]) <= 90 && 65 <= int(str_buffer[8]) && int(str_buffer[8]) <= 90)
      { // If is all OK then write the data
        // Change device ID
        Serial.print("CHID: ");
        Serial.println(str_buffer.substring(7, 9)); // This will change the device ID
        deviceID[0] = str_buffer[7];
        deviceID[1] = str_buffer[8];
        // Also want to store this into EEPROM
        EEPROM.write(0, deviceID[0]);    // Do this seperately
        EEPROM.write(1, deviceID[1]);
      }
      else
      {
        Serial.println("Invalid ID");
      }
    }
  }
  //  Power Value
  //  “aXXP????-“
  //  Where the value ???? is the power in Watts
  if (str_buffer.substring(3, 4) == "P")
  {
    // Update the power value if its valid
    powerAveStr = str_buffer.substring(4, 8);

    if (checkStringOK(powerAveStr) == true)
    {
      powerAve = powerAveStr.toInt(); // Need vaue as an int
      powerFloat = powerAve / 10.0;   // Need value as a float
    }
    else
    {
      Serial.println("Invalid P");
    }
  }

  //  Maximum Graph Power
  //  To change this we use the command:
  //  “aXXMGPDDD---“
  //  Where the value DDD is the maximum graph power in watts (0-999W)
  // This corresponds to 8 or 32 red LEDs being lit (depednign upon display).
  else if (str_buffer.substring(3, 6) == "MGP")
  {
    maxGraphPowerStr = str_buffer.substring(6, 10);
    maxGraphPower = maxGraphPowerStr.toInt();
    // DO NOT CHANGE if outside of max/min values or rubbish data
    if (maxGraphPower >= 0 && maxGraphPower <= 3000)
    {
      // Change max Graph Power
      Serial.print("MGP: ");
      Serial.println(maxGraphPower);

      // Also want to store this into EEPOM
      EEPROM.write(9, maxGraphPower >> 8);    // Do this seperately
      EEPROM.write(10, maxGraphPower & 0xff);
    }
    else
    {
      Serial.println("Invalid MGP");
    }
  }
  //  Count Down Mode
  //  To change this we use the command:
  //  “aXXCD?---“
  //  Where ? is the seconds value of count down
  else if (str_buffer.substring(3, 5) == "CD")
  {
    Serial.println(F("Countdown MODE"));
    countdownSecondsStr = str_buffer.substring(5, 6);
    countdownSeconds = countdownSecondsStr.toInt();
    Serial.println(countdownSeconds);
    winnerFlag = false;
  }

  //  Winner Mode
  //  To change this we use the command:
  //  “aXXWN?---“
  //  Where ? is the winner either 1 or 2 with 0 for a draw
  else if (str_buffer.substring(3, 5) == "WN")
  {
    Serial.println(F("Winner"));
    winnerStr = str_buffer.substring(5, 6);
    winner = winnerStr.toInt();
    Serial.println(winnerStr);
    winnerFlag = true; // stop countdown happening
  }

  //  Timer Mode
  //  To change this we use the command:
  //  “aXXTI----“
  //  Where ---- is the time in 10's seconds
  else if (str_buffer.substring(3, 5) == "TI")
  {
    displayFlag = true;
    displayStr1 = "TIME:";
    displayStr2 = str_buffer.substring(5, 8);

    // Here need to sort out the time from ***.* to number 
    timer = displayStr2.toInt();
    displayStr2 = timer;
    displayStr1.toCharArray(bufDisplay1, displayStr1.length() + 1);
    displayStr2.toCharArray(bufDisplay2, displayStr2.length() + 1);
    offset = 10;    
  }

  //  Energy Adjust Value
  //  To change this we use the command:
  //  “aXXSTMaxE|---“
  //  Where ? is the seconds value of count down
  else if (str_buffer.substring(3, 5) == "ST")
  {
    Serial.println(F("Adjust"));
    displayFlag = true;
    // need to find the test up the the | character
    if (str_buffer.substring(5, 9) == "MaxE")
    {
      displayStr1 = "ENERGY";
      displayStr2 = str_buffer.substring(str_buffer.indexOf('|') + 1, str_buffer.indexOf('k'));
      offset = 0;      
      Serial.println(F("Energy"));
    }
    else if (str_buffer.substring(5, 9) == "MaxP")
    {
      displayStr1 = "MAX P";
      displayStr2 = str_buffer.substring(str_buffer.indexOf('|') + 1, str_buffer.indexOf('W'));
      offset = 0;      
      Serial.println(F("max Power"));
    }
    else if (str_buffer.substring(5, 11) == "Race T")
    {
      displayStr1 = "MAX T";
      displayStr2 = str_buffer.substring(str_buffer.indexOf('|') + 1, str_buffer.indexOf('s'));
      offset = 0;
      Serial.println(F("Max Time"));
    }
    else if (str_buffer.substring(5, 11) == "Panels")
    {
      displayStr1 = "PANELS";
      displayStr2 = str_buffer.substring(str_buffer.indexOf('|') + 1, str_buffer.indexOf('|') + 2);
      offset = 0;
      Serial.println(F("Panels"));
      
    }
    // Need to also cope with the following commands:
    //aAASTEnergy Fill
    //aAASTPower Race
    //aAASTEnergy Race
    else if (str_buffer.substring(5, 16) == "Energy Fill")
    {
      displayStr1 = "ENERGY";
      displayStr2 = "FILL";
      offset = 0;
    }
    else if (str_buffer.substring(5, 15) == "Power Race")
    {
      displayStr1 = "POWER";
      displayStr2 = "RACE";
      offset = 0;      
    }
    else if (str_buffer.substring(5, 16) == "Energy Race")
    {
      displayStr1 = "ENERGY";
      displayStr2 = "RACE";
      offset = 0;      
    }

    displayStr1.toCharArray(bufDisplay1, displayStr1.length() + 1);
    displayStr2.toCharArray(bufDisplay2, displayStr2.length() + 1);
    Serial.println(displayStr1);
    Serial.println(displayStr2);
  }
}
