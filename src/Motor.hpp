#define PIN_PWM 18
//#define PIN_PWM 2

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
bool dirstate = 0;
void setdir(int dir){
  dirstate = dir;
  digitalWrite(PIN_DIR,dirstate);
}
void reverse(){
  dirstate = !dirstate;
  digitalWrite(PIN_DIR,dirstate);
}
void foreward(){
  digitalWrite(PIN_DIR,0);
  dirstate = 0;
}
void backWards(){
  digitalWrite(PIN_DIR,1);
  dirstate = 1;
}
