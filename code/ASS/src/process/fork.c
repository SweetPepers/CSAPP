#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "headers/cpu.h"
#include "headers/memory.h"
#include "headers/common.h"
#include "headers/address.h"
#include "headers/interrupt.h"
#include "headers/process.h"
static uint64_t fork_without_cow();
static uint64_t fork_with_cow();

uint64_t fork(){

  fork_without_cow();
}



static uint64_t fork_without_cow(){
  printf("forking.. \n");

  // direct COPY 

  // find all pages of current process:
  pcb_t *parent_pcb =  get_current_pcb();
  /*  
   In a realistic OS, need to allocate kernel pages as well
   And then copy the data on parent kernel page to child's 
  */
  pcb_t *child_pcb = KERNEL_malloc(sizeof(pcb_t));

  memcpy(child_pcb, parent_pcb, sizeof(pcb_t));
  // update child pid
  child_pcb->pid ++;
  // TODO copy the entire page table of parent 
  parent_pcb->mm.pgd;
  // TODO : find physical frames to copy the pages of parent 


  // All copy work done 

  // call once, return twice
  // parent_pcb->context.regs.rax = child_pcb->pid;
  // child_pcb->context.regs.rax = 0;

  update_userframe_returnval(parent_pcb, child_pcb->pid);
  update_userframe_returnval(child_pcb, 0);


  // add child process PCB to linked list for scheduling
  // it's better the child process is right after parent 



  return 0;
}
//fork_with_cow
static uint64_t fork_naive_copy(){
  
}