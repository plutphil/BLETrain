#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <string>
#define bleServerName "ESP32_TrainController"
#define SERVICE_UUID "2669f374-8734-4f2e-8045-9e52781f078c"
#define CHARACTERISTIC_UUID "8466ab14-a8d5-4231-b0a6-630e59501bb8"
static BLECharacteristic *characteristic;
static BLEAdvertising *advertising;

uint8_t devicesConnected = 0;

const int pwmPin = 2;
float pwmValue = 0;
bool hystEn = false;
int targetPwm = 0;
int hystVal = 10; // pwm steps per seconds 255 = 100%, 
bool enreverseonstop = false;
bool playadverts = false;
bool pendeltriggered = false;

enum {
    REVERTONSTOP,
    HYST_EN,
    HYST_VAL,
    SOUNDVOL,
    PLAYMOTORSOUND,
    MOTORSOUND,

    PENDEL,

    PLAYADVERTS,
    SOUND_SIG1,
    SOUND_SIG2,
    SOUND_SIG1ADV,
    SOUND_SIG2ADV
}SettingsEnum;
int allsettings[20]={0};

/*await sendCmd("e","0 "+Number(enhyst.checked));
await sendCmd("e","1 "+Number(hysteresis.value));
await sendCmd("e","2 "+Number(soundvol.value));
await sendCmd("e","3 "+Number(motorsound.checked));
await sendCmd("e","4 "+Number(revertlock.checked));
await sendCmd("e","5 "+Number(soundmotor.value));*/
class CharacteristicCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *characteristic) {
        std::string value = characteristic->getValue();
        //Serial.print("onWrite ");
        //Serial.print(value.c_str());
        //printf("%i %s\n",value.length(),value.c_str());
        auto numstr = value.substr(1);
        char cmd = value.at(0);
        int inum = atoi(numstr.c_str());
        printf("onWrite %c %d\n",cmd,inum);
        if(cmd=='h'){
            targetPwm = 0;
            pwmValue = 0;
        }else if(cmd=='s'){//speed
            if(!pendeltriggered)
            if(hystEn){
                targetPwm = inum;
            }else{
                targetPwm = inum;
                pwmValue = targetPwm;
                setSpeed(pwmValue);
                //ledcWrite(PWM_CHAN, pwmValue);
            }
        }else if(cmd=='d'){//direction
            if(!enreverseonstop||pwmValue==0){
                setdir(inum);
                if(pwmValue>0){// 
                    setSpeed(pwmValue);
                }
            }
            if(enreverseonstop&&pwmValue!=0){
                characteristic->setValue("d"+std::to_string(dirstate));
                characteristic->notify();
            }
        }else if(cmd=='p'){//play
            if(playadverts){
                myDFPlayer.advertise(inum);
            }else{
                myDFPlayer.play(inum);
            }
        }else if(cmd=='l'){//light
            digitalWrite(PIN_LIGHT,inum);
        }else if(cmd=='a'){//get allsettings
            std::string out = "";
            for (size_t i = 0; i < sizeof(allsettings)/sizeof(allsettings[0]); i++)
            {
                out+=" "+std::to_string(allsettings[i]);
            }
            characteristic->setValue("a"+out);
            characteristic->notify();
        }else if(cmd=='e'){//edit setting
            int spaceindex = numstr.find(' ');
            if(spaceindex>0){
                auto key = numstr.substr(0,spaceindex);
                auto val = numstr.substr(spaceindex+1);
                Serial.println(String("Set setting: '")+key.c_str()+"' : '"+val.c_str()+"'");
                int ikey = atoi(key.c_str());
                int ival = atoi(val.c_str());
                if(ikey>=0&&ikey<sizeof(allsettings)/sizeof(allsettings[0])){
                    allsettings[ikey]=ival;
                }
                if(val.empty()){
                    if(ikey==0){
                        val = std::to_string((int)hystEn);
                        characteristic->setValue("e"+std::to_string(ikey)+" "+val);
                        characteristic->notify();
                    }else if(ikey==1){
                        val = std::to_string((int)hystEn);
                        characteristic->setValue("e"+std::to_string(ikey)+" "+val);
                        characteristic->notify();
                    }
                    
                }
                if(ikey==REVERTONSTOP){
                    enreverseonstop = ival;
                }
                if(ikey==HYST_EN){
                    hystEn = ival;
                }
                if(ikey==HYST_VAL){
                    hystVal = ival;
                    if(hystVal<=0){
                        hystVal = 1;
                    }
                    else if(hystVal>100){
                        hystVal = 100;
                    }
                    
                }
                if(ikey==SOUNDVOL){
                    myDFPlayer.volume(ival);   
                }
                if(ikey==PLAYMOTORSOUND){
                    if(playmotorsound && !ival){
                        myDFPlayer.stop();
                    }
                    else if(!playmotorsound && ival){
                        myDFPlayer.loop(soundmotor);
                    }
                    playmotorsound=ival;
                }
                if(ikey==MOTORSOUND){
                    soundmotor = ival;
                    if(playmotorsound){
                        myDFPlayer.loop(soundmotor);
                    }
                }
                
            }
            
        }
        
    }

    void onRead(BLECharacteristic *characteristic) {
        Serial.println("onRead");
        characteristic->setValue("Hello");
    }
};

class ServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer *server) {
        Serial.println("Client connected");
        devicesConnected++;
        advertising->start();
    }

    void onDisconnect(BLEServer *server) {
        Serial.println("Client disconnected");
        devicesConnected--;
        pwmValue=0;
        targetPwm=0;
        motorStop();
    }
};