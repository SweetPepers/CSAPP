GCC = C:/Program Files/mingw-w64/bin/gcc  #编译器路径
CC = gcc

CFLASS = -Wall -g -O2 -Werror -std=gnu99 -Wno-unused-function

EXE_HARDWARE = $(SRC_DIR)/bin/exe_hardware

SRC_DIR = ./src

#debug
COMMON =$(SRC_DIR)/common/print.c $(SRC_DIR)/common/convert.c


#hardware
CPU = $(SRC_DIR)/hardware/cpu/mmu.c $(SRC_DIR)/hardware/cpu/isa.c

MEMORY = $(SRC_DIR)/hardware/memory/dram.c


# test
TEST_HARDWARE = $(SRC_DIR)/test/test_hardware.c
TEST_EFL = $(SRC_DIR)/test/test_elf.c


.PHONY: hardware
hardware:
	$(CC) $(CFLASS) -I $(SRC_DIR) $(COMMON) $(CPU) $(MEMORY) $(DISK) $(MAIN_HARDWARE) -o $(EXE_HARDWARE)
	$(EXE_HARDWARE)


.PHONY: link
link:
	$(CC) $(CFLASS) -I $(SRC_DIR) $(COMMON) $(TEST_EFL) $(SRC_DIR)/linker/parseElf.c $(SRC_DIR)/linker/staticlink.c -o ./src/bin/test_elf
	./src/bin/test_elf


run:
	

clean:
	rm -f *.o *~ $(EXE_HARDWARE)