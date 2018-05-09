/*
 * The file is licensed under the Apache License, Version 2.0
 * (c) 2017 Helmut Tschemernjak
 * 30826 Garbsen (Hannover) Germany
 */

#ifndef __NVSProperty_SRAM__
#define __NVSProperty_SRAM__


#ifdef ARDUINO
#undef min
#undef max
#undef map
using namespace std;
#include <map>
#define map	std::map // map clashes with Arduino map()
#endif


class NVProperty_SRAM : public NVPropertyProviderInterface {
public:
    NVProperty_SRAM();
    ~NVProperty_SRAM();
    virtual int GetProperty(int key);
    virtual int64_t GetProperty64(int key);
    virtual const char *GetPropertyStr(int key);
    virtual int SetProperty(int key, int64_t value, int type);
    virtual int SetPropertyStr(int key, const char *value, int type);
    virtual int SetPropertyBlob(int key, const void *blob, int size, int type);
    virtual int EraseProperty(int key);
    virtual int ReorgProperties(void);
    virtual int OpenPropertyStore(bool forWrite = false);
    virtual int ClosePropertyStore(bool flush = false);

private:
    struct PropertyEntry {
        int key;
        uint8_t type;
        uint8_t size;
        union {
            int val32;
            int64_t val64;
            void *data;
            char *str;
        };
    };
    map<int, PropertyEntry> _props;
};

#endif // __NVSProperty_SRAM__