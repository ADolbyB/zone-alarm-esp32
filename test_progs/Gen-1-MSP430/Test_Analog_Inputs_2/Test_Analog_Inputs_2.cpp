/*
 * Joel Brigida
 * CDA 4630: Embedded Systems
 * This is a test sketch to test alarm analog and digital inputs.
 * The analog input is a noise sensor (detects breaking glass) and the 
 * digital input is a PIR motion detector.
 * It reads inputs and when a threshold is crossed, it sets off the "alarm" 
 * by illuminating the Red LED.
 */

#include <msp430.h>

const int alarmLED = 8;                             // P2.0;
const int micSense = A5;                            // P1_5;
const int PIR = 6;                                  // P1_4;
int switches1 = 0;                                  // P1IN
int switches2 = 0;                                  // P2IN

int noise = 0;
int noiseTotal = 0;
int noiseTrip = 0;                                  // stores threshold value for last noise alarm trip
short digPIR = 0;                                   // 1 if motion detected
short PIRtrip = 0;                                  // 1 if the PIR tripped the alarm

//bool armed = 0;
short alarm = 0;                                    // 1 if alarm tripped

void setup()
{
    pinMode(alarmLED, OUTPUT);                      // set output for Pin 2.0
    digitalWrite(alarmLED, LOW);                    // turn off LED (Active High)
    pinMode(PIR, INPUT);                            // set PIR sensor as input
    switches1 = P1IN;
    switches2 = P2IN;
}

void loop()
{
    digPIR = digitalRead(PIR);                      // read value for PIR sensor
    if (digPIR)                                     // if motion detected, trip alarm
    {
        alarm = 1;
        PIRtrip = 1;
    }

    for (int i = 0; i < 5; i++)
    {
        noiseTotal += analogRead(micSense);         // add up 5 total readings
        delay(20);                                  // delay 20 mSec
    }
    noise = (noiseTotal / 5);                       // take average reading

    if (noise > 200)                                // if average > 200, trip alarm
    {
        alarm = 1;                                  // set off the alarm
        noiseTrip = noise;                          // store value that tripped alarm
    }
    noise = 0;                                      // reset noise value for next time
    noiseTotal = 0;                                 // reset the total noise

    if (alarm == 1)
    {
        digitalWrite(alarmLED, HIGH);               // turn on LED (Active High)
    }
}