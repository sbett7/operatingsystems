#include "ClientLeaderboardFunctions.h"

/*
This function resets the letter location arrays for the given client to their default values. 
Client *client: a pointer to the client that is having their letter location arrays reset.
Returns: Void.
*/
void resetLetterLocations(Client *client){
	// get the maximum length to perform the minimum required iterations
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

/*
This function checks whether the given letter is present within the client's word. 
Client *client: a pointer to the client that is having their letter location arrays checked.
char letter: a char that contains the letter to look for.
Returns: Void.
*/
void getLetterLocations(Client *client, char letter){
	// get the maximum length to perform the minimum required iterations
	int iterations = max(client->firstLength, client->lastLength);
	
	for(int i = 0; i < iterations; i++){
		//if still iterating over first word, check if letter is present
		if (i < client->firstLength && 
			words[client->wordId].firstWord[i] == letter){
			client->firstWord[i] = TRUE;
		}
		//if still iterating over last word, check if letter is present
		if (i < client->lastLength && 
			words[client->wordId].lastWord[i] == letter){
			client->lastWord[i] = TRUE;
		}
	}
}

/*
This function initialises the provided client with the given username, and sets the
	games played and won to 0.
Client *client: a pointer to the client that is being initialised.
char *username: a pointer to a char array that contains the username of the new client.
Returns: Void.
*/
void initialiseClient(Client *client, char *username){
	client->clientId = numClients;
	printf("A new client has been registed with the following clientID: %d\n", client->clientId);
	client->username = malloc(sizeof(username));
	strcpy(client->username, username);
	client->gamesPlayed = 0;
	client->gamesWon = 0;
}

/*
This function initialises the provided client a random word.
Client *client: a pointer to the client that is being initialised.
int length: an integer that contains the number of words that the Words struct contains.
Returns: Void.
*/
void initialiseClientWords(Client *client, int length){
	client->wordId = getRandomWordId(length);
	client->firstLength = words[client->wordId].firstLength;
	client->lastLength = words[client->wordId].lastLength;
	client->firstWord = calloc(client->firstLength, sizeof(int));
	client->lastWord = calloc(client->lastLength, sizeof(int));
	client->maxGuess = words[client->wordId].maxGuess;
}

/*
This function checks which of the two given clients has won the most games.
Client *clientOne: a pointer to the first client to compare.
Client *clientTwo: a pointer to the client to compare the first client against.
Returns: An integer that specifies whether clientOne or clientTwo has won the most
	games.  Or it will return the result for a draw if they are both equal.
*/
int compareClientGamesWon(Client *clientOne, Client *clientTwo){
	if(clientOne->gamesWon > clientTwo->gamesWon){
		return CLIENT_TWO;
	} else if (clientOne->gamesWon < clientTwo->gamesWon){
		return CLIENT_ONE;
	} else {
		return DRAW;
	}
}

/*
This function checks which of the two given clients has a higher game percentage.
Client *clientOne: a pointer to the first client to compare.
Client *clientTwo: a pointer to the client to compare the first client against.
Returns: An integer that specifies whether clientOne or clientTwo has a higher game
	percentage.  Or it will return the result for a draw if they are both equal.
*/
int compareClientGamesPercentage(Client *clientOne, Client *clientTwo){
	if(clientOne->percentage > clientTwo->percentage){
		return CLIENT_TWO;
	} else if (clientOne->percentage < clientTwo->percentage){
		return CLIENT_ONE;
	} else {
		return DRAW;
	}
}

/*
This function checks which of the two given clients comes first in the alphabet with regard
	to their username.
Client *clientOne: a pointer to the first client to compare.
Client *clientTwo: a pointer to the client to compare the first client against.
Returns: An integer that specifies whether clientOne or clientTwo comes first in the
	alphabet.
*/
int compareClientNames(Client *clientOne, Client *clientTwo){
	int compareResult = strcmp(clientOne->username, clientTwo->username);	
	if(compareResult < 0){
		return CLIENT_TWO;
	} else {
		return CLIENT_ONE;
	} 
}

/*
This function checks how two clients should be ranked against each other.
Client *clientOne: a pointer to the first client to compare.
Client *clientTwo: a pointer to the client to compare the first client against.
Returns: An integer that states whether clientOne is above clientTwo on the leaderboard.
*/
int compareClients(Client *clientOne, Client *clientTwo){
	int compareResult = 0;

	//compare by games won
	compareResult= compareClientGamesWon(clientOne, clientTwo);

	if(compareResult == DRAW){ //if drawn, check game percentage
		compareResult = compareClientGamesPercentage(clientOne, clientTwo);

		if(compareResult == DRAW){ //if drawn again, compare usernames
			compareResult = compareClientNames(clientOne, clientTwo);
		}
		
	}
	return compareResult;
}

/*
This function sorts the clients array in ascending order.
Returns: Void.
*/
void orderLeaderboard(){

	for (int i = 1; i < numClients; i++){
		for (int j = 0; j < numClients - i; j++){
			//if the next client is ranked higher than the current, switch the client positions
			if(compareClients(&clients[j], &clients[j+1]) == CLIENT_TWO){
				temp = clients[j];
				clients[j] = clients[j+1];
				clients[j+1] = temp;
			}	
		}
		
	}
}

/*
This function gets the number of players that have played at least one game.
Returns: An integer with the number of players that have played at least one game.
*/
int getNumberOfPlayersOnLeaderboard(){
	int players = 0;
	for(int i = 0; i < numClients; i++){
		if(clients[i].gamesPlayed != NO_GAMES_PLAYED){
			players++;
		}
	}
	return players;
}

/*
This function sends the leaderboard to the client with the given socket.  This function
	will first send the number of players that are on the leaderboard.  It will then
	send the username, games won, and games played, of each of the users that are on
	the leaderboard.
int socketId: An integer that contains the socket for the client that the leaderboard
	is being sent to.
Returns: Void.
*/
void sendClientLeaderboard(int socketId){
	uint16_t numClientsSent;
	uint16_t messageSizeSent;
	uint16_t clientValue;
	int numPlayersOnboard = getNumberOfPlayersOnLeaderboard();

	numClientsSent = htons(numPlayersOnboard);	
	send(socketId, &numClientsSent, sizeof(uint16_t), 0);
	printf("Number of Clients on the Leader Board: %d\n", numPlayersOnboard);
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

/*
This function adds a new client to the clients struct array with the specified username.
	This will reallocate the array if more than one client is already present within
	the array.
char *username: a pointer to a char array for the username that is to be used for the new client.
Returns: an integer that specifies the client index of the new client.
*/
int addClient(char *username){
	numClients++;
	if(numClients > 0){
		clients = realloc(clients, sizeof (Client) * (numClients));
	}

	initialiseClient(&clients[numClients-1], username);
	return numClients - 1;
}

/*
This function will get the index of a client based upon the given username. If no client with that
	username is found, -1 will be returned.
char *username: a pointer to a char array for the username that is to be used for the client that is
	to be found.
Returns: an integer that specifies the client index of the client with that username.  If no client
	with that username is found, -1 is returned.
*/
int getClientIndexByUsername(char *username){
	if(clients != NULL){
		for(int i = 0; i < numClients; i++){
			if(!strcmp(clients[i].username, username)){
				return i;
			}
		}
	}
	return -1;
}

/*
This function will get the index of a client based upon the given client ID. If no client with that
	ID is found, -1 will be returned.
int clientId: an integer containing the ID that is to be found.
Returns: an integer that specifies the client index of the client with that username.  If no client
	with that username is found, -1 is returned.
*/
int getClientIndexByClientId(int clientId){
	if(clients != NULL){
		for(int i = 0; i < numClients; i++){
			if(clients[i].clientId == clientId){
				return i;
			}
		}
	}
	return -1;
}

/*
This function updates the client with the given username.  It will increment the number of games played
	and add gamesWon to the client.  It will then order the leaderboard.
char *username: a pointer to a char array for the username that is to be used for the client that is
	to be updated.
int gamesWon: an integer that contains the number of gamesWon by the user.  This value is either 1 or 0.
Returns: void.
*/
void updateLeaderboardWithClient(int clientId, int gameWon){
	int clientIndex = getClientIndexByClientId(clientId);

	clients[clientIndex].gamesPlayed++;
	clients[clientIndex].gamesWon += gameWon;

	orderLeaderboard(); 
}

/*
This function manages the freeing of the dynamic memory allocated to the clients struct array.
Returns: void.
*/
void clearClients(){
	for(int i = 0; i < numClients; i++){
		free(clients[i].username);
		//if a word has been given to this client, free it.
		if(clients[i].firstWord != NULL){
			free(clients[i].firstWord);
			free(clients[i].lastWord);
		}
	}
	free(clients);
}

/*
This function decrements the readerCounter variable using the readerCounterMutex.
	It will unlock the writerMutex if the counter is 0.
Returns: void.
*/
void leaderboardReadUnlock(){
	pthread_mutex_lock(&readerCounterMutex);
	readerCounter--;
	
	if(readerCounter == NO_READING_OCCURING){
		pthread_mutex_unlock(&writerMutex);
	}
	
	pthread_mutex_unlock(&readerCounterMutex);
}

/*
This function increments the readerCounter variable using the readerCounterMutex and readerMutex.
	It will lock the writerMutex if the counter is 1.
Returns: void.
*/
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

/*
This function will lock the writerMutex and readerMutex.
Returns: void.
*/
void leaderboardWriteLock(){
	pthread_mutex_lock(&writerMutex);
	pthread_mutex_lock(&readerMutex);
}

/*
This function will unlock the writerMutex and readerMutex.
Returns: void.
*/
void leaderboardWriteUnlock(){
	pthread_mutex_unlock(&writerMutex);
	pthread_mutex_unlock(&readerMutex);
}

/*
This function initialises the reader and writer mutexes and sets the 
	readerCounter to 0.
Returns: void.
*/
void initialiseLeaderboardMutex(){
	readerCounter = 0;
	pthread_mutex_init(&writerMutex, NULL);
	pthread_mutex_init(&readerCounterMutex, NULL);
	pthread_mutex_init(&readerMutex, NULL);
	
}