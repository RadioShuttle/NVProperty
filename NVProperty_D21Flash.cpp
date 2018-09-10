/*
 * This is an unpublished work copyright
 * (c) 2018 Helmut Tschemernjak
 * 30826 Garbsen (Hannover) Germany
 *
 *
 * Use is granted to registered RadioShuttle licensees only.
 * Licensees must own a valid serial number and product code.
 * Details see: www.radioshuttle.de
 */


#if defined(ARDUINO_SAMD_ZERO) || defined(ARDUINO_ARCH_SAMD)

#include <Arduino.h>
#undef min
#undef max
#undef map
#include <algorithm>    // std::max
#include <bitset>
#include <NVPropertyProviderInterface.h>
#include <NVProperty_D21Flash.h>
#include <NVProperty.h>

#include "arduino-util.h"

NVProperty_D21Flash::NVProperty_D21Flash(int propSizekB) {
	_debug = true;
	_propSizekB = propSizekB;
	_pageSize = 8 << NVMCTRL->PARAM.bit.PSZ;
	_numPages = NVMCTRL->PARAM.bit.NVMP;
	_rowSize = _pageSize * 4;
	_startAddress = (uint8_t*)0 + ((_numPages-(_propSizekB * 1024)/_pageSize) * _pageSize);
	_endAddress = _startAddress + (_propSizekB * 1024);
	_lastEntry = NULL;
	
	if (_debug) {
		dprintf("_propSizekB: %d kB", _propSizekB);
		dprintf("_pageSize: %d", _pageSize);
		dprintf("_numPages: %d", _numPages);
		dprintf("_rowSize: %d", _rowSize);
		dprintf("PageOffset: %d", _numPages-((_propSizekB * 1024)/_pageSize));
		dprintf("total: %d", _pageSize * _numPages);
		dprintf("_startAddress: %d", _startAddress);
	}
	_FlashInititalize();
	_GetFlashEntry(0); // inits the _lastEntry record
	if (_debug)
		_DumpAllEntires();
}

NVProperty_D21Flash::~NVProperty_D21Flash() {
}

#if 0
const char *testmsg = \
	"Helmut Tschemernjak Am Kahlen Berg 19 30826 Garbsen" \
	"Monique Saaber Schaeferdamm 32 30827 Garbsen" \
	"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" \
	"BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB" \
	"CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC" \
	"DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD" \
	"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE" \
	"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF";
#endif

void
NVProperty_D21Flash::_FlashInititalize(bool force)
{

	if (_debug) {
		dump("1st-row:", _startAddress, _rowSize);
		dump("2st-row:", _startAddress + _rowSize, _rowSize);
	}

	_flash_header *fh = (_flash_header *)_startAddress;
	if (fh->magic == FLASH_PROP_MAGIC && fh->version == FLASH_PROP_VERSION && fh->sizeKB == _propSizekB) {
		if (_debug)
			dprintf("Flash OK");
		if (!force)
			return;
	}
	if (_debug)
		dprintf("Formatting Flash");
	_flash_header f;
	memset(&f, 0, sizeof(f));
	f.magic = FLASH_PROP_MAGIC;
	f.version = FLASH_PROP_VERSION;
	f.sizeKB = _propSizekB;
	
	int count = (_propSizekB * 1024) / _rowSize;
	int startRow = (int)_startAddress / _rowSize;
	_FlashEraseRow(startRow, count);
	_FlashWrite(_startAddress, &f, sizeof(f));

	// _FlashWrite(_startAddress + _rowSize, testmsg, strlen(testmsg));

	if (_debug) {
		dump("1st-row:", _startAddress, _rowSize);
		// dump("2st-row:", _startAddress + _rowSize, _rowSize * 2);
	}
}


void
NVProperty_D21Flash::_FlashEraseRow(int startRow, int count)
{
	// dprintf("_FlashEraseRow: startRow=%d, count=%d", startRow, count);
	
	for(int i = 0; i < count; i++) {
		uint32_t *startAddr = (uint32_t *)((startRow + i) * _rowSize);
		uint32_t *addr = startAddr;
		bool foundData = false;
		for (int offset = 0; offset < _rowSize; offset += sizeof(uint32_t)) {
			if (*addr++ != 0xffffffff) {
				foundData = true;
				break;
			}
		}
		if (_debug)
			dprintf("_FlashEraseRow: addr=0x%x, count=%d (%s)", startAddr, i,
					foundData ? "erased" : "skipped");
		if (!foundData)
			continue;
		NVMCTRL->STATUS.reg |= NVMCTRL_STATUS_MASK; // Clear error flags
		NVMCTRL->ADDR.reg = (uint32_t)((startRow + i) *_rowSize) / 2;
		NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_ER;
		while (!NVMCTRL->INTFLAG.bit.READY)
			;
	}
}

/*
 * Find out start page, number of pages
 * Check if the page contins FF's than write, otherwise erase first
 */
void
NVProperty_D21Flash::_FlashWrite(uint8_t *address, const void *d, size_t length)
{
	uint8_t *data = (uint8_t *)d;
	
	if (address < _startAddress || address > _startAddress + (_pageSize * _numPages))
		return;
	
	int done = 0;
	
	do {
		uint32_t startPage = (uint32_t)(address + done) / _pageSize;
		int pageOffset = (uint32_t)(address + done) % _pageSize;
		int pageWriteSize = _pageSize - pageOffset;
		
		if (_FlashIsCleared((uint8_t *)(startPage * _pageSize) + pageOffset, pageWriteSize)) {
			// single page write
			int writeLength = std::min(pageWriteSize, (int)length);
			_FlashWritePage(startPage, pageOffset, data, writeLength);
			length -= writeLength;
			done += writeLength;
			data += writeLength;
		} else {
			// row write
			// load row copy
			// erase row
			// merge in new data
			// write row in page copies
			uint32_t startRow = (uint32_t)(address + done) / _rowSize;
			int rowOffset = (uint32_t)(address + done) - (startRow * _rowSize);
			int cplen = std::min((int)length, _rowSize - rowOffset);
			uint8_t *saveddata = new uint8_t[_rowSize];
			if (!saveddata)
				return;

			memcpy(saveddata, (uint8_t *)(startRow * _rowSize), _rowSize);
			// dprintf("startRow=%d rowOffset=%d, cplen=%d", startRow, rowOffset, cplen);
			
			memcpy(saveddata + rowOffset, data, cplen);
			
			_FlashEraseRow(startRow);
			for (int i = 0; i < _rowSize/_pageSize; i++) {
				_FlashWritePage(((startRow * _rowSize) / _pageSize) + i, 0, saveddata + (i * _pageSize), _pageSize);
			}
			length -= cplen;
			done += cplen;
			data += cplen;

			delete[] saveddata;
		}
	} while(length > 0);
}


bool
NVProperty_D21Flash::_FlashIsCleared(uint8_t *address, int len)
{
	while (len > 0) {
		if (*address++ != 0xff) {
			return false;
		}
		len--;
	}
	return true;
}


// TODO trim data to skip leading and trailing 0xff.
void
NVProperty_D21Flash::_FlashWritePage(int page, int offset, uint8_t *data, int length)
{
	uint8_t *addr = (uint8_t *)(page * _pageSize) + offset;
	if (length < 1)
		return;
	
	bool oddAddress = offset & 1;
	
  	NVMCTRL->CTRLB.bit.MANW = 1;	// Disable automatic page write to allow partial page writes.
    NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_PBC; // page buffer clear
    while (!NVMCTRL->INTFLAG.bit.READY)
		;

	// writing must be done in 16-bit or 32bit alignments for the NVM
	while (length > 0) {
		uint16_t *adr16;
		
		if (oddAddress) {
			addr--;
			adr16 = (uint16_t *)addr;
			*adr16 = *data++ << 8 | *addr;
			addr += sizeof(uint16_t);
			length -= 1;
			oddAddress = false;
		} else {
			adr16 = (uint16_t *)addr;
			if (length == 1)
				*adr16 = *data++ | 0xff << 8;
			else {
				uint8_t ih = *data++;
				uint8_t il = *data++;
				*adr16 = ih | il << 8;
			}
			addr += sizeof(uint16_t);
			length -= sizeof(uint16_t);
		}
	}
	
    NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_WP; // write page
    while (!NVMCTRL->INTFLAG.bit.READY)
		;
}


NVProperty_D21Flash::_flashEntry *
NVProperty_D21Flash::_GetFlashEntry(int key, uint8_t *start)
{
	_flashEntry *p;

	if (start)
		p = (_flashEntry *)start;
	else
		p = (_flashEntry *)(_startAddress + sizeof(_flash_header));
	_flashEntry *lastP = NULL;
	while(true) {
		if ((uint8_t*)p >= _endAddress || p->key == NVProperty::PROPERTIES_EOF) {
			if ((uint8_t*)p <= _endAddress)
				_lastEntry = p;
			if (!lastP || lastP->ut.t.deleted)
				return NULL;
			break;
		}
		if (p->key == key)
			lastP = p;

		int len = p->len;
		if (p->ut.t.type == NVProperty::T_STR || p->ut.t.type == NVProperty::T_BLOB) {
			if (p->u.flags.f_str_zero_term)
				len++;
			if (p->u.flags.f_padeven)
				len++;
		}
		if (p->ut.t.type == NVProperty::T_BIT || p->ut.t.deleted)
			p = (_flashEntry *)((uint8_t *)p + FLASH_ENTRY_HEADER_SHORT);
		else
			p = (_flashEntry *)((uint8_t *)p + FLASH_ENTRY_HEADER + len);
	}
	return lastP;
}

void
NVProperty_D21Flash::_DumpAllEntires(void)
{
	if (!_debug)
		return;
	
	dprintf("------------- DumpAllEntires -------- ");

	int index = 0;
	_flashEntry *p = (_flashEntry *)(_startAddress + sizeof(_flash_header));
	while((uint8_t *)p < _endAddress && p->key != 0xff) {

		int64_t value = 0;
    	switch(p->ut.t.type) {
		case NVProperty::T_BIT:
			if (p->ut.t.t_bit)
				value = 1;
			else
				value = 0;
			break;
		case NVProperty::T_8BIT:
			value = p->u.v_8bit;
			break;
		case NVProperty::T_16BIT:
			value = p->data.v_16bit;
			break;
		case NVProperty::T_32BIT:
			{
				int32_t v;
				memcpy(&v, &p->data.v_32bit, sizeof(p->data.v_32bit));
				value = v;
			}
			break;
		case NVProperty::T_64BIT:
			memcpy(&value, p->data.v_64bit, sizeof(p->data.v_64bit));
			break;
		case NVProperty::T_STR:
		case NVProperty::T_BLOB:
			value = p->len;
			break;
		}
		index++;
		if (p->ut.t.deleted) {
			dprintf("Dump[%.2d] Key: %d Type: %d deleted(%d)", index, p->key, p->ut.t.type, p->ut.t.deleted);

		} else if (p->ut.t.type == NVProperty::T_STR) {
			dprintf("Dump[%.2d] Key: %d Type: %d value: %s", index, p->key, p->ut.t.type, p->data.v_str);
		} else if (p->ut.t.type == NVProperty::T_BLOB) {
			dprintf("Dump[%.2d] Key: %d Type: %d len: %d", index, p->key, p->ut.t.type, p->len);
			dump("Blob",  p->data.v_str, p->len);
		} else {
			dprintf("Dump[%.2d] Key: %d Type: %d value: %ld (0x%x)", index, p->key, p->ut.t.type, (int32_t)value, (int32_t)value);
		}
		int len = p->len;
		if (p->ut.t.type == NVProperty::T_STR || p->ut.t.type == NVProperty::T_BLOB) {
			if (p->u.flags.f_str_zero_term)
				len++;
			if (p->u.flags.f_padeven)
				len++;
		}
		if (p->ut.t.type == NVProperty::T_BIT || p->ut.t.deleted)
			p = (_flashEntry *)((uint8_t *)p + FLASH_ENTRY_HEADER_SHORT);
		else
			p = (_flashEntry *)((uint8_t *)p + FLASH_ENTRY_HEADER + len);
	}
	int freebytes = _endAddress -(uint8_t *)_lastEntry;
	if (_lastEntry)
		dprintf("------ %d bytes free -------", freebytes);
}


int
NVProperty_D21Flash::_GetFlashEntryLen(_flashEntry *p)
{
	int len = 0;
	
	if (p->ut.t.type == NVProperty::T_BIT || p->ut.t.deleted)
		len = FLASH_ENTRY_HEADER_SHORT;
	else
		len = FLASH_ENTRY_HEADER + p->len;
		
	if (p->ut.t.type == NVProperty::T_STR || p->ut.t.type == NVProperty::T_BLOB) {
		if (p->u.flags.f_str_zero_term)
			len++;
		if (p->u.flags.f_padeven)
			len++;
	}
	return len;
}


int
NVProperty_D21Flash::_FlashReorgEntries(int minRequiredSpace)
{

	if (_debug)
		dprintf("_FlashReorgEntries: start");

	int totalLen = 0;
	int freeSpace = 0;
	std::bitset<NVProperty::MAX_PROPERTIES> activeKeys;
	std::bitset<NVProperty::MAX_PROPERTIES> copiedKeys;
	
	activeKeys.reset();
	copiedKeys.reset();
	
	_flashEntry *p = (_flashEntry *)(_startAddress + sizeof(_flash_header));
	while((uint8_t *)p < _endAddress && p->key != 0xff) {
		if  (!activeKeys.test(p->key)) {
			_flashEntry *k = _GetFlashEntry(p->key);
			if (k) {
				activeKeys.set(p->key);
				totalLen += _GetFlashEntryLen(k);
			}
		}
		p = (_flashEntry *)((uint8_t *)p + _GetFlashEntryLen(p));
	}

	if (_startAddress + sizeof(_flash_header) + totalLen + minRequiredSpace >= _endAddress)
			return 0;
	
	freeSpace = _endAddress - (_startAddress + sizeof(_flash_header) + totalLen);
	if (_debug)
		dprintf("freeSpace: %d, totalLen: %d", freeSpace, totalLen);
	
	/*
	 * Copy header
	 * while (content {
	 	- scan until tmp page is full
		- write page
	 * }
	 * Erase remaining pages.
	 *
	 * xxxx
	 */
	p = (_flashEntry *)(_startAddress + sizeof(_flash_header));
	uint8_t *saveddata = new uint8_t[_rowSize+sizeof(struct _flashEntry)];
	if (!saveddata)
		return 0;
	uint8_t *t = saveddata;
	int currentRow = (uint32_t)_startAddress / _rowSize;
	int totalCopied = 0;
	
	t = saveddata;
	memcpy(t, _startAddress, sizeof(_flash_header));
	t += sizeof(_flash_header);
	
	while((uint8_t *)p < _endAddress && p->key != 0xff) {
		if (activeKeys.test(p->key) && !copiedKeys.test(p->key)) { // write entry once
			_flashEntry *k = _GetFlashEntry(p->key, (uint8_t *)p);
			int klen = _GetFlashEntryLen(k);
			memcpy(t, k, klen);
			t += klen;
			totalCopied += klen;
			copiedKeys.set(k->key);
			if (t - saveddata >= _rowSize) { // copy page
				_FlashEraseRow(currentRow);
				_FlashWritePage(currentRow * _rowSize, 0, saveddata, _rowSize);
				t = saveddata;
				currentRow++;
			}
		}
		p = (_flashEntry *)((uint8_t *)p + _GetFlashEntryLen(p));
	}

	if (t > saveddata) { // copy remaining
		_FlashEraseRow(currentRow);
		_FlashWritePage(currentRow * _rowSize / _pageSize, 0, saveddata, t - saveddata);
		currentRow++;
	}

	while((uint32_t)0 + currentRow * _rowSize < (uint32_t)_endAddress) {
		_FlashEraseRow(currentRow++);
	}
	delete[] saveddata;
	_GetFlashEntry(0); // inits the _lastEntry record

	if (_debug) {
		dprintf("_FlashReorgEntries: end");
		// _DumpAllEntires();
		// delay(1000);
	}
	
	return _endAddress - _startAddress -  (sizeof(_flash_header) + totalCopied);
}

int
NVProperty_D21Flash::GetProperty(int key)
{
	
	_flashEntry *p = _GetFlashEntry(key);
	if (!p)
		return NVProperty::NVP_ENOENT;
	
    int value = 0;

    switch(p->ut.t.type) {
		case NVProperty::T_BIT:
			if (p->ut.t.t_bit)
				value = 1;
			else
				value = 0;
			break;
		case NVProperty::T_8BIT:
			value = p->u.v_8bit;
			break;
		case NVProperty::T_16BIT:
			value = p->data.v_16bit;
			break;
		case NVProperty::T_32BIT:
			memcpy(&value, &p->data.v_32bit, sizeof(p->data.v_32bit));
			break;
		case NVProperty::T_64BIT:
			{
				int64_t v;
				memcpy(&v, p->data.v_64bit, sizeof(p->data.v_64bit));
				value = v;
			}
			break;
		case NVProperty::T_STR:
		case NVProperty::T_BLOB:
			value = p->len;
			break;
	}

    return value;
}



int64_t
NVProperty_D21Flash::GetProperty64(int key)
{
	_flashEntry *p = _GetFlashEntry(key);
	if (!p)
		return NVProperty::NVP_ENOENT;

    int64_t value = 0;
	
    switch(p->ut.t.type) {
		case NVProperty::T_BIT:
			if (p->ut.t.t_bit)
				value = 1;
			else
				value = 0;
			break;
		case NVProperty::T_8BIT:
			value = p->u.v_8bit;
			break;
		case NVProperty::T_16BIT:
			value = p->data.v_16bit;
			break;
		case NVProperty::T_32BIT:
			{
				int32_t v;
				memcpy(&v, &p->data.v_32bit, sizeof(p->data.v_32bit));
				value =v;
			}
			break;
		case NVProperty::T_64BIT:
			memcpy(&value, p->data.v_64bit, sizeof(p->data.v_64bit));
			break;
		case NVProperty::T_STR:
		case NVProperty::T_BLOB:
			value = p->len;
			break;
	}
    return value;
}

const char *
NVProperty_D21Flash::GetPropertyStr(int key)
{
	_flashEntry *p = _GetFlashEntry(key);
	if (!p || p->ut.t.type != NVProperty::T_STR)
		return NULL;
    return strdup(p->data.v_str);
}

int
NVProperty_D21Flash::GetPropertyBlob(int key, const void *blob, int *size)
{
	_flashEntry *p = _GetFlashEntry(key);
	if (!p || p->ut.t.type != NVProperty::T_BLOB)
		return NVProperty::NVP_ENOENT;
	
	int cplen = std::min(*size, (int)p->len);
	if (blob)
		memcpy((void *)blob, p->data.v_blob, cplen);
	*size = cplen;
	
    return NVProperty::NVP_OK;
}

int
NVProperty_D21Flash::SetProperty(int key, int64_t value, int type)
{
	UNUSED(type);
	uint8_t valbuf[FLASH_ENTRY_HEADER + sizeof(int64_t)];
	_flashEntry *p = (_flashEntry *) valbuf;
	int storeType;
	
	if (GetProperty64(key) == value) // no need to save it again.
	    return NVProperty::NVP_OK;
	
	
	memset(valbuf, 0, sizeof(valbuf));
	
	if (value == 0 ||  value == 1)
		storeType = NVProperty::T_BIT;
	else if (value > -128 && value < 128)
		storeType = NVProperty::T_8BIT;
	else if (value > -32768 && value < 32768)
		storeType = NVProperty::T_16BIT;
	else if (value > -2147483647 && value < 2147483648)
		storeType = NVProperty::T_32BIT;
	else
		storeType = NVProperty::T_64BIT;
	
	p->key = key;
	p->ut.t.type = storeType;
	p->u.v_8bit = 0; // clear flags

	switch(storeType) {
		case NVProperty::T_BIT:
			p->len = 0;
			p->ut.t.t_bit = value;
			break;
		case NVProperty::T_8BIT:
			p->len = 0;
			p->u.v_8bit = value;
			break;
		case NVProperty::T_16BIT:
			p->len = sizeof(p->data.v_16bit);
			p->data.v_16bit = value;
			break;
		case NVProperty::T_32BIT:
			p->len = sizeof(p->data.v_32bit);
			{
				int32_t v = value;
				memcpy(&p->data.v_32bit, &v, sizeof(p->data.v_32bit));
			}
			break;
		case NVProperty::T_64BIT:
			p->len = sizeof(p->data.v_64bit);
			memcpy(p->data.v_64bit, &value, sizeof(p->data.v_64bit));
			break;
	}
	int len;
	if (storeType == NVProperty::T_BIT)
		len = FLASH_ENTRY_HEADER_SHORT;
	else
		len = FLASH_ENTRY_HEADER + p->len;
	if ((uint8_t *)_lastEntry + len >= _endAddress) {
		if (!_FlashReorgEntries(len))
			return NVProperty::NVP_ERR_NOSPACE;
	}

	_FlashWrite((uint8_t *)_lastEntry, p, len);
	_lastEntry = (_flashEntry *)((uint8_t *)_lastEntry + len);

	// _DumpAllEntires();
    return NVProperty::NVP_OK;
}


int
NVProperty_D21Flash::SetPropertyStr(int key, const char *value, int type)
{
	if (type != NVProperty::T_STR)
		return NVProperty::NVP_INVALD_PARM;
	
	const char *str = GetPropertyStr(key);
	if (str && strncmp(value, str, strlen(value)) == 0)
		return NVProperty::NVP_OK;

	int err = NVProperty::NVP_OK;
	
	_flashEntry *p = new _flashEntry[1];
	if (!p)
		return NVProperty::NVP_ERR_NOSPACE;
	memset(p, 0, sizeof(*p));
	
	p->key = key;
	p->ut.t.type = NVProperty::T_STR;
	int cplen = std::min(strlen(value), sizeof(p->data.v_str)-1);
	p->len = cplen;
	memcpy(p->data.v_str, value, cplen+1);
	
	int len = FLASH_ENTRY_HEADER + p->len;
	p->u.flags.f_str_zero_term = true;
	len++; // str. zero term
	if (len & 1) {
		len++; // padd even
		p->u.flags.f_padeven = true;
	}
	if ((uint8_t *)_lastEntry + len >= _endAddress) {
		if (!_FlashReorgEntries(len)) {
			err = NVProperty::NVP_ERR_NOSPACE;
			goto done;
		}
	}

	_FlashWrite((uint8_t *)_lastEntry, p, len);
	_lastEntry = (_flashEntry *)((uint8_t *)_lastEntry + len);

done:
	delete[] p;
	_DumpAllEntires();
    return err;
}

int
NVProperty_D21Flash::SetPropertyBlob(int key, const void *blob, int size, int type)
{
	if (type != NVProperty::T_BLOB)
		return NVProperty::NVP_INVALD_PARM;
	
	_flashEntry *p = _GetFlashEntry(key);
	if (p && p->ut.t.type == NVProperty::T_BLOB && size == p->len) { // check for duplicate
		if (memcmp(blob, p->data.v_blob, size) == 0)
			return NVProperty::NVP_OK;
	}
	int err = NVProperty::NVP_OK;
	
	 p = new _flashEntry[1];
	if (!p)
		return NVProperty::NVP_ERR_NOSPACE;
	memset(p, 0, sizeof(*p));
	
	p->key = key;
	p->ut.t.type = NVProperty::T_STR;
	int cplen = std::min(size, (int)sizeof(p->data.v_blob));
	p->len = cplen;
	memcpy(p->data.v_blob, blob, cplen);
	
	int len = FLASH_ENTRY_HEADER + p->len;

	if (len & 1) {
		len++; // padd even
		p->u.flags.f_padeven = true;
	}
	if ((uint8_t *)_lastEntry + len >= _endAddress) {
		if (!_FlashReorgEntries(len)) {
			err = NVProperty::NVP_ERR_NOSPACE;
			goto done;
		}
	}

	_FlashWrite((uint8_t *)_lastEntry, p, len);
	_lastEntry = (_flashEntry *)((uint8_t *)_lastEntry + len);

done:
	delete[] p;
	_DumpAllEntires();
    return err;
}

int
NVProperty_D21Flash::EraseProperty(int key)
{
	uint8_t valbuf[FLASH_ENTRY_HEADER_SHORT];
	_flashEntry *p = (_flashEntry *) valbuf;

	_flashEntry *op = _GetFlashEntry(key);
	if (!op)
		return NVProperty::NVP_ENOENT;
	if (op->ut.t.deleted)
		return NVProperty::NVP_OK;
	
	memset(valbuf, 0, sizeof(valbuf));
	p->key = key;
	p->ut.t.type = op->ut.t.type;
	p->ut.t.deleted = true;
	
	if ((uint8_t *)_lastEntry + FLASH_ENTRY_HEADER_SHORT > _endAddress) {
			if (!_FlashReorgEntries(FLASH_ENTRY_HEADER_SHORT))
				return NVProperty::NVP_ERR_NOSPACE;
	}

	_FlashWrite((uint8_t *)_lastEntry, p, FLASH_ENTRY_HEADER_SHORT);
	_lastEntry = (_flashEntry *)((uint8_t *)_lastEntry + FLASH_ENTRY_HEADER_SHORT);

	_DumpAllEntires();
	return NVProperty::NVP_OK;
}

int
NVProperty_D21Flash::ReorgProperties(void)
{
	if (_FlashReorgEntries(FLASH_ENTRY_HEADER))
    	return NVProperty::NVP_OK;
	return NVProperty::NVP_ERR_NOSPACE;
}


int
NVProperty_D21Flash::OpenPropertyStore(bool forWrite)
{
	UNUSED(forWrite);
    return NVProperty::NVP_OK;
}

int
NVProperty_D21Flash::ClosePropertyStore(bool flush)
{
	UNUSED(flush);
    return NVProperty::NVP_OK;
}

#endif // ARDUINO_SAMD_ZERO or ARDUINO_ARCH_SAMD
