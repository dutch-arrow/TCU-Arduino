#include "DHT.h"

#define MIN_INTERVAL 2000  /**< min interval value */
#define TIMEOUT UINT32_MAX /**< Used programmatically for timeout. Not a timeout duration. Type: uint32_t. */

/*!
 *  @brief  Instantiates a new DHT class
 *  @param  pin    pin number that sensor is connected
 */
DHT::DHT(uint8_t pin) {
	_pin = pin;
	_bit = digitalPinToBitMask(pin);
	_port = digitalPinToPort(pin);
	_maxcycles = microsecondsToClockCycles(1000); // 1 millisecond timeout for reading pulses from DHT sensor.
}

/*!
 *  @brief  Setup sensor pins and set pull timings
 *  @param  usec Optionally pass pull-up time (in microseconds) before DHT reading
 *               starts. Default is 55 (see function declaration in DHT.h).
 */
void DHT::begin(uint8_t usec) {
	// set up the pins!
	pinMode(_pin, INPUT_PULLUP);
	// Using this value makes sure that millis() - lastreadtime will be
	// >= MIN_INTERVAL right away. Note that this assignment wraps around,
	// but so will the subtraction.
	_lastreadtime = millis() - MIN_INTERVAL;
	pullTime = usec;
}

/*!
 *  @brief  Read temperature
 *	@return Temperature value in Celsius
 */
float DHT::readTemperature() {
	float f = NAN;
	if (read()) {
		f = ((word)(data[2] & 0x7F)) << 8 | data[3];
		f *= 0.1;
		if (data[2] & 0x80) {
			f *= -1;
		}
	}
	return f;
}

/*!
 *  @brief  Read Humidity
 *	@return float value - humidity in percent
 */
float DHT::readHumidity() {
	float f = NAN;
	if (read()) {
		f = ((word)data[0]) << 8 | data[1];
		f *= 0.1;
	}
	return f;
}

/*!
 *  @brief  Read value from sensor.
 *	@return float value
 */
bool DHT::read() {
	// Check if sensor was read less than two seconds ago and return early
	// to use last reading.
	uint32_t currenttime = millis();
	if ((currenttime - _lastreadtime) < MIN_INTERVAL) {
		return _lastresult; // return last correct measurement
	}
	_lastreadtime = currenttime;
// Reset 40 bits of received data to zero.
	data[0] = data[1] = data[2] = data[3] = data[4] = 0;

#if defined(ESP8266)
	yield(); // Handle WiFi / reset software watchdog
#endif

	// Send start signal.  See DHT datasheet for full signal diagram:
	//   http://www.adafruit.com/datasheets/Digital%20humidity%20and%20temperature%20sensor%20AM2302.pdf

	// Go into high impedence state to let pull-up raise data line level and
	// start the reading process.
	pinMode(_pin, INPUT_PULLUP);
	delay(1);

	// First set data line low for a period according to sensor type
	pinMode(_pin, OUTPUT);
	digitalWrite(_pin, LOW);
	delayMicroseconds(1100); // data sheet says "at least 1ms"

	uint32_t cycles[80];
	{
		// End the start signal by setting data line high for 40 microseconds.
		pinMode(_pin, INPUT_PULLUP);

		// Delay a moment to let sensor pull data line low.
		delayMicroseconds(pullTime);

		// Now start reading the data line to get the value from the DHT sensor.

		// Turn off interrupts temporarily because the next sections
		// are timing critical and we don't want any interruptions.
		InterruptLock lock;

		// First expect a low signal for ~80 microseconds followed by a high signal
		// for ~80 microseconds again.
		if (expectPulse(LOW) == TIMEOUT) {
			_lastresult = false;
			return _lastresult;
		}
		if (expectPulse(HIGH) == TIMEOUT) {
			_lastresult = false;
			return _lastresult;
		}

		// Now read the 40 bits sent by the sensor.  Each bit is sent as a 50
		// microsecond low pulse followed by a variable length high pulse.  If the
		// high pulse is ~28 microseconds then it's a 0 and if it's ~70 microseconds
		// then it's a 1.  We measure the cycle count of the initial 50us low pulse
		// and use that to compare to the cycle count of the high pulse to determine
		// if the bit is a 0 (high state cycle count < low state cycle count), or a
		// 1 (high state cycle count > low state cycle count). Note that for speed
		// all the pulses are read into a array and then examined in a later step.
		for (int i = 0; i < 80; i += 2) {
			cycles[i] = expectPulse(LOW);
			cycles[i + 1] = expectPulse(HIGH);
		}
	} // Timing critical code is now complete.

	// Inspect pulses and determine which ones are 0 (high state cycle count < low
	// state cycle count), or 1 (high state cycle count > low state cycle count).
	for (int i = 0; i < 40; ++i) {
		uint32_t lowCycles = cycles[2 * i];
		uint32_t highCycles = cycles[2 * i + 1];
		if ((lowCycles == TIMEOUT) || (highCycles == TIMEOUT)) {
			_lastresult = false;
			return _lastresult;
		}
		data[i / 8] <<= 1;
		// Now compare the low and high cycle times to see if the bit is a 0 or 1.
		if (highCycles > lowCycles) {
			// High cycles are greater than 50us low cycle count, must be a 1.
			data[i / 8] |= 1;
		}
		// Else high cycles are less than (or equal to, a weird case) the 50us low
		// cycle count so this must be a zero.  Nothing needs to be changed in the
		// stored data.
	}

	// Check we read 40 bits and that the checksum matches.
	if (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {
		_lastresult = true;
		return _lastresult;
	} else {
		_lastresult = false;
		return _lastresult;
	}
}

// Expect the signal line to be at the specified level for a period of time and
// return a count of loop cycles spent at that level (this cycle count can be
// used to compare the relative time of two pulses).  If more than a millisecond
// ellapses without the level changing then the call fails with a 0 response.
// This is adapted from Arduino's pulseInLong function (which is only available
// in the very latest IDE versions):
//   https://github.com/arduino/Arduino/blob/master/hardware/arduino/avr/cores/arduino/wiring_pulse.c
uint32_t DHT::expectPulse(bool level) {
#if (F_CPU > 16000000L)
	uint32_t count = 0;
#else
	uint16_t count = 0; // To work fast enough on slower AVR boards
#endif
// On AVR platforms use direct GPIO port access as it's much faster and better
// for catching pulses that are 10's of microseconds in length:
#ifdef __AVR
	uint8_t portState = level ? _bit : 0;
	while ((*portInputRegister(_port) & _bit) == portState) {
		if (count++ >= _maxcycles) {
			return TIMEOUT; // Exceeded timeout, fail.
		}
	}
// Otherwise fall back to using digitalRead (this seems to be necessary on
// ESP8266 right now, perhaps bugs in direct port access functions?).
#else
	while (digitalRead(_pin) == level) {
		if (count++ >= _maxcycles) {
			return TIMEOUT; // Exceeded timeout, fail.
		}
	}
#endif

	return count;
}
