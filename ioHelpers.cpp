#include <string>
#include <bitset>
#include <iostream>
#include <fstream>
#include "constants.h"

using namespace std;

uint64_t makeBlock(string block) {
    // TODO - READ ASCII VAL EACH CHAR
    uint64_t blockNum = 0;
    for (int i = 0; i < block.size(); i++) {
        uint64_t c = block[i] << (i*8);
        blockNum += c;
    }
    return blockNum;
}


bitset<ALTKEYSIZE> makeHexStrToKey(string keyStr) {
    bitset<ALTKEYSIZE> k;
    bitset<8> byte = 0;
    string hexPre = "0x";
    string charInd;
    int ind = 0;
    uint16_t intermediateNum;
    for (int i = 2; i < keyStr.size()-1; i += 2) {
        charInd = hexPre + keyStr[i] + keyStr[i+1];
        intermediateNum = uint16_t(stoul(charInd, nullptr, 16));
        byte = intermediateNum;
        for (int j = 0; j < 8; j++) {
            k[ind] = byte[j];
            ind++;
        }
    }
    cout << k << endl;
    return k;
}

int main(int argc, char *argv[]) {
    ifstream plainTextFile;
    plainTextFile.open("test/testPlain.txt", ios::in);
    string block;
    char curChar;
    while (!plainTextFile.eof()) {
        plainTextFile.get(curChar);
        block += curChar;
        if (block.size() % 8 == 0) {
            cout << "block!" << endl;
        }
    }
    plainTextFile.close();
    return 1;
}