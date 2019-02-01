//#include <cstdio>
#include <string>
#include <stdlib.h>
#include <bitset>
#include <iostream>

using namespace std;

/*
    The Key Schedule K(x)
    o Decryption:
    Input: A number x and it is assumed that K() has access to the current, stored, 64 bit key K. 
    The key is 64 bits and so 8 bytes long. Label the bytes 0 through 7 as for encryption. 
    Now z is the x mod 8 byte of the key K. Right rotate K by 1 bit and store this value as the new key K’. 
    Unlike encryption, the subkey is gotten before the rotation of the key. Output: z.
    Instead you can generate ALL subkeys when the program starts, in this case 
    decryption keys are just the keys used in reverse order.

*/

// TODO add decrypt function
// TODO - cleanup
// TODO - move off of bitsets where it they are just used for convenience
// TODO - add documentation

/*
    makeNewKey - does left rotate of current key
*/
void makeNewKey(uint64_t *curKey) {
    bitset<64> keyBits(*curKey);
    int leftMostBit = keyBits[63];
    *curKey = (*curKey) << 1;
    bitset<64> newKeyBits(*curKey);
    newKeyBits[0] = newKeyBits[0] | leftMostBit;
    *curKey = newKeyBits.to_ullong();
}


/*
    keyEncrypt - does key encryption function and 
        returns the next part of the key
*/
uint64_t keyEncrypt(uint64_t *curKey, uint x) {
    makeNewKey(curKey);
    bitset<64> keyBits(*curKey);
    int outputByte = x % 8;
    int keyIndex = outputByte * 8;
    bitset<8> outputSet(0);
    for (int i=0; i <= 7; i++) {
        outputSet[i] = keyBits[keyIndex++];
    }
    return outputSet.to_ullong();
}


int main(int argc, char *argv[]) {
    // make sure that there is just one argument (the text file)
    /*if (argc != 2) {
        printf("Wrong number of arguments supplied!\n"); 
        exit(1);
    }*/
    uint64_t key = uint64_t(1) << 63; // TODO remove testing value
    cout << keyEncrypt(&key, 0) << endl;
    exit(1);
}