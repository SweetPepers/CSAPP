#ifndef PROCESS_GUARD
#define PROCESS_GUARD

#include <stdint.h>
#include <stdlib.h>
#include "headers/memory.h"

#define KERNEL_STACK_SIZE (8192)


typedef struct KERNEL_THREAD_INFO_STRUCT{
  uint64_t pcb_vaddr;
}thread_info_t;


typedef union KERNEL_STACK_STRUCT
{    
    uint8_t stack[KERNEL_STACK_SIZE];
    struct
    {
        struct PROCESS_CONTROL_BLOCK_STRUCT *pcb;
    } threadinfo;
} kstack_t;

typedef struct PROCESS_CONTROL_BLOCK_STRUCT{
  uint64_t pid;
  struct {
    // page global directory
    // this value is what's in CR3 register
    union 
    {
      uint64_t pgd_paddr;
      pte123_t *pgd;
    };
    // TODO: vm area

  }mm;
  kstack_t * kstack;
  uint64_t rsp; 
  context_t context;
  struct PROCESS_CONTROL_BLOCK_STRUCT * next;
  struct PROCESS_CONTROL_BLOCK_STRUCT * prev;
}pcb_t;

typedef struct STRUCT_PROCESS_CONTEXT{
  cpu_reg_t regs;
  cpu_flags_t flags;
}context_t;


void syscall_init();
void do_syscall(int syscall_no);
void os_schedule();
pcb_t *get_current_pcb();

#endif