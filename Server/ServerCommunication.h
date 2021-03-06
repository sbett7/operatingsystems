#ifndef __SERVER_COMMUNICATION_H__
#define __SERVER_COMMUNICATION_H__

#include <arpa/inet.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <errno.h> 
#include <string.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <sys/wait.h> 
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#include "TextProcessor.h"
#include "ClientLeaderboardFunctions.h"

#define MAX_DATA_SIZE 100 

#define SPACE 0

void getUserCredentials(int socketId, char *user, char *password);

void sendAuthenticationResult(int socketId, int authenticationResult);

int getCommand(int socketIdentifier);

void sendWordDetails(int socketId, int wordDetails[3]);

char getClientGuess(int socketId);

void sendLetterPositions(int socketId, Client *client);

#endif