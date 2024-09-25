#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#define bleServerName "ESP32_TrainController"
#define SERVICE_UUID "2669f374-8734-4f2e-8045-9e52781f078c"
#define CHARACTERISTIC_UUID "8466ab14-a8d5-4231-b0a6-630e59501bb8"
static BLECharacteristic *characteristic;
static BLEAdvertising *advertising;

uint8_t devicesConnected = 0;

const int pwmPin = 2;
int pwmValue = 0;

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
        if(cmd=='s'){
            pwmValue=inum;
            ledcWrite(PWM_CHAN, map(pwmValue,0,100,0,255));
        }else if(cmd=='d'){
            digitalWrite(PIN_DIR,inum);
        }else if(cmd=='h'){

        }else if(cmd=='p'){
            myDFPlayer.play(inum);
        }else if(cmd=='l'){
            digitalWrite(PIN_LIGHT,inum);
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
        motorStop();
    }
};