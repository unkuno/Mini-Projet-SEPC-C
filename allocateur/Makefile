
# Makefile utilisant les règles et variables implicites

CFLAGS=-Wall -g

all: memshell test

memshell: memshell.o mem.o

test:
	( cd tests_allocateur; make test )

clean:
	rm *.o memshell
	( cd tests_allocateur; make clean )
