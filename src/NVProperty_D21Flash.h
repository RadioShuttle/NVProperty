/*
 * Copyright (c) 2019 Helmut Tschemernjak
 * 30826 Garbsen (Hannover) Germany
 * Licensed under the Apache License, Version 2.0);
 */

#ifndef __NVPROPERTY_D21FLASHE_H__
#define __NVPROPERTY_D21FLASHE_H__

class NVProperty_D21Flash : public NVPropertyProviderInterface {
public:
	NVProperty_D21Flash(int propSizekB, bool erase);
	~NVProperty_D21Flash();
	
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
	
private:
	void _FlashInititalize(bool force = false);
	void _FlashEraseRow(int startRow, int count = 1);
	void _FlashWrite(uint8_t *address, const void *data, size_t length);
	bool _FlashIsCleared(uint8_t *address, int len);
	void _FlashWritePage(int page, int offset, uint8_t *data, int length);

	struct _flash_header {
		uint32_t magic;
		uint16_t version;
		uint16_t sizeKB;
	};
	
	static const int FLASH_ENTRY_HEADER			= 4;
	static const int FLASH_ENTRY_HEADER_SHORT	= 2;
	static const int MAX_DATA_ENTRY				= 256;
	
	struct _flashEntry {
		uint8_t key;	// Property key
		struct {
			uint8_t deleted	: 1; // this key has been deleted
			uint8_t t_bit  	: 1; // contains the bool value
			uint8_t reserv1	: 1; //
			uint8_t reserv2	: 1; //
			uint8_t type   	: 4; // NVPType
		} t;
		union {
			int16_t v_16bit;
			int8_t	v_8bit;
			struct {
				uint8_t d_len;				 // data length
				uint8_t f_reserv1  		: 8;
			} option;
		} u;
		union {
			int32_t v_32bit;
			int32_t v_64bit[2];	// use use 2 x 32-bit to avoid 64-bit struct padding
			char v_str[MAX_DATA_ENTRY];
			uint8_t v_blob[MAX_DATA_ENTRY];
		} data;
	};
	
	_flashEntry * _GetFlashEntry(int key, uint8_t *start = NULL);
	int _GetFlashEntryLen(_flashEntry *k);
	int _GetFlashPaddingSize(int len);
	_flashEntry *_lastEntry;
	void _DumpAllEntires(void);
	int _FlashReorgEntries(int minRequiredSpace);
	bool _debug;
	int _propSizekB;
	int _pageSize;
	int _numPages;
	int _rowSize;
	uint8_t *_startAddress;
	uint8_t *_endAddress;

	static const int FLASH_PROP_MAGIC = 0x4c6f5261; // "LORA"
	static const int FLASH_PROP_VERSION = 3;
	static const uint8_t _flashErasedValue = 0xff;
	static const int _bootlLoaderSize = 8192;
	static const int _lockRegions = 16;	// d21 lock regions are always 16 for the entire ram


};

#endif // __NVPROPERTY_D21FLASHE_H__
