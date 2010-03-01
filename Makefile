##
# Makefile for unitlib
##

MSVC_COMPAT = -mms-bitfields -mno-cygwin

CC = gcc
CFLAGS = -O2 -std=c99 -Wall -Wextra

AR = ar
RANLIB = ranlib

TARGET = libunit.a

DLL = libunit.dll
IMPLIB = libunit.lib

LUADLL = unitlib.dll
LUA_VERSION = lua51
LUA_INCLUDE = "$(LUA_DEV)/include"
LUA_LIB     = "$(LUA_DEV)/lib"

HEADER = unitlib-config.h unitlib.h

SRCFILES = unitlib.c parser.c format.c
HDRFILES = unitlib.h intern.h unitlib-config.h

WIN_DLL_INSTALL = /c/Windows
WIN_LIB_INSTALL = /g/Programmieren/lib
WIN_HDR_INSTALL = /g/Programmieren/include

PREFIX = /usr
INSTALL_LIB = $(PREFIX)/lib
INSTALL_HDR = $(PREFIX)/include

OBJFILES = unitlib.o parser.o format.o

TESTPROG = _test.exe
SMASHPROG = _smash.exe

UNITTEST = ultest

.PHONY: test clean allclean

all: $(TARGET)

dll: $(DLL)

lua: $(LUADLL)

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
	
$(TARGET): $(OBJFILES)
	@$(AR) rc $(TARGET) $(OBJFILES)
	@$(RANLIB) $(TARGET)
	
$(DLL): $(SRCFILES) $(HDRFILES) Makefile
	@$(CC) $(CFLAGS) $(MSVC_COMPAT) -shared -o $(DLL) $(SRCFILES) -Wl,--output-def,libunit.def
	lib.exe /DEF:libunit.def /OUT:libunit.lib /MACHINE:X86
	
$(LUADLL): luabinding.o $(OBJFILES) $(HDRFILES)
	@$(CC) $(CFLAGS) -L$(LUA_LIB) -shared -o $(LUADLL) luabinding.o $(OBJFILES) -l$(LUA_VERSION)
	
unitlib.o: unitlib.c $(HDRFILES)
	@$(CC) $(CFLAGS) -o unitlib.o -c unitlib.c
	
parser.o: parser.c $(HDRFILES)
	@$(CC) $(CFLAGS) -o parser.o -c parser.c
	
format.o: format.c $(HDRFILES)
	@$(CC) $(CFLAGS) -o format.o -c format.c
	
luabinding.o: luabinding.c $(HDRFILES)
	@$(CC) $(CFLAGS) -I$(LUA_INCLUDE) -o luabinding.o -c luabinding.c
	
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
	@rm -f libunit.def
	@rm -f libunit.exp