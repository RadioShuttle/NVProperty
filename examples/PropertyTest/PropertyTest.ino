/*
 * Copyright (c) 2019 Helmut Tschemernjak
 * 30826 Garbsen (Hannover) Germany
 * Licensed under the Apache License, Version 2.0);
 */

#include "NVPropertyProviderInterface.h"
#include "NVProperty.h"


#if defined(ARDUINO_SAMD_ZERO) || defined(ARDUINO_ARCH_SAMD)
#define MYSERIAL SerialUSB
#else
#define MYSERIAL Serial
#endif


void setup() {
  MYSERIAL.begin(115200);
  while (!MYSERIAL)
    ; // wait for serial port to connect. Needed for native USB port only

  NVProperty prop;
  
  MYSERIAL.println("NVProperty Version: " + String(prop.GetVersion()));
  /*
   * Here are some some basic Properties stored in the ESP32 OTP memory (EFuse)
   * (OPT = one time programmable )
   * The NVProperty system will automatically look in the following order:
   * - if the key is stored in the S_RAM, return the S_RAM property value
   * - is the key is stored in the S_FLASH, return the S_FLASH property value
   * - is the key is stored in the S_OPT, return the S_OPT property value
   */
  MYSERIAL.println("Testing OTP (eFuse) properties");
  MYSERIAL.println("ADC_VREF millivolts: " + String(prop.GetProperty(prop.ADC_VREF, 1100)));
  MYSERIAL.println("RTC_AGING_CAL: "  + String(prop.GetProperty(prop.RTC_AGING_CAL, 0)));
  MYSERIAL.println("LORA_DEVICE_ID: " + String(prop.GetProperty(prop.LORA_DEVICE_ID, 0)));
  MYSERIAL.print("LORA_CODE_ID: 0x");  MYSERIAL.println(prop.GetProperty(prop.LORA_CODE_ID, 0), HEX);
  MYSERIAL.println("\n");

  /*
   * The set SetProperty ID 11 will fail because it is not opened for read-write
   * Setting a property requires to specify the backend, at present 
   * S_RAM will be valid only until the next reset or when the NVProperty object is released
   * S_FLASH will be permenent stored in a revered MCU flash area which is prevered when uploading new Sketches
   * S_OPT will be store permently in the OTP memory (efuse for the ESP32).
   * 
   * negative return values point to an error, see NVProperty for the error names.
   */
  MYSERIAL.println("Testing RAM 32BIT properties");
  int key = NVProperty::PRIVATE_RANGE_START; // 128
  MYSERIAL.println("SetProperty: key=" + String(key) + " value=0x1234 returns: " + String( prop.SetProperty(key, prop.T_32BIT, 0x1234, prop.S_RAM)));
  /*
   * By default, the proper store is automatically opened for reading,
   * and must be opened for read-write when needed.
   */
   
  MYSERIAL.println("OpenPropertyStore: for write, returns " + String(prop.OpenPropertyStore(true))); // enable for write
  /*
   * Set the property as an INT32 and verify that it is available when reading.
   */
  MYSERIAL.println("SetProperty: key=" + String(key) + " value=0x1234 returns: " + String( prop.SetProperty(key, prop.T_32BIT, 0x1234, prop.S_RAM)));
  MYSERIAL.print("GetProperty: key=" + String(key) + " returns 0x");  MYSERIAL.println(prop.GetProperty(key, 0), HEX);
  /*
   * Erase the property and verify that it is not available anymore.
   */
  MYSERIAL.println("EraseProperty: key=" + String(key) + " returns: " + String(prop.EraseProperty(key, prop.S_RAM)));
  MYSERIAL.println("GetProperty: key=" + String(key) + " returns: " + String(prop.GetProperty(key, 0)));
  MYSERIAL.println("\n");

  
  /*
   * Another test using the flash memory to store properties
   * Please not that it is good to limit the number or writes into flash memory
   * because most flash in the MCU supports only a few thousand writes
   * The following test will write int32 values.
   */
  key = NVProperty::PRIVATE_RANGE_START + 1; // 129
  MYSERIAL.println("Testing FLASH 32BIT properties");
  MYSERIAL.print("GetProperty: key=" + String(key) + " returns: 0x"); MYSERIAL.println(prop.GetProperty(key, 0), HEX);
  MYSERIAL.println("OpenPropertyStore: for write, returns " + String(prop.OpenPropertyStore(true))); // enable for write
  MYSERIAL.println("SetProperty: key=" + String(key) + "  value=0x5678 returns " + String(prop.SetProperty(key, prop.T_32BIT, 0x5678, prop.S_FLASH)));
  MYSERIAL.print("GetProperty: key=" + String(key) + " returns 0x") , MYSERIAL.println(prop.GetProperty(key, 0), HEX); 
  MYSERIAL.println("EraseProperty: key=" + String(key) + " returns: " + String(prop.EraseProperty(key, prop.S_FLASH)));
  MYSERIAL.print("GetProperty: key=" + String(key) + " returns 0x"); MYSERIAL.println(prop.GetProperty(key, 0), HEX);
  MYSERIAL.println("\n");

  /*
   * New test case with strings
   */
  key = NVProperty::PRIVATE_RANGE_START + 2; // 130
  MYSERIAL.println("Testing RAM string properties");
  MYSERIAL.println("GetProperty: key=" + String(key) + " returns " + String(prop.GetProperty(key, "Not found")));
  MYSERIAL.println("SetProperty: key=" + String(key) + " value=Hello World! returns: " + String(prop.SetProperty(33, prop.T_STR, "Hello World!", prop.S_RAM)));
  MYSERIAL.println("GetProperty: key=" + String(key) + " returns " + String(prop.GetProperty(key, "Not found")));
  MYSERIAL.println("\n");

  key = NVProperty::PRIVATE_RANGE_START + 3; // 131
  MYSERIAL.println("Testing FLASH string properties");
  MYSERIAL.println("GetProperty: key=" + String(key) + " returns " + String(prop.GetProperty(key, "Not found")));
  MYSERIAL.println("EraseProperty: key=" + String(key) + " returns " + String(prop.EraseProperty(key)));
  MYSERIAL.println("GetProperty: key=" + String(key) + " returns " + String(prop.GetProperty(key, "Not found")));
  MYSERIAL.println("OpenPropertyStore: for write, returns " + String(prop.OpenPropertyStore(true))); // enable for write
  MYSERIAL.println("SetProperty: key=" + String(key) + " value=Hello World! returns " + String(prop.SetProperty(key, prop.T_STR, "Hello World!", prop.S_FLASH)));
  MYSERIAL.println("GetProperty: key=" + String(key) + " returns " + String(prop.GetProperty(key, "Not found")));

  // TODO Blobs
  
  MYSERIAL.println("\nTesting completed");
}

void loop() {
}
