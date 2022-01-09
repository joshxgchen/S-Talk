#include <stdio.h>
#include "helper.h"
#include "list.h"

int main(int argc, char* args[]){
    //initialization
    pthread_mutex_t testing = PTHREAD_MUTEX_INITIALIZER;
    int status, rv, tempInt;
    struct addrinfo hints, *host, *client, *temp;
    memset(&hints, 0, sizeof hints);    
    hints.ai_family = AF_UNSPEC;        
    hints.ai_socktype = SOCK_DGRAM;     
    hints.ai_flags = AI_PASSIVE; //autofill IP, from Harinder's manual  

    if(argc < 4){   //total # of arguments
        printf("Sorry, did you enter in the IP Address properly?\n");
        return -1;
    }

    if ((status = getaddrinfo(NULL, args[1], &hints, &host)) != 0) {
        printf("getaddrinfo error found, returning -1.\n");
        return -1;
    }

    for(temp = host; temp != NULL; temp = temp->ai_next) { //go next
        if ((tempInt = socket(temp->ai_family, temp->ai_socktype,
                temp->ai_protocol)) == -1) {
            printf("Socket creation failed. ");
            continue;
        }
        if (bind(tempInt, temp->ai_addr, temp->ai_addrlen) == -1) {
            printf("Binding creation failed. ");;
            close(tempInt);
            continue;
        }
        break;
    }

    if(temp == NULL){ //if NULL then rip
        printf("Socket creation failed! Sorry!\n");
        return -1;
    }

    if (tempInt == -1 ){ //if -1, then rip
        printf("Socket creation failed! Sorry!\n");
        return -1;
    }
    if ((rv = getaddrinfo(args[2], args[3], &hints, &client)) != 0) {
        printf("getaddrinfo error found, returning -1.\n");
        return -1;
    }
    
    if(gotValues(&tempInt, &testing) == -1){
        printf("pThread creation failed\n");
        //just close local value
        closeRemoteClient();
        return -1;
    }
    if(sendValues(client, &tempInt, &testing) == -1){
        printf("pThread creation failed\n");
        //kill both values, if we failed at sending then we have to close both
        closeRemoteClient(); //kill all the ptthreads
        closeLocalClient(); //kill pthreads
        return -1;
    }
    killProgram();  //end pthreads
    freeaddrinfo(host);    
    freeaddrinfo(client);  
    pthread_mutex_destroy(&testing); //don't need, free
    return 0;
}
