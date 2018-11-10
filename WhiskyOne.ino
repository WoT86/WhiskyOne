// Visual Micro is in vMicro>General>Tutorial Mode
// 
/*
	Name:       WhiskyOne.ino
	Created:	17.10.2018 20:13:24
	Author:     WOT-PC\WoT
*/

// Define User Types below here or use a .h file
//
// global includes



#include "Includes.h"
#include "Credentials.h"

#include "WhiskyStatusLED.h"
#include "WhiskyLEDStripe.h"
#include "WhiskyServer.h"

#include <TaskSchedulerDeclarations.h>
#include <TaskScheduler.h>

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>

#include <FastLED.h>

#include <Rotary.h>
#include <OneButton.h>



// Define Function Prototypes that use User Types below here or use a .h file
//

#pragma region global vars

CHSV currRotaryColor = CHSV(35, 255, 255);
CRGB currWifiColor = CRGB::OrangeRed;
volatile byte rotaryPosition = DEFAULT_LEDSTRIPE_BRIGHTNESS;
byte currBrightness = rotaryPosition;
byte rotaryMenu = 0;

WhiskyStatusLED errorLED(PIN_ERROR_LED);
WhiskyStatusLED busyLED(PIN_BUSY_LED);

Rotary encoder(PIN_ROTENC_CLK, PIN_ROTENC_DT);
OneButton encoderButton(PIN_ROTENC_SW, true);

// Non blocking Tasker
Scheduler tasker;

// Define LED Tasks with lambda function callbacks
Task tErrorBlink(500, TASK_FOREVER, []() {errorLED.toggle(); }, &tasker, false, NULL, []() {errorLED.turnOFF(); });
Task tBusyBlink(500, TASK_FOREVER, []() {busyLED.toggle(); }, &tasker, false, NULL, []() {busyLED.turnOFF(); });

// Class handles HTTP, MDNS and WIFI
WhiskyServer server(WIFI_SSID, WIFI_PASSWORD, 80, &tErrorBlink);

// Define WIFI Tasks with lambda function callbacks
Task tCheckWifi(10000, TASK_FOREVER, []() {server.checkWiFi(); }, &tasker);

// Define the LED Stripe
WhiskyLEDStripe stripe(LEDSTRIPE_LED_COUNT);

#pragma endregion

// The setup() function runs once each time the micro-controller starts
void setup()
{
	Serial.begin(9600);
	Serial.println("WhiskyOne - Initiliasing...");

	errorLED.init();
	busyLED.init();

	pinMode(PIN_LEDSTRIPE_DATA, OUTPUT);
	digitalWrite(PIN_LEDSTRIPE_DATA, LOW);

	attachInterrupt(PIN_ROTENC_CLK, checkEncoder, CHANGE);
	attachInterrupt(PIN_ROTENC_DT, checkEncoder, CHANGE);

	stripe.init();
	stripe.fullStripeRGB(CRGB::OrangeRed);
	stripe.setBrightness(rotaryPosition);

	server.startWiFi();

	// Define HTTP Callbacks via callbacks
	server.connectRequestHandle("/", handleRoot);
	server.connectRequestHandle("/fullStripe", handelFullStripe);
	server.connectNotFoundHandle(handleNotFound);

	//Encoder Button Callbacks
	encoderButton.attachClick(buttonClicked);
	encoderButton.attachLongPressStop(buttonLongPress);

	tCheckWifi.enable();

}

// Add the main program code into the continuous loop() function
void loop()
{
	stripe.update();
	tasker.execute();
	server.loop();
	encoderButton.tick();
	updateMenu();
}

#pragma region Peripherals
void checkEncoder()
{
	unsigned char result = encoder.process();
	if (result == DIR_CW) {	
		if(rotaryPosition < 255)
			rotaryPosition++;
		Serial.println(String("Rotary Encoder turned CW to " + String(rotaryPosition)));
	}
	else if (result == DIR_CCW) {
		if(rotaryPosition > 0)
			rotaryPosition--;
		Serial.println(String("Rotary Encoder turned CCW to " + String(rotaryPosition)));
	}
}

void updateMenu()
{
	switch (rotaryMenu)
	{
	case 0:	
		if (currBrightness != rotaryPosition)
		{
			Serial.println("Rotary Encoder turned CCW Brightness");
			currBrightness = rotaryPosition;
			stripe.setBrightness(rotaryPosition);
		}	
		break;
	case 1:
		if (currRotaryColor.h != rotaryPosition)
		{
			Serial.println("Rotary Encoder turned CCW Hue");
			currRotaryColor.h = rotaryPosition;
			stripe.fullStripeHSV(currRotaryColor);
		}
		break;
	case 2:
		if (currRotaryColor.s != rotaryPosition)
		{
			Serial.println("Rotary Encoder turned CCW Sat");
			currRotaryColor.s = rotaryPosition;
			stripe.fullStripeHSV(currRotaryColor);
		}
		break;
	case 3:	
		if (currRotaryColor.v != rotaryPosition)
		{
			Serial.println("Rotary Encoder turned CCW Val");
			currRotaryColor.v = rotaryPosition;
			stripe.fullStripeHSV(currRotaryColor);
		}
		break;
	}
}

void buttonClicked()
{
	Serial.println("Rotary Encoder clicked");
	// rolling menu for brightness, hue, saturation and value
	
	switch (rotaryMenu)
	{
	case 0:
		currBrightness = rotaryPosition;
		// set HSV Rotary Color
		stripe.fullStripeHSV(currRotaryColor);
		rotaryPosition = currRotaryColor.h;
		rotaryMenu++;
		break;
	case 1:
		rotaryPosition = currRotaryColor.s;
		rotaryMenu++;
		break;
	case 2:
		rotaryMenu++;
		rotaryPosition = currRotaryColor.v;
		break;
	default:
		rotaryMenu = 0;
		// change to Wifi color
		// HSV Rotary is stored as global
		stripe.fullStripeRGB(currWifiColor);
		rotaryPosition = currBrightness;
		break;
	}
}

void buttonLongPress()
{
	stripe.toggle();
	Serial.println("Rotary encoder - Stripe toggled");
}
#pragma endregion

#pragma region Server Handles
void handleRoot() {
	busyLED.toggle();
	server.server()->send(200, "text/plain", "WhiskyOne - Toggle Strip");
	stripe.toggle();
	busyLED.toggle();
}

void handelFullStripe()
{
	CRGB rgbcol;
	CHSV hsvcol;
	String temp = "";
	String mess = "";
	bool colchanged = false;

	busyLED.toggle();
	
	// RGB Interpreting first

	temp = server.server()->arg("r");

	if (temp.length() > 0)
	{
		rgbcol.red = temp.toInt();
		mess += "Red: " + temp;
		colchanged = true;
	}
	else
	{
		rgbcol.red = 0;
	}

	temp = server.server()->arg("g");

	if (temp.length() > 0)
	{
		rgbcol.green = temp.toInt();
		mess += "Green: " + temp;
		colchanged = true;
	}
	else
	{
		rgbcol.green = 0;
	}
		
	temp = server.server()->arg("b");

	if (temp.length() > 0)
	{
		rgbcol.blue = temp.toInt();
		mess += "Blue: " + temp;
		colchanged = true;
	}
	else
	{
		rgbcol.blue = 0;
	}

	// HSV only if no RGB is set

	if (!colchanged)
	{
		temp = server.server()->arg("h");

		if (temp.length() > 0)
		{
			hsvcol.hue = temp.toInt();
			mess += "Hue " + temp;
			colchanged = true;
		}
		else
		{
			hsvcol.hue = 0;
		}

		temp = server.server()->arg("s");

		if (temp.length() > 0)
		{
			hsvcol.sat = temp.toInt();
			mess += "Saturation: " + temp;
			colchanged = true;
		}
		else
		{
			hsvcol.sat = 0;
		}

		temp = server.server()->arg("v");

		if (temp.length() > 0)
		{
			hsvcol.val = temp.toInt();
			mess += "Value: " + temp;
			colchanged = true;
		}
		else
		{
			hsvcol.val = 0;
		}

		if (colchanged)
			stripe.fullStripeHSV(hsvcol);
	}
	else
		stripe.fullStripeRGB(rgbcol);

	if (!colchanged)
	{
		rgbcol = stripe.getCurrColor();

		mess += "Error no arguments recognized. No changes applied\n";
		mess += "Current Color -> Red: " + String(rgbcol.r) + " Green: " + String(rgbcol.g) + " Blue: " + String(rgbcol.b);
	}
	else
		currWifiColor = stripe.getCurrColor();
		

	mess = "WhiskyOne - FullStripe \n" + mess;

	server.server()->send(200, "text/plain", mess);
	busyLED.toggle();
}

void handleNotFound() {
	busyLED.toggle();
	String message = "File Not Found\n\n";
	message += "URI: ";
	message += server.server()->uri();
	message += "\nMethod: ";
	message += (server.server()->method() == HTTP_GET) ? "GET" : "POST";
	message += "\nArguments: ";
	message += server.server()->args();
	message += "\n";
	for (uint8_t i = 0; i < server.server()->args(); i++) {
		message += " " + server.server()->argName(i) + ": " + server.server()->arg(i) + "\n";
	}
	server.server()->send(404, "text/plain", message);
	busyLED.toggle();
}
#pragma endregion
