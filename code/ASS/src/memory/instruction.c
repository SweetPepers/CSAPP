#include "instruction.h"
#include "cpu/mmu.h"
#include "cpu/register.h"
#include "dram.h"
#include <stdio.h>

static uint64_t decode_od(od_t od){
  if(od.type == IMM) return *((uint64_t *)&od.imm); 
  //返回值为uint64_t need to transfer into uint64_t
  //difference with transfer diectly ?
  if(od.type == REG) return (uint64_t)od.reg1;

  //else  mm
  uint64_t vaddr = 0;
  if (od.type == MM_IMM){
    vaddr = od.imm;
  }
  else if(od.type == MM_REG){
    vaddr = *(od.reg1);
  }
  else if(od.type == MM_IMM_REG){
    vaddr = od.imm + *(od.reg1);
  }
  else if(od.type == MM_REG1_REG2){
    vaddr = *(od.reg1)+*(od.reg2);
  }
  else if(od.type == MM_IMM_REG1_REG2){
    vaddr = od.imm+*(od.reg1)+*(od.reg2);
  }
  else if(od.type == MM_REG2_S){
    vaddr = (*(od.reg2))*od.scal;
  }
  else if(od.type == MM_IMM_REG2_S){
    vaddr = od.imm + (*(od.reg2))*od.scal;
  }
  else if(od.type == MM_REG1_REG2_S){
    vaddr = *(od.reg1) + (*(od.reg2))*od.scal;
  }
  else if(od.type == MM_IMM_REG1_REG2_S){
    vaddr = od.imm+*(od.reg1) + (*(od.reg2))*od.scal;
  }

  return vaddr;  //取消的解码 
}

void instruction_cycle(){
  inst_t *instr = (inst_t *)reg.rip;
  
  //imm:imm
  //reg:value
  //mm:paddr  
  uint64_t src = decode_od(instr->src);
  uint64_t dst = decode_od(instr->dst);

  handler_t handler = handler_table[instr->op];
  handler(src, dst);

  printf("     %s\n", instr->code);
}


void init_handler_table(){
  handler_table[mov_reg_reg] = &mov_reg_reg_handler;
  handler_table[add_reg_reg] = &add_reg_reg_handler;
  handler_table[call] = &call_handler;
  handler_table[push_reg] = &push_reg_handler;
  handler_table[pop_reg] = &pop_reg_handler;
  handler_table[mov_reg_mem] = &mov_reg_mem_handler;
  handler_table[mov_mem_reg] = &mov_mem_reg_handler;
  handler_table[ret] = &ret_handler;

}
void add_reg_reg_handler(uint64_t src, uint64_t dst){
  *(uint64_t*)dst += *(uint64_t*)src;

  reg.rip += sizeof(inst_t);
}


void mov_reg_reg_handler(uint64_t src, uint64_t dst){
  *(uint64_t*)dst = *(uint64_t*)src;
  reg.rip += sizeof(inst_t);
}

void call_handler(uint64_t src, uint64_t dst){
  // src : imm addr of called function 
  // dst : 
  reg.rsp -= 8;  //向下扩一个栈

  //write return addr to rsp memory 
  wirte64bits_dram(
    va2pa(reg.rsp),
    reg.rip + sizeof(inst_t)
  );

  reg.rip = src;
} 

void push_reg_handler(uint64_t src, uint64_t dst){
  // printf("push\n");
  reg.rsp -= 8;
  wirte64bits_dram(va2pa(reg.rsp), *(uint64_t*)src);

  reg.rip += sizeof(inst_t);
}

void pop_reg_handler(uint64_t src, uint64_t dst){
  //src : rbp
  
  *(uint64_t *)src = read64bits_dram(va2pa(reg.rsp));
  reg.rsp += 8;

  reg.rip += sizeof(inst_t);
}



//带内存读写错误
void mov_reg_mem_handler(uint64_t src, uint64_t dst){
  //src : reg
  //dsr : mem va
  // *(uint64_t *)dst  = *(uint64_t *)src;

  wirte64bits_dram(
    va2pa(dst),
    *(uint64_t *)src
  );

  reg.rip += sizeof(inst_t);
}

//problem
//    mov %rsi,-0x20(%rbp)  汇编写错了  气死我了
void mov_mem_reg_handler(uint64_t src, uint64_t dst){
  //src : mem va
  //dst : reg
  // *(uint64_t *)dst  = *(uint64_t *)src;

  *(uint64_t *)dst = read64bits_dram(va2pa(src));
  reg.rip += sizeof(inst_t);
}

void ret_handler(uint64_t src, uint64_t dst){

  uint64_t ret_addr = read64bits_dram(va2pa(reg.rsp));
  reg.rsp += 8;

  reg.rip = ret_addr;
}