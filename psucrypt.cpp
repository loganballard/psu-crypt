#include <string>
#include <bitset>
#include <iostream>
#include <stdlib.h>
#include <limits>
#include <sstream>
#include <fstream>
#include <math.h>

#include "helpers.h"

const bool DEBUG = true;

using namespace std;

uint64_t processBlock(uint64_t block, bitset<KEYSIZE> key, uint16_t subkeyVals[][12], bool gradMode) {
    roundInfo rInfo = whitenInput(block, key);
    rInfo = encrypt(subkeyVals, rInfo, gradMode);
    uint64_t intCipherText = (uint64_t(rInfo.r0) << 16) + uint64_t(rInfo.r1) + (uint64_t(rInfo.r2) << 48) + (uint64_t(rInfo.r3) << 32);
    return whitenOutput(intCipherText, key);
}

template <size_t keyS>
void circularLeftShift(bitset<keyS> *curKey) {
    uint8_t lastBit = (*curKey)[keyS-1];
    (*curKey) <<= 1;
    (*curKey)[0] = lastBit;
}

template <size_t keyS>
uint16_t keyFunc(bitset<keyS> *curKey, uint16_t x) {
    uint16_t outputByte = x % 8;
    uint16_t keyIndex = outputByte * 8;
    bitset<8> outputSet = 0;
    circularLeftShift(curKey);
    for (int i=0; i <= 7; i++) {
        outputSet[i] = (*curKey)[keyIndex++];
    }
    return uint16_t(outputSet.to_ulong());
}

template <size_t keyS>
void makeSubkeys(bitset<keyS> *key, uint16_t subkeys[][12], uint16_t decSubkeys[][12], int numRounds) {
    int k = numRounds - 1;
    for (int i = 0; i < numRounds; i++, k--) {
        for (int j = 0; j < 12; j++) {
            subkeys[i][j] = keyFunc(key, (4*i) + (j % 4));
            decSubkeys[k][j] = subkeys[i][j];
        }
    }
}

string makePadding(int pad) {
    string padding;
    for (int i = 0; i < pad-1; i++) {
        padding += "0";
    }
    padding += to_string(pad);
    return padding;
}

void padInput(string readFilePath, string paddedPlainPath) {
    ifstream inputFile;
    ofstream outputFile;
    inputFile.open(readFilePath, ios::in);
    outputFile.open(paddedPlainPath, ios::out | ofstream::trunc);
    int charCnt = 0;
    char curChar;
    while (inputFile >> noskipws >> curChar) {
        charCnt++;
        outputFile << curChar;
    }
    outputFile << makePadding(8 - (charCnt % 8));
    inputFile.close();
    outputFile.close();
}


// TODO ADD ERROR CHECKING + KEY NOT STARTING WITH 0X HANDLING
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

// TODO - THIS _CAN'T_ BE THE BEST WAY TO DO THIS... 
bitset<GRADKEYSIZE> makeGradKeyFromStr(string keyStr) {
    bitset<GRADKEYSIZE> gradKey;
    string subKey;
    for (int i = 0; i < 16; i++) {
        subKey += keyStr[i];
    }
    bitset<64> preKey = stoull(subKey, nullptr, 16);
    gradKey = preKey.to_ullong();
    gradKey <<= 16;
    subKey.clear();
    for (int i = 16; i < 20; i++) {
        subKey += keyStr[i];
    }
    preKey = stoull(subKey, nullptr, 16);
    for (int i = 0; i < 16; i++) {
        gradKey[i] = preKey[i];
    }
    return gradKey;
}

string leftZeroPadHexBlock(string str, int size) {
    while (str.size() != size) {
        str = "0" + str;
    }
    return str;
}

void encProcessAllBlocks(string readFilePath, string writeFilePath, bitset<KEYSIZE> key, uint16_t subkeyVals[][12], bool gradMode) {
    char curChar;
    string block;
    uint64_t blockNum = 0;
    ofstream outputFile;
    ifstream inputFile;
    
    padInput(readFilePath, TMPFILE);
    inputFile.open(TMPFILE, ios::in);
    outputFile.open(writeFilePath, ios::out | ofstream::trunc);
    while (inputFile >> noskipws >> curChar) {
        block += curChar;
        if (block.size() % 8 == 0) {
            for (int i = 0; i < block.size(); i++) {
                blockNum += uint64_t(block[i]) << ((7-i) * 8);
            }
            stringstream w;
            w << hex << processBlock(blockNum, key, subkeyVals, gradMode);
            outputFile << leftZeroPadHexBlock(w.str(), 16);
            block.clear();
            blockNum = 0;
        }
    }
    inputFile.close();
    outputFile.close();
    if  (remove(TMPFILE) != 0) {
        cout << "something went wrong removing files!" << endl;
        exit(1);
    }
}

string processDecText(string convToASCII) {
    string ascii;
    string byte;
    int trim;
    for (int i = 0; i < convToASCII.size(); i += 2) {
        byte = convToASCII.substr(i,2);
        ascii += (char) stoull(byte, nullptr, 16);
    }
    trim = ascii[ascii.size()-1] - '0';
    return ascii.erase(ascii.size() - trim);
}

void decProcessAllBlocks(string readFilePath, string writeFilePath, bitset<KEYSIZE> key, uint16_t subkeyVals[][12], bool gradMode) {
    char curChar;
    string block;
    ifstream inputFile;
    fstream outputFile;
    string convToASCII;

    inputFile.open(readFilePath, ios::in);
    outputFile.open(writeFilePath, ios::out | ofstream::trunc);
    while (inputFile >> noskipws >> curChar) {
        block += curChar;
        if (block.size() == 16) {
            uint64_t cipherBlock = processBlock(stoull(block, nullptr, 16), key, subkeyVals, gradMode);  
            stringstream w;
            w << hex << cipherBlock;
            convToASCII += leftZeroPadHexBlock(w.str(), 16);
            block.clear();
        }
    }
    outputFile << processDecText(convToASCII);
    outputFile.close();
    inputFile.close();
}

void wrapper(string keyFilePath, string readFilePath, string writeFilePath, bool encrypt) {
    string keyStr = getKey(keyFilePath);
    bool gradMode = keyStr.size() > 16 ? true : false;
    int numRounds = gradMode ? GRADNUMROUNDS : NUMROUNDS;
    uint16_t subkeyVals[numRounds][12];
    uint16_t decSubkeyVals[numRounds][12];
    bitset<KEYSIZE> key;
    
    if (gradMode) {
        bitset<GRADKEYSIZE> gradKey = makeGradKeyFromStr(keyStr);
        makeSubkeys(&gradKey, subkeyVals, decSubkeyVals, GRADNUMROUNDS);
        key = (gradKey >>= 16).to_ullong();
    } else {
        key = stoull(keyStr, nullptr, 16);
        makeSubkeys(&key, subkeyVals, decSubkeyVals, NUMROUNDS);
    }
    if (encrypt) {
        encProcessAllBlocks(readFilePath, writeFilePath, key, subkeyVals, gradMode);
    } else {
        decProcessAllBlocks(readFilePath, writeFilePath, key, decSubkeyVals, gradMode);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        cout << "Wrong number of arguments supplied!" << endl;
        exit(1);
    }
    bool encrypt;
    string keyFilePath;
    string readFilePath;
    string writeFilePath;
    try {
        encrypt = stoi(argv[1]);
        keyFilePath = argv[2];
        readFilePath = argv[3];
        writeFilePath = argv[4];
    }
    catch (...) {
        cout << "Not sure what's going on but something went wrong!" << endl;
        exit(1);
    }
    wrapper(keyFilePath, readFilePath, writeFilePath, encrypt);
    exit(0);
}
