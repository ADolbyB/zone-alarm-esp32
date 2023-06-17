/**
 * Joel Brigida
 * CDA 4630: Embedded Systems
 * Test program for ESP32 to read/write to an SD card over SPI
 * Be sure to set the CS pin correctly at top of code below
 * This also includes code for the RTC to initialize the clock and
 * timestamps events written to the SD card.
*/
#include <SparkFunDS1307RTC.h>
#include <Arduino.h>
#include <Wire.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"

#define PRINT_USA_DATE                                          // MM/DD/YYYY Format (Comment out for DD/MM/YYYY)
#define SQW_INPUT_PIN 2                                         // Input pin to read SQW
#define SQW_OUTPUT_PIN 13                                       // LED to indicate SQW's state

const int SD_CS = 5;                                            // Thing Plus C

void listDir(fs::FS &fs, const char *dirname, uint8_t levels) 
{
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if (!root) 
    {
        Serial.println("Failed to open directory");
        return;
    }
    if (!root.isDirectory()) 
    {
        Serial.println("Not a directory!");
        return;
    }

    File file = root.openNextFile();
    while (file) 
    {
        if (file.isDirectory()) 
        {
            Serial.print("  DIR: ");
            Serial.println(file.name());
            if (levels) 
            {
                listDir(fs, file.name(), levels - 1);
            }
        }
        else 
        {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

void createDir(fs::FS &fs, const char *path) 
{
    Serial.printf("Creating Dir: %s\n", path);
    if (fs.mkdir(path)) 
    {
        Serial.println("Dir created");
    }
    else 
    {
        Serial.println("mkdir failed");
    }
}

void removeDir(fs::FS &fs, const char *path) 
{
    Serial.printf("Removing Dir: %s\n", path);
    
    if (fs.rmdir(path))
    {
        Serial.println("Dir removed");
    }
    else
    {
        Serial.println("rmdir failed");
    }
}

void readFile(fs::FS &fs, const char *path) 
{
    Serial.printf("Reading file: %s\n", path);
    
    File file = fs.open(path);
    if (!file) 
    {
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.print("Read from file: ");
    while (file.available()) 
    {
        Serial.write(file.read());
    }
    file.close();
}

void writeFile(fs::FS &fs, const char *path, const char *message) 
{
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if (!file) 
    {
        Serial.println("Failed to open file for writing");
        return;
    }
    
    if (file.print(message))
    {
        Serial.println("File written");
    }
    else
    {
        Serial.println("Write failed");
    }
    file.close();
}

void appendFile(fs::FS &fs, const char *path, const char *message) 
{
    Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if (!file) 
    {
        Serial.println("Failed to open file for appending");
        return;
    }
    
    if (file.print(message))
    {
        Serial.println("Message appended");
    }
    else
    {
        Serial.println("Append failed");
    }
    file.close();
}

void renameFile(fs::FS &fs, const char *path1, const char *path2) 
{
    Serial.printf("Renaming file %s to %s\n", path1, path2);
    if (fs.rename(path1, path2)) 
    {
        Serial.println("File renamed");
    } 
    else 
    {
        Serial.println("Rename failed");
    }
}

void deleteFile(fs::FS &fs, const char *path) 
{
    Serial.printf("Deleting file: %s\n", path);
    if (fs.remove(path)) 
    {
        Serial.println("File deleted");
    } 
    else 
    {
        Serial.println("Delete failed");
    }
}

void testFileIO(fs::FS &fs, const char *path) 
{
    File file = fs.open(path);
    static uint8_t buf[512];
    size_t len = 0;
    uint32_t start = millis();
    uint32_t end = start;
    
    if (file) 
    {
        len = file.size();
        size_t flen = len;
        start = millis();
        
        while (len) 
        {
            size_t toRead = len;
            if (toRead > 512) 
            {
                toRead = 512;
            }
            file.read(buf, toRead);
            len -= toRead;
        }
        
        end = millis() - start;
        Serial.printf("%u bytes read for %u ms\n", flen, end);
        file.close();
    }
    else
    {
        Serial.println("Failed to open file for reading");
    }

    file = fs.open(path, FILE_WRITE);
    if (!file)
    {
        Serial.println("Failed to open file for writing");
        return;
    }

    size_t i;
    start = millis();
    
    for (i = 0; i < 2048; i++) 
    {
        file.write(buf, 512);
    }
    
    end = millis() - start;
    Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
    file.close();
}

void SD_Test()
{
    Serial.println("SD Card Test Test....");

    if (!SD.begin(SD_CS))
    {
        Serial.println("SD Card Mount Failed!! Exiting....");
        return;
    }
    
    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE)
    {
        Serial.println("No SD card inserted!! Exiting....");
        return;
    }

    Serial.print("SD Card Type: ");                             // Determine Card Type
    if (cardType == CARD_MMC)
    {
        Serial.println("MMC");
    }
    else if (cardType == CARD_SD)
    {
        Serial.println("SD (Std Cap)");
    }
    else if (cardType == CARD_SDHC)
    {
        Serial.println("SDHC");
    }
    else
    {
        Serial.println("UNKNOWN");
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);          // Compute card size in MB
    Serial.printf("SD Card Size: %lluMB\n", cardSize);          // Ref: https://cplusplus.com/reference/string/to_string/

    listDir(SD, "/", 0);
    createDir(SD, "/mydir");
    
    listDir(SD, "/", 0);
    removeDir(SD, "/mydir");
    
    listDir(SD, "/", 2);
    writeFile(SD, "/hello.txt", "Hello ");
    appendFile(SD, "/hello.txt", "World!\n");
    
    readFile(SD, "/hello.txt");
    deleteFile(SD, "/foo.txt");                                 // Should fail (does not exist)
    renameFile(SD, "/hello.txt", "/foo.txt");
    
    readFile(SD, "/foo.txt");
    testFileIO(SD, "/test.txt");
    
    Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
    Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
}

void printTime()
{
    Serial.print(String(rtc.hour()) + ":");                     // Print hour
    if (rtc.minute() < 10)
        Serial.print('0');                                      // Print leading '0' for minute
    
    Serial.print(String(rtc.minute()) + ":");                   // Print minute
    if (rtc.second() < 10)
        Serial.print('0');                                      // Print leading '0' for second
    
    Serial.print(String(rtc.second()));                         // Print second

    if (rtc.is12Hour())                                         // If we're in 12-hour mode
    {
        if (rtc.pm()) 
            Serial.print(" PM");                                // read AM/PM: Returns true if PM
        else 
            Serial.print(" AM");
    }
  
    Serial.print(" | ");

    Serial.print(rtc.dayStr());                                 // Print day string OR
    //Serial.print(rtc.dayC());                                 // Print day character OR
    //Serial.print(rtc.day());                                  // Print day integer (1-7, Sun-Sat)
    Serial.print(" - ");

    #ifdef PRINT_USA_DATE
        Serial.print(String(rtc.month()) + "/" + String(rtc.date()) + "/");   // Print month & date
    #else
        Serial.print(String(rtc.date()) + "/" + String(rtc.month()) + "/");   // Print day & month
    #endif
    
    Serial.println(String(rtc.year()));                         // Print year
}

void setup() 
{
    Serial.begin(115200); // Use the serial monitor to view time/date output
    delay(1000);
    Serial.println("Serial Terminal Running....");
    
    //pinMode(SQW_INPUT_PIN, INPUT_PULLUP);
    //pinMode(SQW_OUTPUT_PIN, OUTPUT);
    //digitalWrite(SQW_OUTPUT_PIN, digitalRead(SQW_INPUT_PIN));
    
    rtc.begin(); // Call rtc.begin() to initialize the library
    
    // (Optional) Sets the SQW output to a 1Hz square wave.
    // (Pull-up resistor is required to use the SQW pin.)
    //rtc.writeSQW(SQW_SQUARE_1);
    
    // Now set the time...
    // You can use the autoTime() function to set the RTC's clock and
    // date to the compiliers predefined time. (It'll be a few seconds
    // behind, but close!)
    //rtc.autoTime();

    // Or you can use the rtc.setTime(s, m, h, day, date, month, year)
    // function to explicitly set the time:
    // e.g. 7:32:16 | Monday October 31, 2016:
    //rtc.setTime(00, 36, 16, 5, 11, 11, 21);  // Uncomment to manually set time
    //rtc.set12Hour(); // Use rtc.set12Hour to set to 12-hour mode
    delay(500);
    //Serial.println("Auto-Time DONE");
}

void loop() 
{
    static int8_t lastSecond = -1;
    
    // Call rtc.update() to update all rtc.seconds(), rtc.minutes(),
    // etc. return functions.
    rtc.update();

    if (rtc.second() != lastSecond) // If the second has changed
    {
        printTime(); // Print the new time
        //Serial.println("Call to printTime() Done");
        
        lastSecond = rtc.second(); // Update lastSecond value
        //Serial.println("lastSecond Updated");
    }

    // Read the state of the SQW pin and show it on the
    // pin 13 LED. (It should blink at 1Hz.)
    //digitalWrite(SQW_OUTPUT_PIN, digitalRead(SQW_INPUT_PIN));
}