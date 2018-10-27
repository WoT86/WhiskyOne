#pragma once

#include <Arduino.h>

#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>

#include <TaskSchedulerDeclarations.h>

class WhiskyServer
{
public:
	typedef std::function<void()> ServerHandle;
public:
	WhiskyServer(const String& ssid, const String& pswd, int port=80, Task* errorBlink = NULL);
	~WhiskyServer();

	void startWiFi(String ssid = "", String pswd = "");
	void startMDNS(String name = "");
	void connectRequestHandle(const String& uri, ServerHandle handle);
	void connectNotFoundHandle(ServerHandle handle);
	void startServer();
	void checkWiFi();
	void loop();

	//really dirty but quick
	ESP8266WebServer* server();

protected:
	String m_wifiSSID;
	String m_wifiPassword;
	String m_deviceName;

	ESP8266WebServer m_httpServer;

	bool m_firstConnect;
	bool m_wifiLost;

	Task* m_tErrorBlink;

protected:
	void toSerial(const char* str);
};
