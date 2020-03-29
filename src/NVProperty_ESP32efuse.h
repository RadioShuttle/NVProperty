/*
 * Copyright (c) 2019 Helmut Tschemernjak
 * 30826 Garbsen (Hannover) Germany
 * Licensed under the Apache License, Version 2.0);
 */

#ifndef __NVPROPERTY_ESP32EFUSE_H__
#define __NVPROPERTY_ESP32EFUSE_H__

class NVProperty_ESP32efuse : public NVPropertyProviderInterface {
public:
    virtual int GetProperty(int key);
    virtual int64_t GetProperty64(int key);
    virtual int GetPropertyBlob(int key, const void *blob, int *size);
    virtual const char *GetPropertyStr(int key);
    virtual int SetProperty(int key, int64_t value, int type);
    virtual int SetPropertyStr(int key, const char *value, int type);
    virtual int SetPropertyBlob(int key, const void *blob, int size, int type);
    virtual int EraseProperty(int key);
    virtual int ReorgProperties(void);
    virtual int OpenPropertyStore(bool forWrite = false);
    virtual int ClosePropertyStore(bool flush = false);
};

#endif // __NVPROPERTY_ESP32EFUSE_H__
