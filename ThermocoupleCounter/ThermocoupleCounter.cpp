#include "ThermocoupleCounter.h"

ThermocoupleCounter::ThermocoupleCounter(int clkPin, int csPin, int doPin, int counterClkPin)
  : thermocouple(clkPin, csPin, doPin), counterClkPin(counterClkPin), channel(0) {}

void ThermocoupleCounter::begin() {
  pinMode(counterClkPin, OUTPUT);
  digitalWrite(counterClkPin, LOW);
  delay(500);
}

void ThermocoupleCounter::update() {
  digitalWrite(counterClkPin, LOW);
  delay(10);
  digitalWrite(counterClkPin, HIGH);
  delay(10);
  digitalWrite(counterClkPin, LOW);

  delay(200);

  double temp = thermocouple.readCelsius();
  double internal = thermocouple.readInternal();
  uint8_t err = thermocouple.readError();

  Serial.print("Thermocouple ");
  Serial.print(channel);
  Serial.print(": ");
  if (err) {
    Serial.print("Error code ");
    Serial.println(err);
  } else {
    Serial.print(temp, 2);
    Serial.print(" °C | Internal: ");
    Serial.print(internal, 2);
    Serial.println(" °C");
  }
  Serial.println("--------------------");

  channel = (channel + 1) % 4;

  delay(500);
}

