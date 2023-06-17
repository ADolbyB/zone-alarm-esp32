/**
 * Joel Brigida
 * CDA-4630: Embedded Systems.
 * This is v2.1 of the Zone Alarm Project.
 * This version only runs on one CPU Core
*/

#include <Arduino.h>
#include <Wire.h>
#include <SparkFunDS1307RTC.h>
#include <Keypad.h>
#include <FastLED.h>
#include <ESP32Servo.h>
#include <LiquidCrystal_I2C.h>

#define LED_PIN 2                   				    // Pin 2 on Thing Plus C is connected to WS2812 LED
#define COLOR_ORDER GRB                 			    // RGB LED in top right corner
#define CHIPSET WS2812
#define NUM_LEDS 1
#define BRIGHTNESS 65

const short servoPin = 4;                    	        // Servo set up on GPIO_4
const short PIR = 39;                   	            // Digital Input for PIR sensor: GPIO_39
const short micSense = 34;						        // audio sensor = GPIO_34 (A2)
CRGB leds[NUM_LEDS];                    			    // Array for LEDS on GPIO_2

int pos = 0;                                            // variable to store the servo position
char myKeyPress = 0;                                    // Sets a char as current user keypress
int digPIR = 0;                                         // 1: motion detected, 0: no motion
const byte ROWS = 4;                                    // set global constant as # of rows to the keypad
const byte COLS = 4;                                    // set global constant as # of columns to the keypad
char *password = "789C";                			    // declare pointer to char array and add password to array
unsigned short position = 0;						    // number of correct keys entered
short armed = 0;
int theAlarm = 0;
unsigned noise = 0;
unsigned noiseTotal = 0;

byte colPins[COLS] = {33, 15, 32, 14};                  // declare pins of keypad as byte arrays
byte rowPins[ROWS] = {26, 25, 12, 27};

const char buttons[ROWS][COLS] = {					    // declare keypad button as 2-D array
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

LiquidCrystal_I2C lcd(0x27, 20, 4);                     // declare lcd object: (ADDRESS, #CHARS, #LINES)
Servo servoObj;  									    // create servo object to control a servo
Keypad userKeypad = Keypad(                             // Declare Keypad object
    makeKeymap(buttons),
    rowPins,
    colPins,
    ROWS,
    COLS
);

void setLocked(int locked)                              // function declaration for setLocked function
{
	if (!locked)                               	        // if door is locked / closed
    {
        leds[0] = CRGB::Green;                          // Green = Go / Open
        FastLED.show();
        servoObj.write(180);                            // unlock servo: move to position OPEN = 360
    }
    else // if (locked)
    {
        leds[0] = CRGB::Blue;                           // Blue = secure
        FastLED.show();
        servoObj.write(0);                              // lock servo: move to position LOCKED = 0
    }
}

void printTimeScrn()                                    // print time to LCD
{
    lcd.setCursor(0, 0);                                // 1st line of display
    //lcd.print("=>> Current Time <<=");
    //lcd.setCursor(0, 1);
    //lcd.setCursor(0, 2);                              // 2nd line of display
    if (rtc.hour() < 10)                                // BUGFIX: always 2 digits: add leading zero
        lcd.print('0');
    
    lcd.print(String(rtc.hour()) + ":");                // Print hour
    if (rtc.minute() < 10)
        lcd.print('0');                                 // Print leading '0' for minute
    
    lcd.print(String(rtc.minute()) + ":");              // Print minute
    if (rtc.second() < 10)
        lcd.print('0');                                 // Print leading '0' for second
    
    lcd.print(String(rtc.second()));                    // Print second
    lcd.print(' ');
    lcd.print(rtc.dayStr());                            // Print day string
    lcd.print(' ');
    if (rtc.month() < 10)
        lcd.print('0');
    
    lcd.print(String(rtc.month()) + "/" +               // Print month
        String(rtc.date()) + "/");                      // Print date
    
    lcd.print(String(rtc.year()));                      // Print year (note: DO NOT USE "println" (artifacts shown))
}

void printTime()                                        // print time to Serial Terminal
{
    Serial.print(String(rtc.hour()) + ":");             // Print hour
    if (rtc.minute() < 10)
        Serial.print('0');                              // Print leading '0' for minute
    
    Serial.print(String(rtc.minute()) + ":");           // Print minute
    if (rtc.second() < 10)
        Serial.print('0');                              // Print leading '0' for second
    
    Serial.print(String(rtc.second()));                 // Print second
    
    if (rtc.is12Hour())                                 // If we're in 12-hour mode
    {
        if (rtc.pm())                                   // read AM/PM state
            Serial.print(" PM");                        // Returns true if PM
        else
            Serial.print(" AM");
    }
    Serial.print(" | ");

    // Options for printing the day:
    Serial.print(rtc.dayStr());                         // Print day string
    //Serial.print(rtc.dayC());                         // Print day character
    //Serial.print(rtc.day());                          // Print day integer (1-7, Sun-Sat)
    Serial.print(" - ");
    Serial.print(String(rtc.month()) + "/" +            // Print month
        String(rtc.date()) + "/");                      // Print date
    Serial.println(String(rtc.year()));                 // Print year
}

void I2C_20x4LCD_set()                                  // Initial Splash Screen
{
    lcd.init();                                         // initialize the lcd 
    // Print a message to the LCD.
    lcd.backlight();
    lcd.setCursor(0, 0);                                // setCursor(Character #, Line #)
    lcd.print("    Joel Brigida    ");
    lcd.setCursor(0, 1);
    lcd.print("   ESP32 Security   ");
    lcd.setCursor(0, 2);
    lcd.print("      CDA 4630      ");
    lcd.setCursor(0, 3);
    lcd.print("  Embedded Systems  ");
    //delay(2000);
    //lcd.clear();
}

void setup()
{
    delay(30);
    Serial.begin(115200);
    Wire.begin(21, 22, 4000);                           // SDA, SCL, DATA speed: Note: QWIIC Connector Pins
    rtc.begin();                                        // Call rtc.begin() to initialize the library
    delay(1000);

    pinMode(PIR, INPUT);              				    // GPIO_39 (Pin A3) Set as input for the PIR sensor
    
    FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness( BRIGHTNESS );
    
    I2C_20x4LCD_set();

    leds[0] = CRGB::White;                   		    // Power up all Pin 2 LEDs
    FastLED.show();
    delay(2000);								        // 1.5 second power on delay
    lcd.clear();                                        // clear LCD Display
    leds[0] = CRGB::Black;                   		    // Turn Off Pin 2 LEDs
    FastLED.show();
    
    ESP32PWM::allocateTimer(0);						    // Allow allocation of all timers
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);
    servoObj.setPeriodHertz(50);                        // standard 50 hz servo
    servoObj.attach(servoPin, 1000, 2000);  		    // attaches the servo on pin 4 to the servo object

    setLocked(true);							        // Lock Door Servo at startup
    armed = 1;
    Serial.print("Setup Running on Core: ");
    Serial.println(xPortGetCoreID());
    //delay(1000);
}

void loop()
{
    Serial.print("Loop Running on Core: ");
    Serial.println(xPortGetCoreID());
    static int8_t lastSecond = -1;

    rtc.update();                                       // update all rtc.seconds(), rtc.minutes(), return functions
    if (rtc.second() != lastSecond)                     // If the second has changed
    {
        printTimeScrn();                                // Print the new time to the LCD screen
        Serial.println("Call to printTime() Done");
      
        lastSecond = rtc.second();                      // Update lastSecond value
        Serial.println("lastSecond Updated");
    }

	myKeyPress = userKeypad.getKey();                   // retrieve key pressed by user
  	if (myKeyPress == '*')							    // relocked anytime or clear password with a '*' press
  	{
        if (!armed)         						    // only beep when locking from unlocked state
        {
            leds[0] = CRGB::Red;
            FastLED.show();
            delay(100);               			        // length of light blink = 100 mS
            leds[0] = CRGB::Black;
            FastLED.show();
            delay(200);
        }
        position = 0;                     	            // reset password counter
        setLocked(true);                  		        // move servo to 0 = LOCKED
        armed = 1;
    }
	
    if (armed)
    {
        digPIR = digitalRead(PIR);            		    // read value for PIR sensor = GPIO_39
        if (digPIR)                            	        // if digPIR == 1 then there is motion
        {
            theAlarm = 1;                               // set alarm trip
            //setPIR = 1;                      		    // store PIR trip
        }

        for (int i = 0; i < 5; i++)
        {
            noiseTotal += analogRead(micSense);  	    // add up 5 total readings
            delay(20);                         		    // delay 20 mSec
        }
        noise = (noiseTotal / 5);             		    // take average reading

        if (noise > 250)                     		    // evaluate average reading
        {
            theAlarm = 1;                      	        // set off the alarm
            //noisetrip = noise;                 	    // store value which tripped alarm
            //noiseCtr++;
        }
        noise = 0;                             	        // reset values so they don't overflow
        noiseTotal = 0;

        if (myKeyPress == password[position])
        {
            position++;                        		    // if the digit matches: increase counter & move to next array index
            Serial.print("Position = ");
            Serial.println(position);                   // debug
        }
        
        if (position == 4)                     	        // if 4 digits were entered correctly
        {
            for (int i = 0; i < 2; i++)
            {
                leds[0] = CRGB::Red;
                FastLED.show();
                delay(100);                 		    // length of blink = 100 mS
                leds[0] = CRGB::Black;
                FastLED.show();
                delay(200);                             // delay between blinks = 200 mS
            }
            setLocked(false);                           // OPEN Sesame...
            armed = 0;
            theAlarm = 0;
            position = 0;                               // reset counter back to ZERO
        }
    }

    lcd.setCursor(0, 1);
    if (armed == 1)
    {
        lcd.print("*** System Armed ***");
    }
    else
    {
        lcd.print("***** Disarmed *****");
    }

    lcd.setCursor(0, 2);
    if (theAlarm == 1)                                  // if alarm is tripped
    {
        lcd.print("=>> Alarm Trip!! <<=");              // display when alarm is tripped
    }
    else
    {
        lcd.print("                    ");              // clear line when Alarm is reset
    }

    if (theAlarm == 1)
    {
        leds[0] = CRGB::Red;             		        // Red = Alarm
        FastLED.show();
    }
}