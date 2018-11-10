#include "WhiskyServer.h"

WhiskyServer::WhiskyServer(const String& ssid, const String& pswd, int port, Task* errorBlink, Task* busyBlink) :
	m_wifiSSID(ssid),
	m_wifiPassword(pswd),
	m_deviceName("WhiskyOne"),
	m_httpServer(port),
	m_firstConnect(true),
	m_wifiLost(false),
	m_tErrorBlink(errorBlink),
	m_tBusyBlink(busyBlink)
{
}

WhiskyServer::~WhiskyServer()
{
}

void WhiskyServer::startWiFi(String ssid, String pswd)
{
	if (ssid.length() > 0)
		this->m_wifiSSID = ssid;

	if (pswd.length() > 0)
		this->m_wifiPassword = pswd;

	this->toSerial("Connecting to WiFi...");

	WiFi.mode(WIFI_STA);
	WiFi.begin(this->m_wifiSSID.c_str(), this->m_wifiPassword.c_str());

	if (this->m_tErrorBlink)
		this->m_tErrorBlink->enable();
}

void WhiskyServer::startMDNS(String name)
{
	if (name.length() > 0)
	{
		this->m_deviceName = name;
	}

	if (MDNS.begin(this->m_deviceName.c_str())) {
		this->toSerial(String("MDNS responder started sending " + this->m_deviceName).c_str());
	}
	else
	{
		this->toSerial("MDNS responder failed to started");
	}
}

void WhiskyServer::connectRequestHandle(const String& uri, ServerHandle handle)
{
	this->m_httpServer.on(uri, handle);
}

void WhiskyServer::connectNotFoundHandle(ServerHandle handle)
{
	this->m_httpServer.onNotFound(handle);
}

void WhiskyServer::startServer()
{
	this->m_httpServer.begin();
	this->toSerial("HTTP server started");
}

void WhiskyServer::checkWiFi()
{
	// called everytime you want to check the connection
	// not available if no blink task assigned

	if (this->m_tErrorBlink == NULL)
		return;

	if (WiFi.status() != WL_CONNECTED)
	{
		if (this->m_firstConnect)
		{
			this->toSerial("Waiting for WiFi...");
		}
		else if (!this->m_tErrorBlink->isEnabled())
		{
			this->toSerial("Connection to WiFi lost!");
			this->m_tErrorBlink->enable();
		}

	}
	else
	{
		if (this->m_firstConnect)
		{
			//First Connection -> transmit some info via serial
			String con = String("Connected to " + this->m_wifiSSID);
			this->toSerial("");
			this->toSerial(con.c_str());
			con = String("IP address: " + WiFi.localIP().toString());
			this->toSerial(con.c_str());

			this->m_tErrorBlink->disable();
			this->m_firstConnect = false;
			this->startMDNS();
			this->startServer();
		}
		else if (this->m_tErrorBlink->isEnabled())
		{
			this->toSerial("Connection to WiFi reestablished!");
			this->m_tErrorBlink->disable();
		}
	}
}

void WhiskyServer::loop()
{
	this->m_httpServer.handleClient();
}

ESP8266WebServer * WhiskyServer::server()
{
	return &this->m_httpServer;
}

void WhiskyServer::toSerial(const char * str)
{
	if (Serial)
		Serial.println(str);
}
