CC = c99 
CFLAGS = -Wall # Show all reasonable warnings
LDFLAGS = -pthread

all: HangmanServer

HangmanServer: HangmanServer.o TextProcessor.o ClientLeaderboardFunctions.o Communication.o

ClientLeaderboardFunctions.o: ClientLeaderboardFunctions.c

Communication.o: Communication.c

TextProcessor.o: TextProcessor.c

HangmanServer.o: HangmanServer.c





clean:
	rm -f HangmanServer *.o 
 
.PHONY: clean
