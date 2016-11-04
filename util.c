#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>

#include "base.h"
#include "util.h"

//#define DBG
#include "dbg.h"


int readStr(int fd, char* buffer, int bufSize){
    char* bufPtr = buffer;
    int totalRead = 0;

    while(totalRead < bufSize - 1){
        int bytesRead = read(fd, bufPtr, 1);
        if (bytesRead < 0){
            return bytesRead;
        }
        if (bytesRead == 0 || *bufPtr == '\0'){
            *bufPtr = '\0';
            return totalRead;
        }

        (*bufPtr)++;
        totalRead++;
    }

    //We must consume the rest of the buffer. Sorry.
    char temp = -1;
    while(temp){
        int bytesRead = read(fd, &temp, 1);
        if (bytesRead < 0){
            return bytesRead;
        }
    }

    buffer[bufSize - 1] = '\0';
    return bufSize - 1;
}


int writeStr(int fd, const char* str){
    int len = strlen(str);

    int written = write(fd, str, len);
    
    if (written < 0){
        return written;
    }

    int termWritten = write(fd, str + len, 1);
    if (termWritten < 0){
        return termWritten;
    }
    return written;
}


int readSizedBuffer(int fd, unsigned char* buffer, int bufSize){
    int size;
    int bytesRead;

    bytesRead = read(fd, &size, 4);
    if (bytesRead < 0){
        return bytesRead;
    }
    if (bytesRead != 4){
        return -1;
    }

    int readSize = size;
    if (size > bufSize){
        readSize = bufSize;
    }

    bytesRead = read(fd, buffer, readSize);
    if (bytesRead < 0){
        return bytesRead;
    }

    if (readSize != size){
        //We must consume the rest of the buffer. Sorry.
        const int garbageSize = 1024;
        unsigned char garbage[garbageSize];
        int bytesLeft = size - readSize;

        while (bytesLeft > garbageSize){
            bytesRead = read(fd, garbage, garbageSize);
            if (bytesRead < 0){
                return bytesRead;
            }
            bytesLeft -= garbageSize;
        }
        bytesRead = read(fd, garbage, bytesLeft);
        if (bytesRead < 0){
            return bytesRead;
        }
    }

    return readSize;
}


int readSizedStr(int fd, char* buffer, int bufSize){
    int bytesRead = readSizedBuffer(fd, (unsigned char*)buffer, bufSize - 1);
    if (bytesRead < 0){
        return bytesRead;
    }

    buffer[bytesRead] = '\0';
    return bytesRead;
}


int writeSizedBuffer(int fd, const unsigned char* buffer, int size){
    int bytesWritten = write(fd, &size, 4);
    if (bytesWritten < 0){
        return bytesWritten;
    }
    if (bytesWritten != 4){
        return -1;
    }

    bytesWritten = write(fd, buffer, size);
    if (bytesWritten < 0){
        return bytesWritten;
    }
    if (bytesWritten != size){
        return -1;
    }

    return bytesWritten;
}


int writeSizedStr(int fd, const char* buffer){
    return writeSizedBuffer(fd, (const unsigned char*)buffer, strlen(buffer));
}


int allocReadSizedBuffer(int fd, unsigned char** dst){
    int size;
    int bytesRead;

    *dst = NULL;

    bytesRead = read(fd, &size, 4);
    if (bytesRead < 0){
        dbg("erorr reading size: %d", bytesRead);
        return bytesRead;
    }
    if (bytesRead != 4){
        dbg("size bytes not available");
        return -1;
    }

    dbg("read size is %d", size);

    unsigned char* buffer = malloc(size);
    if (buffer == NULL){
        dbg("malloc failed");
        return -1;
    }

    bytesRead = read(fd, buffer, size);
    if (bytesRead < 0){
        dbg("read bulk failed");
        free(buffer);
        return bytesRead;
    }
    if (bytesRead != size){
        dbg("read bulk not enough");
        free(buffer);
        return -1;
    }

    *dst = buffer;

    dbg("allocd buffer %s at ptr %p", buffer, buffer);
    
    return size;
}


int allocReadSizedStr(int fd, char** dst){
    int size;
    int bytesRead;

    *dst = NULL;

    bytesRead = read(fd, &size, 4);
    if (bytesRead < 0){
        dbg("erorr reading size: %d", bytesRead);        
        return bytesRead;
    }
    if (bytesRead != 4){
        dbg("size bytes not available");
        return -1;
    }

    //dbg("read size is %d", size);    

    char* buffer = malloc(size + 1);
    if (buffer == NULL){
        dbg("malloc failed");        
        return -1;
    }

    bytesRead = read(fd, buffer, size);
    if (bytesRead < 0){
        dbg("read bulk failed");        
        free(buffer);
        return bytesRead;
    }
    if (bytesRead != size){
        dbg("read bulk not enough");
        free(buffer);
        return -1;
    }

    buffer[size] = '\0';
    *dst = buffer;

    dbg("allocd string %s at ptr %p", buffer, buffer);

    return size;
}


int bufcmp(const void* buf1, int len1, const void* buf2, int len2){
    if (len1 != len2){
        return len1 - len2;
    }

    unsigned char* ptr1 = (unsigned char*)buf1;
    unsigned char* ptr2 = (unsigned char*)buf2;

    for (int i = 0; i < len1; i++){
        if (*ptr1 != *ptr2){
            return *ptr1 - *ptr2;
        }
        ptr1++;
        ptr2++;
    }

    return 0;
}


void free2d(const void** data, int len){
    if (data == NULL){
        return;
    }

    for (int i = 0; i < len; i++){
        dbg("freeing %p (idx %d, data = %s)", data[i], i, (char*)data[i]);
        free((void*)data[i]);
    }

    dbg("freeing %p (base)", data);
    free((void*)data);
}


bool getRandom(unsigned char* buffer, int nrBytes){
    int randomFd = open("/dev/urandom", O_RDONLY);
    if (randomFd == -1){
        return false;
    }
    if (read(randomFd, buffer, nrBytes) != nrBytes){
        close(randomFd);
        return false;
    }
    return true;
}



int execChild(char** arguments, int nrArgs, int* stdoutChannel){
	int childPid = fork();
	if (childPid == -1){
		return -1;
	}
	if (childPid != 0){
		close(stdoutChannel[1]);

        return childPid;
	}
	else{
		dup2(1, stdoutChannel[1]);
        close(stdoutChannel[0]);
        close(stdoutChannel[1]);

        dup2(2, 1);
        
        char** arguments2 = malloc((nrArgs + 1) * sizeof(char*));
        for (int i = 0; i < nrArgs; i++){
            arguments2[i] = arguments[i];
        }
        arguments2[nrArgs] = NULL;

        if (execvp(arguments[0], arguments2) == -1){
            perror("executing child process");
            exit(10);
        }
	}

    //If execution gets here, it's definitely a failure state.
    return -1;
}


int spawnSplitChannels(int (*childMain)(int, int), int* inChannel, int* outChannel){
	int childPid = fork();
	if (childPid == -1){
		return -1;
	}
	if (childPid != 0){
        if (close(inChannel[0]) != 0 || close(outChannel[1]) != 0){
            perror("preparing channels for parent process");
        }

		return childPid;
	}
	else{
        if (close(inChannel[1]) != 0 || close(outChannel[0]) != 0){
            perror("preparing channels for child process");
            exit(1);
        }

		int retcode = childMain(inChannel[0], outChannel[1]);
        exit(retcode);
	}

    //If execution gets here, it's definitely a failure state.
    return -1;
}


bool mkdirRec(char* startDir, char* nextDirs);

bool ensureDirectoryExists(const char* directoryInsideHome){
    char* homePath = getenv("HOME");
    if (homePath == NULL){
        homePath = "/";
    }
    
    char homePathBuffer[4096];
    char childDirBuffer[4096];

    //rc test
    if (getcwd(homePathBuffer, 4096) == NULL){
        perror("getting current directory");
        return false;
    }
    
    //strcpy(homePathBuffer, homePath);
    strcpy(childDirBuffer, directoryInsideHome);

    if (!mkdirRec(homePathBuffer, childDirBuffer)){
        return false;
    }
    return true;
}


const char* skipChars(const char* ptr, char chr);

bool mkdirRec(char* startDir, char* nextDirs){
    if (nextDirs[0] == '\0'){
        return true;
    }

    const char* nextInPath = skipChars(nextDirs, '/');
    char* slashIdx = strchr(nextInPath, '/');
    if (slashIdx == NULL){
        strcat(startDir, "/");
        strcat(startDir, nextInPath);
    }
    else{
        strcat(startDir, "/");
        *slashIdx = '\0';
        strcat(startDir, nextInPath);
    }

    struct stat statData;
    if (stat(startDir, &statData) != -1){
        if (!S_ISDIR(statData.st_mode)){
            perror("An element parent of the requested path is not a directory!");
            return false;
        }
    }
    else{
        if (mkdir(startDir, 0777) != 0){
            perror("creating directory");
            return false;
        }
    }

    if (slashIdx != NULL){
        return mkdirRec(startDir, slashIdx + 1);
    }
    return true;
}

const char* skipChars(const char* ptr, char chr){
    while(*ptr == chr){
        ptr++;
    }
    return ptr;
}