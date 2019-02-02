//#include <cstdio>
#include <string>
#include <stdlib.h>
#include <bitset>
#include <iostream>

using namespace std;

const int KEYSIZE = 80;


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
*/
bitset<8> keyFunc(bitset<KEYSIZE> *curKey, uint x, bool encrypt) {
    int mod = KEYSIZE / 8;
    if (encrypt) circularLeftShift(curKey);
    int outputByte = x % mod;
    int keyIndex = outputByte * 8;
    bitset<8> outputSet(0);
    for (int i=0; i <= 7; i++) {
        outputSet[i] = (*curKey)[keyIndex++];
    }
    if (!encrypt) circularRightShift(curKey);
    return outputSet;
}

int main(int argc, char *argv[]) {
    // make sure that there is just one argument (the text file)
    /*if (argc != 2) {
        printf("Wrong number of arguments supplied!\n"); 
        exit(1);
    }*/
    bitset<80> key(1);
    // TODO remove testing values
    
    exit(1);
}