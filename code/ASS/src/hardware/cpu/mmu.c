#include "headers/cpu.h"
#include "headers/common.h"
#include "headers/memory.h"

uint64_t va2pa(uint64_t vaddr, core_t *cr){
  return vaddr & (0xffffffffffffffff>>(63- MAX_INDEX_PHYSICAL_PAGE));
  // <=>vaddr & 0xffff;
}