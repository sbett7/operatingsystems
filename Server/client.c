#include <stdio.h> 
#include <stdlib.h> 
#include <errno.h> 
#include <string.h> 
#include <netdb.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <unistd.h>

#define MAXDATASIZE 100 /* max number of bytes we can get at once */
#define STRING_SIZE 150

#define ARRAY_SIZE 30

#define PORT_NO 54321 /* PORT Number */

#define FIRST_LENGTH 0
#define LAST_LENGTH 1
#define MAX_GUESSES 2
#define CURRENT_GUESSES 3

#define GAME_CONTINUE 0
#define GAME_LOST 1
#define GAME_WON 2

#define START_MENU 1
#define NOT_AUTHORISED 2
#define MAIN_MENU 3
#define STUB  4

#define TRUE 1
#define FALSE 0

#define LETTER_Z 'z'
#define LETTER_A 'a'

#define HANGMAN 1
#define LEADERBOARD 2
#define EXIT 3

void printMenu(int menuType);

/*
This function sends an array value for menu communication
int socket_id: the socket identifier for the server
int command: the menu command sent to the server
Returns: Void.
*/
void sendCommand(int socket_id, int command) {

	uint16_t item = 0;	

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
This function updates the word array according to the information
	retrieved from the position array
char *word: the client's current word array
int *position: the true/false position array, True means a letter matched in word
char letter: the current user guess
int length: the lenght of the words
Returns: Void.
*/
void updateWord(char *word, int *position, char letter, int length){
	for(int i = 0; i < length; i++){
		if(position[i] == TRUE){
			word[i] = letter;
		}
	}
}

/*
Simple function to check if the word has been completed yet
char *word: the client's current word array
int length: lenght of the words
Returns: int value, TRUE if game is completed.
*/
int checkWordIsDone(char *word, int length){
	for(int i = 0; i < length; i++){
		if(word[i] == '_'){
			return FALSE;
		}
	}
	return TRUE;
}

/*
This function initialises the clients character array for the start
	of the game with '_' to match unknown letters in the word
char *word: the client's current word
int firstWordLength: the value in the array for the end of the first word
	so that the words can be separated by a white space
int length: total length of the words
Returns: Void.
*/
void initWord(char *word, int firstWordLength, int length){
	for(int i = 0; i < length; i++){
		if(i == firstWordLength){
			word[i] = ' ';
		} else{
			word[i] = '_';
		}
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
Prints the client's current word as well as the number of guesses left 
	and the characters already guessed by the client
char *word: the client's current word
char *alreadyGuessed: the array that stores all of the client's guesses
int length: total length of the words
int guessedIndex: number of guesses the client has already made, so that the
	function does not print NULL/unassigned values to the console.
int maxGuesses: total number of guesses based on the words length
int incorrectInput: TRUE/FALSE value that prints an extra line if the client
	enters anything other than a character
Returns: Void.
*/
void printWordInfo(char *word, char *alreadyGuessed, int length, int guessedIndex, int maxGuesses, int incorrectInput){
	
	printf("\nGuessed Letters: ");
	int loopIndex;
	for (loopIndex = 0; loopIndex < guessedIndex; loopIndex++){
	printf("%c",alreadyGuessed[loopIndex]);
	}

	printf("\n\nNumber of guesses left: %d\n\n", maxGuesses - (guessedIndex + 1));

	for(int i = 0; i < length; i++){
		printf("%c ", word[i]);
	}
	printf("\n");
	if (incorrectInput == 1){
		printf("\n\nPlease Enter a letter, numbers are not accepted!\n\n");
	
	}
	printf("\nEnter your guess: ");		
				
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
Prints to the console based on Win/Lose condition
int completetionType: win/lose value
char *username: name of the user
Returns: Void.
*/
void gameStatusPrint(int completionType, char *username){
	printf("\nGame Over\n");
	if (completionType == GAME_LOST){
		printf("Bad Luck %s! You have run out of guesses. The Hangman got you!\n\n", username);
	} else if (completionType == GAME_WON){
		printf("Well done %s! You won this round of Hangman!\n\n", username);
	}

}

/*
Prints client menu values to console (to clean up main)
int menuType: value that decides what lines to print
Returns: Void.
*/
void printMenu(int menuType) {
	if (menuType == START_MENU){	
		printf("=============================================\n\n");
		printf("Welcome to the Online Hangman Gaming System\n\n");
		printf("=============================================\n\n");
		printf("You are required to logon with your Username and Password\n\n");
		printf("Username --> ");
	} else if (menuType == NOT_AUTHORISED){
		printf("\n\nYou Entered Either an Incorrect Username or Password - Disconnecting\n");
	} else if(menuType == MAIN_MENU){
		printf("\nPlease enter a selection:\n");
		printf("<1> Play Hangman\n");
		printf("<2> Show LeaderBoard\n");
		printf("<3> Quit\n");
		printf("\nSelection option 1-3 ->");
	} else if (menuType == STUB){
		printf("\n=============================================\n\n");
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


int main(int argc, char *argv[]) {
	int sockfd, numbytes, i=0;  
	char buf[MAXDATASIZE];
	struct hostent *he;
	struct sockaddr_in their_addr; /* connector's address information */

	if (argc != 3) {
		fprintf(stderr,"usage: client_hostname, port\n");
		exit(1);
	}

	if ((he=gethostbyname(argv[1])) == NULL) {  /* get the host info */
		herror("gethostbyname");
		exit(1);
	}

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}


	their_addr.sin_family = AF_INET;      /* host byte order */
	their_addr.sin_port = htons(atoi(argv[2]));    /* short, network byte order */
	their_addr.sin_addr = *((struct in_addr *)he->h_addr);
	bzero(&(their_addr.sin_zero), 8);     /* zero the rest of the struct */

	if (connect(sockfd, (struct sockaddr *)&their_addr, \
	sizeof(struct sockaddr)) == -1) {
		perror("connect");
		exit(1);
	}

	//fflush(stdout);
	int value = 1;
	int wordInformation[3];
	int currentGuesses = 0;
	char username[MAXDATASIZE];
	char password[MAXDATASIZE];
	int count = 0;
	char burner[16];
	printMenu(START_MENU);
	scanf("%s", username);
	printf("Password --> ");
	scanf("%s", password);
	send(sockfd, &username, MAXDATASIZE, 0);
	send(sockfd, &password, MAXDATASIZE, 0);

	int authorisation = getAuthorisationResult(sockfd);
	if (!authorisation){
		printMenu(NOT_AUTHORISED);
	}
	
	
	while(value != EXIT && authorisation){
		printMenu(MAIN_MENU);
		scanf("%d", &value);
		printf("\n\n");
		sendCommand(sockfd, value);
		
		if(value == HANGMAN){
			receiveWordInformation(sockfd, wordInformation);
			currentGuesses = 0;
			
			int length = wordInformation[FIRST_LENGTH] + wordInformation[LAST_LENGTH] + 1;
			char *word = malloc(length*sizeof(char));
			char completionType;
			char guessedLetters[MAX_GUESSES];
			int *wordPosition = malloc(length*sizeof(int));
			char letterInput;
			char guess[16];
			int incorrectInput;
			
			initWord(word, wordInformation[FIRST_LENGTH], length);
			fgets(burner, 16, stdin);
			while(currentGuesses != wordInformation[MAX_GUESSES] && !checkWordIsDone(word, length)){
				printWordInfo(word, guessedLetters, length, currentGuesses, wordInformation[MAX_GUESSES], incorrectInput);
				incorrectInput = FALSE;				
				fgets(guess, 16, stdin);
				letterInput = guess[0];

				if(letterInput <= LETTER_Z && letterInput >= LETTER_A){			
					sendGuess(sockfd, letterInput);
					getLetterPosition(sockfd, wordPosition, length);
					updateWord(word, wordPosition, letterInput, length);
					guessedLetters[currentGuesses] = letterInput;
					currentGuesses++;
				} else {
					incorrectInput = TRUE;			
				}
				printMenu(STUB);
				
			}
			
			
			if(checkWordIsDone(word,length)){
				completionType = GAME_WON;
			} else if (!checkWordIsDone(word,length)){
				completionType = GAME_LOST;
			} else {
				printf("Error in checkWordIsDone");
			}
			sendGameStatus(sockfd, completionType);
			gameStatusPrint(completionType, username);
			free(word);
			free(wordPosition);
			
		} else if (value == LEADERBOARD){
			receivePrintLeaderboard(sockfd);
		}		
	}
	close(sockfd);

	return 0;
}