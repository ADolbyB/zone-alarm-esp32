/**
 * Joel Brigida
 * CDA 4630: Embedded Systems
 * Another Example to capture time using the WiFi stack with the ESP32.
 * This is useful for time stamping events.
*/

#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

const char *ssid     = "SSID-NAME-HERE";
const char *password = "Network PW Here";

WiFiUDP ntpUDP;                                           // Define NTP Client to get time
NTPClient timeClient(ntpUDP);
String formattedDate;                                     // Variables to save date and time
String dayStamp;
String timeStamp;

void setup()
{
    Serial.begin(115200);
    delay(1000);
    
    Serial.print("Establishing Connection To ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(250);
        Serial.print(".");
    }

    Serial.println("\nWiFi connected, IP Address: ");
    Serial.println(WiFi.localIP());

    timeClient.begin();                                 // Initialize a NTPClient to get time
    timeClient.setTimeOffset(-5 * 3600);                // Time Zone Adjust: EST = GMT -5Hrs
}

void loop()
{
    while(!timeClient.update())
    {
        timeClient.forceUpdate();                       // Force Time Update
    }

    formattedDate = timeClient.getFormattedDate();      // Format: 2018-05-28T16:00:13Z
    Serial.println(formattedDate);

    int splitT = formattedDate.indexOf("T");            // Get string array index of letter 'T'
    dayStamp = formattedDate.substring(0, splitT);      // Extract Date
    
    Serial.print("DATE: ");
    Serial.println(dayStamp);                           // Print Date only

    timeStamp = formattedDate.substring(
        splitT + 1, formattedDate.length() - 1);        // Get time

    Serial.print("HOUR: ");
    Serial.println(timeStamp);                          // Print Time only
    delay(1000);
}