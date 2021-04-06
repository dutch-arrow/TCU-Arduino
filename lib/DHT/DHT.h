#ifndef DHT_H
#define DHT_H

#include "Arduino.h"

/*!
 *  @brief  Class that stores state and functions for DHT
 */
class DHT {
public:
  DHT(uint8_t pin);
  void begin(uint8_t usec = 55);
  float readTemperature();
  float readHumidity();
  bool read();

private:
  uint8_t data[5];
  uint8_t _pin;
#ifdef __AVR
  // Use direct GPIO access on an 8-bit AVR so keep track of the port and
  // bitmask for the digital pin connected to the DHT.  Other platforms will use
  // digitalRead.
  uint8_t _bit, _port;
#endif
  uint32_t _lastreadtime, _maxcycles;
  bool _lastresult;
  uint8_t pullTime; // Time (in usec) to pull up data line before reading

  uint32_t expectPulse(bool level);
};

/*!
 *  @brief  Class that defines Interrupt Lock Avaiability
 */
class InterruptLock {
public:
  InterruptLock() {
#if !defined(ARDUINO_ARCH_NRF52)
    noInterrupts();
#endif
  }
  ~InterruptLock() {
#if !defined(ARDUINO_ARCH_NRF52)
    interrupts();
#endif
  }
};

#endif

