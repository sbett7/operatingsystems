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

#define STRING_SIZE 150

#define MAX_CLIENTS 10

// struct used to hold a word for the hangman game.
struct word{
	int id;
	char *firstWord;
	int firstLength;
	char *lastWord;
	int lastLength;
	int maxGuess;

} word;

typedef struct word Word;

// struct used to hold the authorised user credentials
struct _account{
	char *username;
	char *password;

} account;

typedef struct _account Account;

Account *accounts;
Word *words;

int numWords;
int numAccounts;


int getWordLength(char * word);

void copyWord(char *copy, char *original, int length);

void separateWords(char *delimiter, char *str, char *firstWord, char *secondWord);

int getTextFileLength(char *fileName);

void clearWords();

int readInWords();

int getMaxGuesses(int firstWord, int secondWord);

int getRandomWordId(int length);

int checkStringsEqual(char *stringOne, char *stringTwo, int lengthOne, int lengthTwo);

void storeCredentials();

int checkCredentials(char *username, char *password, int length);

void clearAccounts();




#endif