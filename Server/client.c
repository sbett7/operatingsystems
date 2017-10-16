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

#define TRUE 1
#define FALSE 0



void Send_Array_Data(int socket_id, int command) {

	uint16_t item = 0;	

	item = htons(command);
	send(socket_id, &item, sizeof(uint16_t), 0);
	

}

void receiveWordInformation(int socketId, int wordInformation[3]){
	uint16_t value;
	int retValue;

	for(int i = 0; i < 3; i++){
		recv(socketId, &value, sizeof(uint16_t), 0);
		wordInformation[i] = ntohs(value);
	}
}

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

			printf("Player - %s\nGames Played - %d\nGames Won - %d\n\n", username, gamesPlayed, gamesWon);
		}
	}
}

void getLetterPosition(int socketId, int *position, int length){
	
	uint16_t value;
	for(int i = 0; i < length; i++){
		recv(socketId, &value, sizeof(uint16_t), 0);
		position[i] = ntohs(value);
	}
}

void updateWord(char *word, int *position, char letter, int length){
	for(int i = 0; i < length; i++){
		if(position[i] == TRUE){
			word[i] = letter;
		}
	}
}

int checkWordIsDone(char *word, int length){
	for(int i = 0; i < length; i++){
		if(word[i] == '_'){
			return FALSE;
		}
	}
	return TRUE;
}

void initWord(char *word, int firstWordLength, int length){
	for(int i = 0; i < length; i++){
		if(i == firstWordLength){
			word[i] = ' ';
		} else{
			word[i] = '_';
		}
	}
}

void sendGuess(int socketId, char letter){
	int guess = (int) letter;
	uint16_t value = htons(guess);
	if(send(socketId, &value, sizeof(uint16_t), 0) < 0){
		printf("error\n");
	}
	send(socketId, &letter, sizeof(char), 0);
}

void printWordInfo(char *word, char *alreadyGuessed, int length, int guessedIndex, int maxGuesses){
	
	printf("Guessed Letters: ");
	int loopIndex;
	for (loopIndex = 0; loopIndex < guessedIndex; loopIndex++){
	printf("%c",alreadyGuessed[loopIndex]);
	}

	printf("\n\nNumber of guesses left: %d\n\n", maxGuesses - (guessedIndex + 1));

	for(int i = 0; i < length; i++){
		printf("%c ", word[i]);
	}
	printf("\n");
}

void sendGameStatus(int socketId, int status){

	uint16_t value = htons(status);
	if(send(socketId, &value, sizeof(uint16_t), 0) < 0){
		printf("error\n");
	}
}

void sendCredentials(int socketId, char *username, char *password){
	send(socketId, &username, MAXDATASIZE, 0);
	send(socketId, &password, MAXDATASIZE, 0);
}

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
	printf("=============================================\n\n");
	printf("Welcome to the Online Hangman Gaming System\n\n");
	printf("=============================================\n\n");
	printf("You are required to logon with your Username and Password\n\n");
	printf("Username -->");
	scanf("%s", username);
	printf("Password -->");
	scanf("%s", password);
	send(sockfd, &username, MAXDATASIZE, 0);
	send(sockfd, &password, MAXDATASIZE, 0);

	int authorisation = getAuthorisationResult(sockfd);
	if (!authorisation){
		printf("\n\nYou Entered Either an Incorrect Username or Password - Disconnecting\n");
	}
	
	
	while(value != 3 && authorisation){

		printf("\nPlease enter a selection:\n");
		printf("<1> Play Hangman\n");
		printf("<2> Show LeaderBoard\n");
		printf("<3> Quit\n");
		printf("\nSelection option 1-3 ->");
		scanf("%d", &value);
		printf("command given was: %d\n",value);
		printf("\n\n");
		Send_Array_Data(sockfd, value);
		

		if(value == 1){
			
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
				
				printf("\n");
				//printf("The word is done: %d\n",checkWordIsDone(word, length));
				printWordInfo(word, guessedLetters, length, currentGuesses, wordInformation[MAX_GUESSES]);
				if (incorrectInput == 1){
				printf("\n\nPlease Enter a letter, numbers are not accepted!\n\n");
				incorrectInput = 0;
				printf("\nEnter your guess: ");
				} else {
				printf("\nEnter your guess: ");	
				}			
									
				fgets(guess, 16, stdin);
				letterInput = guess[0];

				if(letterInput <= 'z' && letterInput >= 'a'){			
					sendGuess(sockfd, letterInput);
					getLetterPosition(sockfd, wordPosition, length);
					updateWord(word, wordPosition, letterInput, length);
					guessedLetters[currentGuesses] = letterInput;
					currentGuesses++;
				} else {
				incorrectInput = 1;			
				}
				printf("\n=============================================\n\n");
				
			}
			
			
			if(checkWordIsDone(word,length)){
				completionType = GAME_WON;
			} else{
				completionType = GAME_LOST;
			}
			sendGameStatus(sockfd, completionType);
			printf("\nGame Over\n");
			if (completionType == GAME_LOST){
				printf("Bad Luck %s! You have run out of guesses. The Hangman got you!\n\n", username);
				
			} else if (completionType == GAME_WON){
			printf("Well done %s! You won this round of Hangman!\n\n", username);
			}
			free(word);
			free(wordPosition);
			
		} else if (value == 2){
			receivePrintLeaderboard(sockfd);
		}		
	}
	close(sockfd);

	return 0;
}

