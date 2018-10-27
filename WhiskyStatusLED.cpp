#include "WhiskyStatusLED.h"


WhiskyStatusLED::WhiskyStatusLED() :
	m_isOn(false),
	m_isInit(false),
	m_pin(0)
{
}

WhiskyStatusLED::WhiskyStatusLED(PIN _pin):
	m_isOn(false),
	m_isInit(false),
	m_pin(_pin)
{
}

WhiskyStatusLED::~WhiskyStatusLED()
{
}

void WhiskyStatusLED::init()
{
	if (this->m_pin > 0)
	{
		pinMode(this->m_pin, OUTPUT);
		digitalWrite(this->m_pin, LOW);
		this->m_isInit = true;
	}
}

void WhiskyStatusLED::init(PIN _pin)
{
	this->m_pin = _pin;
	this->init();
}

void WhiskyStatusLED::toggle()
{
	if (this->m_isInit)
	{
		this->m_isOn = !(this->m_isOn);
		digitalWrite(this->m_pin, this->m_isOn);
	}
}

void WhiskyStatusLED::turnOFF()
{
	if (this->m_isOn)
	{
		digitalWrite(this->m_pin, LOW);
	}
}

void WhiskyStatusLED::turnON()
{
	if (this->m_isOn)
	{
		digitalWrite(this->m_pin, HIGH);
	}
}
