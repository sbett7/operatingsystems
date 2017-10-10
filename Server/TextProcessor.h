#ifndef __TEXT_PROCESSOR_H__
#define __TEXT_PROCESSOR_H__
#define _GNU_SOURCE

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


#define HANGMAN_FILE "hangman_text.txt"
#define ACCOUNTS_FILE "Authentication.txt"
#define MAX_LINE 100

#define TRUE 1
#define FALSE 0

#define MAX_GUESSES 26
#define MIN_GUESSES 10

#define max(a,b) (a>b?a:b)
#define min(a,b) (a<b?a:b)

#define CLIENT_ONE 1
#define CLIENT_TWO 2
#define DRAW 3

#define STRING_SIZE 150

#define MAX_CLIENTS 10

#define _GNU_SOURCE
#define NO_PENDING_CONNECTIONS 0
#define READ_OCCURRING 1
#define NO_READING_OCCURING 0

struct word{
	int id;
	char *firstWord;
	int firstLength;
	char *lastWord;
	int lastLength;
	int maxGuess;

} word;

typedef struct word Word;

struct _client{
	int clientId;
	char *username;
	int wordId;
	int *firstWord;
	int firstLength;
	int *lastWord;
	int lastLength;
	int maxGuess;
	int gamesPlayed;
	int gamesWon;
	float percentage;

}_clients, temp;

typedef struct _client Client;

struct _account{
	char *username;
	char *password;

} account;

typedef struct _account Account;

Account *accounts;
Word *words;
Client *clients;
int numClients;


pthread_mutex_t readerCounterMutex;
pthread_mutex_t readerMutex;
pthread_mutex_t writerMutex;

int readerCounter;


int getWordLength(char * word);

void copyWord(char *copy, char *original, int length);

void separateWords(char *delimiter, char *str, char *firstWord, char *secondWord);

int getTextFileLength(char *fileName);

void clearWords();

int readInWords();

int getMaxGuesses(int firstWord, int secondWord);

void clearWords();

int getRandomWordId(int length);

int checkStringsEqual(char *stringOne, char *stringTwo, int lengthOne, int lengthTwo);

void resetLetterLocations(Client *client);
void getLetterLocations(Client *client, char letter);
void initialiseClient(Client *client, char *username);
void initialiseClientWords(Client *client, int length);
void storeCredentials();
int checkCredentials(char *username, char *password, int length);

void clearClients();
void getClientByUsername(char *username, Client client);
int addClient(char *username);
void sendClientLeaderboard(int socketId);
void getLeaderBoardClient(char *userString, int clientId);
void orderLeaderboard();
int compareClients(Client *clientOne, Client *clientTwo);
int compareClientGamesPercentage(Client *clientOne, Client *clientTwo);
int compareClientGamesWon(Client *clientOne, Client *clientTwo);
void updateLeaderboardWithClient(Client *client);
int getClientIndexByUsername(char *username);


/*
void addConnection(int socketId, pthread_mutex_t *pMutex, pthread_cond_t *pCondVar);
struct connection* getConnection(pthread_mutex_t *pMutex);
void handleConnection(int socketId);
void* threadConnectionHandler();
*/

void initialiseMutexConnections();

void leaderboardReadUnlock();
void leaderboardReadLock();
void leaderboardWriteLock();
void leaderboardWriteUnlock();
void initialiseLeaderboardMutex();







#endif