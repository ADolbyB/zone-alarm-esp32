/**
 * Joel Brigida
 * CDA 4630: Embedded Systems
 * Alarm Project v1.3
 * This file combines 2 files:
 * 1) "ZoneAlarm_v12": Has Keypad, Servo, LEDs and sensor inputs.
 * 2) "Test_I2C_3": Uses Real Time Clock and LCD Screen to display the time.
 * This is where the MSP 430 failed: Using the serial port and the Servo together causes 
 * the MSP430 to get stuck in an infinite boot/reboot loop.
 */

//#include <Servo.h>                                    // include servo library
#include <Keypad.h>                                     // include keypad library
#include <LiquidCrystal_I2C.h>
#include <SparkFunDS1307RTC.h>
#include <Wire.h>

//Servo servoObj;                                       // declare servo object
LiquidCrystal_I2C lcd(0x27, 20, 4);                     // declare lcd object: (ADDRESS, #CHARS, #LINES)
char *password = "123A";                                // declare pointer to char array and add password to array
unsigned position = 0;                                  // set the position of password array to 1st index.

const byte ROWS = 4;                                    // set global constant as # of rows to the keypad
const byte COLS = 4;                                    // set global constant as # of columns to the keypad

const char buttons[ROWS][COLS] = {                      // declare keypad button as 2-D array
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {P1_0, P2_6, P2_7, P1_7};          // declare pins of keypad as byte arrays
byte colPins[COLS] = {P1_6, P2_5, P2_4, P2_3};

Keypad the_keypad = Keypad(                             // declare and initialize Keypad Object
    makeKeymap(buttons),
    rowPins,
    colPins,
    ROWS,
    COLS
);

const short blueLED = 4;                                // P1_2;
const short greenLED = 5;                               // P1_3;
const short BUZZER = 8;                                 // P2.0;
const short micSense = A5;                              // P1_5;
const short PIR = 6;                                    // P1_4;

int noise = 0;                                          // value for averaged noise level after 5 samples
int noiseTotal = 0;                                     // total value of 5 noise samples added up
int noiseTrip = 0;                                      // if alarm is set off by noise, this value is stored for debug

short digPIR = 0;                                       // 0 or 1: LOW when sensor is idle, HIGH when motion detected
short setPIR = 0;                                       // Sets and stores 1 when the PIR sensor sets off the alarm
short noiseCtr = 0;                                     // ++ for every noise trip over threshold

char myKeyPress = 0;                                    // Sets a char as current user keypress

short armed = 0;                                        // 0 or 1: 0 = Disarmed state, 1 = Armed state
short alarm = 0;                                        // 0 or 1: 0 = No alarm, 1 = Alarm Tripped

int switches1 = 0;                                      // debug for P1IN binary values
int switches2 = 0;                                      // debug for P2IN binary values

void setLocked(int locked);                             // function declaration for setLocked function
void LCD_Splash();                                      // Welcome screen for LCD

void setup()
{
    // Serial.begin(9600);                              // Serial bus crashes with <Servo.h>
    Wire.begin();                                       // Initialize Pins P2.1 and P2.2 for I2C bus
    rtc.begin();                                        // Initialize Real Time Clock
    lcd.begin();                                        // initialize the lcd

    pinMode(blueLED, OUTPUT);                           // set output for Pin 1.2
    pinMode(greenLED, OUTPUT);                          // set output for Pin 1.3
    pinMode(BUZZER, OUTPUT);                            // set output for Pin 2.0
    digitalWrite(BUZZER, LOW);                          // turn off LED (Active High)
    pinMode(PIR, INPUT);                                // set PIR sensor as input

    LCD_Splash();                                       // Welcome Screen for LCD display
    digitalWrite(blueLED, HIGH);                        // PowerUp test LEDS
    digitalWrite(greenLED, HIGH);                       // PowerUp test LEDS
    delay(2000);
    lcd.clear();
    digitalWrite(blueLED, LOW);
    digitalWrite(greenLED, LOW);

    switches1 = P1IN;                                   // debug: view as binary value
    switches2 = P2IN;                                   // debug: view as binary value

    //servoObj.attach(P1_1);                            // function call to attach servo PWM to pin P1.1
    setLocked(true);                                    // Set servo to lock on initial startup.
    armed = 1;                                          // Set armed status
}

void loop()
{
    static int8_t lastSecond = -1;

    rtc.update();

    if (rtc.second() != lastSecond)                     // If the second has changed
    {
        lcd.setCursor(0, 0);                            // 1st line of display
        lcd.print("=>> Current Time <<=");
        lcd.setCursor(0, 1);
        if (rtc.hour() < 10)                            // BUGFIX: always 2 digits: add leading zero
            lcd.print('0');
        lcd.print(rtc.hour());                          // Print hour
        lcd.print(":");
        if (rtc.minute() < 10)
            lcd.print('0');                             // Print leading '0' for minute
        lcd.print(rtc.minute());                        // Print minute
        lcd.print(":");
        if (rtc.second() < 10)
            lcd.print('0');                             // Print leading '0' for second
        lcd.print(rtc.second());                        // Print second

        if (rtc.is12Hour())                             // If we're in 12-hour mode
        {
            if (rtc.pm())                               // Use rtc.pm() to read the AM/PM state of the hour
                lcd.print(" PM");                       // Returns true if PM
            else
                lcd.print(" AM");
        }
        else
        {
            lcd.print(' ');
            lcd.print(rtc.dayStr());
        }
        
        lcd.print(' ');
        if (rtc.month() < 10)
            lcd.print('0');
        lcd.print(rtc.month());                         // Print month
        lcd.print("/");
        lcd.print(rtc.date());                          // Print date
        lcd.print("/");
        lcd.print(rtc.year());                          // Print year
        lastSecond = rtc.second();                      // Update lastSecond value

        lcd.setCursor(0, 2);
        if (armed == 1)
        {
            lcd.print("*** System Armed ***");
        }
        else
        {
            lcd.print(" **** Disarmed **** ");
        }
        
        lcd.setCursor(0, 3);
        if (alarm == 1)                                 //
        {
            lcd.print("=>> Alarm Trip!! <<=");          // display when alarm is tripped
        }
        else
        {
            lcd.print("                    ");          // clear line when Alarm is reset
        }
    }

    myKeyPress = the_keypad.getKey();                   // retrieve key pressed by user

    if (armed)
    {
        digPIR = digitalRead(PIR);                      // read value for PIR sensor

        if (digPIR)                                     // if digPIR == 1 then there is motion
        {
            alarm = 1;                                  // set alarm condition
            setPIR = 1;                                 // store PIR trip
        }

        for (int i = 0; i < 5; i++)
        {
            noiseTotal += analogRead(micSense);         // add up 5 total readings
            delay(20);                                  // delay 20 mSec
        }

        noise = (noiseTotal / 5);                       // take average reading

        if (noise > 250)                                // evaluate average reading
        {
            alarm = 1;                                  // set off the alarm
            noiseTrip = noise;                          // store value which tripped alarm
            noiseCtr++;
        }
        noise = 0;                                      // reset values so they don't overflow
        noiseTotal = 0;

        if (myKeyPress)
        {
            Serial.println(myKeyPress);
        }

        if (myKeyPress == password[position])
        {
            position++;                                 // if the digit matches: increase counter & move to next array index
        }

        if (position == 4)                              // if 4 digits were entered correctly
        {
            for (int i = 0; i < 3; i++)
            {
                digitalWrite(BUZZER, HIGH);
                delay(100);                             // length of beep = 100 mS
                digitalWrite(BUZZER, LOW);
                delay(200);                             // delay between beeps = 200 mS
            }
            setLocked(false);                           // OPEN Sesame...

            armed = 0;                                  // 0 = disarmed
            alarm = 0;                                  // turn off alarm
            setPIR = 0;                                 // reset PIR activation
            noiseCtr = 0;                               // reset noise counter
            noiseTrip = 0;                              // reset last noise alarm
            position = 0;                               // reset counter back to ZERO
        }
    }

    if (alarm == 1)
    {
        digitalWrite(BUZZER, HIGH);                     // turn on LED or BUZZER (Active High)
    }

    if (myKeyPress == '*')                              // clear password / relock with a '*' press
    {
        if (!armed)                                     // only beep when locking from unlocked state
        {
            digitalWrite(BUZZER, HIGH);
            delay(100);                                 // length of beep = 100 mS
            digitalWrite(BUZZER, LOW);
            delay(200);                                 // delay between beeps = 200 mS
            setLocked(true);                            // move servo to 0 = LOCKED
            armed = 1;                                  // 1 = armed
        }
        position = 0;                                   // reset password counter
    }
}

void setLocked(int locked)                              // function declaration for setLocked function
{
    if (locked)                                         // if door is locked / closed
    {
        digitalWrite(blueLED, HIGH);                    // turn ON Red LED on Pin 1.2
        digitalWrite(greenLED, LOW);                    // turn OFF Green LED on Pin 1.3
        //servoObj.write(360);                          // unlock servo: move to position OPEN = 360
        delay(600);
    }
    else // if (!locked)
    {
        digitalWrite(blueLED, LOW);                     // turn OFF Red LED on Pin 1.2
        digitalWrite(greenLED, HIGH);                   // turn ON Red LED on Pin 1.3
        //servoObj.write(0);                            // lock servo: move to position LOCKED = 0
        delay(600);
    }
}

void LCD_Splash()                                       // Power Up Splash Screen
{
    lcd.backlight();
    lcd.setCursor(0, 0);                                // setCursor(Character #, Line #)
    lcd.print("    Joel Brigida    ");
    lcd.setCursor(0, 1);
    lcd.print("   MSP430 Security  ");
    lcd.setCursor(0, 2);
    lcd.print("      CDA 4630      ");
    lcd.setCursor(0, 3);
    lcd.print("  Embedded Systems  ");
}