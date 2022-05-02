#ifndef INTERRUPT_GUARD
#define INTERRUPT_GUARD

#include <stdint.h>
#include <stdlib.h>


//only use stack0 of TSS
//stored in main memory
typedef struct TSS_S0{
  uint64_t ESP0;
  uint64_t SS0;
} tss_s0_t;



void idt_init();

void call_interrupt_stack_switching(uint64_t int_vec);

#endif