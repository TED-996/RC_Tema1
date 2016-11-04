#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "base.h"
#include "login.h"
#include "hash.h"
#include "util.h"

#define DBG
#include "dbg.h"

#define SaltLength 8
const char* loginDirectory = "/logins";


bool usernameOk(const char* username);
bool getLoginFilename(const char* username, char* dest, int destLen);
bool ensureLoginsDirectory();

bool checkLogin(const char* username, const char* password, UserRights* rights){
    if (!ensureLoginsDirectory()){
        perror("creating login directory");
        exit(17);
    }

    char loginFile[4096];

    if (!getLoginFilename(username, loginFile, 4096)){
        fprintf(stderr, "Problem getting login file filename.\n");
        return false;
    }

    int loginFd = open(loginFile, O_RDONLY);
    if (loginFd == -1){
        return false;
    }

    unsigned char salt[SaltLength + 1];
    char expectedPassword[HashLength];
    if (read(loginFd, salt, SaltLength) != 8 || read(loginFd, rights, 4) != 4 || read(loginFd, expectedPassword, HashLength) != HashLength){
        fprintf(stderr, "Warning: Malformed login file %s.\n", loginFile);
        close(loginFd);
        return false;
    }

    close(loginFd);

    unsigned char passwordHash[HashLength];
    unsigned char saltedPassword[SaltLength + 256 + 4];
    
    memcpy(saltedPassword, salt, SaltLength);
    memcpy(saltedPassword + SaltLength, (int*)rights, 4);
    strcpy((char*) saltedPassword + SaltLength + 4, password);

    hashBuffer(saltedPassword, SaltLength + 4 + strlen(password), passwordHash);

    return (memcmp(passwordHash, expectedPassword, HashLength) == 0);
}

bool usernameOk(const char* username){
    for (const char* ptr = username; *ptr != '\0'; ptr++){
        if (!(
            (*ptr >= 'a' && *ptr <= 'z') ||
            (*ptr >= 'A' && *ptr <= 'Z') ||
            (*ptr >= '0' && *ptr <= '9') ||
            *ptr == '_' ||
            *ptr == '.' ||
            *ptr == '-' ||
            *ptr == '\'')){
            return false;
        }
    }
    return true;
}

bool getLoginFilename(const char* username, char* dest, int destLen){
    char* homePath = getenv("HOME");
    if (homePath == NULL){
        homePath = "/";
    }

    //rc test
    char cwd[4096];
    if (getcwd(cwd, 4096) == NULL){
        perror("getting current directory");
        return false;
    }
    homePath = cwd;

    if (!usernameOk(username)){
        return false;
    }


    int bytesWritten = snprintf(dest, destLen, "%s/%s/%s", homePath, loginDirectory, username);
    if (bytesWritten <= 0 || bytesWritten >= destLen){
        return false;
    }

    return true;
}


bool passwordOk(const char* password);

bool registerUser(const char* username, const char* password, UserRights rights){
    if (!passwordOk(password)){
        return false;
    }

    if (!ensureLoginsDirectory()){
        perror("creating login directory");
        exit(17);
    }

    char loginFile[4096];
    getLoginFilename(username, loginFile, 4096);

    int loginFd = open(loginFile, O_WRONLY | O_CREAT | O_EXCL, 0666);
    if (loginFd == -1){
        return false;
    }

    unsigned char salt[SaltLength];
    if (!getRandom(salt, SaltLength)){
        close(loginFd);
        return false;
    }

    char saltedPassword[256 + SaltLength + 4];
    memset(saltedPassword, 0, 256 + SaltLength + 4);
    memcpy(saltedPassword, salt, SaltLength);
    memcpy(saltedPassword + SaltLength, (int*)&rights, 4);
    strcpy((char*)saltedPassword + SaltLength + 4, password);

    unsigned char passwordHash[HashLength];
    hashBuffer(saltedPassword, SaltLength + 4 + strlen(password), passwordHash);

    if (write(loginFd, salt, SaltLength) != SaltLength || write(loginFd, (int*)&rights, 4) != 4 || write(loginFd, passwordHash, HashLength) != HashLength){
        fprintf(stderr, "Cannot register user to file %s.", loginFile);
        close(loginFd);
        return false;
    }

    close(loginFd);
    return true;
}

bool passwordOk(const char* password){
    for (const char* ptr = password; *ptr; ptr++){
        if (*ptr < 32 || *ptr == 0xFF){
            return false;
        }
    }

    return true;
}


bool usernameExists(const char* username){
    char loginFile[4096];

    if (!getLoginFilename(username, loginFile, 4096)){
        fprintf(stderr, "Problem getting login file filename.\n");
        return false;
    }

    struct stat statData;
    if (stat(loginFile, &statData) == -1){
        return false;
    }
    if (!S_ISREG(statData.st_mode)){
        return false;
    }
    return true;
}


bool ensureLoginsDirectory(){
    return ensureDirectoryExists(loginDirectory);
}