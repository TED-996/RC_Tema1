#include<unistd.h>
#include<string.h>

#include"util.h"


int readStr(int fd, char* buffer, int bufSize){
    char* bufPtr = buffer;
    int chrLeft = bufSize;
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

        *bufPtr++;
        totalRead++;
    }

    //We must consume the rest of the buffer. Sorry.
    char temp = -1;
    while(temp){
        int byresRead = read(fd, &temp, 1);
        if (bytesRead < 0){
            return bytesRead;
        }
    }

    buffer[bufSize - 1] = '\0';
    return bufSize - 1;
}


int writeStr(int fd, char* str){
    char* ptr = str;
    int len = strlen(str);

    int written = write(fd, str, len);
    
    if (written < 0){
        return written;
    }

    int termWritten = write(fd, *str + len, 1);
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
    int bytesRead = readSizedBuffer(fd, buffer, bufSize - 1);
    if (bytesRead < 0){
        return bytesRead;
    }

    buffer[bytesRead] = '\0';
    return bytesRead;
}


int writeSizedBuffer(int fd, unsigned char* buffer, int size){
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


int writeSizedStr(int fd, char* buffer){
    return writeSizedStr(fd, buffer, strlen(buffer));
}


int allocReadSizedBuffer(int fd, unsigned char** dst){
    int size;
    int bytesRead;

    bytesRead = read(fd, &size, 4);
    if (bytesRead < 0){
        return bytesRead;
    }
    if (bytesRead != 4){
        return -1;
    }

    unsigned char* buffer = malloc(size);
    if (buffer == NULL){
        return -1;
    }

    bytesRead = read(fd, buffer, size);
    if (bytesRead < 0){
        free(buffer);
        return bytesRead;
    }
    if (bytesRead != size){
        free(buffer);
        return -1;
    }

    *dst = buffer;
    return size;
}


int allocReadSizedStr(int fd, char** dst){
    int size;
    int bytesRead;

    bytesRead = read(fd, &size, 4);
    if (bytesRead < 0){
        return bytesRead;
    }
    if (bytesRead != 4){
        return -1;
    }

    char* buffer = malloc(size + 1);
    if (buffer == NULL){
        return -1;
    }

    bytesRead = read(fd, buffer, size);
    if (bytesRead < 0){
        free(buffer);
        return bytesRead;
    }
    if (bytesRead != size){
        free(buffer);
        return -1;
    }

    buffer[size] = '\0';
    *dst = buffer;
    return size;
}


int bufcmp(void* buf1, int len1, void* buf2, int len2){
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


void free2d(void** data, int len){
    for (int i = 0; i < len; i++){
        free(data[i]);
    }
    free(data);
}


bool getRandom(char* buffer, int nrBytes){
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