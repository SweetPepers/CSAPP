#include <stdio.h>
#include <string.h>
#include "headers/cpu.h"
#include "headers/common.h"
#include "headers/memory.h"

#define MAX_NUM_INSTRUCTION_CYCLE 100

static void TestAddFunctionCallAndComputation();

void print_register(core_t *cr);
void print_stack(core_t *cr);

int main(){
  TestAddFunctionCallAndComputation();
  return 0;
}


static void TestAddFunctionCallAndComputation()[
  ACTIVE_CORE = 0x0;

  core_t *ac = (core_t *)&cores[ACTIVE_CORE];

  ac->reg.rax = 
  ac->reg.rbx = 
  ac->reg.rcx = 
  ac->reg.rdx = 
  ac->reg.rsi = 
  ac->reg.rdi = 
  ac->reg.rbp = 
  ac->reg.rsp = 

  ac->CF = 0;
  ac->ZF = 0;
  ac->SF = 0;
  ac->OF = 0;

  wirte64bits_dram(va2pa(0x7ffffffee110, ac), 0x0000000000000000,ac);
  wirte64bits_dram(va2pa(0x7ffffffee110, ac), 0x0000000000000000,ac);
  wirte64bits_dram(va2pa(0x7ffffffee110, ac), 0x0000000000000000,ac);
  wirte64bits_dram(va2pa(0x7ffffffee110, ac), 0x0000000000000000,ac);
  wirte64bits_dram(va2pa(0x7ffffffee110, ac), 0x0000000000000000,ac);

  char assembly[15][MAX_INSTRUCTION_CHAR] ={
    "push  %rbp" //0
    "mov %rsp, %rbp" //1
    "mov %rdi, -0x18(%rbp)" //2
    "mov %rsi, -0x20(%rbp)" //3
    "mov -0x18(%rbp), %rdx" //4
    "mov -0x20(%rbp), %rax" //5
    "add %rdx, %rax" //6
    "mov %rax, -0x8(%rbp)" //7
    "mov -0x8(%rbp), %rax" //8
    "pop %rbp" //9
    "retq" //10
    "mov %rdx, %rsi" //11
    "mov %rax, %rdi" //12
    "callq 0" //13
    "mov %rax, -0x8(%rbp)" //14
  };

  ac->rip = (uint64_t)&assembly[11];
  sprintf(assembly[13], "callq $%p", &assembly[0]);

  printf("begin\n");
  int time = 0;
  while(time < 15){
    instruction_cycle(ac);
    print_regsiter(ac);
    print_stack(ac);
    time++;
  }


  //gdb state ret from func
  int match = 1;
  match = match && (ac->reg.rax == 0x1234abcd);
  match = match && (ac->reg.rbx == 0x0);
  match = match && (ac->reg.rcx == 0x8000660);
  match = match && (ac->reg.rdx == 0x12340000);
  match = match && (ac->reg.rsi == 0xabcd);
  match = match && (ac->reg.rdi == 0x12340000);
  match = match && (ac->reg.rbp == 0x7ffffffee210);
  match = match && (ac->reg.rsp == 0x7ffffffee1f0);

  if(match == 1){
    printf("register match\n");
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
    printf("memory match\n");
  }else{
    printf("mem not match\n");
  }

// const uint64_t mem_off_set = 0x7ffffffee1f0 - 0x408B40;
//redeclaration  error caused by include 
// include guard 