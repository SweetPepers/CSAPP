
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <stdio.h>


#define DE

typedef enum{
  MODIFIED, // exclusive dirty  global 1
  EXCLUSIVE,// exclusive clean  global 1
  SHARED,   // shared clean, global >= 2
  INVALID,  //invalid
}state_t;

typedef struct{
  state_t state;
  int value;
}line_t;


#define NUM_PROCESSOR (100)

line_t cache[NUM_PROCESSOR];

int mem_value = 15213;

int check_state(){
  int m_count = 0;
  int e_count = 0;
  int s_count = 0;
  int i_count = 0;

  for(int i = 0;i<NUM_PROCESSOR;i++){
    if(cache[i].state == MODIFIED){
      m_count += 1;
    }
    if(cache[i].state == EXCLUSIVE){
      e_count += 1;
    }
    if(cache[i].state == SHARED){
      s_count += 1;
    }
    if(cache[i].state == INVALID){
      i_count += 1;
    }
  }

/*
      M   E  S  I
  M   X   X  X  O
  E   X   X  X  O
  S   X   X  O  O
  I   O   O  O  O
*/

  #ifdef DEBUG
  printf("M %d\tE %d\tS %d\tI %d\t\n", m_count, e_count, s_count,i_count);
  #endif

  if((m_count  == 1 && i_count == (NUM_PROCESSOR-1)) 
    ||(e_count == 1 && i_count == (NUM_PROCESSOR-1))
    ||(s_count >= 2 && i_count == (NUM_PROCESSOR-s_count))
    ||(i_count == NUM_PROCESSOR)){
      return 1;
    }

  return 0;
} 


//i - the index of processor
//read_value - the address of read value
//int return - if this event is related with target cache line
int read_cacheline(int i, int *read_value){
  if(cache[i].state == MODIFIED){
    //read hit
    #ifdef DEBUG
    printf("[%d] read hit,dirty value %d\n", i, cache[i].value);
    #endif
    *read_value = cache[i].value;  
    return 1;
  }  
  else if(cache[i].state == EXCLUSIVE){
    //read hit
    #ifdef DEBUG
    printf("[%d] read hit, exclusive value %d\n", i, cache[i].value);
    #endif
    *read_value = cache[i].value;
    return 1;
  }
  else if(cache[i].state == SHARED){
    //read hit
    #ifdef DEBUG
    printf("[%d] read hit, shared exclusive value %d\n", i, cache[i].value);
    #endif
    *read_value = cache[i].value;
    return 1;
  }
  else{
    //read miss
    // bus boardcast read miss
    for(int j =0;j<NUM_PROCESSOR;j++){
      if(i == j) continue;
      if(cache[j].state == MODIFIED){
        //write back
        mem_value = cache[j].value;
        cache[j].state = SHARED;

        //update read miss cache
        cache[i].state = SHARED;
        //no bus
        cache[i].value = cache[j].value;
        
        #ifdef DEBUG
        printf("[%d] read miss, [%d] supplies dirty value %d; writeback\n", i, j,cache[i].value);
        #endif
        *read_value = cache[i].value;  
        return 1;
      }
      else if(cache[j].state == EXCLUSIVE){
        // no memory transaction
        cache[i].state = SHARED;
        cache[i].value = cache[j].value;

        cache[j].state = SHARED;
        #ifdef DEBUG
        printf("[%d] read miss, [%d] supplies clean value %d; share_count == 2\n", i, j,cache[i].value);
        #endif
        *read_value = cache[i].value;  
        return 1;
      }
      else if(cache[j].state == SHARED){
        //>=3
        cache[i].state = SHARED;
        cache[i].value = cache[j].value;
        #ifdef DEBUG
        printf("[%d] read miss, [%d] supplies clean value %d; s_count >= 3\n", i, j,cache[i].value);
        #endif
        *read_value = cache[i].value;  
        return 1;
      }
    }
    //all others are invalid
    cache[i].state = EXCLUSIVE;
    cache[i].value = mem_value;
    *read_value = cache[i].value;  
    #ifdef DEBUG
    printf("[%d] read miss, memroy supplies clean value %d; e_count == 1\n", i,cache[i].value);
    #endif
    return 1;
  }

  return 0;
}


//write_value - the value to be written to the physical address
//int return - if this event is related with target physical address
int write_cacheline(int i, int write_value){
  if(cache[i].state == MODIFIED){
    //write hit
    cache[i].value = write_value;
    #ifdef DEBUG
    printf("[%d] write hit,update to value %d\n", i, cache[i].value);
    #endif
    return 1;
  }
  else if(cache[i].state == EXCLUSIVE){
    //write hit
    cache[i].value = write_value;
    cache[i].state = MODIFIED;
    #ifdef DEBUG
    printf("[%d] write hit,update to value %d\n", i, cache[i].value);
    #endif
    return 1;
  }
  else if(cache[i].state == SHARED){
    //broadcast write invalid
    for(int j=0;j<NUM_PROCESSOR;j++){
      if(j == i) continue;
      cache[j].state = INVALID;
      cache[j].value = 0;
    }
    cache[i].state = MODIFIED;
    cache[i].value = write_value;
    #ifdef DEBUG
    printf("[%d] write hit,broadcast invalid;update to value %d\n", i, cache[i].value);
    #endif
    return 1;
  }
  else if (cache[i].state == INVALID)
  {
    for(int j=0;j<NUM_PROCESSOR;j++){
      if(i == j) continue;
      if(cache[j].state == MODIFIED){
        //write back
        mem_value = cache[j].value;
        cache[j].state = INVALID;
        cache[j].value = 0;

        //write allocate
        cache[i].value = mem_value;
        cache[i].state = MODIFIED;
        cache[i].value = write_value;
        #ifdef DEBUG
        printf("[%d] write miss,broadcast invalid to M;update to value %d\n", i, cache[i].value);
        #endif
        return 1;
      }
      else if(cache[j].state == EXCLUSIVE){
        cache[j].state = INVALID;
        cache[j].value = 0;

        cache[i].state = MODIFIED;
        cache[i].value = write_value;
        #ifdef DEBUG
        printf("[%d] write miss,broadcast invalid to E;update to value %d\n", i, cache[i].value);
        #endif
        return 1;

      }
      else if(cache[j].state == SHARED){
        for(int k = 0;k<NUM_PROCESSOR;k++){
          if( i== k)continue;
          cache[k].value = 0;
          cache[k].state = INVALID;
        }

        cache[i].state = MODIFIED;
        cache[i].value = write_value;
        #ifdef DEBUG
        printf("[%d] write miss,broadcast invalid to S;update to value %d\n", i, cache[i].value);
        #endif
        return 1;
      }

    }

    //all other are invalid
    //write allocate
    cache[i].value = mem_value;
    cache[i].state = MODIFIED;
    cache[i].value = write_value;
    #ifdef DEBUG
    printf("[%d] evict;write back value %d\n", i, cache[i].value);
    #endif
    return 1;

  }

  return 0; 
  
}

//i - the index of processor
//int return - if this event is related with target physical address
int evict_cacheline(int i){
  if(cache[i].state == MODIFIED){
    mem_value = cache[i].value;
    cache[i].state = INVALID;
    cache[i].value = 0;


    #ifdef DEBUG
    printf("[%d] evict\n", i);
    #endif
    return 1;
  }
  else if(cache[i].state == EXCLUSIVE || cache[i].state == SHARED){
    if (cache[i].state == SHARED){
      //check SHARED
      //may left only one shared to be exclutive
      int s_count = 0;
      int last_s= - 1;
      for(int j=0; j< NUM_PROCESSOR;j++){
        if(cache[j].state == SHARED && j != i){
          last_s = j;
          s_count++;
        }
      }
      if(s_count == 1){
        cache[last_s].state = EXCLUSIVE;
      }
    }
    cache[i].state = INVALID;
    cache[i].value = 0;
    return 1;
  }

  //evict when cache line si invalid
  return 0;
}



void print_cacheline(){
  for(int i = 0; i<NUM_PROCESSOR;i++){
    char c;
    switch (cache[i].state)
    {
    case MODIFIED:
      c = 'M';
      break;
    case EXCLUSIVE:
      c = 'E';
      break;
    case SHARED:
      c = 'S';
      break;
    case INVALID:
      c = 'I';
      break;
    default:
      c = '?';
      break;
    }
    printf("\t[%d]\t  state %c  value %d\n", i,c, cache[i].value);
  }
  printf("          \tmem_value %d\n", mem_value);
}


int main(){
  srand(123456);
  int read_value;

  int do_print = 0;
  for(int i = 0;i<NUM_PROCESSOR;i++){
    cache[i].state = INVALID;
    cache[i].value = 0;
  }
  #ifdef DEBUG
  print_cacheline();
  #endif

  for(int i = 0;i<10000;i++){
    int core_index = rand() %NUM_PROCESSOR;
    int op = rand() %3;
    if(op == 0){
      // printf("read\n");
      do_print = read_cacheline(core_index, &read_value);
    }
    else if(op == 1){
      // printf("write\n");

      do_print = write_cacheline(core_index, rand());
    }
    else if(op == 2){
      // printf("evict\n");

      do_print = evict_cacheline(core_index);
    }
    if(do_print){
      #ifdef DEBUG
      print_cacheline();
      #endif 
    }

    if(check_state() == 0){
      printf("failed\n");
      exit(0);
    }

  }
  printf("pass\n");
 
  return 0;


}