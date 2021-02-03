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
#include <lwip/inet.h>  // for endianess 
#include <string>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>


// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define kTargetServiceUUID  "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define kTargetServiceName  "OpenLog"
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Property Creation and Management
//
// Dynamically create and manage properties
//--------------------------------------------------------------------------------------

// BLE Codes for our service
#define kBLEDescCharNameUUID            0x2901
#define kBLEDescSFEPropTypeUUID         0xA101
#define kBLEDescSFEPropRangeMinUUID     0xA110
#define kBLEDescSFEPropRangeMaxUUID     0xA111

// Property type codes - sent as a value of the char descriptor 
#define kSFEPropTypeBool       0x1
#define kSFEPropTypeInt        0x2
#define kSFEPropTypeRange      0x3
#define kSFEPropTypeText       0x4


#define kCharacteristicBaudUUID  "beb5483e-36e1-4688-b7f5-ea07361b26a8"


uint32_t baudRate = 115200;

bool deviceConnected = false;
bool newConfig = true;

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

class cbSetBaud: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *baudCharacteristic)
    {
     
      baudRate = stringToValue(baudCharacteristic->getValue());
      newConfig = true;
      Serial.print("cbSetBaud: onWrite: New Value:");
      Serial.println(baudRate);
    }
    // I don't think this is needed
    void onRead(BLECharacteristic *baudCharacteristic){

      // for debugging
      Serial.print("Baud Rate Requested: "); 
      Serial.println(baudRate);
      // Not sure if this is needed in BLE land, but 
      // moving bytes to network endianess ....   
      uint32_t buffer = htonl(baudRate);      
      baudCharacteristic->setValue(baudRate);      

    }
};

class intProperty: public BLECharacteristicCallbacks {

public:
    intProperty( uint32_t *value, void (*updateCB)(uint32_t)) {

        _onSetCB = updateCB;
        pValue = value;
    }

    void onWrite(BLECharacteristic *baudCharacteristic)
    {
     
        uint32_t newValue = stringToValue(baudCharacteristic->getValue());
        Serial.print("OnWriteCallback value: ");
        Serial.println(newValue);
        if(pValue)
            *pValue = newValue;
        _onSetCB(newValue);

    }
    // I don't think this is needed
    void onRead(BLECharacteristic *baudCharacteristic){

      // for debugging
      Serial.print("Integer Property Requested: "); 
      Serial.println(*pValue);

      // moving bytes to network endianess ....   
      //      uint32_t buffer = htonl(value);      
      baudCharacteristic->setValue(*pValue);      

    }



private:
    void (*_onSetCB)(uint32_t);
    uint32_t *pValue;
};

//---------------------------------------------------------------------------------
// Baud Rate Characterisitic 
//
// A Integer Characterisitic (property) example

void onBaudUpdate(uint32_t newValue){

    Serial.print("Update Baud Value: "); 
    Serial.println(baudRate);
}
void setupBaudCharacteristic(BLEService *pService){

    BLECharacteristic *pCharBaud;

    pCharBaud = pService->createCharacteristic(
                            kCharacteristicBaudUUID,
                            BLECharacteristic::PROPERTY_READ  |
                            BLECharacteristic::PROPERTY_WRITE |
                            BLECharacteristic::PROPERTY_NOTIFY );

    // Set our value 
    pCharBaud->setValue(baudRate);  //TODO -- IS THIS NEEDED???

    // Set the value in the callbacks 
    pCharBaud->setCallbacks(new intProperty(&baudRate, onBaudUpdate));

    // Add descriptor to that service (char user description)
    BLEDescriptor * pDesc = new BLEDescriptor((uint16_t)kBLEDescCharNameUUID);
    std::string descStr = "Output Baud Value";
    pDesc->setValue(descStr);
    pCharBaud->addDescriptor(pDesc);

    pDesc = new BLEDescriptor((uint16_t)kBLEDescSFEPropTypeUUID);  // Property type
    uint8_t data=kSFEPropTypeInt;
    pDesc->setValue(&data,1);
    pCharBaud->addDescriptor(pDesc);

}

void setup() {
    Serial.begin(115200);
    Serial.println("Starting BLE work!");

    BLEDevice::init(kTargetServiceName);

    BLEServer *pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());
    BLEService *pService = pServer->createService(kTargetServiceUUID);

    //Setup characterstics
    setupBaudCharacteristic(pService);

    //Begin broadcasting
    pService->start();
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


    Serial.println("BLE Started");
}

void loop() {
  delay(200);

  if (newConfig == true)
  {
    newConfig = false;

    Serial.print("Baud rate:");
    Serial.println(baudRate);
  }

  if (Serial.available())
  {
    byte incoming = Serial.read();

    if (incoming == '1')
    {
      //baudCharacteristic->setValue("Value 1");
      Serial.println("Val set");
    }
    else if (incoming == '2')
    {
      //baudCharacteristic->setValue("Value 2");
      Serial.println("Val set");
    }
    else
    {
      Serial.println("Unknown");
    }

    delay(10);
    while (Serial.available()) Serial.read(); //Clear buffer

  }
}
