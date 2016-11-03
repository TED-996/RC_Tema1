#define _XOPEN_SOURCE 700

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#include <time.h>

#include "base.h"
#include "server.h"
#include "util.h"
#include "login.h"
#include "ipc.h"

#define DBG
#include "dbg.h"


char** readCommand(int inFd, int outFd, int* nrArgs);
bool execCommand(char** command, int nrArgs, int outFd, UserRights* rights);

int serverMain(int inFd, int outFd){
    UserRights userRights = 0;
    dbg("In server main");

    char welcomeStr[300];
    snprintf(welcomeStr, 300, "Welcome!\nWaiting for commands.\n");
    writeSizedStr(outFd, welcomeStr);

    int nrArgs;
    char** command = readCommand(inFd, outFd, &nrArgs);
    while(command != NULL){
        if (nrArgs >= 1){
            dbg("command is %s", command[0]);
        }
        else{
            dbg("command: 0 arguments");
        }

        if (nrArgs == 1 && strcmp(command[0], "exit") == 0){
            free2d((const void**)command, nrArgs);
            break;
        }

        if (!execCommand(command, nrArgs, outFd, &userRights)){
            perror("executing command");
            
            close(inFd);
            close(outFd);

            exit(10);
        }

        dbg("server: freeing command; %d args, at pos %p", nrArgs, command);
        free2d((const void**)command, nrArgs);
        dbg("server: new command");
        command = readCommand(inFd, outFd, &nrArgs);
    }

    close(inFd);
    close(outFd);
    return 0;
}


char** readCommand(int inFd, int outFd, int* nrArgs){
    char** result = NULL;
    int newNrArgs = 0;

    int bytesRead = read(inFd, &newNrArgs, 4);
    if (bytesRead != 4){
        return NULL;
    }
    *nrArgs = newNrArgs;

    result = malloc(sizeof(char*) * (*nrArgs));
    if (result == NULL){
        return NULL;
    }

    for (int i = 0; i < *nrArgs; i++){
        char* arg = 0;
        if (allocReadSizedStr(inFd, &arg) < 0){
            free2d((const void**)result, i);
            return NULL;
        }
        result[i] = arg;
    }

    return result;
}


void execLogin(char** command, int nrArgs, int outFd, UserRights* rights);
void execRegister(char** command, int nrArgs, int outFd, UserRights* rights);
void execSysCmd(char** command, int nrArgs, int outFd, UserRights* rights);

bool execCommand(char** command, int nrArgs, int outFd, UserRights* rights){
    if (nrArgs == 0){
        perror("there should never be 0 arguments");
        return false;
    }

    dbg("current command: %s", command[0]);

    if (nrArgs == 1 && command[0][0] == '\0'){
        writeSizedStr(outFd, "\n");
        writeSizedStr(outFd, "");
        return true;
    }

    if (strcmp(command[0], "login") == 0){
        execLogin(command, nrArgs, outFd, rights);
        return true;
    }

    if (strcmp(command[0], "register") == 0){
        execRegister(command, nrArgs, outFd, rights);
        return true;
    }

    execSysCmd(command, nrArgs, outFd, rights);
    return true;
}


void execLogin(char** command, int nrArgs, int outFd, UserRights* rights){
    if (nrArgs != 3){
        writeSizedStr(outFd, "Invalid arguments.\nUsage: login username password\n");
        writeSizedStr(outFd, "");
        return;
    }

    if (strlen(command[1]) == 0 || strlen(command[1]) > 255){
        writeSizedStr(outFd, "Username invalid.\n");
        writeSizedStr(outFd, "");
        return;
    }
    if (strlen(command[2]) == 0 || strlen(command[2]) > 255){
        writeSizedStr(outFd, "Password invalid.\n");
        writeSizedStr(outFd, "");
        return;
    }

    if (!checkLogin(command[1], command[2], rights)){
        writeSizedStr(outFd, "Username + password combination invalid.\n");
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
        writeSizedStr(outFd, "Insufficient rights.\n");
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
        writeSizedStr(outFd, "Username invalid.\n");
        writeSizedStr(outFd, "");
        return;
    }

    if (strlen(command[2]) == 0 || strlen(command[2]) > 255){
        writeSizedStr(outFd, "Password invalid.\n");
        writeSizedStr(outFd, "");
        return;
    }

    UserRights newRights = 0;
    int nrRights = 0;

    if (strchr(command[3], 'r')){
        newRights |= RightRegister;
        nrRights++;
    }
    if (strchr(command[3], 'l')){
        newRights |= RightSysCmd;
        nrRights++;
    }
    if (strchr(command[3], 'p')){
        newRights |= RightSysCmd;
        nrRights++;
    }
    if (strchr(command[3], 's')){
        newRights |= RightSudo;
        nrRights++;
    }

    if (strlen(command[3]) != nrRights){
        writeSizedStr(outFd, "Invalid rights.\nUnrecognized values.\n");
        writeSizedStr(outFd, "");
        return;
    }

    for (int i = 0; i < 32; i++){
        if ((newRights & (1 << i)) && !(*rights & (1 << i))){
            writeSizedStr(outFd, "You cannot give rights you don't own.\n");
            writeSizedStr(outFd, "");
            return;
        }
    }
    
    if (!registerUser(command[1], command[2], newRights)){
        writeSizedStr(outFd, "Could not register user.\n");
        writeSizedStr(outFd, "");
        return;
    }

    char resultStr[300];
    snprintf(resultStr, 300, "User %s registered.\n", command[1]);

    writeSizedStr(outFd, resultStr);
    writeSizedStr(outFd, "");
}


void execSysCmd(char** command, int nrArgs, int outFd, UserRights* rights){
    if (!(*rights & RightSysCmd)){
        writeSizedStr(outFd, "Insufficient rights.\n");
        writeSizedStr(outFd, "");
        return;
    }

    int stdoutPipes[2];
    if (!openChannel(Pipe, stdoutPipes)){
        perror("opening pipe to child process");
        writeSizedStr(outFd, "Could not start command (opening channels)\n");
        writeSizedStr(outFd, "");
        return;
    }

    int childPid = execChild(command, nrArgs, stdoutPipes);
    if (childPid == -1){
        writeSizedStr(outFd, "Could not start command.\n");
        writeSizedStr(outFd, "");
        return;
    }

    int inFd = stdoutPipes[0];
    int fdFlags = fcntl(inFd, F_GETFL, 0);
    fcntl(inFd, F_SETFL, fdFlags | O_NONBLOCK);

    const int bufferSize = 1024;
    char* buffer[bufferSize];

    struct timespec sleepTime;
    sleepTime.tv_sec = 0;
    sleepTime.tv_nsec = 10 * 1000 * 1000; 

    while(true){
        int bytesRead = read(inFd, buffer, bufferSize);
        if (bytesRead == 0 || (bytesRead < 0 && errno != EAGAIN && errno != EWOULDBLOCK)){
            break;
        }
        if (bytesRead < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)){
            nanosleep(&sleepTime, NULL);
            continue;
        }

        dbg("read %d bytes", bytesRead);

        if (writeSizedBuffer(outFd, (unsigned char*)buffer, bytesRead) != bytesRead){
            perror("writing response to pipe");
            break;
        }
    }
    writeSizedStr(outFd, "\n");

    if (wait(NULL) == -1){
        perror("waiting for child process to close");
        exit(20);
    }

    writeSizedStr(outFd, "");
}

