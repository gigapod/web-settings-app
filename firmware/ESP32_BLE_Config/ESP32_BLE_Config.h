
// Class defs for the property call backs used with the BLE 
// characteristic value data exchange. 
//

#pragma once

#include <string>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

int32_t stringToValue(std::string myString);

/////////////////////////////////////////////////////////////////////
// Define char callback classes that call out to a standard function 
// when a value is updated.
//
// The values is also read from an external storage value that is 
// passed in by pointer/ref.


class IntPropertyValueCB: public BLECharacteristicCallbacks {

public:
    IntPropertyValueCB( int32_t *value, void (*updateCB)(int32_t)) {

        _onSetCB = updateCB;
        pValue = value;
    }

    void onWrite(BLECharacteristic *theCharacteristic)
    {
     
        int32_t newValue = stringToValue(theCharacteristic->getValue());
       // Serial.print("Integer Prop new value: ");
       // Serial.println(newValue);
        if(pValue)
            *pValue = newValue;
        _onSetCB(newValue);

    }
    // I don't think this is needed
    void onRead(BLECharacteristic *theCharacteristic){

      // for debugging
     // Serial.print("Integer Property Requested: "); 
     // Serial.println(*pValue);

      theCharacteristic->setValue(*pValue);      

    }
private:
    void (*_onSetCB)(int32_t);
    int32_t *pValue;
};

//---------------------------------------------------
class FloatPropertyValueCB: public BLECharacteristicCallbacks {

public:
    FloatPropertyValueCB( float *value, void (*updateCB)(float)) {

        _onSetCB = updateCB;
        pValue = value;
    }

    void onWrite(BLECharacteristic *theCharacteristic)
    {
     
        int32_t tmpValue = stringToValue(theCharacteristic->getValue());

        float newValue = *((float*)&tmpValue); // Total hack

       Serial.print("Float Prop new value: ");
       Serial.println(newValue);
        if(pValue)
            *pValue = newValue;
        _onSetCB(newValue);

    }
    // I don't think this is needed
    void onRead(BLECharacteristic *theCharacteristic){

      // for debugging
     // Serial.print("Integer Property Requested: "); 
     // Serial.println(*pValue);

      theCharacteristic->setValue(*pValue);      

    }
private:
    void (*_onSetCB)(float);
    float *pValue;
};
//---------------------------------------------------
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

        //Serial.print("BOOL Prop new Value value: ");
        //Serial.println(newValue);
        if(pValue)
            *pValue = newValue;
        _onSetCB(newValue);

    }
    // I don't think this is needed
    void onRead(BLECharacteristic *theCharacteristic){

      // for debugging
      //Serial.print("Bool Property Requested: "); 
      //Serial.println(*pValue);

      theCharacteristic->setValue((uint8_t*)pValue, 1);      

    }

private:
    void (*_onSetCB)(bool);
    bool *pValue;
};

//---------------------------------------------------
class TextPropertyValueCB: public BLECharacteristicCallbacks {

public:
    TextPropertyValueCB( std::string & value, void (*updateCB)(std::string&)): value(value) {

        _onSetCB = updateCB;

    }

    void onWrite(BLECharacteristic *theCharacteristic)
    {
     
     	value = theCharacteristic->getValue();
     	
       // Serial.print("string prop new value: ");
       // Serial.println(value.c_str());
        _onSetCB(value);

    }
    // I don't think this is needed
    void onRead(BLECharacteristic *theCharacteristic){

      // for debugging
      //Serial.print("String Property Requested: "); 
     // Serial.println(value.c_str());

      theCharacteristic->setValue(value);      

    }



private:
    void (*_onSetCB)(std::string & value);
    std::string value;
};