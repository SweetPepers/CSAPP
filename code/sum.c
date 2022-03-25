#include<stdint.h>

uint64_t sum(uint64_t n){
  if (n == 0){
    return 0;
  }
  return n+sum(n-1);
}

int main(){
  uint64_t a = sum(3);
  return 0;
}