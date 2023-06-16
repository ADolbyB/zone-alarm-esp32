/**
 * Joel Brigida
 * CDA-4630 Embedded Systems
 * This is a test program I used to scan and identify the addresses of the I2C
 * devices I needed to use for the project.
*/

#include <Wire.h>

byte address;
byte busStatus;

void setup()
{
    Serial.begin(9600);
    Wire.begin();
}

void loop()
{
    int numDevices = 0;
  
    delay(5000);                                            // 5 second delay after each scan of the address space.

    busStatus = -1;
    address = 0;

    Serial.println("Scanning for I2C devices ...");
    for(address = 0x00; address < 0x80; address++)
    {
        Wire.beginTransmission(address);
        delay(5);
        busStatus = Wire.endTransmission();
        if (busStatus == 0x00)
        {
            Serial.print("I2C device found at address 0x");
            Serial.print(address, HEX);
            Serial.print("\n");
            numDevices++;
        }
    }
    if (numDevices == 0)
    {
        Serial.print("\nNo I2C devices found\n");
    }
    else
    {
        Serial.print("# devices found: ");
        Serial.print(numDevices);
        Serial.print("\n");
    }
}