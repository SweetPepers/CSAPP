#ifndef CPU_GUARD
#define CPU_GUARD

#include <stdint.h>
#include <stdlib.h>

typedef struct REGISTER_STRUCT{
  union {
    uint64_t rax;
    uint32_t eax;
    uint16_t ax;
    struct
    {
      uint8_t al;
      uint8_t ah;
    };
  };
  union {
    uint64_t rbx;
    uint32_t ebx;
    uint16_t bx;
    struct
    {
      uint8_t bl;
      uint8_t bh;
    };
  };
  union {
    uint64_t rcx;
    uint32_t ecx;
    uint16_t cx;
    struct
    {
      uint8_t cl;
      uint8_t ch;
    };
  };
  union {
    uint64_t rdx;
    uint32_t edx;
    uint16_t dx;
    struct
    {
      uint8_t dl;
      uint8_t dh;
    };
  };
  union {
    uint64_t rsi;
    uint32_t esi;
    uint16_t si;
    struct
    {
      uint8_t sil;
      uint8_t sih;
    };
  };
  union {
    uint64_t rdi;
    uint32_t edi;
    uint16_t di;
    struct
    {
      uint8_t dil;
      uint8_t dih;
    };
  };
  union {
    uint64_t rbp;
    uint32_t ebp;
    uint16_t bp;
    struct
    {
      uint8_t bpl;
      uint8_t bph;
    };
  };
  union {
    uint64_t rsp;
    uint32_t esp;
    uint16_t sp;
    struct
    {
      uint8_t spl;
      uint8_t sph;
    };
  };
  union {
   uint64_t r8;
   uint32_t r8d;
   uint16_t r8w;
   uint8_t  r8b; 
  };
  union {
   uint64_t r9;
   uint32_t r9d;
   uint16_t r9w;
   uint8_t  r9b; 
  };
  union {
   uint64_t r10;
   uint32_t r10d;
   uint16_t r10w;
   uint8_t  r10b; 
  };
  union {
   uint64_t r11;
   uint32_t r11d;
   uint16_t r11w;
   uint8_t  r11b; 
  };
  union {
   uint64_t r12;
   uint32_t r12d;
   uint16_t r12w;
   uint8_t  r12b; 
  };
  union {
   uint64_t r13;
   uint32_t r13d;
   uint16_t r13w;
   uint8_t  r13b; 
  };
  union {
   uint64_t r14;
   uint32_t r14d;
   uint16_t r14w;
   uint8_t  r14b; 
  };
  union {
   uint64_t r15;
   uint32_t r15d;
   uint16_t r15w;
   uint8_t  r15b; 
  };
}reg_t;




/*===============================*/
/*       cpu core                */
/*===============================*/

typedef struct  CORE_STRUCT{
  //program counter or instruction pointer
  union
  {
    uint64_t rip;
    uint64_t eip;
  };
  
  //carry flag :detect overflow for unsigned operations
  uint32_t CF;
  //zero flag:
  uint32_t ZF;
  //sign flag
  uint32_t SF;
  //overflow flag:detect overflow for signed operations
  uint32_t OF;

  reg_t reg;
  // uint64_t pdbr; //page directory base register
}core_t;

#define NUM_CORES 1
core_t cores[NUM_CORES];

uint64_t ACTIVE_CORE;
#define MAX_INSTRUCTION_CHAR 64
#define NUM_INSTRTYPE 14 

void instruction_cycle(core_t *cr);

//translate virtual address to pa in MMU
//each core has a MMU
uint64_t va2pa(uint64_t paddr, core_t *cr);






#endif