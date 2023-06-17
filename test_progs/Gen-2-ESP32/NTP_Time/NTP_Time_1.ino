/**
 * Joel Brigida
 * CDA 4630: Embedded Systems
 * This is a test program for the WiFi stack. This uses Arduino IDE to
 * Flash the ESP32 for this sketch. It pulls time from an internet NTP
 * server over a WiFi Connection. This is useful for timestamping events.
 * Strftime Format References: https://www.foragoodstrftime.com/, https://strftime.org/
 * C++ strftime() ref: https://cplusplus.com/reference/ctime/strftime/
*/

#include <WiFi.h>
#include "time.h"

const char *ssid = "SSID-NAME-HERE";
const char *password = "Network PW Here";

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = (-5 * 3600);                             // EST = GMT -5hrs
const int  daylightOffset_sec = 3600;                               // 0 if not DST, +1 hr for DST

void printLocalTime()
{
    struct tm timeinfo;                                             // reference: https://cplusplus.com/reference/ctime/tm/
    
    if(!getLocalTime(&timeinfo))
    {
        Serial.println("Failed to fetch time from NTP Server");
        return;
    }
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");             // see strftime references
    
    Serial.print("Day of week: ");
    Serial.println(&timeinfo, "%A");
    
    Serial.print("Month: ");
    Serial.println(&timeinfo, "%B");
    
    Serial.print("Day of Month: ");
    Serial.println(&timeinfo, "%d");
    
    Serial.print("Year: ");
    Serial.println(&timeinfo, "%Y");
    
    Serial.print("Hour: ");
    Serial.println(&timeinfo, "%H");
    
    Serial.print("Hour (12 hour format): ");
    Serial.println(&timeinfo, "%I");
    
    Serial.print("Minute: ");
    Serial.println(&timeinfo, "%M");
    
    Serial.print("Second: ");
    Serial.println(&timeinfo, "%S");

    Serial.println("Time variables: ");
    char timeHour[3];
    strftime(timeHour, 3, "%H", &timeinfo);                         // Save Time variables in a buffer for time stamping, etc
    Serial.println(timeHour);
    
    char timeWeekDay[10];
    strftime(timeWeekDay, 10, "%A", &timeinfo);
    Serial.println(timeWeekDay);                                    // Save Time variables in a buffer for time stamping, etc
    Serial.println();
}

void setup()
{
    Serial.begin(115200);
    delay(500);
    
    Serial.print("Establishing Connection To ");                    // connect to WiFi network
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(250);
        Serial.print(".");
    }
    Serial.println("WiFi Connected!");
    
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);       // init and get the time
    printLocalTime();

    WiFi.disconnect(true);                                          // disconnect WiFi as it's no longer needed
    WiFi.mode(WIFI_OFF);
}

void loop()
{
    delay(1000);
    printLocalTime();                                               // Keep printing time to Serial Terminal
}