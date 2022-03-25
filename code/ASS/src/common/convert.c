#include "headers/common.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>



uint64_t string2uint(const char *str){
  return string2uint_range(str, 0, -1);
}

uint64_t string2uint_range(const char *str, int start, int end){
  return 0;
}