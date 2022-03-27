GCC = C:/Program Files/mingw-w64/bin/gcc  #编译器路径
CC = gcc

CFLASS = -Wall -g -O2 -Werror -std=gnu99 -Wno-unused-function

EXE_HARDWARE = exe_hardware

SRC_DIR = ./src

#debug
COMMON =$(SRC_DIR)/common/print.c $(SRC_DIR)/common/convert.c


#hardware
CPU = $(SRC_DIR)/hardware/cpu/mmu.c $(SRC_DIR)/hardware/cpu/isa.c

MEMORY = $(SRC_DIR)/hardware/memory/dram.c

MAIN_HARDWARE = $(SRC_DIR)/main_hardware.c
.PHONY: hardware
hardware:
	$(CC) $(CFLASS) -I $(SRC_DIR) $(COMMON) $(CPU) $(MEMORY) $(DISK) $(MAIN_HARDWARE) -o $(EXE_HARDWARE)
	./$(EXE_HARDWARE)
run:
	

clean:
	rm -f *.o *~ $(EXE_HARDWARE)