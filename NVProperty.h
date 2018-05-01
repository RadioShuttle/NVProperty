/*
 * The file is licensed under the Apache License, Version 2.0
 * (c) 2017 Helmut Tschemernjak
 * 30826 Garbsen (Hannover) Germany
 */


class NVProperty : public NVSRAMProperty, public NVFLASHProperty, public NVPropertyInterface {
public:

    virtual int GetProperty(int key, int defaultValue = 0);
    virtual int64_t GetProperty64(int key, int64_t defaultValue = 0);
    virtual const char *GetProperty(int key, const char *defaultValue = NULL);
    virtual int GetProperty(int key, void  *buffer, int len);
    virtual uint64_t GetPropertySize(int key);

    virtual int SetProperty(int key, NVPType ptype, int64_t value, NVPStore store = S_FLASH);
    virtual int SetProperty(int key, NVPType ptype, const char *value, NVPStore store = S_FLASH);
    virtual int SetProperty(int key, NVPType ptype,  const void *blob, int length, NVPStore store = S_FLASH);
    
    virtual int EraseProperty(int key, NVPStore store = S_FLASH);
    virtual int ReorgProperties(NVPStore store = S_FLASH);
    virtual int OpenPropertyStore(bool forWrite = false);
    virtual int ClosePropertyStore(bool flush = false);
};
