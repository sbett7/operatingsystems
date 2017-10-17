CC = c99 
CFLAGS = -Wall # Show all reasonable warnings
LDFLAGS =

all: HangmanClient

HangmanClient: HangmanClient.o ClientCommunication.o

ClientCommunication.o: ClientCommunication.c

HangmanClient.o: HangmanClient.c





clean:
	rm -f HangmanClient *.o 
 
.PHONY: clean
