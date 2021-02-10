/*
  Set a baud rate to 115200bps over BLE

  Adding descriptor
  https://github.com/espressif/arduino-esp32/blob/master/libraries/BLE/src/BLECharacteristic.h
  https://gist.github.com/heiko-r/f284d95141871e12ca0164d9070d61b4
  Roughly working Characteristic descriptor: https://github.com/espressif/arduino-esp32/issues/1038

  Float to string: https://iotbyhvm.ooo/esp32-ble-tutorials/

  Custom UUIDs? https://www.bluetooth.com/specifications/assigned-numbers/

  ESP32 with chrome: https://github.com/kpatel122/ESP32-Web-Bluetooth-Terminal/blob/master/ESP32-BLE/ESP32-BLE.ino
*/
#include <string>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

#include "ESP32_BLE_Config.h"

#include "sf_ble_prop.h"


#define LED_BUILTIN 13
// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define kTargetServiceUUID  "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define kTargetServiceName  "ESP32 App"
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Property Creation and Management
//
// Dynamically create and manage properties
//--------------------------------------------------------------------------------------



// Our Characteristic UUIDs - and yes, just made these up
#define kCharacteristicBaudUUID     "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define kCharacteristicEnabledUUID  "beb5483e-36e1-4688-b7f5-ea07361b26a9"
#define kCharacteristicMessageUUID  "beb5483e-36e1-4688-b7f5-ea07361b26aa"
#define kCharacteristicSampleUUID   "beb5483e-36e1-4688-b7f5-ea07311b260b"
#define kCharacteristicDateUUID     "beb5483e-36e1-4688-b7f5-ea07311b260c"
#define kCharacteristicTimeUUID     "beb5483e-36e1-4688-b7f5-ea07311b260d"
#define kCharacteristicOffsetUUID     "beb5483e-36e1-4688-b7f5-ea07311b260e"


// PROPERTY Data/Local Variables
uint32_t baudRate = 115200;

bool deviceEnabled = true;

// message prop
std::string strMessage("Welcome");

// sample rate prop (a range)
uint32_t sampleRate = 123;
uint32_t sampleRateMin = 10;
uint32_t sampleRateMax = 240;

// Date prop - date is a string "YYYY-MM-DD"
std::string strDate("2021-03-01");

// Time prop - date is a string "HH:MM" - 
std::string strTime("2:5"); // test value - will be parsed as 02:05

// A float property - "offset value"
float offsetValue = 4.124;

BLECharacteristic *pCharOffset; // will use later
unsigned long ticks; // used for notification

bool deviceConnected = false;


class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      Serial.println("Server Connect!");
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      Serial.println("Server Disconnect");
      deviceConnected = false;
    }
};

//4 bytes come in but they are little endian. Flip them around.
//Convert a std string to a int
int32_t stringToValue(std::string myString)
{
  int newValue = 0;
  for (int i = myString.length() ; i > 0 ; i--)
  {
    newValue <<= 8;
    newValue |= (myString[i - 1]);
  }

  return (newValue);
}

//---------------------------------------------------------------------------------
// Enabled Characterisitic 
//
// A Bool Characterisitic (property) example

void onEnabledUpdate(bool newValue){

    Serial.print("Update Enabled Value: "); 
    Serial.println(deviceEnabled);
    deviceEnabled = newValue;
    digitalWrite(LED_BUILTIN, (deviceEnabled ? HIGH : LOW));    
}
BLECharacteristic * setupEnabledCharacteristic(BLEService *pService){

    BLECharacteristic *pCharBaud;

    pCharBaud = pService->createCharacteristic(
                            kCharacteristicEnabledUUID,
                            BLECharacteristic::PROPERTY_READ  |
                            BLECharacteristic::PROPERTY_WRITE |
                            BLECharacteristic::PROPERTY_NOTIFY );

    // Set the value in the callbacks 
    pCharBaud->setCallbacks(new BoolPropertyValueCB(&deviceEnabled, onEnabledUpdate));

    sf_bleprop_bool(pCharBaud, "Device Enabled");
    
    return pCharBaud;
}


//---------------------------------------------------------------------------------
// Baud Rate Characterisitic 
//
// A Integer Characterisitic (property) example

void onBaudUpdate(int32_t newValue){

    Serial.print("Update Baud Value: "); 
    Serial.println(baudRate);
}
BLECharacteristic * setupBaudCharacteristic(BLEService *pService){

    BLECharacteristic *pCharBaud;

    pCharBaud = pService->createCharacteristic(
                            kCharacteristicBaudUUID,
                            BLECharacteristic::PROPERTY_READ  |
                            BLECharacteristic::PROPERTY_WRITE |
                            BLECharacteristic::PROPERTY_NOTIFY );

    // Set the value in the callbacks 
    pCharBaud->setCallbacks(new IntPropertyValueCB((int32_t*)&baudRate, onBaudUpdate));

    sf_bleprop_int(pCharBaud, "Output Baud Value");
    
    return pCharBaud;
}
//---------------------------------------------------------------------------------
// Message Characterisitic 
//
// A String Characterisitic (property) example

void onMessageUpdate(std::string &  newValue){

    Serial.print("Update Message Value: "); 
    Serial.println(newValue.c_str());
}
BLECharacteristic * setupMessageCharacteristic(BLEService *pService){

    BLECharacteristic *pCharBaud;

    pCharBaud = pService->createCharacteristic(
                            kCharacteristicMessageUUID,
                            BLECharacteristic::PROPERTY_READ  |
                            BLECharacteristic::PROPERTY_WRITE |
                            BLECharacteristic::PROPERTY_NOTIFY );

    // Set the value in the callbacks 
    pCharBaud->setCallbacks(new TextPropertyValueCB(strMessage, onMessageUpdate));

    sf_bleprop_string(pCharBaud, "Device Message");

    return pCharBaud;
}


//---------------------------------------------------------------------------------
// Sample Rate Characterisitic 
//
// A Integer Range Characterisitic (property) example

void onSampleRateUpdate(int32_t newValue){

    Serial.print("Update Sample Value: "); 
    Serial.println(sampleRate);
}
BLECharacteristic * setupSampleRateCharacteristic(BLEService *pService){


    BLECharacteristic *pCharRate;

    pCharRate = pService->createCharacteristic(
                            kCharacteristicSampleUUID,
                            BLECharacteristic::PROPERTY_READ  |
                            BLECharacteristic::PROPERTY_WRITE |
                            BLECharacteristic::PROPERTY_NOTIFY );

    // Set the value in the callbacks - can just use the int callbacks
    pCharRate->setCallbacks(new IntPropertyValueCB((int32_t*)&sampleRate, onSampleRateUpdate));

    sf_bleprop_range(pCharRate, "Sample Rate (sec)", sampleRateMin, sampleRateMax);

    return pCharRate;
}

//---------------------------------------------------------------------------------
// Date Characterisitic 
//
// A Date Characterisitic (property) example - format is a string "YYYY-MM-DD"

void onDateUpdate(std::string &  newValue){

    Serial.print("Update Date Value: "); 
    Serial.println(newValue.c_str());
}
BLECharacteristic * setupDateCharacteristic(BLEService *pService){

    BLECharacteristic *pCharDate;

    pCharDate = pService->createCharacteristic(
                            kCharacteristicDateUUID,
                            BLECharacteristic::PROPERTY_READ  |
                            BLECharacteristic::PROPERTY_WRITE |
                            BLECharacteristic::PROPERTY_NOTIFY );

    // Set the value in the callbacks 
    pCharDate->setCallbacks(new TextPropertyValueCB(strDate, onDateUpdate));

    sf_bleprop_date(pCharDate, "Start Date");


    return pCharDate;   
}
//---------------------------------------------------------------------------------
// Time Characterisitic 
//
// A Time Characterisitic (property) example - format is a string "HH:MM"

void onTimeUpdate(std::string &  newValue){

    Serial.print("Update Time Value: "); 
    Serial.println(newValue.c_str());
}
BLECharacteristic * setupTimeCharacteristic(BLEService *pService){

    BLECharacteristic *pCharTime;

    pCharTime = pService->createCharacteristic(
                            kCharacteristicTimeUUID,
                            BLECharacteristic::PROPERTY_READ  |
                            BLECharacteristic::PROPERTY_WRITE |
                            BLECharacteristic::PROPERTY_NOTIFY );

    // Set the value in the callbacks 
    pCharTime->setCallbacks(new TextPropertyValueCB(strTime, onTimeUpdate));

    sf_bleprop_time(pCharTime, "Start Time");

    return pCharTime;
}

//---------------------------------------------------------------------------------
// FLoat Characterisitic 
//
// A float Characterisitic (property) example 

void onOffsetUpdate(float newValue){

    Serial.print("Update Offset(float) Value: "); 
    Serial.println(newValue);
}
BLECharacteristic * setupOffsetCharacteristic(BLEService *pService){

    BLECharacteristic *pCharOff;

    pCharOff = pService->createCharacteristic(
                            kCharacteristicOffsetUUID,
                            BLECharacteristic::PROPERTY_READ  |
                            BLECharacteristic::PROPERTY_WRITE |
                            BLECharacteristic::PROPERTY_NOTIFY );

    // Set the value in the callbacks 
    pCharOff->setCallbacks(new FloatPropertyValueCB(&offsetValue, onOffsetUpdate));

    sf_bleprop_float(pCharOff, "Offset Bias");

    return pCharOff;

}
//---------------------------------------------------------------------------------

void setup() {
    Serial.begin(115200);
    Serial.println("Starting BLE work!");

    BLEDevice::init(kTargetServiceName);

    BLEServer *pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    // >>> NOTE <<<
    // When creating a service. The default - when you just pass in a UUID,
    // only creates 15 handles. This isn't enough to support the 4 characteristics 
    // in this demo. Setting this to 20 allows for 4 Characteristics.
    // see: https://github.com/nkolban/ESP32_BLE_Arduino/blob/master/src/BLEServer.cpp
    BLEService *pService = pServer->createService(BLEUUID(kTargetServiceUUID), 35, 1);

    //Setup characterstics

    BLECharacteristic *pChar;

    setupEnabledCharacteristic(pService);

    pChar = setupDateCharacteristic(pService);
    sf_bleprop_group(pChar, "Event Details");         
    setupTimeCharacteristic(pService); 

    pChar = setupBaudCharacteristic(pService);
    sf_bleprop_group(pChar, "Device Settings");     
    setupMessageCharacteristic(pService);


    pChar = setupSampleRateCharacteristic(pService);
    sf_bleprop_group(pChar, "Sensor Settings");     
    pCharOffset = setupOffsetCharacteristic(pService);  
    // On ESP32 - to enable notification, you need to add a special descriptor
    pCharOffset->addDescriptor(new BLE2902());


    pService->start();

    //Begin broadcasting

    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    // NOTE: this will broadcast service so it's discoverable before 
    //       device connection. Helps when using BLE scanner
    pAdvertising->addServiceUUID(kTargetServiceUUID);
    pAdvertising->setScanResponse(false);
    pAdvertising->setMinPreferred(0x00);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, (deviceEnabled ? HIGH : LOW));

    ticks = millis();
    Serial.println("BLE Started");
}

void loop() {

    delay(200);

    if(millis() - ticks > 5000){
        // Update the value of offset and set in BLE char
        // Should trigger a notification on client

        offsetValue += .5;
        if(deviceConnected){
            pCharOffset->setValue(offsetValue);
            pCharOffset->notify();
            Serial.print("Incrementing Offset to: "); Serial.println(offsetValue);
        }
        ticks = millis();
    }

}
