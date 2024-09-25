
#define PIN_BAT 35

void initBattery(){
    pinMode(PIN_BAT, INPUT);
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