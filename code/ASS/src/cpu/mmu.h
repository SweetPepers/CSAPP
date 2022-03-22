//memory management unit

//include guard 
#ifndef mmu_guard
#define mmu_guard

#include<stdint.h>

uint64_t va2pa(uint64_t vaddr);

#endif

//singleton  单例模式