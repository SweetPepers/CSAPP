#define _GUN_SOURCE
/*
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sched.h>

#define PAGE_BYTES (4096)

int64_t result_page0[PAGE_BYTES/sizeof(int64_t)];

typedef struct 
{
  int *cache_write_ptr;
  int cpu_id;
  int length;
}param_t;

void *work_thread(void *param){
  param_t *p = param;
  int64_t *ptr = p->cache_write_ptr;
  int cpu_id = p->cpu_id;
  int length = p->length;

  cpu_set_t mask;
  CPU_ZERO(&mask);
  CPU_SET(cpu_id, &mask);
  pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask);

  printf("* thread[%lu] running on cpu[%d] writes to %p\n", pthread_self(), sched_getcpu(),ptr);
  for(int i =0 ;i<length;i++){
    *ptr += 1;
  }
  return NULL;
}

void true_shareing_run(){
  pthread_t t1, t2;

  long t0 = clock();
  param_t p1 = {
    .cache_write_ptr = &result_page0[0],
    .cpu_id = 0,
    .length = 1000000
  };
  param_t p2 = {
    .cache_write_ptr = &result_page0[0],
    .cpu_id = 1,
    .length = 1000000
  };

  long t0 = clock();

  pthread_create(&t1, NULL, work_thread, (void *)&p1);
  pthread_create(&t2, NULL, work_thread, (void *)&p2);

  pthread_join(t1, NULL);
  pthread_join(t2, NULL);

  printf("    [True sharing] \n\t elapsed tick tock : %ld\n", clock()-t0);
}

int main(){
  true_shareing_run();
}

*/