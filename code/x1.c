#include<stdlib.h>
#include<stdio.h>


unsigned LowBit(unsigned x){
    return x&(~x+1);
}


//十六进制数字是否全是字母 x3&(x1+x2)
unsigned Letter(unsigned x){
  // unsigned a ;
  //x3 x2 x1 x0 - hex
  //0  0  1  0  - hex constant
  //0  0  x1 0  取x1
  unsigned x0 = x & 0x11111111;  
  unsigned x1 = x & 0x22222222;
  unsigned x2 = x & 0x44444444;
  unsigned x3 = x & 0x88888888;
  //0001 &1000 get 0000
  return (x3>>3)&(x1>>1|x2>>2);
}

void main(){
    unsigned n = 7;
    printf("S[%u] = \n", n);
    
    while(n>0){
      
      printf("T[%u]\n+", n);
      n = n - LowBit(n);
    }
    printf("T[%u]\n", n);

    printf("%x is a letter 0x%x\n",0xabcacaca, Letter(0xabcaca11));
    printf("is a letter 0x%x\n",Letter(0x12312132));
}


