/**
 * Joel Brigida
 * CDA-4630 Embedded Systems
 * MSP-430 / Energia IDE ZoneAlarm v0.9
 * This is a test program I used to verify user keypad input to the
 * MCU using the Energia IDE. The keypad is scanned continuously and 
 * any key press is printed to the terminal.
*/

#include <Key.h>
#include <Keypad.h>
#include <driverlib.h>

const byte ROWS = 4;
const byte COLS = 4;

char buttons[ROWS][COLS] = {                            // declare keypad button as 2-D array
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {P1_0, P2_6, P2_7, P1_7};          // declare pins of keypad as byte arrays
byte colPins[COLS] = {P1_6, P2_5, P2_4, P2_3};

// declare and initialize Keypad Object
Keypad the_keypad = Keypad(makeKeymap(buttons), rowPins, colPins, ROWS, COLS);

void setup()
{
    Serial.begin(4800);
    delay(1000);
    Serial.println("Keypad Test Begin...");
}

void loop()
{
    char myKeyPress = the_keypad.getKey();

    if (myKeyPress)
    {
        Serial.println(myKeyPress);
    }
}