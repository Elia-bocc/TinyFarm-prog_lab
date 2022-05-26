# definizione del compilatore e dei flag di compilazione
# che vengono usate dalle regole implicite
CC=gcc
CFLAGS=-g -Wall -O -std=c99
LDLIBS=-lm -lrt -pthread

#	eseguibili
EXECS=farm

all: $(EXECS)

farm: farm.o xerrori.o

# target che cancella eseguibili e file oggetto
clean:
	rm -f $(EXECS) *.o