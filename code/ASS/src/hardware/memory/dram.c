#include "headers/cpu.h"
#include "headers/memory.h"
#include "headers/common.h"
#include "headers/address.h"
#include <string.h>
#include <assert.h>
#include <stdio.h>

#define SRAM_CACHE_SRTTING 0  //是否读写

uint8_t sram_cache_read(uint64_t paddr);
void sram_cache_write(uint64_t paddr, uint8_t data);
//flighting 
uint64_t cpu_read64bits_dram(uint64_t paddr, core_t *cr){

  uint64_t val = 0x0;
#ifdef DEBUG_ENABLE_SRAM_CACHE
  //load from sram
  for(int i = 0;i<8; i++){
    val += sram_cache_read(paddr+i)<<(i*8);
  }
  return val;
  #elif
  //read from dram
  val += (((uint64_t)pm[paddr+0])<<0*8);
  val += (((uint64_t)pm[paddr+1])<<1*8);
  val += (((uint64_t)pm[paddr+2])<<2*8);
  val += (((uint64_t)pm[paddr+3])<<3*8);
  val += (((uint64_t)pm[paddr+4])<<4*8);
  val += (((uint64_t)pm[paddr+5])<<5*8);
  val += (((uint64_t)pm[paddr+6])<<6*8);
  val += (((uint64_t)pm[paddr+7])<<7*8);

  return val;
#endif
}


void cpu_wirte64bits_dram(uint64_t paddr, uint64_t data, core_t* cr){
#ifdef DEBUG_ENABLE_SRAM_CACHE
    //load from sram
    for(int i = 0;i<8; i++){
      sram_cache_write((paddr+i)<<(i*8),  (data>>(8*i))  & 0xff);
    }
    return;
#elif

  pm[paddr+0] = (data>>0)  & 0xff;
  pm[paddr+1] = (data>>8)  & 0xff;
  pm[paddr+2] = (data>>16) & 0xff;
  pm[paddr+3] = (data>>24) & 0xff;
  pm[paddr+4] = (data>>32) & 0xff;
  pm[paddr+5] = (data>>40) & 0xff;
  pm[paddr+6] = (data>>48) & 0xff;
  pm[paddr+7] = (data>>56) & 0xff;
#endif
}


//read paddr -> buf
void cpu_readinst_dram(uint64_t paddr, char *buf, core_t *cr){
  for(int i = 0;i<MAX_INSTRUCTION_CHAR;++i){
    buf[i] = (char)pm[paddr + i];
  }
}


//wirite buf -> paddr
void cpu_writeinst_dram(uint64_t paddr, const char *str, core_t *cr){
  
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


//interface of I/O BUS : read and write between the SRAM cache and DRAM memory
void bus_read(uint64_t paddr, uint8_t *block){
  uint64_t dram_base = (paddr >> SRAM_CACHE_OFFSET_LENGTH) << SRAM_CACHE_OFFSET_LENGTH;

  for(int i =0;i<(1<<SRAM_CACHE_OFFSET_LENGTH);i++){
   block[i] = pm[dram_base+i];
  }
}


void bus_write(uint64_t paddr, uint8_t *block){
  uint64_t dram_base = (paddr >> SRAM_CACHE_OFFSET_LENGTH) << SRAM_CACHE_OFFSET_LENGTH;
  
  for(int i =0;i<(1<<SRAM_CACHE_OFFSET_LENGTH);i++){
    pm[dram_base+i] = block[i];
  }
}