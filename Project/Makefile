    CC=gcc
CFLAGS=-g -Wall -pedantic
TXTFILES=log.txt i-banco-sim-*.txt

all: i-banco i-banco-terminal

i-banco-terminal: commandlinereader.o helpers.o i-banco-terminal.o
	$(CC) -pthread -o i-banco-terminal i-banco-terminal.o commandlinereader.o helpers.o

i-banco: contas.o commandlinereader.o helpers.o i-banco.o
	$(CC) -pthread -o i-banco i-banco.o contas.o commandlinereader.o helpers.o


i-banco.o: commandlinereader.c contas.c helpers.c i-banco.c	
	$(CC) $(CFLAGS) -c i-banco.c

i-banco-terminal.o: commandlinereader.c helpers.c i-banco-terminal.c
	$(CC) $(CFLAGS) -c i-banco-terminal.c

contas.o: contas.h contas.c
	$(CC) $(CFLAGS) -c contas.c

commandlinereader.o: commandlinereader.h commandlinereader.c
	$(CC) $(CFLAGS) -c commandlinereader.c
	
helpers.o: helpers.h helpers.c
	$(CC) $(CFLAGS) -c helpers.c -pthread
	

clean:
	$(RM) *.o i-banco
	$(RM) $(TXTFILES)
