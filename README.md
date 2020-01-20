# Power_Meter_EA_SURE
Code for a pedal power display using either an Embedded Adventures LED display or a Sure LED display.

This is based upon other code written for the Embedded Adventures display
https://github.com/bespokegear/Power_Meter_Display_EA_LED_Matrix

The SURE LED matrix code has been taken from here:
https://github.com/re-innovation/HT1632C_DEDP131112

This is a power and energy display program for serial-streamed data from our pedal power meters.
It reads serial data (on D0/D1, Tx/Rx). It parses that data and displays the power on an LED display.
  
This code is for an Arduino Nano
  
It is designed to work with Embedded Adventures Dsiplays (with appropriate driver)
OR it can use an HT1632C module - usually 2 pcs of 32 x 08 LED matrixes.

#SURE LED display information

Code for Sure 8 x 32 LED matrix HT1632 controller board 12V Type: DE-DP131112V200

These have a CS and a CS CLOCK to control up to 8 boards.
The HT1632 code was obtained from here:

https://github.com/gauravmm/HT1632-for-Arduino

Additional notes from:

http://store3.sure-electronics.com/de-dp13119

Using this info

http://cdn2.boxtec.ch/pub/sure/LED_Dot_Matrix-Display_Users-Guide.pdf

Need to also clock through the CS
Need to clock through 8 digital HIGH to get the correct CS as low to all the boards
In future this will need clocking in correctly
This has been added to this HT1632C arduino library included in this repository, but probably not in a very nice way!

https://github.com/re-innovation/HT1632C_DEDP131112
  
#Embedded Adventures LED display Information

Code for the Embedded Adventures LED display unit with information here:

https://www.embeddedadventures.com/datasheets/PLT-1001_hw_v4_doc_v3.pdf

#Arduno Nano Pins

*D0: Tx Serial Data (NOT used)
*D1: Rx Serial Data - reads information from external unit
*D2: Display Mode button (not usually added - done in set-up)
*D3: Button A (reset for User input)
*D4: CS pin - SURE LED board
*D5: Do NOT use!
*D6:
*D7: WR - SURE LED board
*D8: DATA  - SURE LED board
*D9: LED - Blink!
*D10: CS_CLK - SURE LED board
*D11:
*D12: Tx EA Controller
*D13: Rx EA Controller

#Functions

This unit takes values of Time and Power and displays them on a large LED matrix
Time Mode:
The data is sent "aXXT0001-----" where XX is aa reference (does not matter which)
The time is a three digit value of 100's milli-seconds (eg 0.1, 1.2, 10.5 etc)
Want the display to take serial data and update the LED display

Power data is read on the serial port at the serial port baud rate

There are a number of functions which can be adjusted via software commands. The main protocol is based upon LLAP (Lightweight Local Automation Protocol), with some additional features.
http://openmicros.org/index.php/articles/85-llap-lightweight-local-automation-protocol/101-llap-starter
This uses a 12 character message in the format:
“aXXDDDDDDDDD”
Where a is the start signifier and XX is the device ID, from AA to ZZ.
Everything else is in upper case.

Commands this unit can process:
Change device ID
“aXXCHID??---“
Where the XX is the old ID and ?? is the new ID. This is stored to EEPROM

Power
“aXXPDDDDD---“
This is the power output (0000.0W to 9999.9W). This will be an output approximately every second.

Voltage and Current
“aXXVDDDIDDDD“
This is the voltage and current output (V is 00.0 to 99.9V DC) (I is 00.00A to 99.99A). This will be an output approximately every second.

Maximum graph power
“aXXMGP????--“
This is for adjusting the display bar graph height to show the max power displayed:
Max power value = 32 LEDs lit up in a column. Where ???? is the power in watts.
This is stored to EEPROM. Device ID does not matter
