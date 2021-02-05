

#include "ArduinoBLE.h"

#define kTargetServiceUUID  "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define kTargetServiceName  "OpenLog"
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Property Creation and Management
//
// Dynamically create and manage properties
//--------------------------------------------------------------------------------------

// BLE Codes for our service
#define kBLEDescCharNameUUID            "2901"
#define kBLEDescSFEPropTypeUUID         "A101"
#define kBLEDescSFEPropRangeMinUUID     "A110"
#define kBLEDescSFEPropRangeMaxUUID     "A111"

// Property type codes - sent as a value of the char descriptor 
const uint8_t kSFEPropTypeBool      = 0x1;
const uint8_t kSFEPropTypeInt       = 0x2;
const uint8_t kSFEPropTypeRange     = 0x3;
const uint8_t kSFEPropTypeText      = 0x4;


// Our Characteristic UUIDs - and yes, just made these up
#define kCharacteristicBaudUUID     "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define kCharacteristicEnabledUUID  "beb5483e-36e1-4688-b7f5-ea07361b26a9"
#define kCharacteristicMessageUUID  "beb5483e-36e1-4688-b7f5-ea07361b26aa"
#define kCharacteristicSampleUUID   "beb5483e-36e1-4688-b7f5-ea07311b260b"

// helper
#define kMessageMax 64

// Declare the main objects that define this BLE service here. Will connect everything later
// This is a service with four characteristics

// Service
BLEService bleService(kTargetServiceUUID);

// First Characteristic - "Enabled" - bool - is the system enabled 
boolean bIsEnabled = true;
BLEBooleanCharacteristic bleCharEnabled(kCharacteristicEnabledUUID, BLERead | BLENotify | BLEWrite);
BLEDescriptor descEnabledName(kBLEDescCharNameUUID, "Device Enabled");
BLEDescriptor descEnabledType(kBLEDescSFEPropTypeUUID, &kSFEPropTypeBool, sizeof(kSFEPropTypeBool) );

// Second Characteristic - Baud Rate - int - the baud rate of the syste
int32_t baudRate = 115200;
BLEIntCharacteristic bleCharBaudRate(kCharacteristicBaudUUID, BLERead | BLENotify | BLEWrite);
BLEDescriptor descBaudRateName(kBLEDescCharNameUUID, "Baud Rate");
BLEDescriptor descBaudRateType(kBLEDescSFEPropTypeUUID, &kSFEPropTypeInt, sizeof(kSFEPropTypeInt) );

// Third Characteristic - Message - string - a string message for the service
String strMessage = "Welcome";
// Note: Setting max value size of kMessage Max  
BLEStringCharacteristic bleCharMessage(kCharacteristicMessageUUID, BLERead | BLENotify | BLEWrite, kMessageMax);
BLEDescriptor descMessageName(kBLEDescCharNameUUID, "Device Message");
BLEDescriptor descMessageType(kBLEDescSFEPropTypeUUID, &kSFEPropTypeText, sizeof(kSFEPropTypeText) );

// Fourth Characteristic - Sample Rate - Range (int) - how often to sample this thing
uint32_t sampleRate = 123;

const uint32_t sampleRateMin = 10;
const uint32_t sampleRateMax = 240;

BLEIntCharacteristic bleCharSampleRate(kCharacteristicSampleUUID, BLERead | BLENotify | BLEWrite);
BLEDescriptor descSampleRateName(kBLEDescCharNameUUID, "Sample Rate (sec)");
BLEDescriptor descSampleRateType(kBLEDescSFEPropTypeUUID, &kSFEPropTypeRange, sizeof(kSFEPropTypeRange) );
BLEDescriptor descSampleRateMin(kBLEDescSFEPropRangeMinUUID, (uint8_t*)&sampleRateMin, sizeof(sampleRateMin) );
BLEDescriptor descSampleRateMax(kBLEDescSFEPropRangeMaxUUID, (uint8_t*)&sampleRateMax, sizeof(sampleRateMax) );


//--------------- end object setup ------------------------
const byte LED_TO_TOGGLE = 19;

//---------------------------------------------------------------
// Callbacks
void enbabledUpdateCB(BLEDevice central, BLECharacteristic theChar){

    Serial.print("Is Enabled Update: ");
    uint8_t newValue;
    theChar.readValue(newValue); // no readValue for bools
    bIsEnabled = (boolean)newValue;
    Serial.println(bIsEnabled);

    digitalWrite(LED_BUILTIN, (bIsEnabled ? HIGH : LOW));
}
void buadRateUpdateCB(BLEDevice central, BLECharacteristic theChar){

    Serial.print("Baud Rate Update: ");
    theChar.readValue(baudRate);
    Serial.println(baudRate);
}

void messageUpdateCB(BLEDevice central, BLECharacteristic theChar){

    char buffer[kMessageMax];


    if(theChar.valueLength() >= sizeof(buffer)){
        Serial.println("Message Update - new message too long - truncating");
    }
    theChar.readValue((void*)buffer, sizeof(buffer));

    strMessage = buffer;
    Serial.print("Message Update: ");
    Serial.println(strMessage);;
}
void sampleRateUpdateCB(BLEDevice central, BLECharacteristic theChar){

    Serial.print("Sample Rate Update: ");
    theChar.readValue(sampleRate);
    Serial.println(sampleRate);
}
//---------------------------------------------------------------
// Wireup the charactritics 
void setupBLECharacteristics(BLEService& theService){

    // The enabled char
    bleCharEnabled.addDescriptor(descEnabledName);
    bleCharEnabled.addDescriptor(descEnabledType);
    theService.addCharacteristic(bleCharEnabled);  
    bleCharEnabled.setValue(bIsEnabled);
    bleCharEnabled.setEventHandler(BLEWritten, enbabledUpdateCB);

    // The BaudRate char
    bleCharBaudRate.addDescriptor(descBaudRateName);
    bleCharBaudRate.addDescriptor(descBaudRateType);
    theService.addCharacteristic(bleCharBaudRate);  
    bleCharBaudRate.setValue(baudRate);
    bleCharBaudRate.setEventHandler(BLEWritten, buadRateUpdateCB);

    // The Message char
    bleCharMessage.addDescriptor(descMessageName);
    bleCharMessage.addDescriptor(descMessageType);
    theService.addCharacteristic(bleCharMessage);  
    bleCharMessage.setValue(strMessage);
    bleCharMessage.setEventHandler(BLEWritten, messageUpdateCB);    

    // The Sample Rate char
    bleCharSampleRate.addDescriptor(descSampleRateName);
    bleCharSampleRate.addDescriptor(descSampleRateType);
    bleCharSampleRate.addDescriptor(descSampleRateMin);
    bleCharSampleRate.addDescriptor(descSampleRateMax);        
    theService.addCharacteristic(bleCharSampleRate);
    bleCharSampleRate.setValue(sampleRate);     

    bleCharSampleRate.setEventHandler(BLEWritten, sampleRateUpdateCB);             


}
// General Connect callbacks
void blePeripheralConnectHandler(BLEDevice central) {
  // central connected event handler
  Serial.print("Connected event, central: ");
  Serial.println(central.address());
}

void blePeripheralDisconnectHandler(BLEDevice central) {
  // central disconnected event handler
  Serial.print("Disconnected event, central: ");
  Serial.println(central.address());
}

void setup() {

    // start up serial port
    Serial.begin(115200);
    while (!Serial);

    // led to display when connected
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, (bIsEnabled ? HIGH : LOW));

    // start BLE
    if ( ! BLE.begin()){
        Serial.println("starting BLE failed!");
        while (1);
    }
    // assign event handlers for connected, disconnected to peripheral
    BLE.setEventHandler(BLEConnected, blePeripheralConnectHandler);
    BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);

    // name the device
    BLE.setLocalName(kTargetServiceName);
    BLE.setAdvertisedService(bleService);

    // Setup service Characteristics
    setupBLECharacteristics(bleService);

    // add the LED service to the BLE device
    BLE.addService(bleService);

    // broadcast BLE connection
    BLE.advertise();

    Serial.println(F("OLA BLE ready for connections!"));
}

void loop()
{
    // everything is handled in callbacks.
    BLE.poll();

}
