#pragma once

//#define INClEXPRTK;

#include <algorithm>
#include <iostream>
#include <string>
#include <string.h>
#include <array>
#include <vector>
#include <fstream>
#include <stdio.h>      /* printf, NULL */
#include <stdlib.h>     /* srand, rand */
#include <iomanip>
#include <iostream>
#include <filesystem>
#include "resource.h"

#ifdef INCLEXPRTK
#include "ThirdParty/exprtk.hpp"
#endif

//#include <sys/stat.h>
//#include <time.h>       /* time */
//#include <windows.h>

using namespace std;
using namespace std::filesystem;

extern const int StdSectorsPerDisk;                                          //Standard disk
extern const int StdTracksPerDisk;
extern const int StdBytesPerDisk;                 //including track 18

extern const int ExtSectorsPerDisk;                       //Exnteded disk
extern const int ExtTracksPerDisk;
extern const int ExtBytesPerDisk;                   //Including track 18

extern int BytePtr, BitPtr, NibblePtr, BitPos, BitsLeft;

//extern vector<array<unsigned char, 256>> ByteSt;
extern unsigned char ByteSt[] ;

extern int PartialFileIndex;
extern size_t PartialFileOffset;
extern bool FileUnderIO;
extern bool LastFileOfBundle;
extern const int SD_size;
extern unsigned char SD[];
extern const int Loader_size;
extern unsigned char Loader[];
extern const int SS_size;
extern unsigned char SS[];
extern const int SSIO_size;
extern unsigned char SSIO[];
extern const int Ascii2DirArt_size;
extern unsigned char Ascii2DirArt[];
extern const int Petscii2DirArt_size;
extern unsigned char Petscii2DirArt[];

struct FileStruct {
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
extern array<unsigned char, 256> Buffer;
extern int BufferCnt;
extern unsigned char FilesInBuffer;
extern bool NewBundle;
extern int BlockPtr;
extern unsigned char LastByte;
extern int BitsNeededForNextBundle;
extern bool NewBlock;
extern int BundleNo;
extern unsigned char DirBlocks[512];
extern int DirPtr[128];
extern int LastBlockCnt;
extern char LoaderBundles;
extern int BundleCnt;
extern int BlockCnt;
extern bool SetNewBlock;           //This will fire at the previous bundle and will set NewBlock2
extern bool NewBlock;              //This will fire at the specified bundle

extern const unsigned char NearLongMatchTag;    //= 0x84;
extern const unsigned char EndOfBundleTag;      //= 0x00;

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
//Functions
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

void PackFile(int Index);
void ResetBuffer();
bool CloseBuffer();
bool CloseFile();
bool CloseBundle(int NextFileIO, bool LastPartOnDisk);
unsigned char EORtransform(unsigned char Input);
//string ExprTK(string expression_string);
