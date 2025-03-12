//#define PIN_LIGHT 32
#define PIN_LIGHT 16

void initLight(){
    pinMode(PIN_LIGHT, OUTPUT);
    digitalWrite(PIN_LIGHT,LOW);
}