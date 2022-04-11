// include guards to prevent double declaration of any identifiers 
// such as types, enums and static variables
#ifndef INSTRUCTION_GUARD
#define INSTRUCTION_GUARD

#include <stdint.h>

/*======================================*/
/*      instruction set architecture    */
/*======================================*/

// data structures

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
  INST_LEAVE,   //11
}op_t;


typedef enum OPERAND_TYPE{
  OD_EMPTY,                  //0
  OD_IMM,                    //1
  OD_REG, 
  OD_MEM_IMM,                 //3
  OD_MEM_REG1,
  OD_MEM_IMM_REG1,             //5
  OD_MEM_REG1_REG2,
  OD_MEM_IMM_REG1_REG2,       //7
  OD_MEM_REG2_SCAL,
  OD_MEM_IMM_REG2_SCAL,
  OD_MEM_REG1_REG2_SCAL,      //10
  OD_MEM_IMM_REG1_REG2_SCAL   //11
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

#define MAX_NUM_INSTRUCTION_CYCLE 100

#endif