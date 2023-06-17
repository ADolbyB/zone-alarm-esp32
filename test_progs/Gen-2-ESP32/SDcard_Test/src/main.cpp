/**
 * Joel Brigida
 * CDA 4630: Embedded Systems
 * Test program for ESP32 to read/write to an SD card over SPI
 * Be sure to set the CS pin correctly at top of code below
 * This also includes code for the RTC to initialize the clock and
 * timestamps events written to the SD card.
*/

#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include "time.h"
#include "SparkFun_Qwiic_OpenLog_Arduino_Library.h"

const char* ssid = "SSID Name Here";
const char* password = "Network PW Here";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = (-5 * 3600);                        // EST = GMT - 5Hrs
const int   daylightOffset_sec = 3600;                          // EDT Adjustment = +1hr

int ledPin = 13;                                                // Status LED connected to digital pin 13
const byte OpenLogAddress = 42;                                 // Default Qwiic OpenLog I2C address
OpenLog myLog;                                                  // Create Logging Instance

void printLocalTime()
{
    struct tm timeinfo;                                         // reference: https://cplusplus.com/reference/ctime/tm/
    
    if(!getLocalTime(&timeinfo))
    {
        Serial.println("Failed to fetch time from NTP Server");
        return;
    }
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

void setup()
{
    pinMode(ledPin, OUTPUT);

    Serial.begin(9600);
    Wire.begin();                                               // Start I2C communication
    myLog.begin();                                              // Start OpenLog SD Card I2C Communication
    delay(1000);

    Serial.print("Connecting to: ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(250);
        Serial.print(".");
    }
    Serial.println("CONNECTED!!");

    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);   // Init & get time from NTP
    printLocalTime();                                           // Display Time to Serial Terminal

    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    delay(500);

    Serial.println("Run OpenLog New File Test");
    myLog.create("NewFile.txt");
    myLog.append("NewFile.txt");
    myLog.println("1) Run OpenLog New File Test");              // Goes to the default LOG#.txt file
    myLog.println("2) Does this get recorded to newFile.txt??");
    myLog.println("3) This is also recorded to the default log file. But a new file has been created");
    myLog.println("4) If you want to write to a file use appendFile\n\n");
    myLog.syncFile();

    Serial.println("=>> Done! <<=");
}

void loop()
{
    digitalWrite(ledPin, HIGH);                                 // Blink the Status LED when finished
    delay(1000);
    printLocalTime();    
    digitalWrite(ledPin, LOW);
    delay(1000);
}