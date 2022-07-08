#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "headers/cpu.h"
#include "headers/memory.h"
#include "headers/common.h"
#include "headers/address.h"
#include "headers/interrupt.h"
#include "headers/process.h"


int KERNEL_free(void *ptr){
  int stack_lower_bound = 100;
  if((uint64_t)stack_lower_bound > (uint64_t)ptr){
    // ptr is pointing to the area in simulator's heap
    free(ptr);
    return 1;
  } 
  // ptr is pointing to the area in simulator's stack
  return 0;
}