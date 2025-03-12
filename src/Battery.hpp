
#define PIN_BAT 35
#define PIN_CHARGING 18
void initBattery(){
    pinMode(PIN_BAT, INPUT);
    pinMode(PIN_CHARGING, INPUT_PULLUP);
}
float readBattery(){
    float sum;
    int cnt = 100;
    for (int i = 0; i < cnt; i++)
    {
        sum+=analogRead(PIN_BAT);

    }
    return sum/cnt;    
}