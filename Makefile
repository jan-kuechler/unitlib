##
# Makefile for unitlib
##

CC = gcc
CFLAGS = -Wall -O2

AR = ar
RANLIB = ranlib

TARGET = libunit.a

SRCFILES = unitlib.c parser.c format.c
HDRFILES = unitlib.h intern.h

OBJFILES = unitlib.o parser.o format.o

TESTPROG = _test.exe
SMASHPROG = _smash.exe

.PHONY: test clean allclean

all: $(TARGET)

test: $(TESTPROG)
	@./$(TESTPROG)
	
smash: $(SMASHPROG)
	@./$(SMASHPROG)
	
$(TARGET): $(OBJFILES)
	@$(AR) rc $(TARGET) $(OBJFILES)
	@$(RANLIB) $(TARGET)
	
unitlib.o: unitlib.c $(HDRFILES)
	@$(CC) $(CFLAGS) -o unitlib.o -c unitlib.c
	
parser.o: parser.c $(HDRFILES)
	@$(CC) $(CFLAGS) -o parser.o -c parser.c
	
format.o: format.c $(HDRFILES)
	@$(CC) $(CFLAGS) -o format.o -c format.c
	
$(TESTPROG): $(TARGET) _test.c
	@$(CC) -o $(TESTPROG) -L. _test.c -lunit
	
$(SMASHPROG): $(TARGET) _test.c
	@$(CC) -o $(SMASHPROG) -L. -DSMASH _test.c -lunit
	
clean:
	@rm -f $(OBJFILES)
	@rm -f $(TESTPROG)
	
allclean: clean
	@rm -f $(TARGET)
