GCC = C:/Program Files/mingw-w64/bin/gcc  #编译器路径

CFLASS = -Wall -g -O2 -Werror -std=gnu99

EXE = csapp

SRC = ./src

CODE = ./src/memory/instruction.c ./src/disk/code.c ./src/memory/dram.c ./src/cpu/mmu.c ./src/main.c

.PHONY: csapp
csapp:
	$(GCC) $(CFLASS) -I $(SRC) $(CODE) -o $(EXE)
	./$(EXE)
run:
	

clean:
	rm -f *.o *~ $(EXE)