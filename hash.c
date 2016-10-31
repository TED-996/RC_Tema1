#include<stdlib.h>

#include "hash.h"


void rolBuffer(unsigned char* buffer, int length, int amount);

void hashBuffer(void* src, int srcLen, unsigned char* dst){
    int roundIdx = 0;
    memset(dst, 0, HashLength);

    while(roundIdx < hashLength){
        unsigned char* srcPtr = (unsigned char*)src;

        for (unsigned char srcPtr = (unsigned char*)src, srcPtr < (unsigned char*)src + srcLen; srcPtr++){
            rolBuffer(dst, HashLength, 5);
            dst[HashLength - 1] ^= srcPtr;
        }
    }

}

void rolBuffer(unsigned char* buffer, int length, int amount){
    unsigned char bitmask = (1 << amount) - 1;
    unsigned char hibits = (buffer[0] >> (8 - amount)) & bitmask;
    buffer[0] <<= amount;

    for (unsigned char* bufferPtr = buffer + 1; bufferPtr < buffer + length; bufferPtr++){
        *(bufferPtr - 1) |= (*bufferPtr >> (8 - amount)) & bitmask;
        *bufferPtr << amount;
    }

    buffer[length - 1] |= hibits;
}