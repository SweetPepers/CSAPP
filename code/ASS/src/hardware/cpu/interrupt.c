#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "headers/cpu.h"
#include "headers/interrupt.h"
#include "headers/process.h"

typedef void (*interrupt_handler_t)();

// the entry of IDT/IVT
typedef struct IDT_ENTRY_STRUCT
{
  interrupt_handler_t handler; 
} idt_entry_t;

// interrupt descriptor/vector table
idt_entry_t idt[256];

//handler of IDT
void syscall_handler();          // trap gate -- exception
void pagefault_handler();        // trap gate -- exception
// void protectionfault_handler();  // trap gate -- software interrupt / trap
void timer_handler();            //interrupt gate -- local APIC



void idt_init()
{
  idt[0x0e].handler = pagefault_handler;
  idt[0x80].handler = syscall_handler;
  // idt[13].handler = protectionfault_handler;
  idt[0x81].handler = timer_handler;
} 

// user --> kernel 
void call_interrupt_stack_switching(uint64_t int_vec)
{
  assert(0<= int_vec && int_vec <= 255);
  // 1. temporarily saves(internelly) the current contents of 
  // the SS, ESP, EFLAGS, CS and EIP rigusters
  uint64_t rsp_user = cpu_reg.rsp;
  uint64_t rip_user = cpu_pc.rip;
  //TODO SS and CS



  // 2. load the segment selector and stack pointer for the new stack 
  // (that is, the stack for the privilege level being called)
  // form the TSS into the SS and ESP registers and switch to the new stack 

  //TRICK: not use GDT. Instead, TR directly points to TSS(stack0)
  
  tss_s0_t *tss_s0 = (tss_s0_t *)cpu_task_register;
  uint64_t kstack = tss_s0->ESP0; // should be 8kB aligned 
  // stack switching 
  // this kstack is allocated in the heap of emulator 
  cpu_reg.rsp = kstack;


  // 3. Pushes the temporarily saved SS, ESP, EFLAGS, CS and EIP values
  // for the interrupted procedure's stack onto the new stack
  // TODO SS & CS
  // TODO kernel address space & page table mapping
  // TODO use malloc for kernel space
  cpu_reg.rsp = cpu_reg.rsp - 8;
  *(uint64_t *)cpu_reg.rsp = rsp_user;
  cpu_reg.rsp = cpu_reg.rsp - 8;
  *(uint64_t *)cpu_reg.rsp = rip_user;


  /* if implemented the kernel address sapce pn physical memroy 
  cpu_reg.rsp = cpu_reg.rsp - 8;
  cpu_write64bits_dram(
        va2pa(cpu_reg.rsp),
        rsp_user);
  cpu_reg.rsp = cpu_reg.rsp - 8;
  cpu_write64bits_dram(
        va2pa(cpu_reg.rsp),
        rip_user);
  */


  // 4. push and error code on the new stack (if appropriate)

  // 5. loads the segment selector for the new code segment and the new instruction pointer
  // (from the interrupt gate or trap gate)
  // into the CS and EIP registers, repectively
  interrupt_handler_t handler = idt[int_vec].handler;

  // 6. If the call is through an interrupt gate, 
  // clears the IF flag in the EFLAGS register
  // NO NEED because our interrupt is in isa.c:instruction_cycle
  // but for kernel routine the code is running in host's CPU
  // so emulator's interrupt can't interrupt the host's cpu


  // 7. begin execution of the handler procedure at the new privilege level 
  handler();
}





void syscall_handler()
{
  uint64_t syscall_num = cpu_reg.rax;
  do_syscall(syscall_num);
  return;
}


void pagefault_handler()
{
  return;
}



void timer_handler()
{
  return;
}