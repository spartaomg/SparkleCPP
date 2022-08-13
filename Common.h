#pragma once
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sys/stat.h>
#include <stdio.h>      /* printf, NULL */
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <iomanip>
#include <iostream>
#include <filesystem>
using namespace std;
using namespace std::filesystem;

extern int BytePtr, BitPtr;
extern int PartialFileIndex, PartialFileOffset;
extern bool FileUnderIO;
extern bool LastFileOfBundle;
extern const int SD_size;
extern unsigned char SD[];
extern const int SL_size;
extern unsigned char SL[];
extern const int SS_size;
extern unsigned char SS[];
extern const int SSIO_size;
extern unsigned char SSIO[];
extern const int Ascii2DirArt_size;
extern unsigned char Ascii2DirArt[];
extern const int Petscii2DirArt_size;
extern unsigned char Petscii2DirArt[];

extern int BitPos;
extern int BitsLeft;

extern struct FileStruct {
    vector<unsigned char> Prg;
    string FileName;
    string FileAddr;
    string FileOffs;
    string FileLen;
    bool FileIO;
    int iFileAddr;
    int iFileOffs;
    int iFileLen;


    FileStruct(vector<unsigned char> Prg, string FileName, string FileAddr, string FileOffs, string FileLen, bool FileIO) {
        this->Prg = Prg;
        this->FileName = FileName;
        this->FileAddr = FileAddr;
        this->FileOffs = FileOffs;
        this->FileLen = FileLen;
        this->FileIO = FileIO;
        this->iFileAddr = stoul(FileAddr, nullptr, 16);
        this->iFileOffs = stoul(FileOffs, nullptr, 16);
        this->iFileLen = stoul(FileLen, nullptr, 16);
    };
};

extern vector <FileStruct> Prgs;
extern vector <FileStruct> tmpPrgs;
extern vector <FileStruct> VFiles;
extern vector <FileStruct> tmpVFiles;

extern int PrgAdd, PrgLen;
extern unsigned char Buffer[256];

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

bool CloseBuffer();
bool CloseFile();
void PackFile(int Index);
