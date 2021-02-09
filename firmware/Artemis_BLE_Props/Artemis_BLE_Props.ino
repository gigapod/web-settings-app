

#include "ArduinoBLE.h"

#include "sf_ble_prop.h"

#define kTargetServiceUUID  "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define kTargetServiceName  "Artemis App"


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
#define kCharacteristicOffsetUUID   "beb5483e-36e1-4688-b7f5-ea07311b260e"

// helper
#define kMessageMax 64

// Declare the main objects that define this BLE service here. Will connect everything later
// This is a service with four characteristics

// Service
BLEService bleService(kTargetServiceUUID);

// First Characteristic - "Enabled" - bool - is the system enabled 
boolean bIsEnabled = true;
BLEBooleanCharacteristic bleCharEnabled(kCharacteristicEnabledUUID, BLERead | BLENotify | BLEWrite);

// Second Characteristic - Baud Rate - int - the baud rate of the syste
int32_t baudRate = 115200;
BLEIntCharacteristic bleCharBaudRate(kCharacteristicBaudUUID, BLERead | BLENotify | BLEWrite);

// Third Characteristic - Message - string - a string message for the service
String strMessage = "Welcome";
// Note: Setting max value size of kMessage Max  
BLEStringCharacteristic bleCharMessage(kCharacteristicMessageUUID, BLERead | BLENotify | BLEWrite, kMessageMax);

// Fourth Characteristic - Sample Rate - Range (int) - how often to sample this thing
uint32_t sampleRate = 123;

const uint32_t sampleRateMin = 10;
const uint32_t sampleRateMax = 240;

BLEIntCharacteristic bleCharSampleRate(kCharacteristicSampleUUID, BLERead | BLENotify | BLEWrite);


// Fifth Characteristic - Date - date type - a date string of format "YYYY-MM-DD"
String strDate = "2021-03-01";
// Note: Setting max value size of kMessage Max  
BLEStringCharacteristic bleCharDate(kCharacteristicDateUUID, BLERead | BLENotify | BLEWrite, kMessageMax);

// Sixth Characteristic - Time - time type - a time string of format "HH:MM"
String strTime = "12:13";
// Note: Setting max value size of kMessage Max  
BLEStringCharacteristic bleCharTime(kCharacteristicTimeUUID, BLERead | BLENotify | BLEWrite, kMessageMax);

// Seventh Characteristic -Offset - Float type - the offset value
// A float property - "offset value"
float offsetValue = 4.124;

BLEFloatCharacteristic bleCharOffset(kCharacteristicOffsetUUID, BLERead | BLENotify | BLEWrite);

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

    char buffer[kMessageMax+1]={0}; // buffer size + null - zero out 

    int len = theChar.valueLength();
    theChar.readValue((void*)buffer, len > kMessageMax ? kMessageMax : len);

    strMessage = buffer;
    Serial.print("Message Update: ");
    Serial.println(strMessage);;
}
void sampleRateUpdateCB(BLEDevice central, BLECharacteristic theChar){

    Serial.print("Sample Rate Update: ");
    theChar.readValue(sampleRate);
    Serial.println(sampleRate);
}

void dateUpdateCB(BLEDevice central, BLECharacteristic theChar){

    char buffer[kMessageMax+1]={0}; // buffer size + null - zero out 

    int len = theChar.valueLength();
    theChar.readValue((void*)buffer, len > kMessageMax ? kMessageMax : len);

    strDate = buffer;
    Serial.print("Date Update: ");
    Serial.println(strDate);
}

void timeUpdateCB(BLEDevice central, BLECharacteristic theChar){

    char buffer[kMessageMax+1]={0}; // buffer size + null - zero out 

    int len = theChar.valueLength();
    theChar.readValue((void*)buffer, len > kMessageMax ? kMessageMax : len);

    strTime = buffer;
    Serial.print("Time Update: ");
    Serial.println(strTime);
}

void offsetUpdateCB(BLEDevice central, BLECharacteristic theChar){

    Serial.print("Offset (float) Update: ");
    theChar.readValue((void*)&offsetValue, 4);
    Serial.println(offsetValue);
}
//---------------------------------------------------------------
// Wireup the charactritics 
void setupBLECharacteristics(BLEService& theService){

    // The enabled char
    sf_bleprop_bool(bleCharEnabled, "Device Enabled"); // setup property descriptor
    theService.addCharacteristic(bleCharEnabled);  
    bleCharEnabled.setValue(bIsEnabled);
    bleCharEnabled.setEventHandler(BLEWritten, enbabledUpdateCB);


    // The BaudRate char
    sf_bleprop_int(bleCharBaudRate, "Baud Rate"); // setup property descriptor    
    theService.addCharacteristic(bleCharBaudRate);  
    bleCharBaudRate.setValue(baudRate);
    bleCharBaudRate.setEventHandler(BLEWritten, buadRateUpdateCB);


    // The Message char
    sf_bleprop_string(bleCharMessage, "Device Message"); // setup property descriptor        
    theService.addCharacteristic(bleCharMessage);  
    bleCharMessage.setValue(strMessage);
    bleCharMessage.setEventHandler(BLEWritten, messageUpdateCB);    
        
    // The Sample Rate char
    sf_bleprop_range(bleCharSampleRate, "Sample Rate (sec)", sampleRateMin, sampleRateMax); // setup property descriptor            
    theService.addCharacteristic(bleCharSampleRate);
    bleCharSampleRate.setValue(sampleRate);     
    bleCharSampleRate.setEventHandler(BLEWritten, sampleRateUpdateCB);             

    // The date char
    sf_bleprop_date(bleCharDate, "Start Date"); // setup property descriptor            
    theService.addCharacteristic(bleCharDate);  
    bleCharDate.setValue(strDate);
    bleCharDate.setEventHandler(BLEWritten, dateUpdateCB); 

    // The time char
    sf_bleprop_time(bleCharTime, "Start Time"); // setup property descriptor                
    theService.addCharacteristic(bleCharTime);  
    bleCharTime.setValue(strTime);
    bleCharTime.setEventHandler(BLEWritten, timeUpdateCB);  

    // The Offset (float) char
    sf_bleprop_float(bleCharOffset, "Offset Bias"); // setup property descriptor                    
    theService.addCharacteristic(bleCharOffset);  
    bleCharOffset.setValue(offsetValue);
    bleCharOffset.setEventHandler(BLEWritten, offsetUpdateCB);  

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
//    BLE.setAdvertisedService(bleService);    

    // Setup service Characteristics
    setupBLECharacteristics(bleService);

    // add the LED service to the BLE device
    BLE.addService(bleService);
    BLE.setAdvertisedService(bleService);

    // broadcast BLE connection
    BLE.advertise();

    Serial.println(F("OLA BLE ready for connections!"));
}

void loop()
{
    // everything is handled in callbacks.
    BLE.poll();

}
