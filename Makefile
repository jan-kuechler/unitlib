##
# Makefile for unitlib
##

MSVC_COMPAT = -mms-bitfields -mno-cygwin

BIN_DIR = bin
SRC_DIR = src
INC_DIR = include
TST_DIR = test

CC = gcc
CFLAGS = -O2 -std=c99 -Wall -Wextra -I$(INC_DIR)

AR = ar
RANLIB = ranlib

SRCFILES = $(SRC_DIR)/unitlib.c $(SRC_DIR)/parser.c $(SRC_DIR)/format.c
HDRFILES = $(INC_DIR)/unitlib.h $(SRC_DIR)/intern.h $(INC_DIR)/unitlib-config.h

TARGET = $(BIN_DIR)/libunit.a

DLL = $(BIN_DIR)/libunit.dll
IMPLIB = $(BIN_DIR)/libunit.lib

HEADER = $(INC_DIR)/unitlib-config.h $(INC_DIR)/unitlib.h

WIN_DLL_INSTALL = /c/Windows
WIN_LIB_INSTALL = /g/Programmieren/lib
WIN_HDR_INSTALL = /g/Programmieren/include

PREFIX = /usr
INSTALL_LIB = $(PREFIX)/lib
INSTALL_HDR = $(PREFIX)/include

OBJFILES = $(BIN_DIR)/unitlib.o $(BIN_DIR)/parser.o $(BIN_DIR)/format.o

TESTPROG = $(TST_DIR)/test.exe
SMASHPROG = $(TST_DIR)/smash.exe

UNITTEST = $(TST_DIR)/ultest

.PHONY: test clean allclean prepare

all: $(TARGET)

dll: $(DLL)

install-dll: dll
	cp $(DLL) $(WIN_DLL_INSTALL)
	cp $(DLL) $(WIN_LIB_INSTALL)
	cp $(IMPLIB) $(WIN_LIB_INSTALL)
	cp $(HEADER) $(WIN_HDR_INSTALL)

install:
	cp $(TARGET) $(INSTALL_LIB)
	cp $(HEADER) $(INSTALL_HDR)

uninstall:
	rm $(INSTALL_LIB)/$(TARGET)
	cd $(INSTALL_HDR)
	rm $(HEADER)

test: $(TESTPROG)
	@./$(TESTPROG)

utest: $(UNITTEST)
	@./$(UNITTEST)

smash: $(SMASHPROG)
	@./$(SMASHPROG)

$(TARGET): prepare $(OBJFILES)
	@$(AR) rc $(TARGET) $(OBJFILES)
	@$(RANLIB) $(TARGET)

$(DLL): prepare $(SRCFILES) $(HDRFILES) Makefile
	@$(CC) $(CFLAGS) $(MSVC_COMPAT) -shared -o $(DLL) $(SRCFILES) -Wl,--output-def,$(BIN_DIR)/libunit.def
	lib.exe /DEF:$(BIN_DIR)/libunit.def /OUT:$(BIN_DIR)/libunit.lib /MACHINE:X86

$(BIN_DIR)/unitlib.o: $(SRC_DIR)/unitlib.c $(HDRFILES)
	@$(CC) $(CFLAGS) -o $(BIN_DIR)/unitlib.o -c $(SRC_DIR)/unitlib.c

$(BIN_DIR)/parser.o: $(SRC_DIR)/parser.c $(HDRFILES)
	@$(CC) $(CFLAGS) -o $(BIN_DIR)/parser.o -c $(SRC_DIR)/parser.c

$(BIN_DIR)/format.o: $(SRC_DIR)/format.c $(HDRFILES)
	@$(CC) $(CFLAGS) -o $(BIN_DIR)/format.o -c $(SRC_DIR)/format.c

$(TESTPROG): $(TARGET) $(TST_DIR)/_test.c
	@$(CC) -o $(TESTPROG) -g -L. $(TST_DIR)/test.c -lunit

$(SMASHPROG): $(TARGET) $(TST_DIR)/_test.c
	@$(CC) -o $(SMASHPROG) -L. -DSMASH $(TST_DIR)/test.c -lunit

$(UNITTEST): $(TARGET) $(TST_DIR)/unittest.c
	@$(CC) -std=gnu99 -I$(INC_DIR) -o $(UNITTEST) -L$(BIN_DIR) $(TST_DIR)/unittest.c -lunit

prepare:
	@if [ ! -d $(BIN_DIR) ]; then mkdir $(BIN_DIR); fi

clean:
	@rm -f $(OBJFILES)
	@rm -f $(TESTPROG)
	@rm -f $(SMASHPROG)
	@rm -f $(UNITTEST)

allclean: clean
	@rm -f $(TARGET)
	@rm -f $(DLL)
	@rm -f $(IMPLIB)
	@rm -f libunit.def
	@rm -f libunit.exp
