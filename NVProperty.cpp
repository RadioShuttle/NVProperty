/*
 * The file is licensed under the Apache License, Version 2.0
 * (c) 2017 Helmut Tschemernjak
 * 30826 Garbsen (Hannover) Germany
 */


#ifdef ARDUINO
#ifndef __NVPROPERTY_H__
#define __NVPROPERTY_H__

#include <Arduino.h>
#include <NVPropertyInterface.h>
#include <NVSRAMProperty.h>
#include <NVFLASHProperty.h>
#include <NVProperty.h>

#ifdef ARDUINO_ARCH_ESP32
#include <soc/efuse_reg.h>
#endif

/*
 * NVS.h allows to store properties.
 */

int
NVProperty::GetProperty(int key, int defaultValue)
{
    int value = 0;
    int res;
    
    res = SRAM_GetProperty(key);	// RAM first
    if (res != NVP_ENOENT)
        return res;
    res = FLASH_GetProperty(key);	// FLASH second
    if (res != NVP_ENOENT)
        return res;

    // OTP third
    switch(key) {
#ifdef ARDUINO_ARCH_ESP32
        case RTC_AGING_CAL: {
            uint32_t *val = (uint32_t *)EFUSE_BLK3_RDATA6_REG;
            uint32_t v = (*val & 0xff000000) >> 24;
            if (v == 0xff || v == 0)
            	return defaultValue;
            value = v;
        }
        break;
        case LORA_DEVICE_ID: {
            uint32_t *val = (uint32_t *)EFUSE_BLK3_RDATA6_REG;
            uint32_t v = (*val & 0x00ffffff);
            if (v == 0x00ffffff || v == 0)
            	return defaultValue;
            value = v;
        }
        break;
        case LORA_CODE_ID: {
            uint32_t *val = (uint32_t *)EFUSE_BLK3_RDATA7_REG;
            if (*val == 0xffffffff || *val == 0)
	            return defaultValue;
            value = *val;
        }
        case ADC_VREF: {
            uint32_t stepSize = 7;
            uint32_t signbit = 0x10;
            uint32_t databits = 0x0f;
            int stepsize = 7;
            uint32_t *val = (uint32_t *)EFUSE_BLK0_RDATA4_REG;
            uint32_t v = (*val >> 8) & 0x1f;
            bool sign = v & signbit;
            if (sign)
            	v = -((v & databits));
            else
            	v = v & databits;
            if (!v)
            	v = 1100;
            else
            	v = 1100 + (v * stepsize);
            value = v;
        }
        break;
    	default:
            value =  defaultValue;
#else
    	default:
    		value =  defaultValue;
#endif
    }
    return value;
}


int64_t
NVProperty::GetProperty64(int key, int64_t defaultValue)
{
    int64_t res;
    
    res = SRAM_GetProperty64(key);	// RAM first
    if (res != NVP_ENOENT)
        return res;
    res = FLASH_GetProperty64(key);	// FLASH second
    if (res != NVP_ENOENT)
        return res;
    
    return defaultValue;
}

const char *
NVProperty::GetProperty(int key, const char *defaultValue)
{
    const char *res;
    
    res = SRAM_GetPropertyStr(key);	// RAM first
    if (res)
        return res;
    res = FLASH_GetPropertyStr(key);	// FLASH second
    if (res)
        return res;
    
    return defaultValue;
}

int
NVProperty::GetProperty(int key, void  *buffer, int bsize)
{
    // TODO
    // how to return the size?
    return NVP_OK;
}


uint64_t
NVProperty::GetPropertySize(int key)
{
    return 0;
}

int
NVProperty::SetProperty(int key, NVPType ptype, int64_t value, NVPStore store)
{
    if (store == S_OTP) {
        
    } else if (store == S_FLASH) {
        return FLASH_SetProperty(key, value, ptype);
    } else if (store == S_RAM) {
        switch(ptype) {
            case T_32BIT:
                return SRAM_SetProperty(key, value, ptype);
                break;
            case T_STR:
                return NVP_ERR_FAIL;
            case T_BLOB:
                return NVP_ERR_FAIL;
                
            default:
                return NVP_ERR_FAIL;
        }
        return NVP_OK;
    }
    return NVP_ERR_FAIL;
}


int
NVProperty::SetProperty(int key, NVPType ptype, const char *value, NVPStore store)
{
	if (store == S_FLASH) {
    	return FLASH_SetPropertyStr(key, value, ptype);
	} else if (store == S_RAM) {
        return SRAM_SetPropertyStr(key, value, ptype);
    }
    return NVP_NO_STORE;
}

int
NVProperty::SetProperty(int key, NVPType ptype,  const void *blob, int length, NVPStore store)
{
    return NVP_OK;
}

int
NVProperty::EraseProperty(int key, NVPStore store)
{
    if (store == S_OTP) {
        return NVP_NO_PERM;
    } else if (store == S_FLASH) {
        return FLASH_EraseProperty(key);
    } else if (store == S_RAM) {
            return SRAM_EraseProperty(key);
    }
    return NVP_OK;
}

int
NVProperty::ReorgProperties(NVPStore store)
{
    return NVP_OK;
}

int
NVProperty::OpenPropertyStore(bool forWrite)
{
    FLASH_OpenPropertyStore(forWrite);
    
    return NVP_OK;
}

int
NVProperty::ClosePropertyStore(bool flush)
{
    FLASH_ClosePropertyStore();
    return NVP_OK;
}


#endif // __NVPROPERTY_H__

#endif


