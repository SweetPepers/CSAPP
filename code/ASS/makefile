CC = /usr/bin/gcc-9  #编译器路径
CFLASS = -Wall -g -02 -Werror -std=gnu99

EXE = program

SRC = ./src

CODE = ./src/memory/instruction.c ./src/disk/code.c ./src/memory/dram.c ./src/cpu/mmu.c ./src/main.c

.PHONY: program
main:
	$(CC) $(CFLASS) -I$(SRC) $(CODE) -o $(EXE)

run:
	./$(EXE)