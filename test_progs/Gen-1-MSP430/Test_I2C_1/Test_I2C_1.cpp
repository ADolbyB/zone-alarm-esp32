/**
 * Joel Brigida
 * CDA 4630: Embedded Systems
 * This subroutine tests the LCD Display for proper output on the MSP430
 * Works with the Energia IDE (Uses Arduino Language).
 */

#include <LiquidCrystal_I2C.h>
#include <Wire.h>
//#include <Servo.h>
//#include <Keypad.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);             // declare lcd object: (ADDRESS, #CHARS, #LINES)

int count = 0;                                  // Global Variable for display on LCD

void setup()
{
    Serial.begin(9600);
    Wire.begin();                               // SDA, SCL, DATA speed: Note: QWIIC Connector Pins

    lcd.begin();                                // initialize the lcd
    lcd.backlight();                            // Activate backlight

    lcd.setCursor(0, 0);                        // setCursor(Character #, Line #)
    lcd.print("Hello, world!");
    lcd.setCursor(0, 1);
    lcd.print("Testing MSP430");
    lcd.setCursor(0, 2);
    lcd.print("Line #3: Check 1");
    lcd.setCursor(0, 3);
    lcd.print("Line #4: Check 2");
    delay(2000);
    Serial.println("Hello World!");
    lcd.clear();                                // Clear the display
}

void loop()
{
    lcd.setCursor(0, 0);                        // Beginning of top line
    lcd.print("JMB MSP430 Test:");
    lcd.setCursor(0, 1);                        // Beginning of 2nd Line
    lcd.print("Counting:");
    lcd.setCursor(11, 1);                       // 2nd Line after "Counting: "
    lcd.print(count);                           // Display global variable value
    
    Serial.print("Counting: ");                 // Debug: Display on Serial Terminal
    Serial.println(count);
    
    lcd.setCursor(0, 2);                        // Beginning of 3rd Line
    lcd.print("**Embedded Systems**");
    lcd.setCursor(0, 3);                        // Beginning of Bottom Line
    lcd.print("=> ***CDA-4630*** <=");
    
    count++;
    delay(250);
}