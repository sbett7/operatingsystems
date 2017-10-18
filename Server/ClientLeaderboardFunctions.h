#ifndef __CLIENT_LEADERBOARD_H__
#define __CLIENT_LEADERBOARD_H__

#include <arpa/inet.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <errno.h> 
#include <string.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <sys/wait.h> 
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#include "TextProcessor.h"

#define MAX_GUESSES 26
#define MIN_GUESSES 10

#define CLIENT_ONE 1
#define CLIENT_TWO 2
#define DRAW 3

#define MAX_CLIENTS 10
#define NO_GAMES_PLAYED 0

#define TRUE 1
#define FALSE 0

#define READ_OCCURRING 1
#define NO_READING_OCCURING 0

// the client struct that contains all of the required information for a client
struct _client{
	int clientId;
	char *username;
	int wordId;
	int *firstWord;
	int firstLength;
	int *lastWord;
	int lastLength;
	int maxGuess;
};

typedef struct _client Client;

// the leaderboard structure
struct _leaderboard{
	int clientId;
	char *username;
	int gamesPlayed;
	int gamesWon;
	float percentage;
} temp;

typedef struct _leaderboard Leaderboard;

// houses all clients, and acts as the leaderboard
Client *clients;
Leaderboard *leaderboard;

int numClients;
int readerCounter;

pthread_mutex_t readerCounterMutex;
pthread_mutex_t readerMutex;
pthread_mutex_t writerMutex;


void getClientByUsername(char *username, Client client);

int addClient(char *username);

void sendClientLeaderboard(int socketId);

void getLeaderBoardClient(char *userString, int clientId);

void orderLeaderboard();

int compareClients(Leaderboard *clientOne, Leaderboard *clientTwo);

int compareClientGamesPercentage(Leaderboard *clientOne, Leaderboard *clientTwo);

int compareClientGamesWon(Leaderboard *clientOne, Leaderboard *clientTwo);

void updateLeaderboard(int clientId, int gameWon);

int getClientIndexByUsername(char *username);

int getClientIndexById(int clientId);

int getNumberOfPlayersOnLeaderboard();

void leaderboardReadUnlock();

void leaderboardReadLock();

void leaderboardWriteLock();

void leaderboardWriteUnlock();

void initialiseLeaderboardMutex();

void clearClients();

void initialiseClient(Client *client, char *username);

void initialiseClientWords(Client *client, int length);

void resetLetterLocations(Client *client);

void getLetterLocations(Client *client, char letter);

void addClientToLeaderBoard(Client *client, int index);

int checkClientGamesPlayed(Leaderboard *clientOne, Leaderboard *clientTwo);

#endif