/*
 *
 * SparkFun ESP32 BLE Settings App Example
 * =======================================
 *
 * [TODO - Descripton, attribution and license]
 *
 */
#include <string>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// Include BLE2902 - used to enable BLE notifications on a Characterisitcs.
#include <BLE2902.h>

#include "ESP32_BLE_Config.h"

// Include SparkFun functions to define "properties" out of characteristics
// This enables use of the SparkFun BLE Settings Web-App
#include "sf_ble_prop.h"

#define LED_BUILTIN 13

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define kTargetServiceUUID  "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
// Name of this app/service
#define kTargetServiceName  "ESP32 App"

//--------------------------------------------------------------------------------------
// Our Characteristic UUIDs - and yes, just made these up
//--------------------------------------------------------------------------------------
#define kCharacteristicBaudUUID         "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define kCharacteristicEnabledUUID      "beb5483e-36e1-4688-b7f5-ea07361b26a9"
#define kCharacteristicSSIDUUID         "beb5483e-36e1-4688-b7f5-ea07361b26aa"
#define kCharacteristicPasswordUUID     "beb5483e-36e1-4688-b7f5-ea07361b26ab"
#define kCharacteristicSampleUUID       "beb5483e-36e1-4688-b7f5-ea07311b260b"
#define kCharacteristicDateUUID         "beb5483e-36e1-4688-b7f5-ea07311b260c"
#define kCharacteristicTimeUUID         "beb5483e-36e1-4688-b7f5-ea07311b260d"
#define kCharacteristicOffsetUUID       "beb5483e-36e1-4688-b7f5-ea07311b260e"

//-------------------------------------------------------------------------
// BLE Characteristics require persistant values for data storage. Nothing stack-based.
//-------------------------------------------------------------------------
//
// The following define the values (property values) used in this example

// PROPERTY Data/Local Variables
uint32_t baudRate = 115200;

bool deviceEnabled = true;

// SSID and password prop
std::string strSSID("myWiFiNetwork");
std::string strPassword("HelloWiFi");

// sample rate prop (a range)
uint32_t sampleRate = 123;
uint32_t sampleRateMin = 10;
uint32_t sampleRateMax = 240;

// Date prop - date is a string "YYYY-MM-DD"  - formatting is important
std::string strDate("2021-03-01");

// Time prop - date is a string "HH:MM" - 
std::string strTime("2:5"); // test value - will be parsed as 02:05

// A float property - "offset value"
float offsetValue = 4.124;

BLECharacteristic *pCharOffset; // will use later for notifications.
unsigned long ticks; // used for notification


//-------------------------------------------------------------------------
// A BLE client (device) is connected logic.
//-------------------------------------------------------------------------
//
// The system is setup to call callback methods to the below object. 
// A bool is used to keep track of connected state..

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

//-------------------------------------------------------------------------
// Characteristic Setup Section
//-------------------------------------------------------------------------
// The following functions are used to setup specific characteristics
//
// Consisist of a setup function and a callback function that the BLE system
// will call on a value update.
//
//-------------------------------------------------------------------------
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
// SSID Characterisitic 
//
// A String Characterisitic (property) example

void onSSIDUpdate(std::string &  newValue){

    Serial.print("Update SSID Value: "); 
    Serial.println(newValue.c_str());
}
BLECharacteristic * setupSSIDCharacteristic(BLEService *pService){

    BLECharacteristic *pChar;

    pChar = pService->createCharacteristic(
                            kCharacteristicSSIDUUID,
                            BLECharacteristic::PROPERTY_READ  |
                            BLECharacteristic::PROPERTY_WRITE );

    // Set the value in the callbacks 
    pChar->setCallbacks(new TextPropertyValueCB(strSSID, onSSIDUpdate));

    sf_bleprop_string(pChar, "SSID");

    return pChar;
}

//---------------------------------------------------------------------------------
// WiFi Password Characterisitic 
//
// A String Characterisitic (property) example

void onPasswordUpdate(std::string &  newValue){

    Serial.print("Update WiFi Password Value: "); 
    Serial.println(newValue.c_str());
}
BLECharacteristic * setupPasswordCharacteristic(BLEService *pService){

    BLECharacteristic *pChar;

    pChar = pService->createCharacteristic(
                            kCharacteristicPasswordUUID,
                            BLECharacteristic::PROPERTY_READ  |
                            BLECharacteristic::PROPERTY_WRITE );

    // Set the value in the callbacks 
    pChar->setCallbacks(new TextPropertyValueCB(strPassword, onPasswordUpdate));

    sf_bleprop_string(pChar, "Password");

    return pChar;
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
// Setup our system
//---------------------------------------------------------------------------------

void setup() {

    Serial.begin(115200);
    Serial.println("Starting BLE work!");

    // Init BLE  - give it our device name.
    BLEDevice::init(kTargetServiceName);

    // Create our BLE Server - And add callback object (defined above)
    // The callback is used to keep track if a device is connected or not.
    BLEServer *pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    // >>> NOTE <<<
    // When creating a service. The default - when you just pass in a UUID,
    // only creates 15 handles. This isn't enough to support the > 3 characteristics 
    // in this demo. Setting this to 5 per Characteristic seems to work.
    // see: https://github.com/nkolban/ESP32_BLE_Arduino/blob/master/src/BLEServer.cpp
    // 
    // Defining these here to highlight this concept. A
#define NUMBER_OF_CHARACTERISTICS 8
#define ESP32_BLE_HANDLES_PER_CHAR 5
    BLEService *pService = pServer->createService(BLEUUID(kTargetServiceUUID), 
            NUMBER_OF_CHARACTERISTICS * ESP32_BLE_HANDLES_PER_CHAR, 1);

    // >>> Characteristics Setup <<<
    //
    // The order of the setup calls, sets the order the characteristics (properties)
    // in the property/settings application. 
    //
    // A grouping title/area in the settings application is defined by adding a title
    // to the first Characteristic in that group. **ONLY** do this on the first 
    // characterisitic of the group - it's just a title, nothing more.

    BLECharacteristic *pChar;

    setupEnabledCharacteristic(pService);

    // Event Details Group
    pChar = setupDateCharacteristic(pService);
    sf_bleprop_group_title(pChar, "Event Details");     // Add title to 1st char    
    setupTimeCharacteristic(pService); 

    // Device Settings
    pChar = setupBaudCharacteristic(pService);
    sf_bleprop_group_title(pChar, "Device Settings"); // Title

    // WiFi Settings
    pChar = setupSSIDCharacteristic(pService);
    sf_bleprop_group_title(pChar, "WiFi Settings"); // Group title
    setupPasswordCharacteristic(pService);    

    // Sensor Settings 
    pChar = setupSampleRateCharacteristic(pService);
    sf_bleprop_group_title(pChar, "Sensor Settings");     // Group title

    // >> Notifications <<
    //
    // Will send update settings of this characteristic - save the Characterisitc pointer
    // for use in loop, and add the BLE2902 descriptor which is used for BLE Notifications.
    pCharOffset = setupOffsetCharacteristic(pService);  
    // On ESP32 - to enable notification, you need to add a special descriptor
    pCharOffset->addDescriptor(new BLE2902());


    // Startup up the system and start BLE advertising
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

    // We're up, LED ON
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, (deviceEnabled ? HIGH : LOW));

    // Keep track of millis to support our Notification example in loop
    ticks = millis();
    Serial.println("BLE Started");
}

void loop() {

    delay(200);

    // >> Update and Notification Example <<
    //
    // This section updates the offset characterisitc value every 5 secs if a
    // device is connected. Demostrates how to send a notification using the 
    // ESP32 BLE API.
    if(millis() - ticks > 5000){

        // Update the value of offset and set new value in the target Char.
        // Then send notification

        offsetValue += .5;
        if(deviceConnected){
            pCharOffset->setValue(offsetValue);
            pCharOffset->notify();
            Serial.print("Incrementing Offset to: "); Serial.println(offsetValue);
        }
        ticks = millis();
    }

}
