#include "ClientCommunication.c"

#define MAXDATASIZE 100 /* max number of bytes we can get at once */
#define STRING_SIZE 150

#define ARRAY_SIZE 30

#define PORT_NO 54321 /* PORT Number */

#define TRUE 1
#define FALSE 0


/*
This function sends an array value for menu communication
int socket_id: the socket identifier for the server
int command: the menu command sent to the server
Returns: Void.
*/
void sendCommand(int socket_id, int command) {

	uint16_t item = 0;	+

	item = htons(command);
	send(socket_id, &item, sizeof(uint16_t), 0);
	

}

/*
This function obtains the length of the phrase from the server
int socketid: the socket identifier for the server
int wordInformation: array to store the lengths of each word
Returns: Void.
*/
void receiveWordInformation(int socketId, int wordInformation[3]){
	uint16_t value;
	int retValue;

	for(int i = 0; i < 3; i++){
		recv(socketId, &value, sizeof(uint16_t), 0);
		wordInformation[i] = ntohs(value);
	}
}

/*
This function receives each of the values for the leaderboard from the server
	and prints it to the console
int socketid: the socket identifier for the server
int wordInformation: array to store the lengths of each word
Returns: Void.
*/
void receivePrintLeaderboard(int socketId){
	uint16_t value;
	int numClients;
	char username[STRING_SIZE];
	int retval;
	int gamesWon;
	int gamesPlayed;
	int charSize = 0;
	
	recv(socketId, &value, sizeof(uint16_t), 0);
	numClients = ntohs(value);

	if(numClients == 0){
		printf("No Entries in Leaderboard\n");
	}else{
		printf("Number of Clients in Leaderboard: %d\n", numClients);
		for (int i = 0; i < numClients; i++){
			recv(socketId, &value, sizeof(uint16_t), 0);
			charSize = ntohs(value);

			recv(socketId, &username, charSize, 0);

			recv(socketId, &value, sizeof(uint16_t), 0);
			gamesPlayed = ntohs(value);

			recv(socketId, &value, sizeof(uint16_t), 0);

			gamesWon = ntohs(value);
			printMenu(STUB);
			printf("Player - %s\nNumber of Games Played - %d\nNumber of Games Won - %d\n\n", username, gamesPlayed, gamesWon);
			printMenu(STUB);
		}
	}
}

/*
This function checks if the character sent matches the word in any places
int socketid: the socket identifier for the server
int *position: integer array of matched positions
int length: length of the word
Returns: Void.
*/
void getLetterPosition(int socketId, int *position, int length){
	
	uint16_t value;
	for(int i = 0; i < length; i++){
		recv(socketId, &value, sizeof(uint16_t), 0);
		position[i] = ntohs(value);
	}
}

/*
This function sends the character input from the client to the server
char socketId: socket identifier for the server
char letter: character the client has input
Returns: Void.
*/
void sendGuess(int socketId, char letter){
	int guess = (int) letter;
	uint16_t value = htons(guess);
	if(send(socketId, &value, sizeof(uint16_t), 0) < 0){
		printf("error\n");
	}
	send(socketId, &letter, sizeof(char), 0);
}

/*
Sends the current status fo the game to the server
char socketId: socket identifier for the server
char status: value based on win/lose condition
Returns: Void.
*/
void sendGameStatus(int socketId, int status){

	uint16_t value = htons(status);
	if(send(socketId, &value, sizeof(uint16_t), 0) < 0){
		printf("error\n");
	}
}

/*
Sends the username and password to the server
char socketId: socket identifier for the server
char *username: username credential input by the client
char *password: password credential input by the client
Returns: Void.
*/
void sendCredentials(int socketId, char *username, char *password){
	send(socketId, &username, MAXDATASIZE, 0);
	send(socketId, &password, MAXDATASIZE, 0);
}

/*
Retrieves the result of the authorisation attempt by the client
char socketId: socket identifier for the server
Returns: int value for authorisation result
*/
int getAuthorisationResult(int socketId){
	int result;
	uint16_t item;	
	recv(socketId, &item, sizeof(uint16_t), 0);
	
	result = ntohs(item);
	return result;
}

