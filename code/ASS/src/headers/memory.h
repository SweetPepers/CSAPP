#ifndef MEMORY_GUARD
#define MEMORY_GUARD
#include <stdint.h>
#include <stdlib.h>
#include "cpu.h"

//16 physical memory pages
//used only for user process
#define PHYSICAL_MEMROY_SPACE (65536)  //bytes
#define MAX_INDEX_PHYSICAL_PAGE (15)
#define MAX_NUM_PHYSICAL_PAGE (16)
#define PAGE_TABLE_ENTRY_NUMBER (512)

uint8_t pm[PHYSICAL_MEMROY_SPACE];



//page table entry struct
//8bytes
typedef union{
  uint64_t pte_value;
  //csapp 9.7.1 picture 9-23
  struct{
    uint64_t present     : 1;
    uint64_t readonly    : 1;
    uint64_t usermode    : 1;
    uint64_t writethough : 1;
    uint64_t cachedisabled : 1; 
    uint64_t reference   :1;
    uint64_t unused6      :1;
    uint64_t smallpage    :1;
    uint64_t global       :1;
    uint64_t unused9_11   :3;
    /*
    uint64_t paddr        :40;
    uint64_t unused52_62  :11;
    */
    //for malloc, a virtual address on heap is 48bits
    //for real world,a physical page number is  40bits
    uint64_t paddr        :51;  //virtual address(48bits used)
    uint64_t xdisabled    :1;
  };
  struct{
    uint64_t _present      :1;
    uint64_t swap_id       :63;//disk address
  };
}pte123_t; // PGD, PUD, PMD


typedef union{
  uint64_t pte_value;
  //csapp 9.7.1 picture 9-23
  struct{
    uint64_t present      : 1;
    uint64_t readonly     : 1;
    uint64_t usermode     : 1;
    uint64_t writethough  : 1;
    uint64_t cachedisabled : 1; 
    uint64_t reference    : 1;
    uint64_t dirty        : 1;//1 for dirty, 0 for clean
    uint64_t zero7        : 1;
    uint64_t global       : 1;
    uint64_t unused9_11   : 3;
    /*
    uint64_t paddr        :40;
    uint64_t unused52_62  :11;
    */
    //for malloc, a virtual address on heap is 48bits
    //for real world,a physical page number is  40bits
    uint64_t ppn         :40;  //virtual address(48bits used)
    uint64_t unused52_62 : 11;
    uint64_t xdisabled    :1;
  };
  struct{
    uint64_t _present      :1;
    uint64_t daddr         :63;//disk address
  };
}pte4_t;  //PT

//physical page description
typedef struct{
  //TODO : if multiple processes are using this page? E.g. shared library
  pte4_t *pte4; //reversed mapping : ppn -> page_table_entry
  
  int allocated;
  int dirty;
  int time;  //LRU cache

  uint64_t daddr;  //binding the reverse mapping with mapping to disk 
}pd_t;

//for each swapable physical page, create one mapping
pd_t page_map[MAX_NUM_PHYSICAL_PAGE];   





uint64_t cpu_read64bits_dram(uint64_t paddr, core_t *cr);
void cpu_write64bits_dram(uint64_t paddr, uint64_t data, core_t *cr);

void cpu_readinst_dram(uint64_t paddr, char *buf, core_t *cr);
void cpu_writeinst_dram(uint64_t paddr, const char *str, core_t *cr);


void bus_read(uint64_t paddr, uint8_t *block);
void bus_write(uint64_t paddr, uint8_t *block);
#endif