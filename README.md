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

# SURE LED display information

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
  
# Embedded Adventures LED display Information

Code for the Embedded Adventures LED display unit with information here:

https://www.embeddedadventures.com/datasheets/PLT-1001_hw_v4_doc_v3.pdf

# Arduino Nano Pins

* D0: Tx Serial Data (NOT used)
* D1: Rx Serial Data - reads information from external unit
* D2: Display Mode button (not usually added - done in set-up)
* D3: Button A (reset for User input)
* D4: CS pin - SURE LED board
* D5: Do NOT use!
* D6:
* D7: WR - SURE LED board
* D8: DATA  - SURE LED board
* D9: LED - Blink!
* D10: CS_CLK - SURE LED board
* D11:
* D12: Tx EA Controller
* D13: Rx EA Controller

# Functions

The display unit responds to textual commands sent by serial on the serial wire (green and yellow insulation). The serial interface is configured to run at 115200 baud. The device will ignore anything which is not a valid command, as described below.

Display devices have an two-character ID, which can be set so that the device only responds to commands inteneded for that unit. Multiple devices can be connected to the serial bus, and commands directed to a specific device by using this ID.  By default, display units are configured with the wildcard device ID "**", which means it will respond to commands with any ID.  Other valid IDs are any combination of two upper-case alpha characters, i.e. "AA", "AB", ... "ZZ".

Command Syntax
--------------

Commands are ASCII strings with the following structure:

1. The character "a"
2. The two-character destination ID for this command, e.g. "AA"
3. A one or two-character command ID, e.g. "P" or "CD". See "Commands" below for a full list
4. Zero or more bytes which act as a parameter to the specified command

A command is processed as soon as it is complete. Completeness is determined by whichever of these conditions is true first:

* The command parsing buffer is filled
* An ASCII line feed ("\\n") or carriage return ("\\r") is received
* The selected command's expected parameter length has been received. e.g. the "CD" command expects a single byte parameter, once this is read, the command will be triggered.

Command Examples
----------------

.. code::

   aAACD2

* Command intended for display unit with ID == "AA" (or wildcard)
* Command is "CD" - Countdown
* Command parameter is "2"

.. code::

   aZATEWell if this isn't nice I don't know what is

* Command intended for display unit with ID == "ZA" (or wildcard)
* Command is "TE" - Text
* Command parameter is "Well if this isn't nice, I don't know what is"

Commands
--------

Countdown
^^^^^^^^^

* Command ID: "CD"
* Parameter: single character, value "0" to "9"
* Example: "aAACD2"

Animates a growing rectangle on the display. When the rectangle fills the display (after a little less than a second), the digit passed in the parameter is displayed in the center of the display. If that digit is "0", "GO!" is displayed instead of "0".

The color of the rectangle is green for even numbered parameters, and red for odd numbered parameters.

The color of the text is red for even numbered parameters, and green for odd numberered parameters.

Clear
^^^^^

* Command ID: "CL"
* Parameter: None
* Example: "aAACL"

Clears the display.

Dump Configuration
^^^^^^^^^^^^^^^^^^

* Command ID: "DC"
* Parameter: None
* Example: "aAADC"

Dumps the configuration of the unit onto serial, e.g.

.. code::

    Display ID is: **
    Max Graph Power: 400

Set Maximum Power
^^^^^^^^^^^^^^^^^

* Command ID: "MP"
* Parameter: Four digit maximum power in Watts, e.g. "1000"
* Example: "aAAMP1000"

Sets the maximum power value for the graph in power mode. The new value is stored in EEPROM to that it persists after the device is powered off.

Power
^^^^^

* Command ID: "P"
* Parameter: Five-digit number which is a value in tenths of a Watt, e.g. "12345", meaning 1234.5 Watts
* Example: "aAAP12345"

Displays the numeric value passed after conversion into watts (the non-whole part of the converted value is truncated).

Also displays a graph in the background which scrolls along from right to left, showing a history of the power values which have been displayed in the last minute or so. The graph's maximum value is set using the Set Maximum Power command, and stored in non-volatile EEPROM.  If the Power command is called with a value that exceeds the maximum value, the graph will simply be full height.

Set ID
^^^^^^

* Command ID: "ID"
* Parameter: Two character ID. Each character can be an upper case ASCII letter (A-Z), or an asterisk.  e.g. "AD" or "**"
* Example: "aAAIDBB"

When received, this command tells the display unit to change it's ID.  The new ID is stored in EEPROM to that it persists after the device is powered off.

String
^^^^^^

* Command ID: "ST"
* Parameter: An ASCII string, terminated by a line feed or carriage return character
* Example: "aAASTHello World"

This command will display the specified string in font 2 on the display. The string will be left-justified and in orange. This command is now deprecated - the TE command should be used in future projects. It is supported to maintain compatibility with legacy projects.

Text
^^^^

* Command ID: "TE"
* Parameter: An ASII string, terminated by a line feed or carriage return characterone
* Example: "aAATEThis text will be word-wrapped"

The Text command displays text on the display according to state set by the Text Control command below.  By default text is displayed in font 2, left justified and in orange, but these settings an be modified with the Text Control command.

Word-wrapping is automatically performed, and up to five lines of text are supported when the smallest font is used.

Text Control
^^^^^^^^^^^^

* Command ID: "TC"
* Parameter: A two-character config command
* Example: "aAATCF4"

This command is used to control state which is used by the Text command. Three values may be modified:

1. Font size, by using "F1" to "F7"
2. Justification, by using "JL" (left), "JR" (right) and "JC" (center)
3. Color, by using "C1" to "C3"

Timer
^^^^^

* Command ID: "TI"
* Parameter: four digit value in tenths of a second, e.g. "0123" to mean 12.3 seconds
* Example: "aAATI0123"

This command will display a numeric value in a large font in the center of the screen in red. This is used for countdown timers.

Voltage and Current
^^^^^^^^^^^^^^^^^^^

* Command ID: "V"
* Parameter: Three digit voltage value in tenths of a volt, followed by "I" followed by a four digit current value in 100's of milliamps, e.g. "123I4567", which means 12.3 Volts, 45.67 Amps.
* Example: "aAAV123I4567"

This command will display two rows of data:

.. code::

   xx.x volts
   yy.y amps

With values extracted from the parameter as follows:

* The first three digits of the parameter are volts in tenths of a volt, i.e. "145" means 14.5 volts
* The fourth character should be "I"
* The fifth to eighth characters are current in 100s of millivolts, i.e. "0530" means 5.3 amps

Winner
^^^^^^

* Command ID: "WN"
* Parameter: One digit being the player/team number which has won. Use "0" for a tie
* Example: "aAAWN0"

This command displays:

.. code::

   Winner:
      P

...where P is the number of the player who has won a challenge, or "TIE!" if scores are tied.

Building The Firmware
=====================

Pre-requisites
--------------

* Install the Arduino IDE 1.6.6 or later
* Windows & Mac users may find a download link here: https://www.arduino.cc/en/Main/Software
* Major Linux distros carry it in their software repositories, or the link above may be used
* Install **Mutila** library: https://github.com/matthewg42/Mutila
