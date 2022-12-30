#include "common.h"

//#define DEBUG

//--------------------------------------------------------
//  COMPILE TIME VARIABLES FOR VERSION INFO 221230
//--------------------------------------------------------

constexpr unsigned int Year = ((__DATE__[9] - '0') * 10) + (__DATE__[10] - '0');

constexpr unsigned int Month = (__DATE__[0] == 'J') ? ((__DATE__[1] == 'a') ? 1 : ((__DATE__[2] == 'n') ? 6 : 7))    // Jan, Jun or Jul
                             : (__DATE__[0] == 'F') ? 2                                                              // Feb
                             : (__DATE__[0] == 'M') ? ((__DATE__[2] == 'r') ? 3 : 5)                                 // Mar or May
                             : (__DATE__[0] == 'A') ? ((__DATE__[2] == 'p') ? 4 : 8)                                 // Apr or Aug
                             : (__DATE__[0] == 'S') ? 9                                                              // Sep
                             : (__DATE__[0] == 'O') ? 10                                                             // Oct
                             : (__DATE__[0] == 'N') ? 11                                                             // Nov
                             : (__DATE__[0] == 'D') ? 12                                                             // Dec
                             : 0;
constexpr unsigned int Day = (__DATE__[4] == ' ') ? (__DATE__[5] - '0') : (__DATE__[4] - '0') * 10 + (__DATE__[5] - '0');
constexpr unsigned long int VersionBuild = ((Year / 10) * 0x100000) + ((Year % 10) * 0x10000) + ((Month / 10) * 0x1000) + ((Month % 10) * 0x100) + ((Day / 10) * 0x10) + (Day % 10);

constexpr int VersionMajor = 2;
constexpr int VersionMinor = 3;

string Script = "";
string ScriptPath = "";
string ScriptName = "";
string ScriptEntry = "";
string ScriptLine = "";
string ScriptEntryType = "";
const int MaxNumEntries = 5;
string ScriptEntryArray[MaxNumEntries]{};
int NumScriptEntries = -1;

const int StdSectorsPerDisk = 664;                                          //Standard disk
const int StdTracksPerDisk = 35;
const int StdBytesPerDisk = (StdSectorsPerDisk + 19) * 256;                 //including track 18

const int ExtSectorsPerDisk = StdSectorsPerDisk + 85;                       //Exnteded disk
const int ExtTracksPerDisk = 40;
const int ExtBytesPerDisk = StdBytesPerDisk + (85 * 256);                   //Including track 18

int SectorsPerDisk = StdSectorsPerDisk;
int TracksPerDisk = StdTracksPerDisk;

int BlocksFree = SectorsPerDisk;

unsigned char TabT[ExtSectorsPerDisk]{}, TabS[ExtSectorsPerDisk]{}, TabSCnt[ExtSectorsPerDisk]{}, TabStartS[ExtTracksPerDisk + 1]{};  //TabStartS is 1 based
unsigned char ByteSt[ExtBytesPerDisk];

unsigned char Disk[ExtBytesPerDisk];
unsigned char NextTrack;
unsigned char NextSector;                                                   //Next Empty Track and Sector
unsigned char MaxSector = 18;
unsigned char LastSector;

string Prg, ReferenceFile;

bool TmpSetNewBlock = false;
bool SetNewBlock = false;           //This will fire at the previous bundle and will set NewBlock2
bool NewBlock = false;              //This will fire at the specified bundle

string EntryTypeScriptHeader = "[sparkle loader script]";
string EntryTypePath = "path:";
string EntryTypeHeader = "header:";
string EntryTypeID = "id:";
string EntryTypeThisID = "thisside:";
string EntryTypeNextID = "nextside:";
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

unsigned long int ProductID = 0;
unsigned int LineStart, LineEnd;    //LastSS, LastSE;
bool NewBundle;

const int MaxNumDisks = 127;
int DiskSizeA[MaxNumDisks]{};

unsigned char DiskCnt = -1;
int BundleCnt = -1;
int FileCnt = -1;
int VFileCnt = -1;
int CurrentDisk = -1;
int CurrentBundle = -1;
int CurrentFile = -1;
int CurrentScript = -1;
int BundleNo = -1;
bool MaxBundleNoExceeded = false;

vector<FileStruct> Prgs;
vector<FileStruct> tmpPrgs;
vector<FileStruct> VFiles;
vector<FileStruct> tmpVFiles;

unsigned char DirBlocks[512]{};
int DirPtr[128]{};

//Hi-Score File variables
string HSFileName = "";
vector<unsigned char> HSFile;
size_t HSAddress = 0;
size_t HSOffset = 0;
size_t HSLength = 0;
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
unsigned char ThisID = 255;
unsigned char NextID = 255;

int DirTrack, DirSector, DirPos;
string DirArtName = "";
string DirEntry = "";
bool DirEntryAdded = false;

int BlockPtr;
int LastBlockCnt = 0;
char LoaderBundles = 1;
unsigned char FilesInBuffer = 1;

int Track[41]{};
unsigned char CT, CS, CP;
int BlockCnt;
unsigned char StartTrack = 1;
unsigned char StartSector = 0;

bool FirstFileOfDisk = false;
string FirstFileStart = "";

double UncompBundleSize = 0;

int BitsNeededForNextBundle = 0;
int BlocksUsedBySaver = 0;

//VARIABLES ALSO USED BY PACKER

int BytePtr;
int BitPtr;

bool LastFileOfBundle = false;

int PartialFileIndex;
size_t PartialFileOffset;

int PrgAdd, PrgLen;
bool FileUnderIO = false;

bool SaverSupportsIO = false;

int TotalOrigSize{}, TotalCompSize{};

unsigned char LoaderBlockCount = 0;

//------------------------------------------------------------------------------------------------------------------------------------------------------------

string ConvertIntToHextString(const int& i, const int& hexlen)
{
    char hexchar[8]{};
    sprintf_s(hexchar, "%X", i);
    string hexstring = "";
    for (int j = 0; j < hexlen; j ++)
    {
        if (hexchar[j] != 0)
        {
            hexstring += hexchar[j]; //+hexstring;
        }
        else
        {
            break;
        }
    }
    int HSL = hexstring.length();
    for (int j = hexlen; j > HSL; j--)
    {
        hexstring = "0" + hexstring;
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

    if (infile.fail())
    {
        return -1;
    }

    infile.seekg(0, ios_base::end);
    int length = infile.tellg();
    infile.seekg(0, ios_base::beg);

    prg.reserve(length);

    copy(istreambuf_iterator<char>(infile), istreambuf_iterator<char>(), back_inserter(prg));

    return length;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

string ReadFileToString(const string& FileName)
{

    if (!FileExits(FileName))
    {
        return "";
    }

    ifstream t(FileName);

    if (t.fail())
    {
        return "";
    }

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

bool WriteDiskImage(const string& DiskName)
{

    string DiskDir{};

    for (size_t i = 0; i <= DiskName.length() - 1; i++)
    {
        if ((DiskName[i] == '\\') || (DiskName[i] == '/'))
        {
            if (DiskDir[DiskDir.length() - 1] != ':')   //Ignore files in root directory (e.g. c:\disk.d64)
            {
                if (!filesystem::exists(DiskDir))
                {
                    cout << "Creating folder: " << DiskDir << "\n";
                    create_directory(DiskDir);
                }

                if (!filesystem::exists(DiskDir))
                {
                    cerr << "***CRITICAL***\tUnable to create the following folder: " << DiskDir << "\n\n";
                    return false;
                }
            }
        }
        DiskDir += DiskName[i];
    }

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

    if (myFile.is_open())
    {
        cout << "Writing disk image: " << DiskName << "\n\n";
        myFile.write((char*)&Disk[0], BytesPerDisk);
        return true;
    }
    else
    {
        cerr << "***CRITICAL***\tError during writing disk image: " << DiskName << "\n\n";
        return false;
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------
/*
void WriteBinaryFile(const string& FileName, unsigned char* Buffer, streamsize Size) {

    ofstream myFile(FileName, ios::out | ios::binary);
    myFile.write((char*)&Buffer[0], Size);

}
*/
//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool SectorOK(unsigned char T, unsigned char S) {

    int BAMPos = Track[18] + (T * 4) + 1 + (S / 8) + ((T > 35) ? (7 * 4) : 0);
    int BAMBit = 1 << (S % 8);

    return (Disk[BAMPos] & BAMBit) != 0;

}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool FindNextFreeSector() {

    int Counter = 0;
    int MaxS = (CT < 18) ? 21 : (CT < 25) ? 19 : (CT < 31) ? 18 : 17;

    while (Counter < MaxS)
    {
        if (SectorOK(CT, CS))
            return true;

        Counter++;
        (CS < MaxS) ? CS++ : CS = 0;
    }

    return false;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

void DeleteBit(unsigned char T, unsigned char S)
{

    int NumSectorPtr = Track[18] + (T * 4) + ((T > 35) ? 7 * 4 : 0);
    int BitPtr = NumSectorPtr + 1 + (S / 8);        //BAM Position for Bit Change
    unsigned char BitToDelete = 1 << (S % 8);       //BAM Bit to be deleted

    if (Disk[BitPtr] && BitToDelete != 0)
    {
        Disk[BitPtr] &= (255 - BitToDelete);

        unsigned char NumUnusedSectors = 0;

        for (int I = NumSectorPtr + 1; I <= NumSectorPtr + 3; I++)
        {
            unsigned char B = Disk[I];
            for (int J = 0; J < 8; J++)
            {
                if (B % 2 == 1)
                {
                    NumUnusedSectors++;
                }
                B >>= 1;
            }
        }
        Disk[NumSectorPtr] = NumUnusedSectors;

        if (T != 18)
            BlocksFree--;
    }

/*    If(Disk(BitPtr) And BitToDelete) < > 0 Then
        Disk(BitPtr) = Disk(BitPtr) And(255 - BitToDelete)

        Dim NumUnusedSectors As Byte = 0
        For I As Integer = NumSectorPtr + 1 To NumSectorPtr + 3
        Dim B As Byte = Disk(I)
        For J As Integer = 0 To 7
        If B Mod 2 = 1 Then
        NumUnusedSectors += 1
        End If
        B = Int(B / 2)
        Next
        Next
        Disk(NumSectorPtr) = NumUnusedSectors
        End If
*/
        //int BAMPos = Track[18] + (T * 4) + 1 + (S / 8) + ((T > 35) ? (7 * 4) : 0);
        //int BAMBit = 255 - (1 << (S % 8));


        //int BAMPos = Track[18] + (T * 4) + + ((T > 35) ? (7 * 4) : 0);

        //Disk[BAMPos]--;

        //if (UpdateFreeBlocks)
        //BlocksFree--;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool TrackIsFull(unsigned char T) {

    return (Disk[Track[18] + (T * 4) + ((T > 35) ? (7 * 4) : 0)] == 0);

}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

void CalcNextSector(unsigned char IL) {

    int MaxS = (CT < 18) ? 21 : (CT < 25) ? 19 : (CT < 31) ? 18 : 17;

    if (CS >= MaxS)
    {
        CS -= MaxS;
        CS -= ((CT < 18) && (CS > 0)) ? 1 : 0;
    }

    CS += IL;

    if (CS >= MaxS)
    {
        CS -= MaxS;
        CS -= ((CT < 18) && (CS > 0)) ? 1 : 0;
    }

}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool AddInterleave(unsigned char IL) {

    //THIS IS ONLY USED BY InjectLoader()!!!

    if (TrackIsFull(CT))
    {
        if ((CT == 35) || (CT == 18))
            return false;

        CalcNextSector(IL);

        CT++;

        if (CT == 18)
        {
            CT++;
            CS = 3;
        }
        //First sector in new track will be #1 and NOT #0!!!
        //CS = StartSector
    }
    else
    {
        CalcNextSector(IL);
    }

    return FindNextFreeSector();

}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

unsigned char EORtransform(unsigned char Input) {

    switch (Input & 0x09){
    case 0:
        return (Input ^ 0x7f);
    //case 1:
        //return (Input ^ 0x76);
    //case 8:
        //return (Input ^ 0x76);
    case 9:
        return (Input ^ 0x7f);
    default:
        return (Input ^ 0x76);
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

void InjectDirBlocks() {

    //DirBlocks(0) = EORtransform(Track)
    //DirBlocks(1) = EORtransform(Sector)
    //DirBlocks(2) = EORtransform(Remaining sectors on track)
    //DirBlocks(3) = BitPtr

    if (BundleNo >= 0)
    {
        for (int i = BundleNo + 1; i < 128; i++)
        {
            DirBlocks[(i * 4) + 3] = DirBlocks[(BundleNo * 4) + 3];
            DirPtr[i] = DirPtr[BundleNo];
        }
    }

    for (int i = 0; i < 128; i++)
    {
        DirBlocks[i * 4] = EORtransform(TabT[DirPtr[i]]);
        DirBlocks[(i * 4) + 1] = EORtransform(TabStartS[TabT[DirPtr[i]]]);
        DirBlocks[(i * 4) + 2] = EORtransform(TabSCnt[DirPtr[i]]);
    }

    //Resort directory sectors to allow simple copy from $0100 to $0700
    //Dir Block: $00,$ff,$fe,$fd,$fc,...,$01
    //Buffer:    $00,$01,$02,$03,$04,...,$ff

    unsigned char DB0[256]{}, DB1[256]{};
    int B = 0;

    for (int i = 0; i < 256; i++)
    {
        DB0[B] = DirBlocks[i];
        DB1[B] = DirBlocks[i + 256];
        B--;
        if (B < 0)
            B += 256;
    }
    for (int i = 0; i < 256; i++)
    {
        DirBlocks[i] = DB0[i];
        DirBlocks[i + 256] = DB1[i];
    }

    for (int i = 0; i < 512; i++)
    {
        Disk[Track[18] + (17 * 256) + i] = DirBlocks[i];
    }

    DeleteBit(18, 17);
    DeleteBit(18, 18);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool StringIsNumeric(string NumericString)
{
    //Remove unwanted spaces
    while (NumericString.find(" ") != string::npos)
    {
        int Pos = NumericString.find(" ");
        NumericString.replace(Pos, 1, "");
    }

    if (NumericString.length() == 0)  //Handle strings with zero length
    {
        return false;
    }


    //Remove HEX prefix
    if (NumericString.at(0) == '$')
    {
        NumericString.replace(0, 1, "");
    }

    string prefix = NumericString.substr(0, 2);

    if ((prefix == "0x") || (prefix == "0X") || (prefix == "&h") || (prefix == "&H"))
    {
        NumericString.replace(0, 2, "");
    }

    //If decimal -> convert it to hex
    if (NumericString.at(0) == '.')
    {
        NumericString.replace(0, 1, "");
        if (IsNumeric(NumericString))
        {
            int StringToInt = ConvertStringToInt(NumericString);
            int hexlen = 2;
            NumericString = ConvertIntToHextString(StringToInt, hexlen);
        }
        else
        {
            return false;
        }
    }

    return IsHexString(NumericString);

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

    if (ScriptEntryArray[i].length() == 0)  //Handle strings with zero length
    {
        return false;
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
    size_t MaxNumChars = 0;
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
        for (size_t j = ScriptEntryArray[i].size(); j < MaxNumChars; j++)
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
    size_t FAN = 0;
    size_t FON = 0;
    size_t FLN = 0;
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
        if (ReadBinaryFile(FN, HSFile) == -1)
        {
            cerr << "***CRITICAL***\tUnable to open Hi-score File\n";
            return false;
        }

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
                cerr << "***CRITICAL***\tFile paramteres are needed for the Hi-Score File: " << FN << "\n";
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
            //    cerr << "***CRITICAL***\tThe Hi-Score File's size must be at least $100 bytes!\n";
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

        //vector<unsigned char>::iterator First = HSFile.begin();
        if (FON + FLN < HSFile.size())
        {
            HSFile.erase(HSFile.begin() + FON + FLN, HSFile.end()); //Trim HSFile from end of file (Offset+Length-1) to end of vector
        }
        if (FON > 0)
        {
            HSFile.erase(HSFile.begin(), HSFile.begin() + FON);                       //Trim HSFile from beginning of file to Offset
        }

        HSFileName = FN + (FUIO ? "*" : "");
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
                //    cerr << "***CRITICAL***\tThe Hi-Score File's size must be at least $100 bytes!\n";
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

            HSFileName = FN + (FUIO ? "*" : "");
            HSAddress = FAN;
            HSOffset = FON;
            HSLength = FLN;

            bSaverPlugin = true;
        }
        else
        {
            cerr << "***CRITICAL***\tThe Hi-Score File " << HSFileName << " doesn't exist and an empty Hi-Score File could not be created without all 3 parameters.\n";
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

    tmpPrgs.clear();

    VFileCnt = -1;

    tmpVFiles.clear();

    BundleCnt++;

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

    Disk[BAM + (18 * 4) + 0] = 17;      // Track 18 - 17 out of 19 sectors free (sectors 0-1 are used by default)
    Disk[BAM + (18 * 4) + 1] = 252;     // 11111100 - Sector 0 (BAM) and Sector 1 (First Dir sector) are already in use by default

    CT = 18;
    CS = 1;

    //SetMaxSector();

    Disk[BAM + (CS * 256) + 1] = 255;   // First dir sector will start with 00 FF

    NextTrack = 1;
    NextSector = 0;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool ResetDiskVariables() {

    if (DiskCnt == 126) {
        cerr << "***CRITICAL***\tYou have reached the maximum number of disks (127) in this project.\n";
        return false;
    }

    DiskCnt++;

    //Reset Bundle File variables here, to have an empty array for the first compression on a ReBuild
    Prgs.clear();    //this is the one that is needed for the first CompressPart call during a ReBuild

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

    TotalOrigSize = 0;
    TotalCompSize = 0;

    //ReDim ByteSt(-1);
    std::fill_n(ByteSt, ExtBytesPerDisk, 0);
    ResetBuffer();

    D64Name = "";
    DiskHeader = ""; //'"demo disk " + Year(Now).ToString
    DiskID = ""; //'"sprkl"
    DemoName = ""; //'"demo"
    DemoStart = "";
    DirArtName = "";
    LoaderZP = "02";

    ThisID = 255;
    NextID = 255;

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

    BundleCnt = -1;        //'WILL BE INCREASED TO 0 IN ResetBundleVariables
    LoaderBundles = 1;
    FilesInBuffer = 1;

    CurrentBundle = -1;

    //-------------------------------------------------------------

    if (!ResetBundleVariables()) {            //Also adds first bundle

        return false;
    }

    NewBundle = false;

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
            ScriptEntryType += ThisChar;            //Entry Type to lower case
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

        if (ScriptEntryType == EntryTypePath)
        {

            ThisChar = ScriptEntry[Pos++];                  //Keep original D64 path
        }
        else
        {
            ThisChar = tolower(ScriptEntry[Pos++]);     //Lower case everything else
        }

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

    if (LineStart > 0)
    {
        LineStart++;
    }

    //OrigScriptEntry = "";
    ScriptEntry = "";

    char S = Script[LineStart++];

    //Find the first non-linebreak character => LineStart
    while ((S == 13) || (S == 10)) {
        NewBundle = true;
        S = Script[LineStart++];
    }

    //Find the last non-linebreak character => LineEnd
    LineEnd = LineStart--;

    while ((S != 13) && (S != 10) && (LineEnd <= Script.length() - 1)) {
        //OrigScriptEntry += S;
        ScriptEntry += S;// tolower(S);
            S = Script[LineEnd++];
    }

    if (LineEnd == Script.length())
    {
        if ((S != 13) && (S != 10))
        {
            //OrigScriptEntry += S;
            ScriptEntry += S;// tolower(S);
        }
    }
    else
    {
        LineEnd--;
    }

    return SplitScriptEntry();

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
        cerr << "***CRITICAL***\tThe following script was not found and could not be processed: " << SubScriptPath << "\n";
        return false;
    }

    //Find relative path of subscript
    for (int i = sPath.length() - 1; i > 0; i--)
    {
        if ((sPath[i] != '\\') && (sPath[i] != '/'))
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
    //int NumLines = std::count(SubScript.begin(), SubScript.end(), '\n') + 1;

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
        if (Lines[i] != EntryTypeScriptHeader)
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
    string SS2 = (LineEnd < Script.length())? Script.substr(LineEnd, Script.length() - (LineEnd)) : "";
    Script = SS1 + S + SS2;


    //ofstream out1("C:\\Tmp\\SS1.sls");
    //out1 << SS1;
    //out1.close();
    //ofstream out2("C:\\Tmp\\SS2.sls");
    //out2 << SS2;
    //out2.close();
    //WriteTextToFile("C:\\Tmp\\TestScript.sls");

    Lines.clear();

    LineEnd = (LineStart > 0) ? LineStart - 1 : LineStart;

    return true;

}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

int CheckNextIO(int Address, int Length, bool NextFileUnderIO) {

    int LastAddress = Address + Length - 1; //Address of the last byte of a file

    if (LastAddress < 256) {                //On ZP
        return 1;
    }
    else
    {
        return ((LastAddress >= 0xd000) && (LastAddress < 0xe000) && (NextFileUnderIO)) ? 1 : 0;    //Last byte under I/O?
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool CompressBundle() {             //NEEDS PackFile() and CloseFile()

    int PreBCnt = BufferCnt;

    if (Prgs.size() == 0)
        return true;

    //DO NOT RESET ByteSt AND BUFFER VARIABLES HERE!!!

    if ((BufferCnt == 0) && (BytePtr == 255))
    {
        NewBlock = SetNewBlock;
        SetNewBlock = false;
    }
    else
    {
        //----------------------------------------------------------------------------------
        //"SPRITE BUG"
        //Compression bug involving the transitional block - FIXED
        //Fix: include the I/O status of the first file of this bundle in the calculation for
        //finishing the previous bundle
        //----------------------------------------------------------------------------------
        //Before finishing the previous bundle, calculate I/O status of the LAST BYTE of the first file of this bundle
        //(Files already sorted)

        int ThisBundleIO = (Prgs.size() > 0) ? CheckNextIO(Prgs[0].iFileAddr, Prgs[0].iFileLen, Prgs[0].FileIO): 0;
        if (!CloseBundle(ThisBundleIO, false))
            return false;
    }

    //-------------------------------------------------------
    //SAVE CURRENT BIT POINTER AND BUFFER COUNT FOR DIRECTORY
    //-------------------------------------------------------

    if (BundleNo < 128)
    {
        DirBlocks[(BundleNo * 4) + 3] = BitPtr;
        DirPtr[BundleNo] = BufferCnt;
        BundleNo++;
    }
    else
    {
        MaxBundleNoExceeded = true;
    }

    //-------------------------------------------------------

    NewBundle = true;
    LastFileOfBundle = false;

    PartialFileIndex = -1;
    int OrigSize{};

    cout << "Compressing bundle #" << (BundleNo - 1) << "...\t";

    for (size_t i = 0; i < Prgs.size(); i++)
    {
        //Mark the last file in a bundle for better compression
        if (i == Prgs.size() - 1)
            LastFileOfBundle = true;

        //The only two parameters that are needed are FA and FUIO... FileLenA(i) is not used

        if (PartialFileIndex == -1)
            PartialFileOffset = Prgs[i].Prg.size() - 1;

        OrigSize += (Prgs[i].iFileLen + 253) / 254;

        PackFile(i);
        if (i < Prgs.size() - 1)
        {
            //WE NEED TO USE THE NEXT FILE'S ADDRESS, LENGTH AND I/O STATUS HERE
            //FOR I/O BYTE CALCULATION FOR THE NEXT PART - BUG reported by Raistlin/G*P
            PrgAdd = Prgs[i + 1].iFileAddr;
            PrgLen = Prgs[i + 1].iFileLen;
            FileUnderIO = Prgs[i + 1].FileIO;
            CloseFile();
        }
    }
    LastBlockCnt = BlockCnt;
    int BC = BufferCnt - PreBCnt;
    if (BundleNo == 1)
        BC++;

    int FirstT = TabT[PreBCnt];
    int FirstS = TabS[PreBCnt];
    int LastT = TabT[BufferCnt];
    int LastS = TabS[BufferCnt];

    TotalOrigSize += OrigSize;
    TotalCompSize += BC;

    cout << OrigSize << " block" << ((OrigSize == 1) ? " " : "s") << (OrigSize < 10 ? "  " : (OrigSize < 100 ? " " : "")) << "  ->\t" << BC << " block" << ((BC == 1) ? " " : "s") << "\t(" << (BC * 100 / OrigSize) << "%)\t\t"
        << ((FirstT < 10) ? "0" : "") << FirstT << ":" << ((FirstS < 10) ? "0" : "") << FirstS << " - " << ((LastT < 10) ? "0" : "") << LastT << ":" << ((LastS < 10) ? "0" : "") << LastS << "\n";

    if (LastBlockCnt > 255)
    {
        //Parts cannot be larger than 255 blocks compressed
        cerr << "***CRITICAL***\tBundle exceeds 255-block limit! Bundle " << BundleCnt << " would need " << LastBlockCnt << " blocks on the disk.\n";
        return false;
    }

    //IF THE WHOLE Bundle IS LESS THAN 1 BLOCK, THEN "IT DOES NOT COUNT", Bundle Counter WILL NOT BE INCREASED
    if ((PreBCnt == BufferCnt) && (BundleCnt != 0))
        BundleCnt--;

    return true;

}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool CompareFileAddress(FileStruct F1, FileStruct F2) {

    return (F1.iFileAddr > F2.iFileAddr);

}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool SortVirtualFiles()
{
    if (tmpVFiles.size() > 1)
    {
        int FSO{}, FEO{}, FSI{}, FEI{};

        //Check for overlaps
        for (size_t o = 0; o < tmpVFiles.size() - 1; o++)
        {
            FSO = tmpVFiles[o].iFileAddr;               //Outer loop file start
            FEO = FSO + tmpVFiles[o].iFileLen - 1;     //Outer loop file end
            for (size_t i = o + 1; i < tmpVFiles.size(); i++)
            {
                FSI = tmpVFiles[i].iFileAddr;           //Inner loop file start
                FEI = FSI + tmpVFiles[i].iFileLen - 1; //Inner loop file end

                //--|------+------|----OR----|------+------|----OR----|------+------|----OR-----|------+------|--
                //  FSO    FSI    FEO        FSO    FEI    FEO        FSI    FSO    FEI        FSI    FEO    FEI

                if (((FSI >= FSO) && (FSI <= FEO)) || ((FEI >= FSO) && (FEI <= FEO)) || ((FSO >= FSI) && (FSO <= FEI)) || ((FEO >= FSI) && (FEO <= FEI))) {
                    int OLS = (FSO >= FSI) ? FSO : FSI;
                    int OLE = (FEO <= FEI) ? FEO : FEI;

                    if ((OLS >= 0xD000) && (OLE <= 0xDFFF) && (tmpVFiles[o].FileIO != tmpVFiles[i].FileIO))
                    {
                        //Overlap is IO memory only and different IO status - NO OVERLAP
                    }
                    else
                    {
                        cerr << "***CRITICAL***\tThe following two virtual files overlap in Bundle " << dec << (BundleCnt - 1) << ":\n"
                            << tmpVFiles[i].FileName << " ($" << tmpVFiles[i].FileAddr << " - $" << hex << FEI << ")\n"
                            << tmpVFiles[o].FileName << " ($" << tmpVFiles[o].FileAddr << " - $" << hex << FEO << ")\n";
                        return false;
                    }
                }
            }
        }

        //Append adjacent files
        bool Change = true;

        while (Change)
        {

            Change = false;

            for (size_t o = 0; o < tmpVFiles.size() - 1; o++)
            {
                FSO = tmpVFiles[o].iFileAddr;
                FEO = tmpVFiles[o].iFileLen;

                for (size_t i = o + 1; i < tmpVFiles.size(); i++)
                {
                    FSI = tmpVFiles[i].iFileAddr;
                    FEI = tmpVFiles[i].iFileLen;


                    if (FSO + FEO == FSI)
                    {
                        if ((FSI <= 0xd000) || (FSI > 0xdfff) || (tmpVFiles[o].FileIO == tmpVFiles[i].FileIO))
                        {
                            tmpVFiles[o].Prg.insert(tmpVFiles[o].Prg.end(), tmpVFiles[i].Prg.begin(), tmpVFiles[i].Prg.end());

                            Change = true;
                        }
                    }
                    else if (FSI + FEI == FSO)
                    {
                        if ((FSO <= 0xd000) || (FSO > 0xdfff) || (tmpVFiles[o].FileIO == tmpVFiles[i].FileIO))
                        {
                            tmpVFiles[i].Prg.insert(tmpVFiles[i].Prg.end(), tmpVFiles[o].Prg.begin(), tmpVFiles[o].Prg.end());

                            tmpVFiles[o].Prg = tmpVFiles[i].Prg;

                            tmpVFiles[o].FileAddr = tmpVFiles[i].FileAddr;
                            tmpVFiles[o].iFileAddr = tmpVFiles[i].iFileAddr;
                            tmpVFiles[o].FileOffs = tmpVFiles[i].FileOffs;
                            tmpVFiles[o].iFileOffs = tmpVFiles[i].iFileOffs;

                            Change = true;
                        }
                    }

                    if (Change)
                    {
                        //Update merged file's IO status
                        tmpVFiles[o].FileIO = (tmpVFiles[o].FileIO || tmpVFiles[i].FileIO);

                        //New file's length is the length of the two merged files
                        FEO += FEI;
                        tmpVFiles[o].FileLen = ConvertIntToHextString(FEO, 4);
                        tmpVFiles[o].iFileLen = FEO;

                        //Remove File(I) and all its parameters
                        vector<FileStruct>::iterator iter = tmpVFiles.begin() + i;
                        tmpVFiles.erase(iter);

                        //One less file left
                        FileCnt--;
                        break;
                    }
                }
                if (Change)
                    break;
            }
        }
        //Sort files by length (short files first, thus, last block will more likely contain 1 file only = faster depacking)
        sort(tmpVFiles.begin(), tmpVFiles.end(), CompareFileAddress);
    }

    return true;

}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool SortBundle() {

    if (tmpPrgs.size() > 0)
    {
        if (tmpPrgs.size() > 1)
        {
            int FSO{}, FEO{}, FSI{}, FEI{};

            //Check for overlaps
            for (size_t o = 0; o < tmpPrgs.size() - 1; o++)
            {
                FSO = tmpPrgs[o].iFileAddr;               //Outer loop file start
                FEO = FSO + tmpPrgs[o].iFileLen - 1;     //Outer loop file end
                for (size_t i = o + 1; i < tmpPrgs.size(); i++)
                {
                    FSI = tmpPrgs[i].iFileAddr;           //Inner loop file start
                    FEI = FSI + tmpPrgs[i].iFileLen - 1; //Inner loop file end

                //--|------+------|----OR----|------+------|----OR----|------+------|----OR-----|------+------|--
                //  FSO    FSI    FEO        FSO    FEI    FEO        FSI    FSO    FEI        FSI    FEO    FEI

                    if(((FSI >= FSO) && (FSI <= FEO)) || ((FEI >= FSO) && (FEI <= FEO)) || ((FSO >= FSI) && (FSO <= FEI)) || ((FEO >= FSI) && (FEO <= FEI))) {
                        int OLS = (FSO >= FSI) ? FSO : FSI;
                        int OLE = (FEO <= FEI) ? FEO : FEI;

                        if ((OLS >= 0xD000) && (OLE <= 0xDFFF) && (tmpPrgs[o].FileIO != tmpPrgs[i].FileIO))
                        {
                            //Overlap is IO memory only and different IO status - NO OVERLAP
                        }
                        else
                        {
                            cerr << "***CRITICAL***\tThe following two files overlap in Bundle " << dec << (BundleCnt - 1) << ":\n"
                            << tmpPrgs[i].FileName << " ($" << tmpPrgs[i].FileAddr << " - $" << hex << FEI << ")\n"
                            << tmpPrgs[o].FileName << " ($" << tmpPrgs[o].FileAddr << " - $" << hex << FEO << ")\n";
                            return false;
                        }
                    }
                }
            }

            //Append adjacent files
            bool Change=true;

            while (Change)
            {

                Change = false;

                for (size_t o = 0; o < tmpPrgs.size() - 1; o++)
                {
                    FSO = tmpPrgs[o].iFileAddr;
                    FEO = tmpPrgs[o].iFileLen;

                    for (size_t i = o + 1; i < tmpPrgs.size(); i++)
                    {
                        FSI = tmpPrgs[i].iFileAddr;
                        FEI = tmpPrgs[i].iFileLen;


                        if (FSO + FEO == FSI)
                        {
                            if ((FSI <= 0xd000) || (FSI > 0xdfff) || (tmpPrgs[o].FileIO == tmpPrgs[i].FileIO))
                            {
                                tmpPrgs[o].Prg.insert(tmpPrgs[o].Prg.end(), tmpPrgs[i].Prg.begin(), tmpPrgs[i].Prg.end());

                                Change = true;
                            }
                        }
                        else if (FSI + FEI == FSO)
                        {
                            if ((FSO <= 0xd000) || (FSO > 0xdfff) || (tmpPrgs[o].FileIO == tmpPrgs[i].FileIO))
                            {
                                tmpPrgs[i].Prg.insert(tmpPrgs[i].Prg.end(), tmpPrgs[o].Prg.begin(), tmpPrgs[o].Prg.end());

                                tmpPrgs[o].Prg = tmpPrgs[i].Prg;

                                tmpPrgs[o].FileAddr = tmpPrgs[i].FileAddr;
                                tmpPrgs[o].iFileAddr = tmpPrgs[i].iFileAddr;
                                tmpPrgs[o].FileOffs = tmpPrgs[i].FileOffs;
                                tmpPrgs[o].iFileOffs = tmpPrgs[i].iFileOffs;

                                Change = true;
                            }
                        }

                        if (Change)
                        {
                            //Update merged file's IO status
                            tmpPrgs[o].FileIO = (tmpPrgs[o].FileIO || tmpPrgs[i].FileIO);

                            //New file's length is the length of the two merged files
                            FEO += FEI;
                            tmpPrgs[o].FileLen = ConvertIntToHextString(FEO, 4);
                            tmpPrgs[o].iFileLen = FEO;

                            //Remove File(I) and all its parameters
                            vector<FileStruct>::iterator iter = tmpPrgs.begin() + i;
                            tmpPrgs.erase(iter);

                            //One less file left
                            FileCnt--;
                            break;
                        }
                    }
                    if (Change)
                        break;
                }
            }
            //Sort files by length (short files first, thus, last block will more likely contain 1 file only = faster depacking)
            sort(tmpPrgs.begin(), tmpPrgs.end(), CompareFileAddress);
            //for (auto x : tmpPrgs)
            //    cout << x.FileName << "\t" << x.FileAddr << "\n";
       }
        //Once Bundle is sorted, calculate the I/O status of the last byte of the first file and the number of bits that will be needed
        //to finish the last block of the previous bundle (when the I/O status of the just sorted bundle needs to be known)
        //This is used in CloseBuffer

        //Bytes needed: (1)LongMatch Tag, (2)NextBundle Tag, (3)AdLo, (4)AdHi, (5)First Lit, (6)1 Bit Stream Byte (for 1 Lit Bit), (7)+/- I/O
        //+/- 1 Match Bit (if the last sequence of the last bundle is a match sequence, no Match Bit after a Literal sequence)
        //Match Bit will be determened by MLen in SequenceFits() function, NOT ADDED TO BitsNeededForNextBundle here!!!

        //We may be overcalculating here but that is safer than undercalculating which would result in buggy decompression
        //If the last block is not the actual last block of the bundle...
        //With overcalculation, worst case scenario is a little bit worse compression ratio of the last block

       BitsNeededForNextBundle = (6 + CheckNextIO(tmpPrgs[0].iFileAddr, tmpPrgs[0].iFileLen, tmpPrgs[0].FileIO)) * 8;

       // +/- 1 Match Bit which will be added later in CloseBuffer if needed

    }

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

        //Also sort virtual files in bundle
        if (!SortVirtualFiles())
            return false;

        //Then compress files and add them to bundle
        if (!CompressBundle())
            return false;     //THIS WILL RESET NewPart TO FALSE

        Prgs = tmpPrgs;

        SetNewBlock = TmpSetNewBlock;
        TmpSetNewBlock = false;

        VFiles = tmpVFiles;

        //Then reset bundle variables (file vectors, prg vector, block cnt), increase bundle counter
        ResetBundleVariables();
    }

    return true;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool AddVirtualFile()
{

    if (NewBundle) {
        NewBundle = false;
        if (!BundleDone())
            return false;
    }

    string FN = ScriptEntryArray[0];
    string FA = "";
    string FO = "";
    string FL = "";
    size_t FAN = 0;
    size_t FON = 0;
    size_t FLN = 0;
    bool FUIO = false;

    vector<unsigned char> P;

    if (FN.find(":") == string::npos)
    {
        FN = ScriptPath + FN;
    }

    int NumParams = 1;

    //Correct file parameter lengths to 4-8 characters
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

    if (FN.find("*") == FN.length() - 1)
    {
        FUIO = true;
        FN.replace(FN.length() - 1, 1, "");
    }

    //Convert string to lowercase
    for (size_t i = 0; i < FN.length(); i++)
        FN[i] = tolower(FN[i]);

    //Get file variables from script, or get default values if there were none in the script entry
    if (FileExits(FN))
    {
        if (ReadBinaryFile(FN, P) == -1)
        {
            cerr << "***CRITICAL***\tUnable to open the following file: " << FN << "\n";
            return false;
        };

        switch (NumParams)
        {
        case 1:  //No parameters in script

            if ((FN[FN.length() - 4] == '.') && (FN[FN.length() - 3] == 's') && (FN[FN.length() - 2] == 'i') && (FN[FN.length() - 1] == 'd'))
            {   //SID file
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
                cerr << "***CRITICAL***\tFile parameteres are needed for the following Mem entry: " << ScriptEntryType << "\t" << ScriptEntry << "\n";
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
                cerr << "***CRITICAL***\tInvalid file offset detected in the following Mem entry: " << ScriptEntryType << "\t" << ScriptEntry << "\n";
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
                cerr << "***CRITICAL***\tInvalid file offset detected in the following Mem entry: " << ScriptEntryType << "\t" << ScriptEntry << "\n";
                return false;
            }
        }

        FAN = ConvertHexStringToInt(FA);
        FON = ConvertHexStringToInt(FO);
        FLN = ConvertHexStringToInt(FL);

        //Make sure file length is not longer than actual file
        if (FON + FLN > P.size())
        {
            cerr << "***CRITICAL***\tInvalid file length detected in the following Mem entry: " << ScriptEntryType << "\t" << ScriptEntry << "\n";
            return false;
        }

        //Make sure file address+length<=&H10000
        if (FAN + FLN > 0x10000)
        {
            cerr << "***CRITICAL***\tInvalid file address and/or length detected in the following Mem entry: " << ScriptEntryType << "\t" << ScriptEntry << "\n";
            return false;
        }

        //Trim file to the specified chunk (FLN number of bytes starting at FON, to Address of FAN)
        if (FON + FLN < P.size())
        {
            //Trim P from end of file (Offset+Length-1) to end of vector
            vector<unsigned char>::iterator First = P.begin() + FON + FLN;
            vector<unsigned char>::iterator Last = P.end();
            P.erase(First, Last);
        }
        if (FON > 0)
        {
            //Trim P from beginning of file to Offset
            vector<unsigned char>::iterator First = P.begin();
            vector<unsigned char>::iterator Last = P.begin() + FON;
            P.erase(First, Last);
        }
    }
    else
    {
        cerr << "***CRITICAL***\tThe following Mem file does not exist: " << FN << "\n";
        return false;
    }

    VFileCnt += 1;

    tmpVFiles.push_back(FileStruct(P, FN, FA, FO, FL, FUIO));

    return true;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool AddFileToBundle() {

    string FN = ScriptEntryArray[0];
    string FA = "";
    string FO = "";
    string FL = "";
    size_t FAN = 0;
    size_t FON = 0;
    size_t FLN = 0;
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
    //for (int i = 0; i < FN.length(); i++)
        //FN[i] = tolower(FN[i]);

    //Get file variables from script, or get default values if there were none in the script entry
    if (FileExits(FN))
    {
        //P.clear();
        if (ReadBinaryFile(FN, P) == -1)
        {
            cerr << "***CRITICAL***\tUnable to open the following file: " << FN << "\n";
            return false;
        }

        switch (NumParams)
        {
        case 1:  //No parameters in script

            if ((FN[FN.length() - 4] == '.') && (FN[FN.length() - 3] == 's')&& (FN[FN.length() - 2] == 'i')&& (FN[FN.length() - 1] == 'd'))
            {   //SID file
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
                cerr << "***CRITICAL***\tFile parameteres are needed for the following File entry: " << ScriptEntryType << "\t" << ScriptEntry << "\n";
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
                cerr << "***CRITICAL***\tInvalid file offset detected in the following File entry: " << ScriptEntryType << "\t" << ScriptEntry << "\n";
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
                cerr << "***CRITICAL***\tInvalid file offset detected in the following File entry: " << ScriptEntryType << "\t" << ScriptEntry << "\n";
                return false;
            }
        }

        FAN = ConvertHexStringToInt(FA);
        FON = ConvertHexStringToInt(FO);
        FLN = ConvertHexStringToInt(FL);

        //Make sure file length is not longer than actual file
        if(FON + FLN > P.size())
        {
            cerr << "***CRITICAL***\tInvalid file length detected in the following File entry: " << ScriptEntryType << "\t" << ScriptEntry << "\n";
            return false;
        }

        //Make sure file address+length<=&H10000
        if (FAN + FLN > 0x10000)
        {
            cerr << "***CRITICAL***\tInvalid file address and/or length detected in the following File entry: " << ScriptEntryType << "\t" << ScriptEntry << "\n";
            return false;
        }

        //Trim file to the specified chunk (FLN number of bytes starting at FON, to Address of FAN)
        if (FON + FLN < P.size())
        {
            //Trim P from end of file (Offset+Length-1) to end of vector
            vector<unsigned char>::iterator First = P.begin() + FON + FLN;
            vector<unsigned char>::iterator Last = P.end();
            P.erase(First, Last);
        }
        if (FON > 0)
        {
            //Trim P from beginning of file to Offset
            vector<unsigned char>::iterator First = P.begin();
            vector<unsigned char>::iterator Last = P.begin() + FON;
            P.erase(First, Last);
        }
    }
    else
    {
        cerr << "***CRITICAL***\tThe following file does not exist: " << FN << "\n";
        return false;
    }

    UncompBundleSize += FLN / static_cast<double>(256);
    if (FLN % 256 != 0)
    {
        UncompBundleSize++;
    }

    if(FirstFileOfDisk)                     //If Demo Start is not specified, we will use the start address of the first file
    {
        FirstFileStart = FA;
        FirstFileOfDisk = false;
    }

    FileCnt++;

    tmpPrgs.push_back(FileStruct(P, FN, FA, FO, FL, FUIO));

    return true;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool AddFile() {

    if (NewBundle)
    {
        if (!BundleDone())
        {
            return false;
        }
    }

    //Then add file to bundle
    if (!AddFileToBundle())
    {
        return false;
    }

    return true;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

void FindNextDirSector() {

    int LastDirSector = DirSector;

    if (DirSector < 6)
    {
        DirSector++;

        Disk[Track[DirTrack] + (LastDirSector * 256)] = DirTrack;
        Disk[Track[DirTrack] + (LastDirSector * 256) + 1] = DirSector;

        Disk[Track[DirTrack] + (DirSector * 256)] = 0;
        Disk[Track[DirTrack] + (DirSector * 256) + 1] = 255;

        DeleteBit(DirTrack, DirSector);
    }
    else
    {
        DirSector = 0;
    }

    /*
    if (DirSector != 0)
    {
        Disk[Track[DirTrack] + (LastDirSector * 256)] = DirTrack;
        Disk[Track[DirTrack] + (LastDirSector * 256) + 1] = DirSector;

        Disk[Track[DirTrack] + (DirSector * 256)] = 0;
        Disk[Track[DirTrack] + (DirSector * 256) + 1] = 255;
    }
    */

}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

void FindNextDirPos() {

    DirPos = 0;

    while (DirSector != 0)
    {
        for (int i = 2; i < 256; i += 32)
        {
            if (Disk[Track[DirTrack] + (DirSector * 256) + i] == 0)
            {
                DirPos = i;
                return;
            }
        }
        FindNextDirSector();
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

void AddAsmDirEntry(string DirEntry) {

    string EntrySegments[5];
    string delimiter = "\"";        // = "
    int NumSegments = 0;

    for (int i = 0; i < 5; i++)
    {
        EntrySegments[i] = "";
    }

    while ((DirArt.find(delimiter) != string::npos) && (NumSegments < 4))
    {
        EntrySegments[NumSegments++] = DirEntry.substr(0, DirEntry.find(delimiter));
        DirEntry.erase(0, DirEntry.find(delimiter) + delimiter.length());
    }
    EntrySegments[NumSegments++] = DirEntry;

    //EntrySegments[0] = '[name =' OR '[name = @'
    //EntrySegments[1] = 'text' OR '\$XX\$XX\$XX'
    //EntrySegments[2] = 'type ='
    //EntrySegments[3] = 'del' OR 'prg' etc.
    //EntrySegments[4] = ']'

    if (NumSegments > 1)
    {
        if ((EntrySegments[0].find("[") != string::npos) && (EntrySegments[0].find("name") != string::npos) && (EntrySegments[0].find("=") != string::npos))
        {
            unsigned char FileType = 0x80;  //DEL default file type

            if (NumSegments > 3)
            {
                if ((EntrySegments[2].find("type") != string::npos) && (EntrySegments[2].find("=") != string::npos))
                {
                    if (EntrySegments[3] == "*seq")
                    {
                        FileType = 0x01;
                    }
                    else if (EntrySegments[3] == "*prg")
                    {
                        FileType = 0x02;
                    }
                    else if (EntrySegments[3] == "*usr")
                    {
                        FileType = 0x03;
                    }
                    else if (EntrySegments[3] == "del<")
                    {
                        FileType = 0xc0;
                    }
                    else if (EntrySegments[3] == "seq<")
                    {
                        FileType = 0xc1;
                    }
                    else if (EntrySegments[3] == "prg<")
                    {
                        FileType = 0xc2;
                    }
                    else if (EntrySegments[3] == "usr<")
                    {
                        FileType = 0xc3;
                    }
                    else if (EntrySegments[3] == "rel<")
                    {
                        FileType = 0xc4;
                    }
                    else if (EntrySegments[3] == "seq")
                    {
                        FileType = 0x81;
                    }
                    else if (EntrySegments[3] == "prg")
                    {
                        FileType = 0x82;
                    }
                    else if (EntrySegments[3] == "usr")
                    {
                        FileType = 0x83;
                    }
                    else if (EntrySegments[3] == "rel")
                    {
                        FileType = 0x84;
                    }
                    //else
                    //{
                    //    FileType = 0x80;
                    //}
                }
            }

            if (Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 0] == 0)
            {
                Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 0] = FileType;      //All dir entries will point at first file in dir
                Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 1] = 18;            //Track 18 (track pointer of boot loader)
                Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 2] = 7;             //Sector 7 (sector pointer of boot loader)
            }

            if (EntrySegments[0].find("@") != string::npos)
            {
                //Numeric entry
                unsigned char Entry[16];
                string Values[16];

                //Fill array with 16 SHIFT+SPACE characters
                fill_n(Entry, 16, 0xa0);
                fill_n(Values, 16, "");
                //for (int i = 0; i < 16; i++)
                //{
                    //Entry[i] = 0xa0;
                    //Values[i] = "";
                //}

                int NumValues = 0;
                delimiter = "\\";
                while ((EntrySegments[1].find(delimiter) != string::npos) && (NumValues < 15))
                {
                    string ThisValue = EntrySegments[1].substr(0, EntrySegments[1].find(delimiter));
                    if (ThisValue != "")
                    {
                        Values[NumValues++] = ThisValue;
                    }
                    EntrySegments[1].erase(0, EntrySegments[1].find(delimiter) + delimiter.length());
                }
                Values[NumValues++] = EntrySegments[1];

                int Idx = 0;

                for (int v = 0; v < NumValues; v++)
                {
                    if (Values[v] != "")
                    {
                        unsigned char NextChar = 0x20;          //SPACE default char - if conversion is not possible
                        if (Values[v].find("$") == 0)
                        {
                            //Hex Entry
                            Values[v].erase(0, 1);
                            if (IsHexString(Values[v]))
                            {
                                NextChar = ConvertHexStringToInt(Values[v]);
                            }
                            else
                            {
                                break;
                            }
                        }
                        else
                        {
                            //Decimal Entry
                            if (IsNumeric(Values[v]))
                            {
                                NextChar = ConvertStringToInt(Values[v]);
                            }
                            else
                            {
                                break;
                            }
                        }
                        Entry[Idx++] = NextChar;
                        if (Idx == 16)
                        {
                            break;
                        }
                    }
                }
                for (int i = 0; i < 16; i++)
                {
                    Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 3 + i] = Entry[i];
                }
            }
            else
            {
                //Text Entry, pad it with inverted space
                //Fill the slot first with $A0
                for (int i = 0; i < 16; i++)
                {
                    Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 3 + i] = 0xa0;
                }
                string ThisEntry = EntrySegments[1];
                for (size_t i = 0; (i < ThisEntry.length()) && (i < 16); i++)
                {
                    unsigned char NextChar = toupper(ThisEntry[i]);
                    Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 3 + i] = Ascii2DirArt[NextChar];
                }
            }
        }
    }

    if ((DirTrack == 18) && (DirSector == 1) && (DirPos == 2))
    {
        //Very first dir entry MUST be PRG, also add loader block count
        Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 0] = 0x82;
        Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 0x1c] = LoaderBlockCount;
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

void ConvertKickassAsmToDirArt() {

    DirArt = ReadFileToString(DirArtName);

    if (DirArt == "")
    {
        cerr << "***INFO***\tUnable to open the following DirArt file: " << DirArtName << "\n The disk will be built without DirArt.\n";
        return;
    }

    //Convert the whole string to lower case for easier processing
    for (size_t i = 0; i < DirArt.length(); i++)
    {
        DirArt[i] = tolower(DirArt[i]);
    }

    string delimiter = "\n";
    DirTrack = 18;
    DirSector = 1;

    while (DirArt.find(delimiter) != string::npos)
    {
        string DirEntry = DirArt.substr(0, DirArt.find(delimiter));
        DirArt.erase(0, DirArt.find(delimiter) + delimiter.length());
        FindNextDirPos();
        if (DirPos != 0)
        {
            AddAsmDirEntry(DirEntry);   //Convert one line at the time
        }
        else
        {
            break;
        }
    }

    if ((DirArt != "") && (DirPos != 0))
    {
        FindNextDirPos();
        if (DirPos != 0)
        {
            AddAsmDirEntry(DirArt);
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool AddCArrayDirEntry(int RowLen)
{
    DirEntryAdded = false;

    if (DirEntry.find(0x0d) != string::npos)        //Make sure 0x0d is removed from end of string
    {
        DirEntry = DirEntry.substr(0, DirEntry.find(0x0d));
    }

    string Entry[16];
    size_t EntryStart = 0;
    int NumEntries = 0;
    string delimiter = ",";

    while ((EntryStart = DirEntry.find(delimiter)) != string::npos)
    {
        if (DirEntry.substr(0, EntryStart) != "")
        {
            Entry[NumEntries++] = DirEntry.substr(0, EntryStart);
        }

        DirEntry.erase(0, EntryStart + delimiter.length());

        if (NumEntries == RowLen)   //Max row length reached, ignore rest of the char row
        {
            break;
        }
    }

    if ((NumEntries < 16) && (DirEntry != ""))
    {
        for (size_t i = 0; i < DirEntry.length(); i++)
        {
            string S = DirEntry.substr(0, 1);
            DirEntry.erase(0, 1);
            if (IsNumeric(S))
            {
                Entry[NumEntries] += S;
            }
            else
            {
                break;
            }

        }
        NumEntries++;
    }

    if (NumEntries == RowLen)       //Ignore rows that are too short (e.g.
    {
        unsigned char bEntry[16]{};
        bool AllNumeric = true;
        int V = 0;

        for (int i = 0; i < NumEntries; i++)
        {
            string prefix = Entry[i].substr(0, 2);
            if ((prefix == "0x") || (prefix == "&h"))
            {
                Entry[i].erase(0, 2);
                if (IsHexString(Entry[i]))
                {
                    bEntry[V++] = ConvertHexStringToInt(Entry[i]);
                }
                else
                {
                    AllNumeric = false;
                    break;
                }
            }
            else if (Entry[i].substr(0, 1) == "&")
            {
                Entry[i].erase(0, 1);
                if (IsHexString(Entry[i]))
                {
                    bEntry[V++] = ConvertHexStringToInt(Entry[i]);
                }
                else
                {
                    AllNumeric = false;
                    break;
                }
            }
            else
            {
                if (IsNumeric(Entry[i]))
                {
                    bEntry[V++] = ConvertStringToInt(Entry[i]);
                }
                else
                {
                    AllNumeric = false;
                    break;
                }
            }
        }

        if (AllNumeric)
        {

            FindNextDirPos();

            if (DirPos != 0)
            {
                Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 0] = 0x82;      //"PRG" -  all dir entries will point at first file in dir
                Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 1] = 18;        //Track 18 (track pointer of boot loader)
                Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 2] = 7;         //Sector 7 (sector pointer of boot loader)

                for (size_t i = 0; i < 16; i++)
                {
                    Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 3 + i] = Petscii2DirArt[bEntry[i]];
                }
                if ((DirTrack == 18) && (DirSector == 1) && (DirPos == 2))
                {
                    //Very first dir entry, also add loader block count
                    Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 0x1c] = LoaderBlockCount;
                }

                DirEntryAdded = true;

            }
            else
            {
                return false;
            }
        }
    }

    return true;

}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

void ConvertCArrayToDirArt() {

    string DA = ReadFileToString(DirArtName);

    if (DA == "")
    {
        cerr << "***INFO***\tUnable to open the following DirArt file: " << DirArtName << "\n The disk will be built without DirArt.\n";
        return;
    }

    //Convert the whole string to lower case for easier processing
    for (size_t i = 0; i < DA.length(); i++)
    {
        DA[i] = tolower(DA[i]);
    }

    //unsigned char* BinFile = new unsigned char[6*256];
    const char *findmeta = "meta:";
    size_t indexMeta = DA.find(findmeta);

    string sRowLen = "0";
    string sRowCnt = "0";

    if (indexMeta != string::npos)
    {
        size_t i = indexMeta;
        while ((i < DA.length()) && (DA[i] != ' '))     //Skip META:
        {
            i++;
        }

        while ((i < DA.length()) && (DA[i] == ' '))      //Skip SPACE between values
        {
            i++;
        }

        while ((i < DA.length()) && (DA[i] != ' '))
        {
            sRowLen += DA[i++];
        }

        while ((i < DA.length()) && (DA[i] == ' '))      //Skip SPACE between values
        {
            i++;
        }

        while ((i < DA.length()) && (DA[i] != ' '))
        {
            sRowCnt += DA[i++];
        }
    }

    DirTrack = 18;
    DirSector = 1;

    int RowLen = min(ConvertStringToInt(sRowLen),16);
    int RowCnt = min(ConvertStringToInt(sRowCnt),48);

    size_t First = DA.find("{");
    //size_t Last = DA.find("}");

    DA.erase(0, First + 1);

    string LineBreak = "\n";
    size_t LineStart = 0;
    int NumRows = 0;
    bool DirFull = false;

    while (((LineStart = DA.find(LineBreak)) != string::npos) && (NumRows < RowCnt))
    {
        DirEntry = DA.substr(0, LineStart);     //Extract one dir entry

        if (!AddCArrayDirEntry(RowLen))
        {
            DirFull = true;
            break;
        }

        DA.erase(0, LineStart + LineBreak.length());

        if (DirEntryAdded)
        {
            NumRows++;
        }
    }

    if ((!DirFull) && (NumRows < RowCnt))
    {
        DirEntry = DA;
        AddCArrayDirEntry(RowLen);
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

void ConvertBinToDirArt(string DirArtType) {

    vector<unsigned char> DA;

    if (ReadBinaryFile(DirArtName, DA)==-1)
    {
        cerr << "***INFO***\tUnable to open the following file: " << DirArtName << "\nThe disk will be built without DirArt.\n";
        return;
    }

    DirTrack = 18;
    DirSector = 1;

    int DAPtr = (DirArtType == "prg") ? 2 : 0;

    for (size_t b = DAPtr; b < DA.size(); b += 40)
    {
        FindNextDirPos();
        if (DirPos != 0)
        {
            Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 0] = 0x82;      //"PRG" -  all dir entries will point at first file in dir
            Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 1] = 18;        //Track 18 (track pointer of boot loader)
            Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 2] = 7;         //Sector 7 (sector pointer of boot loader)

            for (int i = 0; i < 16; i++)
            {
                if (b + i < DA.size())
                {
                    Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 3 + i] = Petscii2DirArt[DA[b + i]];
                }
                else
                {
                    break;
                }
            }
            if ((DirTrack == 18) && (DirSector == 1) && (DirPos == 2))
            {
                //Very first dir entry, also add loader block count
                Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 0x1c] = LoaderBlockCount;
            }
        }
        else
        {
            break;
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

void AddDirEntry(string DirEntry){

    Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 0] = 0x82;      //"PRG" -  all dir entries will point at first file in dir
    Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 1] = 18;        //Track 18 (track pointer of boot loader)
    Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 2] = 7;         //Sector 7 (sector pointer of boot loader)

    //Remove vbNewLine characters and add 16 SHIFT+SPACE tail characters
    for (int i = 0; i < 16; i++)
    {
        DirEntry += 0xa0;
    }

    //Copy only the first 16 characters of the edited DirEntry to the Disk Directory
    for (int i = 0; i < 16; i++)
    {
        Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 3 + i] = toupper(DirEntry[i]);
    }


    if ((DirTrack == 18) && (DirSector == 1) && (DirPos == 2))
    {
        //Very first dir entry, also add loader block count
        Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 0x1c] = LoaderBlockCount;
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

void ConvertTxtToDirArt() {

    DirArt = ReadFileToString(DirArtName);

    if (DirArt == "")
    {
        cerr << "***INFO***\tUnable to open the following DirArt file: " << DirArtName << "\n The disk will be built without DirArt.\n";
        return;
    }

    string delimiter = "\n";
    DirTrack = 18;
    DirSector = 1;

    while (DirArt.find(delimiter) != string::npos)
    {
        string DirEntry = DirArt.substr(0, DirArt.find(delimiter));
        DirArt.erase(0, DirArt.find(delimiter) + delimiter.length());
        FindNextDirPos();
        if (DirPos != 0)
        {
            AddDirEntry(DirEntry);
        }
        else
        {
            break;
        }
    }

    if ((DirArt != "") && (DirPos != 0))
    {
        FindNextDirPos();
        if (DirPos != 0)
        {
            AddDirEntry(DirArt);
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

void ConvertPetToDirArt() {

    //Binary file With a .pet extension that includes the following information, without any padding:
    //- 1 byte: screen width in chars (40 for whole C64 screen)
    //- 1 byte: screen height in chars (25 for whole C64 screen)
    //- 1 byte: border Color($D020)                              - IGNORED BY SPARKLE
    //- 1 byte: background Color($D021)                          - IGNORED BY SPARKLE
    //- 1 byte: 0 for uppercase PETSCII, 1 for lowercase PETSCII - IGNORED BY SPARKLE
    //- <width * height> bytes: video RAM data
    //- <width * height> bytes: Color RAM data                   - IGNORED BY SPARKLE

    vector<unsigned char> PetFile;

    if (ReadBinaryFile(DirArtName, PetFile) == -1)
    {
        cerr << "***INFO***\tUnable to open the following file: " << DirArtName << "\nThe disk will be built without DirArt.\n";
        return;
    }

    unsigned char RowLen = PetFile[0];
    unsigned char RowCnt = PetFile[1] > 48 ? 48 : PetFile[1];

    DirTrack = 18;
    DirSector = 1;

    for (int rc = 0; rc < RowCnt; rc++)
    {
        FindNextDirPos();
        if (DirPos != 0)
        {

            if (Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 0] == 0)
            {
                Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 0] = 0x82;  //"PRG" -  all dir entries will point at first file in dir
                Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 1] = 18;    //Track 18 (track pointer of boot loader)
                Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 2] = 7;     //Sector 7

                if ((DirTrack == 18) && (DirSector == 1) && (DirPos == 2))
                {
                    //Very first dir entry, also add loader block count
                    Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 0x1c] = LoaderBlockCount;
                }

                for (int i = 0; i < 16; i++)
                {
                    Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 3 + i] = 0xa0;
                }

                for (int i = 0; (i < RowLen) && (i < 16); i++)
                {
                    Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 3 + i] = Petscii2DirArt[PetFile[5 + (rc * RowLen) + i]];
                }
            }
        }
        else
        {
            break;
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

void ConvertD64ToDirArt() {

    vector<unsigned char> DA;

    if(ReadBinaryFile(DirArtName, DA)==-1)
    {
        cerr << "***INFO***\tUnable to open the following DirArt file: " << DirArtName << "\n The disk will be built without DirArt.\n";
        return;
    }

    int T = 18;
    int S = 1;

    DirTrack = 18;
    DirSector = 1;

    bool DirFull = false;
    int DAPtr{};

    while ((!DirFull) && (T != 0))
    {
        DAPtr = Track[T] + (S * 256);
        for (int b = 2; b < 256; b += 32)
        {
            if (DA[DAPtr + b] != 0)
            {
                FindNextDirPos();

                if (DirPos != 0)
                {
                    Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 0] = 0x82;      //"PRG" -  all dir entries will point at first file in dir
                    Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 1] = 18;        //Track 18 (track pointer of boot loader)
                    Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 2] = 7;         //Sector 7 (sector pointer of boot loader)

                    for (int i = 0; i < 16; i++)
                    {
                        Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 3 + i] = DA[DAPtr + b + 3 + i];
                    }

                    if ((DirTrack == 18) && (DirSector == 1) && (DirPos == 2))
                    {
                        //Very first dir entry, also add loader block count
                        Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 0x1c] = LoaderBlockCount;
                    }
                }
                else
                {
                    DirFull = true;
                    break;
                }
            }
        }
        T = DA[DAPtr];
        S = DA[DAPtr + 1];
    }


    if(DiskHeader == "")
    {
        for (int i = 0; i < 16; i++)
        {
            Disk[Track[18] + 0x90 + i] = DA[Track[18] + 0x90 + i];
        }
    }

    if (DiskID == "")
    {
        for (int i = 0; i < 5; i++)
        {
            Disk[Track[18] + 0xa2 + i] = DA[Track[18] + 0xa2 + i];
        }
    }

}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

void AddDirArt() {

    if (DirArtName == "")
    {
        return;
    }

    if (!FileExits(DirArtName))
    {
        cerr << "***INFO***\tThe following DirArt file does not exist: " << DirArtName << "\nThe disk will be built without DirArt.\n";
        return;
    }

    string DirArtType = "";

    int ExtStart = DirArtName.length();

    for (int i = DirArtName.length() - 1; i >= 0; i--)
    {
        if ((DirArtName[i] == '\\') || (DirArtName[i] == '/'))
        {
            break;
        }
        else if (DirArtName[i] == '.')
        {
            ExtStart = i + 1;
            break;
        }
    }

    for (size_t i = ExtStart; i < DirArtName.length(); i++)
    {
        DirArtType += tolower(DirArtName[i]);
    }

    if (DirArtType == "d64")                // Copy DirArt from D64
    {
        ConvertD64ToDirArt();
    }
    else if (DirArtType == "txt")           // Convert TXT file to DirArt
    {
        ConvertTxtToDirArt();
    }
    else if (DirArtType == "prg")           // Convert PRG (binary file with header) to DirArt
    {
        ConvertBinToDirArt(DirArtType);
    }
    else if (DirArtType == "c")             // Convert C array to DirArt
    {
        ConvertCArrayToDirArt();
    }
    else if (DirArtType == "asm")           // Convert Kick Assembler file parameter list to DirArt
    {
        ConvertKickassAsmToDirArt();
    }
    else if (DirArtType == "pet")           // Convert PET file to DirArt
    {
        ConvertPetToDirArt();
    }
    else
    {
        ConvertBinToDirArt("");     // Convert any other binary file to DirArt (same as PRG without header)
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

void AddDemoNameToDisk(unsigned char T, unsigned char S) {

    int B;
    string DN = DemoName;
    unsigned char A{};

    if ((DN == "") && (DirArtName != ""))
    {
        //No DemoName defined, check if we have a DirArt file attached
        //Dirart attached, we will add first dir entry there
        return;
    }

    CT = 18;
    CS = 1;

    int Cnt = Track[CT] + (CS * 256);

    while (Disk[Cnt] != 0)
    {
        Cnt = Track[Disk[Cnt]] + (Disk[Cnt + 1] * 256);
    }

    B = 2;

    while (Disk[Cnt + B] != 0x00)
    {
        B += 32;
        if (B > 256)
        {
            B -= 256;
            CS += 4;
            Disk[Cnt] = CT;
            Disk[Cnt + 1] = CS;
            Cnt = Track[CT] + (CS * 256);
            Disk[Cnt] = 0;
            Disk[Cnt + 1] = 255;
        }
    }

    Disk[Cnt + B] = 0x82;
    Disk[Cnt + B + 1] = T;
    Disk[Cnt + B + 2] = S;

    for (int W = 0; W < 16; W++)
    {
        Disk[Cnt + B + 3 + W] = 0xa0;
    }

    for (size_t W = 0; W < DN.length(); W++)
    {
        int C = DN[W];
        A = Ascii2DirArt[C];
        Disk[Cnt + B + 3 + W] = A;
    }
    Disk[Cnt + B + 0x1c] = LoaderBlockCount;    //Length of boot loader in blocks

}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

void AddHeaderAndID() {

    //unsigned char B;
    int BAM = Track[18];

    for (int i = 0x90; i <= 0xaa; i++)
    {
        Disk[BAM + i] = 0xa0;
    }

    if (DiskHeader == "")
    {
        for (int i = 0; i < 16; i++)
        {
            Disk[BAM + 0x90 + i] = 0x20;
        }
    }
    else
    {
        for (size_t i = 0; i < ((DiskHeader.size() <= 16) ? DiskHeader.size() : 16); i++)
        {
            int DH = DiskHeader[i];
            Disk[BAM + 0x90 + i] = Ascii2DirArt[DH];
        }
    }

    if (DiskID == "")
    {
        for (int i = 0; i < 5; i++)
        {
            Disk[BAM + 0xa2 + i] = 0x20;
        }
    }
    else
    {
        for (size_t i = 0; i < ((DiskID.size() <= 5) ? DiskID.size() : 5); i++)
        {
            int DI = DiskID[i];
            Disk[BAM + 0xa2 + i] = Ascii2DirArt[DI];         //Overwrites Disk ID and DOS type (5 characters max.)
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool InjectSaverPlugin() {

    if (HSFile.size() == 0)
    {
        cerr << "***CRITICAL***\tThe Hi-Score File's size must be multiples of $100 bytes, but not greater than $f00 bytes.\n";
        return false;
    }
    if (HSFileName == "")
    {
        cerr << "***CRITICAL***\tThe Hi-Score File's name is not defined.\n";
        return false;
    }
    if (BundleNo > 125)
    {
        cerr << "***CRITICAL***\tThe Hi-Score File Saver Plugin cannot be added to the disk because the number of file bundles exceeds 126!\n"
        << "The Plugin and the Hi-Score File would use bundle indices $7e and $7f, respectively.\n";
        return false;
    }
    //-----------------
    //  Add SaveCode
    //-----------------

    BlocksUsedBySaver = (HSLength / 0x100) + 1 + 2;

    if (BlocksFree < BlocksUsedBySaver)
    {
        cerr << "***CRITICAL***\tThe Hi-Score File and the Saver Plugin cannot be added because there is not enough free space on the disk!\n";
        return false;
    }

    unsigned char SaveCode[512]{};
    int SaveCodeSize{};

    if (HSFileName.find("*") == HSFileName.length() - 1)
    {
        SaverSupportsIO = true;
        HSFileName.replace(HSFileName.length() - 1, 1, "");

        for(int i = 0; i < SSIO_size; i++)
        {
            SaveCode[i] = SSIO[i];
        }
        SaveCodeSize = SSIO_size;
    }
    else
    {
        SaverSupportsIO = false;

        for (int i = 0; i < SS_size; i++)
        {
            SaveCode[i] = SS[i];
        }
        SaveCodeSize = SS_size;
    }

    //UpdateZP BUG REPORTED BY Rico/Pretzel Logic
    //WE ALSO NEED TO UPDATE ZP OFFSET IN THE SAVER CODE!!!

    //Convert LoaderZP to byte - it has already been validated in UpdateZP
    unsigned char ZP = ConvertHexStringToInt(LoaderZP);

    if (ZP != 2)
    {
        unsigned char OPC_STAZP = 0x85;     //ZP, ZP+1, Bits
        unsigned char OPC_LDAZPY = 0xb1;    //ZP

        unsigned char ZPBase = 0x02;

        for (int i = 0; i <= 249; i++)
        {
            if ((SaveCode[i] == OPC_STAZP) || (SaveCode[i] == OPC_LDAZPY))
            {
                if (SaveCode[i + 1] == ZPBase)
                {
                    SaveCode[i + 1] = ZP;
                    i++;
                }
            }
        }

        for (int i = 0; i <= 249; i++)
        {
            if ((SaveCode[i] == OPC_STAZP) || (SaveCode[i + 1] == ZPBase + 1))
            {
                SaveCode[i + 1] = ZP + 1;
                i++;
            }
        }

        for (int i = 0; i <= 249; i++)
        {
            if ((SaveCode[i] == OPC_STAZP) || (SaveCode[i + 1] == ZPBase + 2))
            {
                SaveCode[i + 1] = ZP + 2;
                i++;
            }
        }
    }

    SaveCode[3] = (HSLength / 256) + 1;
    SaveCode[0x13] = (HSAddress - 1) & 0xff;
    SaveCode[0x1a] = (HSAddress - 1) / 0x100;

    //Calculate sector pointer on disk
    int SctPtr = SectorsPerDisk - 2 - ((HSLength / 256) + 1);

    //Identify first T/S of the saver plugin
    CT = TabT[SctPtr];
    CS = TabS[SctPtr];

    //Copy first block of saver plugin to disk
    for (int i = 0; i < 256; i++)
    {
        Disk[Track[CT] + (CS * 256) + i] = SaveCode[i];
    }

    //Mark sector off in BAM
    DeleteBit(CT, CS);

    //Add plugin to directory
    Disk[Track[18] + (18 * 256) + 8] = EORtransform(CT);               //DirBlocks(0) = EORtransform(Track) = 35
    Disk[Track[18] + (18 * 256) + 7] = EORtransform(TabStartS[CT]);    //DirBlocks(1) = EORtransform(Sector) = First sector of Track(35) (not first sector of file!!!)
    Disk[Track[18] + (18 * 256) + 6] = EORtransform(TabSCnt[SctPtr]);  //DirBlocks(2) = EORtransform(Remaining sectors on track)
    Disk[Track[18] + (18 * 256) + 5] = 0xfe;                                //DirBlocks(3) = BitPtr

    //Next Sector
    SctPtr++;

    //Second T/S of saver plugin
    CT = TabT[SctPtr];
    CS = TabS[SctPtr];

    //Copy second block of saver plugin to disk
    for (int i = 0; i < SaveCodeSize - 256; i++)
    {
        int j = 0 - i;
        if (j < 0)
            j += 256;
        Disk[Track[CT] + (CS * 256) + j] = EORtransform(SaveCode[256 + i]);
    }

    //Mark sector off in BAM
    DeleteBit(CT, CS);

    //-----------------
    //  Add SaveFile
    //-----------------

    SctPtr++;

    CT = TabT[SctPtr];
    CS = TabS[SctPtr];

    Disk[Track[18] + (18 * 256) + 4] = EORtransform(CT);                //DirBlocks(0) = EORtransform(Track) = 35
    Disk[Track[18] + (18 * 256) + 3] = EORtransform(TabStartS[CT]);     //DirBlocks(1) = EORtransform(Sector) = First sector of Track(35) (not first sector of file!!!)
    Disk[Track[18] + (18 * 256) + 2] = EORtransform(TabSCnt[SctPtr]);   //DirBlocks(2) = EORtransform(Remaining sectors on track)
    Disk[Track[18] + (18 * 256) + 1] = 0xfe;                                 //DirBlocks(3) = BitPtr

    DeleteBit(CT, CS);

    unsigned char Buffer[256]{};
    int HSStartAdd = HSAddress + HSLength - 1;
    unsigned char BlockCnt = HSLength / 256;


    //First block
    Buffer[0] = 0;
    Buffer[1] = EORtransform((HSLength / 256));                //Remaining block count (EOR transformed)
    Buffer[255] = 0xfe;                                             //First byte of block
    Buffer[254] = 0x81;                                             //Bit stream
    Buffer[253] = HSStartAdd % 256;                                 //Last byte's address(Lo)
    if (SaverSupportsIO)
    {
        Buffer[252] = 0;                                            //I/O flag
        Buffer[251] = HSStartAdd / 256;                             //Last byte's address(Hi)
        Buffer[250] = 0;                                            //LongLit flag
        Buffer[249] = 0xf6;                                         //Number of literals - 1
    }
    else
    {
        Buffer[252] = HSStartAdd / 256;                             //Last byte's address(Hi)
        Buffer[251] = 0;                                            //LongLit flag
        Buffer[250] = 0xf7;                                         //Number of literals - 1
    }

    for (int i = 2; i <= (SaverSupportsIO ? 248 : 249); i++)
    {
        Buffer[i] = HSFile[HSLength - 1 - (SaverSupportsIO ? 248 : 249) + i];
    }

    for (int i = 0; i < 256; i++)
    {
        Disk[Track[CT] + (CS * 256) + i] = Buffer[i];
    }

    if (SaverSupportsIO)
    {
        HSStartAdd -= 0xf7;
        HSLength -= 0xf7;
    }
    else
    {
        HSStartAdd -= 0xf8;
        HSLength -= 0xf8;
    }

    //Blocks 1 to BlockCnt-1
    for (int i = 1; i < BlockCnt; i++)
    {
        SctPtr++;


        CT = TabT[SctPtr];
        CS = TabS[SctPtr];

        DeleteBit(CT, CS);

        memset(Buffer, 0, sizeof(Buffer));


        Buffer[0] = 0x81;                                   //Bit stream
        Buffer[255] = HSStartAdd % 256;                     //Last byte's address(Lo)
        if (SaverSupportsIO)
        {
            Buffer[254] = 0;                                //I/O flag
            Buffer[253] = HSStartAdd / 256;                 //Last byte's address(hi)
            Buffer[252] = 0;                                //LongLit flag
            Buffer[251] = 0xf9;                             //Number of literals - 1
        }
        else
        {
            Buffer[254] = HSStartAdd / 256;                 //Last byte's address(hi)
            Buffer[253] = 0;                                //LongLit flag
            Buffer[252] = 0xfa;                             //Number of literals - 1

        }

        for (int j = 1; j <= (SaverSupportsIO ? 250 : 251); j++)
        {
            Buffer[j] = HSFile[HSLength - 1 - (SaverSupportsIO ? 250 : 251) + j];
        }

        for (int j = 0; j < 256; j++)
        {
            Disk[Track[CT] + (CS * 256) + j] = Buffer[j];
        }

        if (SaverSupportsIO)
        {
            HSStartAdd -= 0xfa;
            HSLength -= 0xfa;
        }
        else
        {
            HSStartAdd -= 0xfb;
            HSLength -= 0xfb;
        }
    }

    //Last block of Hi-Score File

    SctPtr++;

    CT = TabT[SctPtr];
    CS = TabS[SctPtr];

    DeleteBit(CT, CS);

    memset(Buffer, 0, sizeof(Buffer));


    Buffer[0] = 0x81;                                           //Bit stream
    Buffer[1] = EORtransform(0);                           //New block count = 0 (eor transformed)
    Buffer[255] = HSStartAdd % 256;                             //Last byte's address(Lo)
    if (SaverSupportsIO)
    {
        Buffer[254] = 0;                                        //I/O flag
        Buffer[253] = HSStartAdd / 256;                         //Last byte's address(Hi)
        Buffer[252] = 0;                                        //LongLit flag
        Buffer[251] = HSLength - 1;                             //Number of remaining literals - 1

    }
    else
    {
        Buffer[254] = HSStartAdd / 256;                         //Last byte's address(Hi)
        Buffer[253] = 0;                                        //LongLit flag
        Buffer[252] = HSLength - 1;                             //Number of remaining literals - 1

    }

    for (size_t i = 0; i < HSLength; i++)
    {
        Buffer[(SaverSupportsIO ? 251 : 252) - HSLength + i] = HSFile[i];
    }

    Buffer[(SaverSupportsIO ? 251 : 252) - HSLength - 1] = NearLongMatchTag;    //End of Bundle: 0x84
    Buffer[(SaverSupportsIO ? 251 : 252) - HSLength - 2] = EndOfBundleTag;      //End of Bundle: 0x00


    for (int i = 0; i < 256; i++)
    {
        Disk[Track[CT] + (CS * 256) + i] = Buffer[i];
    }

    return true;

}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool InjectDriveCode(unsigned char& idcSideID, char& idcFileCnt, unsigned char& idcNextID) {

    int BAM = Track[18];

    unsigned char Drive[6 * 256]{};

    for (int i = 0; i < SD_size; i++)
    {
        Drive[i] = SD[i];
    }

    unsigned char B3[256]{};
    int B = 0;

    //Resort and EOR transform Block 3
    for (int i = 0; i < 256; i++)
    {
        B3[B] = EORtransform(Drive[(256 * 3) + i]);
        B--;
        if (B < 0)
            B += 256;
    }

    //-------------------
    //   VersionInfo
    //-------------------
    //Add version info: YY MM DD VV

    int VI = 0x5b;
    Drive[VI + 0] = VersionBuild >> 16;
    Drive[VI + 1] = (VersionBuild & 0xff00) >> 8;
    Drive[VI + 2] = VersionBuild & 0xff;
    Drive[VI + 4] = (VersionMajor << 4) + VersionMinor;

    //-------------------
    //   ProductID
    //-------------------
    //Add Product ID
    int PID = 0x1b;
    Drive[PID + 0] = ProductID >> 16;
    Drive[PID + 1] = (ProductID & 0xff00) >> 8;
    Drive[PID + 2] = ProductID & 0xff;

    //Resort blocks in drive code:
    for (int i = 0; i < 256; i++)
    {
        Drive[(3 * 256) + i] = Drive[(4 * 256) + i];    //Copy ZP GCR Tab and GCR loop to block 3 for loading
        Drive[(4 * 256) + i] = Drive[(5 * 256) + i];    //Copy Init code to block 4 for loading
        Drive[(5 * 256) + i] = B3[i];                   //Copy original block 3 EOR transformed to block 5 to be loaded by init code

    }

    //Add NextID and IL0-IL3 to ZP
    int ZPILTabLoc = 0x60;

    Drive[(3 * 256) + ZPILTabLoc + 0] = 256 - IL3;
    Drive[(3 * 256) + ZPILTabLoc + 1] = 256 - IL2;
    Drive[(3 * 256) + ZPILTabLoc + 2] = 256 - IL1;
    Drive[(3 * 256) + ZPILTabLoc + 3] = idcNextID % 256;
    Drive[(3 * 256) + ZPILTabLoc + 4] = 256 - IL0;

    CT = 18;
    CS = 11;

    for (int c = 0; c <= 5; c++)        //6 blocks to be saved: 18:11, 18:12, 18:13, 18:14, 18:15, (18:16 - block 5)
    {
        for (int i = 0; i < 256; i++)
        {
            Disk[Track[CT] + (CS * 256) + i] = Drive[(c * 256) + i];
        }
        DeleteBit(CT, CS);
        CS++;
    }

    //Add SideID
    Disk[BAM + 255] = EORtransform(idcSideID % 256);

    //Add Custom Interleave Info and Next Side ID
    Disk[BAM + 254] = EORtransform(256 - IL3);
    Disk[BAM + 253] = EORtransform(256 - IL2);
    Disk[BAM + 252] = EORtransform(256 - IL1);
    Disk[BAM + 251] = EORtransform(idcNextID % 256);
    Disk[BAM + 250] = EORtransform(256 - IL0);

    //Add IncludeSaveCode flag and Saver plugin if required
    BlocksUsedBySaver = 0;
    if (bSaverPlugin)
    {
        Disk[BAM + 249] = EORtransform(2);
        if(!InjectSaverPlugin())
            return false;
    }
    else
    {
        Disk[BAM + 249] = EORtransform(0);
    }

    //Also add Product ID to BAM, EOR-transformed
    Disk[BAM + 248] = EORtransform((ProductID / 0x10000) & 0xff);
    Disk[BAM + 247] = EORtransform((ProductID / 0x100) & 0xff);
    Disk[BAM + 246] = EORtransform(ProductID & 0xff);

    return true;

}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool UpdateZP() {

    if (LoaderZP.size() < 2)    //00 - need 2, 01 - need 1
    {
        string DefaultZP = "02";
        for (size_t i = 0; i < LoaderZP.size(); i++)
        {
            LoaderZP = DefaultZP[i] + LoaderZP;
        }
    }
    else if (LoaderZP.size() > 2)
    {
        string DefaultZP = LoaderZP;
        LoaderZP = "";
        for (size_t i = LoaderZP.size()-2; i < LoaderZP.size(); i++)
        {
            LoaderZP += DefaultZP[i];
        }
    }

    //Convert LoaderZP to byte
    unsigned char ZP = ConvertHexStringToInt(LoaderZP) % 256;

    //ZP cannot be $00, $01, or $ff
    if (ZP < 2)
    {
        cerr << "***CRITICAL***\tZeropage value cannot be less than $02.\n";
        return false;
    }
    else if (ZP > 0xfd)
    {
        cerr << "***CRITICAL***\tZeropage value cannot be greater than $fd.\n";
        return false;
    }

    //ZP=02 is the default, no need to update
    if (ZP == 2)
    {
        return true;
    }

    //UpdateZP BUG REPORTED BY Rico/Pretzel Logic

    //Find the JMP $0700 sequence in the code to identify the beginning of loader
    int LoaderBase = 0xffff;
    for (int i = 0; i < Loader_size - 1 - 2; i++)
    {
        if ((Loader[i] == 0x4c) && (Loader[i + 1] == 0x00) && (Loader[i + 2] == 0x07))
        {
            LoaderBase = i + 3;
            break;
        }
    }

    if (LoaderBase == 0xffff)
    {
        cerr << "***CRITICAL***\tZeropage offset could not updated.\n";
        return false;
    }

    unsigned char OPC_STAZP = 0x85;
    unsigned char OPC_ADCZP = 0x65;
    unsigned char OPC_STAZPY = 0x91;
    unsigned char OPC_DECZP = 0xC6;
    unsigned char OPC_LDAZP = 0xA5;
    unsigned char OPC_RORZP = 0x66;
    unsigned char OPC_ASLZP = 0x06;
    unsigned char OPC_EORIMM = 0x49;
    //unsigned char OPC_LDAZPY = 0xB1;

    unsigned char ZPBase = 0x02;

    for (int i = LoaderBase; i < Loader_size - 2; i++)
    {
        if ((Loader[i] == OPC_STAZP) || (Loader[i] == OPC_ADCZP) || (Loader[i] == OPC_STAZPY))
        {
            if ((Loader[i + 1] == ZPBase) && (Loader[i + 2] != OPC_EORIMM))   //Skip STA $0265 EOR #$FF
            {
                Loader[i + 1] = ZP;
                i++;
            }
        }
    }

    for (int i = LoaderBase; i < Loader_size - 1; i++)
    {
        if ((Loader[i] == OPC_STAZP) || (Loader[i] == OPC_DECZP) || (Loader[i] == OPC_LDAZP))
        {
            if (Loader[i + 1] == (ZPBase + 1))
            {
                Loader[i + 1] = ZP + 1;
                i++;
            }
        }
    }

    for (int i = LoaderBase; i < Loader_size - 1; i++)
    {
        if ((Loader[i] == OPC_STAZP) || (Loader[i] == OPC_RORZP) || (Loader[i] == OPC_ASLZP))
        {
            if (Loader[i + 1] == (ZPBase + 2))
            {
                Loader[i + 1] = ZP + 2;
                i++;
            }
        }
    }

    return true;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool InjectLoader(unsigned char T, unsigned char S, unsigned char IL) {

    int B{};
    unsigned char ST{}, SS{}, AdLo{}, AdHi{};


    if (DemoStart != "")                            //Check if we have a Demo Start Address
    {
        B = ConvertHexStringToInt(DemoStart);
    }
    else if (FirstFileStart != "")                  //No Demo Start Address, check if we have the first file's start address
    {
        B = ConvertHexStringToInt(FirstFileStart);
    }
    else
    {
        cerr << "***CRITICAL***\tStart address is missing!\n";
        return false;
    }

    AdLo = (B - 1) % 256;
    AdHi = (B - 1) / 256;

    //unsigned char* Loader = new unsigned char [Loader_size];

    //for (int i = 0; i < Loader_size; i++)
    //{
        //Loader[i] = Loader[i];
    //}


    if (!UpdateZP())
        return false;

    unsigned char LDA_IMM = 0xa9;
    unsigned char PHA = 0x48;

    for (int i = 0; i < Loader_size - 5; i++)      //Find STA $fffb instruction which is 14 bytes from AdLo
    {
        if ((Loader[i] == LDA_IMM) && (Loader[i + 2] == PHA) && (Loader[i + 3] == LDA_IMM) && (Loader[i + 5] == PHA))
        {
            Loader[i + 1] = AdHi;                  //Hi Byte return address at the end of Loader
            Loader[i + 4] = AdLo;                  //Lo Byte return address at the end of Loader
            break;
        }
    }

 /*
     for (int i = 0; i < Loader_size - 6; i++)   //Find JMP Sparkle_LoadFetched instruction
    {
        if ((Loader[i] == 0x10) && (Loader[i + 3] == 0xad) && (Loader[i + 5] == 0x4c))
        {
            Loader[i] = AdHi;                   //Hi Byte return address at the end of Loader
            Loader[i + 3] = AdLo;               //Lo Byte return address at the end of Loader
        }
    }
*/
    //Number of blocks in Loader
    LoaderBlockCount = Loader_size / 254;
    if (Loader_size % 254 != 0)
    {
        LoaderBlockCount += 1;
    }

    CT = T;
    CS = S;

    for (int i = 0; i < LoaderBlockCount; i++)
    {
        ST = CT;
        SS = CS;
        for (int c = 0; c < 254; c++)
        {
            if ((i * 254) + c < Loader_size)
            {
                Disk[Track[CT] + (CS * 256) + 2 + c] = Loader[(i * 254) + c];
            }
        }
        DeleteBit(CT, CS);

        AddInterleave(IL);      //Go to next free sector with Interleave IL

        if (i < LoaderBlockCount - 1)
        {
            Disk[Track[ST] + (SS * 256) + 0] = CT;
            Disk[Track[ST] + (SS * 256) + 1] = CS;
        }
        else
        {
            Disk[Track[ST] + (SS * 256) + 0] = 0;
            if (Loader_size % 254 == 0)
            {
                Disk[Track[ST] + (SS * 256) + 1] = 255;     //254+1
            }
            else
            {
                Disk[Track[ST] + (SS * 256) + 1] = ((Loader_size) % 254) + 1;
            }
        }
    }

    //delete[] Loader;

    return true;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

void CalcTabs() {

    int SMax{}, IL{};
    char Sectors[ExtSectorsPerDisk + 19]{};
    int Tr[41]{};       //0-40
    int S = 0;
    //int LastS = 0;
    int i = 0;

    for (int T = 1; T < ExtTracksPerDisk; T++)
    {
        if (T < 18)
        {
            Tr[T + 1] = Tr[T] + 21;
            Track[T + 1] = Track[T] + (21 * 256);
        }
        else if (T < 25)
        {
            Tr[T + 1] = Tr[T] + 19;
            Track[T + 1] = Track[T] + (19 * 256);
        }
        else if (T < 31)
        {
            Tr[T + 1] = Tr[T] + 18;
            Track[T + 1] = Track[T] + (18 * 256);
        }
        else
        {
            Tr[T + 1] = Tr[T] + 17;
            Track[T + 1] = Track[T] + (17 * 256);
        }
    }

    for (int T = 1; T <= ExtTracksPerDisk; T++)
    {
        if (T == 18)
        {
            TabStartS[T] = -1;
            T++;
            S += 2;
        }

        int SCnt = 0;

        if (T < 18)
        {
            SMax = 21;
            IL = IL0 % SMax;
        }
        else if (T < 25)
        {
            SMax = 19;
            IL = IL1 % SMax;
        }
        else if (T < 31)
        {
            SMax = 18;
            IL = IL2 % SMax;
        }
        else
        {
            SMax = 17;
            IL = IL3 % SMax;
        }

        if (S >= SMax)
        {
            S -= SMax;
            if ((T < 18) && (S > 0))    //Wrap around: Subtract 1 if S>0 for tracks 1-17
                S--;
        }
        TabStartS[T] = S;

        while (SCnt < SMax)
        {
        //NextSector:
            if (Sectors[Tr[T] + S] == 0)
            {
                Sectors[Tr[T] + S] = 1;
                TabT[i] = T;
                TabS[i] = S;
                //LastS = S;
                TabSCnt[i] = SMax - SCnt;
                i++;
                SCnt++;
                S += IL;

                if (S >= SMax)
                {
                    S -= SMax;
                    if ((T < 18) && (S > 0))    //Wrap around: Subtract 1 if S>0 for tracks 1-17
                        S--;
                }
            }
            else
            {
                S++;
                if (S >= SMax)
                    S = 0;
            }
        }
    }

    //WriteBinaryFile("C:\\Tmp\\TabT.bin", TabT, sizeof(TabT));
    //WriteBinaryFile("C:\\Tmp\\TabS.bin", TabS, sizeof(TabS));
    //WriteBinaryFile("C:\\Tmp\\TabSCnt.bin", TabSCnt, sizeof(TabSCnt));
    //WriteBinaryFile("C:\\Tmp\\TabStartS.bin", TabStartS, sizeof(TabStartS));

}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool AddCompressedBundlesToDisk() {

    if (BlocksFree < BufferCnt)
    {
        cerr << "***CRITICAL***\t" << D64Name << " cannot be built because it would require " << BufferCnt << " blocks.\n"
             << "This disk only has " << SectorsPerDisk << " blocks.";
        return false;
    }

    //CalcTabs();

    InjectDirBlocks();

    for (int i = 0; i < BufferCnt; i++)
    {
        CT = TabT[i];
        CS = TabS[i];
        for (int j = 0; j < 256; j++)
        {
            Disk[Track[CT] + (256 * CS) + j] = ByteSt[(i * 256) + j];
        }

        DeleteBit(CT, CS);
    }

    if (BufferCnt < SectorsPerDisk)
    {
        NextTrack = TabT[BufferCnt];
        NextSector = TabS[BufferCnt];
    }
    else
    {
        NextTrack = 18;
        NextSector = 0;
    }

    return true;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

void UpdateBlocksFree() {

    if (TracksPerDisk == ExtTracksPerDisk)
    {
        unsigned char ExtBlocksFree = (BlocksFree > ExtSectorsPerDisk - StdSectorsPerDisk) ? ExtSectorsPerDisk - StdSectorsPerDisk - BlocksUsedBySaver : BlocksFree;
        Disk[Track[18] + 4] += ExtBlocksFree;
    }

}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool FinishDisk(bool LastDisk) {

    if ((BundleCnt == 0) && (FileCnt == -1))
    {
        cerr << "***CRITICAL***\tThis disk does not contain any files!\n";
        return false;
    }

    if (!BundleDone())
        return false;
    if (!CompressBundle())
        return false;
    if (!CloseBundle(0, true))
        return false;
    if (!CloseBuffer())
        return false;

    if (MaxBundleNoExceeded)
    {
        cerr << "***INFO***\tThe number of file bundles is greater than 128 on this disk!\n"
             << "You can only access bundles 0-127 by bundle index. The rest can only be loaded using the LoadNext function.";
    }

    //Now add compressed parts to disk
    if (!AddCompressedBundlesToDisk())                      //Also adds two dir sectors (18:17 and 18:18)
        return false;

    if (!InjectLoader(18,7,1))                      //If InjectLoader(-1, 18, 7, 1) = False Then GoTo NoDisk
        return false;

    if (ThisID == 255)
    {
        ThisID = DiskCnt;
    }

    if (NextID == 255)
    {
        NextID = LastDisk ? 0xff : DiskCnt + 1;            //Negative value means no more disk, otherwise 0x00-0x7f
    }
    if (!InjectDriveCode(ThisID,LoaderBundles,NextID))
        return false;

    AddHeaderAndID();
    AddDemoNameToDisk(18,7);
    AddDirArt();

    UpdateBlocksFree();

    int BlocksUsed = ((TracksPerDisk == StdTracksPerDisk) ? (StdSectorsPerDisk - BlocksFree) : (ExtSectorsPerDisk - BlocksFree));
    double CompRatio = TotalCompSize * 100;
    CompRatio /= TotalOrigSize;

    cout << "\nFinal disk: " << TotalOrigSize << " block" << (TotalOrigSize == 1 ? "" : "s") << " compressed to " << BlocksUsed << " block" << (BlocksUsed == 1 ? ", " : "s, ")
        << BlocksFree << " block" << (BlocksFree == 1 ? "" : "s") << " remaining free. Overall compression ratio : " << setprecision(2) << fixed << CompRatio << " %\n\n";

    if (!WriteDiskImage(D64Name))
    {
        return false;
    }

    return true;

}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool Build() {
    /* initialize random seed: */
    srand(time(NULL));

    /* generate a random number between 0 and 0Xffffff: */
    ProductID = (((rand() & 0xff0) << 16) + (rand() & 0xffff)) & 0xffffff;

    LineStart = 0;
    LineEnd = 0;

    FindNextScriptEntry();

    if (ScriptEntryType != EntryTypeScriptHeader)
    {
        cerr << "***CRITICAL***\tInvalid script file!\n";
        return false;
    }
    if (!ResetDiskVariables())
    {
        return false;
    }
    bool NewD = true;
    NewBundle = false;
    TmpSetNewBlock = false;

    while (LineEnd <= Script.length() - 1) {

        if (FindNextScriptEntry())
        {
            //SplitScriptEntry();

            if (ScriptEntryType == EntryTypePath)
            {
                if (!NewD)
                {
                    NewD = true;
                    if ((!FinishDisk(false)) || (!ResetDiskVariables()))
                        return false;
                }

                if (NumScriptEntries > -1)
                {
                    D64Name = ScriptEntryArray[0];
                    if (D64Name.find(":") == string::npos)
                    {
                        D64Name = ScriptPath + D64Name;                      //sPath is relative - use Sparkle's base folder to make it a full path
                    }

                }

                NewBundle = true;
            }
            else if (ScriptEntryType == EntryTypeThisID)
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
                        ThisID = ConvertHexStringToInt(ScriptEntryArray[0]) % 256;
                        if (ThisID > 127)
                        {
                            cerr << "***CRITICAL***\tThis Side Index must be a hex number and cannot be greater than $7f!\n";
                            return false;
                        }
                    }
                    else
                    {
                        cerr << "***CRITICAL***\tThis Side Index must be a hex number and cannot be greater than $7f!\n";
                        return false;
                    }
                }

                NewBundle = true;
            }
            else if (ScriptEntryType == EntryTypeNextID)
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
                        NextID = ConvertHexStringToInt(ScriptEntryArray[0]) % 256;
                    }
                    else
                    {
                        cerr << "***CRITICAL***\tNext Side Index must be a hex number!\n";
                        return false;
                    }
                }

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
                    cerr << "***INFO***\tThe following DirArt file does not exist: " << ScriptEntryArray[0] << "\nThe disk will be built without DirArt.\n";
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
                        cerr << "***INFO***\tThe Product ID must be a maximum 6-digit long hexadecimal number!\nSparkle will use the following pseudorandom Product ID: " << hex << ProductID <<"\n";
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
                if (!AddFile())
                    return false;
                if (NewD)
                {
                    CalcTabs();
                    NewD = false;       //We have added at least one file to this disk, so next disk info entry will be a new disk
                }
                NewBundle = false;
            }
            else if (ScriptEntryType == EntryTypeMem)
            {

                if(!AddVirtualFile())
                  return false;

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
        //else
        //{
        //    break;
        //}
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
        if ((sPath[i] != '\\') && (sPath[i] != '/'))
        {
            ScriptPath.replace(i, 1, "");
        }
        else
        {
            break;
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------
void PrintInfo()
{
    cout << "USAGE: Sparkle script.sls\n\n";

    /*        cout << "IMPORTANT LOADER FUNCTIONS:\n\n";

            DWORD size = 0;
            const char* data = NULL;
            LoadFileInResource(IDR_INCFILE1, TEXTFILE, size, data);
            // Access bytes in data - here's a simple example involving text output
            // The text stored in the resource might not be NULL terminated.
            char* buffer = new char[size + 1];
            ::memcpy(buffer, data, size);
            buffer[size] = 0; // NULL terminator
            cout << buffer << "\n\n";

            delete[] buffer;
    */

    cout << "SCRIPT TEMPLATE:\n\n";
    cout << "[Sparkle Loader Script]\n\n";
    cout << "Path:\t\tfilepath\\diskname.d64\t\t\t\t\t# path and file name of the D64 image <- THIS IS A COMMENT\n";
    cout << "Header:\t\tdisk header\t\t\t\t\t\t# max. 16 characters\n";
    cout << "ID:\t\tdskid\t\t\t\t\t\t\t# max. 5 characters\n";
    cout << "Name:\t\tdemo name\t\t\t\t\t\t# max. 16 characters\n";
    cout << "Start:\t\tabcd\t\t\t\t\t\t\t# first part's start address\n";
    cout << "DirArt:\t\tfilepath\\diarartfile.d64\t\t\t\t# D64, TXT, PRG, BIN, C array, KickAss ASM formats accepted\n";
    cout << "Tracks:\t\t35\t\t\t\t\t\t\t# 35 vs 40, default: 35 if omitted\n";
    cout << "HSFile:\t\tfilepath\\hsfilename.bin\tabcd\tabcdabcd\tabcd\t# (address) (offset) (length) can be omitted if not needed\n";
    cout << "ProdID:\t\tabcdef\t\t\t\t\t\t\t# 6-digit hex number, once per script, must be the same for disks of a multidisk prod, autogenerated if omitted\n";
    cout << "ThisSide:\tab\t\t\t\t\t\t\t# 2-digit hex number, only if disks of a multidisk prod are built separately\n";
    cout << "NextSide:\tab\t\t\t\t\t\t\t# 2-digit hex number, only if disks of a multidisk prod are built separately\n";
    cout << "IL0:\t\t4\t\t\t\t\t\t\t# 1-20, default: 4 if omitted\n";
    cout << "IL1:\t\t3\t\t\t\t\t\t\t# 1-18, default: 3 if omitted\n";
    cout << "IL2:\t\t3\t\t\t\t\t\t\t# 1-17, default: 3 if omitted\n";
    cout << "IL3:\t\t3\t\t\t\t\t\t\t# 1-16, default: 3 if omitted\n";
    cout << "ZP:\t\t02\t\t\t\t\t\t\t# 02-fd, once per script, must be the same for disks of a multidisk prod, default: 02 if omitted\n\n";
    cout << "# Bundle 0 - files marked with * will be loaded under I/O ($d000-$dfff)\n";
    cout << "File:\t\tfilepath\\file1.prg*\tabcd\tabcdabcd\tabcd\t# (address) (offset) (length)\n";
    cout << "File:\t\tfilepath\\file2.bin\tabcd\tabcdabcd\t\t# (address) (offset) (default length)\n";
    cout << "File:\t\tfilepath\\file3.kla\tabcd\t\t\t\t# (address) (default offset) (default length)\n\n";
    cout << "# Bundle 1 - bundles must be separated by at least one blank line!\n";
    cout << "File:\t\tfilepath\\file4.prg\t\t\t\t\t# (default address) (default offset) (default length)\n\n";
    cout << "# Bundle 2\n";
    cout << "Script:\t\tfilepath\\scriptfile.sls\t\t\t\t\t# this will import another script here...\n\n";
    cout << "# Entries can be fully omitted if they are not needed or the default value is used.\n";
    cout << "# Use TAB(s) to separate entry types and their values!\n";
    cout << "# Unrecognized entries are interpreted as comments.\n";
    cout << "# For more details please read the user manual!\n\n";

}
//----------------------------------------------------------------------------------------------------------------------------------------------------------
/*
void LoadFileInResource(int name, int type, DWORD& size, const char*& data)
{
    HMODULE handle = ::GetModuleHandle(NULL);
    HRSRC rc = ::FindResource(handle, MAKEINTRESOURCE(name),
        MAKEINTRESOURCE(type));
    HGLOBAL rcData = ::LoadResource(handle, rc);
    size = ::SizeofResource(handle, rc);
    data = static_cast<const char*>(::LockResource(rcData));
}
*/
int main(int argc, char* argv[])
{
    auto cstart = std::chrono::system_clock::now();

    cout << "\n**************************************\n";
    cout << "Sparkle " << VersionMajor <<"." << VersionMinor << "." << hex << VersionBuild << dec << " by Sparta 2019-2022\n";
    cout << "**************************************\n\n";

    string AppPath{filesystem::current_path().string() + "\\"};

    if (argc < 2)
    {
#ifdef DEBUG

        //string ScriptFileName = "c:\\Users\\Tamas\\OneDrive\\C64\\Coding\\ThePumpkins\\Backup\\221028\\Scripts\\ThePumpkins.sls";
        string ScriptFileName = "c:\\Users\\Tamas\\source\\repos\\GPMagazine\\Magazine\\Issue33\\Sparkle\\Magazine.sls";
        Script = ReadFileToString(ScriptFileName);
        SetScriptPath(ScriptFileName, AppPath);

#else

        PrintInfo();
        return 1;

#endif // DEBUG

    }
    else
    {
        string ScriptFileName(argv[1]);
        Script = ReadFileToString(ScriptFileName);

        SetScriptPath(ScriptFileName, AppPath);
    }

    if (Script == "")
    {
        cerr <<"***CRITICAL***\tUnable to load script file or the file is empty!\n";
        return -1;
    }

    CalcTabs();     //IS THIS NEEDED HERE???

    if (!Build())
        return -1;

    auto cend = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = cend - cstart;

    std::cout << "Elapsed time: " << elapsed_seconds.count() << "s\n\n";

    return 0;
}
