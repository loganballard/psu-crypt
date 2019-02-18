#include <string>
#include <bitset>
#include <iostream>
#include <fstream>
#include "constants.h"

using namespace std;

int getCharCnt(string inputFile) {
    int charCnt = 0;
    ifstream plainTextFile;
    plainTextFile.open(inputFile, ios::in);
    while (!plainTextFile.eof()) {
        plainTextFile.get();
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

int main(int argc, char *argv[]) {
    ifstream plainTextFile;
    padInput("test/testPlain.txt");
    plainTextFile.open("test/testPlain.txt", ios::in);
    string block;
    char curChar;
    uint64_t blockNum;
    while (!plainTextFile.eof()) {
        plainTextFile.get(curChar);
        block += curChar;
        if (block.size() % 8 == 0) {
            for (int i = 0; i < block.size(); i++) {
                blockNum += uint64_t(block[i]) << ((7-i) * 8);
            }
            // TODO - PROCESS BLOCK!
            cout << "block: " << block << " blocknum: " << hex << blockNum << endl;
            block = "";
        }
    }
    plainTextFile.close();
    return 1;
}
