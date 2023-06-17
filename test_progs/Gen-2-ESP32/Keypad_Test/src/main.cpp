/**
 * Joel Brigida
 * CDA-4630: Embedded Systems
 * ZoneAlarm v2.0 using ESP32, VSCode, and PlatformIO Extension
 * This is a test sketch for Testing Keypad input for ESP32 Thing Plus C
*/
#include <Arduino.h>
#include <Keypad.h>
#include <ESP32Servo.h>
#include <FastLED.h>

#define LED_PIN 2									// Pin 2 on Thing Plus C is connected to WS2812 LED
#define COLOR_ORDER GRB								// RGB LED in top right corner
#define CHIPSET WS2812								// FastLED library controls WS2812 RGB LED
#define NUM_LEDS 1
#define BRIGHTNESS 65

// Recommended PWM GPIO pins on the ESP32 include 2, 4, 12-19 ,21-23, 25-27, 32-33 
const short servoPin = 4;							// Servo set up on GPIO_4
const short PIR = 39;								// Digital Input for PIR sensor: GPIO_39
const short micSense = 34;							// audio sensor = GPIO_34 (A2)

int pos = 0;										// variable to store the servo position
char myKeyPress = 0;								// Sets a char as current user keypress
int digPIR = 0;										// 1: motion detected, 0: no motion
const byte ROWS = 4;								// set global constant as # of rows to the keypad
const byte COLS = 4;								// set global constant as # of columns to the keypad
char *password = "123A";							// declare pointer to char array and add password to array
unsigned short position = 0;						// number of correct keys entered
short armed = 0;
int theAlarm = 0;
unsigned noise = 0;
unsigned noiseTotal = 0;

CRGB leds[NUM_LEDS];								// Array for LEDS on GPIO_2
byte colPins[COLS] = {33, 15, 32, 14};				// declare pins of keypad as byte arrays
byte rowPins[ROWS] = {26, 25, 12, 27};

const char buttons[ROWS][COLS] = {					// declare keypad button as 2-D array
	{'1', '2', '3', 'A'},
  	{'4', '5', '6', 'B'},
  	{'7', '8', '9', 'C'},
	{'*', '0', '#', 'D'}
};

Servo servoObj;										// 16 servo objects can be created on the ESP32
Keypad theKeypad = Keypad(
	makeKeymap(buttons),
	rowPins,
	colPins,
	ROWS,
	COLS
);

void setLocked(int locked);

void setup()
{
  	Serial.begin(115200);
	delay(30);
  	pinMode(PIR, INPUT);							// GPIO_39 (Pin A3) Set as input for the PIR sensor
	
	FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  	FastLED.setBrightness( BRIGHTNESS );
	
	leds[0] = CRGB::White;							// Power up all Pin 2 LEDs
  	FastLED.show();
	delay(1500);									// 1.5 second power on delay
	leds[0] = CRGB::Black;							// Turn Off Pin 2 LEDs
  	FastLED.show();
  
	ESP32PWM::allocateTimer(0);						// Allow allocation of all timers
	ESP32PWM::allocateTimer(1);
	ESP32PWM::allocateTimer(2);
	ESP32PWM::allocateTimer(3);
	servoObj.setPeriodHertz(50);					// standard 50 hz servo
	servoObj.attach(servoPin, 1000, 2000);			// attaches the servo on pin 4 to the servo object

	// using default min/max of 1000us and 2000us
	// different servos may require different min/max settings
	// for an accurate 0 to 180 sweep
	// Serial.println("Servo is setup");

	setLocked(true);								// Lock Door Servo
	armed = 1;
}

void loop()
{
	myKeyPress = theKeypad.getKey();                // retrieve key pressed by user

  	if (myKeyPress == '*')							// only keypress we care about in disarmed state is '*'
  	{                                               // If the lock is open it can be relocked anytime with a '*' press
 		if (!armed)									// only beep when locking from unlocked state
    	{
			leds[0] = CRGB::Red;
			FastLED.show();
			delay(100);								// length of beep = 100 mS
			leds[0] = CRGB::Black;
			FastLED.show();
			delay(200);
    	}
		position = 0;								// reset password counter
		setLocked(true);							// move servo to 0 = LOCKED
		armed = 1;
	}
	
	if (armed)
	{
		digPIR = digitalRead(PIR);					// read value for PIR sensor = GPIO_39
		if (digPIR)									// if digPIR == 1 then there is motion
		{
			theAlarm = 1;
			//setPIR = 1;							// store PIR trip
		}

		for (int i = 0; i < 5; i++)
		{
			noiseTotal += analogRead(micSense);		// add up 5 total readings
			delay(20);								// delay 20 mSec
		}
		noise = (noiseTotal / 5);					// take average reading

		if (noise > 250)							// evaluate average reading
		{
			theAlarm = 1;							// set off the alarm
			//noisetrip = noise;					// store value which tripped alarm
			//noiseCtr++;
		}
		noise = 0;									// reset values so they don't overflow
		noiseTotal = 0;

		if (myKeyPress == password[position])
		{
			position++;								// if the digit matches: increase counter & move to next array index
		}

		if (position == 4)							// if 4 digits were entered correctly
		{
			for (int i = 0; i < 2; i++)
			{
				leds[0] = CRGB::Red;
				FastLED.show();
				delay(100);							// length of beep = 100 mS
				leds[0] = CRGB::Black;
				FastLED.show();
				delay(200);							// delay between beeps = 200 mS
			}
			setLocked(false);						// OPEN Sesame...
			armed = 0;
			theAlarm = 0;
			position = 0;							// reset counter back to ZERO
		}

	}

	if (theAlarm == 1)
	{
		leds[0] = CRGB::Red;						// Red = Alarm
		FastLED.show();
	}
}

void setLocked(int locked)							// function declaration for setLocked function
{
	if (!locked)									// if door is locked / closed
  	{
		leds[0] = CRGB::Green;						// Green = Go / Open
		FastLED.show();
		servoObj.write(180);						// unlock servo: move to position OPEN = 360
  	}
  	else // if (locked)
  	{
		leds[0] = CRGB::Blue;						// Blue = secure
		FastLED.show();
		servoObj.write(0);							// lock servo: move to position LOCKED = 0
  	}
}