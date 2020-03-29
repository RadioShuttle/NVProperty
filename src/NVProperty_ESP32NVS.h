/*
 * Copyright (c) 2019 Helmut Tschemernjak
 * 30826 Garbsen (Hannover) Germany
 * Licensed under the Apache License, Version 2.0);
 */

#ifndef __NVPROPERTY_ESP32NVS_H__
#define __NVPROPERTY_ESP32NVS_H__


class NVProperty_ESP32NVS : public NVPropertyProviderInterface {
public:
    NVProperty_ESP32NVS();
    ~NVProperty_ESP32NVS();
    virtual int GetProperty(int key);
    virtual int64_t GetProperty64(int key);
    virtual const char *GetPropertyStr(int key);
    virtual int GetPropertyBlob(int key, const void *blob, int *size);
    virtual int SetProperty(int key, int64_t value, int type);
    virtual int SetPropertyStr(int key, const char *value, int type);
    virtual int SetPropertyBlob(int key, const void *blob, int size, int type);
    virtual int EraseProperty(int key);
    virtual int ReorgProperties(void);
    virtual int OpenPropertyStore(bool forWrite = false);
    virtual int ClosePropertyStore(bool flush = false);

private:
    char *_setKey(int key) {
        memset(_tmpKey, 0, sizeof(_tmpKey));
        itoa(key, &_tmpKey[0], 10);
        return _tmpKey;
    };
    char _tmpKey[11];
    uint32_t _handle;
    bool _didWrite;
};

#endif // __NVPROPERTY_ESP32NVS_H__
