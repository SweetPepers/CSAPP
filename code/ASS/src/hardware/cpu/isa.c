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

static const char* reg_name_list[72] = {
  "%rax", "%eax", "%ax", "%ah", "%al",
  "%rbx", "%ebx", "%bx", "%bh", "%bl",
  "%rcx", "%ecx", "%cx", "%ch", "%cl",
  "%rdx", "%edx", "%dx", "%dh", "%dl",
  "%rsi", "%esi", "%si", "%sih", "%sil",
  "%rdi", "%edi", "%di", "%dih", "%dil",
  "%rbp", "%ebp", "%bp", "%bph", "%bpl",
  "%rsp", "%esp", "%sp", "%sph", "%spl",
  "%r8", "%r8d", "%r8w", "%r8b",
  "%r9", "%r9d", "%r9w", "%r9b",
  "%r10", "%r10d", "%r10w", "%r10b",
  "%r11", "%r11d", "%r11w", "%r11b",
  "%r12", "%r12d", "%r12w", "%r12b",
  "%r13", "%r13d", "%r13w", "%r13b",
  "%r14", "%r14d", "%r14w", "%r14b",
  "%r15", "%r15d", "%r15w", "%r15b",
};


static uint64_t reflect_register(const char* str, core_t *cr){
  //cr->reg is runtime existing
  reg_t *reg = &(cr->reg);
  uint64_t reg_addr[72] = {
    (uint64_t)&(reg->rax),(uint64_t)&(reg->eax),(uint64_t)&(reg->ax),(uint64_t)&(reg->ah),(uint64_t)&(reg->al),
    (uint64_t)&(reg->rbx),(uint64_t)&(reg->ebx),(uint64_t)&(reg->bx),(uint64_t)&(reg->bh),(uint64_t)&(reg->bl),
    (uint64_t)&(reg->rcx),(uint64_t)&(reg->ecx),(uint64_t)&(reg->cx),(uint64_t)&(reg->ch),(uint64_t)&(reg->cl),
    (uint64_t)&(reg->rdx),(uint64_t)&(reg->edx),(uint64_t)&(reg->dx),(uint64_t)&(reg->dh),(uint64_t)&(reg->dl),
    (uint64_t)&(reg->rsi),(uint64_t)&(reg->esi),(uint64_t)&(reg->si),(uint64_t)&(reg->sih),(uint64_t)&(reg->sil),
    (uint64_t)&(reg->rdi),(uint64_t)&(reg->edi),(uint64_t)&(reg->di),(uint64_t)&(reg->dih),(uint64_t)&(reg->dil),
    (uint64_t)&(reg->rbp),(uint64_t)&(reg->ebp),(uint64_t)&(reg->bp),(uint64_t)&(reg->bph),(uint64_t)&(reg->bpl),
    (uint64_t)&(reg->rsp),(uint64_t)&(reg->esp),(uint64_t)&(reg->sp),(uint64_t)&(reg->sph),(uint64_t)&(reg->spl),
    (uint64_t)&(reg->r8),(uint64_t)&(reg->r8d),(uint64_t)&(reg->r8w),(uint64_t)&(reg->r8b),
    (uint64_t)&(reg->r9),(uint64_t)&(reg->r9d),(uint64_t)&(reg->r9w),(uint64_t)&(reg->r9b),
    (uint64_t)&(reg->r10),(uint64_t)&(reg->r10d),(uint64_t)&(reg->r10w),(uint64_t)&(reg->r10b),
    (uint64_t)&(reg->r11),(uint64_t)&(reg->r11d),(uint64_t)&(reg->r11w),(uint64_t)&(reg->r11b),
    (uint64_t)&(reg->r12),(uint64_t)&(reg->r12d),(uint64_t)&(reg->r12w),(uint64_t)&(reg->r12b),
    (uint64_t)&(reg->r13),(uint64_t)&(reg->r13d),(uint64_t)&(reg->r13w),(uint64_t)&(reg->r13b),
    (uint64_t)&(reg->r14),(uint64_t)&(reg->r14d),(uint64_t)&(reg->r14w),(uint64_t)&(reg->r14b),
    (uint64_t)&(reg->r15),(uint64_t)&(reg->r15d),(uint64_t)&(reg->r15w),(uint64_t)&(reg->r15b),
  };



  for (int i = 0;i<72;i++){
    if(strcmp(str, reg_name_list[i]) == 0){
      //inside reg_name_list
      return reg_addr[i];
    }
  }

  printf("parse register %s error\n", str);
  exit(0);

}

#define MAX_NODE 1000
#define CHARSET 26
static int trie[MAX_NODE][CHARSET] = {0};
static int trie_value[MAX_NODE] = {0};
static int trie_k = 1;

static void trie_insert(char *w, int value){
    int len = strlen(w);
    int p = 0;
    for(int i=0; i<len; i++){
        int c = w[i] - 'a';
        if(!trie[p][c]){
            trie[p][c] = trie_k;
            trie_k++;
        }
        p = trie[p][c];
    }
    trie_value[p] = value;
}

static int trie_search(char *s){
    int len = strlen(s);
    int p = 0;
    for(int i=0; i<len; i++){
        int c = s[i] - 'a';
        if(!trie[p][c]) return 0;
        p = trie[p][c];
    }
    return trie_value[p];
}

/*
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
*/

static void build_trie(){
  // int inst_nums = 11;
  char* inst_str[13] = {
    "mov",
    "movq",
    "push",
    "pop",
    "leaveq",
    "callq",
    "retq",
    "ret",
    "add",
    "sub",
    "cmpq",
    "jne",
    "jmp",
  };
  int value[13] = {
    0,
    0,
    1,
    2,
    3,
    4,
    5,
    5,
    6,
    7,
    8,
    9,
    10,
  };

  for (int i = 0; i<13;i++){
    trie_insert(inst_str[i], value[i]);
  }
  
}

static void parse_instruction(const char* str, inst_t *inst, core_t *cr){
  char op_str[64] = {'\0'};
  int op_len = 0;
  char src_str[64] = {'\0'};
  int src_len = 0;
  char dst_str[64] = {'\0'};
  int dst_len = 0;

  char c;
  int count_parentheses = 0;
  int state = 0;

  for(int i = 0;i<strlen(str);i++){
    c = str[i];
    if(c == '(' || c == ')'){
      count_parentheses ++;
    }

    //state transfer
    if(state == 0 && c != ' '){
      state = 1;
    }else if(state == 1 && c == ' '){
      state = 2;
      continue;
    }else if(state == 2 && c != ' '){
      state = 3;
      
    }else if(state == 3 && c == ',' && (count_parentheses == 0||count_parentheses == 2)){
      state = 4;
      continue;
    }else if(state == 4 && c != ' '&& c != ','){
      state = 5;
      
    }else if(state == 5 && c == ' '){
      state = 6;
      continue;
    }


    if(state == 1){
      op_str[op_len] = c;
      op_len++;
      continue;
    }else if(state == 3){
      src_str[src_len] = c;
      src_len++;
      continue;
    }else if(state == 5){
      dst_str[dst_len] = c;
      dst_len++;
      continue;
    }


  
  } //end for


  //op_str , src_str, dst_str

  parse_operand(src_str, &inst->src, cr);
  parse_operand(dst_str, &inst->dst, cr);



  
  //better done by trie tree   前缀树 / 字典树
  build_trie();
  inst->op = trie_search(op_str);

  debug_printf(DEBUG_PARSEINST, "[%s (%d)] [%s (%d)]  [%s (%d)]\n", 
  op_str, inst->op, src_str, inst->src.type, dst_str, inst->dst.type);
}

void TestParseInstruction(){
  ACTIVE_CORE = 0x0;
  core_t *ac = (core_t *)&cores[ACTIVE_CORE];
  
  char assembly[15][MAX_INSTRUCTION_CHAR] = {
    "push  %rbp",            //0
    "mov %rsp, %rbp",        //1
    "mov %rdi, -0x18(%rbp)", //2
    "mov %rsi, -0x20(%rbp)", //3
    "mov -0x18(%rbp), %rdx", //4
    "mov -0x20(%rbp), %rax", //5
    "add %rdx, %rax",        //6
    "mov %rax, -0x8(%rbp)",  //7
    "mov -0x8(%rbp), %rax",  //8
    "pop %rbp",              //9
    "retq",                  //10
    "mov %rdx, %rsi",        //11
    "mov %rax, %rdi",        //12
    "callq 0",               //13
    "mov %rax, -0x8(%rbp)",  //14
  };

  inst_t inst;
  for(int i = 0; i< 15;i++){
    parse_instruction(assembly[i], &inst, ac);
  }


}


static void parse_operand(const char* str, od_t *od, core_t *cr){
  //str : assembly code string , e.g. mov $rsp,$rbp
  //od : pointer to the address to store the parsed operand
  //cr : active core processor


  od->type = EMPTY;
  od->imm = 0;
  od->reg1 = 0;
  od->reg2 = 0;
  od->scal = 0;
  int str_len = strlen(str); // tail is '\n'

  if (str_len == 0){ //EMPTY
    return;
  }

  if(str[0] == '$'){
    //immediate
    od->type = IMM;
    od->imm = string2uint_range(str, 1,-1);
  }else if(str[0] == '%'){
    //register
    od->type = REG;
    od->reg1 = (uint64_t *)reflect_register(str, cr); 
    return;
  }else
  {
    //memory     legal

    char imm[64] = {'\0'};
    int imm_len = 0;
    
    char reg1[64] = {'\0'};
    int reg1_len = 0;
    
    char reg2[64]= {'\0'};
    int reg2_len = 0;
    
    char scal[64]= {'\0'};
    int scal_len = 0;

    int ca = 0; // (  parentheses
    int cb = 0; // , comma
    for (int i = 0; i< str_len;++i){
      char c = str[i];
      if(c == '(' || c == ')'){
        ca ++;
      }else if (c == ','){
        cb ++;
      }else{
        //parse imm(reg1,reg2,scal) 
        //not allow ' '
        if (ca == 0){
          // xxx
          imm[imm_len] = c;
          imm_len++;
          continue;
        }
        else if (ca == 1)
        {
          if(cb == 0){
            //???(xxxx
            //(xxx
            reg1[reg1_len] = c;
            reg1_len++;
            continue;
          }else if(cb == 1){
            //???(???,xxxx
            //(???,xxxx
            //???(,xxx
            //(,xxx
            reg2[reg2_len] = c;
            reg2_len++;
            continue;
          }else if(cb == 2){
            //(???,???,xxx
            scal[scal_len] = c;
            scal_len++;
            continue;
          }
          
        }
        
      }
    } //end for

    // imm,reg1,reg2,scal
    if(imm_len>0){
      od->imm = string2uint(imm); 
      if(ca == 0){  //don't have reg
        od->type = MEM_IMM;
        return;
      }
    }
    if (scal_len>0){
       od->scal = string2uint(scal);
       //scal can only be 1,2,4 and 8
       if(od->scal!= 1 &&od->scal!= 2 &&od->scal!= 4 &&od->scal!= 8){
         printf("%s is not a legal scaler\n",scal);
         exit(0);
       }
    }
    if(reg1_len>0){
      od->reg1 = (uint64_t *)reflect_register(reg1,cr);
    }
    if(reg2_len>0){
      od->reg2 = (uint64_t *)reflect_register(reg2, cr);
    }

    //set operand
    if(cb == 0){  // , = 1
      od->type = imm_len > 0? MEM_IMM_REG : MEM_REG;
    }else if(cb == 1){
      od->type = imm_len>0 ? MEM_IMM_REG1_REG2:MEM_REG1_REG2;
    }else if(cb == 2){
      if (reg1_len >0){
        od->type = imm_len>0? MEM_IMM_REG1_REG2_SCAL:MEM_REG1_REG2_SCAL;
      }else{
        od->type = imm_len>0?MEM_IMM_REG2_SCAL:MEM_REG2_SCAL;
      }
    }    


  } // end else


}


void TestPaseingOperand(){
  ACTIVE_CORE = 0;

  core_t *ac = (core_t *)&cores[ACTIVE_CORE];
  const char* strs[11] = {
    "$0x123",
    "%rbp",
    "0xabcd",
    "(%rsp)",
    "-0x18(%rbp)",
    "(%rsp,%rax)",
    "0xabcd(%rax,%rbx)", //7
    "(,%rax,8)", //8
    "0xabcd(,%rax,8)", //9
    "(%rax,%rbx,8)",  //10
    "0xabcd(%rax,%rbx,4)"  //11
  };

  printf("rax %p\n", &(ac->reg.rax));
  printf("rbx %p\n", &(ac->reg.rbx));
  printf("rsp %p\n", &(ac->reg.rsp));

  for (int i =0;i<11;i++){
    od_t od;
    parse_operand(strs[i],&od, ac);

    printf("\n%s\n", strs[i]);
    printf("od enum type : %d\n", od.type);
    printf("od imm: %16lx\n", (uint64_t)od.imm);
    printf("od reg1:%16lx\n", (uint64_t)od.reg1);
    printf("od reg2:%16lx\n", (uint64_t)od.reg2);
    printf("od scal:%16lx\n", (uint64_t)od.scal);
  }
}
static void mov_handler          (od_t *src_od, od_t* dst_od, core_t *cr);
static void push_handler         (od_t *src_od, od_t* dst_od, core_t *cr);
static void pop_handler          (od_t *src_od, od_t* dst_od, core_t *cr);
static void leave_handler        (od_t *src_od, od_t* dst_od, core_t *cr);
static void call_handler         (od_t *src_od, od_t* dst_od, core_t *cr);
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
  cr->flags.__flag_values = 0;
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
  cr->reg.rsp += 8;
  next_rip(cr);
  reset_cflags(cr);
  return;
}

static void leave_handler(od_t *src_od, od_t* dst_od, core_t *cr){
  //mov %rbp, %rsp
  cr->reg.rsp = cr->reg.rbp;

  //pop %rbp
  cr->reg.rbp = read64bits_dram(
      va2pa(cr->reg.rsp, cr),
      cr
    );
  cr->reg.rsp += 8;

  reset_cflags(cr);
  next_rip(cr);
  return ;
}


static void call_handler(od_t *src_od, od_t* dst_od, core_t *cr){
  //src : imm addr of called function 
  //dst : empty
  uint64_t src = decode_operand(src_od);

  
  //add rip to stack
  wirte64bits_dram(
    va2pa(cr->reg.rsp, cr),
    cr->rip+sizeof(char)*MAX_INSTRUCTION_CHAR,
    cr
  );

  cr->reg.rsp -= 8;
  cr->rip = src;
  reset_cflags(cr);
  return;
}

static void ret_handler(od_t *src_od, od_t* dst_od, core_t *cr){
  //src : empty
  //dst : empty
  cr->reg.rsp += 8;
  uint64_t ret_addr = read64bits_dram(
    va2pa(cr->reg.rsp, cr),
    cr
    );
  
  cr->rip = ret_addr;
  reset_cflags(cr);
  return;
}

static void add_handler(od_t *src_od, od_t* dst_od, core_t *cr){
  
  uint64_t src = decode_operand(src_od);
  uint64_t dst = decode_operand(dst_od);
  uint64_t val = *(uint64_t *)dst;


  
  if(src_od->type == REG && dst_od->type == REG){
    //src : reg
    //dst : reg
    src = *(uint64_t*)src;
    //set flags
  }else if(src_od->type == MEM_IMM && dst_od->type == REG){
    //src : mem
    //dst : reg
    src = read64bits_dram(
      va2pa(src, cr),
      cr
    );
  }else if(src_od->type == IMM && dst_od->type == REG){
    //src : imm num
    //dst : reg
    src = src;
  }

  val += src;

  //set  flags
  int val_sign = (val >> 63)&0x1;
  int src_sign = (src >> 63)&0x1;
  int dst_sign = (*(uint64_t *)dst >> 63)&0x1;  
  //unsigned overflow
  cr->flags.CF = (val < src);  
  cr->flags.ZF = (val == 0);
  cr->flags.SF = ((val>>63) & 0x1);
  cr->flags.OF = (val_sign == 1&&src_sign ==0 && dst_sign == 0) ||(val_sign == 0 &&src_sign ==1 && dst_sign == 1);


  *(uint64_t *)dst = val;
  next_rip(cr);
  // reset_cflags(cr);
  return;
}

static void sub_handler(od_t *src_od, od_t* dst_od, core_t *cr){
  //dst = dst - src
  uint64_t src = decode_operand(src_od);
  uint64_t dst = decode_operand(dst_od);
  uint64_t val = *(uint64_t *)dst;

  if(src_od->type == REG && dst_od->type == REG){
    //src : reg
    //dst : reg
    src = *(uint64_t*)src;
    //set flags
  }else if(src_od->type == MEM_IMM && dst_od->type == REG){
    //src : mem
    //dst : reg
    src = read64bits_dram(
      va2pa(src, cr),
      cr
    );
  }else if(src_od->type == IMM && dst_od->type == REG){
    //src : imm num
    //dst : reg
    src = src;
  }
  src = ~src+1;  // sign
  val += src;

  //set  flags
  int val_sign = (val >> 63)&0x1;
  int src_sign = (src >> 63)&0x1;
  int dst_sign = (*(uint64_t *)dst >> 63)&0x1;

  cr->flags.CF = (val < src);  
  cr->flags.ZF = (val == 0);
  cr->flags.SF = ((val>>63) & 0x1);
  cr->flags.OF = (val_sign == 1&&src_sign ==0 && dst_sign == 0) ||(val_sign == 0 &&src_sign ==1 && dst_sign == 1);


  *(uint64_t *)dst = val;
  next_rip(cr);
  // reset_cflags(cr);
  return;
}

static void cmp_handler(od_t *src_od, od_t* dst_od, core_t *cr){
  //dst = dst - src
  uint64_t src = decode_operand(src_od);
  uint64_t dst = decode_operand(dst_od);
  uint64_t val = 0;
  if(src_od->type == IMM && dst_od->type == MEM_IMM_REG){
    //src : imm
    //dst : mem_imm
    src = ~src+1;
    dst = read64bits_dram(va2pa(dst, cr), cr);
    val = dst + src;
    //set flags
  }
  // }else if(src_od->type == MEM_IMM && dst_od->type == REG){
  //   //src : mem
  //   //dst : reg
  //   src = read64bits_dram(
  //     va2pa(src, cr),
  //     cr
  //   );
  // }else if(src_od->type == IMM && dst_od->type == REG){
  //   //src : imm num
  //   //dst : reg
  //   src = src;
  // }

  //set  flags
  int val_sign = (val >> 63)&0x1;
  int src_sign = (src >> 63)&0x1;
  int dst_sign = (dst >> 63)&0x1;

  cr->flags.CF = (val < src);  
  cr->flags.ZF = (val == 0);
  cr->flags.SF = ((val>>63) & 0x1);
  cr->flags.OF = (val_sign == 1&&src_sign ==0 && dst_sign == 0) ||(val_sign == 0 &&src_sign ==1 && dst_sign == 1);


  // *(uint64_t *)dst = val;
  next_rip(cr);
  // reset_cflags(cr);
  return;
}

static void jne_handler(od_t *src_od, od_t* dst_od, core_t *cr){
  uint64_t src = decode_operand(src_od);
  // uint64_t dst = decode_operand(dst_od);
  // if(src_od->type == IMM){
  //reason  $  so src is not a imm, program regard it as mem_imm
  // src_od->type = 3(MEM_IMM)

  if(cr->flags.ZF != 1){
    //last instruction val != 0
    //jump
    cr->rip = src;
  }
  next_rip(cr);
  reset_cflags(cr);
}


static void jmp_handler(od_t *src_od, od_t* dst_od, core_t *cr){
  uint64_t src = decode_operand(src_od);
  cr->rip = src;
  reset_cflags(cr);
}


void instruction_cycle(core_t *cr){
  //fetch
  // const char *inst_str = (const char*)cr->rip;
  char inst_str[MAX_INSTRUCTION_CHAR+10];
  readinst_dram(va2pa(cr->rip, cr),inst_str, cr);

  debug_printf(DEBUG_INSTRUCTIONCYCLE, "%16lx      %s\n", cr->rip, inst_str);


  //DECODE : decode the run-time instruction operands
  inst_t inst;

  parse_instruction(inst_str, &inst, cr);


  //EXECUTE 
  handler_t handler = handler_table[inst.op];
  handler(&(inst.src), &(inst.dst), cr);

}

void print_register(core_t *cr){
  if ((DEBUG_VERBOSE_SET & DEBUG_REGISTERS) == 0x0){
    return ;
  }

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

  printf("CF=%u\tZF=%u\tSF=%u\tOF=%u\n", 
  cr->flags.CF, cr->flags.ZF, cr->flags.SF, cr->flags.OF);
}

void print_stack(core_t *cr){
  if((DEBUG_VERBOSE_SET & DEBUG_PRINTSTACK) == 0x0){
    return ;
  }

  int n = 10;
  uint64_t* high = (uint64_t *)&pm[va2pa(cr->reg.rsp, cr)];
  high = &high[n];

  uint64_t va = (cr->reg).rsp + n*8;

  for(int i = 0; i< 2*n;i++){
    uint64_t *ptr = (uint64_t *)(high-i);
    printf("0x%16lx: %16lx", va, (uint64_t)*ptr);

    if (i==n){
      printf(" <== rsp");
    }
    printf("\n");
    va -= 8;
  }
}


