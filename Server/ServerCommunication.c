#define _GNU_SOURCE
#include "TextProcessor.h"

#include <stdio.h>       /* standard I/O routines                     */
#include <pthread.h>     /* pthread functions and data structures     */
#include <stdlib.h>      /* rand() and srand() functions              */
#include <time.h>
#include <unistd.h>


#define MAX_CONNECTIONS 10
#define BACKLOG 10

#define HANGMAN 	1
#define LEADERBOARD 	2
#define EXIT		3

#define GAME_CONTINUE 0
#define GAME_LOST 1
#define GAME_WON 2
#define NO_ACTIVE_CONNECTION -1
#define WON_GAME 1
#define LOST_GAME 0 

#define MAX_DATA_SIZE 100

#define DEFAULT_SOCKET 12345

#define LETTER_Z 'z'
#define LETTER_A 'a'

pthread_t  clientThreads[MAX_CONNECTIONS];

pthread_mutex_t connectionMutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
pthread_cond_t  gotConnection   = PTHREAD_COND_INITIALIZER;



struct connection{
	int socketId;
	struct connection* next;
};

struct thread{
	int threadId;
	int socketId;
};

//pthread_mutex_t connectionMutex;
//pthread_cond_t gotConnection;

int numConnections = 0;
int numActiveConnections = 0;
int rc;
int closeProgram = FALSE;

struct connection *connections = NULL;
struct connection *lastConnection = NULL;

struct thread *threads;


void getUserCredentials(int socketId, char *user, char *password){
	int numBytes = recv(socketId, user, MAX_DATA_SIZE, 0);

	user[numBytes] = '\0';

	numBytes = recv(socketId, password, MAX_DATA_SIZE, 0);
	
	password[numBytes] = '\0';
	printf("%s, %s\n", user, password);
}

void sendAuthenticationResult(int socketId, int authenticationResult){
	uint16_t item  = htons(authenticationResult);	
	send(socketId, &item, sizeof(uint16_t), 0);
}

int getCommand(int socketIdentifier){
	int command = 0;
	uint16_t value = 0;

	recv(socketIdentifier, &value, sizeof(uint16_t), 0);
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
	//printf("got to read value\n");

	while(status != GAME_CONTINUE || status != GAME_WON || status != GAME_LOST){
		recv(socketId, &value, sizeof(uint16_t), 0);
	
		status = ntohs(value);
	}
	return status;
}

void hangmanGame(int socketId, Client *client){
	int *wordInformation = malloc(3 * sizeof(int));
	char guess;
	int wonGame = LOST_GAME;

	initialiseClientWords(client, numWords);
	wordInformation[0] = words[client->wordId].firstLength;
	wordInformation[1] = words[client->wordId].lastLength;
	wordInformation[2] = words[client->wordId].maxGuess;
	
	sendWordLength(socketId, wordInformation);
	
	while(1){
		guess = getClientGuess(socketId);
		if(guess <= LETTER_Z && guess >= LETTER_A){
			getLetterLocations(client, guess);
			sendLetterPositions(socketId, client);
			resetLetterLocations(client);
			wordInformation[2]--;
		} else if (guess == GAME_WON || guess == GAME_LOST){
			break;
		}
	}
	
	printf("Locking Write Mutex for Client %d\n", getClientIndexByUsername(client->username) + 1);
	leaderboardWriteLock();
	printf("Updating leaderboard for Client %d\n", getClientIndexByUsername(client->username) + 1);
	//printf("%s, %d, %d\n", client->username, client->gamesPlayed, client->gamesWon);

	printf("Unlocking Write Mutex for Client %d\n", getClientIndexByUsername(client->username) + 1);
	if(guess == GAME_WON){
		wonGame = WON_GAME;
	}

	updateLeaderboardWithClient(client->username, wonGame);
	printf("got to unlocking\n");
	leaderboardWriteUnlock();

	free(wordInformation);
}

void performCommand(int command, int socketId, Client *client){
	switch(command){
		case HANGMAN:			
			hangmanGame(socketId, client);
			break;
		case LEADERBOARD:
			leaderboardReadLock();
			sendClientLeaderboard(socketId);
			leaderboardReadUnlock();

			break;
		case EXIT:
			printf("closing connection");
			close(socketId);
			break;
	}
}

void addConnection(int socketId, pthread_mutex_t *pMutex, pthread_cond_t *pCondVar){
	struct connection *clientConnection;

	clientConnection = (struct connection*)malloc(sizeof(struct connection));

	if(!clientConnection){
		fprintf(stderr, "AddConnection: out of memory\n");
		exit(1);
	}

	clientConnection->socketId = socketId;
	clientConnection->next = NULL;
	
	rc = pthread_mutex_lock(pMutex);
	
	if(numConnections == NO_PENDING_CONNECTIONS){
		connections = clientConnection;
		lastConnection = clientConnection;
	} else{
		lastConnection->next = clientConnection;
		lastConnection = clientConnection;
	}
	numConnections++;

	rc = pthread_mutex_unlock(pMutex);
	rc = pthread_cond_signal(pCondVar);
	
}

struct connection* getConnection(pthread_mutex_t *pMutex){
	struct connection *clientConnection;
	
	rc = pthread_mutex_lock(pMutex);
	
	if(numConnections > 0){
		clientConnection = connections;
		connections = clientConnection->next;
		if(connections == NULL){
			lastConnection = NULL;
		}
		numConnections--;
		
	} else{

		clientConnection = NULL;
	}

	rc = pthread_mutex_unlock(pMutex);
	return clientConnection;
}

void handleConnection(int socketId){
	int command = 0;
	int accountVerified;
	char *username = malloc(MAX_DATA_SIZE * sizeof(char));
	char *password = malloc(MAX_DATA_SIZE * sizeof(char));
	int clientIndex = 0;
	getUserCredentials(socketId, username, password);

	accountVerified = checkCredentials(username, password, numAccounts);

	printf("Account Verified: %d\n", accountVerified);

	sendAuthenticationResult(socketId, accountVerified);
	if(accountVerified == TRUE){

		clientIndex = getClientIndexByUsername(username);

		if(clientIndex == -1){
			clientIndex = addClient(username);
			printf("Client Index: %d\n", clientIndex);
		}
		
		while (command != EXIT){
			command = getCommand(socketId);
			//printf("command received was: %d\n", command);
			performCommand(command, socketId, &clients[clientIndex]);
		}
		free(username);
		free(password);
	} else{
		printf("User was not verified, closing connection");
		close(socketId);
	}
}

void* threadConnectionHandler(void* data){
	struct connection *clientConnection;
	rc = pthread_mutex_lock(&connectionMutex);
	int threadId = *(int*)data;
	while(1){
		if(numConnections > 0){
			clientConnection = getConnection(&connectionMutex);

			if(clientConnection){
				numActiveConnections++;
		
				threads[threadId].socketId = clientConnection->socketId;		
				rc = pthread_mutex_unlock(&connectionMutex);
				handleConnection(clientConnection->socketId);
				close(clientConnection->socketId);
				free(clientConnection);
				rc = pthread_mutex_lock(&connectionMutex);
				threads[threadId].socketId = NO_ACTIVE_CONNECTION;
				
				numActiveConnections--;
			}
		} else{
			rc = pthread_cond_wait(&gotConnection, &connectionMutex);
		}
	}
}

void clearThreads(){
	for (int i = 0; i < MAX_CONNECTIONS; i++){
		if(threads[i].socketId != NO_ACTIVE_CONNECTION){
			printf("closing Thread %d's connection\nSocketId = %d", i, threads[i].socketId);
			close(threads[i].socketId);
			//pthread_join(i, NULL);
		}
	}
	free(threads);
	/*	
	for(int i = 0; i < MAX_CONNECTIONS; i++){
		pthread_exit(&clientThreads[i]);
	}
	*/
}

void endProgram(){
	printf("\nShutting Down Server...\n");
	clearThreads();
	clearClients();
	clearWords();	
	clearAccounts();
	
	printf("Goodbye!\n");
	exit(0);
}

void endProgramHandler() {
	endProgram();
}



void createThreads(){ 	
	for(int i = 0; i < MAX_CONNECTIONS; i++){
		pthread_create(&clientThreads[i], NULL, threadConnectionHandler, (void*)&i);
		threads[i].socketId = NO_ACTIVE_CONNECTION;
	}
}

int main(int argc, char *argv[]){
	int sockFd, newFd;
	int serverSocket;
	struct sockaddr_in serverAddress;
	struct sockaddr_in clientAddress;
	socklen_t sin_size;
	signal(SIGINT, endProgramHandler);

	//initialiseMutexConnections();
	initialiseLeaderboardMutex();

	numWords = getTextFileLength(HANGMAN_FILE);

	//subtract 1 to disclude header
	numAccounts = getTextFileLength(ACCOUNTS_FILE) - 1;
	words = malloc(numWords * sizeof(Word));
	accounts = malloc(numAccounts * sizeof(Account));
	clients = malloc(sizeof(Client));
	numClients = 0;

	threads = malloc(MAX_CONNECTIONS * sizeof(struct thread));

	storeCredentials();
	readInWords();

	if (argc != 2) {
		serverSocket = DEFAULT_SOCKET;
		printf("No socket provided by the user.  Using default socket\n");
	} else{
		serverSocket = atoi(argv[1]);
	}
	printf("The socket used by the server is: %d\n\n", serverSocket);
	
	if((sockFd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("socket error");
		exit(1);
	}

	serverAddress.sin_family = AF_INET;         /* host byte order */
	serverAddress.sin_port = htons(serverSocket);     /* short, network byte order */
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

	createThreads();

	while(closeProgram != TRUE){
		sin_size = sizeof(struct sockaddr_in);
		if ((newFd = accept(sockFd, (struct sockaddr *)&clientAddress, \
		&sin_size)) == -1) {
			perror("accept");
			continue;
		}
		printf("closeProgram is: %d\n", closeProgram);
		printf("server: got connection from %s\n", \
			inet_ntoa(clientAddress.sin_addr));
		if(numActiveConnections != MAX_CONNECTIONS){
			addConnection(newFd, &connectionMutex, &gotConnection);
		} else{
			printf("Reached maximum number of connections. closing new connection\n");
			close(newFd);
		}
		
	}
	printf("got here\n");
	endProgram();
	return 0;
}

