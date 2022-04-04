#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "headers/linker.h"
#include "headers/common.h"



int read_elf(const char *filename, uint64_t bufaddr);
void free_elf(elf_t *elf);

int main(){

  elf_t src[2];
  parse_elf("./files/sum.elf.txt", &src[0]);
  printf("====================================\n");
  parse_elf("./files/main.elf.txt", &src[1]);
  elf_t dst;
  elf_t *srcp[2];
  srcp[0] = &src[0];
  srcp[1] = &src[1];
  link_elf((elf_t **)&srcp, 2, &dst);
  write_eof("./files/exe/output.eof.txt", &dst);
  free_elf(&src[0]);
  free_elf(&src[1]);
  return 0;
}