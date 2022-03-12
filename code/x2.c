#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<stdint.h>
//有点失败
uint32_t uint2float(uint32_t u){  //s1e8f23
  if(u == 0) return 0;
  // printf("u = %x\n", u);

  //
  uint32_t s = u&0x80000000;
  uint32_t f;
  uint32_t e;

  u = u&0x7fffffff;
  int i;
  int n = 0;
  for( i = 0; i<31 ;++i){  //找最左边的1 之前得先把最高位去掉
    if (u >> i == 0x1){
      n = i;
      break;
    }
  }

  if (n <23){ //no near
    
    // s = (u>>31) & 0x1; //mask 过滤
    f = u & (0xffffffff >> (33-n));
    //E = e+bias = n-1+127 = 1+126 最左边带个1
    e = n+126;
    printf("n : %d\n f: %x\n e: %d\n u:%x\n", n,f, e, u);
    return s|e<<23|f;
  }else  //这里还得改
  {
    //should near   xgrs
    uint32_t x = (u >>(n-23))&0x1; 
    uint32_t g = (u>>(n-22))&0x1;
    uint32_t r = (u>>(n-21))&0x1;
    uint32_t s_ = 0;

    int j = 0;
    for(;j<n-23-1;++j){
      s_ = s_|((u>>j)&0x1);
    }

    e = 23+127;
    f = 0;

    return s|(e<<23)|f;
  }
  
}

int main(){
  uint32_t u = 0x80081231;
  printf("%x\n",uint2float(u));
  return 0;
}