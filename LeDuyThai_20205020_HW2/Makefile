# Makefile for a simple C program

# Compiler
CC = gcc

# Compiler flags
# CFLAGS = -Wall

# Source files
SRC = LeDuyThai_20205020_HW2.c

# Executable name
EXE = resolver.exe

all: $(EXE)

$(EXE): $(SRC)
	$(CC) $(CFLAGS) -o $(EXE) $(SRC)

clean:
	rm -f $(EXE)

.PHONY: all clean
