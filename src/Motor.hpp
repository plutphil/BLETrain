#define PIN_PWM 18
#define PWM_CHAN 1
#define PIN_DIR 19

void initMotor(){
  pinMode(PIN_PWM,OUTPUT);
  digitalWrite(PIN_PWM,0);
  
  ledcSetup(PWM_CHAN, 1000, 8);
  ledcAttachPin(PIN_PWM,PWM_CHAN);
  ledcWrite(PWM_CHAN, 0);

  pinMode(PIN_DIR,OUTPUT);
  digitalWrite(PIN_DIR,0);
}
void loopMotor(){

}
void motorStop(){
    ledcWrite(PWM_CHAN, 0);
    digitalWrite(PIN_DIR,0);
}
