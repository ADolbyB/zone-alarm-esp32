/**
 * Joel Brigida
 * CDA 4630: Embedded Systems
 * This is a test sketch used with VS Code & PlatformIO to scan the I2C
 * bus for devices. At the time, PlatformIO did not work well with the 
 * MSP430, so I went back to using Energia and TI CCS.
*/

#include <Arduino.h>
#include <Wire.h>

byte busStatus;
byte address;
int numDevices = 0;

void setup()
{
    Serial.begin(9600);
    Wire.begin(P2_1, P2_2, 4000);
}

void loop()
{
    delay(5000);
    numDevices = 0;                                         // Reset counter each loop iteration

    Serial.println("Scanning for I2C devices ...");
    
    for(address = 0x00; address < 0x80; address++)
    {
        Wire.beginTransmission(address);
        delay(100);
        busStatus = Wire.endTransmission();
        
        if (busStatus != 0x00)
        {
            Serial.println("I2C device found at address 0x%02p", address);
            Serial.print(address, HEX);
            numDevices++;
        }
    }
    
    if (numDevices == 0)
    {
        Serial.print("\nNo I2C devices found\n");
    }
    else
    {
        Serial.println("\n# I2C devices = %d\n", numDevices);
    }
}