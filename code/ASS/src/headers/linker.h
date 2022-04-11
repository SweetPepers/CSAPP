#ifndef LINKER_GUARD
#define LINKER_GUARD
#include <stdlib.h>
#include <stdint.h>

#define MAX_CHAR_SECTION_NAME (32)

typedef struct
{
  char sh_name[MAX_CHAR_SECTION_NAME];
  uint64_t sh_addr;
  uint64_t sh_offset; //line offset of effective line index

  uint64_t sh_size;

} sh_entry_t;

#define MAX_CHAR_SYMBOL_NAME (64)

typedef enum
{
  STB_LOACL,
  STB_GLOBAL,
  STB_WEAK
} st_bind_t;

typedef enum
{
  STT_NOTYPE,
  STT_OBJECT,
  STT_FUNC,
} st_type_t;

typedef struct
{
  char st_name[MAX_CHAR_SYMBOL_NAME];
  st_bind_t bind;
  st_type_t type;
  char st_shndex[MAX_CHAR_SECTION_NAME];
  uint64_t st_value; //in-section offset
  uint64_t st_size;  //count of lines of symbol

} st_entry_t;

typedef enum
{
  R_X86_64_32,
  R_X86_64_PC32,
  R_X86_64_PLT32
} reltype_t;

//relocation entry type
typedef struct
{
  //use line index + char offset instead of byte offset to locate the symbol
  uint64_t r_row;   //line index of the symbol in buffer section
  uint64_t r_col;   //char offset in the buffer line
  reltype_t type;   //relocation type
  uint64_t sym;     //symbol table index
  int64_t r_addend; //constant part of relocation expression
} rl_entry_t;

#define MAX_ELF_FILE_LENGTH (64) //max 64 effective lines
#define MAX_ELF_FILE_WIDTH (128) //max 128 chars per line

typedef struct
{
  char buffer[MAX_ELF_FILE_LENGTH][MAX_ELF_FILE_WIDTH];
  uint64_t line_count;

  uint64_t sht_count;
  sh_entry_t *sht;

  uint64_t symt_count;
  st_entry_t *symt;

  uint64_t reltext_count;
  rl_entry_t *reltext;

  uint64_t reldata_count;
  rl_entry_t *reldata;

} elf_t;

typedef struct
{
  elf_t *elf;
  st_entry_t *src;
  st_entry_t *dst; //dst symbol:used for reloaction -find the function referencing the undefined symbol
  //TODO:
  //relocation entry(referencing section, referenced symbol) converted to (referencing symbol, referenced symbol)
} smap_t;

void parse_elf(char *filename, elf_t *elf);

void free_elf(elf_t *elf);

void link_elf(elf_t **srcs, int num_srcs, elf_t *dst);

void write_eof(const char *filename, elf_t *eof);
#endif