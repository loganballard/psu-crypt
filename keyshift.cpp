//#include <cstdio>
//#include <string>
#include <bitset>
#include <iostream>
#include <stdlib.h>
#include "./constants.h"

using namespace std;

unsigned short getFTableValue(unsigned short input) {
    uint col = input & LOWFTABLEBITMASK;
    uint row = (input & HIGHFTABLEBITMASK) >> 4;
    return ftable[(row*16)+col];
}

void whitenInput(uint64_t block, bitset<KEYSIZE> key) {
    uint64_t w1 = block & WHITEN1;
    uint64_t w2 = block & WHITEN2;
    uint64_t w3 = block & WHITEN3;
    uint64_t w4 = block & WHITEN4;

    // TODO - more
}

/*
    whitenOutput
        xor the key with the output block to make the
        ciphertext
*/
uint64_t whitenOutput(uint64_t block, bitset<KEYSIZE> key) {
    bitset<64> smallerKey(0);
    for (int i = 0; i < 64; i++) {
        smallerKey[i] = key[i];
    }
    return (block ^ smallerKey.to_ullong());
}

void circularRightShift(bitset<KEYSIZE> *curKey) {
    int firstBit = (*curKey)[0];
    (*curKey) >>= 1;
    (*curKey)[KEYSIZE-1] = firstBit;
}

void circularLeftShift(bitset<KEYSIZE> *curKey) {
    int lastBit = (*curKey)[KEYSIZE-1];
    (*curKey) <<= 1;
    (*curKey)[0] = lastBit;
}

/*
    Key Functions for encryption and decryption
        For encryption, set encrypt = true
        For decryption, set encrypt = false

    TODO - move off of bitset for output for good
           use shifting
*/
uint8_t keyFunc(bitset<KEYSIZE> *curKey, uint x, bool encrypt) {
    int mod = KEYSIZE / 8;
    int outputByte = x % mod;
    int keyIndex = outputByte * 8;
    bitset<8> outputSet(0);
    if (encrypt) circularLeftShift(curKey);
    for (int i=0; i <= 7; i++) {
        outputSet[i] = (*curKey)[keyIndex++];
    }
    if (!encrypt) circularRightShift(curKey);
    return uint8_t(outputSet.to_ulong());
}

int main(int argc, char *argv[]) {
    // make sure that there is just one argument (the text file)
    /*if (argc != 2) {
        printf("Wrong number of arguments supplied!\n"); 
        exit(1);
    }*/
    exit(1);
}