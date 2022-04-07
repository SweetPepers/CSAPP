#include "headers/address.h"
#include "headers/memory.h"
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <stdio.h>

#ifdef CACHE_SIMULATION_VERIFICATION

int cache_hit_count = 0;
int cache_miss_count = 0;
int cache_evict_count = 0;
int dirty_bytes_in_cache_count = 0;
int dirty_bytes_evict_count = 0;

char trace_buf[20];
char *trace_ptr = (char*)&trace_buf;

#else
#define NUM_CACHE_LINE_PRE_SET (8)

#endif



typedef enum{
  CACHE_LINE_INVALID,
  CACHE_LINE_CLEAN,
  CACHE_LINE_DIRTY
}sram_cacheline_state_t;

//wirte-through and no-write-allocate
typedef enum{
  CACHE_LINE_INVALID,
  CACHE_LINE_VALID
}sram_cacheline_state_t2;

typedef struct{
  sram_cacheline_state_t state;
  int time;  //for LRU
  uint64_t tag;
  uint8_t block[1<<SRAM_CACHE_OFFSET_LENGTH];  //64bytes <=> 8 uint64_t
}sram_cacheline_t; 

typedef struct{
  sram_cacheline_t lines[NUM_CACHE_LINE_PRE_SET]; 
}sram_cacheset_t;


typedef struct{
  sram_cacheset_t sets[1<<SRAM_CACHE_INDEX_LENGTH];
}sram_cache_t;


static sram_cache_t cache;  //init to .bss  -> INVALID

uint8_t sram_cache_read(uint64_t paddr_value){
  address_t paddr ={
    .paddr_value = paddr_value,
  };
  sram_cacheset_t *set =  &cache.sets[paddr.ci];
  sram_cacheline_t *victim = NULL;
  sram_cacheline_t *invalid = NULL;
  //update LRU time
  int max_time = -1;
  
  for(int i =0 ;i<NUM_CACHE_LINE_PRE_SET;i++){

    set->lines[i].time++;
    if(max_time < set->lines[i].time){

      //select this line as victim
      victim = &set->lines[i];
      max_time = set->lines[i].time;
    }
    if(set->lines[i].state == CACHE_LINE_INVALID){
      //exist one invalid line as candidate for cache miss
      invalid  = &set->lines[i];
    }
  }

  //tey cache hit
  for(int i=0;i<NUM_CACHE_LINE_PRE_SET;i++){
    sram_cacheline_t line = set->lines[i];
    if(line.state != CACHE_LINE_INVALID && line.tag == paddr.ct){
      //cache hit  find byte
      line.time = 0;
      return line.block[paddr.co];  
    }

  }
  //miss : load from memroy 

  //try to find one free cacheline 
  if(invalid != NULL){
    //load data from DRAM to this invalid cache line
    bus_read(paddr.paddr_value, &(invalid->block));
    invalid->state = CACHE_LINE_CLEAN;

    //update LRU time and tag
    invalid->time = 0;
    invalid->tag = paddr.ct;
    return invalid->block[paddr.co];
  }
  
  //no free cache line, use LRU policy
  //select a victim by replacement policy if set is full
  assert(victim != NULL);
  if(victim->state == CACHE_LINE_DIRTY){
    //write back
    bus_wirte(paddr.paddr_value, &(victim->block));
  }
  //if clean , discard this victim directly
  //update state
  victim->state == CACHE_LINE_INVALID;

  bus_read(paddr.paddr_value, &(victim->block));
  victim->state = CACHE_LINE_CLEAN;

  //update LRU time and tag
  victim->time = 0;
  victim->tag = paddr.ct;
  return victim->block[paddr.co];
}

void sram_cache_write(uint64_t paddr_value, uint8_t data){
  address_t paddr ={
    .paddr_value = paddr_value
  };
  sram_cacheset_t *set =  &cache.sets[paddr.ci];

  sram_cacheline_t *victim = NULL;
  sram_cacheline_t *invalid = NULL; //for write-allocate
  //update LRU time
  int max_time = -1;
  
  for(int i =0 ;i<NUM_CACHE_LINE_PRE_SET;i++){

    set->lines[i].time++;
    if(max_time < set->lines[i].time){

      //select this line as victim
      victim = &set->lines[i];
      max_time = set->lines[i].time;
    }
    if(set->lines[i].state == CACHE_LINE_INVALID){
      //exist one invalid line as candidate for cache miss
      invalid  = &set->lines[i];
    }
  }

  //try cache hit
  for(int i=0;i<NUM_CACHE_LINE_PRE_SET;i++){
    sram_cacheline_t line = set->lines[i];
    if(line.state != CACHE_LINE_INVALID && line.tag == paddr.ct){
#ifdef CACHE_SIMULATION_VERIFICATION
      sprintf(trace_buf, "hit");
      cache_hit_count++;
#endif

      //cache hit  find byte
      line.time = 0;
      line.block[paddr.co] = data;  

      //update state
      line.state = CACHE_LINE_DIRTY;
      return;
    }

  }
  //miss : load from memroy 
  //write-allocate

  //try to find one free cacheline 
  if(invalid != NULL){
    //load data from DRAM to this invalid cache line
    bus_read(paddr.paddr_value, &(invalid->block));
    invalid->state = CACHE_LINE_DIRTY;

    //update LRU time and tag
    invalid->time = 0;
    invalid->tag = paddr.ct;
    invalid->block[paddr.co] = data;
    return;
  }
  
  //no free cache line, use LRU policy
  //select a victim by replacement policy if set is full
  assert(victim != NULL);
  if(victim->state == CACHE_LINE_DIRTY){
    //write back
    bus_wirte(paddr.paddr_value, &(victim->block));
    
  }
  //if clean , discard this victim directly
  //update state
  victim->state == CACHE_LINE_INVALID;

  bus_read(paddr.paddr_value, &(victim->block));
  victim->state = CACHE_LINE_DIRTY;

  //update LRU time and tag
  victim->time = 0;
  victim->tag = paddr.ct;
  victim->block[paddr.co] = data;
}

