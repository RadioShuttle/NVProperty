/*
 * Copyright (c) 2019 Helmut Tschemernjak
 * 30826 Garbsen (Hannover) Germany
 * Licensed under the Apache License, Version 2.0);
 */

#ifdef ARDUINO_ARCH_ESP32

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <NVPropertyProviderInterface.h>
#include <NVProperty_ESP32NVS.h>
#include <NVProperty.h>


#include <nvs.h>


NVProperty_ESP32NVS::NVProperty_ESP32NVS()
{
    _didWrite = false;
    _handle = 0;
}

NVProperty_ESP32NVS::~NVProperty_ESP32NVS()
{
    if (_handle) {
    	nvs_close(_handle);
    }
}

int
NVProperty_ESP32NVS::GetProperty(int key)
{
    int32_t value;
    
    esp_err_t err = nvs_get_i32(_handle, _setKey(key), &value);
    if(!err)
        return value;
    
    return NVProperty::NVP_ENOENT;
};

const char *
NVProperty_ESP32NVS::GetPropertyStr(int key)
{
    char *value;
    size_t len = sizeof(value);
    
    esp_err_t err = nvs_get_str(_handle, _setKey(key), NULL, &len);
    if (!err && len > 0) {
        /*
         * Yes we leak memory here, however for the Arduino ESP32 this ok
         * to make the API easier for users
         */
        value = new char[len+1];
        if (value == NULL)
            return NULL;
        memset(value, 0, len+1);
        esp_err_t err = nvs_get_str(_handle, _setKey(key), value, &len);
    	if(!err)
        	return value;
    }
    
    return NULL; // NVP_ENOENT
}

int
NVProperty_ESP32NVS::GetPropertyBlob(int key, const void *blob, int *size)
{
    char *value;
    size_t len = sizeof(value);
    
    esp_err_t err = nvs_get_blob(_handle, _setKey(key), NULL, &len);
    if (!err && len > 0) {
        /*
         * Yes we leak memory here, however for the Arduino ESP32 this ok
         * to make the API easier for users
         */
        value = new char[len];
        if (value == NULL)
            return 0;
        memset(value, 0, len);
        esp_err_t err = nvs_get_blob(_handle, _setKey(key), value, &len);
    	if(!err)
        	return len;
    }
    
    return 0; // NVP_ENOENT
}


int64_t
NVProperty_ESP32NVS::GetProperty64(int key)
{
    int64_t value;
    
    esp_err_t err = nvs_get_i64(_handle, _setKey(key), &value);
    if(!err)
        return value;
    
    return NVProperty::NVP_ENOENT;
}

int
NVProperty_ESP32NVS::SetProperty(int key, int64_t value, int type)
{
    esp_err_t err = 0;

    switch(type) {
        case NVProperty::T_BIT:
        case NVProperty::T_8BIT:
        case NVProperty::T_16BIT:
        case NVProperty::T_32BIT: { // T_32BIT and less
            int32_t i32 = (int32_t) value;
            err = nvs_set_i32(_handle, _setKey(key), i32);
        }
            break;
        case NVProperty::T_64BIT:
            err = nvs_set_i64(_handle, _setKey(key), value);
        	break;
        case NVProperty::T_STR:
            break;
        case NVProperty::T_BLOB:
            break;
    }
    
    if (!err) {
    	_didWrite = true;
        return NVProperty::NVP_OK;
    }
    
    return NVProperty::NVP_ENOENT;
}

int
NVProperty_ESP32NVS::SetPropertyStr(int key, const char *value, int type)
{
    esp_err_t err = 0;
    
    if (type != NVProperty::T_STR)
        return NVProperty::NVP_INVALD_PARM;
    
    err = nvs_set_str(_handle, _setKey(key), value);

    if (!err) {
        _didWrite = true;
        return NVProperty::NVP_OK;
    }
    
    return NVProperty::NVP_ENOENT;
}

int
NVProperty_ESP32NVS::SetPropertyBlob(int key, const void *blob, int size, int type)
{
    esp_err_t err = 0;
    
    if (type != NVProperty::T_BLOB)
        return NVProperty::NVP_INVALD_PARM;
    
    err = nvs_set_blob(_handle, _setKey(key), blob, size);

    if (!err) {
        _didWrite = true;
        return NVProperty::NVP_OK;
    }
    
    return NVProperty::NVP_ENOENT;
}


int
NVProperty_ESP32NVS::EraseProperty(int key)
{
    esp_err_t err = nvs_erase_key(_handle, _setKey(key));
    if (!err) {
    	_didWrite = true;
    	return 0;
    }
    return NVProperty::NVP_ENOENT;
}


int
NVProperty_ESP32NVS::ReorgProperties(void)
{
    return NVProperty::NVP_OK;
}

int
NVProperty_ESP32NVS::OpenPropertyStore(bool forWrite)
{
    esp_err_t err;
    
    if (forWrite) {
        err = nvs_open("RS", NVS_READWRITE, &_handle);
    } else {
        err = nvs_open("RS", NVS_READONLY, &_handle);
    }
    if (err)
        return NVProperty::NVP_ERR_FAIL;
    return NVProperty::NVP_OK;
}

int
NVProperty_ESP32NVS::ClosePropertyStore(bool flush)
{
    esp_err_t err = 0;

    if (_didWrite && flush) {
        err = nvs_commit(_handle);
    }
    _didWrite = false;
    if (err)
        return NVProperty::NVP_ERR_FAIL;
    return NVProperty::NVP_OK;
}

#endif // ARDUINO_ARCH_ESP32
