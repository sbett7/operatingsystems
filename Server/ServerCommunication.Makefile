CC = c99 
CFLAGS = -Wall # Show all reasonable warnings
LDFLAGS = -pthread

all: ServerCommunication

ServerCommunication: ServerCommunication.o TextProcessor.o

ServerCommunication.o: ServerCommunication.c

TextProcessor.o: TextProcessor.c

clean:
	rm -f ServerCommunication *.o 
 
.PHONY: clean
