#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "headers/linker.h"
#include "headers/common.h"



int read_elf(const char *filename, uint64_t bufaddr);
void free_elf(elf_t *elf);

int main(){

  elf_t elf;
  parse_elf("./files/sum.elf.txt", &elf);
  free_elf(&elf);
  return 0;
}