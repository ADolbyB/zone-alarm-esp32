/**
 * Joel Brigida
 * CDA-4630: Embedded Systems.
 * This is v2.0 of the Zone Alarm Project. At the time of writing this
 * code, I did not know how to use GitHub yet, so I kept separate files
 * as I iteratively developed. This is the 1st "version" using the ESP32
 * instead of the MSP430.
 * This version only runs on one CPU Core
*/

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SparkFunDS1307RTC.h>
#include <Keypad.h>
#include <FastLED.h>
#include <ESP32Servo.h>

#define LED_PIN     2                           // Pin 2 on Thing Plus C is connected to WS2812 LED
#define COLOR_ORDER GRB                         // RGB LED in top right corner
#define CHIPSET     WS2812
#define NUM_LEDS    1
#define BRIGHTNESS  25

const byte ROWS = 4;                            // set global constant as # of rows to the keypad
const byte COLS = 4;                            // set global constant as # of columns to the keypad

const char buttons[ROWS][COLS] = {              // declare keypad button as 2-D array
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};  

byte colPins[COLS] = {33, 15, 32, 14};          // declare pins of keypad as byte arrays
byte rowPins[ROWS] = {26, 25, 12, 27};

Servo servoObj;                                 // declare servo object
LiquidCrystal_I2C lcd(0x27, 20, 4);             // declare LCD object: (ADDRESS, #CHARS, #LINES)

Keypad userKeypad = Keypad(                     // Declare keypad object
    makeKeymap(buttons),
    rowPins,
    colPins,
    ROWS,
    COLS
);

CRGB leds[NUM_LEDS];                            // Array for LEDS on GPIO_2

char *password = "123A";                        // declare pointer to char array and add password to array
unsigned position = 0;                          // set the position of password array to 1st index.

const short BUZZER = 8;                         // Alarm Buzzer on pin 8
const short micSense = A2;                      // Pin for Noise Sensor: A2 = GPIO_34
const short PIR = 39;                           // Pin for PIR detection = GPIO_39;

int noise = 0;                                  // value for averaged noise level after 5 samples
int noiseTotal = 0;                             // total value of 5 noise samples added up
int noiseTrip = 0;                              // if alarm is set off by noise, this value is stored for debug

short digPIR = 0;                               // 0 or 1: LOW when sensor is idle, HIGH when motion detected
short setPIR = 0;                               // Sets and stores 1 when the PIR sensor sets off the alarm
short noiseCount = 0;                           // ++ for every noise trip over threshold

char myKeyPress = 0;                            // Sets a char as current user keypress

short armed = 0;                                // 0 or 1: 0 = Disarmed state, 1 = Armed state
int alarm = 0;                                  // 0 or 1: 0 = No alarm, 1 = Alarm Tripped

int count = 0;
int count1 = 0;
int count2 = 0;
int count3 = 0;
int color = 0;

void updateTime()
{
    static int8_t lastSecond = -1;
    
    rtc.update();                               // update all rtc.seconds(), rtc.minutes(), etc. & return
    if (rtc.second() != lastSecond)             // If the second has changed
    {
        lastSecond = rtc.second();              // Update lastSecond value
    }
}

void setLocked(int locked)                      // Changes servo position & Status LED
{
    if (locked)                                 // if door is locked / closed
    {
        leds[0] = CRGB::Green;                  // Green = Go / Open
        FastLED.show();
        servoObj.write(360);                    // unlock servo: move to position OPEN = 360
    }
    else // if (!locked)
    {
        leds[0] = CRGB::Blue;                   // Blue = secure
        FastLED.show();
        servoObj.write(0);                      // lock servo: move to position LOCKED = 0
    }
}

void printTimeScrn()                            // Print time to LCD
{
    lcd.setCursor(0, 0);                        // 1st line of display
    //lcd.print("=>> Current Time <<=");
    //lcd.setCursor(0, 1);
    //lcd.print(rtc.dayStr());                  // Print day string
    //lcd.setCursor(0, 2);                      // 2nd line of display
    if (rtc.hour() < 10)                        // BUGFIX: always 2 digits: add leading zero
        lcd.print('0');
    lcd.print(rtc.hour());                      // Print hour
    lcd.print(":");
    if (rtc.minute() < 10)
        lcd.print('0');                         // Print leading '0' for minute
    lcd.print(rtc.minute());                    // Print minute
    lcd.print(":");
    if (rtc.second() < 10)
        lcd.print('0');                         // Print leading '0' for second
    lcd.print(String(rtc.second()));            // Print second

    if (rtc.is12Hour())                         // If we're in 12-hour mode
    {
        if (rtc.pm())                           // Use rtc.pm() to read the AM/PM state
            lcd.print(" PM");                   // Returns true if PM
        else 
            lcd.print(" AM");
    }
    else // 24-hr mode
    {
        lcd.print(' ');                         
        lcd.print(rtc.dayStr());
    }
  
    //lcd.print(" | ");
    //lcd.setCursor(0, 3);

    // Options for printing the day, pick one:
    //Serial.print(rtc.dayStr());               // Print day string
    //Serial.print(rtc.dayC());                 // Print day character
    //Serial.print(rtc.day());                  // Print day integer (1-7, Sun-Sat)
    //Serial.print(" - ");
    
    lcd.print(' ');
    if (rtc.month() < 10)
        lcd.print('0');
    lcd.print(String(rtc.month()) + "/" +       // Print month
        String(rtc.date()) + "/");              // Print date
    //lcd.print("20");                          // print 1st half of year
    lcd.print(String(rtc.year()));              // BUGFIX: Print year: DO NOT USE "println" (artifacts shown))
  
    /** LCD Test Code:
    lcd.setCursor(0, 1);
    lcd.print("Sensor 1:           ");
    lcd.setCursor(11, 1);
    lcd.print(count1++);                        // print 1st value
    if (count1 > 254)
        count1 = 0;
    lcd.setCursor(0, 2);
    lcd.print("Sensor 2:           ");
    lcd.setCursor(11, 2);
    lcd.print(count2);
    count2 += 2;
    if (count2 > 254)
        count2 = 0;
    lcd.setCursor(0, 3);
    lcd.print("Sensor 3:           ");
    lcd.setCursor(11, 3);
    lcd.print(count3);
    count3 += 3;
    if (count3 > 254)
        count3 = 0;
    */
}

void printTime()                                // Print time to Serial Terminal
{
    Serial.print(String(rtc.hour()) + ":");     // Print hour
    if (rtc.minute() < 10)
        Serial.print('0');                      // Print leading '0' for minute
    Serial.print(String(rtc.minute()) + ":");   // Print minute
    if (rtc.second() < 10)
        Serial.print('0');                      // Print leading '0' for second
    Serial.print(String(rtc.second()));         // Print second

    if (rtc.is12Hour())                         // If we're in 12-hour mode
    {
        if (rtc.pm())                           // Use rtc.pm() to read the AM/PM state of the hour
            Serial.print(" PM");                // Returns true if PM
        else 
            Serial.print(" AM");
    }
  
    Serial.print(" | ");

    // Options for printing the day, pick one:
    Serial.print(rtc.dayStr());                 // Print day string
    //Serial.print(rtc.dayC());                 // Print day character
    //Serial.print(rtc.day());                  // Print day integer (1-7, Sun-Sat)
    Serial.print(" - ");
    Serial.print(String(rtc.month()) + "/" +    // Print month
        String(rtc.date()) + "/");              // Print date
    Serial.println(String(rtc.year()));         // Print year
}

void I2C_20x4LCD_set()                          // 2 sec Splash Screen
{
    leds[0] = CRGB::White;                      // Power up Pin 2 LEDs
    FastLED.show();

    lcd.init();                                 // initialize the lcd 
    lcd.backlight();                            // activate backlight
    lcd.setCursor(0, 0);                        // setCursor(Character #, Line #)
    lcd.print("      CDA-4630      ");
    lcd.setCursor(0, 1);
    lcd.print("   ESP32 Security   ");
    lcd.setCursor(0, 2);
    lcd.print("    Joel Brigida    ");
    lcd.setCursor(0, 3);
    lcd.print("  Embedded Systems  ");
      
    delay(2000);
      
    lcd.clear();
    leds[0] = CRGB::Black;
    FastLED.show();
}

void setup() 
{
    delay(30);                                  // sanity delay
    FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness( BRIGHTNESS );

    Serial.begin(115200);
    Wire.begin(21, 22, 4000);                   // SDA, SCL, DATA speed: Note: QWIIC Connector Pins
    rtc.begin();                                // Call rtc.begin() to initialize the RTC
    delay(1000);
    
    //Set Time: e.g. 7:32:16 | Monday October 31, 2016:
    //rtc.setTime(16, 32, 7, 2, 31, 10, 16);    // Uncomment to manually set time
    //rtc.setTime(00, 20, 22, 5, 11, 11, 21);
    //I2C_16x2LCD_set();                        // Function Call to setup 16 x 2 LCD
    //rtc.set24Hour(true);
    //rtc.writeSQW(0);                          // Turn off square wave output

    I2C_20x4LCD_set();                          // intialize and Splash Screen (2 sec)
    
    //pinMode(BUZZER, OUTPUT);                  // set output for Pin 2.0
    //digitalWrite(BUZZER, LOW);                // turn off LED (Active High)
    //pinMode(PIR, INPUT);                      // set PIR sensor as input

}

void loop() 
{
    updateTime();
    printTimeScrn();                            // Print new time to LCD screen
    printTime();                                // Print new time to Serial Terminal

    myKeyPress = userKeypad.getKey();           // retrieve key pressed by user

    if (myKeyPress)
    {
        Serial.println(myKeyPress);             // debug
    }

    if (myKeyPress == password[position])
    {
        position++;                             // if the digit matches: increase counter & move to next array index
    }

    if (position == 4)                          // if 4 digits on keypad were entered correctly
    {
        for (int i = 0; i < 3; i++)
        {
            digitalWrite(BUZZER, HIGH);
            delay(100);                         // length of beep = 100 mS
            digitalWrite(BUZZER, LOW);
            delay(200);                         // delay between beeps = 200 mS
        }
        setLocked(false);                       // OPEN Sesame...
        armed = false;
        position = 0;                           // reset counter back to ZERO
    }

    if (myKeyPress == '*')                      // relocked anytime or clear with a '*' press
    {
        if (!armed)                             // only beep when locking from unlocked state
        {
            digitalWrite(BUZZER, HIGH);
            delay(100);                         // length of beep = 100 mS
            digitalWrite(BUZZER, LOW);
            delay(200);
        }
        position = 0;                           // reset password counter
        setLocked(true);                        // move servo to 0 = LOCKED
        armed = true;
    }

    lcd.setCursor(0, 1);                        // 2nd Line of display for Alarm Status
    if (armed == 1)
    {
        lcd.print("*** System Armed ***");
    }
    else
    {
        lcd.print(" **** Disarmed **** ");
    }

    lcd.setCursor(0, 2);                        // 3rd line of display for alarm trip
    if (alarm == 1)                             // Alarm has been tripped
    {
        lcd.print("=>> Alarm Trip!! <<=");      // display when alarm is tripped
    }
    else
    {
        lcd.print("                    ");      // clear line when Alarm is reset
    }

    if (armed)                                  // Alarm Input Detection when armed
    {
        digPIR = digitalRead(PIR);              // read value for PIR sensor
        if (digPIR)                             // if digPIR == 1 then there is motion
        {
            alarm = 1;                          // set alarm condition
            setPIR = 1;                         // store PIR trip
        }

        for (int i = 0; i < 5; i++)
        {
            noiseTotal += analogRead(micSense); // add up 5 total readings
            delay(20);                          // delay 20 mSec
        }
        noise = (noiseTotal / 5);               // take average reading

        if (noise > 250)                        // evaluate average reading
        {
            alarm = 1;                          // set off the alarm
            noiseTrip = noise;                  // store value which tripped alarm
            noiseCount++;                       // increment # of noise trips
        }
        noise = 0;                              // reset values so they don't overflow
        noiseTotal = 0;

        if (myKeyPress == password[position])   // keypad entry
        {
            position++;                         // if the digit matches: increase counter & move to next array index
        }
        
        if (position == 4)                      // if 4 digits were entered correctly
        {
            for (int i = 0; i < 3; i++)
            {
                digitalWrite(BUZZER, HIGH);
                delay(100);                     // length of beep = 100 mS
                digitalWrite(BUZZER, LOW);
                delay(200);                     // delay between beeps = 200 mS
            }
            setLocked(false);                   // OPEN Sesame...then CLEAR all values after disarm
            armed = 0;                          // 0 = disarmed
            alarm = 0;                          // turn off alarm
            setPIR = 0;                         // reset PIR activation
            noiseCount = 0;                     // reset noise counter
            noiseTrip = 0;                      // reset last noise alarm
            position = 0;                       // reset counter back to ZERO
        }
    }
}