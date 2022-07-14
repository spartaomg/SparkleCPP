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
unsigned int LineStart, LineEnd;    //LastSS, LastSE;
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

vector<vector<unsigned char>> Prgs;
vector<string> FileNameV;
vector<string> FileAddrV;
vector<string> FileOffsV;
vector<string> FileLenV;
vector<bool> FileIOV;

vector<vector<unsigned char>> tmpPrgs;
vector<string> tmpFileNameV;
vector<string> tmpFileAddrV;
vector<string> tmpFileOffsV;
vector<string> tmpFileLenV;
vector<bool> tmpFileIOV;

vector<vector<unsigned char>> VFiles;
vector<string> VFileNameV;
vector<string> VFileAddrV;
vector<string> VFileOffsV;
vector<string> VFileLenV;
vector<bool> VFileIOV;

vector<vector<unsigned char>> tmpVFiles;
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

void WriteTextToFile(const string& DiskName)
{
    ofstream out(DiskName);
    out << Script;
    out.close();
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

    //vector<unsigned char> P;

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

    LineStart = LineEnd;

    ScriptEntry = "";
    char S = Script[LineStart++];

    //Find the first non-linebreak character => LineStart
    while ((S == 13) || (S == 10)) {
        NewBundle = true;
        S = Script[LineStart++];
    }

    //Find the last non-linebreak character => LineEnd
    LineEnd = --LineStart;
    while ((S != 13) && (S != 10) && (LineEnd <= Script.length() - 1)) {
        ScriptEntry += tolower(S);
            S = Script[LineEnd++];
    }

    LineEnd--;

    return true;

}
//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool InsertScript(string& SubScriptPath)
{
    ScriptLine = ScriptEntry;

    //Calculate full path

    if (SubScriptPath.find(":") == string::npos)
    {
        SubScriptPath = ScriptPath + SubScriptPath;
    }

    string sPath = SubScriptPath;

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
    int NumLines = std::count(SubScript.begin(), SubScript.end(), '\n') + 1;

    //Store lines in vector
    vector<string> Lines;

    while (SubScript.find("\n") != string::npos)
    {
        Lines.push_back(SubScript.substr(0, SubScript.find("\n")));
        SubScript.erase(0, SubScript.find("\n") + 1);   //Erase line from SubScript, including the line break
    }
    Lines.push_back(SubScript);                         //Rest of the SubScript, after the last line break

    string S = "";
    for (vector <string>::size_type i = 0; i != Lines.size(); i++)
    {
        ScriptEntry = (Lines[i]);
        
        SplitScriptEntry();                 //This will also convert the script entry to lower case

        //Skip Script Header
        if (Lines[i] != ScriptHeader)
        {
            if (S != "")
                S += "\n";
            
            //Add relative path of subscript to relative path of subscript entries
            if ((ScriptEntryType == EntryTypeFile) || (ScriptEntryType == EntryTypeMem) || (ScriptEntryType == EntryTypeHSFile))
            {
                if (ScriptEntryArray[0].find(":") == string::npos)
                {
                    ScriptEntryArray[0] = sPath + ScriptEntryArray[0];
                }
                Lines[i] = ScriptEntryType + "\t" + ScriptEntryArray[0];
                for (int j = 1; j <= NumScriptEntries; j++)
                {
                    Lines[i] += "\t" + ScriptEntryArray[j];
                }
            }
            else if ((ScriptEntryType == EntryTypeScript) || (ScriptEntryType == EntryTypePath) || (ScriptEntryType == EntryTypeDirArt))
            {
                if (ScriptEntryArray[0].find(":") == string::npos)
                {
                    ScriptEntryArray[0] = sPath + ScriptEntryArray[0];
                }
                Lines[i] = ScriptEntryType + "\t" + ScriptEntryArray[0];
            }
        }
        S += Lines[i];
    }

    string SS1 = Script.substr(0, LineStart);
    string SS2 = Script.substr(LineEnd, Script.length() - (LineEnd));
    Script = SS1 + S + SS2;
    
    WriteTextToFile("C:\\Tmp\\TestScript.sls");

    Lines.clear();

    return true;

}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool CloseBundle(int NextFileIO, bool LastPartOnDisk) {


    //SequenceFits() -> in Packer.cpp
    //EORTransform() -> in Packer.cpp
    //AddBits() -> in Packer.cpp
    //ClosBuffer()
    //AddLitBits() -> in Packer.cpp

    return true;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool CompressBundle() {

    //PackFile -> in Packer.cpp
    //CloseFile() -> here!!!

/*
        Dim PreBCnt As Integer = BufferCnt

        If Prgs.Count = 0 Then Exit Function        'GoTo NoComp DOES NOT WORK!!!

        'DO NOT RESET ByteSt AND BUFFER VARIABLES HERE!!!

        If (BufferCnt = 0) And (BytePtr = 255) Then
            NewBlock = SetNewBlock          'SetNewBlock is true at closing the previous bundle, so first it just sets NewBlock2
            SetNewBlock = False             'And NewBlock will fire at the desired bundle
        Else
            If FromEditor = False Then      'Don't finish previous bundle here if we are calculating bundle size from Editor

                '----------------------------------------------------------------------------------
                '"SPRITE BUG"
                'Compression bug involving the transitional block - FIXED
                'Fix: include the I/O status of the first file of this bundle in the calculation for
                'finishing the previous bundle
                '----------------------------------------------------------------------------------

                'Before finishing the previous bundle, calculate I/O status of the LAST BYTE of the first file of this bundle
                '(Files already sorted)
                Dim ThisBundleIO As Integer = If(FileIOA.Count > 0, CheckNextIO(FileAddrA(0), FileLenA(0), FileIOA(0)), 0)
                If CloseBundle(ThisBundleIO, False) = False Then GoTo NoComp
            End If
        End If

        '-------------------------------------------------------
        'SAVE CURRENT BIT POINTER AND BUFFER COUNT FOR DIRECTORY
        '-------------------------------------------------------

        If FromEditor = False Then
            'Only if we are NOT in the Editor
            If BundleNo < 128 Then
                DirBlocks((BundleNo * 4) + 3) = BitPtr
                DirPtr(BundleNo) = BufferCnt
                BundleNo += 1
            Else
                MaxBundleNoExceeded = True
            End If
        End If

        '-------------------------------------------------------

        NewBundle = True
        LastFileOfBundle = False

        PartialFileIndex = -1

        For I As Integer = 0 To Prgs.Count - 1
            'Mark the last file in a bundle for better compression
            If I = Prgs.Count - 1 Then LastFileOfBundle = True
            'The only two parameters that are needed are FA and FUIO... FileLenA(i) is not used

            If PartialFileIndex = -1 Then PartialFileOffset = Prgs(I).ToArray.Length - 1

            PackFile(Prgs(I).ToArray, I, FileAddrA(I), FileIOA(I))
            If I < Prgs.Count - 1 Then
                'WE NEED TO USE THE NEXT FILE'S ADDRESS, LENGTH AND I/O STATUS HERE
                'FOR I/O BYTE CALCULATION FOR THE NEXT PART - BUG reported by Raistlin/G*P
                PrgAdd = Convert.ToInt32(FileAddrA(I + 1), 16)
                PrgLen = Prgs(I + 1).Length ' Convert.ToInt32(FileLenA(I + 1), 16)
                FileUnderIO = FileIOA(I + 1)
                CloseFile()
            End If
        Next

        LastBlockCnt = BlockCnt

        If LastBlockCnt > 255 Then
            'Parts cannot be larger than 255 blocks compressed
            'There is some confusion here how PartCnt is used in the Editor and during Disk building...
            MsgBox("Bundle " + If(CompressBundleFromEditor = True, BundleCnt + 1, BundleCnt).ToString + " would need " + LastBlockCnt.ToString + " blocks on the disk." + vbNewLine + vbNewLine + "Bundles cannot be larger than 255 blocks compressed!", vbOKOnly + vbCritical, "Bundle exceeds 255-block limit!")
            If CompressBundleFromEditor = False Then GoTo NoComp
        End If

        'IF THE WHOLE Bundle IS LESS THAN 1 BLOCK, THEN "IT DOES NOT COUNT", Bundle Counter WILL NOT BE INCREASED
        If PreBCnt = BufferCnt Then
            BundleCnt -= 1
        End If
*/
    return true;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

int CheckNextIO(string sAddress, string sLength, bool NextFileUnderIO) {

    int pAddress = ConvertStringToInt(sAddress);

    if (pAddress < 256) {
        return 1;
    }
    else
    {
        if ((pAddress >= 0xd000) && (pAddress < 0xe000) && (NextFileUnderIO))
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool SortBundle() {

/*
        If tmpPrgs.Count = 0 Then Exit Function
        If tmpPrgs.Count = 1 Then GoTo SortDone

        Dim Change As Boolean
        Dim FSO, FEO, FSI, FEI As Integer   'File Start and File End Outer loop/Inner loop
        Dim PO(), PI() As Byte
        Dim S As String
        Dim bIO As Boolean

        '--------------------------------------------------------------------------------
        'Check files for overlap

        For O As Integer = 0 To tmpPrgs.Count - 2
            FSO = Convert.ToInt32(tmpFileAddrA(O), 16)              'Outer loop File Start
            FEO = FSO + Convert.ToInt32(tmpFileLenA(O), 16) - 1     'Outer loop File End
            For I As Integer = O + 1 To tmpPrgs.Count - 1
                FSI = Convert.ToInt32(tmpFileAddrA(I), 16)          'Inner loop File Start
                FEI = FSI + Convert.ToInt32(tmpFileLenA(I), 16) - 1 'Inner loop File End
                '--|------+------|----OR----|------+------|----OR----|------+------|----OR-----|------+------|--
                '  FSO    FSI    FEO        FSO    FEI    FEO        FSI    FSO    FEI        FSI    FEO    FEI
                If ((FSI >= FSO) And (FSI <= FEO)) Or ((FEI >= FSO) And (FEI <= FEO)) Or ((FSO >= FSI) And (FSO <= FEI)) Or ((FEO >= FSI) And (FEO <= FEI)) Then
                    Dim OLS As Integer = If(FSO >= FSI, FSO, FSI)  'Overlap Start address
                    Dim OLE As Integer = If(FEO <= FEI, FEO, FEI)  'Overlap End address

                    If (OLS >= &HD000) And (OLE <= &HDFFF) And (tmpFileIOA(O) <> tmpFileIOA(I)) Then
                        'Overlap is IO memory only and different IO status - NO OVERLAP
                    Else
                        MsgBox("The following two files overlap in Bundle " + (BundleCnt - 1).ToString + ":" _
                           + vbNewLine + vbNewLine + tmpFileNameA(I) + " ($" + Hex(FSI) + " - $" + Hex(FEI) + ")" + vbNewLine + vbNewLine _
                           + tmpFileNameA(O) + " ($" + Hex(FSO) + " - $" + Hex(FEO) + ")", vbOKOnly + vbExclamation)
                    End If
                End If
            Next
        Next

        '--------------------------------------------------------------------------------
        'Append adjacent files
Restart:
        Change = False

        For O As Integer = 0 To tmpPrgs.Count - 2
            FSO = Convert.ToInt32(tmpFileAddrA(O), 16)
            FEO = Convert.ToInt32(tmpFileLenA(O), 16)
            For I As Integer = O + 1 To tmpPrgs.Count - 1
                FSI = Convert.ToInt32(tmpFileAddrA(I), 16)
                FEI = Convert.ToInt32(tmpFileLenA(I), 16)

                If FSO + FEO = FSI Then
                    'Inner file follows outer file immediately
                    If (FSI <= &HD000) Or (FSI > &HDFFF) Then
                        'Append files as they meet outside IO memory
Append:                 PO = tmpPrgs(O)
                        PI = tmpPrgs(I)
                        ReDim Preserve PO(FEO + FEI - 1)

                        For J As Integer = 0 To FEI - 1
                            PO(FEO + J) = PI(J)
                        Next

                        tmpPrgs(O) = PO

                        Change = True
                    Else
                        If tmpFileIOA(O) = tmpFileIOA(I) Then
                            'Files meet inside IO memory, append only if their IO status is the same
                            GoTo Append
                        End If
                    End If
                ElseIf FSI + FEI = FSO Then
                    'Outer file follows inner file immediately
                    If (FSO <= &HD000) Or (FSO > &HDFFF) Then
                        'Prepend files as they meet outside IO memory
Prepend:                PO = tmpPrgs(O)
                        PI = tmpPrgs(I)
                        ReDim Preserve PI(FEI + FEO - 1)

                        For J As Integer = 0 To FEO - 1
                            PI(FEI + J) = PO(J)
                        Next

                        tmpPrgs(O) = PI

                        tmpFileAddrA(O) = tmpFileAddrA(I)

                        Change = True
                    Else
                        If tmpFileIOA(O) = tmpFileIOA(I) Then
                            'Files meet inside IO memory, prepend only if their IO status is the same
                            GoTo Prepend
                        End If
                    End If
                End If

                If Change = True Then
                    'Update merged file's IO status
                    tmpFileIOA(O) = tmpFileIOA(O) Or tmpFileIOA(I)   'BUG FIX - REPORTED BY RAISTLIN/G*P
                    'New file's length is the length of the two merged files
                    FEO += FEI

                    tmpFileLenA(O) = ConvertIntToHex(FEO, 4)
                    'Remove File(I) and all its parameters
                    For J As Integer = I To tmpPrgs.Count - 2
                        tmpFileNameA(J) = tmpFileNameA(J + 1)
                        tmpFileAddrA(J) = tmpFileAddrA(J + 1)
                        tmpFileOffsA(J) = tmpFileOffsA(J + 1)     'this may not be needed later
                        tmpFileLenA(J) = tmpFileLenA(J + 1)
                        tmpFileIOA(J) = tmpFileIOA(J + 1)
                    Next
                    'One less file left
                    FileCnt -= 1
                    ReDim Preserve tmpFileNameA(tmpPrgs.Count - 2), tmpFileAddrA(tmpPrgs.Count - 2), tmpFileOffsA(tmpPrgs.Count - 2), tmpFileLenA(tmpPrgs.Count - 2)
                    ReDim Preserve tmpFileIOA(tmpPrgs.Count - 2)
                    tmpPrgs.Remove(tmpPrgs(I))
                    GoTo Restart
                End If
            Next
        Next

        '--------------------------------------------------------------------------------
        'Sort files by length (short files first, thus, last block will more likely contain 1 file only = faster depacking)
ReSort:
        Change = False
        For I As Integer = 0 To tmpPrgs.Count - 2
            'Sort except if file length < 4, to allow for ZP relocation script hack
            If Convert.ToInt32(tmpFileAddrA(I), 16) < Convert.ToInt32(tmpFileAddrA(I + 1), 16) Then
                PI = tmpPrgs(I)
                tmpPrgs(I) = tmpPrgs(I + 1)
                tmpPrgs(I + 1) = PI

                S = tmpFileNameA(I)
                tmpFileNameA(I) = tmpFileNameA(I + 1)
                tmpFileNameA(I + 1) = S

                S = tmpFileAddrA(I)
                tmpFileAddrA(I) = tmpFileAddrA(I + 1)
                tmpFileAddrA(I + 1) = S

                S = tmpFileOffsA(I)
                tmpFileOffsA(I) = tmpFileOffsA(I + 1)
                tmpFileOffsA(I + 1) = S

                S = tmpFileLenA(I)
                tmpFileLenA(I) = tmpFileLenA(I + 1)
                tmpFileLenA(I + 1) = S

                bIO = tmpFileIOA(I)
                tmpFileIOA(I) = tmpFileIOA(I + 1)
                tmpFileIOA(I + 1) = bIO
                Change = True
            End If
        Next
        If Change = True Then GoTo ReSort

SortDone:
        'Once Bundle is sorted, calculate the I/O status of the last byte of the first file and the number of bits that will be needed
        'to finish the last block of the previous bundle (when the I/O status of the just sorted bundle needs to be known)
        'This is used in CloseBuffer

        'Bytes needed: (1)LongMatch Tag, (2)NextBundle Tag, (3)AdLo, (4)AdHi, (5)First Lit, (6)1 Bit Stream Byte (for 1 Lit Bit), (7)+/- I/O
        '+/- 1 Match Bit (if the last sequence of the last bundle is a match sequence, no Match Bit after a Literal sequence)
        'Match Bit will be determened by MLen in SequenceFits() function, NOT ADDED TO BitsNeededForNextBundle here!!!

        'We may be overcalculating here but that is safer than undercalculating which would result in buggy decompression
        'If the last block is not the actual last block of the bundle...
        'With overcalculation, worst case scenario is a little bit worse compression ratio of the last block
        BitsNeededForNextBundle = (6 + CheckNextIO(tmpFileAddrA(0), tmpFileLenA(0), tmpFileIOA(0))) * 8
        ' +/- 1 Match Bit which will be added later in CloseBuffer if needed
*/
    return true;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool BundleDone() {

    //First finish last bundle, if it exists
    if (tmpPrgs.size() > 0)
    {
        CurrentBundle++;

        //Sort files in bundle
        if (!SortBundle())
            return false;
        //Then compress files and add them to bundle
        if (!CompressBundle())
            return false;     //THIS WILL RESET NewPart TO FALSE

        Prgs = tmpPrgs;
        FileNameV = tmpFileNameV;
        FileAddrV = tmpFileAddrV;
        FileOffsV = tmpFileOffsV;
        FileLenV = tmpFileLenV;
        FileIOV = tmpFileIOV;
        SetNewBlock = TmpSetNewBlock;
        TmpSetNewBlock = false;

        VFiles = tmpVFiles;
        VFileNameV = tmpVFileNameV;
        VFileAddrV = tmpVFileAddrV;
        VFileOffsV = tmpVFileOffsV;
        VFileLenV = tmpVFileLenV;
        VFileIOV = tmpVFileIOV;

        //Then reset bundle variables (file vectors, prg vector, block cnt), increase bundle counter
        ResetBundleVariables();
    }
    
    return true;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool AddFileToBundle() {

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
    if (FN.find("*") == FN.length() - 1)
    {
        FUIO = true;
        FN.replace(FN.length() - 1, 1, "");
    }

    //Convert string to lowercase
    for (int i = 0; i < FN.length(); i++)
        FN[i] = tolower(FN[i]);

    //Get file variables from script, or get default values if there were none in the script entry
    if (FileExits(FN))
    {
        //P.clear();
        ReadBinaryFile(FN, P);

        switch (NumParams)
        {
        case 1:  //No parameters in script
            
            if ((FN[FN.length() - 4] = '.') && (FN[FN.length() - 3] = 's')&& (FN[FN.length() - 2] = 'i')&& (FN[FN.length() - 1] = 'd'))
            {
                FA = ConvertIntToHextString(P[P[7]] + (P[P[7] + 1] * 256), 4);
                FO = ConvertIntToHextString(P[7] + 2, 8);
                FL = ConvertIntToHextString(P.size() - P[7] - 2, 4);
            }
            else if (P.size() > 2)                                      //We have at least 3 bytes in the file
            {
                FA = ConvertIntToHextString(P[0] + (P[1] * 256), 4);    //First 2 bytes define load address
                FO = "00000002";                                        //Offset=2, Length=prg length-2
                FL = ConvertIntToHextString(P.size() - 2, 4);
            }
            else                                                        //Short file without paramters -> HARD STOP
            {
                cout << "***CRITICAL***\nFile parameteres are needed for the following File entry: " << ScriptEntryType << "\t" << ScriptEntry << "\n";
                return false;
            }
            break;
        case 2:  //One parameter in script
            FA = ScriptEntryArray[1];                                   //Load address from script
            FO = "00000000";                                            //Offset will be 0, length=prg length
            FL = ConvertIntToHextString(P.size(), 4);
            break;

        case 3:  //Two parameters in script
            FA = ScriptEntryArray[1];                                   //Load address from script
            FO = ScriptEntryArray[2];                                   //Offset from script
            FON = ConvertHexStringToInt(FO);                            //Make sure offset is valid
            if (FON > P.size() - 1)
            {
                cout << "***CRITICAL***\nInvalid file offset detected in the following File entry: " << ScriptEntryType << "\t" << ScriptEntry << "\n";
                return false;
            }
            FL = ConvertIntToHextString(P.size() - FON, 4);             //Length=prg length-offset
            break;

        case 4:  //Three parameters in script
            FA = ScriptEntryArray[1];                                   //Load address from script
            FO = ScriptEntryArray[2];                                   //Offset from script
            FL = ScriptEntryArray[3];                                   //Length from script
            FON = ConvertHexStringToInt(FO);                            //Make sure offset is valid
            if (FON > P.size() - 1)
            {
                cout << "***CRITICAL***\nInvalid file offset detected in the following File entry: " << ScriptEntryType << "\t" << ScriptEntry << "\n";
                return false;
            }
        }

        FAN = ConvertHexStringToInt(FA);
        FON = ConvertHexStringToInt(FO);
        FLN = ConvertHexStringToInt(FL);

        //Make sure file length is not longer than actual file
        if(FON + FLN > P.size())
        {
            cout << "***CRITICAL***\nInvalid file length detected in the following File entry: " << ScriptEntryType << "\t" << ScriptEntry << "\n";
            return false;
        }

        //Make sure file address+length<=&H10000
        if (FAN + FLN > 0x10000)
        {
            cout << "***CRITICAL***\nInvalid file address and/or length detected in the following File entry: " << ScriptEntryType << "\t" << ScriptEntry << "\n";
            return false;
        }

        //Trim file to the specified chunk (FLN number of bytes starting at FON, to Address of FAN)
        vector<unsigned char>::iterator First = P.begin();
        P.erase(First + FON + FLN, First + P.size() - 1);   //Trim P from end of file (Offset+Length-1) to end of vector
        P.erase(First, First + FON - 1);                    //Trim P from beginning of file to Offset
    }
    else
    {
        cout << "***CRITICAL***\The following file does not exist: " << FN << "\n";
        return false;
    }

    tmpFileNameV.push_back(FN);
    tmpFileAddrV.push_back(FA);
    tmpFileOffsV.push_back(FO);         //This may not be needed later
    tmpFileLenV.push_back(FL);
    tmpFileIOV.push_back(FUIO);

    UncompBundleSize += FLN / 256;
    if (FLN % 256 != 0)
    {
        UncompBundleSize++;
    }
    
    if(FirstFileOfDisk)                     //If Demo Start is not specified, we will use the start address of the first file
    {
        FirstFileStart = FA;
        FirstFileOfDisk = false;
    }
    
    tmpPrgs.push_back(P);

    return true;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool AddFile() {

    if (NewBundle)
    {
        if (!BundleDone)
        {
            return false;
        }
    }

    //Then add file to bundle
    if (!AddFileToBundle)
    {
        return false;
    }

    return true;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool BuildDiskFromScript() {
    /* initialize random seed: */
    srand(time(NULL));

    /* generate a random number between 0 and 0Xffffff: */
    ProductID = rand() % 0xffffff;

    LineStart = 0;
    LineEnd = 0;

    FindNextScriptEntry();


    if (ScriptEntry != ScriptHeader)
    {
        cout << "Invalid script file!" + '\n';

        return false;

    }

    bool NewD = true;
    NewBundle = false;
    TmpSetNewBlock = false;

    while (LineEnd <= Script.length() - 1) {

        if (FindNextScriptEntry() == true)
        {
            SplitScriptEntry();

            if (ScriptEntryType == EntryTypePath)
            {
                if (!NewD)
                {
                    NewD = true;
                    if ((!FinishDisk(false)) || (!ResetDiskVariables()))
                        return false;
                }

                if (NumScriptEntries > -1)
                    D64Name = ScriptEntryArray[0];

                NewBundle = true;
            }
            else if (ScriptEntryType == EntryTypeHeader)
            {
                if (!NewD)
                {
                    NewD = true;
                    if ((!FinishDisk(false)) || (!ResetDiskVariables()))
                        return false;
                }

                if (NumScriptEntries > -1)
                    DiskHeader = ScriptEntryArray[0];

                NewBundle = true;
            }
            else if (ScriptEntryType == EntryTypeID)
            {
                if (!NewD)
                {
                    NewD = true;
                    if ((!FinishDisk(false)) || (!ResetDiskVariables()))
                        return false;
                }

                if (NumScriptEntries > -1)
                    DiskID = ScriptEntryArray[0];

                NewBundle = true;
            }
            else if (ScriptEntryType == EntryTypeName)
            {
                if (!NewD)
                {
                    NewD = true;
                    if ((!FinishDisk(false)) || (!ResetDiskVariables()))
                        return false;
                }

                if (NumScriptEntries > -1)
                    DemoName = ScriptEntryArray[0];

                NewBundle = true;
            }
            else if (ScriptEntryType == EntryTypeStart)
            {
                if (!NewD)
                {
                    NewD = true;
                    if ((!FinishDisk(false)) || (!ResetDiskVariables()))
                        return false;
                }

                if (NumScriptEntries > -1)
                    DemoStart = ScriptEntryArray[0];

                NewBundle = true;
            }
            else if (ScriptEntryType == EntryTypeDirArt)
            {
                if (!NewD)
                {
                    NewD = true;
                    if ((!FinishDisk(false)) || (!ResetDiskVariables()))
                        return false;
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
                    cout << "The following DirArt file does not exist: " << ScriptEntryArray[0] << "\nThe disk will be built without DirArt.\n";
                }

                NewBundle = true;
            }
            else if (ScriptEntryType == EntryTypeZP)
            {
                if (!NewD)
                {
                    NewD = true;
                    if ((!FinishDisk(false)) || (!ResetDiskVariables()))
                        return false;
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
                if (!NewD)
                {
                    NewD = true;
                    if ((!FinishDisk(false)) || (!ResetDiskVariables()))
                        return false;
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
                if (!NewD)
                {
                    NewD = true;
                    if ((!FinishDisk(false)) || (!ResetDiskVariables()))
                        return false;
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
                if (!NewD)
                {
                    NewD = true;
                    if ((!FinishDisk(false)) || (!ResetDiskVariables()))
                        return false;
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
                if (!NewD)
                {
                    NewD = true;
                    if ((!FinishDisk(false)) || (!ResetDiskVariables()))
                        return false;
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
                if (!NewD)
                {
                    NewD = true;
                    if ((!FinishDisk(false)) || (!ResetDiskVariables()))
                        return false;
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
                if (!NewD)
                {
                    NewD = true;
                    if ((!FinishDisk(false)) || (!ResetDiskVariables()))
                        return false;
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
                if (!NewD)
                {
                    NewD = true;
                    if ((!FinishDisk(false)) || (!ResetDiskVariables()))
                        return false;
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
            {
                return false;
            }
                
                NewBundle = true;   //Files in the embedded script will ALWAYS be in a new bundle (i.e. scripts cannot be embedded in a bundle)!!!
            }
            else if (ScriptEntryType == EntryTypeFile)
            {
                //Add files to bundle array, if new bundle=true, we will first sort, compress and add previous bundle to disk
                if (!AddFile)
                    return false;

                NewD = false;       //We have added at least one file to this disk, so next disk info entry will be a new disk
                NewBundle = false;
            }
            else if (ScriptEntryType == EntryTypeMem)
            {

                //if(!AddVirtualFile)
                //  return false;

                NewBundle = false;
                //NewD - false;     //IS THIS NEEDED???
            }
            else if (ScriptEntryType == EntryTypeAlign)
            {
                if (!NewD)
                {
                    TmpSetNewBlock = true;
                }
            }
            else
            {
                if (NewBundle)
                {
                    if (!BundleDone())
                        return false;
                    
                    NewBundle = false;
                }
            }
        }
        else
        {
            break;
        }
    }

    if (!FinishDisk(true))
        return false;

    return true;

}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

void SetScriptPath(string sPath, string aPath)
{
    if (sPath.find(":") == string::npos)
    {
        sPath = aPath + sPath;                      //sPath is relative - use Sparkle's base folder to make it a full path
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
        string ScriptFileName = "C:\\Tmp\\SpaceXDemo.sls";
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

    if (!BuildDiskFromScript())
        return -1;

    return 0;
}
