/*
 * The file is licensed under the Apache License, Version 2.0
 * (c) 2017 Helmut Tschemernjak
 * 30826 Garbsen (Hannover) Germany
 */

#ifndef __NVPROPERTYPROVIDERINTERFACE__
#define __NVPROPERTYPROVIDERINTERFACE__

class NVPropertyProviderInterface {
public:
   	virtual ~NVPropertyProviderInterface() { }
    virtual int GetProperty(int key) = 0;
    virtual int64_t GetProperty64(int key) = 0;
    virtual const char *GetPropertyStr(int key) = 0;
    virtual int SetProperty(int key, int64_t value, int type) = 0;
    virtual int SetPropertyStr(int key, const char *str, int type) = 0;
    virtual int SetPropertyBlob(int key, const void *blob, int size, int type) = 0;
    virtual int EraseProperty(int key) = 0;
    virtual int ReorgProperties(void) = 0;
    virtual int OpenPropertyStore(bool forWrite = false) = 0;
    virtual int ClosePropertyStore(bool flush = false) = 0;
};

#endif // __NVPROPERTYPROVIDERINTERFACE__