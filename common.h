#pragma once

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
#include <sstream>
#include <filesystem>
#include "thirdparty/lodepng.h"
#include "thirdparty/tinyexpr.h"

using namespace std;

namespace fs = filesystem;

extern const int StdSectorsPerDisk;                 //Standard disk
extern const int StdTracksPerDisk;
extern const int StdBytesPerDisk;                   //Includes track 18

extern const int ExtSectorsPerDisk;                 //Extended disk
extern const int ExtTracksPerDisk;
extern const int ExtBytesPerDisk;                   //Includes track 18

extern int SectorsPerDisk;
extern int BlocksUsedByPlugin;

extern int BytePtr, BitPtr, NibblePtr, BitPos, BitsLeft;

extern unsigned char ByteSt[] ;

extern int PartialFileIndex;
extern size_t PartialFileOffset;
extern bool FileUnderIO;
extern bool LastFileOfBundle;
extern const int sd_size;
extern unsigned char sd[];
extern const int loader_size;
extern unsigned char loader[];
extern const int ss_size;
extern unsigned char ss[];
extern const int ssio_size;
extern unsigned char ssio[];
extern const int sf_size;
extern unsigned char sf[];
extern const int ascii2dirart_size;
extern unsigned char ascii2dirart[];
extern const int petscii2dirart_size;
extern unsigned char petscii2dirart[];
extern const int pixelcnttab_size;
extern unsigned char pixelcnttab[];
extern const int charsettab_size;
extern unsigned char charsettab[];
extern const int sc_size;
extern unsigned char sc[];

struct PluginStruct {
    bool HasDirIndex;
    int PluginDirIndex;
    int PluginType;
    string HSFileName;
    string HSFileAddress;
    string HSFileOffset;
    string HSFileLength;

    PluginStruct(bool HasDirIndex, int PluginDirIndex, int PluginType, string HSFileName, string HSFileAddress, string HSFileOffset, string HSFileLength)
    {
        this->HasDirIndex = HasDirIndex;
        this->PluginDirIndex = PluginDirIndex;
        this->PluginType = PluginType;
        this->HSFileName = HSFileName;
        this->HSFileAddress = HSFileAddress;
        this->HSFileOffset = HSFileOffset;
        this->HSFileLength = HSFileLength;
    }
};

extern vector <PluginStruct> PluginFiles;

struct FileStruct {
    vector<unsigned char> Prg;
    string FileName;
    string FileAddr;
    string FileOffs;
    string FileLen;
    bool FileIO;
    bool FileUncompressed;
    int iFileAddr;
    int iFileOffs;
    int iFileLen;

    FileStruct(vector<unsigned char> Prg, string FileName, string FileAddr, string FileOffs, string FileLen, bool FileIO, bool FileUncompressed)
    {
        this->Prg = Prg;
        this->FileName = FileName;
        this->FileAddr = FileAddr;
        this->FileOffs = FileOffs;
        this->FileLen = FileLen;
        this->FileIO = FileIO;
        this->FileUncompressed = FileUncompressed;
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

bool PackFile(int Index);
void ResetBuffer(bool FirstBlock = false);
bool CloseBuffer();
bool CloseFile();
bool CloseBundle(int NextFileIO, bool LastPartOnDisk);
unsigned char EORtransform(unsigned char Input);
