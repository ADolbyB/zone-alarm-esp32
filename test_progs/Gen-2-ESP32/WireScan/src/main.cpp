/**
 * Joel Brigida
 * CDA 4630: Embedded Systems
 * Test program for ESP32 to read addresses of I2C devices.
 * Very similar to the MSP430 Version in Gen-1 folder.
 * I included the libraries to check flash memory consumption.
*/

#include <Arduino.h>
#include <Wire.h>
#include <SparkFunDS1307RTC.h>
#include <SparkFun_Qwiic_OpenLog_Arduino_Library.h>

byte address;
byte busStatus;
int numdevices;

void setup() 
{
    Serial.begin(115200);
    delay(1000);
    Wire.begin(21, 22, 4000);                           // ESP32: SDA, SCL, DATA speed
    Serial.print("Starting Device Scan....\n");
}

void loop()
{
    numDevices = 0;
    busStatus = -1;
    address = 0;

    delay(5000);

    Serial.println("Scanning for I2C devices ...");
    for(address = 0x00; address < 0x80; address++)
    {
        Wire.beginTransmission(address);
        delay(100);
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
        Serial.println("\nNo I2C devices found\n");
    }
    else
    {
        Serial.print("# devices found: ");
        Serial.print(numDevices);
        Serial.print("\n");
    }
}