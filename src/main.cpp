#include <Arduino.h>
#include "Motor.hpp"
#include "Battery.hpp"
#include "Sound.hpp"
#include "Light.hpp"

#include "BLEWrapper.hpp"
#include <BLE2902.h>
void setup() {
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
}

void loop() {
    delay(1000);
    float batval = readBattery();

    printf("BatVal %0.2f\n",batval);
    characteristic->setValue(String(String("b")+batval).c_str());
    characteristic->notify();

    if (myDFPlayer.available()) {
        //Print the detail message from DFPlayer to handle different errors and states.
        printDetail(myDFPlayer.readType(), myDFPlayer.read()); 
    }
}