/*
 * This is an unpublished work copyright
 * (c) 2017 Helmut Tschemernjak
 * 30826 Garbsen (Hannover) Germany
 *
 *
 * Use is granted to registered RadioShuttle licensees only.
 * Licensees must own a valid serial number and product code.
 * Details see: www.radioshuttle.de
 */

#ifdef ARDUINO_ARCH_ESP32

#include <Arduino.h>
#include <NVPropertyProviderInterface.h>
#include <NVProperty_ESP32efuse.h>
#include <NVProperty.h>

#include <soc/efuse_reg.h>

/*
 * NVS.h allows to store properties.
 */
int
NVProperty_ESP32efuse::GetProperty(int key)
{
    int value;
    
    switch(key) {
        case NVProperty::RTC_AGING_CAL: {
            uint32_t RTC_SIGN_BIT = 0x80;
            uint32_t *val = (uint32_t *)EFUSE_BLK3_RDATA6_REG;
            uint32_t v = (*val & 0xff000000) >> 24;
            if (v == 0xff || v == 0)
                return NVProperty::NVP_ENOENT;
            if (v & RTC_SIGN_BIT) {
                v = -(v & (RTC_SIGN_BIT-1));
            }
            value = v;
        }
        break;
        case NVProperty::ADC_VREF: {
            uint32_t stepsize = 7;
            uint32_t signbit = 0x10;
            uint32_t databits = 0x0f;
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
        case NVProperty::HARDWARE_REV: {
            /*
             * TODO: Find a place for our hardware revision
             * maybe we can code features like LoRa, RTC, Booster, etc.
             * For the ESP32 8 bit storage should be sufficient.
             */
            value = 0;
        }
        break;
        case NVProperty::LORA_DEVICE_ID: {
            uint32_t *val = (uint32_t *)EFUSE_BLK3_RDATA6_REG;
            uint32_t v = (*val & 0x00ffffff);
            if (v == 0x00ffffff || v == 0)
                return NVProperty::NVP_ENOENT;
            value = v;
        }
            break;
        case NVProperty::LORA_CODE_ID: {
            uint32_t *val = (uint32_t *)EFUSE_BLK3_RDATA7_REG;
            if (*val == 0xffffffff || *val == 0)
                return NVProperty::NVP_ENOENT;
            value = *val;
        }
            break;
    	default:
            return NVProperty::NVP_ENOENT;
    }
    return value;
}



int64_t
NVProperty_ESP32efuse::GetProperty64(int key)
{
    return NVProperty::NVP_ENOENT;
}

const char *
NVProperty_ESP32efuse::GetPropertyStr(int key)
{
    return NULL;
}

int
NVProperty_ESP32efuse::GetPropertyBlob(int key, const void *blob, int *size)
{
    return NVProperty::NVP_ENOENT;
}

int
NVProperty_ESP32efuse::SetProperty(int key, int64_t value, int type)
{
    return NVProperty::NVP_ERR_FAIL;
}


int
NVProperty_ESP32efuse::SetPropertyStr(int key, const char *value, int type)
{
    return NVProperty::NVP_ERR_FAIL;
}

int
NVProperty_ESP32efuse::SetPropertyBlob(int key, const void *blob, int size, int type)
{
    return NVProperty::NVP_ERR_FAIL;
}

int
NVProperty_ESP32efuse::EraseProperty(int key)
{
    return NVProperty::NVP_NO_PERM;
}

int
NVProperty_ESP32efuse::ReorgProperties(void)
{
    return NVProperty::NVP_OK;
}


int
NVProperty_ESP32efuse::OpenPropertyStore(bool forWrite)
{
    if (forWrite)
        return NVProperty::NVP_NO_PERM;
    return NVProperty::NVP_OK;
}

int
NVProperty_ESP32efuse::ClosePropertyStore(bool flush)
{
    return NVProperty::NVP_OK;
}

#endif // ARDUINO_ARCH_ESP32
