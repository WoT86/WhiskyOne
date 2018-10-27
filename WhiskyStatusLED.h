#pragma once

#include "Arduino.h"

class WhiskyStatusLED
{
public:
	typedef uint8_t PIN;

public:

	WhiskyStatusLED();
	WhiskyStatusLED(PIN _pin);
	~WhiskyStatusLED();

	void init();
	void init(PIN _pin);

	void toggle();
	void turnOFF();
	void turnON();
protected:
	bool m_isOn;
	bool m_isInit;
	PIN m_pin;
};

