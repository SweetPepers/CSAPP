#ifndef INST_GUARD
#define INST_GUARD
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>



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

#endif