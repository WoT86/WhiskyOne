// WhiskyLEDStripe.h

#ifndef _WHISKYLEDSTRIPE_h
#define _WHISKYLEDSTRIPE_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#ifndef PIN_LEDSTRIPE_DATA
	#define PIN_LEDSTRIPE_DATA 5
#endif // !#define PIN_LEDSTRIPE_DATA

#ifndef DEFAULT_LEDSTRIPE_BRIGHTNESS
	#define DEFAULT_LEDSTRIPE_BRIGHTNESS 128
#endif // !DEFAULT_LEDSTRIPE_BRIGHTNESS

#ifndef FASTLED_ESP8266_MCU_PIN_ORDER
	#define FASTLED_ESP8266_MCU_PIN_ORDER
#endif // !FASTLED_ESP8266_MCU_PIN_ORDER


#include <FastLED.h>

class WhiskyLEDStripe
{
	typedef uint8_t PIN;

 public:
	 WhiskyLEDStripe(uint16_t numLEDs);
	 ~WhiskyLEDStripe();

	void init();

	void fullStripeRGB(CRGB color);
	void fullStripeHSV(CHSV color);

	void setBrightness(byte bright);
	void toggle();

	bool isOn();

	void update();

protected:
	CRGB*			m_LEDs;
	const PIN		m_dataPIN;
	bool			m_isReady;
	uint8_t			m_currBrightness;

	uint16_t m_LEDCount;
};

#endif

