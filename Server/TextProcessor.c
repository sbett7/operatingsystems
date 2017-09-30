#include "TextProcessor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int getTextFileLength(char *fileName){
	FILE *fp = fopen(fileName, "r");
	int lines = 0;
	char str[MAX_LINE];
	if(fp == NULL){
		return -1;
	}	
	
	while(fgets(str, MAX_LINE, fp) != NULL){
		lines++;
		
	}
	fclose(fp);
	return lines;
}

int getWordLength(char* word){
	for(int i = 0; i < MAX_LINE; i++){
		if(word[i] == '\0'){
			return i;
		}
	}
	return 0;
}

void copyWord(char *copy, char *original, int length){
	for(int i=0; i <= length;i++){
		copy[i] = original[i];
	}
}

void separateWords(char * delimiter, char *str, char *firstWord, char *secondWord){
	strcpy(firstWord, strtok(str, delimiter));
	strcpy(secondWord, strtok(NULL, "\n"));
}

int readInWords(){
	FILE *fp = fopen(HANGMAN_FILE, "r");
	char str[MAX_LINE];
	char *firstWord = malloc(MAX_LINE * sizeof(char));
	char *secondWord = malloc(MAX_LINE * sizeof(char));
	int i = 0;
	
	if(fp == NULL){
		return -1;
	}
	
	while(fgets(str, sizeof(str), fp) != NULL){
		
		separateWords(",", str, firstWord, secondWord);
		
		words[i].firstLength = getWordLength(firstWord);
		words[i].lastLength = getWordLength(secondWord);

		words[i].firstWord = malloc(words[i].firstLength * sizeof(char));
		words[i].lastWord = malloc(words[i].lastLength * sizeof(char));
		
		

		copyWord(words[i].firstWord, firstWord, words[i].firstLength);
		copyWord(words[i].lastWord, secondWord, words[i].lastLength);

		words[i].maxGuess = getMaxGuesses(words[i].firstLength, words[i].lastLength);
		i++;
		
	}
	free(firstWord);
	free(secondWord);
	fclose(fp);	
	return 1;
}

int getMaxGuesses(int firstWord, int secondWord){
	int length = firstWord + secondWord + MIN_GUESSES;	
	return min(length, MAX_GUESSES);
}

void clearWords(){
	for(int i = 0; i < (int)(sizeof(words)/sizeof(Word)); i++){
		free(words[i].firstWord);
		free(words[i].lastWord);
	}
	free(words);
}

int getRandomWordId(int length){
	return random() % length;
}

void resetLetterLocations(Client *client){
	int iterations = max(client->firstLength, client->lastLength);

	for(int i =0; i < iterations; i++){
		if (i < client->firstLength){
			client->firstWord[i] = FALSE;
		}
		
		if (i < client->lastLength){ 
			client->lastWord[i] = FALSE;
		}
	}
}

void getLetterLocations(Client *client, char letter){
	int iterations = max(client->firstLength, client->lastLength);
	
	for(int i = 0; i < iterations; i++){
		if (i < client->firstLength && 
			words[client->wordId].firstWord[i] == letter){
			client->firstWord[i] = TRUE;
		}
		
		if (i < client->lastLength && 
			words[client->wordId].lastWord[i] == letter){
			client->lastWord[i] = TRUE;
		}
	}
}

void initialiseClient(int socketId, Client *client, char *username){
	client->clientId = socketId;
	client->username = malloc(sizeof(username));
	strcpy(client->username, username);
	client->gamesPlayed = 0;
	client->gamesWon = 0;
}

void initialiseClientWords(Client *client, int length){
	client->wordId = getRandomWordId(length);
	client->firstLength = words[client->wordId].firstLength;
	client->lastLength = words[client->wordId].lastLength;
	client->firstWord = calloc(client->firstLength, sizeof(int));
	client->lastWord = calloc(client->lastLength, sizeof(int));
	client->maxGuess = words[client->wordId].maxGuess;
}

int readCredentials(char *username, char *userPassword){
	FILE *fp;
	char str[MAX_LINE];
	char *userTemp = malloc(MAX_LINE * sizeof(char));
	char *passTemp = malloc(MAX_LINE * sizeof(char));
	char *user;
	char *pass;
	int i = 0;
	int userLength = 0;
	int passLength = 0;
	int authenticator = FALSE;

	fp = fopen(ACCOUNTS_FILE, "r");
	if(fp == NULL){
		printf("Could not Authenticate file");
		return FALSE;
	}
	while(fgets(str, MAX_LINE, fp) != NULL && authenticator == FALSE){
		if(i > 0){
			separateWords(" \r\t\n", str, userTemp, passTemp);
			if(passTemp[0] == '\t');{
				strcpy(passTemp, strtok(passTemp, "\t"));
			}
			userLength = getWordLength(userTemp);
			passLength = getWordLength(passTemp);
			user = malloc(userLength * sizeof(char));
			pass = malloc(passLength * sizeof(char));

			copyWord(user, userTemp, userLength);
			copyWord(pass, passTemp, passLength);
			
			if(!(strcmp(username, user) && strcmp(userPassword, pass))){
				authenticator = TRUE;
			}

			free(user);
			free(pass);
		}
		i++;
	}
	fclose(fp);
	return authenticator;
}
