/**
 * Joel Brigida
 * Original Design: 12/5/2021
 * Minor Updates: 6/9/2023: fixed code spacing, replaced `delay()` -> `vTaskDelay()`
 * Added `vTaskDelete(NULL)` to detup() function.
*/

#include <Arduino.h>
#include <Wire.h>
#include <SparkFunDS1307RTC.h>                              // TODO: Update Library for RTC & check functionality.
#include <Keypad.h>
#include <FastLED.h>
#include <ESP32Servo.h>
#include <LiquidCrystal_I2C.h>

/** Directives for <FastLED.h> */

#define LED_PIN 2                                           // Pin 2 on Thing Plus C is connected to WS2812 LED
#define COLOR_ORDER GRB                                     // RGB LED in top right corner
#define CHIPSET WS2812
#define NUM_LEDS 1
#define BRIGHTNESS 25

/** GPIO Pin Assignments */

const short servoPin = 4;                                   // Servo set up on GPIO_4
const short PIR = 39;                                       // Digital Input for PIR sensor: GPIO_39
const short micSense = 34;                                  // audio sensor = GPIO_34 (A2)
const short buzzer = 13;                                    // Buzzer for alarm = GPIO_13 (On-Board LED)
CRGB leds[NUM_LEDS];                                        // Array for LEDS on GPIO_2

/** Global Variable Declarations */

int pos = 0;                                                // variable to store the servo position
char myKeyPress = 0;                                        // Sets a char as current user keypress
int digPIR = 0;                                             // PIR Sensor: 1 = motion detected, 0 = no motion
char* password = (char*)"789C";                             // declare pointer to char array and add password to array
unsigned short position = 0;                                // number of correct password keys entered
short armed = 0;                                            // armed status: 0 = disarmed, 1 = armed
int the_alarm = 0;                                          // alarm trip: 0 = no trip, 1 = motion, 2 = noise
unsigned noise = 0;                                         // average noise reading of the room
unsigned noiseTotal = 0;                                    // cumulative noise reading before averaging 5 values

/** Keypad variable initializations */

const byte ROWS = 4;                                        // set global constant as # of rows to the keypad
const byte COLS = 4;                                        // set global constant as # of columns to the keypad
byte colPins[COLS] = {33, 15, 32, 14};                      // declare pins of keypad as byte arrays
byte rowPins[ROWS] = {26, 25, 12, 27};
const char buttons[ROWS][COLS] = {                          // declare keypad button as 2-D array
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

/** Global Semaphore Variables */

// TODO: Update With Mutexes for critical section.
unsigned short semAlarm = 1;                                // semaphore for the_alarm variable
unsigned short semArmed = 1;                                // semaphore for armed variable
unsigned short accessArmLCD = 1;                            // variables to enable status write to the LCD screen ONCE
unsigned short accessAlarmLCD = 1;    

/** Object Initializations */

LiquidCrystal_I2C lcd(0x27, 20, 4);                         // declare lcd object: (ADDRESS, #CHARS, #LINES)
Keypad the_keypad = Keypad(makeKeymap(buttons), rowPins, colPins, ROWS, COLS);
Servo servo_obj;                                            // create servo object to control a servo
TaskHandle_t Task0;                                         // Task0 runs on core 0
TaskHandle_t Task1;                                         // Task1 runs on core 1

/** C Style Function Declarations */
// TODO: Move Function Definitions Up Here....
void Task0code(void *pvParameters);                         // Function Runs Code on CPU Core 0
void Task1code(void *pvParameters);                         // Function Runs Code on CPU Core 1
void setLocked(int locked);                                 // Function for Servo motor movement
void printTime();                                           // DEBUG: Prints time to the serial terminal
void printTimeScrn();                                       // Prints time to the LCD screen
void I2C_20x4LCD_set();                                     // Custom splash screen function on powerup

void setup()
{
    vTaskDelay(30 / portTICK_PERIOD_MS);
    //Serial.begin(115200);                                 // Disable Debug Serial function when not needed (DEBUG)
    Wire.begin(21, 22, 4000);                               // SDA, SCL, DATA speed: Note: QWIIC Connector Pins
    rtc.begin();                                            // Call rtc.begin() to initialize the RTC clock library

    //Set Time: e.g. 7:32:16 | Monday October 31, 2016:
    //rtc.setTime(16, 32, 7, 2, 31, 10, 16);                // Example: Uncomment to manually set time
    //rtc.setTime(00, 50, 8, 2, 6, 12, 21);                 // Set Time Proper
    //vTaskDelay(100 / portTICK_PERIOD_MS);
    //rtc.set24Hour(true);                                  // Set 24Hr
    //vTaskDelay(100 / portTICK_PERIOD_MS);
    //rtc.writeSQW(0);                                      // Turn off square wave output
    
    pinMode(PIR, INPUT);                                    // GPIO_39 (Pin A3) Set as input for the PIR sensor
    pinMode(buzzer, OUTPUT);                                // GPIO_36 (Pin A4) Set as output for the alarm buzzer
    vTaskDelay(30 / portTICK_PERIOD_MS);
    digitalWrite(buzzer, HIGH);                             // TEST buzzer at power on during splash screen
    
    FastLED.addLeds <CHIPSET, LED_PIN, COLOR_ORDER> (leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS);
    
    leds[0] = CRGB::White;                                  // Power up all Pin 2 LEDs for Power On Test
    FastLED.show();
    I2C_20x4LCD_set();                                      // Display Splash Screen on Power Up
    vTaskDelay(2000 / portTICK_PERIOD_MS);                  // 2 second power on delay
    digitalWrite(buzzer, LOW);                              // Turn off buzzer / LED (Active High)
    lcd.clear();                                            // clear LCD Display
    
    leds[0] = CRGB::Black;                                  // Turn Off Pin 2 LEDs
    FastLED.show();
    
    ESP32PWM::allocateTimer(0);                             // Allow allocation of all timers (for servo)
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);
    servo_obj.setPeriodHertz(50);                           // standard 50 hz servo
    servo_obj.attach(servoPin, 1000, 2000);                 // attaches the servo on pin 4 to the servo object

    setLocked(true);                                        // Lock Door Servo after power on
    armed = 1;                                              // set armed state

    //create a task that will be executed in the Task0code() function, with priority 1 and executed on core 0
    xTaskCreatePinnedToCore(
        Task0code,                                          /* Task function. */
        "Task0",                                            /* name of task. */
        10000,                                              /* Stack size of task */
        NULL,                                               /* parameter of the task */
        1,                                                  /* priority of the task */
        &Task0,                                             /* Task handle to keep track of created task */
        0                                                   /* pin task to core 0 */
    );

    //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 1
    xTaskCreatePinnedToCore(
        Task1code,                                          /* Task function. */
        "Task1",                                            /* name of task. */
        10000,                                              /* Stack size of task */
        NULL,                                               /* parameter of the task */
        1,                                                  /* priority of the task */
        &Task1,                                             /* Task handle to keep track of created task */
        1                                                   /* pin task to core 1 */
    );
    
    vTaskDelete(NULL);
}

void loop() {}

// TODO: Make Tasks More Atomic to make better use of scheduler
void Task0code(void *pvParameters)                          // CPU Core 0 controls Clock and LCD Screen
{
    for(;;)
    {
        static int8_t lastSecond = -1;
        rtc.update();

        if (rtc.second() != lastSecond)                     // If the second has changed
        {
            printTimeScrn();                                // Print the new time to the LCD screen
            //printTime();                                  // DEBUG: print time to the serial terminal
            lastSecond = rtc.second();                      // Update lastSecond value
        }

        lcd.setCursor(0, 1);                                // Line #2 of the LCD display
        if (semArmed == 1)                                  // if semaphore is ENABLED
        {
            if (accessArmLCD == 1)                          // if the LCD "armed" status has been updated
            {
                if (armed == 1)
                {
                    lcd.print("*** System Armed ***");
                }
                else
                {
                    lcd.print("***** Disarmed *****");
                }
            }
            accessArmLCD = 0;                               // disallow LCD write again until there is a status change
        }

        lcd.setCursor(0, 2);                                // Line #3 of the LCD display
        if (semAlarm == 1)                                  // if semaphore is ENABLED
        {
            if (accessAlarmLCD == 1)                        // if the LCD "alarm" status has been updated
            {
                if (the_alarm == 1)                         // if alarm is tripped
                {
                    lcd.print("* Motion Detected! *");      // display when motion is detected
                }
                else if (the_alarm == 2)
                {
                    lcd.print("** Glass Broken!! **");      // display when glass is broken
                }
                else // if (the_alarm == 0)
                {
                    lcd.print("                    ");      // clear line when Alarm is reset
                }
            }
            accessAlarmLCD = 0;                             // disallow LCD write again until there is a status change
        }
    }
}

void Task1code(void *pvParameters)                          // CPU Core 1 controls Alarm Functions
{
    for(;;)
    {
        myKeyPress = the_keypad.getKey();                   // retrieve key pressed by user

        if (myKeyPress == '*')                              // only keypress we care about in disarmed state is '*'
        {                                                   // If the lock is open it can be relocked anytime with a '*' press
            if (!armed)                                     // only beep when locking from unlocked state
            {
                leds[0] = CRGB::Red;
                FastLED.show();
                delay(100);                                 // length of beep = 100 mS
                leds[0] = CRGB::Black;
                FastLED.show();
                delay(200);
                setLocked(true);                            // move servo to 0 = LOCKED
            }
            position = 0;                                   // reset password counter
            semArmed = 0;                                   // DISABLE Core 0 access to "armed"
            armed = 1;                                      // set armed state
            accessArmLCD = 1;                               // allow an LCD write event for "armed" status
            semArmed = 1;                                   // ENABLE Core 0 access to "armed"
        }

        if (armed)
        {
            digPIR = digitalRead(PIR);                      // read value for PIR sensor = GPIO_39

            if (digPIR)                                     // if digPIR == 1 then there is motion
            {
                semAlarm = 0;                               // DISABLE Core 0 access to "the_alarm" variable
                the_alarm = 1;                              // Set alarm trip for LCD Screen: 1 = PIR trip for LCD
                accessAlarmLCD = 1;                         // allow an LCD write event for "alarm" status
                digitalWrite(buzzer, HIGH);                 // Turn on the buzzer (or LED)
                leds[0] = CRGB::Red;                        // Red = Alarm
                FastLED.show();
                semAlarm = 1;                               // ENABLE Core 0 access to "the_alarm"
            }

            for (int i = 0; i < 5; i++)
            {
                noiseTotal += analogRead(micSense);         // add up 5 total readings
                delay(20);                                  // delay 20 mSec
            }

            noise = (noiseTotal / 5);                       // take average reading

            if (noise > 250)                                // evaluate average reading
            {
                semAlarm = 0;                               // DISABLE Core 0 access to "the_alarm"
                the_alarm = 2;                              // Set alarm trip for LCD Screen: 2 = micSense trip for LCD
                accessAlarmLCD = 1;                         // allow an LCD write event for "alarm" status
                digitalWrite(buzzer, HIGH);                 // Turn on the buzzer (or LED)
                leds[0] = CRGB::Red;                        // Red = Alarm
                FastLED.show();
                semAlarm = 1;                               // ENABLE Core 0 access to "the_alarm"
            }
            noise = 0;                                      // reset values so they don't overflow
            noiseTotal = 0;

            if (myKeyPress == password[position])
            {
                position++;                                 // if the digit matches: increase counter & move to next array index
            }

            if (position == 4)                              // if 4 digits were entered correctly
            {
                for (int i = 0; i < 2; i++)
                {
                    leds[0] = CRGB::Red;
                    FastLED.show();
                    delay(100);                             // length of beep = 100 mS
                    leds[0] = CRGB::Black;
                    FastLED.show();
                    delay(200);                             // delay between beeps = 200 mS
                }
                setLocked(false);                           // OPEN Sesame...

                semArmed = 0;                               // DISABLE Core 0 access to "armed"
                armed = 0;                                  // set disarm state
                accessArmLCD = 1;                           // allow an LCD write event for "armed" status
                semArmed = 1;                               // ENABLE Core 0 access to "armed"
                
                semAlarm = 0;                               // DISABLE Core 0 access to "the_alarm"
                the_alarm = 0;                              // clear alarm state for LCD Screen
                accessAlarmLCD = 1;                         // allow an LCD write event for "armed" status
                digitalWrite(buzzer, LOW);                  // Turn off the buzzer (or LED)
                semAlarm = 1;                               // ENABLE Core 0 access to "the_alarm"
                
                position = 0;                               // reset counter back to ZERO
            }
        }
    }
}

void setLocked(int locked)                                  // function declaration for setLocked function
{
    if (!locked)                                            // if door is locked / closed
    {
        leds[0] = CRGB::Green;                              // Green = Go / Open
        FastLED.show();
        servo_obj.write(180);                               // unlock servo: move to position OPEN = 180
    }
    else // if (locked)
    {
        leds[0] = CRGB::Blue;                               // Blue = secure
        FastLED.show();
        servo_obj.write(0);                                 // lock servo: move to position LOCKED = 0
    }
}

void printTime()                                  
{
    //TODO: Try `sprintf()` method with a buffer.
    /** Time/Date format: 12:34:56 Su MM/DD/YY */           // prints Time to the Serial Terminal (for debug)
    Serial.print(String(rtc.hour()) + ":");                 // Print hour
    if (rtc.minute() < 10)
        Serial.print('0');                                  // Print leading '0' for minute
    Serial.print(String(rtc.minute()) + ":");               // Print minute
    if (rtc.second() < 10)
        Serial.print('0');                                  // Print leading '0' for second
    Serial.print(String(rtc.second()));                     // Print second
    
    Serial.print(" | ");

    // Few options for printing the day, pick one:
    Serial.print(rtc.dayStr());                             // Print day string
    //Serial.print(rtc.dayC());                             // Print day character
    //Serial.print(rtc.day());                              // Print day integer (1-7, Sun-Sat)
    Serial.print(" - ");
    Serial.print(String(rtc.month()) + "/" +                // Print month
        String(rtc.date()) + "/");                          // Print date
    Serial.println(String(rtc.year()));                     // Print year
}

void printTimeScrn()
{
    /** Time/Date format: 12:34:56 Su MM/DD/YY */
    lcd.setCursor(0, 0);                                    // 1st line of display
    if (rtc.hour() < 10)                                    // BUGFIX: always 2 digits: add leading zero
        lcd.print('0');
    lcd.print(rtc.hour());                                  // Print hour
    lcd.print(":");
    if (rtc.minute() < 10)
        lcd.print('0');                                     // Print leading '0' for minute
    lcd.print(rtc.minute());                                // Print minute
    lcd.print(":");
    if (rtc.second() < 10)
        lcd.print('0');                                     // Print leading '0' for second
    lcd.print(rtc.second());                                // Print second
    lcd.print(' ');
    lcd.print(rtc.dayStr());                                // Print day string
    lcd.print(' ');
    if (rtc.month() < 10)
        lcd.print('0');
    lcd.print(rtc.month());                                 // Print month
    lcd.print("/");
    lcd.print(rtc.date());                                  // Print date
    lcd.print("/");
    lcd.print(rtc.year());                                  // Print year (note: DO NOT USE "println" (artifacts shown))
}

void I2C_20x4LCD_set()
{
    lcd.init();                                             // initialize the lcd 
    // Print a message to the LCD.
    lcd.backlight();
    lcd.setCursor(0, 0);                                    // setCursor(Character #, Line #)
    lcd.print("    Joel Brigida    ");
    lcd.setCursor(0, 1);
    lcd.print("   ESP32 Security   ");
    lcd.setCursor(0, 2);
    lcd.print("  Embedded Systems  ");
    lcd.setCursor(0, 3);
    lcd.print("      CDA-4630      ");
    //vTaskDelay(2000 / portTICK_PERIOD_MS);
    //lcd.clear();
}