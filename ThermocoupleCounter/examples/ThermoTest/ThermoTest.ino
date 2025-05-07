#include <ThermocoupleCounter.h>

ThermocoupleCounter tc(3, 4, 5, 11);

void setup() {
  Serial.begin(9600);
  tc.begin();
}

void loop() {
  tc.update();
}

