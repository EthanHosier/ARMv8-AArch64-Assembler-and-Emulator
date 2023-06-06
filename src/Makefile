CC      ?= gcc
CFLAGS  ?= -std=c99 -g\
	-D_POSIX_SOURCE -D_DEFAULT_SOURCE\
	-Wall -Werror -pedantic-errors -Wstrict-prototypes

.SUFFIXES: .c .o

.PHONY: all clean

all: assemble emulate

assemble: assemble.o
emulate: emulate.o

clean:
	$(RM) *.o assemble emulate
	
