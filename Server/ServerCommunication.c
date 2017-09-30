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

#include "TextProcessor.h"

#define BACKLOG 10

#define HANGMAN 	1
#define LEADERBOARD 	2
#define EXIT		3

#define GAME_CONTINUE 0
#define GAME_LOST 1
#define GAME_WON 2

#define MAX_DATA_SIZE 100

/*
void *clientHandler(void *socketDesc){
	int socket = *(int*)socketDesc;
}

*/

int numWords;

void getUserCredentials(int socketId, char *user, char *password){
	printf("got here\n");
	int numBytes = recv(socketId, user, MAX_DATA_SIZE, 0);

	user[numBytes] = '\0';
	printf("got here\n");
	numBytes = recv(socketId, password, MAX_DATA_SIZE, 0);
	
	password[numBytes] = '\0';
}

void sendAuthenticationResult(int socketId, int authenticationResult){
	uint16_t item  = htons(authenticationResult);	
	send(socketId, &item, sizeof(uint16_t), 0);
}

int getCommand(int socketIdentifier){
	int command = 0;
	uint16_t value = 0;
	int retValue = 0;
	//printf("got to read value\n");

	retValue = recv(socketIdentifier, &value, sizeof(uint16_t), 0);
	command = ntohs(value);
	return command;
}

void sendWordLength(int socketId, int wordDetails[3]){

	uint16_t item = 0;

	for(int i = 0; i < 3; i++){
		item = htons(wordDetails[i]);
		if(send(socketId, &item, sizeof(uint16_t), 0) < 0){
			printf("error\n");
		}
		//printf("%d\n", send(socketId, &item, sizeof(uint16_t), 0));
	}
}

char getClientGuess(int socketId){
	char guess;
	uint16_t value;	
	recv(socketId, &value, sizeof(uint16_t), 0);
	
	guess = (char)ntohs(value);
	return guess;
}

void sendLetterPositions(int socketId, Client *client){
	int length = client->firstLength + client->lastLength + 1;
	uint16_t item = 0;
	int letter = 0;
	for(int i = 0; i < length; i++){
		if(i < client->firstLength){
			letter = client->firstWord[i];
		} else if (i > client->firstLength){
			letter = client->lastWord[(i-1) - client->firstLength];
		} else {
			letter = FALSE;
		}
		item = htons(letter);
		send(socketId, &item, sizeof(uint16_t), 0);
	}
}

int getGameStatus(int socketId){
	int status = -1;
	uint16_t value = 0;
	int retValue = 0;
	//printf("got to read value\n");

	while(status != GAME_CONTINUE || status != GAME_WON || status != GAME_LOST){
		retValue = recv(socketId, &value, sizeof(uint16_t), 0);
	
		status = ntohs(value);
	}
	return status;
}

void performCommand(int command, int socketId, Client *client){
	int *wordInformation = malloc(3 * sizeof(int));
	char guess;
	int status = GAME_CONTINUE;
	printf("got here");
	initialiseClientWords(client, numWords);
	//printf("%d, %d\n", client->firstLength, client->lastLength);
	printf("%s %s\n", words[client->wordId].firstWord, words[client->wordId].lastWord);

	wordInformation[0] = words[client->wordId].firstLength;
	wordInformation[1] = words[client->wordId].lastLength;
	wordInformation[2] = words[client->wordId].maxGuess;

	switch(command){
		case HANGMAN:			
			printf("requested game\n");
			client->gamesPlayed++;
			sendWordLength(socketId, wordInformation);
			
			while(1){
				guess = getClientGuess(socketId);
				if(guess <= 'z' && guess >= 'a'){
					getLetterLocations(client, guess);
					sendLetterPositions(socketId, client);
					resetLetterLocations(client);
					wordInformation[2]--;
					printf("Guess: %c\nNumber of guesses remaining: %d\n", guess, wordInformation[2]);
				} else if (guess == GAME_WON || guess == GAME_LOST){
					break;
				}
			}
			printf("fininshed game\n");
			if(guess == GAME_WON){
				client->gamesWon++;
				printf("%s has won: %d\n%s has played: %d\n",client->username, client->gamesWon,client->username, client->gamesPlayed);
				printf("\nGame Completed with player winning\n");
			} else {
				printf("%s has won: %d\n%s has played: %d\n",client->username, client->gamesWon,client->username, client->gamesPlayed);
				printf("\nGame Completed with player losing\n");
			}
			break;
		case LEADERBOARD:
			printf("requested leaderboard\n");
			break;
		case EXIT:
			printf("Client is disconnecting\n");
			close(socketId);
			exit(0);
			break;
	}
	free(wordInformation);
}

void HangmanFunction(void* socketId){
	int sockFd = *(int*) socketId;
	int command = HANGMAN;
	int accountVerified;
	char *username = malloc(MAX_DATA_SIZE * sizeof(char));
	char *password = malloc(MAX_DATA_SIZE * sizeof(char));
	getUserCredentials(sockFd, username, password);	
	accountVerified = readCredentials(username, password);
	sendAuthenticationResult(sockFd, accountVerified);
	if(accountVerified){
		Client client;
		client.clientId = sockFd;
		initialiseClient(sockFd, &client, username);
	
	
		while (command != EXIT){
			command = getCommand(sockFd);
			performCommand(HANGMAN, sockFd, &client);
		}
	} else{
		
	}
	
	
}

int main(int argc, char *argv[]){
	int sockFd, newFd;
	struct sockaddr_in serverAddress;
	struct sockaddr_in clientAddress;
	socklen_t sin_size;

	pthread_t client_thread;

	numWords = getTextFileLength(HANGMAN_FILE);
	words = malloc(numWords * sizeof(Word));
	
	readInWords();

	if (argc != 2) {
		fprintf(stderr,"usage: port number\n");
		exit(1);
	}
	
	if((sockFd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("socket error");
		exit(1);
	}

	serverAddress.sin_family = AF_INET;         /* host byte order */
	serverAddress.sin_port = htons(atoi(argv[1]));     /* short, network byte order */
	serverAddress.sin_addr.s_addr = INADDR_ANY; /* auto-fill with my IP */
	
	 memset(serverAddress.sin_zero, '\0', sizeof(serverAddress.sin_zero));

	if(bind(sockFd, (struct sockaddr *)&serverAddress, sizeof(struct sockaddr)) == -1){
		perror("binding error");
		exit(1);
	}

	if(listen(sockFd, BACKLOG) == -1){
		perror("listening error");
	}

	printf("server has begun listening\n");

	while(1){
		sin_size = sizeof(struct sockaddr_in);
		if ((newFd = accept(sockFd, (struct sockaddr *)&clientAddress, \
		&sin_size)) == -1) {
			perror("accept");
			continue;
		}
		printf("server: got connection from %s\n", \
			inet_ntoa(clientAddress.sin_addr));
		


		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_create(&client_thread, NULL, (void*)HangmanFunction, (void*)&newFd);

		
		pthread_join(client_thread, NULL);
		//close(newFd);  /* parent doesn't need this */

		while(waitpid(-1,NULL,WNOHANG) > 0); /* clean up child processes */
		
	}
}

