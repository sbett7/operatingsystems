#ifndef __CLIENTCOMMUNICATION_H__
#define __CLIENTCOMMUNICATION_H__

#include <stdio.h> 
#include <stdlib.h> 
#include <errno.h> 
#include <string.h> 
#include <netdb.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <unistd.h>

#define STRING_SIZE 100

#define TRUE 1
#define FALSE 0

#define STUB  4

#define WORD_INFORMATION_ARRAY_SIZE 3

int command;

int *position;
int length;
char username[STRING_SIZE];
char password[STRING_SIZE];
char letter;
int status;

void sendCommand(int socket_id, int command);

void receiveWordInformation(int socketId, int wordInformation[3]);

void receivePrintLeaderboard(int socketId);

void getLetterPosition(int socketId, int *position, int length);

void sendGuess(int socketId, char letter);

void sendGameStatus(int socketId, int status);

void sendCredentials(int socketId, char *user, char *password);

int getAuthorisationResult(int socketId);

void printStub();

#endif