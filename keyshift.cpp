//#include <cstdio>
#include <string>
#include <bitset>
#include <iostream>
#include <stdlib.h>
#include <limits>
#include <sstream>
#include <math.h>
#include "./constants.h"

using namespace std;

uint8_t getFTableValue(uint8_t input) {
    uint8_t col = input & LOWFTABLEBITMASK;
    uint8_t row = (input & HIGHFTABLEBITMASK) >> 4;
    return ftable[(row*16)+col];
}

roundInfo whitenInput(uint64_t block, bitset<ALTKEYSIZE> key) {
    roundInfo rInfo;
    bitset<16> k0, k1, k2, k3;
    uint8_t j = 16;
    uint8_t k = 32;
    uint8_t l = 48;
    for (int i = 0; i < 16; i++, j++, k++, l++) {
        k0[i] = key[i];
        k1[i] = key[j];
        k2[i] = key[k];
        k3[i] = key[l];
    }
    rInfo.r3 = (block & WHITEN1) ^ (k0.to_ulong());
    rInfo.r2 = ((block & WHITEN2) >> 16) ^ (k1.to_ulong());
    rInfo.r1 = ((block & WHITEN3) >> 32) ^ (k2.to_ulong());
    rInfo.r0 = ((block & WHITEN4) >> 48) ^ (k3.to_ulong());
    return rInfo;
}

/*
    whitenOutput
        xor the key with the output block to make the
        ciphertext
*/
uint64_t whitenOutput(uint64_t block, bitset<ALTKEYSIZE> key) {
    bitset<16> k0, k1, k2, k3;
    uint8_t j = 16;
    uint8_t k = 32;
    uint8_t l = 48;
    for (int i = 0; i < 16; i++, j++, k++, l++) {
        k0[i] = key[i];
        k1[i] = key[j];
        k2[i] = key[k];
        k3[i] = key[l];    
    }
    uint64_t c3 = (block & WHITEN1) ^ (k0.to_ulong());
    uint64_t c2 = ((block & WHITEN2) >> 16) ^ (k1.to_ulong());
    uint64_t c1 = ((block & WHITEN3) >> 32) ^ (k2.to_ulong());
    uint64_t c0 = ((block & WHITEN4) >> 48) ^ (k3.to_ulong());
    uint64_t ret = (c0 << 48) + (c1 << 32) + (c2 << 16) + c3;
    return ret;
}

void circularRightShift(bitset<ALTKEYSIZE> *curKey) {
    uint8_t firstBit = (*curKey)[0];
    (*curKey) >>= 1;
    (*curKey)[ALTKEYSIZE-1] = firstBit;
}

void circularLeftShift(bitset<ALTKEYSIZE> *curKey) {
    uint8_t lastBit = (*curKey)[ALTKEYSIZE-1];
    (*curKey) <<= 1;
    (*curKey)[0] = lastBit;
}

uint16_t keyFunc(bitset<ALTKEYSIZE> *curKey, uint16_t x, bool encrypt) {
    uint16_t outputByte = x % 8;
    uint16_t keyIndex = outputByte * 8;
    bitset<8> outputSet = 0;
    if (encrypt) circularLeftShift(curKey);
    for (int i=0; i <= 7; i++) {
        outputSet[i] = (*curKey)[keyIndex++];
    }
    if (!encrypt) circularRightShift(curKey);
    return uint16_t(outputSet.to_ulong());
}

uint16_t gPerm(uint16_t w, uint16_t roundNo, uint16_t subkeyVals[ALTNUMROUNDS][12], uint16_t start) {
    uint8_t g1, g2, g3, g4, g5, g6;
    g1 = uint8_t(w >> 8);
    g2 = uint8_t((w << 8) >> 8);
    g3 = getFTableValue(g2 ^ subkeyVals[roundNo][start]) ^ g1;
    g4 = getFTableValue(g3 ^ subkeyVals[roundNo][start + 1]) ^ g2;
    g5 = getFTableValue(g4 ^ subkeyVals[roundNo][start + 2]) ^ g3;
    g6 = getFTableValue(g5 ^ subkeyVals[roundNo][start + 3]) ^ g4;
    printf("g1 %x g2 %x g3 %x g4 %x g5 %x g6 %x\n", g1, g2, g3, g4, g5, g6);
    uint16_t left = g5;
    return ((left << 8) + g6);
}

fInfo fFunc(roundInfo rInfo, uint16_t subkeyVals[ALTNUMROUNDS][12]) {
   fInfo f;
   uint32_t t0 = gPerm(rInfo.r0, rInfo.roundNo, subkeyVals, 0);
   uint32_t t1 = gPerm(rInfo.r1, rInfo.roundNo, subkeyVals, 4);
   f.f0 = (t0 + (2*t1) + ((subkeyVals[rInfo.roundNo][8] << 8) + subkeyVals[rInfo.roundNo][9])) % TWOSIXTEEN;
   f.f1 = ((2*t0) + t1 + ((subkeyVals[rInfo.roundNo][10] << 8) + subkeyVals[rInfo.roundNo][11])) % TWOSIXTEEN;
   cout << "t0: " << hex << t0 << " t1: " << hex << t1 << endl;
   cout << "f0: " << hex << f.f0 << " f1: " << hex << f.f1 << endl;
   return f;
}

roundInfo encrypt(uint16_t subkeyVals[ALTNUMROUNDS][12], roundInfo rInfo) {
    uint16_t tmp1, tmp2;
    fInfo fFuncReturn;
    rInfo.roundNo = 0;
    for (int i = 0; i < 16; i++) {    
        cout << "round " << i << " go!" << endl;
        fFuncReturn = fFunc(rInfo, subkeyVals);
        tmp1 = rInfo.r0;
        tmp2 = rInfo.r1;
        rInfo.r0 = rInfo.r2 ^ fFuncReturn.f0;
        rInfo.r1 = rInfo.r3 ^ fFuncReturn.f1;
        rInfo.r2 = tmp1;
        rInfo.r3 = tmp2;
        rInfo.roundNo++;
        cout << "data for the round: \n\tr0: " << rInfo.r0 << " r1: " << rInfo.r1 << " r2: " << rInfo.r2 << " r3: " << rInfo.r3 << " rNo: " << rInfo.roundNo << endl; 
    }
    return rInfo;
}

roundInfo decrypt(uint16_t subkeyVals[ALTNUMROUNDS][12], roundInfo rInfo) {
    uint16_t tmp1, tmp2;
    fInfo fFuncReturn;
    rInfo.roundNo = 0;
    for (int i = 0; i < 16; i++) {    
        cout << "decrypt round " << i << " go!" << endl;
        fFuncReturn = fFunc(rInfo, subkeyVals);
        tmp1 = rInfo.r0;
        tmp2 = rInfo.r1;
        rInfo.r0 = rInfo.r2 ^ fFuncReturn.f0;
        rInfo.r1 = rInfo.r3 ^ fFuncReturn.f1;
        rInfo.r2 = tmp1;
        rInfo.r3 = tmp2;
        rInfo.roundNo++;
        cout << "data for the round: \n\tr0: " << rInfo.r0 << " r1: " << rInfo.r1 << " r2: " << rInfo.r2 << " r3: " << rInfo.r3 << " rNo: " << rInfo.roundNo << endl; 
    }
    return rInfo;
}

int main(int argc, char *argv[]) {
    string testKeyStr = "0xabcdef0123456789";
    string plaintext = "0x0123456789abcdef";
    bitset<64> testKey = stoull(testKeyStr, nullptr, 16);
    uint16_t subkeyVals[ALTNUMROUNDS][12];
    uint16_t decSubkeyVals[ALTNUMROUNDS][12];
    for (int i = 0; i < ALTNUMROUNDS; i++) {
        for (int j = 0; j < 12; j++) {
            subkeyVals[i][j] = keyFunc(&testKey, (4*i) + (j % 4), true);
        }
    }
    int k = 0;
    for (int i = ALTNUMROUNDS - 1; i >= 0; i--) {
        for (int j = 0; j < 12; j++) {
            decSubkeyVals[k][j] = subkeyVals[i][j];
        }
        k++;
    }
    for (int i = 0; i < ALTNUMROUNDS; i++) {
        for (int j = 0; j < 12; j++) {
            cout << hex << decSubkeyVals[i][j] << " ";
        }
        cout << endl;
    }
    roundInfo rInfo = whitenInput(stoull(plaintext, nullptr, 16), testKey);
    rInfo = encrypt(subkeyVals, rInfo);
    uint64_t intCipherText = (uint64_t(rInfo.r0) << 16) + uint64_t(rInfo.r1) + (uint64_t(rInfo.r2) << 48) + (uint64_t(rInfo.r3) << 32);
    uint64_t cipherBlock = whitenOutput(intCipherText, testKey);
    cout << "cipher block: " << hex << cipherBlock << endl;
    
    roundInfo decRoundInfo = whitenInput(cipherBlock, testKey);
    decRoundInfo = encrypt(decSubkeyVals, decRoundInfo);
    uint64_t intPlainText = (uint64_t(decRoundInfo.r0) << 16) + uint64_t(decRoundInfo.r1) + (uint64_t(decRoundInfo.r2) << 48) + (uint64_t(decRoundInfo.r3) << 32);
    uint64_t plainBlock = whitenOutput(intPlainText, testKey);
    cout << "plain block: " << hex << plainBlock << endl;
    
    exit(1);
}

/*
    Lets write this is psuedo code

    Pre everything
        - Check Arguments
        - Read Key File
            - Translate into a bitset
        - Calculate the Entire subkey array
        - Open Plaintext File
        - Divide into 64 bit blocks
            - Pad out with 00s
 
    Encryption
        - Whiten
            - XOR with key
        - F Function
            - Takes R0, R1, Round Number, 12 subkeys
            - Calls the G Function
                - takes a 16 bit number, some key values as inputs
                - Gets some stuff off of the f table
        - Undo final Swap
        - output whitening (same as input whitening)
    
    Decryption
        - Same as encryption, use subkeys in reverse
*/