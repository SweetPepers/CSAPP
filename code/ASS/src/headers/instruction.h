// include guards to prevent double declaration of any identifiers
// such as types, enums and static variables
#ifndef INSTRUCTION_GUARD
#define INSTRUCTION_GUARD

#include <stdint.h>

/*======================================*/
/*      instruction set architecture    */
/*======================================*/

typedef enum OPERAND_TYPE
{
  OD_EMPTY, // 0
  OD_IMM,   // 1
  OD_REG,   // 2
  OD_MEM,   // 3
} od_type_t;

typedef struct OPERAND_STRUCT
{
  od_type_t type; // OD_IMM, OD_REG, OD_MEM
  uint64_t value; // the value
} od_t;

// handler table storing the handlers to different instruction types
typedef void (*op_t)(od_t *, od_t *);

// local variables are allocated in stack in run-time
// we don't consider local STATIC variables
// ref: Computer Systems: A Programmer's Perspective 3rd
// Chapter 7 Linking: 7.5 Symbols and Symbol Tables
typedef struct INST_STRUCT
{
  op_t op;  // enum of operators. e.g. mov, call, etc.
  od_t src; // operand src of instruction
  od_t dst; // operand dst of instruction
} inst_t;

#define MAX_NUM_INSTRUCTION_CYCLE 100

#endif

// // include guards to prevent double declaration of any identifiers
// // such as types, enums and static variables
// #ifndef INSTRUCTION_GUARD
// #define INSTRUCTION_GUARD

// #include <stdint.h>

// /*======================================*/
// /*      instruction set architecture    */
// /*======================================*/

// // data structures

// typedef enum INST_OPERATOR{
//   INST_MOV,     //0
//   INST_PUSH,    //1
//   INST_POP,     //2
//   INTS_LEAVE,   //3
//   INST_CALL,    //4
//   INST_RET,     //5
//   INST_ADD,     //6
//   INST_SUB,     //7
//   INST_CMP,     //8
//   INST_JNE,     //9
//   INST_JMP,     //10
//   INST_LEAVE,   //11
//   INST_LEA,     //12
// }op_t;

// typedef enum OPERAND_TYPE{
//   OD_EMPTY,                  //0
//   OD_IMM,                    //1
//   OD_REG,
//   OD_MEM_IMM,                 //3
//   OD_MEM_REG1,
//   OD_MEM_IMM_REG1,             //5
//   OD_MEM_REG1_REG2,
//   OD_MEM_IMM_REG1_REG2,       //7
//   OD_MEM_REG2_SCAL,
//   OD_MEM_IMM_REG2_SCAL,
//   OD_MEM_REG1_REG2_SCAL,      //10
//   OD_MEM_IMM_REG1_REG2_SCAL   //11
// } od_type_t;

// //operand
// typedef struct OPERAND_STRUCT{
//   od_type_t type; //imm reg mem

//   uint64_t value;
//   int64_t imm;
//   int64_t scal;  //scale number to register 2
//   uint64_t *reg1;
//   uint64_t *reg2;
// }od_t;

// //csapp 7.5 symbols and symbol tables
// typedef struct INSTRUCT_STRUCT{
//   op_t op;  //mov push
//   od_t src;
//   od_t dst;
// }inst_t;

// // handler table storing the handlers to different instruction types
// typedef void (*handler_t)(od_t *, od_t *);

// #define MAX_NUM_INSTRUCTION_CYCLE 100

// #endif