/*
 * The file is licensed under the Apache License, Version 2.0
 * (c) 2017 Helmut Tschemernjak
 * 30826 Garbsen (Hannover) Germany
 */

#ifndef __NVPROPERTY_H__
#define __NVPROPERTY_H__

class NVProperty {
public:
    NVProperty();
    ~NVProperty();
public:
    enum NVPType {
        T_BIT	= 1,
        T_8BIT	= 2,
        T_16BIT	= 3,
        T_32BIT	= 4,
        T_64BIT	= 5,
        T_STR	= 6,
        T_BLOB	= 7,	/* blobs can be up to 255 bytes long */
    };
    
    enum NVPStore {
        S_OTP	= 0x01,
        S_FLASH	= 0x02,
        S_RAM	= 0x04,
    };
    
    enum NVPErrCode {
        NVP_OK = 0,
        NVP_NO_FLASH 	= -1,
        NVP_NO_RAM 		= -2,
        NVP_NO_STORE 	= -3,
        NVP_NO_PERM 	= -4,
        NVP_ERR_NOSPACE	= -5,
        NVP_ERR_FAIL	= -6,
        NVP_INVALD_PARM	= -7,
        NVP_ENOENT		= -0x12345687,
    };

    /*
     * Get property protocol version to allow
     * API differences between multiple versions
     */
    int GetVersion(void) { return 100; };

    /*
     * A simple GetProperty retuns its values as an int or int64
     * The order should always be S_RAM,S_FLASH, S_OTP
     */
    int GetProperty(int key, int defaultValue = 0);
    int64_t GetProperty64(int key, int64_t defaultValue = 0);
    const char *GetProperty(int key, const char *defaultValue = NULL);
    /*
     * when a block gets returned the buffer is filled up to the property
     * or max at the bsize length.
     */
    int GetProperty(int key, void  *buffer, int *size);
    /*
     * GetPropertySize will be helpful for getting blob or string sizes.
     */
    uint64_t GetPropertySize(int key);

    /*
     * SetProperty
     * It requires to use OpenPropertyStore and finally ClosePropertyStore(true)
     * to write out all properties.
     *
     */
    int SetProperty(int key, NVPType ptype, int64_t value, NVPStore store = S_FLASH);
    int SetProperty(int key, NVPType ptype, const char *value, NVPStore store = S_FLASH);
    int SetProperty(int key, NVPType ptype,  const void *blob, int length, NVPStore store = S_FLASH);
    
    int EraseProperty(int key, NVPStore store = S_FLASH);
    int ReorgProperties(NVPStore store = S_FLASH);
    int OpenPropertyStore(bool forWrite = false);
    int ClosePropertyStore(bool flush = false);

    enum Properties {
        RTC_AGING_CAL	= 1, // int8_t the RTC aging calibration value
        LORA_DEVICE_ID	= 2, // uint32_t the LoRa device ID
        LORA_CODE_ID	= 3, // uint32_t the Code for the RadioShuttle protocol
        ADC_VREF		= 4, // the adc refernce volatge in millivolt
        WIFI_SSID		= 5,
        WIFI_PASSWORD	= 6,
        
        PRIVATE_RANGE_START = 128,
        PRIVATE_RANGE_END 	= 254,
        MAX_PROPERTIES		= 255,
    };

private:
    NVPropertyProviderInterface *_ram;
    NVPropertyProviderInterface *_flash;
    NVPropertyProviderInterface *_otp;
    bool _allowWrite;
    bool _didOpen;
};

#endif
