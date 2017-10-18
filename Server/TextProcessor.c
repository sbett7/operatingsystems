#define _GNU_SOURCE
#include "TextProcessor.h"

/*
This function gets the length of the file that has had its name given
	as a parameter.
char *fileName: a pointer to a char array that contains the name of the
	file that is to be opened.
Returns: An integer that contains the length of the file.
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

/*
This function finds the length of the char array.
char *word: a pointer to a char array that contains word that
	is to be measured.
Returns: An integer that contains the length of the char array.
*/
int getWordLength(char* word){
	for(int i = 0; i < MAX_LINE; i++){
		if(word[i] == '\0'){
			return i;
		}
	}
	return 0;
}

/*
This function finds the length of the char array.
char *word: a pointer to a char array that is to be used to copy the original word.
char *original: a pointer to a char array that contains the word that is
	to be copied.
int length: An integer that specifies the length of the word to copy.
Returns: void.
*/
void copyWord(char *copy, char *original, int length){
	for(int i=0; i <= length;i++){
		copy[i] = original[i];
	}
}

/*
This function separates two words that are separated by a special character.
char *delimiter: a pointer to a char array that contains the special character to
	separate by.
char *str: a pointer to a char array that is to be separated by the delimiter.
char *firstWord: a pointer to a char array where the first word is to be copied to.
char *secondWord: a pointer to a char array where the second word is to be copied to.
Returns: void.
*/
void separateWords(char * delimiter, char *str, char *firstWord, char *secondWord){
	strcpy(firstWord, strtok(str, delimiter));
	strcpy(secondWord, strtok(NULL, "\n"));
}

/*
This function reads the words in the hangman_text file into the Words struct.
	It will store the length of each word and the maximum number of guesses
	for each entry within the Words struct.
Returns: void.
*/
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
		
		//get words
		separateWords(",", str, firstWord, secondWord);
		
		//get the length of each word
		words[i].firstLength = getWordLength(firstWord);
		words[i].lastLength = getWordLength(secondWord);

		// allocate space for the new words
		words[i].firstWord = malloc(words[i].firstLength * sizeof(char));
		words[i].lastWord = malloc(words[i].lastLength * sizeof(char));
		
		//copy words to the words struct
		copyWord(words[i].firstWord, firstWord, words[i].firstLength);
		copyWord(words[i].lastWord, secondWord, words[i].lastLength);

		//calculate maximum guesses for this entry
		words[i].maxGuess = getMaxGuesses(words[i].firstLength, words[i].lastLength);
		printf("Word %d: %s %s\n", i + 1, words[i].firstWord, words[i].lastWord);
		i++; //increment counter
		
	}
	free(firstWord);
	free(secondWord);
	fclose(fp);
	return 1;	
}

/*
This function calculates the maximum number of guesses based off the minimum
	value of an equation and the length of the alphabet.
	The equation is LENGTH = FIRST_LENGTH + LAST_LENGTH + 10.
	Thus maximum guesses is MIN(LENGTH, 26)
int firstWord: an integer that contains the length of the first word.
int secondWord: an integer that contains the length of the second word.
Returns: An integer with the maximum number of guesses.
*/
int getMaxGuesses(int firstWord, int secondWord){
	int length = firstWord + secondWord + MIN_GUESSES;	
	return min(length, MAX_GUESSES);
}

/*
This function manages the freeing of the dynamic space in the Words struct.
Returns: Void.
*/
void clearWords(){
	for(int i = 0; i < (int)(sizeof(words)/sizeof(Word)); i++){
		free(words[i].firstWord);
		free(words[i].lastWord);
	}
	free(words);
}

/*
This function gets a random word index using the length of the Words struct.
int firstWord: an integer with the length of the Words struct.
Returns: An integer with the randomised index for a word.
*/
int getRandomWordId(int length){
	return random() % length;
}

/*
This function stores the credentials within the Authentication text file
	within the accounts struct.
Returns: void.
*/
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
		printf("Could not open file");
	}

	while(fgets(str, MAX_LINE, fp) != NULL){
		// check if the line is reading the header line in Authentication.txt
		if(i > 0){
			//separate by tab/new line/ space
			separateWords(" \r\t\n", str, userTemp, passTemp);
			//if a tab is still present at start of word, trim again
			if(passTemp[0] == '\t');{
				strcpy(passTemp, strtok(passTemp, "\t"));
			}
			
			//get length of credentials and store them in the struct
			userLength = getWordLength(userTemp);
			passLength = getWordLength(passTemp);
			accounts[i-1].username = malloc(userLength * sizeof(char)); // i-1 due to header line in file
			accounts[i-1].password = malloc(passLength * sizeof(char));

			copyWord(accounts[i-1].username, userTemp, userLength);
			copyWord(accounts[i-1].password, passTemp, passLength);
		}
		i++; // increment counter
	}
	free(userTemp);
	free(passTemp);
	fclose(fp);
}

/*
This function checks a set of credentials against all of the credentials within the accounts struct.
char *username: a pointer to a char array that contains the username to check.
char *password: a pointer to a char array that contains the password to check.
int length: the length of the accounts struct.
Returns: An integer that is 1(TRUE) if a match is found, or  0(FALSE) if no match is found.
*/
int checkCredentials(char *username, char *password, int length){
	int usernameLength = getWordLength(username);
	int passwordLength = getWordLength(password);

	int accountUserLength;
	int accountPassLength;
	
	for(int i = 0; i < length; i++){
		accountUserLength = getWordLength(accounts[i].username);
		accountPassLength = getWordLength(accounts[i].password);

		if(checkStringsEqual(username, accounts[i].username, usernameLength, accountUserLength) &&
		    		checkStringsEqual(password, accounts[i].password, passwordLength, accountPassLength)){
			return TRUE;
		}
	}
	return FALSE;
}

/*
This function manages the freeing of the dynamic memory allocated to the accounts struct.
Returns: Void.
*/
void clearAccounts(){
	for(int i = 0; i < numAccounts; i++){
		free(accounts[i].username);
		free(accounts[i].password);
	}
	free(accounts);
}

/*
This function checks if two char arrays are equal.
char *stringOne: a pointer to a char array of the first string
char *stringTwo: a pointer to a char array of the second string.
int lengthOne: an integer that contains the length of stringOne.
int lengthOne: an integer that contains the length of stringTwo.
Returns: An integer that is 1(TRUE) if the strings are equal, else 0(FALSE).
*/
int checkStringsEqual(char *stringOne, char *stringTwo, int lengthOne, int lengthTwo){
	
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

/*
This function sets the seed of the random number generator based on the current time.
	This ensures that the sequence of random numbers is different each time that
	the program is started.
*/
void initialiseRandomNumberGenerator(){
	srand(time(NULL));
}

