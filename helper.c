#include <stdio.h>
#include "helper.h"
#include "list.h"

void tempFreeFN(void* item){
  return; 
}
//implementation
static List *infoGiven, *infoGotten;                                              
static int *counter, *counter2;  
static struct addrinfo *local;
static pthread_t mechThread, monitorThread;                                       
static pthread_mutex_t dataLock, endLock, infoLock = PTHREAD_MUTEX_INITIALIZER;  
static pthread_cond_t condLock, endLink, intelSync = PTHREAD_COND_INITIALIZER;      
static pthread_mutex_t *tempMech, *tempMech2;                                        
static char *inputVal, *outputVal, *msgGot, *msgSent;	
static pthread_t finished, hostThread, clientThread;		
struct sockaddr_storage savedVal;

static void *clientSending(){
    int totalLength;
    int critSectionCheck = 1;
    char tempChar[46];
    inet_ntop(local->ai_family, &(((struct sockaddr_in*)(local->ai_addr))->sin_addr),tempChar,sizeof(tempChar));
    printf("Sending to %s:%d\n", tempChar, ntohs(((struct sockaddr_in*)(local->ai_addr))->sin_port));
    while(critSectionCheck){     
        int tempCounting;                             
        pthread_mutex_lock(tempMech);
        totalLength = List_count(infoGiven);
        pthread_mutex_unlock(tempMech);
        if(totalLength == 0){
            pthread_mutex_lock(&dataLock);
            pthread_cond_wait(&condLock, &dataLock);
            pthread_mutex_unlock(&dataLock);
        }
        pthread_mutex_lock(tempMech);
        outputVal = (char *)List_trim(infoGiven);
        pthread_mutex_unlock(tempMech);        
        critSectionCheck = strcmp(outputVal, "!\n"); 
        if ((tempCounting=sendto(*counter, outputVal, strlen(outputVal), 0,local->ai_addr, local->ai_addrlen)) == -1) {
          perror("Sending data failed.");
        }
        pthread_mutex_lock(&dataLock);
        free(outputVal);
        outputVal = NULL;
        pthread_mutex_unlock(&dataLock);
    }
    return 0;
}

static void *localSending(){
    char charArray[4000];
    memset(charArray, 0, 4000);
    char* p_charArray = NULL;

    int critcalSectionLocker = 1;
    while(critcalSectionLocker){
        p_charArray = fgets(charArray, 4000, stdin);
        if(p_charArray == NULL){
            break;
        }
        critcalSectionLocker = strcmp(charArray, "!\n");                 
        inputVal = malloc(strlen(charArray)+1);
        if(inputVal==NULL){
          printf("No more memory.\n");
          break;
        }
        strcpy(inputVal, charArray); 
        pthread_mutex_lock(&dataLock);
        pthread_mutex_lock(tempMech);
        if(List_prepend(infoGiven, inputVal) == true){
          printf("You're over the character array limit.\n");
        }
        pthread_mutex_unlock(tempMech);
        pthread_cond_signal(&condLock);
        inputVal = NULL;
        pthread_mutex_unlock(&dataLock);
        memset(charArray, 0, strlen(charArray)+1); 
    }
    sleep(1);	
    endProgram();   
    return 0;
}
void closeRemoteClient(){  
	pthread_cancel(hostThread);
	pthread_cancel(clientThread);
	pthread_join(hostThread, NULL);
	pthread_join(clientThread, NULL);
	
	if(msgGot)
		free(msgGot);
	msgGot = NULL;

	if(msgSent)
		free(msgSent);
	msgSent = NULL;
	close(*counter2);
	
	pthread_mutex_lock(tempMech2);	
	if(infoGotten){
		List_free(infoGotten, tempFreeFN);
		infoGotten = NULL;
	}
	pthread_mutex_unlock(tempMech2);
	pthread_cond_destroy(&intelSync);
	pthread_mutex_destroy(&infoLock);
}

void closeLocalClient(){
    pthread_cancel(mechThread);
    pthread_cancel(monitorThread);
    pthread_join(monitorThread, NULL);
    pthread_join(mechThread, NULL);
    if(inputVal){
	    free(inputVal); //deallocate memory
    }
    inputVal = NULL;
    if(outputVal){
      free(outputVal);
    }
    outputVal = NULL;
    close(*counter);
    pthread_mutex_lock(tempMech); 
    if(infoGiven){
      List_free(infoGiven, tempFreeFN);
      infoGiven = NULL;
    }
    pthread_mutex_unlock(tempMech);
    pthread_cond_destroy(&condLock);
    pthread_mutex_destroy(&dataLock);
}


static void* quitExecution(){  
    closeRemoteClient();         
    closeLocalClient();   
    return 0;
}

void killProgram(){       
    pthread_mutex_lock(&endLock);
        pthread_cond_wait(&endLink, &endLock);  
    pthread_mutex_unlock(&endLock);
    if(pthread_create(&finished, NULL, quitExecution, NULL)){
        printf("Application failed to exit smoothly\n");
    }

    pthread_join(finished, NULL);
    pthread_cond_destroy(&endLink);
    pthread_mutex_destroy(&endLock);
}

int sendValues(struct addrinfo *values, int *trace, pthread_mutex_t *input){
    local = (struct addrinfo *)values;
    tempMech = input;
    counter = trace;
    if(pthread_create(&mechThread, NULL, localSending, NULL)){
		  printf("Failed to make pThread 1.\n");
		  return -1;
	  }
    if(pthread_create(&monitorThread, NULL, clientSending, NULL)){
		  printf("Failed to make pThread 2.\n");
		  return -1;
	  }
    infoGiven = List_create(); 
    return 0;
}



static void *remoteGetting(){
	socklen_t tempor = sizeof(savedVal);
	char charArray2[4000];
	int tempCounting2;
	int critSectionCheck2 = 1;
	while (critSectionCheck2) {
    int whatsBigger;
    if (tempCounting2 < 4000){
      whatsBigger = tempCounting2;
    }
    else{
      whatsBigger = 4000 - 1;  
    } 
		if((tempCounting2 = recvfrom(*counter2, charArray2, 4000, 0, (struct sockaddr *)&savedVal, &tempor)) == -1) {
			perror("Revcfrom error was found.");
			break;	
		}					
		charArray2[whatsBigger] = 0;
		msgGot = malloc(whatsBigger+1); 	
    strcpy(msgGot, charArray2); 		
		memset(charArray2, 0, tempCounting2); 		
		critSectionCheck2 = strcmp(msgGot, "!\n"); 		
		pthread_mutex_lock(&infoLock);
		pthread_mutex_lock(tempMech2);
		if(List_prepend(infoGotten, msgGot)){
			printf("You're over the character array limit.\n");
			free(msgGot);
		}
		pthread_mutex_unlock(tempMech2);
		msgGot = NULL;	
		pthread_cond_signal(&intelSync);
		pthread_mutex_unlock(&infoLock);
	}
	sleep(1);	
	endProgram();	
	return 0;	
}


static void *clientGettor()
{
	int totalLength2;
	int critSectionCheck2 = 1;
	while (critSectionCheck2) {
		pthread_mutex_lock(tempMech2);
			totalLength2 = List_count(infoGotten);
		pthread_mutex_unlock(tempMech2);
		if(totalLength2 == 0){				
			pthread_mutex_lock(&infoLock);
				pthread_cond_wait(&intelSync, &infoLock);
			pthread_mutex_unlock(&infoLock);
		}
		pthread_mutex_lock(tempMech2);
		msgSent = (char *)List_trim(infoGotten);  
		pthread_mutex_unlock(tempMech2);
		fputs(msgSent,stdout);			
		critSectionCheck2 = strcmp(msgSent, "!\n");		
		pthread_mutex_lock(&infoLock);
		free(msgSent);
		msgSent = NULL;
		pthread_mutex_unlock(&infoLock);	
	}
	return 0;
}

int gotValues(int *trace2, pthread_mutex_t* input2){
	tempMech2 = input2;
	counter2 = trace2;
	infoGotten = List_create();
	if(pthread_create(&hostThread, NULL, remoteGetting, NULL)){		
		printf("Failed to make pThread 3.\n");
		return -1;
	}
	if(pthread_create(&clientThread, NULL, clientGettor, NULL)){
		printf("Failed to make pThread 4.\n");
		return -1;
	}
	return 0;
}

void endProgram(){                                 
    pthread_mutex_lock(&endLock);
        pthread_cond_signal(&endLink);             
    pthread_mutex_unlock(&endLock);
}
