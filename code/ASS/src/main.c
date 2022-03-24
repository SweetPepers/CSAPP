#include "cpu/register.h"
#include "memory/instruction.h"
#include "memory/dram.h"
#include "cpu/mmu.h"
#include "disk/elf.h"

// const uint64_t mem_off_set = 0x7ffffffee1f0 - 0x408B40;
//redeclaration  error caused by include 
// include guard 

int main(){

  
  init_handler_table();

  //init reg
  reg.rax = 0x12340000;
  reg.rbx = 0x0;
  reg.rcx = 0x8000660;
  reg.rdx = 0xabcd;
  reg.rsi = 0x7ffffffee2f8;
  reg.rdi = 0x1;
  reg.rbp = 0x7ffffffee210;
  reg.rsp = 0x7ffffffee1f0;
  
  reg.rip = (uint64_t)&program[11];
  
  //init memory

  //栈从高往低走
  wirte64bits_dram(va2pa(0x7ffffffee210), 0x08000660);//rbp
  wirte64bits_dram(va2pa(0x7ffffffee208), 0x0);
  wirte64bits_dram(va2pa(0x7ffffffee200), 0xabcd);
  wirte64bits_dram(va2pa(0x7ffffffee1f8), 0x12340000);
  wirte64bits_dram(va2pa(0x7ffffffee1f0), 0x08000660);//rsp

  print_regsiter();
  print_stack();
  //run inst 
  for(int i = 0;i<7;i++){
    instruction_cycle();
    print_regsiter();
    print_stack();
  }

 

  
  //varify

  int match = 1;
  match = match && (reg.rax == 0x1234abcd);
  match = match && (reg.rbx == 0x0);
  match = match && (reg.rcx == 0x8000660);
  match = match && (reg.rdx == 0x12340000);
  match = match && (reg.rsi == 0xabcd);
  match = match && (reg.rdi == 0x12340000);
  match = match && (reg.rbp == 0x7ffffffee210);
  match = match && (reg.rsp == 0x7ffffffee1f0);

  if(match == 1){
    printf("register mathc\n");
  }else{
    printf("reg not match\n");
  }
  
  match = 1;
  
  match = match && (read64bits_dram(va2pa(0x7ffffffee210)) == 0x08000660); //rbp
  match = match && (read64bits_dram(va2pa(0x7ffffffee208)) == 0x1234abcd);
  match = match && (read64bits_dram(va2pa(0x7ffffffee200)) == 0xabcd);
  match = match && (read64bits_dram(va2pa(0x7ffffffee1f8)) == 0x12340000);  //栈从高往低走
  match = match && (read64bits_dram(va2pa(0x7ffffffee1f0)) == 0x08000660);  //rsp


  if(match == 1){
    printf("memory mathc\n");
  }else{
    printf("mem not match\n");
  }

  return 0;
}
//   reg.rax = 0x12345678abcd12ff;
//   printf("eax :%08x\n", reg.eax);
//   printf("ax :%04x\n", reg.ax);
//   printf("ah :%02x\n", reg.ah);
//   printf("al :%02x\n", reg.al);
//   return 0;
// }


// uint64_t decode_od(od_t od){
//   if(od.type == IMM) return od.imm;
//   if(od.type == REG) return *(uint64_t *)od.reg1;
//   uint64_t addr = MM_LEN +0xff;
//   return mm[addr % MM_LEN];
// }