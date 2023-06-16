/**
 * Joel Brigida: Embedded Systems
 * This is a 2nd subroutine to test the LCD Display and the RTC at the same time
 * This subroutine displays the current time from the RTC to the LCD Screen
 * Works with Energia IDE (Arduino Language)
 */

#include <LiquidCrystal_I2C.h>
#include <SparkFunDS1307RTC.h>
#include <Wire.h>
//#include <Servo.h>
//#include <Keypad.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);                 // declare lcd object: (ADDRESS, #CHARS, #LINES)
int count = 0;

void setup()
{
    Serial.begin(9600);
    Wire.begin();                                   // Initialize Pins P2.1 and P2.2 for I2C bus
    rtc.begin();
    lcd.begin();                                    // initialize the lcd
    LCD_Splash();                                   // Welcome Screen for LCD display
}

void loop()
{
    Time_Refresh();                                 // Update Time on LCD
}

void Time_Refresh()
{
    static int8_t lastSecond = -1;
    rtc.update();                                   // updates all rtc.seconds(), rtc.minutes(), etc.

    if (rtc.second() != lastSecond)                 // If the second has changed
    {
       printTime();                                 // Print the new time to the Serial Terminal
       printTimeScrn();                             // Print new time to the LCD display
       lastSecond = rtc.second();                   // Update lastSecond value
    }
}

void LCD_Splash()
{
    // Print a message to the LCD.
    lcd.backlight();
    lcd.setCursor(0, 0);                            // setCursor(Character #, Line #)
    lcd.print("Hello, world!");
    lcd.setCursor(0, 1);
    lcd.print("Testing MSP430");
    lcd.setCursor(0, 2);
    lcd.print("Joel Brigida");
    lcd.setCursor(0, 3);
    lcd.print("Home Security");
    Serial.println("***Begin Time Display***");
    delay(2000);
    lcd.clear();
}

void printTimeScrn()                                // Show time on LCD
{
    lcd.setCursor(0, 0);                            // 1st line of display
    lcd.print("Current Time:");
    lcd.setCursor(0, 1);                            // 2nd line of display
    lcd.print(String(rtc.hour()) + ":");            // Print hour
    
    if (rtc.minute() < 10)
        lcd.print('0');                             // Print leading '0' for minute
    lcd.print(String(rtc.minute()) + ":");          // Print minute
    
    if (rtc.second() < 10)
        lcd.print('0');                             // Print leading '0' for second
    lcd.print(String(rtc.second()));                // Print second

    if (rtc.is12Hour())                             // If we're in 12-hour mode
    {
        if (rtc.pm())                               // reads the AM/PM state of the hour
            lcd.print(" PM");                       // Returns true if PM
        else
            lcd.print(" AM");
    }

    // lcd.print(" | ");
    lcd.setCursor(0, 2);

    // Options for printing the day, pick one:
    // lcd.print(rtc.dayStr());                     // Print day string
    // lcd.print(rtc.dayC());                       // Print day character
    // Serial.print(rtc.day());                     // Print day integer (1-7, Sun-Sat)
    // lcd.print(" - ");
    #ifdef PRINT_USA_DATE
    lcd.print(String(rtc.month()) + "/" +           // Print month
                 String(rtc.date()) + "/");         // Print date
    #else
    lcd.print(String(rtc.date()) + "/" +            // (or) print date
                 String(rtc.month()) + "/");        // Print month
    #endif
    lcd.print("20");                                // print 1st half of year
    lcd.print(String(rtc.year()));                  // Print year (note: DO NOT USE "println" (artifacts shown))
}

void printTime()                                    // Show time in Serial Terminal
{
    Serial.print(String(rtc.hour()) + ":");         // Print hour
    if (rtc.minute() < 10)
        Serial.print('0');                          // Print leading '0' for minute
    
    Serial.print(String(rtc.minute()) + ":");       // Print minute
    if (rtc.second() < 10)
        Serial.print('0');                          // Print leading '0' for second
    
    Serial.print(String(rtc.second()));             // Print second

    if (rtc.is12Hour())                             // If we're in 12-hour mode
    {
        if (rtc.pm())                               // reads the AM/PM state of the hour
            Serial.print(" PM");                    // Returns true if PM
        else
            Serial.print(" AM");
    }

    Serial.print(" | ");

    // Options for printing the day, pick one:
    Serial.print(rtc.dayStr());                     // Print day string
    // Serial.print(rtc.dayC());                    // Print day character
    // Serial.print(rtc.day());                     // Print day integer (1-7, Sun-Sat)
    Serial.print(" - ");
    
    #ifdef PRINT_USA_DATE
    Serial.print(String(rtc.month()) + "/" +        // Print month
                 String(rtc.date()) + "/");         // Print date
    #else
    Serial.print(String(rtc.date()) + "/" +         // (or) print date
                 String(rtc.month()) + "/");        // Print month
    #endif
    Serial.println(String(rtc.year()));             // Print year
}
