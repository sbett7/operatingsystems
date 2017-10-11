#include "Communication.h"

/*
This function waits to receive the username and password from the client
int socketId: An integer that relates to the socket of the client that is
	is sending the information.
char *user: A char array that is used to store the received username. 
char *password: A char array that is used to store the received password. 	
Returns: Void
*/
void getUserCredentials(int socketId, char *user, char *password){
	//receive the username	
	int numBytes = recv(socketId, user, MAX_DATA_SIZE, 0);
	//set the last element to be a terminating character.
	user[numBytes] = '\0';

	//receive password
	numBytes = recv(socketId, password, MAX_DATA_SIZE, 0);

	//set the last element to be a terminating character.
	password[numBytes] = '\0';
}

/*
This function sends the result of the authentication method for the username
	and password to the client.
int socketId: An integer that relates to the socket of the client that is
	is receiving the information.
int authenticationResult: An integer with the result of the authentication method. 
Returns: Void
*/
void sendAuthenticationResult(int socketId, int authenticationResult){
	uint16_t item  = htons(authenticationResult);	
	send(socketId, &item, sizeof(uint16_t), 0);
}

/*
This function waits to receive the command value from the client.
int socketId: An integer that relates to the socket of the client that is
	is sending the information.
Returns: An integer that relates to the command that the client wishes to perform.
*/
int getCommand(int socketIdentifier){
	int command = 0;
	uint16_t value = 0;

	recv(socketIdentifier, &value, sizeof(uint16_t), 0);
	command = ntohs(value);
	return command;
}

/*
This function sends the information about the user's word to the client.
int socketId: An integer that relates to the socket of the client that is
	is receiving the information.
int wordDetails[3]: An integer array with 3 elements. The information sent
	includes the first, and last word's length, and the maximum number of guesses,
	in that order. 
Returns: Void
*/
void sendWordDetails(int socketId, int wordDetails[3]){
	uint16_t item = 0;

	for(int i = 0; i < 3; i++){
		item = htons(wordDetails[i]);
		send(socketId, &item, sizeof(uint16_t), 0);
	}
}

/*
This function waits to receive the client's guess for the word.
int socketId: An integer that relates to the socket of the client that is
	is sending the information.
Returns: The client's guess in char format.
*/
char getClientGuess(int socketId){
	char guess;
	uint16_t value;	

	recv(socketId, &value, sizeof(uint16_t), 0);
	guess = (char)ntohs(value);
	return guess;
}

/*
This function sends the correctly guessed letter's positions to the client.
	It will send each individual position separately.
int socketId: An integer that relates to the socket of the client that is
	is receiving the information.
Client *client: The client struct that contains the word information that is to 
	be sent to the client. 
Returns: Void
*/
void sendLetterPositions(int socketId, Client *client){
	int length = client->firstLength + client->lastLength + 1;
	uint16_t item = 0;
	int letter = 0;

	//for all positions from the start of first word to end of last word
	for(int i = 0; i < length; i++){
		//if i is before the end of first word, send first word
		if(i < client->firstLength){
			letter = client->firstWord[i];
		} else if (i > client->firstLength){ // if letter is after first word, and space send last word
			letter = client->lastWord[(i-1) - client->firstLength];
		} else { // send space
			letter = SPACE;
		}
		item = htons(letter);
		send(socketId, &item, sizeof(uint16_t), 0);
	}
}
