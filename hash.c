#include <stdlib.h>
#include <string.h>

#include "base.h"
#include "hash.h"


void rolBuffer(unsigned char* buffer, int length, int amount);

void hashBuffer(void* src, int srcLen, unsigned char* dst){
    int roundIdx = 0;
    memset(dst, 0, HashLength);

    unsigned char* srcCpy = (unsigned char*) src;

    unsigned char hashBuffer[HashLength + 1];
    memset(hashBuffer, 0, HashLength + 1);

    while(roundIdx < HashLength){
        for (unsigned char* srcPtr = srcCpy; srcPtr < srcCpy + srcLen; srcPtr++){
            rolBuffer(hashBuffer, HashLength + 1, 5);
            hashBuffer[0] ^= *srcPtr;
        }
        roundIdx++;
    }

    memcpy(dst, hashBuffer, HashLength);

}

void rolBuffer(unsigned char* buffer, int length, int amount){
    unsigned char bitmask = (1 << amount) - 1;
    unsigned char hibits = (buffer[0] >> (8 - amount)) & bitmask;
    buffer[0] <<= amount;

    for (unsigned char* bufferPtr = buffer + 1; bufferPtr < buffer + length; bufferPtr++){
        *(bufferPtr - 1) |= (*bufferPtr >> (8 - amount)) & bitmask;
        *bufferPtr <<= amount;
    }

    buffer[length - 1] |= hibits;
}