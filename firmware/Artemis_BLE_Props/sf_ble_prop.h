

#pragma once


#include "ArduinoBLE.h"
//--------------------------------------------------------------------------------------
// Logic to setup the C++/Arduino side of BLE properties that work with the SparkFun
// BLE Setttings Web App. 
//--------------------------------------------------------------------------------------
//
// The settings app defines a type protocol based on BLE Descriptors added to
// Characteritics. This file simplifies / automatets setting this up. 
//

// BLE UUID Codes for our Protocol Descriptors 
#define kBLEDescCharNameUUID            "2901"
#define kBLEDescSFEPropTypeUUID         "A101"
#define kBLEDescSFEPropRangeMinUUID     "A110"
#define kBLEDescSFEPropRangeMaxUUID     "A111"

// Property type codes - sent as a value of the char descriptor 
const uint8_t kSFEPropTypeBool      = 0x1;
const uint8_t kSFEPropTypeInt       = 0x2;
const uint8_t kSFEPropTypeRange     = 0x3;
const uint8_t kSFEPropTypeText      = 0x4;
const uint8_t kSFEPropTypeDate      = 0x5;
const uint8_t kSFEPropTypeTime      = 0x6;
const uint8_t kSFEPropTypeFloat     = 0x7;

// TODO: Add a method for setting order here - based on when a thing is added
void _sf_bleprop_core(BLECharacteristic & bleChar, const char *strName, const uint8_t& propType){

    BLEDescriptor* descName = new BLEDescriptor( kBLEDescCharNameUUID, strName);
    bleChar.addDescriptor(*descName);
    BLEDescriptor* descType = new BLEDescriptor( kBLEDescSFEPropTypeUUID, &propType, sizeof(propType));    
    bleChar.addDescriptor(*descType);
}
#define sf_bleprop_bool(__char__, __name__) _sf_bleprop_core(__char__, __name__, kSFEPropTypeBool)
#define sf_bleprop_int(__char__, __name__) _sf_bleprop_core(__char__, __name__, kSFEPropTypeInt)
#define sf_bleprop_string(__char__, __name__) _sf_bleprop_core(__char__, __name__, kSFEPropTypeText)
#define sf_bleprop_date(__char__, __name__) _sf_bleprop_core(__char__, __name__, kSFEPropTypeDate)
#define sf_bleprop_time(__char__, __name__) _sf_bleprop_core(__char__, __name__, kSFEPropTypeTime)
#define sf_bleprop_float(__char__, __name__) _sf_bleprop_core(__char__, __name__, kSFEPropTypeFloat)

void sf_bleprop_range(BLECharacteristic &bleChar, const char *strName, const uint32_t& vMin, const uint32_t& vMax){


    _sf_bleprop_core(bleChar, strName, kSFEPropTypeRange);


    BLEDescriptor* descMin = new BLEDescriptor(kBLEDescSFEPropRangeMinUUID, (uint8_t*)&vMin, sizeof(vMin) );   
    bleChar.addDescriptor(*descMin);
    BLEDescriptor* descMax = new BLEDescriptor( kBLEDescSFEPropRangeMaxUUID, (uint8_t*)&vMax, sizeof(vMax) );   
    bleChar.addDescriptor(*descMax);
}


