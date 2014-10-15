#include "ets_sys.h"



#ifndef ONEWIRE_SEARCH
#define ONEWIRE_SEARCH 1
#endif

void onewire_init(uint8 pin);

uint8 onewire_reset(void);

void onewire_writebit(uint8 v);

uint8 onewire_readbit(void);

void onewire_write(uint8 v, bool power);

void onewire_writebytes(uint16 *str, uint8 length, bool power);

uint8 onewire_read(void);

void onewire_readbytes(uint16 *str, uint8 length);

void onewire_select(uint16 rom[8]);

void onewire_skip(void);

void onewire_depower(void);

void onewire_resetsearch(void);

void onewire_targetsearch(uint8 family_code);

uint8 onewire_search(uint16 *newAddr);

uint8 onewire_crc8(uint8 *str, uint8 length);

