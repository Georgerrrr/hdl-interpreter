CC = gcc
CFLAGS = -g -c

logic: main.o alloc.o tAssert.o str.o lexer.o parse.o chip.o
	$(CC) $^ -o hdl

clean: 
	rm -f hdl
	rm -f *.o
