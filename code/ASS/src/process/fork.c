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


void update_userframe_returnval(pcb_t *p, int retval){
  p->context.regs.rax = retval;

  // updata userframe's rax
  uint64_t uf_vaddr =  (uint64_t)p->kstack + KERNEL_STACK_SIZE - sizeof(trap_frame_t) - sizeof(user_frame_t) ;
  user_frame_t *uf = (user_frame_t *)uf_vaddr;
  uf->general_registers.rax = retval; 
}



static uint64_t get_newpid(){
  // find the max pid of current processes
  pcb_t *p = get_current_pcb();
  pcb_t *x = p;
  uint64_t max_pid = p->pid;
  while(x->next != p){
    max_pid = max_pid < x->pid? x->pid:max_pid;
    x = x->next;
  }
  
  return max_pid+1;
}


pte123_t * copy_pagetable_dfs(pte123_t *src, int level){
  // allocate one page for dst 
  pte123_t *dst = KERNEL_malloc(sizeof(pte123_t) *PAGE_TABLE_ENTRY_NUMBER);
  

  //copy the current pagetable to dst
  memcpy(dst, src, sizeof(pte123_t)*PAGE_TABLE_ENTRY_NUMBER);


  if(level == 4){
    return dst;
  }
  // check src
  for(int i = 0;i<PAGE_TABLE_ENTRY_NUMBER;i++){
    if(src[i].present == 1){
      pte123_t *src_next = (pte123_t *)src[i].paddr;
      dst[i].paddr= (uint64_t)copy_pagetable_dfs(src_next, level+1);
    }
  }

}


/*
|63--52|51-------------------------12|11------0|
|------|page table physical base addr|---------|
*/
#define PAGE_TABLE_ENTRY_PADDR_MUSK (~(((0xffffffffffffffff)>>12)<<12))



static int compare_pagetables_dfs(pte123_t *p, pte123_t *q, int level){
  for(int i = 0; i<PAGE_TABLE_ENTRY_NUMBER;i++){
    if((p[i].pte_value & PAGE_TABLE_ENTRY_PADDR_MUSK) != 
      (q[i].pte_value & PAGE_TABLE_ENTRY_PADDR_MUSK)){
        assert(0);
      }

    if(level < 4 &&p[i].present == 1 &&      compare_pagetables_dfs(
      (pte123_t*)(uint64_t)p[i].paddr, 
      (pte123_t*)(uint64_t)q[i].paddr, level+1) == 0){
        assert(0);
      }
  }

  return 1;
}

static int compare_pagetable(pte123_t *p, pte123_t *q){
  return compare_pagetables_dfs(p, q, 1);
}



uint64_t syscall_fork(){

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
  // all system calls are actually compiled to binaries.
  // use C to write the fork
  // Actually, we should write instructions: mov_handler, add_handler, etc.

  // DIRECT COPY 
  // find all pages of current process:
  pcb_t *parent_pcb = get_current_pcb();

  // In a realistic OS, you need to allocate kernel pages as well 
  // And then copy the data on parent kernel page to child's 
  pcb_t *child_pcb = KERNEL_malloc(sizeof(pcb_t));
  memcpy(child_pcb, parent_pcb, sizeof(pcb_t));
  
  // COW opatimize : create a kernel stack for child process

  // NOTE: KERNEL stack must be ALIGNED
  kstack_t *child_kstack = aligned_alloc(KERNEL_STACK_SIZE, KERNEL_STACK_SIZE);
  memcpy(child_kstack, (kstack_t*)parent_pcb->kstack, sizeof(kstack_t));
  child_pcb->kstack = child_kstack;
  child_kstack->threadinfo.pcb = child_pcb;


  // prepare child process context(especially RSP) for context switching 
  child_pcb->context.regs.rsp = (uint64_t)child_kstack+KERNEL_STACK_SIZE - sizeof(user_frame_t) - sizeof(trap_frame_t);




  //update child pcb
  child_pcb->pid = get_newpid();

  // add child process PCB to linked list for scheduling 
  // its better the child process is right after parent
  pcb_t *parent_next = child_pcb;
  child_pcb->prev = parent_pcb;
  if(child_pcb->next == parent_pcb){
    // parent is the only PCB in the linked list 
    parent_pcb->prev = child_pcb;
  }

  // call once, return twice
  update_userframe_returnval(parent_pcb, child_pcb->pid);
  update_userframe_returnval(child_pcb, 0);
  

  

  // copy the entire page table of parent
  child_pcb->mm.pgd =  copy_pagetable_dfs(parent_pcb->mm.pgd, 1);
  assert(compare_pagetables(child_pcb->mm.pgd, parent_pcb->mm.pgd) == 1);
  //TODO: find physical frames to copy the pages of parent


  return 0;

 }

