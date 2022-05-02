#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "headers/cpu.h"
#include "headers/interrupt.h"
#include "headers/memory.h"

typedef void (*syscall_handler_t)();

// the entry of IDT/IVT
typedef struct SYSCALL_ENTRY_STRUCT
{
  syscall_handler_t handler; 
} syscall_entry_t;


syscall_entry_t syscall_table[64];

//handlers of syscall  
static void write_handler();
static void getpid_handler();  //0x01
static void fork_handler();
static void execve_handler();
static void exit_handler();
static void wait_handler();
static void kill_handler();


void syscall_init()
{
  syscall_table[01].handler = write_handler;
  syscall_table[39].handler = getpid_handler;
  syscall_table[57].handler = fork_handler;
  syscall_table[59].handler = execve_handler;
  syscall_table[60].handler = exit_handler;
  syscall_table[61].handler = wait_handler;
  syscall_table[62].handler = kill_handler;
} 


static void write_handler()
{
  // assembly begin 
  uint64_t file_no = cpu_reg.rdi;
  uint64_t buf_vaddr = cpu_reg.rsi;
  uint64_t buf_len = cpu_reg.rdx;

  // assembly end

  for(int i =0 ; i< buf_len;i++){
    printf("%c", pm[va2pa(buf_vaddr+i)]);
  }

}


static void getpid_handler()
{

}

static void fork_handler()
{

}
static void execve_handler()
{

}

static void exit_handler()
{
  // assembly begin 
  uint64_t exit_status = cpu_reg.rdi;
  // assembly end

  //The following resourse are allocated in KERNEL STACK
  printf("Good bye~~~\n");
}

static void wait_handler()
{

}
static void kill_handler()
{

}


void do_syscall(int syscall_no)
{
  assert(syscall_no >= 0 && syscall_no <= 64);
  syscall_table[syscall_no].handler();

}