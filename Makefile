CC     = gcc
CFLAGS = -g3 -std=c99 -pedantic -Wall

all: lzw encode decode clean

lzw.o trie.o: code.h trie.h

lzw: lzw.o trie.o code.o
	${CC} ${CFLAGS} $^ -o $@

encode: lzw
	ln -f lzw encode

decode: encode
	ln -f encode decode

clean: 
	rm -f lzw.o lzw