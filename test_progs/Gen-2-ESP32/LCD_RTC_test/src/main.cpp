/**
 * Joel Brigida
 * CDA-4630: Embedded Systems
 * ZoneAlarm Test using ESP32, VSCode, and PlatformIO Extension
 * This is a test sketch to initialize and test the RTC with the ESP32
 * Using a keypad input, PIR sensor.sas
*/

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SparkFunDS1307RTC.h>

int count = 0;
int count1 = 0;
int count2 = 0;
int count3 = 0;

LiquidCrystal_I2C lcd(0x27, 20, 4);                // declare lcd object: (ADDRESS, #CHARS, #LINES)

void printTime();
void printTimeScrn();
void I2C_Poll();                                   // debug: Poll for I2C addresses
void I2C_16x2LCD_set();                            // setup functions for LCD screens
void I2C_20x4LCD_set();
void I2C_16x2LCD_mesg();                           // test message functions for LCD screens
void I2C_20x4LCD_mesg();

void setup() 
{
   Serial.begin(115200);
   Wire.begin(21, 22, 4000);                       // SDA, SCL, DATA speed: Note: QWIIC Connector Pins
   rtc.begin();                                    // Call rtc.begin() to initialize the library
   // Set Time: e.g. 7:32:16 | Monday October 31, 2016:
   // rtc.setTime(16, 32, 7, 2, 31, 10, 16);       // Uncomment to manually set time
   // rtc.setTime(00, 20, 22, 5, 11, 11, 21);
   // I2C_16x2LCD_set();                           // Function Call to setup 16 x 2 LCD
   // rtc.set24Hour(false);
   I2C_20x4LCD_set();
   //rtc.writeSQW(0);                              // Turn off square wave output
}

void loop()
{
   //I2C_Poll();                                   // function call to poll all I2C addresses: DEBUG
   //I2C_20x4LCD_mesg();
   static int8_t lastSecond = -1;
  
   // Call rtc.update() to update all rtc.seconds(), rtc.minutes(),
   // etc. return functions.
   rtc.update();

   if (rtc.second() != lastSecond)                 // If the second has changed
   {
      printTimeScrn();                             // Print the new time to the LCD screen
      Serial.println("Call to printTime() Done");
    
      lastSecond = rtc.second();                   // Update lastSecond value
      Serial.println("lastSecond Updated");
   }
}

void printTimeScrn()
{
   lcd.setCursor(0, 0);                            // 1st line of display
   //lcd.print("=>> Current Time <<=");
   //lcd.setCursor(0, 1);
   //lcd.print(rtc.dayStr());                      // Print day string
   //lcd.setCursor(0, 2);                          // 2nd line of display
   if (rtc.hour() < 10)                            // BUGFIX: always 2 digits: add leading zero
      lcd.print('0');
   lcd.print(String(rtc.hour()) + ":");            // Print hour
   if (rtc.minute() < 10)
      lcd.print('0');                              // Print leading '0' for minute
   lcd.print(String(rtc.minute()) + ":");          // Print minute
   if (rtc.second() < 10)
      lcd.print('0');                              // Print leading '0' for second
   lcd.print(String(rtc.second()));                // Print second

   if (rtc.is12Hour())                             // If we're in 12-hour mode
   {
      // Use rtc.pm() to read the AM/PM state of the hour
      if (rtc.pm()) 
         lcd.print(" PM");                      // Returns true if PM
      else w
         lcd.print(" AM");
   }
  
   //lcd.print(" | ");
   // lcd.setCursor(0, 3);

   // Few options for printing the day, pick one:
   //Serial.print(rtc.dayStr());                   // Print day string
   //Serial.print(rtc.dayC());                     // Print day character
   //Serial.print(rtc.day());                      // Print day integer (1-7, Sun-Sat)
   //Serial.print(" - ");

   lcd.print(' ');
   if (rtc.month() < 10)
      lcd.print('0');
   lcd.print(String(rtc.month()) + "/" +           // Print month
      String(rtc.date()) + "/");                   // Print date
   //lcd.print("20");                              // print 1st half of year
   lcd.print(String(rtc.year()));                  // Print year (note: DO NOT USE "println" (artifacts shown))
   lcd.setCursor(0, 1);
   lcd.print("Sensor 1:           ");
   lcd.setCursor(11, 1);
   lcd.print(count1++);                            // print 1st value
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
   lcd.print(count3);QWqwqwwqWQ
   count3 += 3;
   if (count3 > 254)
      count3 = 0;
}

void printTime()
{
   Serial.print(String(rtc.hour()) + ":");         // Print hour
   if (rtc.minute() < 10)
      Serial.print('0');                           // Print leading '0' for minute
   Serial.print(String(rtc.minute()) + ":");       // Print minute
   if (rtc.second() < 10)
      Serial.print('0');                           // Print leading '0' for second
   Serial.print(String(rtc.second()));             // Print second

   if (rtc.is12Hour())                             // If we're in 12-hour mode
   {
      if (rtc.pm())                                // Use rtc.pm() to read the AM/PM state of the hour
         Serial.print(" PM");                      // Returns true if PM
      else 
         Serial.print(" AM");
   }
  
   Serial.print(" | ");

   // Few options for printing the day, pick one:
   Serial.print(rtc.dayStr());                     // Print day string
   //Serial.print(rtc.dayC());                     // Print day character
   //Serial.print(rtc.day());                      // Print day integer (1-7, Sun-Sat)
   Serial.print(" - ");
   Serial.print(String(rtc.month()) + "/" +        // Print month
      String(rtc.date()) + "/");                   // Print date
   Serial.println(String(rtc.year()));             // Print year
}

void I2C_Poll()
{
   byte busStatus = -1;
   byte address;
   int nDevices = 0;

   delay(5000);

   Serial.println("Scanning for I2C devices ...");
   for(address = 0x00; address < 0x80; address++)
      {
         Wire.beginTransmission(address);
         //delay(100);
         busStatus = Wire.endTransmission();
         if (busStatus == 0x00)
            {
               Serial.printf("I2C device found at address 0x%02X\n", address);
               nDevices++;
            }
      }
  if (nDevices == 0)
      {
         Serial.println("\nNo I2C devices found\n");
      }
  else
      {
         Serial.printf("\n# I2C devices = %d\n", nDevices);
      }
}

void I2C_16x2LCD_set()
{
   lcd.init();                                     // initialize the lcd 
   // Print a message to the LCD.
   lcd.backlight();
   lcd.setCursor(0,0);                             // setCursor(Character #, Line #)
   lcd.print("Hello, world!");
   lcd.setCursor(0,1);
   lcd.print("Testing ESP32");
   delay(2000);
   lcd.clear();
}

void I2C_16x2LCD_mesg()
{
   lcd.setCursor(0, 0);
   lcd.print("JMB ESP-32 Test:");
   lcd.setCursor(0, 1);
   lcd.print("Counting:");
   lcd.setCursor(11, 1);
   lcd.print(count);
   count++;
   delay(250);
}

void I2C_20x4LCD_set()
{
   lcd.init();                                     // initialize the lcd 
   // Print a message to the LCD.
   lcd.backlight();
   lcd.setCursor(0, 0);                            // setCursor(Character #, Line #)
   lcd.print("  <Dark Ridge, LLC> ");
   lcd.setCursor(0, 1);
   lcd.print("   ESP32 Security   ");
   lcd.setCursor(0, 2);
   lcd.print("    Joel Brigida    ");
   lcd.setCursor(0, 3);
   lcd.print("  Embedded Systems  ");
   delay(2000);
   lcd.clear();
}

void I2C_20x4LCD_mesg()
{
   lcd.setCursor(0, 0);
   lcd.print("JMB ESP-32 Test:");
   lcd.setCursor(0, 1);
   lcd.print("Counting:");
   lcd.setCursor(11, 1);
   lcd.print(count);
   lcd.setCursor(0, 2);
   lcd.print("Test for Line #3");
   lcd.setCursor(0, 3);
   lcd.print("Test for Line #4");
   count++;
   delay(250);
}