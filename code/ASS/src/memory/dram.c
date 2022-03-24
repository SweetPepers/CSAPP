#include "dram.h"
#include "cpu/register.h"
#include "cpu/mmu.h"

#define SRAM_CACHE_SRTTING 0  //是否读写

//flighting 
uint64_t read64bits_dram(uint64_t paddr){
  if(SRAM_CACHE_SRTTING == 1){
    return 0x0;
  }
  uint64_t val = 0x0;
  val += (((uint64_t)mm[paddr+0])<<0*8);
  val += (((uint64_t)mm[paddr+1])<<1*8);
  val += (((uint64_t)mm[paddr+2])<<2*8);
  val += (((uint64_t)mm[paddr+3])<<3*8);
  val += (((uint64_t)mm[paddr+4])<<4*8);
  val += (((uint64_t)mm[paddr+5])<<5*8);
  val += (((uint64_t)mm[paddr+6])<<6*8);
  val += (((uint64_t)mm[paddr+7])<<7*8);

  return val;
}


void wirte64bits_dram(uint64_t paddr, uint64_t data){
  if (SRAM_CACHE_SRTTING == 1){
    return ;
  }

  mm[paddr+0] = (data>>0)  & 0xff;
  mm[paddr+1] = (data>>8)  & 0xff;
  mm[paddr+2] = (data>>16) & 0xff;
  mm[paddr+3] = (data>>24) & 0xff;
  mm[paddr+4] = (data>>32) & 0xff;
  mm[paddr+5] = (data>>40) & 0xff;
  mm[paddr+6] = (data>>48) & 0xff;
  mm[paddr+7] = (data>>56) & 0xff;
}

void print_regsiter(){
  printf("rax = %16lx\trbx = %16lx\trcx = %16lx\trdx = %16lx\n", (unsigned long)reg.rax, (unsigned long)reg.rbx, (unsigned long)reg.rcx, (unsigned long)reg.rdx);
  printf("rsi = %16lx\trdi = %16lx\trbp = %16lx\trsp = %16lx\n",(unsigned long)reg.rsi, (unsigned long)reg.rdi, (unsigned long)reg.rbp, (unsigned long)reg.rsp);
  printf("rip = %16lx\n", (unsigned long)reg.rip);
}

void print_stack(){
  int n = 10;
  uint64_t* high = (uint64_t *)&mm[va2pa(reg.rsp)];
  high = &high[n];

  for(int i = 0; i< 2*n;i++){
    uint64_t *ptr = (uint64_t *)(high-i);
    printf("%p: %16lx", ptr, (unsigned long)*ptr);

    if (i==n){
      printf(" <== rsp");
    }
    printf("\n");
  }
}