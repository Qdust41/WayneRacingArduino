#include <SPI.h>
#include "Adafruit_MAX31855.h"

// MAX31855 SPI pins
#define MAXCLK 3 // Must be pwm (Max Clock)
#define MAXCS  4 // digital (Chipselect)
#define MAXDO  5 // Must be pwm (Digital Output)

// Counter clock pin These pins will have to be changed
#define COUNTER_CLK 11 //Must be pwm (Unknown)


Adafruit_MAX31855 thermocouple(MAXCLK, MAXCS, MAXDO);

int channel = 0;  // Channel index (0–3)

void setup() {
  Serial.begin(9600);
  pinMode(COUNTER_CLK, OUTPUT);
  digitalWrite(COUNTER_CLK, LOW);  
  delay(500); // Let everything settle
}

void loop() {

  digitalWrite(COUNTER_CLK, LOW);   
  delay(10);                        
  digitalWrite(COUNTER_CLK, HIGH);  
  delay(10);                        
  digitalWrite(COUNTER_CLK, LOW);   


  delay(200);  // 200 ms wait


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
