#include <Arduino.h>
//#define PIN_REED 15
#define PIN_REED 19

void setupReed()
{
  pinMode(PIN_REED, INPUT_PULLUP);
}
bool getReed()
{
  return !digitalRead(PIN_REED);
}