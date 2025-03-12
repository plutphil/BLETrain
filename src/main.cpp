#include <Arduino.h>
#include "Motor.hpp"
#include "Battery.hpp"
#include "Sound.hpp"
#include "Light.hpp"

#include "BLEWrapper.hpp"
#include <BLE2902.h>
#include "Reed.hpp"
TaskHandle_t myTaskHandle = NULL;
void Demo_Task(void *arg)
{
    for(;;){
        float batval = readBattery();
        //batval = batval/4096.f*3.3f*10.f*1.176764077f;
        batval = batval*0.009480765269;
        printf("\rBatVal %0.2f    ",batval);
        characteristic->setValue(String(String("b")+batval).c_str());
        characteristic->notify();
        if (myDFPlayer.available()) {
            //Print the detail message from DFPlayer to handle different errors and states.
            printDetail(myDFPlayer.readType(), myDFPlayer.read()); 
        }
        for (size_t i = 0; i < 10; i++)
        {
            characteristic->setValue(
                "v"+std::to_string(pwmValue)+std::string("\n")+
                "r"+std::to_string(getReed())+std::string("\n")+
                "c"+std::to_string(digitalRead(PIN_CHARGING))
            );
            characteristic->notify();
            delay(100);
        }
    }
}

void setup() {
    setupReed();
    Serial.begin(115200);
    initMotor();
    initBattery();
    dfPlayerSetup();
    initLight();
    BLEDevice::init("ESP32");
    BLEServer *server = BLEDevice::createServer();

    server->setCallbacks(new ServerCallbacks());

    BLEService *service = server->createService(SERVICE_UUID);

    characteristic = service->createCharacteristic(CHARACTERISTIC_UUID, 
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE);
    //characteristic->addDescriptor(new BLE2902());
    // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
    // Create a BLE Descriptor
    characteristic->addDescriptor(new BLE2902());
    characteristic->setCallbacks(new CharacteristicCallbacks());

    service->start();

    advertising = BLEDevice::getAdvertising();
    advertising->addServiceUUID(SERVICE_UUID);
    advertising->setScanResponse(false);
    advertising->setMinPreferred(0x0);
    advertising->start();

    Serial.println("Waiting for clients to connect...");

    xTaskCreate(Demo_Task, "Demo_Task", 4096, NULL, 10, &myTaskHandle);
    
    enreverseonstop=allsettings[REVERTONSTOP]=0;
    hystEn=allsettings[HYST_EN]=0;
    hystVal=allsettings[HYST_VAL]=100;
    allsettings[SOUNDVOL]=20;
    allsettings[PLAYMOTORSOUND]=0;
    allsettings[MOTORSOUND]=2;
    allsettings[PENDEL]=1;

}
long lasttime = 0;
long pendeltimeout = 0;
long driveSpeed = 0;
void loop() {
    delay(100);
    long now = millis();
    float delta = (now-lasttime)/1000.f; // time in seconds since last tick
    lasttime = now;
    if(allsettings[PENDEL]){
        if(pendeltimeout<now){
            if(getReed()){
                pendeltriggered = true;
                pendeltimeout = now+3000;
                driveSpeed = targetPwm;
                targetPwm=0;
                if(!hystEn){
                    pwmValue=0;
                }
            }
        }
        if(pendeltriggered){
            if(pwmValue==0){
                reverse();
                characteristic->setValue("d"+std::to_string(dirstate));
                characteristic->notify();
                pendeltriggered=false;
            }
        }
    }
    if(hystEn){
        //printf("t %d v %f d %f\n",targetPwm,pwmValue,delta);
        if(pwmValue<targetPwm){
            pwmValue+=delta*hystVal;
            if(pwmValue>targetPwm){
                pwmValue = targetPwm;
            }
        }else if(pwmValue>targetPwm){
            pwmValue-=delta*hystVal;
            if(pwmValue<targetPwm){
                pwmValue = targetPwm;
            }
        }else{
        }
        setSpeed(pwmValue);
        //ledcWrite(PWM_CHAN, pwmValue);
    }
    
}