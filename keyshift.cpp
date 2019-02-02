//#include <cstdio>
#include <string>
#include <stdlib.h>
#include <bitset>
#include <iostream>

using namespace std;


// TODO - cleanup
// TODO - move off of bitsets where it they are just used for convenience

void circularRightShift(uint64_t *curKey) {
    *curKey = ((*curKey) >> 1) + ((*curKey )<< 63);
}

void circularLeftShift(uint64_t *curKey) {
    *curKey = ((*curKey) << 1) + ((*curKey) >> 63);
}

/*
    Key Functions for encryption and decryption
        For encryption, set encrypt = true
        For decryption, set encrypt = false
*/
uint64_t keyFunc(uint64_t *curKey, uint x, bool encrypt) {
    if (encrypt) {
        circularLeftShift(curKey);
    }
    bitset<64> keyBits(*curKey);
    int outputByte = x % 8;
    int keyIndex = outputByte * 8;
    bitset<8> outputSet(0);
    for (int i=0; i <= 7; i++) {
        outputSet[i] = keyBits[keyIndex++];
    }
    if (!encrypt) {
        circularRightShift(curKey);
    }
    return outputSet.to_ullong();
}

int main(int argc, char *argv[]) {
    // make sure that there is just one argument (the text file)
    /*if (argc != 2) {
        printf("Wrong number of arguments supplied!\n"); 
        exit(1);
    }*/
    uint64_t key = uint64_t(1) << 2; // TODO remove testing value
    cout << keyFunc(&key, 0, true) << endl;
    cout << keyFunc(&key, 0, true) << endl;
    exit(1);
}