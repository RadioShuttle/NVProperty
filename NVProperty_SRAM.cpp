/*
 * The file is licensed under the Apache License, Version 2.0
 * (c) 2017 Helmut Tschemernjak
 * 30826 Garbsen (Hannover) Germany
 */

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <NVPropertyProviderInterface.h>
#include <NVProperty.h>
#include <NVProperty_SRAM.h>

NVProperty_SRAM::NVProperty_SRAM()
{
    
}

NVProperty_SRAM::~NVProperty_SRAM()
{
    map<int, PropertyEntry>::iterator re;
    for(re = _props.begin(); re != _props.end(); re++) {
        if (re->second.type == NVProperty::T_STR)
            free(re->second.str);
        if (re->second.type == NVProperty::T_BLOB)
            delete[] (char *)re->second.data;
    }
    
    _props.clear();
}

int
NVProperty_SRAM::GetProperty(int key)
{
    map<int, PropertyEntry>::iterator it = _props.find(key);
    if(it != _props.end()) {
        switch (it->second.type) {
            case NVProperty::T_STR:
                return NVProperty::NVP_ENOENT;
    	        break;
            case NVProperty::T_BLOB:
                return NVProperty::NVP_ENOENT;
                break;
            default:
                return it->second.val32;
        }
    }
    return NVProperty::NVP_ENOENT;
};


int64_t
NVProperty_SRAM::GetProperty64(int key)
{
    map<int, PropertyEntry>::iterator it = _props.find(key);
    if(it != _props.end()) {
        switch (it->second.type) {
            case NVProperty::T_STR:
                return NVProperty::NVP_ENOENT;
                break;
            case NVProperty::T_BLOB:
                return NVProperty::NVP_ENOENT;
                break;
            default:
                return it->second.val64;
        }
    }
    return NVProperty::NVP_ENOENT;
}

const char *
NVProperty_SRAM::GetPropertyStr(int key)
{
    map<int, PropertyEntry>::iterator it = _props.find(key);
    if(it != _props.end()) {
        if (it->second.type == NVProperty::T_STR) {
            return (const char *)it->second.str;
        }
    }
    return NULL;
}


int
NVProperty_SRAM::SetProperty(int key, int64_t value, int type)
{
    map<int, PropertyEntry>::iterator it = _props.find(key);
    if(it != _props.end()) {
        it->second.val32 = value;
        return 0;
    }
    
    struct PropertyEntry r;
    memset(&r, 0, sizeof(r));
    r.key = key;
    r.type = type;
    if (type <= NVProperty::T_32BIT) {
        r.size = sizeof(r.val32);
        r.val32 = value;
    } else if (type == NVProperty::T_64BIT) {
        r.size = sizeof(r.val64);
        r.val64 = value;
    }
    
    _props.insert(std::pair<int,PropertyEntry> (key, r));
    
    return NVProperty::NVP_OK;
}

int
NVProperty_SRAM::SetPropertyStr(int key, const char *str, int type)
{
    map<int, PropertyEntry>::iterator it = _props.find(key);
    if(it != _props.end()) {
        if (it->second.str)
            free(it->second.str);
        it->second.str = strdup(str);
        it->second.size = strlen(str)+1;
        return NVProperty::NVP_OK;
    }
    
    struct PropertyEntry r;
    memset(&r, 0, sizeof(r));
    r.key = key;
    r.type = type;
    r.size = strlen(str)+1;
    r.str = strdup(str);
    
    _props.insert(std::pair<int,PropertyEntry> (key, r));
        
    return NVProperty::NVP_OK;
}

int
NVProperty_SRAM::SetPropertyBlob(int key, const void *blob, int size, int type)
{
    map<int, PropertyEntry>::iterator it = _props.find(key);
    if(it != _props.end()) {
        if (it->second.data)
        	delete[] (char *)it->second.data;
        it->second.size = size;
        it->second.data = new char[size];
        memcpy(it->second.data, blob, size);

        return NVProperty::NVP_OK;
    }
    
    struct PropertyEntry r;
    memset(&r, 0, sizeof(r));
    r.key = key;
    r.type = type;
    r.size = size;
    r.data = new char[size];
    memcpy(r.data, blob, size);
    _props.insert(std::pair<int,PropertyEntry> (key, r));
    
    return NVProperty::NVP_OK;
}

int
NVProperty_SRAM::EraseProperty(int key)
{
    map<int, PropertyEntry>::iterator it = _props.find(key);
    if(it == _props.end()) {
        return NVProperty::NVP_ENOENT;
    }
    if (it->second.type == NVProperty::T_STR)
        free((char *)it->second.data);
    if (it->second.type == NVProperty::T_BLOB)
        delete[] (char *)it->second.data;

    _props.erase(it->first);
    return NVProperty::NVP_OK;
}

int
NVProperty_SRAM::ReorgProperties(void)
{
    return NVProperty::NVP_OK;
}

int
NVProperty_SRAM::OpenPropertyStore(bool forWrite)
{
    return NVProperty::NVP_OK;
}

int
NVProperty_SRAM::ClosePropertyStore(bool flush)
{
    return NVProperty::NVP_OK;
}