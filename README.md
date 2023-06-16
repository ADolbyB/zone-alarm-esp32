# ESP32 Zone Alarm

A scalable ESP32 based single zone alarm system using Arduino framework.

This was an individual term project for FAU Embedded Systems CDA-4630 class.

Gen-1 originally started with the [TI MSP-EXP430G2ET LaunchPad](https://www.ti.com/tool/MSP-EXP430G2ET) with the MSP430-G2553 MCU.

For that portion of the project, I used TI [Code Composer Studio IDE](https://www.ti.com/tool/CCSTUDIO#downloads) and also the [Energia IDE](https://energia.nu/download/) to start the project.

This all turned out to be insufficient to satisfy my requirements, so I upgraded to the ESP32 MCU.

The dev board I chose for this project was the [ESP32 Thing Plus C](https://www.sparkfun.com/products/20168) from [SparkFun](https://www.sparkfun.com/) Electronics.

Gen-2 was continued reusing the code from Gen-1 but using the ESP32, Visual Studio Code and the PlatformIO extension.

This code and project is fully functional, however it is still a work in progress being rewritten in the ESP32 port of FreeRTOS. In the `test_progs` folder are previous versions and other test programs which have assisted me in the deployment of this project. I keep them as a reference, since at the time of building the project, I was not using GitHub, so I created a new iteration manually as a separate folder and added / subtracted functionality as needed for testing.

## Status:

![GitHub repo size](https://img.shields.io/github/repo-size/ADolbyB/zone-alarm-esp32?label=Repo%20Size&logo=Github)