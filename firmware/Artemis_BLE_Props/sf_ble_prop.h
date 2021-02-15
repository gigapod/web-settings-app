/*
 *
 * sf_ble_prop.h
 *
 *  The SparkFun BLE Property system.
 *
 *  Used to add "property" attributes to a BLE Characteristic using a BLE Descriptor. The
 *  property information is used by a client application to render a property sheet 
 *  that represents
 *
 *  USE
 *
 * 
 *  HISTORY
 *    Feb, 2021     - Initial developement - KDB
 *  
 */


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
// BLE UUID Code for our Protocol Descriptor 
#define kBLEDescSFEPropCoreUUID         "A101"


// Property type codes - sent as a value of the char descriptor 
uint8_t kSFEPropTypeBool      = 0x1;
uint8_t kSFEPropTypeInt       = 0x2;
uint8_t kSFEPropTypeRange     = 0x3;
uint8_t kSFEPropTypeText      = 0x4;
uint8_t kSFEPropTypeDate      = 0x5;
uint8_t kSFEPropTypeTime      = 0x6;
uint8_t kSFEPropTypeFloat     = 0x7;
uint8_t kSFEPropTypeSelect    = 0x8;


// Descriptor Data block encoding block types

#define kBlkRange       0x02
#define kBlkTitle       0x01
#define kBlkSelectOp    0x03

//-------------------------------------------------------------------------
static uint8_t sort_pos=0;  // if there are over 256 props, this system has bigger issues

//-------------------------------------------------------------------------
// Platform dependant defs ...
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

/////////////////////////////////////////////////////////////////////////////
// Define a class that encapsulates all the routines and logic to 
// encode data and build the BLE descriptors used to define 
// a property.
//
// All class methods are class (static) methods. This provides a
// cleaner API and allows the use of method overloading.
// 
// The class implements a singleton pattern. Allows for ".method()" call notation.

class sfBLEProperties {


private:
    sfBLEProperties(){};

    ~sfBLEProperties(){};

public:
    // -----------------------------------------------
    // singleton things
    static sfBLEProperties& getInstance(void){

        static sfBLEProperties instance;
        return instance;
    }

    // Delete copy and assignment constructors - b/c this is singleton.
    sfBLEProperties(sfBLEProperties const&) = delete;
    void operator=(sfBLEProperties const&) = delete;
    // -----------------------------------------------

    // -----------------------------------------------    
    // public API methods

    static void add_bool(sfe_bleprop_charc_t bleChar,  sfe_ble_const char *strName){
        sfBLEProperties::add_basic(bleChar, strName, kSFEPropTypeBool);
    }

    //-------------------------------------------------------------------------
    static void add_int(sfe_bleprop_charc_t bleChar,  sfe_ble_const char *strName){
        sfBLEProperties::add_basic(bleChar, strName, kSFEPropTypeInt);
    } 

    //-------------------------------------------------------------------------
    static void add_string(sfe_bleprop_charc_t bleChar,  sfe_ble_const char *strName){
        sfBLEProperties::add_basic(bleChar, strName, kSFEPropTypeText);
    } 

    //-------------------------------------------------------------------------
    static void add_float(sfe_bleprop_charc_t bleChar,  sfe_ble_const char *strName){
        sfBLEProperties::add_basic(bleChar, strName, kSFEPropTypeFloat);
    } 

    //-------------------------------------------------------------------------
    static void add_date(sfe_bleprop_charc_t bleChar,  sfe_ble_const char *strName){
        sfBLEProperties::add_basic(bleChar, strName, kSFEPropTypeDate);
    } 

    //-------------------------------------------------------------------------
    static void add_time(sfe_bleprop_charc_t bleChar,  sfe_ble_const char *strName){
        sfBLEProperties::add_basic(bleChar, strName, kSFEPropTypeTime);
    } 

    //-------------------------------------------------------------------------
    static void add_range(sfe_bleprop_charc_t bleChar,  sfe_ble_const char *strName,  
                      sfe_ble_const uint32_t& vMin, sfe_ble_const uint32_t& vMax){

        uint8_t dBuffer[kSFBLEBufferSize] = {0};
        uint16_t iNext = 0;

        // core information
        iNext = sfBLEProperties::encode_core(dBuffer, iNext, strName, kSFEPropTypeRange);

    	// encode for range values
		dBuffer[iNext++] = kBlkRange; // block type: range block of data

    	uint32_t range[2] = {vMin, vMax};    
    	memcpy((void*)(dBuffer+iNext), (void*)range, sizeof(range));

        sfBLEProperties::set_descriptor(bleChar, dBuffer, iNext+sizeof(range));
	}

    //-------------------------------------------------------------------------
    // Add a select property - a property that has a list of possible values.
    //
    // The value options are contained in a string, seperated by "|" characters.
    //
    static void add_select(sfe_bleprop_charc_t bleChar,  sfe_ble_const char *strName,
                           sfe_ble_const char *strOptions){

        uint8_t dBuffer[kSFBLEBufferSize] = {0};
        uint16_t iNext = 0;

        // If the option string is too big, we return
        if(!strOptions || !strlen(strOptions) || strlen(strOptions) > kSFBLEMaxString)
            return;

        // core information
        iNext = sfBLEProperties::encode_core(dBuffer, iNext, strName, kSFEPropTypeSelect);

        // encode for select options
        dBuffer[iNext++] = kBlkSelectOp; // block type: range block of data
        iNext = sfBLEProperties::encode_string(dBuffer, iNext, strOptions, strlen(strOptions));

        sfBLEProperties::set_descriptor(bleChar, dBuffer, iNext);

    }
    //-------------------------------------------------------------------------
    // add_title()
    //
    // Add a title before the next property. This value is stashed until the next
    // add property call, when the value is encoded into that properties 
    // descriptor data block.
    //
    // Any string longer that kSFBLEMaxString is clipped. 
    static void add_title(sfe_ble_const char *strTitle){

        if(!strTitle || !strlen(strTitle))
            return;

        strlcpy(sfBLEProperties::titleBuffer, strTitle, sizeof(sfBLEProperties::titleBuffer));
    }

    // Buffer to stash a title 
    static char titleBuffer[kSFBLEMaxString];

private:

    //-------------------------------------------------------------------------
    // encode_title()
    //
    // Add a title string to the desc buffer if a title exists.
    static size_t encode_title(uint8_t *pBuffer, size_t iNext){
        // is there a title?
        int nTitle = strlen(sfBLEProperties::titleBuffer);     
        if(!nTitle)
            return iNext;

        pBuffer[iNext++] = kBlkTitle; // block type: title string
        iNext = sfBLEProperties::encode_string(pBuffer, iNext, sfBLEProperties::titleBuffer, nTitle );

        // zero out buffer string 
        memset((void*)sfBLEProperties::titleBuffer, '\0', sizeof(sfBLEProperties::titleBuffer));

        return iNext + nTitle;
    }
    //-------------------------------------------------------------------------
    // encode_attributes()
    //
    // Encode the general attributes of the property    
    static  size_t encode_attributes(uint8_t *pBuffer, size_t iNext, uint8_t& propType){

        pBuffer[iNext]      = propType;
        pBuffer[iNext+1]    = sort_pos++;

        return iNext+4; // alloc 4 bytes for - 2 for the future! 
    }
    //-------------------------------------------------------------------------
    // encode_string
    // 
    // Add a c String to our descriptor buffer
    //
    // Returns new end point in buffer.
    //
    // iEnd is the next open byte.
    static size_t encode_string(uint8_t *pBuffer, size_t iNext, sfe_ble_const char *pString, size_t nString){

        if(!pString || nString < 1)
            return iNext;

        // Will need length of string plus 
        pBuffer[iNext] = nString;
        iNext++;
        memcpy((void*)(pBuffer+iNext), pString, nString);

        return iNext+nString;
    }
    //-------------------------------------------------------------------------
    // add_core
    //
    // Add core attributes and name to a property.    
    static size_t encode_core(uint8_t *pBuffer, size_t iNext, sfe_ble_const char *strName, uint8_t& propType){

        // Attributes
        iNext = sfBLEProperties::encode_attributes(pBuffer, iNext, propType);

        // Prop Name
        int nName = strlen(strName);
        nName = nName > kSFBLEMaxString ? kSFBLEMaxString : nName;
        iNext = sfBLEProperties::encode_string(pBuffer, iNext, strName, nName );

        // check title and return;
        iNext = sfBLEProperties::encode_title(pBuffer, iNext);

        return iNext;
    }

    //-------------------------------------------------------------------------
    // Function that actually adds the descriptor - this is platform dependant.
    static void set_descriptor(sfe_bleprop_charc_t bleChar, uint8_t *pData, size_t size){

        uint8_t *pBuffer = new uint8_t[size]; // the descp needs persistant memory...

        memcpy((void*)pBuffer, (void*)pData, size);

#ifdef ESP32
        BLEDescriptor * pDesc = new BLEDescriptor(kBLEDescSFEPropCoreUUID);
        pDesc->setValue(pBuffer, size);
        bleChar->addDescriptor(pDesc);
#else
        BLEDescriptor* desc = new BLEDescriptor( kBLEDescSFEPropCoreUUID, pBuffer, size);
        bleChar.addDescriptor(*desc);
#endif
    }

    //-------------------------------------------------------------------------
    // Add a basic value 
    //
    // If a property type is just defined by TYPE and NAME, this routine is what
    // to call
    static void add_basic(sfe_bleprop_charc_t bleChar,  sfe_ble_const char *strName, uint8_t& propType){

        uint8_t dBuffer[kSFBLEBufferSize] = {0};
        uint16_t iNext = 0;
    
        iNext = sfBLEProperties::encode_core(dBuffer, iNext, strName, propType);

        sfBLEProperties::set_descriptor(bleChar, dBuffer, iNext);  

    }
};
char sfBLEProperties::titleBuffer[kSFBLEMaxString]={0}; // C++ static class var init/storage

// Stash our singleton here - enable a .method() use pattern
auto& BLEProperties = sfBLEProperties::getInstance();



