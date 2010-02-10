##
# Makefile for unitlib
##

CC = gcc
CFLAGS = -Wall

AR = ar
RANLIB = ranlib

TARGET = libunit.a

SRCFILES = unitlib.c parser.c format.c
HDRFILES = unitlib.h intern.h

OBJFILES = unitlib.o parser.o format.o

EXE = .exe

.PHONY: test

all: $(TARGET)

test: _test$(EXE)
	@./_test$(EXE)
	
$(TARGET): $(OBJFILES)
	@$(AR) rc $(TARGET) $(OBJFILES)
	@$(RANLIB) $(TARGET)
	
unitlib.o: unitlib.c $(HDRFILES)
	@$(CC) $(CFLAGS) -o unitlib.o -c unitlib.c
	
parser.o: parser.c $(HDRFILES)
	@$(CC) $(CFLAGS) -o parser.o -c parser.c
	
format.o: format.c $(HDRFILES)
	@$(CC) $(CFLAGS) -o format.o -c format.c
	
_test$(EXE): $(TARGET) _test.c
	@$(CC) -o_test.exe -L. _test.c -lunit
	
clean:
	@rm -f $(OBJFILES)
	
allclean: clean
	@rm -f $(TARGET)
