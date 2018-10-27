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

#include "WhiskyStatusLED.h"

#include <TaskSchedulerDeclarations.h>
#include <TaskScheduler.h>

#include <FastLED.h>

#include <Rotary.h>
#include <OneButton.h>

#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>


// Define Function Prototypes that use User Types below here or use a .h file
//

#pragma region global vars

WhiskyStatusLED errorLED(PIN_ERROR_LED);
WhiskyStatusLED busyLED(PIN_BUSY_LED);

byte ledStripBrightness = DEFAULT_LEDSTRIP_BRIGHTNESS;

Rotary encoder(PIN_ROTENC_CLK, PIN_ROTENC_DT);
OneButton encoderButton(PIN_ROTENC_SW, true);

const char* ssid = "........";
const char* password = "........";

ESP8266WebServer server(80);


// Non blocking Tasker
Scheduler tasker;

// Define Tasks with lambda function callbacks
Task tErrorBlink(500, TASK_FOREVER, []() {errorLED.toggle(); }, &tasker, false, NULL, []() {errorLED.turnOFF(); });
Task tBusyBlink(500, TASK_FOREVER, []() {busyLED.toggle(); }, &tasker, false, NULL, []() {busyLED.turnOFF(); });

Task tCheckWifi(10000, TASK_FOREVER, &CheckWiFiStatus, &tasker);

#pragma endregion

// The setup() function runs once each time the micro-controller starts
void setup()
{
	Serial.begin(9600);
	Serial.println("WhiskyOne - Initiliasing...");

	errorLED.init();
	busyLED.init();

	pinMode(PIN_LEDSTRIP_DATA, OUTPUT);
	digitalWrite(PIN_LEDSTRIP_DATA, 0);

	attachInterrupt(PIN_ROTENC_CLK, checkEncoder, CHANGE);
	attachInterrupt(PIN_ROTENC_DT, checkEncoder, CHANGE);

	Serial.print("Connecting to WiFi...");
	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);

	// Wait for connection
	while (WiFi.status() != WL_CONNECTED) {
		errorLED.toggle();
		delay(500);
		errorLED.toggle();
		Serial.print(".");
	}

	Serial.println("");
	Serial.print("Connected to ");
	Serial.println(ssid);
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());

	if (MDNS.begin("WhiskyOne")) {
		Serial.println("MDNS responder started");
	}

	server.on("/", handleRoot);

	server.onNotFound(handleNotFound);

	server.begin();
	Serial.println("HTTP server started");

	tCheckWifi.enable();

}

// Add the main program code into the continuous loop() function
void loop()
{
	tasker.execute();
	server.handleClient();
	encoderButton.tick();
}

void CheckWiFiStatus()
{
	if (WiFi.status() != WL_CONNECTED)
	{
		if (!tErrorBlink.isEnabled())
		{
			Serial.println("Connection to WiFi lost!");
			tErrorBlink.enable();
		}
		
	}
	else
	{ 
		if (tErrorBlink.isEnabled())
		{
			Serial.println("Connection to WiFi reestablished!");
			tErrorBlink.disable();
		}
	}
		
}

#pragma region Peripherals
void checkEncoder()
{
	unsigned char result = encoder.process();
	if (result == DIR_CW) {
		ledStripBrightness++;
		Serial.println(ledStripBrightness);
	}
	else if (result == DIR_CCW) {
		ledStripBrightness--;
		Serial.println(ledStripBrightness);
	}
}

void buttonClicked()
{
	Serial.println("Click");
}

void buttonLongPress()
{
	ledStripBrightness = DEFAULT_LEDSTRIP_BRIGHTNESS;
	Serial.println("Reset Position");
}
#pragma endregion

#pragma region Server Handles
void handleRoot() {
	busyLED.toggle();
	server.send(200, "text/plain", "WhiskyOne - Toggle Strip");
	busyLED.toggle();
}

void handleNotFound() {
	busyLED.toggle();
	String message = "File Not Found\n\n";
	message += "URI: ";
	message += server.uri();
	message += "\nMethod: ";
	message += (server.method() == HTTP_GET) ? "GET" : "POST";
	message += "\nArguments: ";
	message += server.args();
	message += "\n";
	for (uint8_t i = 0; i < server.args(); i++) {
		message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
	}
	server.send(404, "text/plain", message);
	busyLED.toggle();
}
#pragma endregion
