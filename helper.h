#ifndef _HELPER_H_
#define _HELPER_H_
#include "list.h"
#include <netdb.h>
#include <pthread.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>		
#include <unistd.h>		
#include <netinet/in.h>
#include <arpa/inet.h>

//might need more funcs
int sendValues(struct addrinfo *values, int *trace, pthread_mutex_t *input);
void closeLocalClient();
int gotValues(int *trace2, pthread_mutex_t* input2);
void killProgram();
void closeRemoteClient();
void endProgram();

#endif