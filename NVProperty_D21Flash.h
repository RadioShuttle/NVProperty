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

#ifndef __NVPROPERTY_D21FLASHE__
#define __NVPROPERTY_D21FLASHE__

class NVProperty_D21Flash : public NVPropertyProviderInterface {
public:
	NVProperty_D21Flash();
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
	
	static const int FLASH_ENTRY_HEADER	= 4;
	struct _flashEntry {
		uint8_t key;	// Property key
		union {
			struct {
				uint8_t deleted : 1;
				uint8_t type : 7;	 // NVPType
			} t;
		} ut;
		uint8_t len;	// length;
		union {
			struct {
				uint8_t f_padeven 		: 1; // the length has been padded to an even size;
				uint8_t f_str_zero_term : 1; // the string has a zero byte added.
				uint8_t f_t_bit    		: 1; // contains the bool value
				uint8_t f_reserv1  		: 5;
			} flags;
			int8_t	v_8bit;
		} u;
		union {
			int16_t v_16bit;
			int32_t v_32bit;
			int32_t v_64bit[2];	// use use 2 x 32-bit to avoid 64-bit struct padding
			char v_str[256];
			uint8_t v_blob[256];
		} data;
	};
	_flashEntry * _GetFlashEntry(int key);
	_flashEntry *_lastEntry;

	void _DumpAllEntires(void);
	int _pageSize;
	int _numPages;
	int _rowSize;
	uint8_t *_startAddress;
	uint8_t *_endAddress;

	static const int MAX_FLASH_PROP_SIZE = 2; // in kB
	static const int FLASH_PROP_MAGIC = 0x4c6f5261; // "LORA"
	static const int FLASH_PROP_VERSION = 2;
};

#endif // __NVPROPERTY_D21FLASHE__