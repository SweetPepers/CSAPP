#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "headers/linker.h"
#include "headers/common.h"

static int parse_table_entry(char *str, char ***ent)
{
  //ent(char***)->stack(char**)->heap(char*)->(str)
  // .csv file
  // a1,a2,a3,a4
  // comma is 1 less than colomns

  //parse line as table entries
  int count_col = 1;
  int len = strlen(str);

  //count columns
  for (int i = 0; i < len; ++i)
  {
    if (str[i] == ',')
    {
      count_col++;
    }
  }

  //malloc and create list
  char **arr = malloc(count_col * sizeof(char *));
  *ent = arr;

  int col_index = 0;
  int col_width = 0;
  char col_buf[32];
  for (int i = 0; i < len + 1; i++)
  {
    if (str[i] == ',' || str[i] == '\0')
    {
      assert(col_index < count_col);

      // malloc and copy
      char *col = malloc((col_width + 1) * sizeof(char)); //+1 for '\0'
      for (int j = 0; j < col_width; j++)
      {
        col[j] = col_buf[j];
      }
      col[col_width] = '\0';

      //update
      arr[col_index] = col;
      col_index++;
      col_width = 0;
    }
    else
    {
      assert(col_width < 32);
      col_buf[col_width] = str[i];
      col_width++;
    }

  } // end for
  return count_col;
}

static void free_table_entry(char **ent, int n)
{
  for (int i = 0; i < n; i++)
  {
    free(ent[i]);
  }
  free(ent);
}

static void parse_sh(char *str, sh_entry_t *sh)
{
  //.text,0x0,4,22

  char **cols;
  int num_cols = parse_table_entry(str, &cols);
  assert(num_cols == 4);

  strcpy(sh->sh_name, cols[0]);
  sh->sh_addr = string2uint(cols[1]);
  sh->sh_offset = string2uint(cols[2]);
  sh->sh_size = string2uint(cols[3]);

  free_table_entry(cols, num_cols);
}

static void print_sh_entry(sh_entry_t *sh)
{
  debug_printf(DEBUG_LINKER, "%s\t%x\t%d\t%d\n",
               sh->sh_name,
               sh->sh_addr,
               sh->sh_offset,
               sh->sh_size);
}

static void parse_symtab(char *str, st_entry_t *ste)
{
  //st_name,bind       ,type   ,st_shndex,st_value,st_size
  // sum   ,STB_GLOBAL,STT_FUNC,.text    ,0        ,22
  char **cols;
  int num_cols = parse_table_entry(str, &cols);
  assert(num_cols == 6);
  assert(ste != NULL);

  //bind
  strcpy(ste->st_name, cols[0]);
  if (strcmp(cols[1], "STB_LOCAL") == 0)
  {
    ste->bind = STB_LOACL;
  }
  else if (strcmp(cols[1], "STB_GLOBAL") == 0)
  {
    ste->bind = STB_GLOBAL;
  }
  else if (strcmp(cols[1], "STB_WEAK") == 0)
  {
    ste->bind = STB_WEAK;
  }
  else
  {
    printf("symbol bind is neither LOCAL, GOLBAL, nor WEAK");
    exit(1);
  }
  //type
  if (strcmp(cols[2], "STT_NOTYPE") == 0)
  {
    ste->type = STT_NOTYPE;
  }
  else if (strcmp(cols[2], "STT_OBJECT") == 0)
  {
    ste->type = STT_OBJECT;
  }
  else if (strcmp(cols[2], "STT_FUNC") == 0)
  {
    ste->type = STT_FUNC;
  }
  else
  {
    printf("symbol type is neither NOTYPE, OBJECT, nor FUNC");
    exit(1);
  }
  //sh_shndex
  strcpy(ste->st_shndex, cols[3]);

  ste->st_value = string2uint(cols[4]);
  ste->st_size = string2uint(cols[5]);

  free_table_entry(cols, num_cols);
}

static void print_symtab_entry(st_entry_t *ste)
{
  debug_printf(DEBUG_LINKER, "%s\t%d\t%d\t%s\t%x\t%x\n",
               ste->st_name,
               ste->bind,
               ste->type,
               ste->st_shndex,
               ste->st_value,
               ste->st_size);
}

static int read_elf(const char *filename, uint64_t bufaddr)
{
  FILE *fp;
  fp = fopen(filename, "r");
  if (fp == NULL)
  {
    debug_printf(DEBUG_LINKER, "unable to open file %s\n", filename);
    exit(1);
  }

  //read text file line by line

  char line[MAX_ELF_FILE_WIDTH];
  int line_counter = 0;
  while (fgets(line, MAX_ELF_FILE_WIDTH, fp) != NULL)
  {
    int len = strlen(line);
    if ((len == 0) ||
        (len >= 1 && (line[0] == '\n' || line[0] == '\r' || line[0] == '\t')) ||
        (len >= 2 && line[0] == '/' && line[1] == '/'))
    {
      continue;
    }

    //check if is empty line
    int iswhite = 0;
    for (int i = 1; i < len; i++)
    {
      iswhite = iswhite && (line[i] == ' ' || line[i] == '\t' || line[i] == '\r');
    }
    if (iswhite == 1)
    {
      continue;
    }

    // the line is not empty and contains information
    if (line_counter < MAX_ELF_FILE_LENGTH)
    {

      uint64_t addr = bufaddr + line_counter * MAX_ELF_FILE_WIDTH * sizeof(char);
      char *linebuf = (char *)addr;
      int i = 0;
      while (i < len && i < MAX_ELF_FILE_WIDTH)
      {

        if (line[i] == '\n' ||
            line[i] == '\r' ||
            (i + 1 < len && i + 1 < MAX_ELF_FILE_WIDTH && line[i] == '/' && line[i + 1] == '/'))
        {
          break;
        }
        linebuf[i] = line[i];
        i++;
      }
      linebuf[i + 1] = '\0';
      // printf("%s",linebuf);

      line_counter++;
      // printf("%s",line);
    }
    else
    {
      debug_printf(DEBUG_LINKER, "elf file %s is too long (>%d)\n", filename, MAX_ELF_FILE_LENGTH);
      fclose(fp);
      exit(1);
    }
  }
  fclose(fp);

  assert(string2uint((char *)bufaddr) == line_counter);
  return line_counter;
}

static void parse_relocation(char *str, rl_entry_t *rte)
{
  // 4,7,R_X86_64_PC32,0,-4
  char **cols;
  int num_cols = parse_table_entry(str, &cols);
  assert(num_cols == 5);
  assert(rte != NULL);

  //row, clo
  rte->r_row = string2uint(cols[0]);
  rte->r_col = string2uint(cols[1]);

  //type
  if (strcmp(cols[2], "R_X86_64_32") == 0)
  {
    rte->type = R_X86_64_32;
  }
  else if (strcmp(cols[2], "R_X86_64_PC32") == 0)
  {
    rte->type = R_X86_64_PC32;
  }
  else if (strcmp(cols[2], "R_X86_64_PLT32") == 0)
  {
    rte->type = R_X86_64_PLT32;
  }
  else
  {
    printf("relocation type is neither R_X86_64_32, R_X86_64_PC32 nor R_X86_64_PLT32\n");
    exit(1);
  }

  //sym
  rte->sym = string2uint(cols[3]);
  //r_addend   int64_t
  uint64_t bitmap = string2uint(cols[4]);
  rte->r_addend = *(int64_t *)&bitmap;

  free_table_entry(cols, num_cols);
}

static void printf_relocation_entry(rl_entry_t *rte)
{
  debug_printf(DEBUG_LINKER, "%ld\t%ld\t%d\t%x\t%d\n",
               rte->r_row,
               rte->r_col,
               rte->type,
               rte->sym,
               rte->r_addend);
}

void parse_elf(char *filename, elf_t *elf)
{
  assert(elf != NULL);
  sh_entry_t *rtext_sh = NULL;
  sh_entry_t *rdata_sh = NULL;

  int line_count = read_elf(filename, (uint64_t) & (elf->buffer));
  for (int i = 0; i < line_count; i++)
  {
    printf("[%d]\t%s\n", i, elf->buffer[i]);
  }

  //parse section headers
  int sh_count = string2uint(elf->buffer[1]);
  elf->sht = malloc(sh_count * sizeof(sh_entry_t));
  sh_entry_t *symtab_sh = NULL;

  for (int i = 0; i < sh_count; i++)
  {
    parse_sh(elf->buffer[2 + i], &(elf->sht[i]));
    print_sh_entry(&(elf->sht[i]));

    if (strcmp(elf->sht[i].sh_name, ".symtab") == 0)
    {
      //.symtab,0x0,26,2
      // this is the section header for symbol table
      symtab_sh = &(elf->sht[i]);
    }
    else if (strcmp(elf->sht[i].sh_name, ".rel.text") == 0)
    {
      //
      rtext_sh = &(elf->sht[i]);
    }
    else if (strcmp(elf->sht[i].sh_name, ".rel.data") == 0)
    {
      rdata_sh = &(elf->sht[i]);
    }
  }

  assert(symtab_sh != NULL);

  //parse symtab
  elf->sht_count = string2uint(elf->buffer[1]);
  elf->symt_count = symtab_sh->sh_size;
  elf->symt = malloc(elf->symt_count * sizeof(st_entry_t));
  for (int j = 0; j < symtab_sh->sh_size; j++)
  {
    // int i = j+symtab_sh->sh_offset;  //26+0
    parse_symtab(elf->buffer[j + symtab_sh->sh_offset], &(elf->symt[j]));
    print_symtab_entry(&(elf->symt[j]));
  }

  //parse relocation table
  //.rel.text
  if (rtext_sh != NULL)
  {
    elf->reltext_count = rtext_sh->sh_size;
    elf->reltext = malloc(elf->reltext_count * sizeof(rl_entry_t));
    for (int i = 0; i < rtext_sh->sh_size; i++)
    {
      parse_relocation(
          elf->buffer[i + rtext_sh->sh_offset],
          &(elf->reltext[i]));
      int st = elf->reltext[i].sym;
      assert(0 <= st && st < elf->symt_count);
      printf_relocation_entry(&(elf->reltext[i]));
    }
  }
  else
  {
    elf->reltext_count = 0;
    elf->reltext = NULL;
  }

  //.rel.data
  if (rdata_sh != NULL)
  {
    elf->reldata_count = rdata_sh->sh_size;
    elf->reldata = malloc(elf->reldata_count * sizeof(rl_entry_t));
    for (int i = 0; i < rdata_sh->sh_size; i++)
    {
      parse_relocation(
          elf->buffer[i + rdata_sh->sh_offset],
          &(elf->reldata[i]));
      int st = elf->reldata[i].sym;
      assert(0 <= st && st < elf->symt_count);
      printf_relocation_entry(&(elf->reldata[i]));
    }
  }
  else
  {
    elf->reldata_count = 0;
    elf->reldata = NULL;
  }
}

void free_elf(elf_t *elf)
{
  assert(elf != NULL);
  free(elf->sht);
}

void write_eof(const char *filename, elf_t *eof)
{
  FILE *fp;
  fp = fopen(filename, "w");
  if (fp == NULL)
  {
    debug_printf(DEBUG_LINKER, "unable to open file %s\n", filename);
    exit(1);
  }

  for (int i = 0; i < eof->line_count; i++)
  {
    fprintf(fp, "%s\n", eof->buffer[i]);
  }
  fclose(fp);

  //free hash table
  //hashtable_free(link_constan_disk);
}