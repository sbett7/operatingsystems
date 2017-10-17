#define _GNU_SOURCE
#include "ClientCommunication.h"

#include <stdio.h> 
#include <stdlib.h> 
#include <errno.h> 
#include <string.h> 
#include <netdb.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <unistd.h>
#include <string.h>
#include <signal.h>

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
#define PASSWORD 5
#define EXIT_PROGRAM 6

#define TRUE 1
#define FALSE 0

#define LETTER_Z 'z'
#define LETTER_A 'a'

#define HANGMAN 1
#define LEADERBOARD 2
#define EXIT 3

#define UPPER_A 'A'
#define UPPER_Z 'Z'
#define TO_LOWER_CASE 32
#define ALPHABET 26

#define INPUT_BUFFER_SIZE 16
#define GUESS 0
#define SPACE 1

int socketId = 0;
int *wordPosition;
char *word;
char *guessedLetters;

void exitProgram();


/*
This function converts a letter to its lowercase form if it is in uppercase.
char letter: the letter that is to be converted.
Returns: the converted letter as a char.
*/
char toLowerCase(char letter){
	if(letter <= UPPER_A && letter >= UPPER_Z){
		printf("letter: %c", letter);
		letter += TO_LOWER_CASE;
		printf("new letter: %c", letter);	
	}
	return letter;
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
	word[length] = '\0';
	for(int i = 0; i < length; i++){
		if(i == firstWordLength){
			word[i] = ' ';
		} else{
			word[i] = '_';
		}
	}
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
	
	for (int loopIndex = 0; loopIndex < guessedIndex; loopIndex++){
		printf("%c ",alreadyGuessed[loopIndex]);
	}

	printf("\n\nNumber of guesses left: %d\n\n", maxGuesses - guessedIndex);

	for(int i = 0; i < length; i++){
		printf("%c ", word[i]);
	}
	printf("\n");
	if (incorrectInput == 1){
		printf("\n\nPlease Enter a letter in lowercase, numbers and uppercase letters are not accepted!\n\n");
	
	}
	printf("\nEnter your guess: ");		
				
}

/*
Prints to the console based on Win/Lose condition
int completetionType: win/lose value
char *username: name of the user
Returns: Void.
*/
void gameStatusPrint(int completionType, char *user, char *word){
	printf("\nGame Over\n");
	if (completionType == GAME_LOST){
		printf("Bad Luck %s! You have run out of guesses. The Hangman got you!\n\n", user);
	} else if (completionType == GAME_WON){
		printf("Well done %s! You won this round of Hangman!\n\n", user);
		printf("The word was:	 %s\n\n", word);
	}

}

/*
Prints client menu values to console (to clean up main)
int menuType: value that decides what lines to print
Returns: Void.
*/
void printMenu(int menuType) {

	switch(menuType){
		case START_MENU:
			printf("=============================================\n\n");
			printf("Welcome to the Online Hangman Gaming System\n\n");
			printf("=============================================\n\n");
			printf("You are required to logon with your Username and Password\n\n");
			printf("Username --> ");
			break;
		case PASSWORD:
			printf("Password --> ");
			break;
		case NOT_AUTHORISED:
			printf("\n\nYou have entered an incorrect Username or Password - Disconnecting\n");
			break;
		case MAIN_MENU:
			printf("\nPlease enter a selection:\n");
			printf("<1> Play Hangman\n");
			printf("<2> Show LeaderBoard\n");
			printf("<3> Quit\n");
			printf("\nSelection option 1-3 --> ");
			break;
		case STUB:
			printf("\n=============================================\n\n");
			break;
		case EXIT_PROGRAM:
			printf("\n=============================================\n\n");
			printf("Closing down Hangman Client!\n\nThanks for playing!\n");
			printf("\n=============================================\n\n");
			break;
	}
}

/*
Checks whether the game was won or lost and sends the result to the server.
	Prints the result to the command line console.
char *word: the word that is to be checked.
int length: the length of the word that is to be checked.
Returns: void.
*/
void checkGameCompletion(char *word, int length){
	int completionType;
	int isWordDone = checkWordIsDone(word,length);
	if(isWordDone){
		completionType = GAME_WON;
	} else {
		completionType = GAME_LOST;
	}
	sendGameStatus(socketId, completionType);
	gameStatusPrint(completionType, username, word);
}

/*
reallocates the memory for the hangman data structures to fit the
	word that has been given to the client.
int length: the required length for the wordPosition and word arrays.
int maxGuess: the required length for the guessedLetters array.
Returns: void
*/
void setWordDataSize(int length, int maxGuess){
	wordPosition = realloc(wordPosition, length * sizeof(int));
	word = realloc(word, length * sizeof(char));
	if(word == NULL){
		printf("failed to allocate word\n");	
	}
	guessedLetters = realloc(guessedLetters , maxGuess * sizeof(char));
}

/*
Main game logic for the hangman game.  Sends a client guess to the server and 
	checks if it matches any entry in the word array.  If the user runs out of
	guesses or the word is completed, the game is ended and the completion type
	is checked.
Returns: void
*/
void playHangmanGame(){
	int wordInformation[WORD_INFORMATION_ARRAY_SIZE];
	int length = 0;
	int currentGuesses = 0;
	int incorrectInput = FALSE;

	char guessedLetters[MAX_GUESSES];
	char letterInput;
	char guess[INPUT_BUFFER_SIZE];
	char burner[INPUT_BUFFER_SIZE];

	receiveWordInformation(socketId, wordInformation);
	length = wordInformation[FIRST_LENGTH] + wordInformation[LAST_LENGTH] + SPACE;
	
	setWordDataSize(length, wordInformation[MAX_GUESSES]);	
	initWord(word, wordInformation[FIRST_LENGTH], length);

	fgets(burner, INPUT_BUFFER_SIZE, stdin);
	
	while(currentGuesses != wordInformation[MAX_GUESSES] && !checkWordIsDone(word, length)){
		printWordInfo(word, guessedLetters, length, currentGuesses, wordInformation[MAX_GUESSES], incorrectInput);
		incorrectInput = FALSE;				
		fgets(guess, INPUT_BUFFER_SIZE, stdin);
		letterInput = guess[GUESS];

		//if letter is between A-Z or a-z, send guess to server and update word
		//else set incorrectInput flag
		if((letterInput <= LETTER_Z && letterInput >= LETTER_A)){
			sendGuess(socketId, letterInput);
			getLetterPosition(socketId, wordPosition, length);
			updateWord(word, wordPosition, letterInput, length);
			guessedLetters[currentGuesses] = letterInput;
			currentGuesses++;
		} else {
			incorrectInput = TRUE;			
		}
		printMenu(STUB);
		
	}
	
	checkGameCompletion(word, length);
}

/*
Prompts the user for the command that they wish to perform and sends it to
	the server.
Returns: An integer with the command type.
*/
int getCommand(){
	int command = 0;
	printMenu(MAIN_MENU);
	scanf("%d", &command);
	printf("\n\n");
	sendCommand(socketId, command);
	return command;
}

/*
Performs the specified command that the specified command that the user
	has provided.
int command: an integer with the user specified command that is to be 
	performed.
*/
void performCommand(int command){
	switch(command){
		case HANGMAN:
			playHangmanGame();
			break;
		case LEADERBOARD:
			receivePrintLeaderboard(socketId);
			break;

		case EXIT:
			
			break;
	}
}

/*
Retrieves the user's credentials and sends it to the server.
	The authentication result from the server is retrieved,
	and will either exit the program if the user is not authorised,
	or continue.
Returns: void
*/
void authenticateUser(){
	char password[STRING_SIZE];
	int authenticationResult = 0;

	printMenu(START_MENU);
	scanf("%s", username);
	
	printMenu(PASSWORD);
	scanf("%s", password);

	send(socketId, &username, STRING_SIZE, 0);
	send(socketId, &password, STRING_SIZE, 0);
	
	authenticationResult = getAuthorisationResult(socketId);

	if (!authenticationResult){
		printMenu(NOT_AUTHORISED);
		exitProgram();
	}
}
/*
Handler for exiting the client program. Frees dynamic memory
	and closes the connection.
Returns: void
*/
void exitProgram(){
	printMenu(EXIT_PROGRAM);
	free(wordPosition);
	free(word);
	close(socketId);
	exit(0);
}

/*
Initialises the data structures for the client program.
Returns: void
*/
void intialiseClientDataStructures(){
	wordPosition= malloc(ALPHABET * sizeof(int));
	word = malloc(ALPHABET * sizeof(char));
	guessedLetters = malloc(ALPHABET * sizeof(char));
	
}

int main(int argc, char *argv[]) {

	int command = 0;
	
	signal(SIGINT, exitProgram);
	
	struct hostent *he;
	struct sockaddr_in their_addr;

	if (argc != 3) {
		fprintf(stderr,"usage: client_hostname, port\n");
		exit(1);
	}

	if ((he=gethostbyname(argv[1])) == NULL) {
		herror("gethostbyname");
		exit(1);
	}

	if ((socketId = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	their_addr.sin_family = AF_INET;
	their_addr.sin_port = htons(atoi(argv[2]));
	their_addr.sin_addr = *((struct in_addr *)he->h_addr);
	bzero(&(their_addr.sin_zero), 8);     


	if (connect(socketId, (struct sockaddr *)&their_addr, \
	sizeof(struct sockaddr)) == -1) {
		perror("connect");
		exit(1);
	}

	authenticateUser(socketId);
	
	//game controller
	while(command != EXIT){
		command = getCommand();
		performCommand(command);
	}
	exitProgram();

	return 0;
}