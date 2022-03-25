#include "headers/cpu.h"
#include "headers/common.h"
#include "headers/memory.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef enum INST_OPERATOR{
  INST_MOV,     //0
  INST_PUSH,    //1
  INST_POP,     //2
  INTS_LEAVE,   //3
  INST_CALL,    //4
  INST_RET,     //5
  INST_ADD,     //6
  INST_SUB,     //7
  INST_CMP,     //8
  INST_JNE,     //9
  INST_JMP,     //10
}op_t;


typedef enum OPERAND_TYPE{
  EMPTY,                  //0
  IMM,                    //1
  REG, 
  MEM_IMM,                 //3
  MEM_REG,
  MEM_IMM_REG,             //5
  MEM_REG1_REG2,
  MEM_IMM_REG1_REG2,       //7
  MEM_REG2_SCAL,
  MEM_IMM_REG2_SCAL,
  MEM_REG1_REG2_SCAL,      //10
  MEM_IMM_REG1_REG2_SCAL   //11
} od_type_t;


//operand
typedef struct OPERAND_STRUCT{
  od_type_t type; //imm reg mem
   
  int64_t imm;
  int64_t scal;  //scale number to register 2
  uint64_t *reg1;
  uint64_t *reg2;
}od_t;



//csapp 7.5 symbols and symbol tables
typedef struct INSTRUCT_STRUCT{
  op_t op;  //mov push
  od_t src;  
  od_t dst;
}inst_t;


//map the string assembly code to inst_t instance
static void parse_instruction(const char* str, inst_t *inst, core_t *cr);
static void parse_operand(const char* str, od_t *od, core_t *cr);
static uint64_t decode_operand(od_t *od);

static uint64_t decode_operand(od_t *od){
  if(od->type == IMM) return *((uint64_t *)&od->imm); 
  //返回值为uint64_t need to transfer into uint64_t
  //difference with transfer diectly ?
  if(od->type == REG) return (uint64_t)od->reg1;
  else if (od->type == EMPTY)
  {
    return 0;
  }else{
     //else  mm
    uint64_t vaddr = 0;
    if (od->type == MEM_IMM){
      vaddr = od->imm;
    }
    else if(od->type == MEM_REG){
      vaddr = *(od->reg1);
    }
    else if(od->type == MEM_IMM_REG){
      vaddr = od->imm + *(od->reg1);
    }
    else if(od->type == MEM_REG1_REG2){
      vaddr = *(od->reg1)+*(od->reg2);
    }
    else if(od->type == MEM_IMM_REG1_REG2){
      vaddr = od->imm+*(od->reg1)+*(od->reg2);
    }
    else if(od->type == MEM_REG2_SCAL){
      vaddr = (*(od->reg2))*od->scal;
    }
    else if(od->type == MEM_IMM_REG2_SCAL){
      vaddr = od->imm + (*(od->reg2))*od->scal;
    }
    else if(od->type == MEM_REG1_REG2_SCAL){
      vaddr = *(od->reg1) + (*(od->reg2))*od->scal;
    }
    else if(od->type == MEM_IMM_REG1_REG2_SCAL){
      vaddr = od->imm+*(od->reg1) + (*(od->reg2))*od->scal;
    }
    return vaddr;  //取消的解码   
  }
  return 0;
}


static void parse_instruction(const char* str, inst_t *inst, core_t *cr){

}

static void parse_operand(const char* str, od_t *od, core_t *cr){

}


static void mov_handler          (od_t *src_od, od_t* dst_od, core_t *cr);
static void push_handler         (od_t *src_od, od_t* dst_od, core_t *cr);
static void pop_handler          (od_t *src_od, od_t* dst_od, core_t *cr);
static void leave_handler          (od_t *src_od, od_t* dst_od, core_t *cr);
static void call_handler          (od_t *src_od, od_t* dst_od, core_t *cr);
static void ret_handler          (od_t *src_od, od_t* dst_od, core_t *cr);
static void add_handler          (od_t *src_od, od_t* dst_od, core_t *cr);
static void sub_handler          (od_t *src_od, od_t* dst_od, core_t *cr);
static void cmp_handler          (od_t *src_od, od_t* dst_od, core_t *cr);
static void jne_handler          (od_t *src_od, od_t* dst_od, core_t *cr);
static void jmp_handler          (od_t *src_od, od_t* dst_od, core_t *cr);

//handlers to  different instruction types
typedef void (*handler_t)(od_t *, od_t*, core_t*);
//table of pointers
static handler_t handler_table[NUM_INSTRTYPE] = {
  &mov_handler,      //0
  &push_handler,     //1
  &pop_handler,      //2
  &leave_handler,    //3
  &call_handler,     //4
  &ret_handler,      //5
  &add_handler,      //6
  &sub_handler,      //7
  &cmp_handler,      //8
  &jne_handler,      //9
  &jmp_handler,      //10
};

static inline void reset_cflags(core_t *cr){
  cr->CF = 0;
  cr->OF = 0;
  cr->SF = 0;
  cr->ZF = 0;
}


static inline void next_rip(core_t *cr){
  //handle the fixed-length of assmbly string here
  //which can be variable as true in X86 inst
  //risc-v is a fixed length ISA
  cr->rip += sizeof(char)*MAX_INSTRUCTION_CHAR;
}

static void mov_handler(od_t *src_od, od_t* dst_od, core_t *cr){
  uint64_t src = decode_operand(src_od);
  uint64_t dst = decode_operand(dst_od);

  if (src_od->type == REG && dst_od->type == REG){
    //src : reg
    //dst : reg
    *(uint64_t*)dst = *(uint64_t*)src;
    next_rip(cr);
    reset_cflags(cr);
    return;
  }
  // >= relative to memory
  else if(src_od->type == REG && dst_od->type >= MEM_IMM){  
    //src : reg
    //dst : mem va
    wirte64bits_dram(
      va2pa(dst, cr),
      *(uint64_t*)src,
      cr
    );    
    next_rip(cr);
    reset_cflags(cr);
    return;
  }
  else if(src_od->type >= MEM_IMM && dst_od->type == REG){
    //src : mem va
    //dst : reg
    *(uint64_t*)dst = read64bits_dram(
      va2pa(src, cr),
      cr
    );  
    next_rip(cr);
    reset_cflags(cr);
    return;
  }
  else if(src_od->type == IMM && dst_od->type == REG){
    //src : immediate number (uint64_t bit map)
    //dst : reg
    *(uint64_t*)dst = src;
    next_rip(cr);
    reset_cflags(cr);
    return;
  }

}


static void push_handler(od_t *src_od, od_t* dst_od, core_t *cr){
  uint64_t src = decode_operand(src_od);
  // uint64_t dst = decode_operand(dst_od);

  if(src_od->type == REG){
    //src : reg
    //dst : empty
    cr->reg.rsp -= 8;
    wirte64bits_dram(
      va2pa(cr->reg.rsp, cr),
      *(uint64_t*)src,
      cr
    );
    next_rip(cr);
    reset_cflags(cr);
    return;
  }
}

static void pop_handler(od_t *src_od, od_t* dst_od, core_t *cr){
  uint64_t src = decode_operand(src_od);

  if(src_od->type == REG){
    *(uint64_t*)src = read64bits_dram(
      va2pa(cr->reg.rsp, cr),
      cr
    );
  }
  next_rip(cr);
  reset_cflags(cr);
  return;
}

static void leave_handler(od_t *src_od, od_t* dst_od, core_t *cr){

  return ;
}


static void call_handler(od_t *src_od, od_t* dst_od, core_t *cr){
  //src : imm addr of called function 
  //dst : empty
  uint64_t src = decode_operand(src_od);

  cr->reg.rsp -= 8;
  //add rip to stack
  wirte64bits_dram(
    va2pa(cr->reg.rsp, cr),
    cr->reg.rsp+sizeof(char)*MAX_INSTRUCTION_CHAR,
    cr
  );
  cr->reg.rsp = src;
  reset_cflags(cr);
  return;
}

static void ret_handler(od_t *src_od, od_t* dst_od, core_t *cr){
  //src : empty
  //dst : empty
  uint64_t ret_addr = read64bits_dram(
    va2pa(cr->reg.rsp, cr),
    cr
    );
  cr->reg.rsp += 8;
  cr->rip = ret_addr;
  reset_cflags(cr);
  return;
}

static void add_handler(od_t *src_od, od_t* dst_od, core_t *cr){
  //src : reg
  //dst : reg
  uint64_t src = decode_operand(src_od);
  uint64_t dst = decode_operand(dst_od);
  *(uint64_t *)dst += *(uint64_t*)src;
  next_rip(cr);
  reset_cflags(cr);
  return;
}

static void sub_handler(od_t *src_od, od_t* dst_od, core_t *cr){
  //src : reg
  //dst : reg
  uint64_t src = decode_operand(src_od);
  uint64_t dst = decode_operand(dst_od);
  *(uint64_t *)dst -= *(uint64_t*)src;
  next_rip(cr);
  reset_cflags(cr);
  return;
}

static void cmp_handler(od_t *src_od, od_t* dst_od, core_t *cr){

}

static void jne_handler(od_t *src_od, od_t* dst_od, core_t *cr){

}


static void jmp_handler(od_t *src_od, od_t* dst_od, core_t *cr){

}


void instruction_cycle(core_t *cr){
  //fetch
  const char *inst_str = (const char*)cr->rip;
  debug_printf(DEBUG_INSTRUCTIONCYCLE, "%lx      %s\n", (unsigned long)cr->rip, inst_str);


  //DECODE : decode the run-time instruction operands
  inst_t inst;

  parse_instruction(inst_str, &inst, cr);


  //EXECUTE 
  handler_t handler = handler_table[inst.op];
  hanlder(&(inst.src), &(inst.dst), cr);

}


void print_register(core_t *cr){
  if((DEBUG_VERBOSE_SET & DEBUG_REGISTERS) == 0x0){
    return ;
  }


}



void print_regsiter(core_t *cr){
  printf("rax = %16lx\trbx = %16lx\trcx = %16lx\trdx = %16lx\n", 
  (unsigned long)cr->reg.rax, 
  (unsigned long)cr->reg.rbx, 
  (unsigned long)cr->reg.rcx, 
  (unsigned long)cr->reg.rdx);

  printf("rsi = %16lx\trdi = %16lx\trbp = %16lx\trsp = %16lx\n",
  (unsigned long)cr->reg.rsi, 
  (unsigned long)cr->reg.rdi, 
  (unsigned long)cr->reg.rbp, 
  (unsigned long)cr->reg.rsp);
  
  printf("rip = %16lx\n", (unsigned long)cr->rip);
}

void print_stack(core_t *cr){
  int n = 10;
  uint64_t* high = (uint64_t *)&pm[va2pa(cr->reg.rsp, cr)];
  high = &high[n];

  for(int i = 0; i< 2*n;i++){
    uint64_t *ptr = (uint64_t *)(high-i);
    printf("%p: %16lx", ptr, (unsigned long)*ptr);

    if (i==n){
      printf(" <== rsp");
    }
    printf("\n");
  }
}