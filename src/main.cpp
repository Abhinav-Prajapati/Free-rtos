#include <Arduino.h>
#define led_pin 12

void setup() {
  pinMode(led_pin, OUTPUT);
}

void loop() {
  digitalWrite(led_pin, HIGH);
  delay(1000)
  digitalWrite(led_pin, LOW);
  delay(1000)
}