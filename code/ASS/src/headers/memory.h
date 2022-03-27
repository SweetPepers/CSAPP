#ifndef MEMORY_GUARD
#define MEMORY_GUARD
#include <stdint.h>
#include <stdlib.h>
#include "cpu.h"

//16 physical memory pages
#define PHYSICAL_MEMROY_SPACE 65536
#define MAX_INDEX_PHYSICAL_PAGE 15

uint8_t pm[PHYSICAL_MEMROY_SPACE];

uint64_t read64bits_dram(uint64_t paddr, core_t *cr);
void wirte64bits_dram(uint64_t paddr, uint64_t data, core_t *cr);

void readinst_dram(uint64_t paddr, char *buf, core_t *cr);
void writeinst_dram(uint64_t paddr, const char *str, core_t *cr);

#endif