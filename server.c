#include<unistd.h>
#include<stdin.h>
#include<stdlib.h>

#include "util.h"
#include "login.h"


bool waitLogin(int inFd, int outFd, char* username, UserRights* rights);
char** readCommand(int inFd, int outFd);
void execCommand(char** command, int outFd, char* username, UserRights rights);

void serverMain(int inFd, int outFd){
    UserRights userRights = 0;
    char username[256];

    if (!waitLogin(inFd, outFd, username, &UserRights)){
        return;
    }

    char welcomeStr[300];
    snprintf(welcomeStr, 300, "Welcome %s!\nWaiting for commands.", username);
    writeSizedStr(outFd, welcomeStr);

    char** command = readCommand(inFd, outFd);
    while(command != NULL){
        execCommand(command, outFd, username, rights);

        free2d(command);
        command = readCommand(inFd, outFd);
    }
}


bool waitLogin(int inFd, int outFd, char* username, UserRights* rights){
    char password[256];

    while(True){
        int readResult = readSizedStr(inFd, username, 256);
        if (readResult < 0){
            perror("Error reading username.");
            return false;
        }
        if (readResult == 0){
            printf("Login aborted, client shutting down.");
            return false;
        }
        
        readResult = readSizedStr(inFd, password, 256);
        if (readResult < 0){
            perror("Error reading password.");
            return false;
        }
        if (readResult == 0){
            printf("Login aborted, client shutting down.");
            return false;
        }

        if (checkLogin(username, password, rights)){
            return true;
        }
    }

    //Never gets here, but aaaanyway.
    return false;
}


char** readCommand(int inFd, int outFd){
    char** result = NULL;

    int nrArgs;
    int bytesRead = read(inFd, &nrArgs, 4);
    if (bytesRead != 4){
        return NULL;
    }

    char** result = malloc(4 * nrArgs);
    if (result == NULL){
        return NULL;
    }

    for (int i = 0; i < nrArgs; i++){
        char* arg;
        if (allocReadSizedStr(fd, &arg) < 0){
            free2d(result, i);
            return NULL;
        }
        result[i] = arg;
    }

    return result;
}

void execCommand(char** command, int outFd, char* username, UserRights rights){
    //TODO
}
