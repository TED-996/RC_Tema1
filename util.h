#ifndef RCT1_UTIL_H
#define RCT1_UTIL_H

int readStr(int fd, char* buffer, int bufSize);
int writeStr(int fd, const char* str);

int readSizedBuffer(int fd, unsigned char* buffer, int bufSize);
int readSizedStr(int fd, char* buffer, int bufSize);
int writeSizedBuffer(int fd, const unsigned char* buffer, int size);
int writeSizedStr(int fd, const char* buffer);

int allocReadSizedBuffer(int fd, unsigned char** dst);
int allocReadSizedStr(int fd, char** dst);

int bufcmp(const void* buf1, int len1, const void* buf2, int len2);
void free2d(const void** data, int len);

bool getRandom(unsigned char* buffer, int nrBytes);

int spawnSplitChannels(int (*childMain)(int, int), int* inChannel, int* outChannel);
int execChild(char** arguments, int nrArgs, int* stdoutChannel);

bool ensureDirectoryExists(const char* directoryInsideHome);

#endif