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
        ADC_VREF		= 2, // the adc refernce volatge in millivolt
        HARDWARE_REV	= 3, // the hardware revision of the board
        
        LORA_DEVICE_ID	= 10, // uint32_t the LoRa device ID
        LORA_CODE_ID	= 11, // uint32_t the Code for the RadioShuttle protocol
        LORA_REMOTE_ID	= 12, // specifies the server address
        LORA_REMOTE_ID_ALT = 13, // specifies the alternate server address
        LORA_FREQUENCY 	= 14,	// channel frequency in Hz, e.g. 868100000
        LORA_BANDWIDTH	= 15,	// channel bandwidth in Hz, e.g. 125000
        LORA_SPREADING_FAKTOR = 16, // e.g. 7
        LORA_TXPOWER	= 17,	// e.g. 14 for 15 dBm.
        LORA_FREQUENCY_OFFSET = 18,
        LORA_AES_KEY	= 19,	// AES keys are per app, there are only two placeholders
        LORA_AES_KEY_ALT = 20,
        
        LOC_LONGITUDE	= 25,	// a string
        LOC_LATITUDE 	= 26,	// a string
        LOC_NAME 		= 27, 	// a string with the location name
        HOSTNAME 		= 28,	// the device host name
        
        WIFI_SSID		= 30,
        WIFI_PASSWORD	= 31,
        WIFI_SSID_ALT	= 32,
        WIFI_PASSWORD_ALT = 33,
        USE_DHCP		= 34,
        MAC_ADDR		= 35,
        NET_IP_ADDR		= 36,
        NET_IP_ROUTER	= 37,
        NET_IP_DNS_SERVER = 38,
        
        MQTT_SERVER		= 40,
        MQTT_SERVER_ALT	= 41,
        
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
