

#pragma once

#ifdef ESP32 
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#else 
#include "ArduinoBLE.h"
#endif

//--------------------------------------------------------------------------------------
// Logic to setup the C++/Arduino side of BLE properties that work with the SparkFun
// BLE Setttings Web App. 
//--------------------------------------------------------------------------------------
//
#define kSFBLEBufferSize 256

#define kSFBLEMaxString 64
// The settings app defines a type protocol based on BLE Descriptors added to
// Characteritics. This file simplifies / automatets setting this up. 
//  
// BLE UUID Codes for our Protocol Descriptors 
#define kBLEDescSFEPropCoreUUID         "A101"
#define kBLEDescSFEPropRangeLimitsUUID  "A110"
#define kBLEDescSFEGroupTitleUUID       "A112"


// Property type codes - sent as a value of the char descriptor 
uint8_t kSFEPropTypeBool      = 0x1;
uint8_t kSFEPropTypeInt       = 0x2;
uint8_t kSFEPropTypeRange     = 0x3;
uint8_t kSFEPropTypeText      = 0x4;
uint8_t kSFEPropTypeDate      = 0x5;
uint8_t kSFEPropTypeTime      = 0x6;
uint8_t kSFEPropTypeFloat     = 0x7;


/*
 *  Encoding block types
*/
#define kBlkRange  0x02
#define kBlkTitle  0x01

//-------------------------------------------------------------------------
static uint8_t sort_pos=0;  // if there are over 256 props, this system has bigger issues

//-------------------------------------------------------------------------
// ESP DEFS
#ifdef ESP32

 // ESP hates const
#define sfe_ble_const 

// typedef for ESP32
typedef BLECharacteristic* sfe_bleprop_charc_t;

#else
//-------------------------------------------------------------------------
// ArduinoBLE DEFS
// ArduinoBLE likes it's const

#define sfe_ble_const const

//-------------------------------------------------------------------------
// typedef for ArduinoBLE
typedef BLECharacteristic& sfe_bleprop_charc_t;

#endif
//-------------------------------------------------------------------------
// addString
// 
// Add a c String to our descriptor buffer
//
// Returns new end point
//
// iEnd is the next open byte.
size_t _add_string(uint8_t *pBuffer, size_t iNext, sfe_ble_const char *pString, size_t nString){

    if(!pString || nString < 1)
        return iNext;

    // Will need length of string plus 
    pBuffer[iNext] = nString;
    iNext++;
    memcpy((void*)(pBuffer+iNext), pString, nString);

    return iNext+nString;
}

size_t _add_attributes(uint8_t *pBuffer, size_t iNext, uint8_t& propType){

    pBuffer[iNext]      = propType;
    pBuffer[iNext+1]    = sort_pos++;

    return iNext+4; // alloc 4 bytes for 
}
//-------------------------------------------------------------------------
// Function that actually adds the descriptor - this is platform dependant.
void _sf_bleprop_add_desc(sfe_bleprop_charc_t bleChar, const char* uuid, uint8_t *pData, size_t size){

    uint8_t *pBuffer = new uint8_t[size];

    memcpy((void*)pBuffer, (void*)pData, size);

#ifdef ESP32
    BLEDescriptor * pDesc = new BLEDescriptor(uuid);
    pDesc->setValue(pBuffer, size);
    bleChar->addDescriptor(pDesc);
#else
    BLEDescriptor* desc = new BLEDescriptor( uuid, pBuffer, size);
    bleChar.addDescriptor(*desc);
#endif
}

//-------------------------------------------------------------------------
size_t _sf_bleprop_core(uint8_t *pBuffer, size_t iNext, sfe_ble_const char *strName, uint8_t& propType){

    // Attributes
    iNext = _add_attributes(pBuffer, iNext, propType);

    // Prop Name
	int nName = strlen(strName);
	nName = nName > kSFBLEMaxString ? kSFBLEMaxString : nName;
    iNext = _add_string(pBuffer, iNext, strName, nName );

    return iNext;
}

//-------------------------------------------------------------------------
void _sf_bleprop_basic(sfe_bleprop_charc_t bleChar,  sfe_ble_const char *strName, uint8_t& propType,
                        sfe_ble_const char *strTitle){

    uint8_t dBuffer[kSFBLEBufferSize] = {0};
    uint16_t iNext = 0;
    
    iNext = _sf_bleprop_core(dBuffer, iNext, strName, propType);

    // is there a title?
    if(strTitle){
        int nTitle = strlen(strTitle);
        nTitle = nTitle > kSFBLEMaxString ? kSFBLEMaxString : nTitle;
        dBuffer[iNext++] = kBlkTitle;
        iNext = _add_string(dBuffer, iNext, strTitle, nTitle );
    }
    _sf_bleprop_add_desc(bleChar, kBLEDescSFEPropCoreUUID, dBuffer, iNext);  

}
//-------------------------------------------------------------------------
// macros for other types - simple types
#define sf_bleprop_bool(__char__, __name__, __title__) _sf_bleprop_basic(__char__, __name__, kSFEPropTypeBool, __title__)
#define sf_bleprop_int(__char__, __name__, __title__) _sf_bleprop_basic(__char__, __name__, kSFEPropTypeInt, __title__)
#define sf_bleprop_string(__char__, __name__, __title__) _sf_bleprop_basic(__char__, __name__, kSFEPropTypeText, __title__)
#define sf_bleprop_date(__char__, __name__, __title__) _sf_bleprop_basic(__char__, __name__, kSFEPropTypeDate, __title__)
#define sf_bleprop_time(__char__, __name__, __title__) _sf_bleprop_basic(__char__, __name__, kSFEPropTypeTime, __title__)
#define sf_bleprop_float(__char__, __name__, __title__) _sf_bleprop_basic(__char__, __name__, kSFEPropTypeFloat, __title__)

void sf_bleprop_range(sfe_bleprop_charc_t bleChar,  sfe_ble_const char *strName,  
					  sfe_ble_const uint32_t& vMin, sfe_ble_const uint32_t& vMax,
                      sfe_ble_const char *strTitle){


    uint8_t dBuffer[kSFBLEBufferSize] = {0};
    uint16_t iNext = 0;

    // core information
    iNext = _sf_bleprop_core(dBuffer, iNext, strName, kSFEPropTypeRange);

    // is there a title?
    if(strTitle){
        int nTitle = strlen(strTitle);
        nTitle = nTitle > kSFBLEMaxString ? kSFBLEMaxString : nTitle;
        dBuffer[iNext++] = kBlkTitle;
        iNext = _add_string(dBuffer, iNext, strTitle, nTitle );
    }
    // encode or range values

    dBuffer[iNext++] = kBlkRange;

    
    // Descriptor values must be persistant, not stack-based. Expect users not to care/know,
    // so alloc copies..
    uint32_t range[2] = {vMin, vMax};    
    memcpy((void*)(dBuffer+iNext), (void*)range, sizeof(range));

    iNext += sizeof(range);

    _sf_bleprop_add_desc(bleChar, kBLEDescSFEPropCoreUUID, dBuffer, iNext);      

}


