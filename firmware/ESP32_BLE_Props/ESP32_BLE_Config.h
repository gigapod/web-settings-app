

/*
 *
 * ESP32_BLE_Config.h
 *
 *  ESP32 example of the  SparkFun BLE Property system.
 *
 *  This header defines callback classes for the ESP32 property values -
 *  or BLE Characteristics. The goal is to simplify the use of callbacks
 *  for the ESP32 BLE. 
 *
 *  HISTORY
 *    Feb, 2021     - Initial developement - KDB
 * 
 *==================================================================================
 * Copyright (c) 2021 SparkFun Electronics
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *==================================================================================
 * 
 */
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