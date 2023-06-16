/*
 * Joel Brigida
 * CDA4630: Embedded Systems
 * This is a test sketch to test the LCD with the servo.
 * The servo is moved continuously in a for() loop and the position data
 * is read and written to the LCD.
 * Note that this LCD is not I2C, it is the 8-bit Parallel input type.
 * LCD pins assignment:
 * Power pins: A & VDD connected to VCC; (VO, RW, VSS & K) connected to GND
 * Signal pins: RS = P2_0, E = P2_1, D4 = P2_2, D5 = P2_3, D6 = P2_4, D7 = P2_5
 */

#include <msp430.h>
#include <LiquidCrystal.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <stdarg.h>
#include <Servo.h>

Servo myServo;                                          // Instantiate servo object

int IRdist = 2;                                         // analog pin used to connect the potentiometer
int setDist = 300;
int servo;                                              // variable to read the value from the analog pin
int distance;


LiquidCrystal lcd (P2_0, P2_1, P2_2, P2_3, P2_4, P2_5);

void setup() 
{
    lcd.begin(16, 2);                                   // Initialize the 16*2 LCD display
    delay(100);
    lcd.clear();

    myServo.attach(19);                                 // attaches the servo on pin 9 to the servo object

    for(int j = 1; j <= 10; j++)                        // Servo initial test pattern
    { 
        for(int i = 0; i <= 18 * j; i++) 
        {
            myServo.write(i);
            delay(100 / (18 * j)); 
        } 
        delay(1000);
        
        for(int i = 0; i <= 18 * j; i++) 
        {
            myServo.write(18 * j - i);
            delay(100 / (18 * j)); 
        } 
        delay(1000);
    }
}

void loop()
{
    distance = 0;
    for(int i = 0; i <= 2; i++) 
    {
        distance += analogRead(IRdist);                 // read potentiometer value: [0, 1023]
        __delay_cycles(5000); 
    } 
    distance = distance / 3;
    
    if (distance <= 300)                                // set servo limits to 300-900
    {
        distance = 300;
    }
    
    if (distance >= 900) 
    {
        distance = 900;
    }  
    __delay_cycles(1000);

    if ((distance > 300) && (abs(setDist - distance) <= setDist * .1))
    {
        setDist = distance;                             // map the servo value from the distance
        
        servo = map(distance, 300, 900, 0, 180);        // scale servo between 0 and 180
        if (servo <= 0) 
        {
            servo = 0;                                  // set servo limits 0-180
        }  
        
        if (servo >= 180) 
        {
            servo = 180;
        }
        myServo.write(servo);
        __delay_cycles(10000);                          // run servo and wait for it to get there
    }

    lcd.setCursor(0,0);                                 // Print Servo Values on the LCD
    lcd.print("distance ");
    lcd.setCursor(9,0);
    lcd.print("servo  ");
    lcd.setCursor(0,1);
    lcd.print("         ");
    lcd.setCursor(9,1);
    lcd.print("       ");
    lcd.setCursor(0,1);
    lcd.print(distance);
    lcd.setCursor(4,1);
    lcd.print(setDist);
    lcd.setCursor(9,1);
    lcd.print(servo);

    __delay_cycles(1000000);                            // run servo & wait for it to get there
}