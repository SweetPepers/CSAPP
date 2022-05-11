#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "headers/cpu.h"
#include "headers/interrupt.h"
#include "headers/process.h"
#include "headers/address.h"

static void push_context()
{
  // check the size
  uint32_t ctx_size = sizeof(context_t);
  assert(ctx_size < KERNEL_STACK_SIZE);

  //get the range of kernel stack
  uint64_t rsp = cpu_reg.rsp;
  uint64_t kstack_top_vaddr = (rsp >> 13) << 13 +KERNEL_STACK_SIZE;
  uint64_t kstack_bottom_vaddr = (rsp >> 13) << 13;

  assert(kstack_bottom_vaddr <= rsp && rsp < kstack_top_vaddr);
  assert(rsp-ctx_size >= kstack_bottom_vaddr);

  // get the vaddr of kstack low
  uint64_t rsp = cpu_reg.rsp;
  // assert((rsp + ctx_size) == get_kstack_top_TSS());

  // store user frame to kstack
  rsp -= ctx_size;
  context_t ctx = {
      .general_registers = cpu_reg,
      .flags = cpu_flags};
  memcpy((user_frame_t *)rsp, &ctx, ctx_size);

  // push RSP
  cpu_reg.rsp = rsp;

  //store the current rsp to rsp field of pcb_t
  pcb_t *old_pcb = get_current_pcb();
  old_pcb->rsp = rsp;
}

static void pop_context()
{
  // check the size
  uint32_t ctx_size = sizeof(context_t);
  assert(ctx_size < KERNEL_STACK_SIZE);

  // get the vaddr of kstack low
  uint64_t rsp = cpu_reg.rsp;
  uint64_t kstack_top_vaddr = (rsp >> 13) << 13 +KERNEL_STACK_SIZE;
  uint64_t kstack_bottom_vaddr = (rsp >> 13) << 13;

  assert(kstack_bottom_vaddr <= rsp && rsp < kstack_top_vaddr);
  assert(rsp + ctx_size < kstack_top_vaddr);

  // restore user frame from kstack
  user_frame_t ctx;
  memcpy(&ctx, (context_t *)rsp, ctx_size);
  rsp += ctx_size;

  // restore cpu registers from user frame
  memcpy(&cpu_reg, &ctx.general_registers, sizeof(cpu_reg));
  memcpy(&cpu_flags, &ctx.flags, sizeof(cpu_flags));

  // pop rsp
  cpu_reg.rsp = rsp;


}


void os_schedule(){

  uint64_t rsp = cpu_reg.rsp;
  uint64_t kstack_bottom_vaddr = (rsp >> 13) << 13 + KERNEL_STACK_SIZE;
  
  kstack_t *kstack_old = (kstack_t *)kstack_bottom_vaddr;
  pcb_t *pcb_old = kstack_old->threadinfo.pcb;


  // pcb_new should be selected by scheduling algorithm
  // TODO
  pcb_t *pcb_new = pcb_old->next;

  //context switch 
  //store the context of the old process(kernel mode)

  // this push will push the context of the old process
  push_context();

  cpu_reg.rsp = pcb_new->rsp;

  // restore the context of the new process
  pop_context();
  
  // TODO : update TR -> TSS
  // update CR3 -> page table in MMU
  cpu_controls.cr3 = pcb_new->mm.pgd_paddr;
  return;
}

pcb_t *get_current_pcb()
{
    kstack_t *ks = (kstack_t *)get_kstack_RSP();
    pcb_t *current_pcb = ks->threadinfo.pcb;
    return current_pcb;
}