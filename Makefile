##
# Makefile for unitlib
##

CC = gcc
CFLAGS = -O2 -std=c99 -Wall -Wextra

AR = ar
RANLIB = ranlib

TARGET = libunit.a

DLL = libunit.dll
IMPLIB = libunit.lib

HEADER = unitlib.h

SRCFILES = unitlib.c parser.c format.c
HDRFILES = unitlib.h intern.h config.h

LIB_INSTALL = /g/Programmieren/lib
HDR_INSTALL = /g/Programmieren/include

OBJFILES = unitlib.o parser.o format.o

TESTPROG = _test.exe
SMASHPROG = _smash.exe

UNITTEST = ultest

.PHONY: test clean allclean

all: $(TARGET)

dll: $(DLL)

install-dll: dll
	cp $(DLL) $(LIB_INSTALL)
	cp $(IMPLIB) $(LIB_INSTALL)
	cp $(HEADER) $(HDR_INSTALL)

test: $(TESTPROG)
	@./$(TESTPROG)
	
utest: $(UNITTEST)
	@./$(UNITTEST)
	
smash: $(SMASHPROG)
	@./$(SMASHPROG)
	
$(TARGET): $(OBJFILES)
	@$(AR) rc $(TARGET) $(OBJFILES)
	@$(RANLIB) $(TARGET)
	
$(DLL): $(SRCFILES) $(HDRFILES)
	@$(CC) $(CFLAGS) -shared -o $(DLL) $(SRCFILES) -Wl,--out-implib,$(IMPLIB)
	
unitlib.o: unitlib.c $(HDRFILES)
	@$(CC) $(CFLAGS) -o unitlib.o -c unitlib.c
	
parser.o: parser.c $(HDRFILES)
	@$(CC) $(CFLAGS) -o parser.o -c parser.c
	
format.o: format.c $(HDRFILES)
	@$(CC) $(CFLAGS) -o format.o -c format.c
	
$(TESTPROG): $(TARGET) _test.c
	@$(CC) -o $(TESTPROG) -g -L. _test.c -lunit
	
$(SMASHPROG): $(TARGET) _test.c
	@$(CC) -o $(SMASHPROG) -L. -DSMASH _test.c -lunit
	
$(UNITTEST): $(TARGET) unittest.c
	@$(CC) -std=gnu99 -o $(UNITTEST) -L. unittest.c -lunit
	
clean:
	@rm -f $(OBJFILES)
	@rm -f $(TESTPROG)
	@rm -f $(SMASHPROG)
	@rm -f $(UNITTEST)
	@rm -f debug.log
	
allclean: clean
	@rm -f $(TARGET)
	@rm -f $(DLL)
	@rm -f $(IMPLIB)
