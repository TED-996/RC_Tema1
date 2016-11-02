#include<unistd.h>
#include<stdin.h>
#include<stdlib.h>
#include<strings.h>

#include "util.h"
#include "login.h"


char** readCommand(int inFd, int outFd, int* nrArgs);
bool execCommand(char** command, int nrArgs, int outFd, UserRights* &rights);

int serverMain(int inFd, int outFd){
    UserRights userRights = 0;

    char welcomeStr[300];
    snprintf(welcomeStr, 300, "Welcome!\nWaiting for commands.");
    writeSizedStr(outFd, welcomeStr);

    int nrArgs;    
    char** command = readCommand(inFd, outFd, &nrArgs);
    while(command != NULL){
        if (!execCommand(command, nrArgs, outFd, &rights)){
            perror("executing command");
            
            close(intFd);
            close(outFd);

            exit(10);
        }

        free2d(command, nrArgs);
        command = readCommand(inFd, outFd, &nrArgs);
    }

    close(intFd);
    close(outFd);
    return 0;
}


char** readCommand(int inFd, int outFd, int* nrArgs){
    char** result = NULL;

    int bytesRead = read(inFd, nrArgs, 4);
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


void execLogin(char** command, int nrArgs, int outFd, UserRights* rights);
void execRegister(char** command, int nrArgs, int outFd, UserRights* rights);

bool execCommand(char** command, int nrArgs, int outFd, UserRights* rights){
    if (nrArgs == 0){
        perror("there should never be 0 arguments");
        return false;
    }

    if (nrArgs == 1 && command[0][0] == '\0'){
        writeSizedStr(outFd, "\n");
        writeSizedStr(outFd, "");
        return true;
    }

    if (strcmp(command[0], "login") == 0){
        execLogin(command, nrArgs, outFd, rights);
        return;
    }

    if (strcmp(command[0], "register") == 0){
        execRegister(command, nrArgs, outFd, rights);
        return;
    }
}


void execLogin(char** command, int nrArgs, int outFd, UserRights* rights){
    if (nrArgs != 3){
        writeSizedStr(outFd, "Invalid arguments.\nUsage: login username password\n");
        writeSizedStr(outFd, "");
        return;
    }

    if (strlen(command[1]) == 0 || strlen(command[1]) > 255){
        writeSizedStr(outFd, "Username invalid.");
        writeSizedStr(outFd, "");
        return;
    }
    if (strlen(command[2]) == 0 || strlen(command[2]) > 255){
        writeSizedStr(outFd, "Password invalid.");
        writeSizedStr(outFd, "");
        return;
    }

    if (!checkLogin(command[1], command[2], rights)){
        writeSizedStr(outFd, "Username + password combination invalid.");
        writeSizedStr(outFd, "");
        return;
    }

    char welcomeStr[300];
    snprintf(welcomeStr, 300, "Welcome %s!\n", command[1]);

    writeSizedStr(outFd, welcomeStr);
    writeSizedStr(outFd, "");
}


void execRegister(char** command, int nrArgs, int outFd, UserRights* rights){
    if (!(*rights & RightRegister)){
        writeSizedStr(outFd, "Insufficient rights.");
        writeSizedStr(outFd, "");
        return;
    }

    if (nrArgs != 4){
        writeSizedStr(outFd, "Invalid arguments.\nUsage: register username password rights\n");
        writeSizedStr(outFd, "\trights: {r}{l}{p}{s} (register, limited, protected, sudo)\n");
        writeSizedStr(outFd, "");
        return;
    }

    if (strlen(command[1]) == 0 || strlen(command[1]) > 255){
        writeSizedStr(outFd, "Username invalid.");
        writeSizedStr(outFd, "");
        return;
    }

    if (strlen(command[2]) == 0 || strlen(command[2]) > 255){
        writeSizedStr(outFd, "Password invalid.");
        writeSizedStr(outFd, "");
        return;
    }

    UserRights newRights = 0;

    if (strchr(command[3], 'r')){
        newRights |= RightRegister;
    }
    if (strchr(command[3], 'l')){
        newRights |= RightSysCmd;
    }
    if (strchr(command[3], 'p`')){
        newRights |= RightSysCmd;
    }
    if (strchr(command[3], 's')){
        newRights |= RightSudo;
    }

    for (int i = 0; i < 32; i++){
        if (newRights & (1 << i) && !(*rights & (1 << i))){
            writeSizedStr(outFd, "You cannot give rights you don't own.");
            writeSizedStr(outFd, "");
            return;
        }
    }
    
    if (!register(command[1], command[2], newRights)){
        writeSizedStr(outFd, "Could not register user.");
        writeSizedStr(outFd, "");
        return;
    }

    char resultStr[300];
    snprintf(resultStr, 300, "User %s registered.\n", command[1]);

    writeSizedStr(outFd, resultStr);
    writeSizedStr(outFd, "");
}
