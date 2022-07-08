#ifndef INTERRUPT_GUARD
#define INTERRUPT_GUARD

#include <stdint.h>
#include <stdlib.h>
#include <setjmp.h>

#define KERNEL_malloc malloc


// USE this function VERY VERY carefully
int KERNEL_free(void *ptr);

// executed by hardware(cpu)
typedef struct {
  // error code
  uint64_t rip;
  // CS: segment register
  // eflags
  uint64_t rsp;
  // SS : stack segment;
} trap_frame_t;

// managed by software(os)(in handler)
typedef struct {
  cpu_reg_t general_registers;
  cpu_flags_t flags;
} user_frame_t;

void idt_init();

void interrupt_stack_switching(uint64_t int_vec);
void interrupt_return_stack_switching();

uint64_t get_kstack_top_TSS();
uint64_t get_kstack_RSP();


/*  You will learn this from CSAPP: Exceptional Control Flow: Nonlocal Jumps.
 *  Nonlocal jump is a form of user-level exceptional control flow provided by C.
 *  We will use nonlocal jumps to implement interrut return.
 *  
 *  Think this: we implement 3 kinds of interrupts:
 *      -   System call
 *      -   Timer interrupt
 *      -   Page fault
 *  But their return addresses are different. And OS scheduling & context switch
 *  makes it even more complicated.
 * 
 *      System call
 *              process 1, int $0x80
 *                      Interrupt happens inside the `int` instruction.
 *                      So to successfully return to next process 1 instruction,
 *                      the return instruction to be pushed is the next.
 *                      THUS, WE NEED TO INCREMENT RIP BEFORE INVOKING INTERRUPT
 *              process 1, ret addr
 * 
 *                  instruction_cycle
 *                      int_handler
 *                          increase_pc // this must be called before interrupt
 *                          interrupt_stack_switching
 *                                      // so the pushed RIP will be the next instruction
 *                              syscall_handler
 *                                  os_schedule
 *                              interrupt_return_stack_switching 
 *                              // jump to the return instruction of process 2
 * 
 *      Timer interrupt
 *              process 1, add
 *                      Interrupt happens between the execution of these 2 instructions
 *                      So the execution of instruction i should be completed.
 *                      AND RIP PUSHED TO TRAP FRAME IS ALREADY RETURN ADDRESS
 *              process 1, ret addr
 * 
 *                  instruction_cycle
 *                      add_handler
 *                      interrupt_stack_switching
 *                                  // so the pushed RIP will be the next instruction
 *                          timer_handler
 *                              os_schedule
 *                          interrupt_return_stack_switching 
 *                          // jump to the return instruction of process 2
 * 
 *      Page fault
 *              process 1, mov $1234, [0x7fffffffffff]
 *                      Interrupt happens inside the `mov` instruction, the va2pa
 *                      address translation phase. Check this call stack
 *                  
 *                  instruction_cycle
 *                      mov_handler
 *                          va2pa
 *                              page_walk
 *                                  interrupt_stack_switching
 *                                          // Since RIP has not been updated, RIP pushed to 
 *                                          // trap frame is just this `mov` instruction.
 *                                          // Thus when OS sched back to process 1,
 *                                          // this instruction would be rescheduled.
 *                                          // And physical memory is not touched since 
 *                                          // address translation fails.
 *                                      pagefault_handler
 *                                          os_schedule
 *                                      interrupt_return_stack_switching 
 *                                      // jump to the return instruction of process 2
 *                          cpu_write64bits_dram    // will not be executed due to non-local jump
 *                          increase_pc             // will not be executed due to non-local jump
 */
jmp_buf USER_INSTRUCTION_ON_IRET;


#endif
