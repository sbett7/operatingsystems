#define _GNU_SOURCE
#include "TextProcessor.h"

/*
void initialiseMutexConnections(){
	pthread_mutex_init(&connectionMutex, PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP);
	pthread_cond_init(&gotConnection, PTHREAD_COND_INITIALIZER);
}
*/

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

void initialiseClient(Client *client, char *username){
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

void storeCredentials(){
	FILE *fp;
	char str[MAX_LINE];
	char *userTemp = malloc(MAX_LINE * sizeof(char));
	char *passTemp = malloc(MAX_LINE * sizeof(char));
	int i = 0;
	int userLength = 0;
	int passLength = 0;

	fp = fopen(ACCOUNTS_FILE, "r");
	if(fp == NULL){
		printf("Could not Authenticate file");
	}
	while(fgets(str, MAX_LINE, fp) != NULL){
		if(i > 0){
			
			separateWords(" \r\t\n", str, userTemp, passTemp);
			if(passTemp[0] == '\t');{
				strcpy(passTemp, strtok(passTemp, "\t"));
			}
			userLength = getWordLength(userTemp);
			passLength = getWordLength(passTemp);
			accounts[i-1].username = malloc(userLength * sizeof(char));
			accounts[i-1].password = malloc(passLength * sizeof(char));

			copyWord(accounts[i-1].username, userTemp, userLength);
			copyWord(accounts[i-1].password, passTemp, passLength);
		}
		i++;
	}
	free(userTemp);
	free(passTemp);
	fclose(fp);
}

int checkCredentials(char *username, char *password, int length){
	int usernameLength = getWordLength(username);
	int passwordLength = getWordLength(password);

	int accountUserLength;
	int accountPassLength;
	
	for(int i = 0; i < length - 1; i++){
		accountUserLength = getWordLength(accounts[i].username);
		accountPassLength = getWordLength(accounts[i].password);

		if(checkStringsEqual(username, accounts[i].username, usernameLength, accountUserLength) &&
		    		checkStringsEqual(password, accounts[i].password, passwordLength, accountPassLength)){
			return TRUE;
		}
	}
	return FALSE;
}

int checkStringsEqual(char *stringOne, char *stringTwo, int lengthOne, int lengthTwo){
	printf("%d %d\n", lengthOne, lengthTwo);
	
	if(lengthOne != lengthTwo){
		return FALSE;
	}
	
	for (int i = 0; i < lengthOne; i++){
		if(stringOne[i] != stringTwo[i]){
			return FALSE;
		}
	}

	return TRUE;
}


int compareClientGamesWon(Client *clientOne, Client *clientTwo){
	if(clientOne->gamesWon > clientTwo->gamesWon){
		return CLIENT_TWO;
	} else if (clientOne->gamesWon < clientTwo->gamesWon){
		return CLIENT_ONE;
	} else {
		return DRAW;
	}
}

int compareClientGamesPercentage(Client *clientOne, Client *clientTwo){
	if(clientOne->percentage > clientTwo->percentage){
		return CLIENT_TWO;
	} else if (clientOne->percentage < clientTwo->percentage){
		return CLIENT_ONE;
	} else {
		return DRAW;
	}
}

int compareClientNames(Client *clientOne, Client *clientTwo){
	int compareResult = strcmp(clientOne->username, clientTwo->username);	
	if(compareResult > 0){
		return CLIENT_TWO;
	} else {
		return CLIENT_ONE;
	} 
}

int compareClients(Client *clientOne, Client *clientTwo){
	int compareResult = 0;

	compareResult= compareClientGamesWon(clientOne, clientTwo);
	if(compareResult == DRAW){
		compareResult = compareClientGamesPercentage(clientOne, clientTwo);
		if(compareResult == DRAW){
			compareResult = compareClientNames(clientOne, clientTwo);
		}
		
	}
	return compareResult;
}

void orderLeaderboard(){

	for (int i = 1; i < numClients; i++){
		if(clients[i].gamesPlayed == 0){
			continue;		
		}
		for (int j = 0; j < numClients - i; j++){
			if(clients[j].gamesPlayed == 0){
				continue;		
			}
			
			if(compareClients(&clients[j], &clients[j+1]) == CLIENT_TWO){
				temp = clients[j];
				clients[j] = clients[j+1];
				clients[j+1] = temp;
			}	
		}
		
	}
}

int getNumberOfPlayersOnLeaderboard(){
	int players = 0;
	for(int i = 0; i < numClients; i++){
		if(clients[i].gamesPlayed != NO_GAMES_PLAYED){
			players++;
		}
	}
	return players;
}

void sendClientLeaderboard(int socketId){
	uint16_t numClientsSent;
	uint16_t messageSizeSent;
	uint16_t clientValue;
	int numPlayersOnboard = getNumberOfPlayersOnLeaderboard();

	numClientsSent = htons(numPlayersOnboard);	
	send(socketId, &numClientsSent, sizeof(uint16_t), 0);
	printf("Num of Read Clients: %d\n", numPlayersOnboard);
	for(int i = 0; i < numClients; i++){
		if(clients[i].gamesPlayed != NO_GAMES_PLAYED){
			messageSizeSent = htons(strlen(clients[i].username));
			send(socketId, &messageSizeSent, sizeof(uint16_t), 0);
			send(socketId, clients[i].username, strlen(clients[i].username), 0);
			
			clientValue = htons(clients[i].gamesPlayed);
			send(socketId, &clientValue, sizeof(uint16_t), 0);
			clientValue = htons(clients[i].gamesWon);
			send(socketId, &clientValue, sizeof(uint16_t), 0);
		}
	}
}

int addClient(char *username){
	if(numClients > 0){
		numClients++;
		printf("number of clients that are listed are: %d\n", numClients);
		clients = realloc(clients, sizeof (Client) * (numClients));
		printf("Initialising Client %d \n", numClients);
		initialiseClient(&clients[numClients-1], username);
		return numClients - 1;
	} else {
		initialiseClient(&clients[0], username);
		return numClients++;
	}
}

void getClientByUsername(char *username, Client client){
	if(clients != NULL){
		int numClients = sizeof(clients)/sizeof(clients[0]);
		for(int i = 0; i < numClients; i++){
			if(!strcmp(clients[i].username,username)){
				client = clients[i];
				break;
			}
		}
	}
}

int getClientIndexByUsername(char *username){
	if(clients != NULL){
		for(int i = 0; i < numClients; i++){
			if(!strcmp(clients[i].username,username)){
				return i;
			}
		}
	}
	return -1;
}

void updateLeaderboardWithClient(char *username, int gameWon){
	int clientIndex = getClientIndexByUsername(username);

	clients[clientIndex].gamesPlayed++;
	clients[clientIndex].gamesWon += gameWon;

	orderLeaderboard(); 
}


void clearAccounts(){
	for(int i = 0; i < numAccounts; i++){
		free(accounts[i].username);
		free(accounts[i].password);
	}
	free(accounts);
}

void clearClients(){
	for(int i = 0; i < numClients; i++){
		free(clients[i].username);
		if(clients[i].firstWord != NULL){
			free(clients[i].firstWord);
			free(clients[i].lastWord);
		}
	}
	free(clients);
}














void leaderboardReadUnlock(){
	pthread_mutex_lock(&readerCounterMutex);
	readerCounter--;
	
	if(readerCounter == NO_READING_OCCURING){
		pthread_mutex_unlock(&writerMutex);
	}
	
	pthread_mutex_unlock(&readerCounterMutex);
}

void leaderboardReadLock(){
	pthread_mutex_lock(&readerMutex);
	pthread_mutex_lock(&readerCounterMutex);
	readerCounter++;
	
	if(readerCounter == READ_OCCURRING){
		pthread_mutex_lock(&writerMutex);
	}
	
	pthread_mutex_unlock(&readerCounterMutex);
	pthread_mutex_unlock(&readerMutex);
}

void leaderboardWriteLock(){
	pthread_mutex_lock(&writerMutex);
	pthread_mutex_lock(&readerMutex);
}

void leaderboardWriteUnlock(){
	pthread_mutex_unlock(&writerMutex);
	pthread_mutex_unlock(&readerMutex);
}

void initialiseLeaderboardMutex(){
	readerCounter = 0;
	pthread_mutex_init(&writerMutex, NULL);
	pthread_mutex_init(&readerCounterMutex, NULL);
	pthread_mutex_init(&readerMutex, NULL);
	
}















