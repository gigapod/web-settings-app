

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
// The settings app defines a type protocol based on BLE Descriptors added to
// Characteritics. This file simplifies / automatets setting this up. 
//

// BLE UUID Codes for our Protocol Descriptors 
#define kBLEDescCharNameUUID            "A100"
#define kBLEDescSFEPropTypeUUID         "A101"
#define kBLEDescSFEPropRangeMinUUID     "A110"
#define kBLEDescSFEPropRangeMaxUUID     "A111"
#define kBLEDescSFEGroupTitleUUID       "A112"


// Property type codes - sent as a value of the char descriptor 
 uint8_t kSFEPropTypeBool      = 0x1;
 uint8_t kSFEPropTypeInt       = 0x2;
 uint8_t kSFEPropTypeRange     = 0x3;
 uint8_t kSFEPropTypeText      = 0x4;
 uint8_t kSFEPropTypeDate      = 0x5;
 uint8_t kSFEPropTypeTime      = 0x6;
 uint8_t kSFEPropTypeFloat     = 0x7;

//-------------------------------------------------------------------------
// Sort order counter -- Add order == Sort order
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
//
// ArduinoBLE likes it's const

#define sfe_ble_const const

//-------------------------------------------------------------------------
// typedef for ArduinoBLE
typedef BLECharacteristic& sfe_bleprop_charc_t;

#endif

// Function that actually adds the descriptor - this is platform dependant.
void _sf_bleprop_add_desc(sfe_bleprop_charc_t bleChar, const char* uuid, sfe_ble_const uint8_t *pData, int size){

#ifdef ESP32
    BLEDescriptor * pDesc = new BLEDescriptor(uuid);
    pDesc->setValue(pData, size);
    bleChar->addDescriptor(pDesc);
#else
    BLEDescriptor* desc = new BLEDescriptor( uuid, pData, size);
    bleChar.addDescriptor(*desc);
#endif
}

//-------------------------------------------------------------------------
void _sf_bleprop_core(sfe_bleprop_charc_t bleChar,  sfe_ble_const char *strName, sfe_ble_const uint8_t& propType){

    _sf_bleprop_add_desc(bleChar, kBLEDescCharNameUUID, (sfe_ble_const uint8_t*)strName, strlen(strName));

    // Setup our attribute mask field. 32 bits for the future. Alloc b/c some BLE systems
    // need it's own memory for value fields.
    //
    // Layout MSB to LSB
    //  [TBD][TBD][ORDER][TYPE]
    uint32_t * attr = new uint32_t;
    *attr = (sort_pos++ << 8 ) | propType; // notice inc on order position
    _sf_bleprop_add_desc(bleChar, kBLEDescSFEPropTypeUUID, (sfe_ble_const uint8_t*)attr, sizeof(uint32_t));    

}

// macros for other types - simple types
#define sf_bleprop_bool(__char__, __name__) _sf_bleprop_core(__char__, __name__, kSFEPropTypeBool)
#define sf_bleprop_int(__char__, __name__) _sf_bleprop_core(__char__, __name__, kSFEPropTypeInt)
#define sf_bleprop_string(__char__, __name__) _sf_bleprop_core(__char__, __name__, kSFEPropTypeText)
#define sf_bleprop_date(__char__, __name__) _sf_bleprop_core(__char__, __name__, kSFEPropTypeDate)
#define sf_bleprop_time(__char__, __name__) _sf_bleprop_core(__char__, __name__, kSFEPropTypeTime)
#define sf_bleprop_float(__char__, __name__) _sf_bleprop_core(__char__, __name__, kSFEPropTypeFloat)

void sf_bleprop_range(sfe_bleprop_charc_t bleChar,  sfe_ble_const char *strName,  
					  sfe_ble_const uint32_t& vMin, sfe_ble_const uint32_t& vMax){

    _sf_bleprop_core(bleChar, strName, kSFEPropTypeRange);

    // Descriptor values must be persistant, not stack-based. Expect users not to care/know,
    // so alloc copies..
    uint32_t * pMin = new uint32_t;
    uint32_t * pMax = new uint32_t;    

    *pMin = vMin;
    *pMax = vMax;
    _sf_bleprop_add_desc(bleChar, kBLEDescSFEPropRangeMinUUID, ( uint8_t*)pMin, sizeof(uint32_t));    
    _sf_bleprop_add_desc(bleChar, kBLEDescSFEPropRangeMaxUUID, ( uint8_t*)pMax, sizeof(uint32_t));        

}

void sf_bleprop_group_title(sfe_bleprop_charc_t bleChar,  sfe_ble_const char *strGroup){

    _sf_bleprop_add_desc(bleChar, kBLEDescSFEGroupTitleUUID, ( uint8_t*)strGroup, strlen(strGroup));        
}


