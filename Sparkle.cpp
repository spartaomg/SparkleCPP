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
#include "Sparkle.h"
#include <iostream>
#include <filesystem>

using namespace std;
using namespace std::filesystem;

string Script = "";
string ScriptPath = "";
string ScriptName = "";
string ScriptEntry = "";
string ScriptLine = "";
string ScriptEntryType = "";
string ScriptHeader = "[sparkle loader script]";
const int MaxNumEntries = 5;
string ScriptEntryArray[MaxNumEntries]{};
int NumScriptEntries = -1;

const int StdSectorsPerDisk = 664;                                          //Standard disk
const int StdTracksPerDisk = 35;
const int StdBytesPerDisk = 174848;                                         //including track 18

const int ExtSectorsPerDisk = StdSectorsPerDisk + 85;                       //Exnteded disk
const int ExtTracksPerDisk = 40;
const int ExtBytesPerDisk = StdBytesPerDisk + (85 * 256);                   //Including track 18

int SectorsPerDisk = StdSectorsPerDisk;
int TracksPerDisk = StdTracksPerDisk;

int BlocksFree = SectorsPerDisk;

unsigned char TabT[ExtSectorsPerDisk], TabS[ExtSectorsPerDisk], TabSCnt[ExtSectorsPerDisk], TabStartS[ExtTracksPerDisk+1];  //TabStartS is 1 based

char Disk[ExtBytesPerDisk];
unsigned char NextTrack;
unsigned char NextSector;                                                   //Next Empty Track and Sector
unsigned char MaxSector = 18;
unsigned char LastSector;

string Prg, ReferenceFile;

bool TmpSetNewBlock = false;
bool SetNewBlock = false;           //This will fire at the previous bundle and will set NewBlock2
bool NewBlock = false;              //This will fire at the specified bundle

string EntryTypePath = "path:";
string EntryTypeHeader = "header:";
string EntryTypeID = "id:";
string EntryTypeName = "name:";
string EntryTypeStart = "start:";
string EntryTypeDirArt = "dirart:";
string EntryTypeZP = "zp:";
string EntryTypeIL0 = "il0:";
string EntryTypeIL1 = "il1:";
string EntryTypeIL2 = "il2:";
string EntryTypeIL3 = "il3:";
string EntryTypeProdID = "prodid:";
string EntryTypeTracks = "tracks:";
string EntryTypeHSFile = "hsfile:";
string EntryTypeScript = "script:";
string EntryTypeFile = "file:";
string EntryTypeMem = "mem:";
string EntryTypeAlign = "align";

int ProductID = 0;
unsigned int SS, SE, LastSS, LastSE;
bool NewBundle;

const int MaxNumDisks = 127;
int DiskSizeA(MaxNumDisks);

int DiskCnt = -1;
int BundleCnt = -1;
int FileCnt = -1;
int VFileCnt = -1;
int CurrentDisk = -1;
int CurrentBundle = -1;
int CurrentFile = -1;
int CurrentScript = -1;
int BundleNo = -1;
bool MaxBundleNoExceeded = false;

vector<string> Prgs;
vector<string> FileNameV;
vector<string> FileAddrV;
vector<string> FileOffsV;
vector<string> FileLenV;
vector<bool> FileIOV;

vector<string> tmpPrgs;
vector<string> tmpFileNameV;
vector<string> tmpFileAddrV;
vector<string> tmpFileOffsV;
vector<string> tmpFileLenV;
vector<bool> tmpFileIOV;

vector<string> VFiles;
vector<string> VFileNameV;
vector<string> VFileAddrV;
vector<string> VFileOffsV;
vector<string> VFileLenA;
vector<bool> VFileIOV;

vector<string> tmpVFiles;
vector<string> tmpVFileNameV;
vector<string> tmpVFileAddrV;
vector<string> tmpVFileOffsV;
vector<string> tmpVFileLenV;
vector<bool> tmpVFileIOV;

unsigned char DirBlocks[512]{};
unsigned char DirPtr[128]{};

//Hi-Score File variables
string HSFileName = "";
vector<unsigned char> HSFile;
int HSAddress = 0;
int HSOffset = 0;
int HSLength = 0;
bool bSaverPlugin =false;

//Interleave constants and variables
const unsigned char DefaultIL0 = 4;
const unsigned char DefaultIL1 = 3;
const unsigned char DefaultIL2 = 3;
const unsigned char DefaultIL3 = 3;
unsigned char IL0 = DefaultIL0;
unsigned char IL1 = DefaultIL1;
unsigned char IL2 = DefaultIL2;
unsigned char IL3 = DefaultIL3;

int BufferCnt = 0;

string D64Name = "";
string DiskHeader = "";
string DiskID = "";
string DemoName = "";
string DemoStart = "";
string LoaderZP = "02";
string DirArt = "";

int DirTrack, DirSector, DirPos;
string DirArtName = "";
string DirEntry = "";

int BlockPtr;
unsigned char LastBlockCnt = 0;
int LoaderBundles = 1;
unsigned char FilesInBuffer = 1;

int Track[41]{}, CT, CS, BlockCnt;
unsigned char StartTrack = 1;
unsigned char StartSector = 0;

bool FirstFileOfDisk = false;
string FirstFileStart = "";

int TotalBundles = 0;
vector<int> BundleSizeV;
vector<int> BundleOrigSizeV;
double UncompBundleSize = 0;


//------------------------------------------------------------------------------------------------------------------------------------------------------------

string ConvertIntToHextString(const int& i, const int& hexlen)
{
    char hexchar[8];
    sprintf_s(hexchar, "%X", i);
    string hexstring = "";
    for (int j = 0; j < hexlen; j ++)
    {
        hexstring = hexchar[7 - j] + hexstring;
    }
    return hexstring;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------

int ConvertStringToInt(const string& s)
{
    return stoul(s, nullptr, 10);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------

int ConvertHexStringToInt(const string& s)
{
    return stoul(s, nullptr, 16);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------

bool IsHexString(const string& s)
{
    return !s.empty() && all_of(s.begin(), s.end(), [](unsigned char c) { return std::isxdigit(c); });
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------

bool IsNumeric(const string& s)
{
    return !s.empty() && find_if(s.begin(),s.end(), [](unsigned char c) { return !isdigit(c); }) == s.end();
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------

inline bool FileExits(const string& FileName)
{
    struct stat buffer;
    return (stat(FileName.c_str(), &buffer) == 0);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

int ReadBinaryFile(const string& FileName, vector<unsigned char>& prg)
{

    if (!FileExits(FileName))
    {
        return -1;
    }

    prg.clear();

    ifstream infile(FileName, ios_base::binary);

    infile.seekg(0, ios_base::end);
    int length = infile.tellg();
    infile.seekg(0, ios_base::beg);

    prg.reserve(length);

    copy(istreambuf_iterator<char>(infile), istreambuf_iterator<char>(), back_inserter(prg));

    return length;
}


string ReadFileToString(const string& FileName)
{

    if (!FileExits(FileName))
    {
        return "";
    }

    ifstream t(FileName);

    string str;

    t.seekg(0, ios::end);
    str.reserve(t.tellg());
    t.seekg(0, ios::beg);

    str.assign((istreambuf_iterator<char>(t)), istreambuf_iterator<char>());

    return str;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

void WriteDiskImage(const string& DiskName)
{
    int BytesPerDisk;
    if (TracksPerDisk == StdTracksPerDisk)
    {
        BytesPerDisk = StdBytesPerDisk;
    }
    else
    {
        BytesPerDisk = ExtBytesPerDisk;
    }

    ofstream myFile(DiskName, ios::out | ios::binary);
    myFile.write(Disk, BytesPerDisk);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool ParameterIsNumeric(int i)
{
    //Remove unwanted spaces
        while (ScriptEntryArray[i].find(" ") != string::npos)
    {
        int Pos = ScriptEntryArray[i].find(" ");
        ScriptEntryArray[i].replace(Pos, 1, "");
    }

    //Remove HEX prefix
    if (ScriptEntryArray[i].at(0) == '$')
    {
        ScriptEntryArray[i].replace(0, 1, "");
    }

    string s = ScriptEntryArray[i].substr(0, 2);

    if ((s == "0x") || (s == "0X")|| (s == "&h")|| (s == "&H"))
    {
        ScriptEntryArray[i].replace(0, 2, "");
    }

    //If decimal -> convert it to hex
    if (ScriptEntryArray[i].at(0) == '.')
    {
        ScriptEntryArray[i].replace(0, 1, "");
        if (IsNumeric(ScriptEntryArray[i]))
        {
            int ScriptEntryInt = ConvertStringToInt(ScriptEntryArray[i]);
            int hexlen;
            if (i == 2)
            {
                hexlen = 8;     //Offset
            }
            else
            {
                hexlen = 4;     //Address, Length
            }
            ScriptEntryArray[i] = ConvertIntToHextString(ScriptEntryInt, hexlen);
        }
        else
        {
            return false;
        }
    }

    return IsHexString(ScriptEntryArray[i]);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

void CorrectParameterStringLength(const int& i)
{
    int MaxNumChars = 0;
    if (i == 2)
    {
        MaxNumChars = 8;    //File Offset max. $ffff ffff (dword)
    }
    else
    {
        MaxNumChars = 4;    //File Address, File Length max. $ffff
    }

    if (ScriptEntryArray[i].size() < MaxNumChars)
    {
        for (int j = ScriptEntryArray[i].size(); j < MaxNumChars; j++)
        {
            ScriptEntryArray[i] = "0" + ScriptEntryArray[i];
        }
    }
    else if (ScriptEntryArray[i].size() > MaxNumChars)
    {
        ScriptEntryArray[i].erase(0, ScriptEntryArray[i].size() - MaxNumChars);
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool AddHSFile()
{
    string FN = ScriptEntryArray[0];
    string FA = "";
    string FO = "";
    string FL = "";
    int FAN = 0;
    int FON = 0;
    int FLN = 0;
    bool FUIO = false;

    vector<unsigned char> P;

    if (FN.find(":") == string::npos)
    {
        FN = ScriptPath + FN;
    }

    int NumParams = 1;

    for (int i = 1; i <= NumScriptEntries; i++)
    {
        if (ParameterIsNumeric(i))
        {
            NumParams++;
        }
        else
        {
            break;
        }
        CorrectParameterStringLength(i);
    }

    //Get file variables from script, or get default values if there were none in the script entry
    if (FN.find("*") == FN.length()-1)
    {
        FUIO = true;
        FN.replace(FN.length() - 1, 1, "");
    }
    if (FileExits(FN))
    {
        HSFile.clear();
        ReadBinaryFile(FN, HSFile);

        switch (NumParams)
        {
        case 1:  //No parameters in script
            if (HSFile.size() > 2)                                              //We have at least 3 bytes in the file
            {
                FA = ConvertIntToHextString(HSFile[0] + (HSFile[1] * 256), 4);  //First 2 bytes define load address
                FO = "00000002";                                                //Offset=2, Length=prg length-2
                FL = ConvertIntToHextString(HSFile.size() - 2, 4);
            }
            else                                                                //Short file without paramters -> HARD STOP
            {
                cout << "***CRITICAL***\nFile paramteres are needed for the Hi-Score File: " << FN << "\n";
                return false;
            }
            break;
        case 2:  //One parameter in script
            FA = ScriptEntryArray[1];                                   //Load address from script
            FO = "00000000";                                            //Offset will be 0, length=prg length
            FL = ConvertIntToHextString(HSFile.size(), 4);
            break;

        case 3:  //Two parameters in script
            FA = ScriptEntryArray[1];                                   //Load address from script
            FO = ScriptEntryArray[2];                                   //Offset from script
            FON = ConvertHexStringToInt(FO);                            //Make sure offset is valid
            if (FON > HSFile.size() - 1)
            {
                FON = HSFile.size() - 1;                                //If offset>prg length-1 then correct it
                FO = ConvertIntToHextString(FON, 8);
            }
            FL = ConvertIntToHextString(HSFile.size() - FON, 4);        //Length=prg length-offset
            break;

        case 4:  //Three parameters in script
            FA = ScriptEntryArray[1];                                   //Load address from script
            FO = ScriptEntryArray[2];                                   //Offset from script
            FL = ScriptEntryArray[3];                                   //Length from script
            FON = ConvertHexStringToInt(FO);                            //Make sure offset is valid
            if (FON > HSFile.size() - 1)
            {
                FON = HSFile.size() - 1;                                //If offset>prg length-1 then correct it
                FO = ConvertIntToHextString(FON, 8);
            }
        }

        FAN = ConvertHexStringToInt(FA);
        FON = ConvertHexStringToInt(FO);
        FLN = ConvertHexStringToInt(FL);

        //Make sure file length is not longer than actual file (should not happen)
        if (FON + FLN > HSFile.size())
        {
            FLN = HSFile.size() - FON;
        }

        //Make sure file address+length<=&H10000
        if (FAN + FLN > 0x10000)
        {
            FLN = (0x10000 - FAN);// & 0xf00;
            //if (FLN < 0x100)
            //{
            //    cout << "***CRITICAL***\nThe Hi-Score File's size must be at least $100 bytes!\n";
            //        return false;
            //}
        }

        //Round UP to nearest $100, at least $100 but not more than $0f00 bytes
        if ((FLN % 0x100 != 0) || (FLN == 0))
        {
            FLN += 0x100;
        }

        FLN &= 0xf00;

        FL = ConvertIntToHextString(FLN, 4);

        vector<unsigned char>::iterator First = HSFile.begin();
        HSFile.erase(First + FON + FLN, First + HSFile.size() - 1); //Trim HSFile from end of file (Offset+Length-1) to end of vector
        HSFile.erase(First, First + FON - 1);                       //Trim HSFile from beginning of file to Offset

        HSFileName = FN + ((FUIO) ? "*" : "");
        HSAddress = FAN;
        HSOffset = FON;
        HSLength = FLN;

        bSaverPlugin = true;
    }
    else
    {
        //HSFile does not exist, see if we can create a blank HSFile using the provided parameters...
        if(NumParams == 4)
        {
            FA = ScriptEntryArray[1];                                   //Load address from script
            FO = ScriptEntryArray[2];                                   //Offset from script
            FL = ScriptEntryArray[3];                                   //Length from script

            FAN = ConvertHexStringToInt(FA);
            FON = ConvertHexStringToInt(FO);
            FLN = ConvertHexStringToInt(FL);

            //Make sure file address+length<=&H10000
            if (FAN + FLN > 0x10000)
            {
                FLN = (0x10000 - FAN);// & 0xf00;
                //if (FLN < 0x100)
                //{
                //    cout << "***CRITICAL***\nThe Hi-Score File's size must be at least $100 bytes!\n";
                //        return false;
                //}
            }

            //Round UP to nearest $100, at least $100 but not more than $0f00 bytes
            if ((FLN % 0x100 != 0) || (FLN == 0))
            {
                FLN += 0x100;
            }

            FLN &= 0xf00;

            FL = ConvertIntToHextString(FLN, 4);

            HSFile.reserve(FLN);
            HSFile.resize(FLN, 0);

            HSFileName = FN + ((FUIO) ? "*" : "");
            HSAddress = FAN;
            HSOffset = FON;
            HSLength = FLN;

            bSaverPlugin = true;
        }
        else
        {
            cout << "***CRITICAL***\nThe Hi-Score File " << HSFileName << " doesn't exist and an empty Hi-Score File could not be created without all 3 parameters.\n";
            return false;
        }
    }

    return true;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

void UpdateDiskSizeOnTheFly()
{
    if (TracksPerDisk == ExtTracksPerDisk)
    {
        BlocksFree = ExtSectorsPerDisk;
        SectorsPerDisk = ExtSectorsPerDisk;

        for (int i = (36 + 7) * 4; i < ((41 + 7) * 4); i++)
        {
            Disk[Track[18] + i] = 0xff;
        }
        for (int i = 36; i <= 40; i++)
        {
            Disk[Track[18] + ((i + 7) * 4) + 0] = 17;
            Disk[Track[18] + ((i + 7) * 4) + 3] = 1;
        }
    }
    else
    {
        BlocksFree = StdSectorsPerDisk;
        SectorsPerDisk = StdSectorsPerDisk;

        for (int i = (36 + 7) * 4; i < ((41 + 7) * 4); i++)
        {
            Disk[Track[18] + i] = 0;
        }
        for (int i = 36; i <= 40; i++)
        {
            Disk[Track[18] + ((i + 7) * 4) + 0] = 0;
            Disk[Track[18] + ((i + 7) * 4) + 3] = 0;
        }
    }
}

/*
void SetMaxSector() {

    if (CT < 18)
    {
        MaxSector = 20;
    }
    else if (CT < 25)
    {
        MaxSector = 18;
    }
    else if (CT < 31)
    {
        MaxSector = 17;
    }
    else
    {
        MaxSector = 16;
    }

}
*/

//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool ResetBundleVariables() {

    FileCnt = -1;
    tmpFileNameV.clear();
    tmpFileAddrV.clear();
    tmpFileOffsV.clear();
    tmpFileLenV.clear();
    tmpFileIOV.clear();

    tmpPrgs.clear();

    VFileCnt = -1;
    tmpVFileNameV.clear();
    tmpVFileAddrV.clear();
    tmpVFileOffsV.clear();
    tmpVFileLenV.clear();
    tmpVFileIOV.clear();

    tmpVFiles.clear();

    BundleCnt++;

    TotalBundles++;

    BundleSizeV.clear();
    BundleOrigSizeV.clear();

    BlockCnt = 0;

    UncompBundleSize = 0;

    return true;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

void NewDisk() {

    std::fill_n(Disk, ExtBytesPerDisk, 0);
    if(TracksPerDisk == ExtTracksPerDisk)
    {
        BlocksFree = ExtSectorsPerDisk;
        SectorsPerDisk = ExtSectorsPerDisk;
    }
    else
    {
        BlocksFree = StdSectorsPerDisk;
        SectorsPerDisk = StdSectorsPerDisk;
    }

    int BAM = Track[18];

    Disk[BAM] = 0x12;                            //Track 18
    Disk[BAM+1] = 0x01;                          //Sector 01
    Disk[BAM+2] = 0x41;                          //"A"

    for(int i = 0x90; i <= 0xaa; i++)
    {
        Disk[BAM + i] = 0x20;                    //Name, ID, DOS type
    }

    //-------------------------------------------

    for (int i = 4; i < (36 * 4); i++)
    {
        Disk[BAM + i] = 0xff;
    }

    for (int i = 1; i <= 17; i++)
    {
        Disk[BAM + (i * 4) + 0] = 21;
        Disk[BAM + (i * 4) + 3] = 31;
    }

    for (int i = 18; i <= 24; i++)
    {
        Disk[BAM + (i * 4) + 0] = 19;
        Disk[BAM + (i * 4) + 3] = 7;
    }

    for (int i = 25; i <= 30; i++)
    {
        Disk[BAM + (i * 4) + 0] = 18;
        Disk[BAM + (i * 4) + 3] = 3;
    }

    for (int i = 31; i <= 35; i++)
    {
        Disk[BAM + (i * 4) + 0] = 17;
        Disk[BAM + (i * 4) + 3] = 1;
    }

    Disk[BAM + (18 * 4) + 0] = 17;
    Disk[BAM + (18 * 4) + 1] = 252;

    CT = 18;
    CS = 1;

    //SetMaxSector();

    BAM = Track[CT] + (256 * CS);
    Disk[BAM + 1] = 255;

    NextTrack = 1;
    NextSector = 0;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool ResetDiskVariables() {

    if (DiskCnt == 126) {
        cout << "You have reached the maximum number of disks (127) in this project. " + '\n';
        return false;
    }

    DiskCnt++;

    //Reset Bundle File variables here, to have an empty array for the first compression on a ReBuild
    Prgs.clear();    //this is the one that is needed for the first CompressPart call during a ReBuild

    FileNameV.clear();
    FileAddrV.clear();
    FileOffsV.clear();
    FileLenV.clear();
    FileIOV.clear();

    //Reset directory arrays
    fill_n(DirBlocks, 512, 0);
    fill_n(DirPtr, 128, 0);

    //Reset disk system to support 35 tracks
    TracksPerDisk = StdTracksPerDisk;

    //Reset Hi-Score Saver plugin variables
    bSaverPlugin = false;
    HSFileName = "";
    HSAddress = 0;
    HSOffset = 0;
    HSLength = 0;

    //'Reset interleave
    IL0 = DefaultIL0;
    IL1 = DefaultIL1;
    IL2 = DefaultIL2;
    IL3 = DefaultIL3;


    BufferCnt = 0;
    BundleNo = 0;
    MaxBundleNoExceeded = false;

    //ReDim ByteSt(-1);
    //ResetBuffer();

    D64Name = "";
    DiskHeader = ""; //'"demo disk " + Year(Now).ToString
    DiskID = ""; //'"sprkl"
    DemoName = ""; //'"demo"
    DemoStart = "";
    DirArtName = "";
    LoaderZP = "02";

    //Reset Disk image
    NewDisk();

    BlockPtr = 1;

    //'-------------------------------------------------------------

    StartTrack = 1;
    StartSector = 0;

    NextTrack = StartTrack;
    NextSector = StartSector;

    //BitsSaved = 0;
    //BytesSaved = 0;

    FirstFileOfDisk = true;  //To save Start Address of first file on disk if Demo Start is not specified

    //-------------------------------------------------------------
    
    BundleCnt = -1;        //'WILL BE INCREASED TO 0 IN ResetPartVariables
    LoaderBundles = 1;
    FilesInBuffer = 1;

    CurrentBundle = -1;

    //-------------------------------------------------------------

    if (ResetBundleVariables() == false) {            //Also adds first bundle

        return false;
    }

    NewBundle = false;

    return true;

}

bool FinishDisk(bool LastDisk) {

    return true;

}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool SplitScriptEntry() {

    unsigned int Pos = 0;
    unsigned char ThisChar;
    ScriptEntryType = "";
    while (Pos < ScriptEntry.length()) {
        ThisChar = tolower(ScriptEntry[Pos++]);
        if (ThisChar != '\t') {
            ScriptEntryType += ThisChar;
        }
        else
        {
            break;
        }
     }
    std::fill_n(ScriptEntryArray, MaxNumEntries, "");

    NumScriptEntries = -1;

    while (Pos < ScriptEntry.length()) {

        if (NumScriptEntries == -1)
            NumScriptEntries++;

        ThisChar = tolower(ScriptEntry[Pos++]);

        if (ThisChar != '\t') {
            ScriptEntryArray[NumScriptEntries] += ThisChar;
        }
        else
        {
            if (ScriptEntryArray[NumScriptEntries] != "") {
                if (NumScriptEntries < MaxNumEntries - 1) {
                    NumScriptEntries ++;
                }
                else
                {
                    ScriptEntryArray[NumScriptEntries] += ThisChar;
                }
            }
        }
    }

    return true;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool FindNextScriptEntry() {

    ScriptEntry = "";
    unsigned char S = Script[SS];

    while((S == 13) || (S == 10)) {
        NewBundle = true;
        if (S == 13) {
            SS++;
        }
        SS++;
        S = Script[SS];
    }

    SE = SS;

    while ((S != 13) && (S != 10) && (SE <= Script.length() - 1)) {
        ScriptEntry += tolower(S);
        SE++;
        if (SE <= Script.length() - 1) {
            S = Script[SE];
        }
    }

    if (S == 13) {
        SE++;
    }
    SS = SE + 1;
    SE = SS + 1;

    return true;

}
//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool InsertScript(string& SubScriptPath)
{
    ScriptLine = ScriptEntry;

    string sPath = SubScriptPath;

    //Calculate full path

    if (SubScriptPath.find(":") == string::npos)
    {
        SubScriptPath = ScriptPath + SubScriptPath;
    }

    if (!FileExits(SubScriptPath))
    {
        cout << "***CRITICAL***\nThe following script was not found and could not be processed: " << SubScriptPath << "\n";
        return false;
    }

    //Find relative path of subscript
    for (int i = sPath.length() - 1; i > 0; i--)
    {
        if (sPath[i] != '\\')
        {
            sPath.replace(i, 1, "");
        }
        else
        {
            break;
        }
    }

    //Read subscript file
    string SubScript = ReadFileToString(SubScriptPath);

    //Count line breakes
    int NumLines = std::count(SubScript.begin(), SubScript.end(), '\n');

    //Store lines in vector
    vector<string> Lines;

    while (SubScript.find("\n") != string::npos)
    {
        Lines.push_back(SubScript.substr(0, SubScript.find("\n")));
        SubScript.erase(0, SubScript.find("\n") + 1);   //Erase line from SubScript, including the line break
    }
    Lines.push_back(SubScript);                         //Rest of the SubScript, after the last line break

    string S = "";
    for (int i = 0; i < Lines.size(); i++)
    {
        ScriptEntry = (Lines[i]);
        
        SplitScriptEntry();                 //This will also convert the script entry to lower case

        //Skip Script Header
        if (Lines[i] != ScriptHeader)
        {
            if (S != "")
                S += "\n";
            
            //Add relative path of subscript to relative path of subscript entries
            if (ScriptEntryType == "file:")
            {
                if (ScriptEntryArray[0].find(":") == string::npos)
                {
                    ScriptEntryArray[0] = sPath + ScriptEntryArray[0];
                }
                Lines[i] = EntryTypeFile + "\t" + ScriptEntryArray[0];
                for (int j = 1; j <= NumScriptEntries; j++)
                {
                    Lines[i] += "\t" + ScriptEntryArray[j];
                }
            }
            else if (ScriptEntryType == "mem:")
            {
                if (ScriptEntryArray[0].find(":") == string::npos)
                {
                    ScriptEntryArray[0] = sPath + ScriptEntryArray[0];
                }
                Lines[i] = EntryTypeMem + "\t" + ScriptEntryArray[0];
                for (int j = 1; j <= NumScriptEntries; j++)
                {
                    Lines[i] += "\t" + ScriptEntryArray[j];
                }
            }
            else if (ScriptEntryType == "script:")
            {
                if (ScriptEntryArray[0].find(":") == string::npos)
                {
                    ScriptEntryArray[0] = sPath + ScriptEntryArray[0];
                }
                Lines[i] = EntryTypeScript + "\t" + ScriptEntryArray[0];
            }
            else if (ScriptEntryType == "path:")
            {
                if (ScriptEntryArray[0].find(":") == string::npos)
                {
                    ScriptEntryArray[0] = sPath + ScriptEntryArray[0];
                }
                Lines[i] = EntryTypePath + "\t" + ScriptEntryArray[0];
            }
            else if (ScriptEntryType == "dirart:")
            {
                if (ScriptEntryArray[0].find(":") == string::npos)
                {
                    ScriptEntryArray[0] = sPath + ScriptEntryArray[0];
                }
                Lines[i] = EntryTypeDirArt + "\t" + ScriptEntryArray[0];
            }
            else if (ScriptEntryType == "hsfile:")
            {
                if (ScriptEntryArray[0].find(":") == string::npos)
                {
                    ScriptEntryArray[0] = sPath + ScriptEntryArray[0];
                }
                Lines[i] = EntryTypeHSFile + "\t" + ScriptEntryArray[0];
                for (int j = 1; j <= NumScriptEntries; j++)
                {
                    Lines[i] += "\t" + ScriptEntryArray[j];
                }
            }
        }
        S += Lines[i];
    }

    string SS1 = Script.substr(0, LastSS - 1);
    string SS2 = Script.substr(SS, Script.length() - SS);
    Script = SS1 + S + SS2;
    
    SS = LastSS;
    SE = LastSE;

    Lines.clear();

    return true;

}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool BuildDiskFromScript() {
    /* initialize random seed: */
    srand(time(NULL));

    /* generate a random number between 0 and 0Xffffff: */
    ProductID = rand() % 0xffffff;

    SS = 0;
    SE = 0;

    FindNextScriptEntry();


    if (ScriptEntry != ScriptHeader)
    {
        cout << "Invalid script file!" + '\n';

        return false;

    }

    //SS = SE;

    bool NewD = true;
    NewBundle = false;
    TmpSetNewBlock = false;

    while (SE <= Script.length() - 1) {

        LastSS = SS;
        LastSE = SE;

        if (FindNextScriptEntry() == true)
        {
            SplitScriptEntry();

            if (ScriptEntryType == EntryTypePath)
            {
                if (NewD == false)
                {
                    NewD = true;
                    if ((!FinishDisk(false)) || (!ResetDiskVariables()))
                        return false;
                    //if (FinishDisk(false) == false)
                    //   return false;
                    //if (ResetDiskVariables() == false)
                    //    return false;
                }

                if (NumScriptEntries > -1)
                    D64Name = ScriptEntryArray[0];

                NewBundle = true;
            }
            else if (ScriptEntryType == EntryTypeHeader)
            {
                if (NewD == false)
                {
                    NewD = true;
                    if ((!FinishDisk(false)) || (!ResetDiskVariables()))
                        return false;
                    //if (FinishDisk(false) == false)
                    //   return false;
                    //if (ResetDiskVariables() == false)
                    //    return false;
                }

                if (NumScriptEntries > -1)
                    DiskHeader = ScriptEntryArray[0];

                NewBundle = true;
            }
            else if (ScriptEntryType == EntryTypeID)
            {
                if (NewD == false)
                {
                    NewD = true;
                    if ((!FinishDisk(false)) || (!ResetDiskVariables()))
                        return false;
                    //if (FinishDisk(false) == false)
                    //   return false;
                    //if (ResetDiskVariables() == false)
                    //    return false;
                }

                if (NumScriptEntries > -1)
                    DiskID = ScriptEntryArray[0];

                NewBundle = true;
            }
            else if (ScriptEntryType == EntryTypeName)
            {
                if (NewD == false)
                {
                    NewD = true;
                    if ((!FinishDisk(false)) || (!ResetDiskVariables()))
                        return false;
                    //if (FinishDisk(false) == false)
                    //   return false;
                    //if (ResetDiskVariables() == false)
                    //    return false;
                }

                if (NumScriptEntries > -1)
                    DemoName = ScriptEntryArray[0];

                NewBundle = true;
            }
            else if (ScriptEntryType == EntryTypeStart)
            {
                if (NewD == false)
                {
                    NewD = true;
                    if ((!FinishDisk(false)) || (!ResetDiskVariables()))
                        return false;
                    //if (FinishDisk(false) == false)
                    //   return false;
                    //if (ResetDiskVariables() == false)
                    //    return false;
                }

                if (NumScriptEntries > -1)
                    DemoStart = ScriptEntryArray[0];

                NewBundle = true;
            }
            else if (ScriptEntryType == EntryTypeDirArt)
            {
                if (NewD == false)
                {
                    NewD = true;
                    if ((!FinishDisk(false)) || (!ResetDiskVariables()))
                        return false;
                    //if (FinishDisk(false) == false)
                    //   return false;
                    //if (ResetDiskVariables() == false)
                    //    return false;
                }
                if (ScriptEntryArray[0].find(":") == string::npos)
                {
                    ScriptEntryArray[0] = ScriptPath + ScriptEntryArray[0];
                }

                if (FileExits(ScriptEntryArray[0]))
                {
                    DirArtName = ScriptEntryArray[0];

                }
                else
                {
                    cout << "The following DirArt file does not exist: " << ScriptEntryArray[0] << "\nThe disk will be built without the DirArt file.\n";
                }

                NewBundle = true;
            }
            else if (ScriptEntryType == EntryTypeZP)
            {
                if (NewD == false)
                {
                    NewD = true;
                    if ((!FinishDisk(false)) || (!ResetDiskVariables()))
                        return false;
                    //if (FinishDisk(false) == false)
                    //   return false;
                    //if (ResetDiskVariables() == false)
                    //    return false;
                }

                if (DiskCnt == 0)
                {
                    if (NumScriptEntries > -1)
                    {
                        LoaderZP = ScriptEntryArray[0];
                    }
                }

                NewBundle = true;
            }
            else if (ScriptEntryType == EntryTypeIL0)
            {
                if (NewD == false)
                {
                    NewD = true;
                    if ((!FinishDisk(false)) || (!ResetDiskVariables()))
                        return false;
                    //if (FinishDisk(false) == false)
                    //   return false;
                    //if (ResetDiskVariables() == false)
                    //    return false;
                }

                if (NumScriptEntries > -1)
                {
                    int TmpIL = 0;
                    if (IsNumeric(ScriptEntryArray[0]) == true)
                    {
                        TmpIL = stoi(ScriptEntryArray[0]);
                    }
                    if (TmpIL % 21 > 0)
                    {
                        IL0 = TmpIL;
                    }
                    else
                    {
                        IL0 = DefaultIL0;
                    }
                }

                NewBundle = true;
            }
            else if (ScriptEntryType == EntryTypeIL1)
            {
                if (NewD == false)
                {
                    NewD = true;
                    if ((!FinishDisk(false)) || (!ResetDiskVariables()))
                        return false;
                    //if (FinishDisk(false) == false)
                    //   return false;
                    //if (ResetDiskVariables() == false)
                    //    return false;
                }

                if (NumScriptEntries > -1)
                {
                    int TmpIL = 0;
                    if (IsNumeric(ScriptEntryArray[0]) == true)
                    {
                        TmpIL = stoi(ScriptEntryArray[0]);
                    }
                    if (TmpIL % 19 > 0)
                    {
                        IL1 = TmpIL;
                    }
                    else
                    {
                        IL1 = DefaultIL1;
                    }
                }

                NewBundle = true;
            }
            else if (ScriptEntryType == EntryTypeIL2)
            {
                if (NewD == false)
                {
                    NewD = true;
                    if ((!FinishDisk(false)) || (!ResetDiskVariables()))
                        return false;
                    //if (FinishDisk(false) == false)
                    //   return false;
                    //if (ResetDiskVariables() == false)
                    //    return false;
                }

                if (NumScriptEntries > -1)
                {
                    int TmpIL = 0;
                    if (IsNumeric(ScriptEntryArray[0]) == true)
                    {
                        TmpIL = stoi(ScriptEntryArray[0]);
                    }
                    if (TmpIL % 18 > 0)
                    {
                        IL2 = TmpIL;
                    }
                    else
                    {
                        IL2 = DefaultIL2;
                    }
                }

                NewBundle = true;
            }
            else if (ScriptEntryType == EntryTypeIL3)
            {
                if (NewD == false)
                {
                    NewD = true;
                    if ((!FinishDisk(false)) || (!ResetDiskVariables()))
                        return false;
                    //if (FinishDisk(false) == false)
                    //   return false;
                    //if (ResetDiskVariables() == false)
                    //    return false;
                }

                if (NumScriptEntries > -1)
                {
                    int TmpIL = 0;
                    if (IsNumeric(ScriptEntryArray[0]) == true)
                    {
                        TmpIL = stoi(ScriptEntryArray[0]);
                    }
                    if (TmpIL % 17 > 0)
                    {
                        IL3 = TmpIL;
                    }
                    else
                    {
                        IL3 = DefaultIL3;
                    }
                }

                NewBundle = true;
            }
            else if (ScriptEntryType == EntryTypeProdID)
            {
                if (NewD == false)
                {
                    NewD = true;
                    if ((!FinishDisk(false)) || (!ResetDiskVariables()))
                        return false;
                    //if (FinishDisk(false) == false)
                    //   return false;
                    //if (ResetDiskVariables() == false)
                    //    return false;
                }
                if (NumScriptEntries > -1)
                {
                    if (IsHexString(ScriptEntryArray[0]))
                    {
                        ProductID = ConvertHexStringToInt("0x" + ScriptEntryArray[0]) & 0xffffff;
                    }
                    else
                    {
                        cout << "The Product ID must be a maximum 6-digit long hexadecimal number!\nSparkle will use the following pseudorandom Product ID: " << hex << ProductID <<"\n";
                    }
                }

                NewBundle = true;
            }
            else if (ScriptEntryType == EntryTypeTracks)
            {
                if (NewD == false)
                {
                    NewD = true;
                    if ((!FinishDisk(false)) || (!ResetDiskVariables()))
                        return false;
                    //if (FinishDisk(false) == false)
                    //   return false;
                    //if (ResetDiskVariables() == false)
                    //    return false;
                }

                if (NumScriptEntries > -1)
                {
                    if (ScriptEntryArray[0]=="40")
                    {
                        TracksPerDisk = ExtTracksPerDisk;
                        SectorsPerDisk = ExtSectorsPerDisk;
                    }
                    else
                    {
                        TracksPerDisk = StdTracksPerDisk;
                        SectorsPerDisk = StdSectorsPerDisk;
                    }
                    UpdateDiskSizeOnTheFly();
                }

                NewBundle = true;
            }
            else if (ScriptEntryType == EntryTypeHSFile)
            {
                if (NewD == false)
                {
                    NewD = true;
                    if ((!FinishDisk(false)) || (!ResetDiskVariables()))
                        return false;
                    //if (!ResetDiskVariables())
                    //    return false;
                }
                if (NumScriptEntries > -1)
                {
                    if (ScriptEntryArray[0] !="")
                    {
                        if (AddHSFile() == false)
                            return false;
                    }
                }

                NewBundle = true;
            }
            else if (ScriptEntryType == EntryTypeScript)
            {
            if (!InsertScript(ScriptEntryArray[0]))
                return false;
                
            NewBundle = true;
            }
            else if (ScriptEntryType == EntryTypeFile)
            {

                NewD = false;
                NewBundle = false;
            }
            else if (ScriptEntryType == EntryTypeMem)
            {

                NewBundle = false;
            }
            else if (ScriptEntryType == EntryTypeAlign)
            {
                if (NewD == false)
                {
                    TmpSetNewBlock = true;
                }
            }
            else
            {
                if (NewBundle == true)
                {
                    //if (BundleDone() == false)
                    //  return false;
                    NewBundle = false;
                }
            }
        }
        else
        {
            break;
        }

        //SS = SE;
    }

    return true;

}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

void SetScriptPath(string sPath, string aPath)
{
    if (sPath.find(":") == string::npos)
    {
        sPath = aPath + sPath;                      //sPAth is relative - use Sparkle's base folder to make it a full path
    }

    ScriptName = sPath;

    ScriptPath = sPath;
    for (int i = sPath.length() - 1; i >= 0; i--)
    {
        if (sPath[i] != '\\')
        {
            ScriptPath.replace(i, 1, "");
        }
        else
        {
            break;
        }
    }

    //cout << ScriptPath << "\n";
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    cout << "Sparkle by Sparta/OMG 2019-2022" << '\n';

    string AppPath{filesystem::current_path().string() + "\\"};

    if (argc < 2)
    {
        ///cerr << "Usage: Sparkle script.sls" << '\n';

		//return 1;
        string ScriptFileName = "Test.sls";
        Script = ReadFileToString(ScriptFileName);

        SetScriptPath(ScriptFileName, AppPath);
    }
    else
    {
        string ScriptFileName(argv[1]);
        Script = ReadFileToString(ScriptFileName);

        SetScriptPath(ScriptFileName, AppPath);
    }


    if (Script == "")
    {
        cerr <<"***CRITICAL***\nUnable to load script file or  the file is empty!\n";
        return 1;
    }

    Track[1] = 0;
    for (int T = 1; T < ExtTracksPerDisk - 1; T++)
    {
        if (T < 18)
        {
            Track[T + 1] = Track[T] + (21 * 256);
        }
        else if (T < 25)
        {
            Track[T + 1] = Track[T] + (19 * 256);
        }
        else if (T < 31)
        {
            Track[T + 1] = Track[T] + (18 * 256);
        }
        else
        {
            Track[T + 1] = Track[T] + (17 * 256);
        }
    }

    //NewDisk();

    //WriteDiskImage("C:\\Tmp\\Disk.d64");

    BuildDiskFromScript();

    return 0;
}
