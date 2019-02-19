#include <string>
#include <bitset>
#include <iostream>
#include <stdlib.h>
#include <limits>
#include <sstream>
#include <fstream>
#include <math.h>

#include "encFun.h"

const bool DEBUG = true;

using namespace std;

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
    for (int i = 0; i < 16; i++) {
        subKey += keyStr[i];
    }
    bitset<64> preKey = stoull(subKey, nullptr, 16);
    subKey.clear();
    for (int i = 0; i < 64; i++) {
        gradKey[i] = preKey[i];
    }
    cout << gradKey << endl;
    gradKey <<= 16;
    cout << gradKey << endl;
    for (int i = 16; i < 20; i++) {
        subKey += keyStr[i];
    }
    preKey = stoull(subKey, nullptr, 16);
    for (int i = 0; i < 16; i++) {
        gradKey[i] = preKey[i];
    }
    return gradKey;
}

void encProcessAllBlocks(string readFilePath, string writeFilePath, bitset<KEYSIZE> key, uint16_t subkeyVals[][12], bool gradMode) {
    char curChar;
    string block;
    uint64_t blockNum = 0;
    ifstream inputFile;
    ofstream outputFile;
    
    inputFile.open(readFilePath, ios::binary);
    padInput(readFilePath);
    outputFile.open(writeFilePath, ios::app);
    while (inputFile >> noskipws >> curChar) {
        block += curChar;
        if (block.size() % 8 == 0) {
            for (int i = 0; i < block.size(); i++) {
                blockNum += uint64_t(block[i]) << ((7-i) * 8);
            }
            uint64_t cipherBlock = processBlock(blockNum, key, subkeyVals, gradMode);
            stringstream w;
            w << hex << cipherBlock;
            string write = w.str();
            if (write.size() == 15) {
                write = "0" + write;
            }
            outputFile << write << "\n";
            block.clear();
            blockNum = 0;
        }
    }
    outputFile.close();
    inputFile.close();
}

void decProcessAllBlocks(string readFilePath, string writeFilePath, bitset<KEYSIZE> key, uint16_t subkeyVals[][12], bool gradMode) {
    char curChar;
    string block;
    uint64_t blockNum = 0;
    ifstream inputFile;
    fstream outputFile;
    
    inputFile.open(readFilePath, ios::binary);
    outputFile.open("./tmp.txt", ios::app);
    while (inputFile >> noskipws >> curChar) {
        block += curChar;
        if (curChar == '\n') {
            blockNum = stoull(block, nullptr, 16);
            uint64_t cipherBlock = processBlock(blockNum, key, subkeyVals, gradMode);  
            stringstream w;
            w << hex << cipherBlock;
            string write = w.str();
            if (write.size() == 15) {
                write = "0" + write;
            }
            outputFile << write;
            block.clear();
            blockNum = 0;
        }
    }
    outputFile.close();
    string convToASCII;
    outputFile.open("./tmp.txt", ios::in);
    while (outputFile >> noskipws >> curChar) {
        convToASCII += curChar;
    }
    outputFile.close();
    outputFile.open(writeFilePath, ios::out | ofstream::trunc);
    for(int i = 0; i < convToASCII.size(); i += 2) {
        string byte = convToASCII.substr(i,2);
        char chr = (char) stoull(byte, nullptr, 16);
        outputFile << chr;
    }
    outputFile.close();
    inputFile.close();
}

void functionalityWrapper(string keyFilePath, string plainFilePath, string cipherFilePath, string decryptPath, bool encrypt) {
    string keyStr = getKey(keyFilePath);
    bool gradMode = keyStr.size() > 16 ? true : false;
    int numRounds = gradMode ? GRADNUMROUNDS : NUMROUNDS;
    uint16_t subkeyVals[numRounds][12];
    uint16_t decSubkeyVals[numRounds][12];
    bitset<KEYSIZE> key;
    if (gradMode) {
        bitset<GRADKEYSIZE> gradKey = makeGradKeyFromStr(keyStr);
        makeGradSubkeys(&gradKey, subkeyVals, decSubkeyVals);
        gradKey >>= 16;
        for (int i = 0; i < KEYSIZE; i++) {
            key[i] = gradKey[i];
        }
    } else {
        key = stoull(keyStr, nullptr, 16);
        makeSubkeys(&key, subkeyVals, decSubkeyVals);
    }
    if (encrypt) {
        encProcessAllBlocks(plainFilePath, cipherFilePath, key, subkeyVals, gradMode);
    } else {
        decProcessAllBlocks(cipherFilePath, decryptPath, key, decSubkeyVals, gradMode);
    }
}

int main(int argc, char *argv[]) {
    string keyFilePath = "test/key.txt";
    string plainFilePath = "test/rebTest.txt";
    string cipherFilePath = "test/ciphertext.txt";
    string decryptPath = "test/decrypt.txt";
    functionalityWrapper(keyFilePath, plainFilePath, cipherFilePath, decryptPath, true);
    functionalityWrapper(keyFilePath, plainFilePath, cipherFilePath, decryptPath, false);
    return 1;
}
