//#include <cstdio>
//#include <string>
#include <bitset>
#include <iostream>
#include <stdlib.h>
#include <limits>
#include "./constants.h"

using namespace std;

roundInfo rInfo;

unsigned short getFTableValue(unsigned short input) {
    uint col = input & LOWFTABLEBITMASK;
    uint row = (input & HIGHFTABLEBITMASK) >> 4;
    return ftable[(row*16)+col];
}

void whitenInput(uint64_t block, bitset<KEYSIZE> key) {
    cout << key << endl << endl;
    bitset<64> asdf = block;
    cout << asdf << endl << endl;
    uint64_t w0 = block & WHITEN1;
    uint64_t w1 = (block & WHITEN2) >> 16;
    uint64_t w2 = (block & WHITEN3) >> 32;
    uint64_t w3 = (block & WHITEN4) >> 48;
    bitset<16> k0;
    bitset<16> k1;
    bitset<16> k2;
    bitset<16> k3;
    int j = 16;
    int k = 32;
    int l = 48;
    for (int i = 0; i < 16; i++, j++, k++, l++) {
        k0[i] = key[i];
        k1[i] = key[j];
        k2[i] = key[k];
        k3[i] = key[l];
    }
    rInfo.r0 = w0 ^ k0.to_ulong();
    rInfo.r1 = w1 ^ k1.to_ulong();
    rInfo.r2 = w2 ^ k2.to_ulong();
    rInfo.r3 = w3 ^ k3.to_ulong();
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