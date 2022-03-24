
#ifndef dram_guard
#define dram_guard
#include <stdint.h>


#define MM_LEN 1000

uint8_t mm[MM_LEN];  //1000bytes memory 
// physical memroy 0->999

//直接写64位会溢出 要做处理

uint64_t read64bits_dram(uint64_t paddr);
void wirte64bits_dram(uint64_t paddr, uint64_t data);
void print_regsiter();
void print_stack();

//virsual address 0->0xffffffffffffffff
// mm[0x777788888777qqqq % MM_LEN];

#endif