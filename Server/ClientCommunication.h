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

#define MAXDATASIZE 100 /* max number of bytes we can get at once */
#define STRING_SIZE 150
#define ARRAY_SIZE 30
#define PORT_NO 54321 /* PORT Number */
#define TRUE 1
#define FALSE 0

int command;
int wordinformation[3];
int *position;
int length;
char username[MAXDATASIZE];
char password[MAXDATASIZE];
char letter;
int status;

void sendCommand(int socket_id, int command);

void receiveWordInformation(int socketId, int wordInformation[3]);

void receivePrintLeaderboard(int socketId);

void getLetterPosition(int socketId, int *position, int length);

void sendGuess(int socketId, char letter);

void sendGameStatus(int socketId, int status);

void sendCredentials(int socketId, char *username, char *password);

int getAuthorisationResult(int socketId);

#endif
