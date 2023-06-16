/*
 * Joel Brigida
 * CDA4630: Embedded Systems
 * This is a test sketch to test alarm analog and digital inputs.
 * It reads inputs and when the threshold is crossed, it sets off the "alarm" and blinks the Red LED
 * This is a sketch I wrote in Energia IDE that I imported to TI CCS.
 */

#include <msp430.h>

const int alarmLED = P2_0;                      // Micro Pin references
const int micSense = A5;                        // P1_5;
const int PIR = P1_4;                           // P1_4;

int noise = 0;
int noiseTotal = 0;
int switches1;
int switches2;

unsigned digPIR = 0;
bool armed = 0;
bool alarm = 0;

void setup()
{
    pinMode(alarmLED, OUTPUT);                  // set output for Pin 1.2
    digitalWrite(alarmLED, LOW);                // turn off LED (Active High)
    delay(100);
    pinMode(PIR, INPUT);
    delay(10);
    switches1 = P1IN;
    switches2 = P2IN;
}

void loop()
{
    for (int i = 0; i < 5; i++)
    {
        noiseTotal += analogRead(micSense);     // add up 5 total readings
        delay(20);                              // delay 20 mSec
    }
    noise = (noiseTotal / 5);                   // take average reading: value overwritten each iteration
    noiseTotal = 0;                             // reset the total noise

    digPIR = digitalRead(PIR);                  // return value for PIR sensor

    if (noise > 200 || digPIR)
    {
        alarm = 1;                              // set off the alarm
    }

    if (alarm == 1)
    {
        switches2 ^= BIT0;                      // Toggle on & off
    }
    delay(150);
}