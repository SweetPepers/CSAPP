#include <stdio.h>
#include <string.h>
#include "headers/cpu.h"
#include "headers/common.h"
#include "headers/memory.h"

#define MAX_NUM_INSTRUCTION_CYCLE 100

static void TestAddFunctionCallAndComputation();
static void TestSumFunctionCallAndComputation();

static void TestString2Uint();
void TestPaseingOperand();
void TestParseInstruction();

void writeinst_dram(uint64_t paddr, const char *str);
void readinst_dram(uint64_t paddr, char *buf);


void print_register();
void print_stk();



int main(){
  // TestParseInstruction();
  // TestPaseingOperand();
  // TestString2Uint();
  TestAddFunctionCallAndComputation();
  TestSumFunctionCallAndComputation();
  return 0;
}

static void TestString2Uint(){

  const char* nums[10] = {
    "0",
    "-0",
    "1231",
    "-13123",
    "0x123",
    "-0x1231",
    "2147483647",
    "-2147483648",
    "0x8000000000000000",
    "0xffffffffffffffff"
  };

  for(int i = 0;i<10;i++){
    printf("%s => %16lx\n", nums[i], string2uint(nums[i]));
  }

}

static void TestAddFunctionCallAndComputation(){

  cpu_reg.rax = 0x0;
  cpu_reg.rbx = 0x0;
  cpu_reg.rcx = 0x0;
  cpu_reg.rdx = 0x0;
  cpu_reg.rsi = 0x0;
  cpu_reg.rdi = 0x0;
  cpu_reg.rbp = 0x0;  
  cpu_reg.rsp = 0x7ffffffee220;  //why don't wrok ?????????

  
  cpu_flags.__flags_value = 0;

  wirte64bits_dram(va2pa(0x7ffffffee110), 0x0000000000000000);
  wirte64bits_dram(va2pa(0x7ffffffee110), 0x0000000000000000);
  wirte64bits_dram(va2pa(0x7ffffffee110), 0x0000000000000000);
  wirte64bits_dram(va2pa(0x7ffffffee110), 0x0000000000000000);
  wirte64bits_dram(va2pa(0x7ffffffee110), 0x0000000000000000);

  char assembly[15][MAX_INSTRUCTION_CHAR] ={
    "push    %rbp",          //0    0x00400000
    "mov %rsp, %rbp",        //1
    "mov %rdi, -0x18(%rbp)", //2
    "mov %rsi, -0x20(%rbp)", //3
    "mov -0x18(%rbp), %rdx", //4
    "mov -0x20(%rbp), %rax", //5
    "add %rdx, %rax",        //6
    "mov %rax, -0x8(%rbp)",  //7
    "mov -0x8(%rbp), %rax",  //8
    "pop %rbp",              //9
    "retq",                  //10
    "mov %rdx, %rsi",        //11
    "mov %rax, %rdi",        //12
    "callq 0x00400000",      //13
    "mov %rax, -0x8(%rbp)",  //14
  };

  


  for(int i = 0;i<15;i++){
    // printf("%d : %s\n", i, assembly[i]);
    writeinst_dram(va2pa(i*0x40+0x00400000), assembly[i]);
  }

  // ->rip = (uint64_t)&assembly[11];
  
  cpu_pc.rip = MAX_INSTRUCTION_CHAR * sizeof(char) *11 +0x00400000;

  printf("begin\n");
  int time = 0;

  while(time < 15){
    instruction_cycle();
    print_register();
    print_stk();
    time++;
  }


  //gdb state ret from func
  int match = 1;
  match = match && (cpu_reg.rax == 0x1234abcd);
  match = match && (cpu_reg.rbx == 0x0);
  match = match && (cpu_reg.rcx == 0x8000660);
  match = match && (cpu_reg.rdx == 0x12340000);
  match = match && (cpu_reg.rsi == 0xabcd);
  match = match && (cpu_reg.rdi == 0x12340000);
  match = match && (cpu_reg.rbp == 0x7ffffffee210);
  match = match && (cpu_reg.rsp == 0x7ffffffee1f0);

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
}
// const uint64_t mem_off_set = 0x7ffffffee1f0 - 0x408B40;
//redeclaration  error caused by include 
// include guard 

static void TestSumFunctionCallAndComputation(){

  cpu_reg.rax = 0x3;
  cpu_reg.rbx = 0x0;
  cpu_reg.rcx = 0x8000650;
  cpu_reg.rdx = 0x7ffffffee328;
  cpu_reg.rsi = 0x7ffffffee318;
  cpu_reg.rdi = 0x1;
  cpu_reg.rbp = 0x7ffffffee230;
  cpu_reg.rsp = 0x7ffffffee220;

  cpu_flags.__flags_value = 0;

  wirte64bits_dram(va2pa(0x7ffffffee230), 0x0000000000000650);
  wirte64bits_dram(va2pa(0x7ffffffee228), 0x0000000000000000);
  wirte64bits_dram(va2pa(0x7ffffffee220), 0x00007ffffffee310);


  char assembly[19][MAX_INSTRUCTION_CHAR] = {
    "push    %rbp",           //0
    "mov   %rsp,%rbp",        //1
    "sub   $0x10,%rsp",       //2
    "mov   %rdi,-0x8(%rbp)",  //3
    "cmpq  $0x0,-0x8(%rbp)",  //4
    "jne   0x400200",         //5   jump to 8
    "mov   $0x0,%eax",        //6
    "jmp   0x400380",         //7   jump to 14
    "mov   -0x8(%rbp),%rax",  //8
    "sub   $0x1,%rax",        //9
    "mov   %rax,%rdi",        //10
    "callq 0x00400000",       //11
    "mov   -0x8(%rbp),%rdx",  //12
    "add   %rdx,%rax",        //13
    "leaveq",                 //14
    "retq",                   //15
    "mov   $0x3,%edi",        //16
    "callq 0x00400000",       //17
    "mov   %rax,-0x8(%rbp)",  //18
  };

  //copy va to physical memroy
  for(int i = 0;i<19;i++){
    writeinst_dram(va2pa(i*0x40+0x00400000), assembly[i]);
  }


  //start rip  is  assembly[16]
  cpu_pc.rip = MAX_INSTRUCTION_CHAR * sizeof(char) *16 +0x00400000;

  printf("begin\n");
  int time = 0;
  
  //cycle times is not sure
  while(cpu_pc.rip <= 18*0x40+0x00400000 && time < MAX_NUM_INSTRUCTION_CYCLE){
    instruction_cycle();
    print_register();
    print_stk();
    time++;
  }



  //gdb state ret from func
  int match = 1;
  match = match && (cpu_reg.rax == 0x6);
  match = match && (cpu_reg.rbx == 0x0);
  match = match && (cpu_reg.rcx == 0x8000650);
  match = match && (cpu_reg.rdx == 0x3);
  match = match && (cpu_reg.rsi == 0x7ffffffee318);
  match = match && (cpu_reg.rdi == 0x0);
  match = match && (cpu_reg.rbp == 0x7ffffffee230);
  match = match && (cpu_reg.rsp == 0x7ffffffee220);

  if(match == 1){
    printf("register match\n");
  }else{
    printf("reg not match\n");
  }
  
  match = 1;
  
  match = match && (read64bits_dram(va2pa(0x7ffffffee230)) == 0x0); //rbp
  match = match && (read64bits_dram(va2pa(0x7ffffffee228)) == 0x0);
  match = match && (read64bits_dram(va2pa(0x7ffffffee220)) == 0x00007ffffffee310);  //rsp


  if(match == 1){
    printf("memory match\n");
  }else{
    printf("mem not match\n");
  }


}


