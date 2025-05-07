#ifndef THERMOCOUPLECOUNTER_H
#define THERMOCOUPLECOUNTER_H

#include <Arduino.h>
#include <Adafruit_MAX31855.h>

class ThermocoupleCounter {
public:
  ThermocoupleCounter(int clkPin, int csPin, int doPin, int counterClkPin);
  void begin();
  void update();
private:
  Adafruit_MAX31855 thermocouple;
  int counterClkPin;
  int channel;
};

#endif

