/**
 * Joel Brigida
 * CDA-4630 Embedded Systems
 * This is a test program I used on the MSP430 to test the keypad functionality with
 * a servo motor. The keypad is used to enter a password, and if locked and 
 * the password is correct, the servo motor moves to the locked state.
*/

//#include <LiquidCrystal_I2C.h>
#include <Servo.h>                              // include servo library
#include <Keypad.h>

Servo servo_obj;                                // declare servo object
char *password = "1234";                        // declare pointer to char array and add password to array
unsigned position = 0;                          // set the position of password array to 1st index.

const byte ROWS = 4;                            // set global constant as # of rows to the keypad
const byte COLS = 4;                            // set global constant as # of columns to the keypad

const char buttons[ROWS][COLS] = {              // declare keypad button as 2-D array
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {P1_0, P2_6, P2_7, P1_7};  // declare pins of keypad as byte arrays
byte colPins[COLS] = {P1_6, P2_5, P2_4, P2_3};

// declare and initialize Keypad Object
Keypad the_keypad = Keypad(makeKeymap(buttons), rowPins, colPins, ROWS, COLS);

const int blueLED = P1_2;
const int greenLED = P1_3;
const int buzzer = P2_0;

//bool armed = true;                            // powers up in armed state???

void setup()
{    
    pinMode(blueLED, OUTPUT);                   // set output for Pin 1.2
    pinMode(greenLED, OUTPUT);                  // set output for Pin 1.3
    pinMode(buzzer, OUTPUT);                    // set output for Pin 2.0
    digitalWrite(buzzer, LOW);                  // turn off buzzer

    servo_obj.attach(P1_1);                     // function call to attach servo PWM to pin P1.1
    delay(100);
    setLocked(true);                            // Set locked status to true on initial startup.
}

void loop()
{
    char myKeyPress = the_keypad.getKey();      // retrieve key pressed by user

    if (myKeyPress == password[position])
    {
        position++;                             // if the digit matches: increase counter & move to next array index
    }

    if (position == 4)                          // if 4 digits were entered correctly
    {
        for (int i = 0; i < 3; i++)
        {
            digitalWrite(buzzer, HIGH);
            delay(100);                         // length of beep = 100 mS
            digitalWrite(buzzer, LOW);
            delay(200);                         // delay between beeps = 200 mS
        }
        setLocked(false);                       // OPEN Sesame...
    }

    if (myKeyPress == '*')
    {
        // If the lock is open it can be relocked anytime with a '*' press
        if (position != 0)
        {
            digitalWrite(buzzer, HIGH);
            delay(100);                         // length of beep = 100 mS
            digitalWrite(buzzer, LOW);
            delay(200);
        }
        position = 0;                           // reset password counter
        setLocked(true);                        // move servo to 0 = LOCKED
    }
    delay(100);
}

void setLocked(int locked)                      // function declaration for setLocked function
{
    if (locked)                                 // if door is locked / closed
    {
        digitalWrite(blueLED, HIGH);            // turn ON Red LED on Pin 1.2
        digitalWrite(greenLED, LOW);            // turn OFF Green LED on Pin 1.3
        servo_obj.write(360);                   // unlock servo: move to position OPEN = 360
        //armed = false;
    }
    else // if (!locked)
    {
        digitalWrite(blueLED, LOW);             // turn OFF Red LED on Pin 1.2
        digitalWrite(greenLED, HIGH);           // turn ON Red LED on Pin 1.3
        servo_obj.write(0);                     // lock servo: move to position LOCKED = 0
        //armed = true;
    }
}