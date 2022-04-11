#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "headers/linker.h"
#include "headers/common.h"
#include "headers/instruction.h"
#include "headers/cpu.h"

#define MAX_SYMBOL_MAP_LENGTH 64
#define MAX_SECTION_BUFFER_LENGTH 64

//internal mapping between source and destination symbol entries

static void symbol_processing(elf_t **srcs, int num_srcs, elf_t *dst, smap_t *smap_table, int *smap_count);
static void simple_resolution(st_entry_t *sym, elf_t *sym_elf, smap_t *candidate);
static void compute_section_header(elf_t *dst, smap_t *smap_table, int *smap_count);
static void merge_section(elf_t **srcs, int num_srcs, elf_t *dst, smap_t *smap_table, int *smap_count);

/*

  relocation

*/
static void relocation_processing(elf_t **srcs, int num_srcs, elf_t *dst, smap_t *smap_table, int *smap_count);

static void R_X86_64_32_handler(elf_t *dst, sh_entry_t *sh, int row_referencing, int col_referencing,
                                int addend, st_entry_t *sym_referenced);
static void R_X86_64_PC32_handler(elf_t *dst, sh_entry_t *sh, int row_referencing, int col_referencing,
                                  int addend, st_entry_t *sym_referenced);

typedef void (*rela_handler_t)(elf_t *dst, sh_entry_t *sh, int row_referencing, int col_referencing,
                               int addend, st_entry_t *sym_referenced);

static rela_handler_t handler_table[3] = {
    &R_X86_64_32_handler,   //1
    &R_X86_64_PC32_handler, //2
    &R_X86_64_PC32_handler, //3
    //PC32 is same as PLT32
};

static const char *get_stb_string(st_bind_t bind);
static const char *get_stt_string(st_type_t type);

void link_elf(elf_t **srcs, int num_srcs, elf_t *dst)
{
  memset(dst, 0, sizeof(elf_t));

  //create the map table to connect the source elf filese and destination elf file symbols
  int smap_count = 0;
  smap_t smap_table[MAX_SYMBOL_MAP_LENGTH];

  //update the smap table - symbol processing
  symbol_processing(srcs, num_srcs, dst, (smap_t *)&smap_table, &smap_count);

  printf("---------------------smap_table-----------------------\n");

  for (int i = 0; i < smap_count; i++)
  {
    st_entry_t *ste = smap_table[i].src;
    debug_printf(DEBUG_LINKER, "%s\t%d\t%d\t%s\t%x\t%x\n",
                 ste->st_name,
                 ste->bind,
                 ste->type,
                 ste->st_shndex,
                 ste->st_value,
                 ste->st_size);
  }
  //compute dst section header table and write into buffer
  compute_section_header(dst, smap_table, &smap_count);

  dst->symt = malloc(dst->symt_count * sizeof(st_entry_t));

  // merge the symbol content from ELF src into dst sections
  merge_section(srcs, num_srcs, dst, (smap_t *)&smap_table, &smap_count);
  printf("-------------------------------\n");
  printf("after mergeing the sections:\n");
  for (int i = 0; i < dst->line_count; i++)
  {
    printf("%s\n", dst->buffer[i]);
  }

  //update buffer:relocate the referencing  in buffer
  //relocating: update the relocation entries from ELF files into EOF buffer
  relocation_processing(srcs, num_srcs, dst, (smap_t *)&smap_table, &smap_count);

  //check EOF file
  if ((DEBUG_VERBOSE_SET & DEBUG_LINKER) != 0)
  {
    printf("-------\nfinal output EOF:\n");
    for (int i = 0; i < dst->line_count; i++)
    {
      printf("%s\n", dst->buffer[i]);
    }
  }
}

static void symbol_processing(elf_t **srcs, int num_srcs, elf_t *dst, smap_t *smap_table, int *smap_count)
{
  dst->symt_count = 0;
  //for every elf files
  for (int i = 0; i < num_srcs; i++)
  {
    elf_t *elfp = srcs[i]; //pointer
    //for every symbol from this elf file
    dst->symt_count += elfp->symt_count;
    for (int j = 0; j < elfp->symt_count;)
    {

      st_entry_t *sym = &(elfp->symt[j]);
      if (sym->bind == STB_LOACL)
      {
        //insert the static symbol to new elf with confidence:
        //compiler would check if the symbol is redeclared symbol
        assert(*smap_count < MAX_SYMBOL_MAP_LENGTH);
        //even if local symbol has the same name, just insert it into dst
        smap_table[*smap_count].src = sym;
        smap_table[*smap_count].elf = elfp;
        (*smap_count)++;
        //source symbol should be used by destination symbol
      }
      else if (sym->bind == STB_GLOBAL)
      {
        //ignore STB_WEAK
        //for other bind:GLOBAL and WEAK
        //it's possible to have name conflict
        //check if this symbol has been used by destination symbol
        for (int k = 0; k < *smap_count; k++)
        {

          // check name conflict
          st_entry_t *candidate = smap_table[k].src;

          if (candidate->bind == STB_GLOBAL &&
              (strcmp(candidate->st_name, sym->st_name) == 0))
          {
            simple_resolution(sym, elfp, &smap_table[k]);
            dst->symt_count--;
            goto NEXT_SYMBOL_PROCESS;
          }
        }
        //don't find any conflict
        assert(*smap_count < MAX_CHAR_SECTION_NAME);
        //update the smap
        smap_table[*smap_count].src = sym;
        smap_table[*smap_count].elf = elfp;
        (*smap_count)++;
      }

    NEXT_SYMBOL_PROCESS:
      j++;
    }
  }

  for (int i = 0; i < *smap_count; i++)
  {
    //check SHN_UNDEF here
    st_entry_t *s = smap_table[i].src;
    assert(strcmp(s->st_shndex, "SHN_UNDEF") != 0);
    assert(s->type != STT_NOTYPE);

    //the remaining COMMON go to .bss
    if (strcmp(s->st_shndex, "COMMON") == 0)
    {
      char *bss = ".bss";
      for (int j = 0; j < strlen(s->st_shndex); j++)
      {
        if (j < 4)
        {
          s->st_shndex[j] = bss[j];
        }
        else
        {
          s->st_shndex[j] = '\0';
        }
      }
      s->st_value = 0;
    }
  }
}

static inline int symbol_precedence(st_entry_t *sym)
{
  /*
    bind    type     shndex            prec
    ----------------------------------------
    global  notype   undef             0-undefined
    global  object   data,bss,rodata   2-defined
    global  object   common            1-tentative
    global  func     text              2-defined  
   */
  assert(sym->bind == STB_GLOBAL);
  if (strcmp(sym->st_shndex, "SHN_UNDEF") == 0 && sym->type == STT_NOTYPE)
  {
    return 0;
  }
  if (strcmp(sym->st_shndex, "COMMON") == 0 && sym->type == STT_OBJECT)
  {
    //Tentative : section to be decided after symbol resolution
    return 1;
  }
  if ((strcmp(sym->st_shndex, ".text") == 0 && sym->type == STT_FUNC) ||
      (strcmp(sym->st_shndex, ".data") == 0 && sym->type == STT_OBJECT) ||
      (strcmp(sym->st_shndex, ".rodata") == 0 && sym->type == STT_OBJECT) ||
      (strcmp(sym->st_shndex, ".bss") == 0 && sym->type == STT_OBJECT))
  {
    return 2;
  }

  //error
  debug_printf(DEBUG_LINKER, "symbol resolution:cannot determine the symbol \"%s\"precedence\n ", sym->st_name);
  exit(0);
}

static void simple_resolution(st_entry_t *sym, elf_t *sym_elf, smap_t *candidate)
{
  //sym: symbol from current elf file
  //candidate: pointer to the internal map table slot: src -> dst

  //determines with symbol is the one to be kept with 3 rules
  //csapp 7.6.1
  //rule 1: multiple strong symbols with the same name are not allowed
  //rule 2: given a strong symbol and mutiple weak symbols with the same name, choose the strong symbol
  //rule 3: given multiple weak symbols with the same name, choose any of the weak symbols

  int pre1 = symbol_precedence(sym);
  int pre2 = symbol_precedence(candidate->src);

  //implement rule 1
  if (pre1 == 2 && pre2 == 2)
  {
    debug_printf(DEBUG_LINKER,
                 "symbol resolution:strong symbol \"%s\" is redeclared\n", sym->st_name); //not allowed
    exit(0);
  }
  //rule 2
  if (pre1 > pre2)
  {
    candidate->src = sym;
    candidate->elf = sym_elf;
  }
  //rule 3 do nothing
}

static void compute_section_header(elf_t *dst, smap_t *smap_table, int *smap_count)
{
  //only have .text, .rodate, .data as symbols in the section
  // .bss is not taking any section memory
  int count_text = 0, count_rodata = 0, count_data = 0;
  for (int i = 0; i < *smap_count; i++)
  {
    st_entry_t *sym = smap_table[i].src;
    if (strcmp(sym->st_shndex, ".text") == 0)
    {
      count_text += sym->st_size;
    }
    else if (strcmp(sym->st_shndex, ".rodata") == 0)
    {
      count_rodata += sym->st_size;
    }
    else if (strcmp(sym->st_shndex, ".data") == 0)
    {
      count_data += sym->st_size;
    }
  }

  //count the section
  dst->sht_count = (count_text != 0) + (count_data != 0) + (count_rodata != 0) + 1; // with symtab
  //count the total lines
  dst->line_count = 1 + 1 + dst->sht_count + count_text + count_rodata + count_data + *smap_count;

  //the target dst : line_count, sht_count, sht,.text,.rodata, .data, .symtab
  //print target dst to buffer
  sprintf(dst->buffer[0], "%ld", dst->line_count);
  sprintf(dst->buffer[1], "%ld", dst->sht_count);

  //compute the run-time address of the sections:compact in memory
  uint64_t text_runtime_addr = 0x00400000;
  uint64_t rodata_runtime_addr = text_runtime_addr + count_text * MAX_INSTRUCTION_CHAR * sizeof(char);
  uint64_t data_runtime_addr = rodata_runtime_addr + count_rodata * sizeof(uint64_t);
  uint64_t symtab_runtime_addr = 0; //for eof, symtab is not loaded into memroy but still on disk

  //write section header table
  assert(dst->sht == NULL);
  dst->sht = malloc(dst->sht_count * sizeof(sh_entry_t));

  //write in .text .rodata .data order
  //the start of the offset
  uint64_t section_offset = 1 + 1 + dst->sht_count;
  int sh_index = 0;
  sh_entry_t *sh = NULL;

  if (count_text > 0)
  {
    //get the pointer
    assert(sh_index < dst->sht_count);
    sh = &(dst->sht[sh_index]);

    //write the fields
    strcpy(sh->sh_name, ".text");
    sh->sh_addr = text_runtime_addr;
    sh->sh_offset = section_offset;
    sh->sh_size = count_text;

    //write into buffer
    sprintf(dst->buffer[1 + 1 + sh_index], "%s,0x%lx,%ld,%ld",
            sh->sh_name, sh->sh_addr, sh->sh_offset, sh->sh_size);

    //update the index
    sh_index++;
    section_offset += sh->sh_size;
  }
  if (count_rodata > 0)
  {
    //get the pointer
    assert(sh_index < dst->sht_count);
    sh = &(dst->sht[sh_index]);

    //write the fields
    strcpy(sh->sh_name, ".rodata");
    sh->sh_addr = rodata_runtime_addr;
    sh->sh_offset = section_offset;
    sh->sh_size = count_rodata;

    //write into buffer
    sprintf(dst->buffer[1 + 1 + sh_index], "%s,0x%lx,%ld,%ld",
            sh->sh_name, sh->sh_addr, sh->sh_offset, sh->sh_size);

    //update the index
    sh_index++;
    section_offset += sh->sh_size;
  }
  if (count_data > 0)
  {
    //get the pointer
    assert(sh_index < dst->sht_count);
    sh = &(dst->sht[sh_index]);

    //write the fields
    strcpy(sh->sh_name, ".data");
    sh->sh_addr = data_runtime_addr;
    sh->sh_offset = section_offset;
    sh->sh_size = count_data;

    //write into buffer
    sprintf(dst->buffer[1 + 1 + sh_index], "%s,0x%lx,%ld,%ld",
            sh->sh_name, sh->sh_addr, sh->sh_offset, sh->sh_size);

    //update the index
    sh_index++;
    section_offset += sh->sh_size;
  }

  // .stmtab
  //get the pointer
  assert(sh_index < dst->sht_count);
  sh = &(dst->sht[sh_index]);

  //write the fields
  strcpy(sh->sh_name, ".symtab");
  sh->sh_addr = symtab_runtime_addr;
  sh->sh_offset = section_offset;
  sh->sh_size = *smap_count;

  //write into buffer
  sprintf(dst->buffer[1 + 1 + sh_index], "%s,0x%lx,%ld,%ld",
          sh->sh_name, sh->sh_addr, sh->sh_offset, sh->sh_size);

  assert(sh_index + 1 == dst->sht_count);
  //print sht
  if ((DEBUG_VERBOSE_SET & DEBUG_LINKER) != 0)
  {
    printf("-----------------------------------\n");
    printf("Destination ELF's SHT in BUFFER:\n");
    for (int i = 0; i < 2 + dst->sht_count; i++)
    {
      printf("%s\n", dst->buffer[i]);
    }
  }
}

static void merge_section(elf_t **srcs, int num_srcs, elf_t *dst, smap_t *smap_table, int *smap_count)
{
  int line_written = 1 + 1 + dst->sht_count;
  int symt_written = 0;
  int sym_section_offset = 0;

  for (int section_index = 0; section_index < dst->sht_count; section_index++)
  {
    //get the section by section id
    sh_entry_t *target_sh = &dst->sht[section_index];
    target_sh->sh_addr = 0;
    sym_section_offset = 0;

    //merge the sections
    //scan every input ELF file
    for (int i = 0; i < num_srcs; i++)
    { //for each src file
      int src_section_index = -1;
      //scan every section in this file
      for (int j = 0; j < srcs[i]->sht_count; j++)
      {
        //check if this ELF srcs[i] contains the same section as target_sh
        if (strcmp(target_sh->sh_name, srcs[i]->sht[j].sh_name) == 0)
        {
          //have found the same section name
          src_section_index = j;
          break;
        }
      }
      //check if we have found this target section from src ELF
      if (src_section_index == -1)
      {
        //the current ELF srcs[i] does not contain the target_sh
        //search for the next ELF
        continue;
      }
      else
      {
        //found
        for (int j = 0; j < srcs[i]->symt_count; j++)
        {
          st_entry_t *sym = &(srcs[i]->symt[j]);
          if (strcmp(target_sh->sh_name, sym->st_shndex) != 0)
          {
            continue;
          }
          for (int k = 0; k < *smap_count; k++)
          {
            //scan the cached dst symbols to check
            if (sym == smap_table[k].src)
            {
              //this symbol should be merged into section target_sh

              //copy this symbol from srcs[i] into dst.buffer
              for (int t = 0; t < sym->st_size; ++t)
              {
                //copy buffer
                int dst_index = line_written + t;
                //specific symbol
                int src_index = srcs[i]->sht[src_section_index].sh_offset //find target_sh->symbol  .text .data
                                + sym->st_value                           //find target_object/func/data
                                + t;                                      //line

                assert(dst_index < MAX_ELF_FILE_LENGTH);
                assert(src_index < MAX_ELF_FILE_LENGTH);
                strcpy(dst->buffer[dst_index], srcs[i]->buffer[src_index]);
              }

              assert(symt_written < dst->symt_count);
              //copy the entry
              strcpy(dst->symt[symt_written].st_name, sym->st_name);
              strcpy(dst->symt[symt_written].st_shndex, sym->st_shndex);
              dst->symt[symt_written].st_value = sym_section_offset;
              dst->symt[symt_written].st_size = sym->st_size;
              dst->symt[symt_written].bind = sym->bind;
              dst->symt[symt_written].type = sym->type;

              st_entry_t *write_sym = &(dst->symt[symt_written]);
              sprintf(dst->buffer[dst->line_count - dst->symt_count + symt_written], "%s,%s,%s,%s,%ld,%ld",
                      write_sym->st_name,
                      get_stb_string(write_sym->bind),
                      get_stt_string(write_sym->type),
                      write_sym->st_shndex,
                      write_sym->st_value,
                      write_sym->st_size);

              //update the smap_table
              //help relocation
              smap_table[k].dst = &dst->symt[symt_written];

              //update the counter
              symt_written += 1;
              line_written += sym->st_size;
              sym_section_offset += sym->st_size;
            }
          }
        }
      }
    }
  }
}

static void relocation_processing(elf_t **srcs, int num_srcs, elf_t *dst, smap_t *smap_table, int *smap_count)
{

  sh_entry_t *eof_text_sh = NULL;
  sh_entry_t *eof_data_sh = NULL;

  for (int i = 0; i < dst->sht_count; i++)
  {
    if (strcmp(dst->sht[i].sh_name, ".text") == 0)
    {
      eof_text_sh = &(dst->sht[i]);
    }
    if (strcmp(dst->sht[i].sh_name, ".data") == 0)
    {
      eof_data_sh = &(dst->sht[i]);
    }
  }

  for (int i = 0; i < num_srcs; i++)
  {
    elf_t *elf = srcs[i];
    /*
    //find section hearder  .text   .data
    sh_entry_t *text_sh = NULL;
    sh_entry_t *data_sh = NULL;
    for(int j = 0;j<elf->sht_count;j++){
       if(strcmp(elf->sht[j].sh_name, ".text") == 0){
         text_sh = &(elf->sht[j]);
       }else  if(strcmp(elf->sht[j].sh_name, ".data") == 0){
         data_sh = &(elf->sht[j]);
       }
    }
*/
    //.rel.text
    for (int j = 0; j < elf->reltext_count; j++)
    {
      rl_entry_t *r = &elf->reltext[j];

      //search the referencing symbol
      for (int k = 0; k < elf->symt_count; k++)
      {
        st_entry_t *sym = &elf->symt[k];

        if (strcmp(sym->st_shndex, ".text") == 0)
        {
          //must be referenced by a .text symbol
          //check if this symbol is the one referencing

          int sym_text_start = sym->st_value;
          int sym_text_end = sym->st_value + sym->st_size;

          if (sym_text_start <= r->r_row && r->r_row <= sym_text_end)
          {
            //symt[k] is referencing reltext[j].sym
            //search the smap table to find the EOF location
            int smap_found = 0;
            for (int t = 0; t < *smap_count; t++)
            {
              if (smap_table[t].src == sym)
              {
                smap_found = 1;
                //update the new referencing position in EOF
                st_entry_t *eof_referencing = smap_table[t].dst;
                //search the being referenced symbol
                for (int u = 0; u < *smap_count; u++)
                {
                  //TODO EOF symbal name/
                  //get the referencd symbol name

                  if (
                      strcmp(elf->symt[r->sym].st_name, smap_table[u].dst->st_name) == 0 &&
                      (smap_table[u].dst->bind == STB_GLOBAL))
                  {
                    st_entry_t *eof_referenced = smap_table[u].dst;
                    (handler_table[(int)r->type])(
                        dst, eof_text_sh,
                        r->r_row - sym->st_value + eof_referencing->st_value,
                        r->r_col,
                        r->r_addend,
                        eof_referenced);
                  }
                }
              }
            }
            //referencing must be in smap_table
            assert(smap_found == 1);
          }
        }
      }
    } //end for .rel.text

    //.rel.data
    for (int j = 0; j < elf->reldata_count; j++)
    {
      rl_entry_t *r = &elf->reldata[j];

      //search the referencing symbol
      for (int k = 0; k < elf->symt_count; k++)
      {
        st_entry_t *sym = &elf->symt[k];

        if (strcmp(sym->st_shndex, ".data") == 0)
        {
          //must be referenced by a .data symbol
          //check if this symbol is the one referencing

          int sym_data_start = sym->st_value;
          int sym_data_end = sym->st_value + sym->st_size;

          if (sym_data_start <= r->r_row && r->r_row <= sym_data_end)
          {
            //symt[k] is referencing reldata[j].sym
            //search the smap table to find the EOF location
            int smap_found = 0;
            for (int t = 0; t < *smap_count; t++)
            {
              if (smap_table[t].src == sym)
              {
                smap_found = 1;
                //update the new referencing position in EOF
                st_entry_t *eof_referencing = smap_table[t].dst;
                //search the being referenced symbol
                for (int u = 0; u < *smap_count; u++)
                {
                  //TODO EOF symbal name/
                  //get the referencd symbol name

                  if (
                      strcmp(elf->symt[r->sym].st_name, smap_table[u].dst->st_name) == 0 &&
                      (smap_table[u].dst->bind == STB_GLOBAL))
                  {
                    st_entry_t *eof_referenced = smap_table[u].dst;
                    (handler_table[(int)r->type])(
                        dst, eof_data_sh,
                        r->r_row - sym->st_value + eof_referencing->st_value,
                        r->r_col,
                        r->r_addend,
                        eof_referenced);
                  }
                }
              }
            }
            //referencing must be in smap_table
            assert(smap_found == 1);
          }
        }
      }
    }
  }

  return;
}
static uint64_t get_symbol_runtime_address(elf_t *dst, st_entry_t *sym)
{

  uint64_t text_base = 0x00400000;
  uint64_t rodata_base = 0;
  uint64_t data_base = 0;
  int inst_size = sizeof(inst_t);
  int data_size = sizeof(uint64_t);

  //must visit in .text .rodata .data order
  sh_entry_t *sht = dst->sht;
  for (int i = 0; i < dst->sht_count; i++)
  {
    if (strcmp(sht[i].sh_name, ".text") == 0)
    {

      rodata_base = text_base + sht[i].sh_size * inst_size;
      data_base = rodata_base;
    }
    else if (strcmp(sht[i].sh_name, ".rodata") == 0)
    {
      data_base = rodata_base + sht[i].sh_size * data_size;
    }
  }

  //check this symbol's section
  if (strcmp(sym->st_shndex, ".text") == 0)
  {
    return text_base + inst_size * sym->st_value;
  }
  else if (strcmp(sym->st_shndex, ".rodata") == 0)
  {
    return rodata_base + sym->st_value * data_size;
  }
  else if (strcmp(sym->st_shndex, ".data") == 0)
  {
    return data_base + sym->st_value * data_size;
  }

  return 0xffffffffffffffff;
}

static void write_relocation(char *dst, uint64_t val)
{
  char temp[20];

  //sprintf will add '\0' at end
  sprintf(temp, "0x%016lx", val);

  for (int i = 0; i < 18; i++)
  {
    dst[i] = temp[i];
  }
}

static void R_X86_64_32_handler(elf_t *dst, sh_entry_t *sh, int row_referencing, int col_referencing,
                                int addend, st_entry_t *sym_referenced)
{

  // printf("row = %d col = %d, sym referenced = %s\n", row_referencing, col_referencing, sym_referenced->st_name);
  uint64_t sym_address = get_symbol_runtime_address(dst, sym_referenced);
  char *s = &dst->buffer[sh->sh_offset + row_referencing][col_referencing];

  write_relocation(s, sym_address);
}

static void R_X86_64_PC32_handler(elf_t *dst, sh_entry_t *sh, int row_referencing, int col_referencing,
                                  int addend, st_entry_t *sym_referenced)
{
  assert(strcmp(sh->sh_name, ".text") == 0);

  // printf("row = %d col = %d, sym referenced = %s\n", row_referencing, col_referencing, sym_referenced->st_name);
  uint64_t sym_address = get_symbol_runtime_address(dst, sym_referenced);
  uint64_t rip_value = 0x000400000 + (row_referencing + 1) * sizeof(inst_t);
  char *s = &dst->buffer[sh->sh_offset + row_referencing][col_referencing];

  write_relocation(s, sym_address - rip_value);
}

static const char *get_stb_string(st_bind_t bind)
{
  char *res = NULL;
  if (bind == STB_LOACL)
  {
    res = "STB_LOACL";
  }
  else if (bind == STB_GLOBAL)
  {
    res = "STB_GLOBAL";
  }
  else if (bind == STB_WEAK)
  {
    res = "STB_WEAK";
  }
  else
  {
    printf("not exist this bind %d", bind);
    exit(1);
  }
  return res;
}

static const char *get_stt_string(st_type_t type)
{
  char *res = NULL;
  if (type == STT_NOTYPE)
  {
    res = "STT_NOTYPE";
  }
  else if (type == STT_OBJECT)
  {
    res = "STT_OBJECT";
  }
  else if (type == STT_FUNC)
  {
    res = "STT_FUNC";
  }
  else
  {
    printf("not exist this type %d", type);
    exit(1);
  }
  return res;
}
