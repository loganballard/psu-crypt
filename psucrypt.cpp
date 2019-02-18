#include <string>
#include <bitset>
#include <iostream>
#include <stdlib.h>
#include <limits>
#include <sstream>
#include <fstream>
#include <math.h>
#include "constants.h"

using namespace std;

uint8_t getFTableValue(uint8_t input) {
    uint8_t col = input & LOWFTABLEBITMASK;
    uint8_t row = (input & HIGHFTABLEBITMASK) >> 4;
    return ftable[(row*16)+col];
}

roundInfo whitenInput(uint64_t block, bitset<KEYSIZE> key) {
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
uint64_t whitenOutput(uint64_t block, bitset<KEYSIZE> key) {
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

void circularLeftShift(bitset<KEYSIZE> *curKey) {
    uint8_t lastBit = (*curKey)[KEYSIZE-1];
    (*curKey) <<= 1;
    (*curKey)[0] = lastBit;
}

void gradCircularLeftShift(bitset<GRADKEYSIZE> *curKey) {
    uint8_t lastBit = (*curKey)[GRADKEYSIZE-1];
    (*curKey) <<= 1;
    (*curKey)[0] = lastBit;
}

uint16_t keyFunc(bitset<KEYSIZE> *curKey, uint16_t x) {
    uint16_t outputByte = x % 8;
    uint16_t keyIndex = outputByte * 8;
    bitset<8> outputSet = 0;
    circularLeftShift(curKey);
    for (int i=0; i <= 7; i++) {
        outputSet[i] = (*curKey)[keyIndex++];
    }
    return uint16_t(outputSet.to_ulong());
}

uint16_t gradKeyFunc(bitset<GRADKEYSIZE> *curKey, uint16_t x) {
    uint16_t outputByte = x % 8;
    uint16_t keyIndex = outputByte * 8;
    bitset<8> outputSet = 0;
    gradCircularLeftShift(curKey);
    for (int i=0; i <= 7; i++) {
        outputSet[i] = (*curKey)[keyIndex++];
    }
    return uint16_t(outputSet.to_ulong());
}

uint16_t gPerm(uint16_t w, uint16_t roundNo, uint16_t subkeyVals[][12], uint16_t start) {
    uint8_t g1, g2, g3, g4, g5, g6;
    g1 = uint8_t(w >> 8);
    g2 = uint8_t((w << 8) >> 8);
    g3 = getFTableValue(g2 ^ subkeyVals[roundNo][start]) ^ g1;
    g4 = getFTableValue(g3 ^ subkeyVals[roundNo][start + 1]) ^ g2;
    g5 = getFTableValue(g4 ^ subkeyVals[roundNo][start + 2]) ^ g3;
    g6 = getFTableValue(g5 ^ subkeyVals[roundNo][start + 3]) ^ g4;
    // printf("g1 %x g2 %x g3 %x g4 %x g5 %x g6 %x\n", g1, g2, g3, g4, g5, g6);
    uint16_t left = g5;
    return ((left << 8) + g6);
}

fInfo fFunc(roundInfo rInfo, uint16_t subkeyVals[][12]) {
   fInfo f;
   uint32_t t0 = gPerm(rInfo.r0, rInfo.roundNo, subkeyVals, 0);
   uint32_t t1 = gPerm(rInfo.r1, rInfo.roundNo, subkeyVals, 4);
   f.f0 = (t0 + (2*t1) + ((subkeyVals[rInfo.roundNo][8] << 8) + subkeyVals[rInfo.roundNo][9])) % TWOSIXTEEN;
   f.f1 = ((2*t0) + t1 + ((subkeyVals[rInfo.roundNo][10] << 8) + subkeyVals[rInfo.roundNo][11])) % TWOSIXTEEN;
   // cout << "t0: " << hex << t0 << " t1: " << hex << t1 << endl;
   // cout << "f0: " << hex << f.f0 << " f1: " << hex << f.f1 << endl;
   return f;
}

roundInfo encrypt(uint16_t subkeyVals[][12], roundInfo rInfo, bool gradMode) {
    uint16_t tmp1, tmp2;
    fInfo fFuncReturn;
    rInfo.roundNo = 0;
    for (int i = 0; i < 16; i++) {    
        // cout << "round " << i << " go!" << endl;
        fFuncReturn = fFunc(rInfo, subkeyVals);
        tmp1 = rInfo.r0;
        tmp2 = rInfo.r1;
        rInfo.r0 = rInfo.r2 ^ fFuncReturn.f0;
        rInfo.r1 = rInfo.r3 ^ fFuncReturn.f1;
        rInfo.r2 = tmp1;
        rInfo.r3 = tmp2;
        rInfo.roundNo++;
        // cout << "data for the round: \n\tr0: " << rInfo.r0 << " r1: " << rInfo.r1 << " r2: " << rInfo.r2 << " r3: " << rInfo.r3 << " rNo: " << rInfo.roundNo << endl; 
    }
    return rInfo;
}

uint64_t processBlock(uint64_t block, bitset<KEYSIZE> key, uint16_t subkeyVals[][12], bool gradMode) {
    roundInfo rInfo = whitenInput(block, key);
    rInfo = encrypt(subkeyVals, rInfo, gradMode);
    uint64_t intCipherText = (uint64_t(rInfo.r0) << 16) + uint64_t(rInfo.r1) + (uint64_t(rInfo.r2) << 48) + (uint64_t(rInfo.r3) << 32);
    return whitenOutput(intCipherText, key);
}

void makeGradSubkeys(bitset<GRADKEYSIZE> *key, uint16_t subkeys[GRADNUMROUNDS][12], uint16_t decSubkeys[GRADNUMROUNDS][12]) {
    for (int i = 0; i < GRADNUMROUNDS; i++) {
        for (int j = 0; j < 12; j++) {
            subkeys[i][j] = gradKeyFunc(key, (4*i) + (j % 4));
        }
    }
    int k = 0;
    for (int i = GRADNUMROUNDS - 1; i >= 0; i--) {
        for (int j = 0; j < 12; j++) {
            decSubkeys[k][j] = subkeys[i][j];
        }
        k++;
    }
}

void makeSubkeys(bitset<KEYSIZE> *key, uint16_t subkeys[NUMROUNDS][12], uint16_t decSubkeys[NUMROUNDS][12]) {
    for (int i = 0; i < NUMROUNDS; i++) {
        for (int j = 0; j < 12; j++) {
            subkeys[i][j] = keyFunc(key, (4*i) + (j % 4));
        }
    }
    int k = 0;
    for (int i = NUMROUNDS - 1; i >= 0; i--) {
        for (int j = 0; j < 12; j++) {
            decSubkeys[k][j] = subkeys[i][j];
        }
        k++;
    }
}

int getCharCnt(string inputFile) {
    int charCnt = 0;
    char scratch;
    ifstream plainTextFile;
    plainTextFile.open(inputFile, ios::in);
    while (plainTextFile >> noskipws >> scratch) {
        charCnt++;
    }
    plainTextFile.close();
    return charCnt;
}

void padFile(string inputFile, int charCnt, int pad) {
    string padding;
    ofstream plainTextFile;
    plainTextFile.open(inputFile, ios::app);
    for (int i = 0; i < pad-1; i++) {
        padding += '0';
    }
    padding += to_string(pad);
    plainTextFile << padding;
    plainTextFile.close();
}

void padInput(string inputFile) {
    int charCnt = getCharCnt(inputFile);
    int pad = 8 - (charCnt % 8);
    padFile(inputFile, charCnt, pad);
}

string getKey(string keyFilePath) {
    string key;
    char curChar;
    ifstream keyFile;
    keyFile.open(keyFilePath, ios::in);
    while (keyFile >> noskipws >> curChar) {
        key += curChar;
    }
    keyFile.close();
    return key;
}

bitset<GRADKEYSIZE> makeGradKeyFromStr(string keyStr) {
    bitset<GRADKEYSIZE> gradKey;
    string subKey;
    for (int i = 0; i < 18; i++) {
        subKey += keyStr[i];
    }
    bitset<64> preKey = stoull(subKey, nullptr, 16);
    subKey.clear();
    for (int i = 0; i < 64; i++) {
        gradKey[i] = preKey[i];
    }
    gradKey <<= 16;
    subKey = "0x";
    for (int i = 18; i < keyStr.size(); i++) {
        subKey += keyStr[i];
    }
    preKey = stoull(subKey, nullptr, 16);
    for (int i = 0; i < 16; i++) {
        gradKey[i] = preKey[i];
    }
    return gradKey;
}

void processAllBlocks(string plainFilePath, string cipherFilePath, bitset<KEYSIZE> key, uint16_t subkeyVals[][12], bool gradMode) {
    char curChar;
    string block;
    uint64_t blockNum = 0;
    ifstream plainTextFile;
    ofstream outputFile;

    padInput(plainFilePath);
    plainTextFile.open(plainFilePath, ios::in);
    outputFile.open(cipherFilePath, ios::app);
    while (plainTextFile >> noskipws >> curChar) {
        block += curChar;
        if (block.size() % 8 == 0) {
            for (int i = 0; i < block.size(); i++) {
                blockNum += uint64_t(block[i]) << ((7-i) * 8);
            }
            uint64_t cipherBlock = processBlock(blockNum, key, subkeyVals, gradMode);
            outputFile << hex << cipherBlock;
            block.clear();
            blockNum = 0;
        }
    }
    outputFile.close();
    plainTextFile.close();
}

int main(int argc, char *argv[]) {
    string keyFilePath = "test/key.txt";
    string plainFilePath = "test/testPlain2.txt";
    string cipherFilePath = "test/ciphertext.txt";
    string keyStr = getKey(keyFilePath);
    bool gradMode = keyStr.size() > 18 ? true : false;
    uint16_t subkeyVals[NUMROUNDS][12];
    uint16_t decSubkeyVals[NUMROUNDS][12];
    uint16_t gradSubkeyVals[GRADNUMROUNDS][12];
    uint16_t gradDecSubkeyVals[GRADNUMROUNDS][12];
    if (!gradMode) {
        bitset<KEYSIZE> key = stoull(keyStr, nullptr, 16);
        makeSubkeys(&key, subkeyVals, decSubkeyVals);
        processAllBlocks(plainFilePath, cipherFilePath, key, subkeyVals, gradMode);
    } else {
        bitset<GRADKEYSIZE> gradKey = makeGradKeyFromStr(keyStr);
        makeGradSubkeys(&gradKey, gradSubkeyVals, gradDecSubkeyVals);
        bitset<KEYSIZE> key;
        gradKey >>= 16;
        for (int i = 0; i < KEYSIZE; i++) {
            key[i] = gradKey[i];
        }
        processAllBlocks(plainFilePath, cipherFilePath, key, gradSubkeyVals, gradMode);
    }
    
    return 1;
}
