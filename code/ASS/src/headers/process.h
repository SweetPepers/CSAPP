#ifndef PROCESS_GUARD
#define PROCESS_GUARD

#include <stdint.h>
#include <stdlib.h>

#define KERNEL_STACK_SIZE (8192)

void syscall_init();
void do_syscall(int syscall_no);




typedef union KERNEL_STACK_STRUCT{

  uint8_t stack[KERNEL_STACK_SIZE];
  // TODO : add thread info
}kstack_t;

#endif