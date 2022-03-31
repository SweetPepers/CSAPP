#include "headers/cpu.h"
#include "headers/memory.h"
#include "headers/common.h"
#include <string.h>
#include <assert.h>
#include <stdio.h>

#define SRAM_CACHE_SRTTING 0  //是否读写

//flighting 
uint64_t read64bits_dram(uint64_t paddr, core_t *cr){
  if(DEBUG_ENABLE_SRAM_CACHE == 1){
    return 0x0;
  }
  uint64_t val = 0x0;
  val += (((uint64_t)pm[paddr+0])<<0*8);
  val += (((uint64_t)pm[paddr+1])<<1*8);
  val += (((uint64_t)pm[paddr+2])<<2*8);
  val += (((uint64_t)pm[paddr+3])<<3*8);
  val += (((uint64_t)pm[paddr+4])<<4*8);
  val += (((uint64_t)pm[paddr+5])<<5*8);
  val += (((uint64_t)pm[paddr+6])<<6*8);
  val += (((uint64_t)pm[paddr+7])<<7*8);

  return val;
}


void wirte64bits_dram(uint64_t paddr, uint64_t data, core_t* cr){
  if (SRAM_CACHE_SRTTING == 1){
    return ;
  }

  pm[paddr+0] = (data>>0)  & 0xff;
  pm[paddr+1] = (data>>8)  & 0xff;
  pm[paddr+2] = (data>>16) & 0xff;
  pm[paddr+3] = (data>>24) & 0xff;
  pm[paddr+4] = (data>>32) & 0xff;
  pm[paddr+5] = (data>>40) & 0xff;
  pm[paddr+6] = (data>>48) & 0xff;
  pm[paddr+7] = (data>>56) & 0xff;
}


//read paddr -> buf
void readinst_dram(uint64_t paddr, char *buf, core_t *cr){
  for(int i = 0;i<MAX_INSTRUCTION_CHAR;++i){
    buf[i] = (char)pm[paddr + i];
  }
}


//wirite buf -> paddr
void writeinst_dram(uint64_t paddr, const char *str, core_t *cr){
  
  int len = strlen(str);
  assert(len < MAX_INSTRUCTION_CHAR);

  for(int i = 0; i< MAX_INSTRUCTION_CHAR;i++){
    if(i<len){
      pm[paddr+i] = (uint8_t)str[i];
    }else
    {
      pm[paddr+i] = 0;
    }
  }
}
