# NVProperty – non volatile settings library

A property storage system for storing non volatile settings in MCU flash or OTP memory. The storage is highly memory and flash size optmized. It supports keys and variable data types like bits, numbers, strings and blobs. There are pre-defined enums for storage keys (8-bit, 1-127, e.g. WIFI_SSID = 30) and custom keys (128-254). It supports intelligent re-writing of properties in flash memory to overcome limitations on limited flash write/erase cycles in MCUs, a kind of wear levelling done in software via the NVProperty library.

The NVProperty has a general API interface and supports multiple MCUs types. Additional MCU models can be added by knowledge programmers. The API is very simple to use and is identical on Arduino and Mbed OS, independent what kind of MCU is being used. A property editor allows to set properties without any programming.

CODE as simple as this:
NVProperty prop;
const char *ssid = prop.GetProperty(prop.WIFI_SSID);
int value =  prop.GetProperty(prop.ADC_VREF);

Helmut Tschemernjak
www.radioshuttle.de

## Supported platforms
- Mbed OS
- Arduino ESP32, D21

## Technical background
The NVProperty storage is usually at the end of the flash memory e.g. 16 kb, assuming that the application does not reach the end of the flash area. Even the NVProperty is optimized to distribute writes across its storage area, the number of writes should be limited to avoid flash erase cycle limitations.

The following table gives an estimate for a 32kB NVProperty storage to show how many writes of a simple number (e.g. SetPropertyNET_IP_ADDR, T_32BIT, ...) are possible before reaching the end of life for the MCU flash chip.

- Microchoip D21:	xxxx million writes
- STM32L4:		xxxx million writes
- ESP32: 		(unkown because the ESP32 uses a different store backend) 

Please note: Larger NVProperty storage area will allow better write distribution, larger storage values, e.g. Strings will reduce the number of writes. It is recommended to write properties only when it is needed or collect changes and write it only from time to time.


## TODOs
- Add ESP32 efuse (OTP) memory writes

##  Credits
This driver has initially been written by the RadioShuttle engineers (www.radioshuttle.de). Many thanks to everyone who helped bringing this project forward.

##  Links
- Training video: "RadioShuttle - Non Volatile Settings (NVProperty)" https://www.youtube.com/watch?v=8zlBuu4SDGY
