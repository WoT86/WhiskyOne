// 
// 
// 

#include "WhiskyLEDStripe.h"

WhiskyLEDStripe::WhiskyLEDStripe(uint16_t numLEDs):
	m_dataPIN(PIN_LEDSTRIPE_DATA),
	m_LEDCount(numLEDs),
	m_isReady(false),
	m_currBrightness(0)
{
	this->m_LEDs = (this->m_LEDCount > 0) ? new CRGB[numLEDs] : NULL;	
}

WhiskyLEDStripe::~WhiskyLEDStripe()
{
	if (m_LEDs)
		delete this->m_LEDs;
}

void WhiskyLEDStripe::init()
{
	if (this->m_LEDs)
	{
		FastLED.addLeds<NEOPIXEL, PIN_LEDSTRIPE_DATA>(this->m_LEDs, this->m_LEDCount);
		FastLED.setBrightness(DEFAULT_LEDSTRIPE_BRIGHTNESS);
		this->m_currBrightness = DEFAULT_LEDSTRIPE_BRIGHTNESS;
		FastLED.clear();
		FastLED.show();
		this->m_isReady = true;
	}
}

void WhiskyLEDStripe::fullStripeRGB(CRGB color)
{
	if (this->m_isReady)
	{
		for (uint16_t i = 0; i < this->m_LEDCount; i++)
			this->m_LEDs[i] = color;

		FastLED.show();
	}
}

void WhiskyLEDStripe::fullStripeHSV(CHSV color)
{
	if (this->m_isReady)
	{
		for (uint16_t i = 0; i < this->m_LEDCount; i++)
			this->m_LEDs[i] = color;

		FastLED.show();
	}
}

void WhiskyLEDStripe::setBrightness(byte bright)
{
	if (this->m_isReady)
	{
		FastLED.setBrightness(bright);
		this->m_currBrightness = bright;
		FastLED.show();
	}
}

void WhiskyLEDStripe::toggle()
{
	if (FastLED.getBrightness() == 0)
	{
		if (this->m_currBrightness == 0)
			this->m_currBrightness = DEFAULT_LEDSTRIPE_BRIGHTNESS;
		
		FastLED.setBrightness(this->m_currBrightness);
	}
	else
	{
		FastLED.setBrightness(0);
	}
}

bool WhiskyLEDStripe::isOn()
{
	return FastLED.getBrightness() > 0;
}

void WhiskyLEDStripe::update()
{
	FastLED.show();
}

