#include "cpu/register.h"
#include "memory/instruction.h"


int main(){
  return 0;
}
//   reg.rax = 0x12345678abcd12ff;
//   printf("eax :%08x\n", reg.eax);
//   printf("ax :%04x\n", reg.ax);
//   printf("ah :%02x\n", reg.ah);
//   printf("al :%02x\n", reg.al);
//   return 0;
// }


// uint64_t decode_od(od_t od){
//   if(od.type == IMM) return od.imm;
//   if(od.type == REG) return *(uint64_t *)od.reg1;
//   uint64_t addr = MM_LEN +0xff;
//   return mm[addr % MM_LEN];
// }