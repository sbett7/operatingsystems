#define _GNU_SOURCE
#include "TextProcessor.h"
#include "ClientLeaderboardFunctions.h"
#include "ServerCommunication.h"

#include <stdio.h>       /* standard I/O routines                     */
#include <pthread.h>     /* pthread functions and data structures     */
#include <stdlib.h>      /* rand() and srand() functions              */
#include <time.h>
#include <unistd.h>

#define GAME_LOST 1
#define GAME_WON 2
#define WON_GAME 1
#define LOST_GAME 0

#define MAX_CONNECTIONS 10
#define NO_PENDING_CONNECTIONS 0
#define NO_ACTIVE_CONNECTION -1

#define BACKLOG 10

#define HANGMAN 	1
#define LEADERBOARD 	2
#define EXIT		3

#define DEFAULT_SOCKET 12345

#define LETTER_Z 'z'
#define LETTER_A 'a'

#define FIRST_WORD 0
#define LAST_WORD 1
#define MAX_GUESS 2

#define NO_CLIENT_LISTED -1
#define PARAMETER_GIVEN 2

pthread_t  clientThreads[MAX_CONNECTIONS];

pthread_mutex_t connectionMutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
pthread_mutex_t activeConnectionMutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
pthread_cond_t  gotConnection   = PTHREAD_COND_INITIALIZER;

/* Linked List that contains waiting connections */
struct connection{
	int socketId;
	struct connection* next;
};

/* struct that relates a thread to a specific socketId	*/
struct thread{
	int threadId;
	int socketId;
};

// connection global variables
int numConnections = 0;
int numActiveConnections = 0;
int rc;

// global connection linked lists
struct connection *connections = NULL;
struct connection *lastConnection = NULL;

// global threads struct array
struct thread *threads;

/*
This function is used to perform the Hangman game.  The function will perform
	the game and update the leaderboard after the game has finished.
int socketId: an integer that relates to the socket of the client playing
	the hangman game.
Client *client: a pointer to a Client struct that contains all of the 
	client's current information.
Returns: Void
*/
void hangmanGame(int socketId, Client *client){
	int *wordInformation = malloc(3 * sizeof(int));
	char guess;
	int wonGame = LOST_GAME;

	//initialise the client and the word information
	initialiseClientWords(client, numWords);
	wordInformation[FIRST_WORD] = words[client->wordId].firstLength;
	wordInformation[LAST_WORD] = words[client->wordId].lastLength;
	wordInformation[MAX_GUESS] = words[client->wordId].maxGuess;
	
	//send word information to the client
	sendWordDetails(socketId, wordInformation);
	free(wordInformation);
	
	//play the game until the client has sent back a finished command
	while(1){
		guess = getClientGuess(socketId);
		if(guess <= LETTER_Z && guess >= LETTER_A){
			getLetterLocations(client, guess);
			sendLetterPositions(socketId, client);
			resetLetterLocations(client);
		}
		if(guess == GAME_WON || guess == GAME_LOST){
			break;
		}
	}
	
	if(guess == GAME_WON){
		wonGame = WON_GAME;
	}
	
	//START CRITICAL SECTION - functions located in ClientLeaderboardFunctions.c
	leaderboardWriteLock(); 
	updateLeaderboardWithClient(client->clientId, wonGame);
	leaderboardWriteUnlock();
	//END CRITICAL SECTION
}

/*
This function acts as the menu controller for the client.  It performs
	the command that was given to the function.
int command: The command to that is to be performed.
int socketId: an integer that relates to the socket of the client that
	is communicating with the server.
Client *client: a pointer to a Client struct that contains all of the 
	client's current information.
Returns: Void
*/
void performCommand(int command, int socketId, Client *client){
	switch(command){
		case HANGMAN:			
			hangmanGame(socketId, client);
			break;
		case LEADERBOARD:
			//START CRITICAL SECTION - functions located in ClientLeaderboardFunctions.c
			leaderboardReadLock();
			sendClientLeaderboard(socketId);
			leaderboardReadUnlock();
			//END CRITICAL SECTION
			break;
		case EXIT:
			printf("closing connection");
			close(socketId);
			break;
	}
}

/*
This function adds a pending connection to the linked list.
int socketId: an integer that relates to the socket of the client that
	is communicating with the server.
pthread_mutex_t *pMutex: a pointer to a mutex object that protects the linked 
	list when adding a connection to the list.
pthread_cond_t *pCondVar: a pointer to a mutex condition object that is used to signal
	when a connection has been added to the linked list.
Returns: Void
*/
void addConnection(int socketId, pthread_mutex_t *pMutex, pthread_cond_t *pCondVar){
	
	//initialise the connection object
	struct connection *clientConnection;
	clientConnection = (struct connection*)malloc(sizeof(struct connection));

	if(!clientConnection){
		fprintf(stderr, "AddConnection: out of memory\n");
		exit(1);
	}
	clientConnection->socketId = socketId;
	clientConnection->next = NULL;
	
	//CRITICAL SECTION
	rc = pthread_mutex_lock(pMutex);
	
	//add connection to the start of the list if no pending connections
	if(numConnections == NO_PENDING_CONNECTIONS){
		connections = clientConnection;
		lastConnection = clientConnection;
	} else{ //add connection to end of list
		lastConnection->next = clientConnection;
		lastConnection = clientConnection;
	}
	numConnections++;

	rc = pthread_mutex_unlock(pMutex);
	rc = pthread_cond_signal(pCondVar);
	//END CRITICAL SECTION
}

/*
This function gets a connection from the head of the connections linked list.
pthread_mutex_t *pMutex: a pointer to a mutex object that protects the linked 
	list when reading and removing a connection from the list.
Returns: a struct connection at the head of the connections linked list.
*/
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

/*
This function handles the connection with the client by verifying their credentials.
	If the user cannot be authenticated, the connection is terminated, otherwise
	the server will allow the client to send commands for it to perform. 
int socketId: the socket identifier for the client that is connected to the server.
Returns: Void.
*/
void handleConnection(int socketId){
	int command = 0;
	int accountVerified;
	char *username = malloc(MAX_DATA_SIZE * sizeof(char));
	char *password = malloc(MAX_DATA_SIZE * sizeof(char));
	int clientIndex = 0;
	
	//get and authenticate credentials, and return result to client
	getUserCredentials(socketId, username, password);
	accountVerified = checkCredentials(username, password, numAccounts);
	sendAuthenticationResult(socketId, accountVerified);
	
	//if authenticated, get client information/register new client
	if(accountVerified == TRUE){

		clientIndex = getClientIndexByUsername(username);

		//if no client present, register new client credentials
		if(clientIndex == NO_CLIENT_LISTED){
			leaderboardWriteLock();
			clientIndex = addClient(username);
			leaderboardWriteUnlock();
		}
		
		while (command != EXIT){
			command = getCommand(socketId);
			performCommand(command, socketId, &clients[clientIndex]);
		}

		free(username);
		free(password);
	} else{
		printf("Connection could not be authenticated. Closing the connection.");
	}
}

/*
This function handles the threadpool.  It will pass a pending connection to a waiting
	thread.  If no connection is available, it will wait for a connection.
void *data: A pointer to an integer that contains the thread's id.
Returns: Void.
*/
void* threadConnectionHandler(void* data){
	struct connection *clientConnection;
	rc = pthread_mutex_lock(&connectionMutex);
	int threadId = *((int*)data);

	while(1){
		if(numConnections > 0){

			clientConnection = getConnection(&connectionMutex);

			if(clientConnection){
				pthread_mutex_lock(&activeConnectionMutex);
				numActiveConnections++;
				pthread_mutex_unlock(&activeConnectionMutex);

				threads[threadId].socketId = clientConnection->socketId;
						
				rc = pthread_mutex_unlock(&connectionMutex);
				handleConnection(clientConnection->socketId);
				close(clientConnection->socketId);
				free(clientConnection);
				rc = pthread_mutex_lock(&connectionMutex);
				threads[threadId].socketId = NO_ACTIVE_CONNECTION;
				
				pthread_mutex_lock(&activeConnectionMutex);
				numActiveConnections--;
				pthread_mutex_unlock(&activeConnectionMutex);
			}
		} else{
			rc = pthread_cond_wait(&gotConnection, &connectionMutex);
		}
	}
}

/*
This function creates new threads and provides them with a thread id.
	It also initialises the thread connections struct to have no active connections.
Returns: Void.
*/
void createThreads(){
		
	for(int i = 0; i < MAX_CONNECTIONS; i++){
		threads[i].threadId = i;
		threads[i].socketId = NO_ACTIVE_CONNECTION;
		pthread_create(&clientThreads[i], NULL, threadConnectionHandler, (void*)&threads[i].threadId);
		
		
	}
}


/*
This function closes all active connections and frees the threadpool.
Returns: Void.
*/
void clearThreads(){
	for (int i = 0; i < MAX_CONNECTIONS; i++){
		if(threads[i].socketId != NO_ACTIVE_CONNECTION){
			printf("closing Thread %d's connection\nSocketId = %d\n", i, threads[i].socketId);
			close(threads[i].socketId);
		}
	}
	free(threads);
}

/*
This function closes all pending connections and frees the connection linked lists.
Returns: Void.
*/
void clearConnections(){
	for(; connections != NULL; connections = connections->next){
		close(connections->socketId);
	}

	free(connections);
	free(lastConnection);
}

/*
This function is the event handler for the SIGINT interrupt.  It will perform
	cleanup of the server before closing the program.
Returns: Void.
*/
void endProgramHandler() {
	printf("\nShutting Down Server...\n");
	clearThreads();
	clearConnections();
	clearClients();
	clearWords();	
	clearAccounts();

	printf("Goodbye!\n");
	exit(0);
}

/*
This function initialises all of the objects that are required for
	the server to play hangman and manage clients and connections.
Returns: Void.
*/
void initialiseHangmanObjects(){
	initialiseLeaderboardMutex();

	// get length of text files
	numWords = getTextFileLength(HANGMAN_FILE);
	numAccounts = getTextFileLength(ACCOUNTS_FILE) - 1; //subtract 1 to remove the username/password header

	//initialise structures and variables
	words = malloc(numWords * sizeof(Word));
	accounts = malloc(numAccounts * sizeof(Account));
	clients = malloc(sizeof(Client));
	threads = malloc(MAX_CONNECTIONS * sizeof(struct thread));
	numClients = 0;
	numActiveConnections = 0;
	numConnections = 0;

	//read in credentials and words from text file
	storeCredentials();
	readInWords();
	
	//initialise random number generator
	initialiseRandomNumberGenerator();
	
	createThreads();
}

int main(int argc, char *argv[]){
	int sockFd, newFd;
	struct sockaddr_in serverAddress;
	struct sockaddr_in clientAddress;
	socklen_t sin_size;
	signal(SIGINT, endProgramHandler);

	int serverSocket;
	initialiseHangmanObjects();
	

	//set default socket if no socket was provided 
	if (argc != PARAMETER_GIVEN) {
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

	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(serverSocket);     
	serverAddress.sin_addr.s_addr = INADDR_ANY; 
	
	memset(serverAddress.sin_zero, '\0', sizeof(serverAddress.sin_zero));

	if(bind(sockFd, (struct sockaddr *)&serverAddress, sizeof(struct sockaddr)) == -1){
		perror("binding error");
		exit(1);
	}

	if(listen(sockFd, BACKLOG) == -1){
		perror("listening error");
	}

	while(1){
		sin_size = sizeof(struct sockaddr_in);
		
		if ((newFd = accept(sockFd, (struct sockaddr *)&clientAddress, \
		&sin_size)) == -1) {
			perror("accept");
			continue;
		}

		printf("A connection has been received from %s\n", \
			inet_ntoa(clientAddress.sin_addr));
		
		if(numActiveConnections != MAX_CONNECTIONS){
			addConnection(newFd, &connectionMutex, &gotConnection);
		} else{
			printf("Reached maximum number of connections. closing new connection\n");
			close(newFd);
		}
		
	}
	return 0;
}

