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

byte ledStripBrightness = 0;

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
	digitalWrite(PIN_LEDSTRIPE_DATA, 0);

	attachInterrupt(PIN_ROTENC_CLK, checkEncoder, CHANGE);
	attachInterrupt(PIN_ROTENC_DT, checkEncoder, CHANGE);

	stripe.init();
	stripe.fullStripeRGB(CRGB::OrangeRed);

	server.startWiFi();

	// Define HTTP Callbacks via callbacks
	server.connectRequestHandle("/", handleRoot);
	server.connectNotFoundHandle(handleNotFound);

	server.startMDNS();
	server.startServer();

	tCheckWifi.enable();

}

// Add the main program code into the continuous loop() function
void loop()
{
	stripe.update();
	tasker.execute();
	server.loop();
	encoderButton.tick();
}

#pragma region Peripherals
void checkEncoder()
{
	unsigned char result = encoder.process();
	if (result == DIR_CW) {
		ledStripBrightness++;
		stripe.setBrightness(ledStripBrightness);
		Serial.println(ledStripBrightness);
	}
	else if (result == DIR_CCW) {
		ledStripBrightness--;
		stripe.setBrightness(ledStripBrightness);
		Serial.println(ledStripBrightness);
	}
}

void buttonClicked()
{
	Serial.println("Click");
}

void buttonLongPress()
{
	ledStripBrightness = DEFAULT_LEDSTRIPE_BRIGHTNESS;
	stripe.setBrightness(ledStripBrightness);
	Serial.println("Reset Position");
}
#pragma endregion

#pragma region Server Handles
void handleRoot() {
	busyLED.toggle();
	server.server()->send(200, "text/plain", "WhiskyOne - Toggle Strip");
	stripe.toggle();
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
