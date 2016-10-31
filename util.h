#ifndef RCT1_UTIL_H
#define RCT1_UTIL_H

int readStr(int fd, char* buffer, int bufSize);
int writeStr(int fd, char* str);

//int readSizedBuffer(int fd, unsigned char* buffer, int bufSize);
//int readSizedStr(int fd, char* buffer, int bufSize);
int writeSizedBuffer(int fd, unsigned char* buffer, int size);
int writeSizedStr(int fd, char* buffer);

int allocReadSizedBuffer(int fd, unsigned char** dst);
int allocReadSizedStr(int fd, unsigned char** dst);

int bufcmp(void* buf1, int len1, void* buf2, int len2);
void free2d(void** data, int len);

bool getRandom(char* buffer, int nrBytes);

#endif