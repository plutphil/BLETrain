
#define PIN_LIGHT 32
void initLight(){
    pinMode(PIN_LIGHT, OUTPUT);
    digitalWrite(PIN_LIGHT,LOW);
}