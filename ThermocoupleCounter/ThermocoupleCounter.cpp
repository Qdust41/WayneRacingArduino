#include "ThermocoupleCounter.h"

ThermocoupleCounter::ThermocoupleCounter(int clkPin, int csPin, int doPin, int counterClkPin)
  : thermocouple(clkPin, csPin, doPin), counterClkPin(counterClkPin), channel(0) {}

void ThermocoupleCounter::begin() {
  pinMode(counterClkPin, OUTPUT);
  digitalWrite(counterClkPin, LOW);
  delay(500);
}

int ThermocoupleCounter::update() {
  digitalWrite(counterClkPin, LOW);
  delay(10);
  digitalWrite(counterClkPin, HIGH);
  delay(10);
  digitalWrite(counterClkPin, LOW);

  double temp = thermocouple.readCelsius();
  double internal = thermocouple.readInternal();
  uint8_t err = thermocouple.readError();

  if (err) {
    Serial.println(err);
  } else {
    return temp, internal;
  }
  Serial.println("--------------------");

  channel = (channel + 1) % 4;
}

