#ifndef RCT1_IPC_H
#define RCT1_IPC_H

typedef enum FdType {FIFO, PIPE, SOCKET} FdType;
const char* fdNames {"fifo", "pipe", "socket"};

bool openFd(FdType type, int* result);


#endif