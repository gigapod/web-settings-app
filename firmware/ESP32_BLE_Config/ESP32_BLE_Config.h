
// Class defs for the property call backs used with the BLE 
// characteristic value data exchange. 
//

#pragma once

#include <string>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

int32_t stringToValue(std::string myString);

class IntPropertyValueCB: public BLECharacteristicCallbacks {

public:
    IntPropertyValueCB( int32_t *value, void (*updateCB)(int32_t)) {

        _onSetCB = updateCB;
        pValue = value;
    }

    void onWrite(BLECharacteristic *theCharacteristic)
    {
     
        int32_t newValue = stringToValue(theCharacteristic->getValue());
        Serial.print("INT OnWriteCallback value: ");
        Serial.println(newValue);
        if(pValue)
            *pValue = newValue;
        _onSetCB(newValue);

    }
    // I don't think this is needed
    void onRead(BLECharacteristic *theCharacteristic){

      // for debugging
      Serial.print("Integer Property Requested: "); 
      Serial.println(*pValue);

      // moving bytes to network endianess ....   
      //      uint32_t buffer = htonl(value);      
      theCharacteristic->setValue(*pValue);      

    }



private:
    void (*_onSetCB)(int32_t);
    int32_t *pValue;
};

class BoolPropertyValueCB: public BLECharacteristicCallbacks {

public:
    BoolPropertyValueCB( bool *value, void (*updateCB)(bool)) {

        _onSetCB = updateCB;
        pValue = value;
    }

    void onWrite(BLECharacteristic *theCharacteristic)
    {
     
     	uint8_t * pData = theCharacteristic->getData();
     	if(!pData){
     		Serial.println("Error with bool onwrite");
     		return;
     	}
     	bool newValue = (bool)*pData;

        Serial.print("BOOL OnWriteCallback value: ");
        Serial.println(newValue);
        if(pValue)
            *pValue = newValue;
        _onSetCB(newValue);

    }
    // I don't think this is needed
    void onRead(BLECharacteristic *theCharacteristic){

      // for debugging
      Serial.print("Integer Property Requested: "); 
      Serial.println(*pValue);

      // moving bytes to network endianess ....   
      //      uint32_t buffer = htonl(value);      
      theCharacteristic->setValue((uint8_t*)pValue, 1);      

    }



private:
    void (*_onSetCB)(bool);
    bool *pValue;
};

class TextPropertyValueCB: public BLECharacteristicCallbacks {

public:
    TextPropertyValueCB( std::string & value, void (*updateCB)(std::string&)): value(value) {

        _onSetCB = updateCB;

    }

    void onWrite(BLECharacteristic *theCharacteristic)
    {
     
     	value = theCharacteristic->getValue();
     	
        Serial.print("STRING OnWriteCallback value: ");
        Serial.println(value.c_str());
        _onSetCB(value);

    }
    // I don't think this is needed
    void onRead(BLECharacteristic *theCharacteristic){

      // for debugging
      Serial.print("String Property Requested: "); 
      Serial.println(value.c_str());

      // moving bytes to network endianess ....   
      //      uint32_t buffer = htonl(value);      
      theCharacteristic->setValue(value);      

    }



private:
    void (*_onSetCB)(std::string & value);
    std::string value;
};