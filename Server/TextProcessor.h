#ifndef __TEXT_PROCESSOR_H__
#define __TEXT_PROCESSOR_H__

#define HANGMAN_FILE "hangman_text.txt"
#define ACCOUNTS_FILE "Authentication.txt"
#define MAX_LINE 100

#define TRUE 1
#define FALSE 0

#define MAX_GUESSES 26
#define MIN_GUESSES 10

#define max(a,b) (a>b?a:b)
#define min(a,b) (a<b?a:b)

#define MAX_CLIENTS 20

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

}clients[MAX_CLIENTS], temp;

typedef struct _client Client;

Word *words;


int getWordLength(char * word);

void copyWord(char *copy, char *original, int length);

void separateWords(char *delimiter, char *str, char *firstWord, char *secondWord);

int getTextFileLength(char *fileName);

void clearWords();

int readInWords();

int getMaxGuesses(int firstWord, int secondWord);

void clearWords();

int getRandomWordId(int length);

void resetLetterLocations(Client *client);
void getLetterLocations(Client *client, char letter);
void initialiseClient(int socketId, Client *client, char *username);
void initialiseClientWords(Client *client, int length);
int readCredentials(char *username, char *userPassword);

#endif