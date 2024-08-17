#include "common.h"

//#define TESTDISK

//#define DEBUG

//#define NEWIO

//--------------------------------------------------------
//  COMPILE TIME VARIABLES FOR BUILD INFO 240817
//--------------------------------------------------------

constexpr unsigned int FullYear = ((__DATE__[7] - '0') * 1000) + ((__DATE__[8] - '0') * 100) + ((__DATE__[9] - '0') * 10) + (__DATE__[10] - '0');

constexpr unsigned int Year = ((__DATE__[9] - '0') * 10) + (__DATE__[10] - '0');

constexpr unsigned int Month = (__DATE__[0] == 'J') ? ((__DATE__[1] == 'a') ? 1 : ((__DATE__[2] == 'n') ? 6 : 7))    // Jan, Jun or Jul
                             : (__DATE__[0] == 'F') ? 2                                                              // Feb
                             : (__DATE__[0] == 'M') ? ((__DATE__[2] == 'r') ? 3 : 5)                                 // Mar or May
                             : (__DATE__[0] == 'A') ? ((__DATE__[1] == 'p') ? 4 : 8)                                 // Apr or Aug
                             : (__DATE__[0] == 'S') ? 9                                                              // Sep
                             : (__DATE__[0] == 'O') ? 10                                                             // Oct
                             : (__DATE__[0] == 'N') ? 11                                                             // Nov
                             : (__DATE__[0] == 'D') ? 12                                                             // Dec
                             : 0;
constexpr unsigned int Day = (__DATE__[4] == ' ') ? (__DATE__[5] - '0') : (__DATE__[4] - '0') * 10 + (__DATE__[5] - '0');

constexpr unsigned int VersionBuild = ((Year / 10) * 0x100000) + ((Year % 10) * 0x10000) + ((Month / 10) * 0x1000) + ((Month % 10) * 0x100) + ((Day / 10) * 0x10) + (Day % 10);

constexpr int VersionMajor = 3;
constexpr int VersionMinor = 1;

string Script = "";
string ScriptPath = "";
string ScriptName = "";
string ScriptEntry = "";
string ScriptLine = "";
string ScriptEntryType = "";
const int MaxNumEntries = 5;                                                //file name + 3 parameters + comment
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

const string EntryTypeScriptHeader = "[sparkle loader script]";
const string EntryTypePath = "path:";
const string EntryTypeHeader = "header:";
const string EntryTypeID = "id:";
const string EntryTypeThisID = "thisside:";
const string EntryTypeNextID = "nextside:";
const string EntryTypeName = "name:";
const string EntryTypeStart = "start:";
const string EntryTypeDirArt = "dirart:";
const string EntryTypeZP = "zp:";
const string EntryTypeIL0 = "il0:";
const string EntryTypeIL1 = "il1:";
const string EntryTypeIL2 = "il2:";
const string EntryTypeIL3 = "il3:";
const string EntryTypeProdID = "prodid:";
const string EntryTypeTracks = "tracks:";
const string EntryTypeHSFile = "hsfile:";
const string EntryTypeScript = "script:";
const string EntryTypeFile = "file:";
const string EntryTypeMem = "mem:";
const string EntryTypeAlign = "align";
#ifdef TESTDISK
    const string EntryTypeTestDisk = "testdisk";
#endif // TESTDISK
const string EntryTypeDirIndex = "dirindex:";
const string EntryTypeComment = "comment:";

int ProductID = 0;
size_t LineStart, LineEnd;

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

unsigned char AltDirBitPtr[128]{};
int AltDirBundleNo[128]{};
int TmpDirEntryIndex = 0;
int DirEntryIndex = 0;
bool DirEntriesUsed = false;

//Hi-Score File variables
string HSFileName = "";
vector<unsigned char> HSFile;
size_t HSAddress = 0;
size_t HSOffset = 0;
size_t HSLength = 0;
bool bSaverPlugin = false;

#ifdef TESTDISK
    bool bTestDisk = false;
#endif

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

size_t DirTrack, DirSector, DirPos;
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
int BlocksUsedByPlugin = 0;

//VARIABLES ALSO USED BY PACKER

int BytePtr;
int BitPtr;

bool LastFileOfBundle;

int PartialFileIndex;
size_t PartialFileOffset;

int PrgAdd, PrgLen;
bool FileUnderIO = false;

#ifdef NEWIO
bool BundleUnderIO = false;
#endif

bool SaverSupportsIO = false;

int TotalOrigSize{}, TotalCompSize{};

unsigned char LoaderBlockCount = 0;

string ParameterString = "";
te_parser tep;
bool bEntryHasExpression = false;
string ParsedEntries = "";

string HomeDir = "";

string DirArtType = "";

unsigned int ImgWidth = 0;
unsigned int ImgHeight = 0;
unsigned int BGCol = 0;
unsigned int FGCol = 0;

vector <unsigned char> Image;   //pixels in RGBA format (4 bytes per pixel)
vector <unsigned char> ImgRaw;  //raw image

int Mplr = 0;

typedef struct tagBITMAPINFOHEADER {
    int32_t biSize;
    int32_t biWidth;
    int32_t biHeight;
    int16_t biPlanes;
    int16_t biBitCount;
    int32_t biCompression;
    int32_t biSizeImage;
    int32_t biXPelsPerMeter;
    int32_t biYPelsPerMeter;
    int32_t biClrUsed;
    int32_t biClrImportant;
} BITMAPINFOHEADER, * LPBITMAPINFOHEADER, * PBITMAPINFOHEADER;

typedef struct tagRGBQUAD {
    unsigned char rgbBlue;
    unsigned char rgbGreen;
    unsigned char rgbRed;
    unsigned char rgbReserved;
} RGBQUAD;

typedef struct tagBITMAPINFO {
    BITMAPINFOHEADER bmiHeader;
    RGBQUAD          bmiColors[1];
} BITMAPINFO, * LPBITMAPINFO, * PBITMAPINFO;

BITMAPINFO BmpInfo;
BITMAPINFOHEADER BmpInfoHeader;

//------------------------------------------------------------------------------------------------------------------------------------------------------------

string FindAbsolutePath(string FilePath, string ScriptFilePath)
{
#if _WIN32
    if ((FilePath.size() < 3) || ((FilePath.substr(1, 2) != ":\\") && (FilePath.substr(1, 2) != ":/")))
    {
        //FilePath is relative - make it a full path
        FilePath = ScriptFilePath + FilePath;
    }
#elif __APPLE__ || __linux__
    if (FilePath[0] != '/')
    {
        if ((FilePath.size() > 1) && (FilePath[0] == '~') && (FilePath[1] == '/'))
        {
            //Home directory (~/...) -> replace "~" with HomeDir if known
            if (!HomeDir.empty())
            {
                FilePath = HomeDir + FilePath.substr(1);
            }
            else
            {
                cerr << "***INFO***\tUnable to identify the user's Home directory...\n";
            }
        }
        else
        {
            //FilePath is relative - make it a full path
            FilePath = ScriptFilePath + FilePath;
        }
    }
#endif

    return FilePath;

}
//------------------------------------------------------------------------------------------------------------------------------------------------------------

string ConvertIntToHextString(const int i, const int hexlen)
{
    std::stringstream hexstream;
    hexstream << setfill('0') << setw(hexlen) << hex << i;
    //cout << dec << i << "\t\t" << hex << hexstream.str() << "\n";
    return hexstream.str();

    /*
    char hexchar[8]{};
    sprintf_s(hexchar, "%X", i);
    string hexstring = "";
    for (int j = 0; j < hexlen; j++)
    {
        if (hexchar[j] != 0)
        {
            hexstring += tolower(hexchar[j]); //+hexstring;
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
    */
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

string ConvertHexStringToDecimalString(const string& s)
{
    return to_string(ConvertHexStringToInt(s));
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
/*
inline bool FileExists(const string& FileName)
{
    struct stat buffer;
    return (stat(FileName.c_str(), &buffer) == 0);
}
*/
//----------------------------------------------------------------------------------------------------------------------------------------------------------

int ReadBinaryFile(const string& FileName, vector<unsigned char>& prg)
{

    if (!fs::exists(FileName))
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

    infile.close();

    return length;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

string ReadFileToString(const string& FileName, bool CorrectFilePath)
{

    if (!fs::exists(FileName))
    {
        return "";
    }

    ifstream infile(FileName);

    if (infile.fail())
    {
        return "";
    }

    string str;

    infile.seekg(0, ios::end);
    str.reserve(infile.tellg());
    infile.seekg(0, ios::beg);

    str.assign((istreambuf_iterator<char>(infile)), istreambuf_iterator<char>());

    for (int i = str.length() - 1; i >= 0; i--)         //Done from end to start as the length of the string may change
    {
        if (str[i] == '\r')
        {
            str.replace(i, 1, "");      //Windows does this automatically, remove '\r' (0x0d) chars if string contains any
        }
        else if ((str[i] == '\\') && (CorrectFilePath))
        {
            str.replace(i, 1, "/");     //Replace '\' with '/' in file paths, Windows can also handle this
        }
    }

    infile.close();

    return str;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------
/*
void WriteTextToFile(const string& DiskName)
{
    ofstream out(DiskName);
    out << Script;
    out.close();
}
*/
//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool CreateDirectory(const string& DiskDir)
{
    if (!fs::exists(DiskDir))
    {
        cout << "Creating folder: " << DiskDir << "\n";
        fs::create_directories(DiskDir);
    }

    if (!fs::exists(DiskDir))
    {
        cerr << "***CRITICAL***\tUnable to create the following folder: " << DiskDir << "\n";
        return false;
    }
    return true;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool WriteDiskImage(const string& DiskName)
{

    string DiskDir{};

    for (size_t i = 0; i <= DiskName.length() - 1; i++)
    {
#if _WIN32 
        if ((DiskName[i] == '\\') || (DiskName[i] == '/'))
        {
            if (DiskDir[DiskDir.length() - 1] != ':')   //Don't try to create root directory
            {
                if (!CreateDirectory(DiskDir))
                    return false;
            }
        }
#elif __APPLE__ || __linux__
        if ((DiskName[i] == '/') && (DiskDir.size() > 0) && (DiskDir != "~"))   //Don't try to create root directory and home directory
        {
            if (!CreateDirectory(DiskDir))
                return false;
        }
#endif
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

        if (!myFile.good())
        {
            cerr << "***CRITICAL***\tError during writing " << DiskName << "\n";
            myFile.close();
            return false;
        }

        myFile.close();
        return true;
    }
    else
    {
        cerr << "***CRITICAL***\tError opening file for writing disk image: " << DiskName << "\n";
        return false;
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------
/*
void WriteBinaryFile(const string& FileName, unsigned char* Buffer, streamsize Size)
{
    ofstream myFile(FileName, ios::out | ios::binary);
    myFile.write((char*)&Buffer[0], Size);
}
*/
//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool SectorOK(unsigned char T, unsigned char S)
{
    int BAMPos = Track[18] + (T * 4) + 1 + (S / 8) + ((T > 35) ? (7 * 4) : 0);
    int BAMBit = 1 << (S % 8);

    return (Disk[BAMPos] & BAMBit) != 0;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool FindNextFreeSector()
{
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
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool TrackIsFull(unsigned char T)
{
    return (Disk[Track[18] + (T * 4) + ((T > 35) ? (7 * 4) : 0)] == 0);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

void CalcNextSector(unsigned char IL)
{
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

bool AddInterleave(unsigned char IL)
{
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

unsigned char EORtransform(unsigned char Input)
{
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

void InjectDirBlocks()
{
    //DirBlocks(0) = EORtransform(Track), it is used by the drive code, current track
    //DirBlocks(1) = EORtransform(StartSector), it is used by the drive code, the first sector on the track after track change
    //DirBlocks(2) = EORtransform(Remaining sectors on track), it is used by the drive code, remaining sectors on the track
    //DirBlocks(3) = BitPtr, NOT EOR transformed, it is used on the C64 side

    if (DirEntriesUsed)
    {
        //Create first DirEntry for Bundle #0
        DirBlocks[0] = EORtransform(TabT[0]);
        DirBlocks[1] = EORtransform(TabStartS[TabT[0]]);
        DirBlocks[2] = EORtransform(TabSCnt[0]);
        DirBlocks[3] = 0;
        
        //Fill the whole directory with Bundle #0's values
        for (int i = 4; i < 512; i++)
        {
            DirBlocks[i] = DirBlocks[i - 4];
        }

        //Overwrite directory entries if an alternative DirEntry index was assigned
        for (int i = 1; i < 128; i++)
        {
            if (AltDirBundleNo[i] != 0)
            {
                int BN = AltDirBundleNo[i];
                int BP = AltDirBitPtr[i];
                DirBlocks[i * 4] = EORtransform(TabT[BN]);
                DirBlocks[(i * 4) + 1] = EORtransform(TabStartS[TabT[BN]]);
                DirBlocks[(i * 4) + 2] = EORtransform(TabSCnt[BN]);
                DirBlocks[(i * 4) + 3] = BP;
            }
        }
    }
    else
    {
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
/*
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
*/
//----------------------------------------------------------------------------------------------------------------------------------------------------------

string CreateExpressionString(int p)
{

    //Remove unwanted spaces
    while (ScriptEntryArray[p].find(' ') != string::npos)
    {
        int Pos = ScriptEntryArray[p].find(' ');
        ScriptEntryArray[p].replace(Pos, 1, "");
    }

    string Expr = "";
    bool IsDecimal = false;
    string HexString = "";
    size_t i = 0;

    while (i < ScriptEntryArray[p].size())
    {
        char NextChar = tolower(ScriptEntryArray[p].at(i++));

        if (NextChar == '.')        //Expect decimal number
        {
            IsDecimal = true;
        }
        else if (NextChar == '$')   //Identify '$' hex prefix
        {
            IsDecimal = false;
        }
        else if ((NextChar == '&') && (i < ScriptEntryArray[p].size()))         //Identify '&H' hex prefix
        {
            if (tolower(ScriptEntryArray[p].at(i)) == 'h')
            {
                i++;
                IsDecimal = false;
            }
        }
        else if ((NextChar == '0') && (i < ScriptEntryArray[p].size()))         //Identify '0x' hex prefix
        {
            if (tolower(ScriptEntryArray[p].at(i)) == 'x')
            {
                i++;
                IsDecimal = false;
            }
            else
            {
                if (IsDecimal)
                {
                    Expr += NextChar;
                }
                else
                {
                    HexString += NextChar;
                }
            }
        }
        else if ((NextChar >= 'a') && (NextChar <= 'f'))
        {
            IsDecimal = false;
            HexString += NextChar;
        }
        else if ((NextChar >= '0') && (NextChar <= '9'))
        {
            if (IsDecimal)
            {
                Expr += NextChar;
            }
            else
            {
                HexString += NextChar;
            }
        }
        else
        {
            Expr += (!HexString.empty() ? ConvertHexStringToDecimalString(HexString) : "") + NextChar;
            IsDecimal = false;
            HexString = "";
        }
    }

    Expr += (!HexString.empty() ? ConvertHexStringToDecimalString(HexString) : "");

    return Expr;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool EvaluateParameterExpression()
{
    for (int i = 1; i <= NumScriptEntries; i++)
    {
        if (ScriptEntryArray[i].length() > 0)
        {
            if (ScriptEntryArray[i].at(0) == '=')
            {
                string Expr = CreateExpressionString(i);

                //cout << "Expression: " << Expr << "\n";

                if (!tep.compile(Expr))
                {
                    cerr << "***CRITICAL***\tBundle #" << BundleCnt << " File #" << (FileCnt + 1) << "\tError the following file parameter expression:\t'" << ScriptEntryArray[i] << "'\n";
                    //cout << "\t\t\t\t\t\t\t\t      Error near here:\t  " << setfill(' ') << setw(tep.get_last_error_position()) << "^\n";
                    string EM = tep.get_last_error_message();
                    if (!EM.empty())
                    {
                        cerr << "Error message:\t" << EM << "\n";
                    }
                    return false;
                }

                double r = tep.evaluate(Expr);

                if (isnan(r))
                {
                    cerr << "***CRITICAL***\tBundle #" << BundleCnt << " File #" << (FileCnt + 1) << "\tError the following file parameter expression:\t'" << ScriptEntryArray[i] << "'\n";
                    //cout << "\t\t\t\t\t\t\t\t      Error near here:\t  " << setfill(' ') << setw(tep.get_last_error_position()) << "^\n";
                    string EM = tep.get_last_error_message();
                    if (!EM.empty())
                    {
                        cerr << "Error message:\t" << EM << "\n";
                    }
                    return false;
                }
                else if (r < 0)
                {
                    cerr << "***CRITICAL***\tBundle #" << BundleCnt << " File #" << (FileCnt + 1) << "\tError the following file parameter expression:\t'" << ScriptEntryArray[i] << "'\n";
                    //cout << "\t\t\t\t\t\t\t\t      Error near here:\t  " << setfill(' ') << setw(tep.get_last_error_position()) << "^\n";
                    cerr << "Error message:\tResult cannot be a negative number.\n";
                    return false;
                }

                unsigned long long Result = (unsigned long long)r;

                if ((i == 2) && (Result > 0xffff))
                {
                    if (Result > 0xffffffff)
                    {
                        cerr << "***CRITICAL***\t Bundle #" << BundleCnt << " File #" << (FileCnt + 1) << "\tError in the following file parameter expression: '" << ScriptEntryArray[i] << "'\n";
                        return false;
                    }
                    else
                    {
                        ParameterString = ConvertIntToHextString((int)Result, 8);
                    }
                }
                else
                {
                    if (Result > 0xffff)
                    {
                        cerr << "***CRITICAL***\t Bundle #" << BundleCnt << " File #" << (FileCnt + 1) << "\tError in the following file parameter expression: '" << ScriptEntryArray[i] << "'\n";
                        return false;
                    }
                    else
                    {
                        ParameterString = ConvertIntToHextString(Result, 4);
                    }
                }

                //cout << "Result: " << ParameterString << "\n";

                if (!ParameterString.empty())
                {
                    ScriptEntryArray[i] = ParameterString;
                    bEntryHasExpression = true;
                }
                else
                {
                    cerr << "***CRITICAL***\t Bundle #" << BundleCnt << " File #" << (FileCnt + 1) << "\tError in the following file parameter expression: '" << ScriptEntryArray[i] << "'\n";
                    return false;
                }
            }

        }
    }

    return true;
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
    if ((i == 1) && (ScriptEntryArray[i] == "-"))
    {
        return;
    }
    
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
    string FN = FindAbsolutePath(ScriptEntryArray[0], ScriptPath);
    string FA = "";
    string FO = "";
    string FL = "";
    size_t FAN = 0;
    size_t FON = 0;
    size_t FLN = 0;
    bool FUIO = false;

    int NumParams = 1;

    bEntryHasExpression = false;

    if (!EvaluateParameterExpression())
    {
        return false;
    }
    if (bEntryHasExpression)
    {
        ParsedEntries += "Hi-score File:\t";
        for (int i = 0; i <= NumScriptEntries; i++)
        {
            ParsedEntries += "\t" + ScriptEntryArray[i];
        }
        ParsedEntries += "\n";
    }

    for (int i = 1; i <= NumScriptEntries; i++)
    {
        if (ParameterIsNumeric(i))
        {
            NumParams++;
        }
        else
        {
            if ((i == 1) && (ScriptEntryArray[i] == "-"))
            {
                NumParams++;
            }
            else
            {
                break;
            }
        }
        CorrectParameterStringLength(i);
    }

    //Get file variables from script, or get default values if there were none in the script entry
    if (FN.find("*") == FN.length()-1)
    {
        FUIO = true;
        FN.replace(FN.length() - 1, 1, "");
    }

    if (fs::exists(FN))
    {
        HSFile.clear();
        if (ReadBinaryFile(FN, HSFile) == -1)
        {
            cerr << "***CRITICAL***\tUnable to open Hi-score File\n";
            return false;
        }
        else if (HSFile.size() == 0)
        {
            cerr << "***CRITICAL***\tHi-score File cannot be 0 bytes long\n";
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
            if (FA == "-")
            {
                //PRG file - load the whole file less the first two address bytes
                FA = ConvertIntToHextString(HSFile[0] + (HSFile[1] * 256), 4);
                FO = "00000002";
                FL = ConvertIntToHextString(HSFile.size() - 2, 4);
            }
            else
            {
                FO = "00000000";                                            //Offset will be 0, length=prg length
                FL = ConvertIntToHextString(HSFile.size(), 4);
            }
            break;
        case 3:  //Two parameters in script          
            FA = ScriptEntryArray[1];                                   //Load address from script
            if (FA == "-")
            {
                //PRG file
                FAN = (size_t)(HSFile[0] + HSFile[1] * 256);
                FA = ConvertIntToHextString(FAN, 4);
                FO = ScriptEntryArray[2];
                int iFON = ConvertHexStringToInt(FO) + 2 - (int)FAN;
                if ((iFON < 0) || ((size_t)iFON > HSFile.size() - 1))
                {
                    cerr << "***CRITICAL***\tInvalid memory segment start parameter in the Hi-Score File: " << ScriptEntryType << "\t" << ScriptEntry << "\n";
                    return false;
                }
                FO = ConvertIntToHextString(iFON, 4);
                FL = ConvertIntToHextString(HSFile.size() - iFON, 4);             //Length=prg length-offset
            }
            else
            {
                FO = ScriptEntryArray[2];                                   //Offset from script
                FON = ConvertHexStringToInt(FO);                            //Make sure offset is valid
                if (FON > HSFile.size() - 1)
                {
                    cerr << "***CRITICAL***\tInvalid file offset in the following Hi-Score File entry: " << ScriptEntryType << "\t" << ScriptEntry << "\n";
                    return false;
                }
                FL = ConvertIntToHextString(HSFile.size() - FON, 4);             //Length=prg length-offset
            }
            break;
        case 4:  //Three parameters in script
            FA = ScriptEntryArray[1];                                   //Load address from script
            FO = ScriptEntryArray[2];                                   //Offset from script
            FL = ScriptEntryArray[3];                                   //Length from script
            if (FA == "-")
            {
                //PRG file
                FAN = (size_t)(HSFile[0] + HSFile[1] * 256);
                FA = ConvertIntToHextString(FAN, 4);
                int iFLN = ConvertHexStringToInt(FL) - ConvertHexStringToInt(FO) + 1;
                FL = ConvertIntToHextString(iFLN, 4);
                int iFON = ConvertHexStringToInt(FO) + 2 - (int)FAN;
                if ((iFON < 0) || ((size_t)iFON > HSFile.size() - 1))
                {
                    cerr << "***CRITICAL***\tInvalid memory segment start parameter in the following File entry: " << ScriptEntry << "\n";
                    return false;
                }
                FO = ConvertIntToHextString(iFON, 8);
            }
            else
            {
                FON = ConvertHexStringToInt(FO);                            //Make sure offset is valid
                if (FON > HSFile.size() - 1)
                {
                    cerr << "***CRITICAL***\tInvalid file offset in the Hi-Score File: " << ScriptEntryType << "\t" << ScriptEntry << "\n";
                    return false;
                }
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
void SetMaxSector()
{
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

bool ResetBundleVariables()
{
    FileCnt = -1;

    tmpPrgs.clear();

    VFileCnt = -1;

    tmpVFiles.clear();

    BundleCnt++;

    BlockCnt = 0;

    UncompBundleSize = 0;

    TmpSetNewBlock = false;

    TmpDirEntryIndex = 0;

#ifdef NEWIO
    BundleUnderIO = false;
#endif
    return true;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

void NewDisk()
{
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

bool ResetDiskVariables()
{
    if (DiskCnt == 126)
    {
        cerr << "***CRITICAL***\tYou have reached the maximum number of disks (127) in this project.\n";
        return false;
    }

    DiskCnt++;

    //Reset Bundle File variables here, to have an empty array for the first compression on a ReBuild
    Prgs.clear();    //this is the one that is needed for the first CompressPart call during a ReBuild

    //Reset directory arrays
    fill_n(DirBlocks, 512, 0);
    fill_n(DirPtr, 128, 0);
    fill_n(AltDirBitPtr, 128, 0);
    fill_n(AltDirBundleNo, 128, 0);

    //Reset disk system to support 35 tracks
    TracksPerDisk = StdTracksPerDisk;

    //Reset Hi-Score Saver plugin variables
    bSaverPlugin = false;
    HSFileName = "";
    HSAddress = 0;
    HSOffset = 0;
    HSLength = 0;

#ifdef TESTDISK
    //Not a fetch test disk
    bTestDisk = false;
#endif // TESTDISK

    //Reset interleave
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

    ParsedEntries = "";

    DirEntriesUsed = false;

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

    if (!ResetBundleVariables())
    {            //Also adds first bundle
        return false;
    }

    NewBundle = false;

    return true;

}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool SplitScriptEntry()
{
    unsigned int Pos = 0;
    unsigned char ThisChar = 0;
    ScriptEntryType = "";
    NumScriptEntries = -1;

    std::fill_n(ScriptEntryArray, MaxNumEntries, "");

    while (Pos < ScriptEntry.length())
    {
        ThisChar = tolower(ScriptEntry[Pos++]);
        if ((ThisChar != ' ') && (ThisChar != '\t'))
        {
            ScriptEntryType += ThisChar;            //Entry Type to lower case
        }
        else
        {
            break;
        }
    }

    if (ScriptEntryType == EntryTypeAlign)
    {
        return true;    //This is an Align entry, rest of the script entry is ignored (it has no parameters)
    }

    //Find beginning of first script entry parameter - here both space and tAB are allowed
    while (Pos < ScriptEntry.length())
    {
        ThisChar = ScriptEntry[Pos];

        if ((ThisChar == ' ') || (ThisChar == '\t'))
        {
            Pos++;
        }
        else
        {
            break;
        }
    }

    NumScriptEntries = 0;

    bool BracketsOn = false;
    bool FileNameInBrackets = false;

    while (Pos < ScriptEntry.length())
    {
        ThisChar = ScriptEntry[Pos++];

        //If the file name/first parameter is between quotation marks then allow space as entry parameter separator OUTSIDE quotation marks
            
        if (ThisChar == '\"')               //File:    "C:\demo\part\file 1.prg*"    -    2000    2fff
        {
            BracketsOn = !BracketsOn;       //We are within quotaiton marks
            FileNameInBrackets = true;      //we can use space as entry parameter separator OUTSIDE quotes
        }
        else if (FileNameInBrackets)
        {
            if (BracketsOn)
            {
                ScriptEntryArray[NumScriptEntries] += ThisChar; //We are within curley brackets -> this is a file name
            }
            else
            {
                if ((ThisChar != '\t') && (ThisChar != ' '))
                {
                    ScriptEntryArray[NumScriptEntries] += ThisChar;
                }
                else
                {
                    if (!ScriptEntryArray[NumScriptEntries].empty())
                    {
                        if (NumScriptEntries < MaxNumEntries - 1)
                        {
                            NumScriptEntries++;
                        }
                        else
                        {
                            ScriptEntryArray[NumScriptEntries] += ThisChar;
                        }
                    }
                }
            }
        }
        else
        {
            //File name/first parameter is NOT between quotation marks - we can only allow TAB as entry parameter separator
            if (ThisChar != '\t')
            {
                ScriptEntryArray[NumScriptEntries] += ThisChar;
            }
            else
            {
                if (!ScriptEntryArray[NumScriptEntries].empty())
                {
                    if (NumScriptEntries < MaxNumEntries - 1)
                    {
                        NumScriptEntries++;
                    }
                    else
                    {
                        ScriptEntryArray[NumScriptEntries] += ThisChar;
                    }
                }
            }
        }
    }

    return true;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool FindNextScriptEntry()
{
    while ((LineEnd = Script.find("\n", LineStart)) == LineStart)
    {
        NewBundle = true;
        LineStart++;
    }
    
    if (LineEnd != string::npos)
    {
        ScriptEntry = Script.substr(LineStart, LineEnd - LineStart);
    }
    else
    {
        ScriptEntry = Script.substr(LineStart);
    }

    LineStart = LineEnd + 1;

    return SplitScriptEntry();
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool InsertScript(string& SubScriptPath)
{
    ScriptLine = ScriptEntry;

    //Calculate full path
    SubScriptPath = FindAbsolutePath(SubScriptPath, ScriptPath);

    string sPath = SubScriptPath;

    if (!fs::exists(SubScriptPath))
    {
        cerr << "***CRITICAL***\tThe following script was not found and could not be processed: " << SubScriptPath << "\n";
        return false;
    }

    //Find relative path of subscript
    for (int i = sPath.length() - 1; i > 0; i--)
    {
#if _WIN32
        if ((sPath[i] != '\\') && (sPath[i] != '/'))
        {
            sPath.replace(i, 1, "");
    }
        else
        {
            break;
        }
#elif __APPLE__ || __linux__
        if (sPath[i] != '/')
        {
            sPath.replace(i, 1, "");
        }
        else
        {
            break;
        }
#endif
    }

    //Read subscript file
    string SubScript = ReadFileToString(SubScriptPath, true);

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
            if (!S.empty())
                S += "\n";

            //Add relative path of subscript to relative path of subscript entries
            if ((ScriptEntryType == EntryTypeFile) || (ScriptEntryType == EntryTypeMem) || (ScriptEntryType == EntryTypeHSFile))
            {
                ScriptEntryArray[0] = FindAbsolutePath(ScriptEntryArray[0], sPath);

                Lines[i] = ScriptEntryType + "\t" + ScriptEntryArray[0];

                for (int j = 1; j <= NumScriptEntries; j++)
                {
                    Lines[i] += "\t" + ScriptEntryArray[j];
                }
            }
            else if ((ScriptEntryType == EntryTypeScript) || (ScriptEntryType == EntryTypePath) || (ScriptEntryType == EntryTypeDirArt))
            {
                ScriptEntryArray[0] = FindAbsolutePath(ScriptEntryArray[0], sPath);

                Lines[i] = ScriptEntryType + "\t" + ScriptEntryArray[0];
            }
        }
        S += Lines[i];
    }

    string SS1 = Script.substr(0, LineStart);
    string SS2 = (LineEnd != string::npos) ? Script.substr(LineEnd, Script.length() - LineEnd) : "";
    Script = SS1 + "\n" + S + "\n" + SS2;   //Add an extre line break before and after the SubScript

    Lines.clear();

    LineEnd = LineStart++;

    return true;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

int CheckNextIO(int Address, int Length, bool NextFileUnderIO)
{
    int LastAddress = Address + Length - 1; //Address of the last byte of a file

    if (LastAddress < 256)
    {                //On ZP
        return 1;
    }
    else
    {
        return ((LastAddress >= 0xd000) && (LastAddress < 0xe000) && (NextFileUnderIO)) ? 1 : 0;    //Last byte under I/O?
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool CompressBundle()             //NEEDS PackFile() and CloseFile()
{
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
    
    int DirIdx = 0;
    
    if (DirEntriesUsed)
    {
        if (BundleNo < 256)
        {
            if (DirEntryIndex > 0)
            {
                DirIdx = DirEntryIndex;
                AltDirBitPtr[DirEntryIndex] = BitPtr;
                AltDirBundleNo[DirEntryIndex] = BufferCnt;
            }
        }
        else
        {
            cerr << "***INFO***\tThe number of file bundles is greater than 256 on this disk!\n"
                << "A DirEntry value can only be assigned to bundles 1-255.\n";
            return false;
        }
    }
    else
    {
        if (BundleNo < 128)
        {
            DirBlocks[(BundleNo * 4) + 3] = BitPtr;
            DirPtr[BundleNo] = BufferCnt;
        }
        else
        {
            MaxBundleNoExceeded = true;
        }
    }
    BundleNo++;

    //-------------------------------------------------------

    NewBundle = true;
    //LastFileOfBundle = false;

    PartialFileIndex = -1;
    int OrigSize{};

    if (BundleNo == 1)
    {
        cout << "\t\t\t\t  Original\tCompressed\t Ratio\t\t T:S  -  T:S\n";
    }

    cout << "Compressing bundle #" << (BundleNo - 1) << "...\t";

    for (size_t i = 0; i < Prgs.size(); i++)
    {
        //Mark the last file in a bundle for better compression
        //if (i == Prgs.size() - 1)
        //    LastFileOfBundle = true;

        LastFileOfBundle = (i == Prgs.size() - 1);

        //The only two parameters that are needed are FA and FUIO... FileLenA(i) is not used

        if (PartialFileIndex == -1)
            PartialFileOffset = Prgs[i].Prg.size() - 1;

        OrigSize += (Prgs[i].iFileLen + 253) / 254;

        if (!PackFile(i))
            return false;

        if (i < Prgs.size() - 1)
        {
            //WE NEED TO USE THE NEXT FILE'S ADDRESS, LENGTH AND I/O STATUS HERE
            //FOR I/O BYTE CALCULATION FOR THE NEXT PART - BUG reported by Raistlin/G*P
            PrgAdd = Prgs[i + 1].iFileAddr;
            PrgLen = Prgs[i + 1].iFileLen;
            FileUnderIO = Prgs[i + 1].FileIO;
            if (!CloseFile())
                return false;
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
    int CR = BC * 100 / OrigSize;

    cout << (OrigSize < 10 ? "  " : (OrigSize < 100 ? " " : "")) << OrigSize << " block" << ((OrigSize == 1) ? "   ->\t" : "s  ->\t");
    cout << (BC < 10 ? "  " : (BC < 100 ? " " : "")) << BC << " block" << ((BC == 1) ? " \t" : "s\t");
    cout << (CR < 10 ? "  (" : (CR < 100 ? " (" : "(")) << CR << "%)\t\t";
    cout << ((FirstT < 10) ? "0" : "") << FirstT << ":" << ((FirstS < 10) ? "0" : "") << FirstS << " - ";
    cout << ((LastT < 10) ? "0" : "") << LastT << ":" << ((LastS < 10) ? "0" : "") << LastS;

    if (DirIdx > 0)
    {
        cout << "\tDir Index: $" << hex << ((DirIdx < 16) ? "0" : "") << DirIdx << dec << "";
    }
    cout << "\n";

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

bool CompareFileAddress(FileStruct F1, FileStruct F2)
{
    return (F1.iFileAddr > F2.iFileAddr);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool SortVirtualFiles()
{
    int FSO{}, FEO{}, FSI{}, FEI{};

    if (tmpVFiles.size() > 1)
    {
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

                if (((FSI >= FSO) && (FSI <= FEO)) || ((FEI >= FSO) && (FEI <= FEO)) || ((FSO >= FSI) && (FSO <= FEI)) || ((FEO >= FSI) && (FEO <= FEI)))
                {
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
    
    if ((tmpVFiles.size() > 0) && (tmpPrgs.size() > 0))
    {
        //Check for overlaps with FILES in the same bundle
        for (size_t o = 0; o < tmpVFiles.size(); o++)
        {
            int FSO = tmpVFiles[o].iFileAddr;               //Outer loop virtual file start
            int FEO = FSO + tmpVFiles[o].iFileLen - 1;      //Outer loop virtual file end

            for (size_t i = 0; i < tmpPrgs.size(); i++)
            {
                int FSI = tmpPrgs[i].iFileAddr;             //Inner loop file start
                int FEI = FSI + tmpPrgs[i].iFileLen - 1;    //Inner loop file end

                //--|------+------|----OR----|------+------|----OR----|------+------|----OR-----|------+------|--
                //  FSO    FSI    FEO        FSO    FEI    FEO        FSI    FSO    FEI        FSI    FEO    FEI

                if (((FSI >= FSO) && (FSI <= FEO)) || ((FEI >= FSO) && (FEI <= FEO)) || ((FSO >= FSI) && (FSO <= FEI)) || ((FEO >= FSI) && (FEO <= FEI)))
                {
                    int OLS = (FSO >= FSI) ? FSO : FSI;
                    int OLE = (FEO <= FEI) ? FEO : FEI;

                    if ((OLS >= 0xD000) && (OLE <= 0xDFFF) && (tmpVFiles[o].FileIO != tmpPrgs[i].FileIO))
                    {
                        //Overlap is IO memory only and different IO status - NO OVERLAP
                    }
                    else
                    {
                        cerr << "***CRITICAL***\tThe following file and virtual file overlap in Bundle " << dec << (BundleCnt - 1) << ":\n"
                            << "File:\t" << tmpPrgs[i].FileName << " ($" << tmpPrgs[i].FileAddr << " - $" << hex << FEI << ")\n"
                            << "Mem:\t" << tmpVFiles[o].FileName << " ($" << tmpVFiles[o].FileAddr << " - $" << hex << FEO << ")\n";
                        return false;
                    }
                }
            }
        }
    }

    return true;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool SortBundle()
{
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

                    if(((FSI >= FSO) && (FSI <= FEO)) || ((FEI >= FSO) && (FEI <= FEO)) || ((FSO >= FSI) && (FSO <= FEI)) || ((FEO >= FSI) && (FEO <= FEI)))
                    {
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
            //Sort files by address
            sort(tmpPrgs.begin(), tmpPrgs.end(), CompareFileAddress);
            //for (auto x : tmpPrgs)
            //    cout << x.FileName << "\t" << x.FileAddr << "\n";
       }
#ifdef NEWIO

       if (BundleUnderIO)
       {
           vector<unsigned char> IOOff;
           IOOff.push_back(0x34);
           tmpPrgs.insert(tmpPrgs.begin(), FileStruct(IOOff, "IOOff", "023bfe", "00", "01", false));

           vector<unsigned char> IOOn;
           IOOn.push_back(0x35);
           tmpPrgs.push_back(FileStruct(IOOn, "IOOn", "02fe", "00", "01", false));
       }
#endif

        //Once Bundle is sorted, calculate the I/O status of the last byte of the first file and the number of bits that will be needed
        //to finish the last block of the previous bundle (when the I/O status of the just sorted bundle needs to be known)
        //This is used in CloseBuffer

        //Bytes needed: (1)LongMatch Tag, (2)NextBundle Tag, (3)AdLo, (4)AdHi, (5)First Lit, (6)1 Bit Stream Byte (=BitPtr for 1st Lit Bit), (7)+/- I/O
        //+/- 1 Match Bit (if the last sequence of the last bundle is a match sequence, no Match Bit after a Literal sequence)
        //Match Bit will be determened by MLen in SequenceFits() function, NOT ADDED TO BitsNeededForNextBundle here!!!

        //We may be overcalculating here but that is safer than undercalculating which would result in buggy decompression
        //If the last block is not the actual last block of the bundle...
        //With overcalculation, worst case scenario is a little bit worse compression ratio of the last block

        //DO NOT ADD A LITERAL BIT - WE ALREADY HAVE 1 BYTE ADDED HERE FOR THE FIRST BIT STREAM BYTE (=BitPtr)

       BitsNeededForNextBundle = (6 + CheckNextIO(tmpPrgs[0].iFileAddr, tmpPrgs[0].iFileLen, tmpPrgs[0].FileIO)) * 8;

       // +/- 1 Match Bit which will be added later in CloseBuffer if needed

    }

    return true;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool BundleDone()
{
    //First finish last bundle, if it exists
    if (tmpPrgs.size() > 0)
    {
        CurrentBundle++;

        //Sort files in NEXT bundle (tmpPrgs()) - this will also recalculate BitsNeededForNextbundle (48 vs 56 bits)
        if (!SortBundle())
            return false;

        //Also sort virtual files in NEXT bundle (tmpVFiles()), here we will also make sure files and virtual files do not overlap in the NEXT bundle
        if (!SortVirtualFiles())
            return false;

        //Then compress files in THIS bundle (Prgs())
        if (!CompressBundle())
            return false;                   //THIS WILL RESET NewBundle TO FALSE

        Prgs = tmpPrgs;                     //tmpPrgs gets cleared in ResetBundleVariables

        SetNewBlock = TmpSetNewBlock;       //TmpSetNewBlock gets reset in ResetBundleVariables

        DirEntryIndex = TmpDirEntryIndex;   //TmpDirEntryIndex gets reset in ResetBundleVariables

        VFiles = tmpVFiles;                 //TmpVFiles gets cleared in ResetBundleVariables

        //Then reset bundle variables (file vectors, prg vector, block cnt), increase bundle counter
        ResetBundleVariables();
    }

    return true;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool AddVirtualFile()
{

    if (NewBundle)
    {
        NewBundle = false;
        if (!BundleDone())
            return false;
    }

    string FN = FindAbsolutePath(ScriptEntryArray[0], ScriptPath);
    string FA = "";
    string FO = "";
    string FL = "";
    size_t FAN = 0;
    size_t FON = 0;
    size_t FLN = 0;
    bool FUIO = false;

    vector<unsigned char> P;

    bEntryHasExpression = false;

    if (!EvaluateParameterExpression())
    {
        return false;
    }
    if (bEntryHasExpression)
    {
        ParsedEntries += "Bundle #" + to_string(BundleCnt) + " Mem #" + to_string(VFileCnt + 1) + ":";
        for (int i = 0; i <= NumScriptEntries; i++)
        {
            ParsedEntries += "\t" + ScriptEntryArray[i];
        }
        ParsedEntries += "\n";
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
            if ((i == 1) && (ScriptEntryArray[i] == "-"))
            {
                NumParams++;
            }
            else
            {
                break;
            }
        }
        CorrectParameterStringLength(i);
    }

    if (FN.find("*") == FN.length() - 1)
    {
        FUIO = true;
        FN.replace(FN.length() - 1, 1, "");
    }

    //Get file variables from script, or get default values if there were none in the script entry
    if (fs::exists(FN))
    {
        if (ReadBinaryFile(FN, P) == -1)
        {
            cerr << "***CRITICAL***\tUnable to open the following file: " << FN << "\n";
            return false;
        }
        else if (P.size() == 0)
        {
            cerr << "***CRITICAL***\t The following file is 0 bytes long: " << FN << "\n";
            return false;
        }

        switch (NumParams)
        {
        case 1:  //No parameters in script

            if ((FN[FN.length() - 4] == '.') && (tolower(FN[FN.length() - 3]) == 's') && (tolower(FN[FN.length() - 2]) == 'i') && (tolower(FN[FN.length() - 1]) == 'd'))
            {   //SID file
                size_t P7 = P[7];
                FA = ConvertIntToHextString(P[P7] + (P[P7 + 1] * 256), 4);
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
            if (FA == "-")
            {
                //PRG file - load the whole file less the first two address bytes
                FA = ConvertIntToHextString(P[0] + (P[1] * 256), 4);
                FO = "00000002";
                FL = ConvertIntToHextString(P.size() - 2, 4);
            }
            else
            {
                FO = "00000000";                                            //Offset will be 0, length=prg length
                FL = ConvertIntToHextString(P.size(), 4);
            }
            break;
        case 3:  //Two parameters in script          
            FA = ScriptEntryArray[1];                                   //Load address from script
            if (FA == "-")
            {
                //PRG file
                FAN = (size_t)(P[0] + P[1] * 256);
                FA = ConvertIntToHextString(FAN, 4);
                FO = ScriptEntryArray[2];
                int iFON = ConvertHexStringToInt(FO) + 2 - (int)FAN;
                if ((iFON < 0) || ((size_t)iFON > P.size() - 1))
                {
                    cerr << "***CRITICAL***\tInvalid memory segment start parameter in the following Mem entry: " << ScriptEntryType << "\t" << ScriptEntry << "\n";
                    return false;
                }
                FO = ConvertIntToHextString(iFON, 4);
                FL = ConvertIntToHextString(P.size() - iFON, 4);             //Length=prg length-offset
            }
            else
            {
                FO = ScriptEntryArray[2];                                   //Offset from script
                FON = ConvertHexStringToInt(FO);                            //Make sure offset is valid
                if (FON > P.size() - 1)
                {
                    cerr << "***CRITICAL***\tInvalid file offset in the following Mem entry: " << ScriptEntryType << "\t" << ScriptEntry << "\n";
                    return false;
                }
                FL = ConvertIntToHextString(P.size() - FON, 4);             //Length=prg length-offset
            }
            break;
        case 4:  //Three parameters in script
            FA = ScriptEntryArray[1];                                   //Load address from script
            FO = ScriptEntryArray[2];                                   //Offset from script
            FL = ScriptEntryArray[3];                                   //Length from script
            if (FA == "-")
            {
                //PRG file
                FAN = (size_t)(P[0] + P[1] * 256);
                FA = ConvertIntToHextString(FAN, 4);
                int iFLN = ConvertHexStringToInt(FL) - ConvertHexStringToInt(FO) + 1;
                FL = ConvertIntToHextString(iFLN, 4);
                int iFON = ConvertHexStringToInt(FO) + 2 - (int)FAN;
                if ((iFON < 0) || ((size_t)iFON > P.size() - 1))
                {
                    cerr << "***CRITICAL***\tInvalid memory segment start parameter in the following File entry: " << ScriptEntry << "\n";
                    return false;
                }
                FO = ConvertIntToHextString(iFON, 8);
            }
            else
            {
                FON = ConvertHexStringToInt(FO);                            //Make sure offset is valid
                if (FON > P.size() - 1)
                {
                    cerr << "***CRITICAL***\tInvalid file offset in the following Mem entry: " << ScriptEntryType << "\t" << ScriptEntry << "\n";
                    return false;
                }
            }
        }

        FAN = ConvertHexStringToInt(FA);
        FON = ConvertHexStringToInt(FO);
        FLN = ConvertHexStringToInt(FL);

        //File length cannot be 0 bytes
        if (FLN == 0)
        {
            cerr << "***CRITICAL***\tInvalid file length in the following Mem entry: " << ScriptEntryType << "\t" << ScriptEntry << "\n";
            return false;
        }

        //Make sure file length is not longer than actual file
        if (FON + FLN > P.size())
        {
            cerr << "***CRITICAL***\tInvalid file length in the following Mem entry: " << ScriptEntryType << "\t" << ScriptEntry << "\n";
            return false;
        }

        //Make sure file address+length<=&H10000
        if (FAN + FLN > 0x10000)
        {
            cerr << "***CRITICAL***\tInvalid file address and/or length in the following Mem entry: " << ScriptEntryType << "\t" << ScriptEntry << "\n";
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

bool AddFileToBundle()
{
    string FN = FindAbsolutePath(ScriptEntryArray[0], ScriptPath);
    string FA = "";
    string FO = "";
    string FL = "";
    size_t FAN = 0;
    size_t FON = 0;
    size_t FLN = 0;
    bool FUIO = false;

    vector<unsigned char> P;

    bEntryHasExpression = false;

    if (!EvaluateParameterExpression())
    {
        return false;
    }
    if (bEntryHasExpression)
    {
        ParsedEntries += "Bundle #" + to_string(BundleCnt) + " File #" + to_string(FileCnt + 1) + ":";
        for (int i = 0; i <= NumScriptEntries; i++)
        {
            ParsedEntries += "\t" + ScriptEntryArray[i];
        }
        ParsedEntries +=  "\n";
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
            if ((i == 1) && (ScriptEntryArray[i] == "-"))
            {
                NumParams++;
            }
            else
            {
                break;
            }
        }
        CorrectParameterStringLength(i);
    }

    //Get file variables from script, or get default values if there were none in the script entry
/*
    if (FN.find("*") == FN.length() - 1)
    {
#ifdef NEWIO
        BundleUnderIO = true;
#endif
        FUIO = true;
        FN.replace(FN.length() - 1, 1, "");
    }
*/
    if (FN.at(FN.length() - 1) == '*')
    {
#ifdef NEWIO
        BundleUnderIO = true;
#endif
        FUIO = true;
        FN.replace(FN.length() - 1, 1, "");
    }

    //Get file variables from script, or get default values if there were none in the script entry
    if (fs::exists(FN))
    {
        //P.clear();
        if (ReadBinaryFile(FN, P) == -1)
        {
            cerr << "***CRITICAL***\tUnable to open the following file: " << FN << "\n";
            return false;
        }
        else if (P.size() == 0)
        {
            cerr << "***CRITICAL***\t The following file is 0 bytes long: " << FN << "\n";
            return false;
        }

        switch (NumParams)
        {
        case 1:  //No parameters in script

            if ((FN[FN.length() - 4] == '.') && (tolower(FN[FN.length() - 3]) == 's') && (tolower(FN[FN.length() - 2]) == 'i') && (tolower(FN[FN.length() - 1]) == 'd'))
            {   //SID file
                size_t P7 = P[7];
                FA = ConvertIntToHextString(P[P7] + (P[P7 + 1] * 256), 4);
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
            if (FA == "-")
            {
                //PRG file - load the whole file less the first two address bytes
                FA = ConvertIntToHextString(P[0] + (P[1] * 256), 4);
                FO = "00000002";
                FL = ConvertIntToHextString(P.size() - 2, 4);
            }
            else
            {
                FO = "00000000";                                            //Offset will be 0, length=prg length
                FL = ConvertIntToHextString(P.size(), 4);
            }
            break;
        case 3:  //Two parameters in script          
            FA = ScriptEntryArray[1];                                   //Load address from script
            if (FA == "-")
            {
                //PRG file
                FAN = (size_t)(P[0] + P[1] * 256);
                FA = ConvertIntToHextString(FAN, 4);
                FO = ScriptEntryArray[2];
                int iFON = ConvertHexStringToInt(FO) + 2 - (int)FAN;
                if ((iFON < 0) || ((size_t)iFON > P.size() - 1))
                {
                    cerr << "***CRITICAL***\tInvalid memory segment start parameter in the following File entry: " << ScriptEntry << "\n";
                    return false;
                }
                FO = ConvertIntToHextString(iFON, 4);
                FL = ConvertIntToHextString(P.size() - iFON, 4);             //Length=prg length-offset
            }
            else
            {
                FO = ScriptEntryArray[2];                                   //Offset from script
                FON = ConvertHexStringToInt(FO);                            //Make sure offset is valid
                if (FON > P.size() - 1)
                {
                    cerr << "***CRITICAL***\tInvalid file offset in the following File entry: " << ScriptEntry << "\n";
                    return false;
                }
                FL = ConvertIntToHextString(P.size() - FON, 4);             //Length=prg length-offset
            }
            break;
        case 4:  //Three parameters in script
            FA = ScriptEntryArray[1];                                   //Load address from script
            FO = ScriptEntryArray[2];                                   //Offset from script
            FL = ScriptEntryArray[3];                                   //Length from script
            if (FA == "-")
            {
                //PRG file
                FAN = (size_t)(P[0] + P[1] * 256);
                FA = ConvertIntToHextString(FAN, 4);
                int iFLN = ConvertHexStringToInt(FL) - ConvertHexStringToInt(FO) + 1;
                FL = ConvertIntToHextString(iFLN, 4);
                int iFON = ConvertHexStringToInt(FO) + 2 - (int)FAN;
                if ((iFON < 0) || ((size_t)iFON > P.size() - 1))
                {
                    cerr << "***CRITICAL***\tInvalid memory segment start parameter in the following File entry: " << ScriptEntry << "\n";
                    return false;
                }
                FO = ConvertIntToHextString(iFON, 8);
            }
            else
            {
                FON = ConvertHexStringToInt(FO);                            //Make sure offset is valid
                if (FON > P.size() - 1)
                {
                    cerr << "***CRITICAL***\tInvalid file offset in the following File entry: " << ScriptEntry << "\n";
                    return false;
                }
            }
        }

        FAN = ConvertHexStringToInt(FA);
        FON = ConvertHexStringToInt(FO);
        FLN = ConvertHexStringToInt(FL);

        //File length cannot be 0 bytes
        if (FLN == 0)
        {
            cerr << "***CRITICAL***\tInvalid file length in the following File entry: " << ScriptEntry << "\n";
            return false;
        }
        //Make sure file length is not longer than actual file
        if(FON + FLN > P.size())
        {
            cerr << "***CRITICAL***\tInvalid file length in the following File entry: " << ScriptEntry << "\n";
            return false;
        }

        //Make sure file address+length<=&H10000
        if (FAN + FLN > 0x10000)
        {
            cerr << "***CRITICAL***\tInvalid file address and/or length in the following File entry: " << ScriptEntry << "\n";
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

bool AddFile()
{
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

void FindNextDirSector()
{
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

void FindNextDirPos()
{

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

bool AddAsmDirEntry(string AsmDirEntry)
{
    for (size_t i = 0; i < AsmDirEntry.length(); i++)
    {
        AsmDirEntry[i] = tolower(AsmDirEntry[i]);
    }

    string EntrySegments[5];
    string delimiter = "\"";        // = " (entry gets split at quotation mark) NOT A BUG, DO NOT CHANGE THIS
    int NumSegments = 0;

    for (int i = 0; i < 5; i++)
    {
        EntrySegments[i] = "";
    }

    while ((AsmDirEntry.find(delimiter) != string::npos) && (NumSegments < 4))
    {
        EntrySegments[NumSegments++] = AsmDirEntry.substr(0, AsmDirEntry.find(delimiter));
        AsmDirEntry.erase(0, AsmDirEntry.find(delimiter) + delimiter.length());
    }
    EntrySegments[NumSegments++] = AsmDirEntry;

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

            FindNextDirPos();

            if (DirPos != 0)
            {
                if ((DirTrack == 18) && (DirSector == 1) && (DirPos == 2))
                {
                    //If this is the very first directory entry, it must be a PRG...
                    FileType = 0x82;
                    //Very first dir entry, also add loader block count
                    Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 0x1c] = LoaderBlockCount;
                }
                Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 0] = FileType;  //...otherwise, always keep FileType from ASM file
                Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 1] = 18;        //Track 18 (track pointer of boot loader)
                Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 2] = 7;         //Sector 7 (sector pointer of boot loader)

                if (EntrySegments[0].find("@") != string::npos)
                {
                    //Numeric entry
                    unsigned char Entry[16];
                    string Values[16];

                    //Fill array with 16 SHIFT+SPACE characters
                    fill_n(Entry, 16, 0xa0);
                    fill_n(Values, 16, "");

                    int NumValues = 0;
                    delimiter = "\\";
                    while ((EntrySegments[1].find(delimiter) != string::npos) && (NumValues < 15))
                    {
                        string ThisValue = EntrySegments[1].substr(0, EntrySegments[1].find(delimiter));
                        if (!ThisValue.empty())
                        {
                            Values[NumValues++] = ThisValue;
                        }
                        EntrySegments[1].erase(0, EntrySegments[1].find(delimiter) + delimiter.length());
                    }
                    Values[NumValues++] = EntrySegments[1];

                    int Idx = 0;

                    for (int v = 0; v < NumValues; v++)
                    {
                        if (!Values[v].empty())
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
                    for (size_t i = 0; i < 16; i++)
                    {
                        Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 3 + i] = Entry[i];
                    }
                }
                else
                {
                    //Text Entry, pad it with inverted space
                    //Fill the slot first with $A0
                    for (size_t i = 0; i < 16; i++)
                    {
                        Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 3 + i] = 0xa0;
                    }
                    string ThisEntry = EntrySegments[1];
                    for (size_t i = 0; (i < ThisEntry.length()) && (i < 16); i++)
                    {
                        unsigned char NextChar = toupper(ThisEntry[i]);
                        Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 3 + i] = ascii2dirart[NextChar];
                    }
                }
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

bool AddAsmDiskParameters()
{
    size_t Track18 = Track[18];

    bool QuoteOn = false;
    for (size_t i = 0; i < DirEntry.length(); i++)
    {
        if (DirEntry[i] == '\"')
        {
            QuoteOn = !QuoteOn;
        }
        else if (!QuoteOn)
        {
            DirEntry[i] = tolower(DirEntry[i]);
        }
    }

    string AsmD64Name = "";
    string delimiter = "filename";

    if (DirEntry.find(delimiter) != string::npos)
    {
        AsmD64Name = DirEntry;
        AsmD64Name.erase(0, AsmD64Name.find(delimiter) + delimiter.length());
        DirEntry.erase(DirEntry.find(delimiter), DirEntry.find(delimiter) + delimiter.length());

        if (AsmD64Name.find("\"") != string::npos)
        {
            AsmD64Name = AsmD64Name.substr(AsmD64Name.find("\"") + 1);
            if (AsmD64Name.find("\"") != string::npos)
            {
                AsmD64Name = AsmD64Name.substr(0, AsmD64Name.find("\""));
            }
            else
            {
                AsmD64Name = "";
            }
        }
        else
        {
            AsmD64Name = "";
        }
    }

    if ((!AsmD64Name.empty()) && (D64Name.empty()))
    {
        D64Name = FindAbsolutePath(AsmD64Name, ScriptPath);
    }

    string AsmDiskHeader = "";
    delimiter = "name";
    if (DirEntry.find(delimiter) != string::npos)
    {
        AsmDiskHeader = DirEntry;
        AsmDiskHeader.erase(0, AsmDiskHeader.find(delimiter) + delimiter.length());

        if (AsmDiskHeader.find("\"") != string::npos)
        {
            AsmDiskHeader = AsmDiskHeader.substr(AsmDiskHeader.find("\"") + 1);
            if (AsmDiskHeader.find("\"") != string::npos)
            {
                AsmDiskHeader = AsmDiskHeader.substr(0, AsmDiskHeader.find("\""));
            }
            else
            {
                AsmDiskHeader = "";
            }
        }
        else
        {
            AsmDiskHeader = "";
        }
    }

    if ((!AsmDiskHeader.empty()) && (DiskHeader.empty()))
    {
        for (size_t i = 0; i < 16; i++)
        {
            if (i < AsmDiskHeader.size())
            {
                Disk[Track18 + 0x90 + i] = ascii2dirart[(size_t)AsmDiskHeader[i]];
            }
        }
    }

    string AsmDiskID = "";
    delimiter = "id";
    if (DirEntry.find(delimiter) != string::npos)
    {
        AsmDiskID = DirEntry;
        AsmDiskID.erase(0, AsmDiskID.find(delimiter) + delimiter.length());

        if (AsmDiskID.find("\"") != string::npos)
        {
            AsmDiskID = AsmDiskID.substr(AsmDiskID.find("\"") + 1);
            if (AsmDiskID.find("\"") != string::npos)
            {
                AsmDiskID = AsmDiskID.substr(0, AsmDiskID.find("\""));
            }
            else
            {
                AsmDiskID = "";
            }
        }
        else
        {
            AsmDiskID = "";
        }
    }

    if ((!AsmDiskID.empty()) && (DiskID.empty()))
    {
        for (size_t i = 0; i < 5; i++)
        {
            if (i < AsmDiskID.size())
            {
                Disk[Track18 + 0xa2 + i] = ascii2dirart[(size_t)AsmDiskID[i]];
            }
        }
    }

    return true;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

void ImportDirArtFromAsm()
{
    DirArt = ReadFileToString(DirArtName, false);

    if (DirArt.empty())
    {
        cerr << "***INFO***\tUnable to open the following DirArt file: " << DirArtName << "\nThe disk will be built without DirArt.\n";
        return;
    }

    //Convert the whole string to lower case for easier processing
    for (size_t i = 0; i < DirArt.length(); i++)
    {
        DirArt[i] = tolower(DirArt[i]);
    }

    string delimiter = "\n";
    //DirTrack = 18;
    //DirSector = 1;


    while (DirArt.find(delimiter) != string::npos)
    {
        DirEntry = DirArt.substr(0, DirArt.find(delimiter));
        DirArt.erase(0, DirArt.find(delimiter) + delimiter.length());
        string EntryType = DirEntry.substr(0, DirEntry.find("["));

        for (size_t i = 0; i < EntryType.length(); i++)
        {
            EntryType[i] = tolower(EntryType[i]);
        }

        if (EntryType.find(".disk") != string::npos)
        {
            if (!AddAsmDiskParameters())
                return;
        }
        else
        {
            if (!AddAsmDirEntry(DirEntry))   //Convert one line at the time
                return;
        }
    }

    if ((!DirArt.empty()) && (DirPos != 0))
    {
        DirEntry = DirArt;
        string EntryType = DirEntry.substr(0, DirEntry.find("["));

        for (size_t i = 0; i < EntryType.length(); i++)
        {
            EntryType[i] = tolower(EntryType[i]);
        }

        if (EntryType.find(".disk") != string::npos)
        {
            if (!AddAsmDiskParameters())
                return;
        }
        else
        {
            AddAsmDirEntry(DirEntry);
        }
    }

    return;
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
        if (!DirEntry.substr(0, EntryStart).empty())
        {
            Entry[NumEntries++] = DirEntry.substr(0, EntryStart);
        }

        DirEntry.erase(0, EntryStart + delimiter.length());

        if (NumEntries == RowLen)   //Max row length reached, ignore rest of the char row
        {
            break;
        }
    }

    if ((NumEntries < 16) && (!DirEntry.empty()))
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
                    Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 3 + i] = petscii2dirart[bEntry[i]];
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

void ImportDirArtFromCArray()
{
    string DA = ReadFileToString(DirArtName, false);

    if (DA.empty())
    {
        cerr << "***INFO***\tUnable to open the following DirArt file: " << DirArtName << "\nThe disk will be built without DirArt.\n";
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

    //DirTrack = 18;
    //DirSector = 1;

    int RowLen = min(ConvertStringToInt(sRowLen),16);
    int RowCnt = min(ConvertStringToInt(sRowCnt),48);

    size_t First = DA.find("{");
    //size_t Last = DA.find("}");

    DA.erase(0, First + 1);

    string LineBreak = "\n";
    size_t CALineStart = 0;
    int NumRows = 0;
    bool DirFull = false;

    while (((CALineStart = DA.find(LineBreak)) != string::npos) && (NumRows < RowCnt))
    {
        DirEntry = DA.substr(0, CALineStart);     //Extract one dir entry

        if (!AddCArrayDirEntry(RowLen))
        {
            DirFull = true;
            break;
        }

        DA.erase(0, CALineStart + LineBreak.length());

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

void ImportDirArtFromBinary()
{
    vector<unsigned char> DA;

    if (ReadBinaryFile(DirArtName, DA)==-1)
    {
        cerr << "***INFO***\tUnable to open the following DirArt file: " << DirArtName << "\nThe disk will be built without DirArt.\n";
        return;
    }
    else if (DA.size() == 0)
    {
        cerr << "***INFO***\t The DirArt file is 0 bytes long.\nThe disk will be built without DirArt.\n";
        return;
    }

    //DirTrack = 18;
    //DirSector = 1;

    size_t DAPtr = (DirArtType == "prg") ? 2 : 0;
    size_t EntryStart = DAPtr;
    while (DAPtr <= DA.size())
    {
        if (((DAPtr == DA.size()) && (DAPtr != EntryStart)) || (DAPtr == EntryStart + 16) || (DAPtr < DA.size() && (DA[DAPtr] == 0xa0)))
        {
            //if (DAPtr > EntryStart)   //BUG FIX - accept empty lines, i.e. treat each 0xa0 chars as line ends
            //{
                FindNextDirPos();

                if (DirPos != 0)
                {
                    Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 0] = 0x82;      //"PRG" -  all dir entries will point at first file in dir
                    Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 1] = 18;        //Track 18 (track pointer of boot loader)
                    Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 2] = 7;         //Sector 7 (sector pointer of boot loader)

                    for (size_t i = 0; i < 16; i++)                 //Fill dir entry with 0xa0 chars first
                    {
                        Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 3 + i] = 0xa0;
                    }

                    for (size_t i = 0; i < DAPtr - EntryStart; i++) //Then copy dir entry
                    {
                        Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 3 + i] = petscii2dirart[DA[EntryStart + i]];
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

                while ((DAPtr < DA.size()) && (DAPtr < EntryStart + 39) && (DA[DAPtr] != 0xa0))
                {
                    DAPtr++;        //Find last char of screen row (start+39) or first $a0-byte if sooner
                }
            //}
            EntryStart = DAPtr + 1; //Start of next line
        }
        DAPtr++;
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

void ImportDirArtFromTxt()
{
    DirArt = ReadFileToString(DirArtName, false);

    if (DirArt.empty())
    {
        cerr << "***INFO***\tUnable to open the following DirArt file: " << DirArtName << "\nThe disk will be built without DirArt.\n";
        return;
    }

    string delimiter = "\n";
    //DirTrack = 18;
    //DirSector = 1;

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

    if ((!DirArt.empty()) && (DirPos != 0))
    {
        FindNextDirPos();
        if (DirPos != 0)
        {
            AddDirEntry(DirArt);
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

void ImportDirArtFromPet()
{
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
    else if (PetFile.size() == 0)
    {
        cerr << "***INFO***\t The DirArt file is 0 bytes long: " << DirArtName << "\nThe disk will be built without DirArt.\n";
        return;
    }

    unsigned char RowLen = PetFile[0];
    unsigned char RowCnt = PetFile[1] > 48 ? 48 : PetFile[1];

    //DirTrack = 18;
    //DirSector = 1;

    for (size_t rc = 0; rc < RowCnt; rc++)
    {
        FindNextDirPos();
        if (DirPos != 0)
        {

            if (Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 0] == 0)
            {
                Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 0] = 0x82;  //"PRG" -  all dir entries will point at first file in dir
                Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 1] = 18;    //Track 18 (track pointer of boot loader)
                Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 2] = 7;     //Sector 7
            }

            if ((DirTrack == 18) && (DirSector == 1) && (DirPos == 2))
            {
                //Very first dir entry, also add loader block count
                Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 0x1c] = LoaderBlockCount;
            }

            for (size_t i = 0; i < 16; i++)
            {
                Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 3 + i] = 0xa0;
            }

            for (size_t i = 0; (i < RowLen) && (i < 16); i++)
            {
                Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 3 + i] = petscii2dirart[PetFile[5 + (rc * RowLen) + i]];
            }
        }
        else
        {
            break;
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

void ImportDirArtFromD64()
{
    vector<unsigned char> DA;

    if(ReadBinaryFile(DirArtName, DA) == -1)
    {
        cerr << "***INFO***\tUnable to open the following DirArt file: " << DirArtName << "\nThe disk will be built without DirArt.\n";
        return;
    }
    else if ((DA.size() != 174848) && (DA.size() != 196608))
    {
        cerr << "***INFO***\t Invalid D64 DirArt file size.\nThe disk will be built without DirArt.\n";
        return;
    }

    size_t T = 18;
    size_t S = 1;

    //DirTrack = 18;
    //DirSector = 1;

    bool DirFull = false;
    size_t DAPtr{};

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
                    if ((DirTrack == 18) && (DirSector == 1) && (DirPos == 2))
                    {
                        //Very first dir entry MUST be PRG
                        Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 0] = 0x82;
                        //Very first dir entry, also add loader block count
                        Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 0x1c] = LoaderBlockCount;
                    }
                    else
                    {
                        //All other entries - import filetype from DirArt
                        Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 0] = DA[DAPtr + b + 0];
                    }
                    
                    Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 1] = 18;        //Track 18 (track pointer of boot loader)
                    Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 2] = 7;         //Sector 7 (sector pointer of boot loader)

                    for (int i = 0; i < 16; i++)
                    {
                        Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 3 + i] = DA[DAPtr + b + 3 + i];
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

    size_t Track18 = Track[18];

    if(DiskHeader.empty())
    {
        for (size_t i = 0; i < 16; i++)
        {
            Disk[Track18 + 0x90 + i] = DA[Track18 + 0x90 + i];
        }
    }

    if (DiskID.empty())
    {
        for (size_t i = 0; i < 5; i++)
        {
            Disk[Track18 + 0xa2 + i] = DA[Track18 + 0xa2 + i];
        }
    }

}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

unsigned int GetPixel(size_t X, size_t Y)
{
    size_t Pos = Y * ((size_t)ImgWidth * 4) + (X * 4);

    unsigned char R = Image[Pos + 0];
    unsigned char G = Image[Pos + 1];
    unsigned char B = Image[Pos + 2];
    unsigned char A = Image[Pos + 3];

    unsigned int Col = (R * 0x1000000) + (G * 0x10000) + (B * 0x100) + A;

    return Col;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool IdentifyColors()
{
    unsigned int Col1 = GetPixel(0, 0);  //The first of two allowed colors per image
    unsigned int Col2 = Col1;


    //First find two colors (Col1 is already assigned to pixel(0,0))
    for (size_t y = 0; y < ImgHeight; y++)
    {
        for (size_t x = 0; x < ImgWidth; x++)
        {
            unsigned int ThisCol = GetPixel(x, y);
            if ((ThisCol != Col1) && (Col2 == Col1))
            {
                Col2 = ThisCol;
            }
            else if ((ThisCol != Col1) && (ThisCol != Col2))
            {
                cerr << "***INFO***\tThis image contains more than two colors.\nThe disk will be built without DirArt.\n";
                return false;
            }
        }
    }

    if (Col1 == Col2)
    {
        cerr << "***INFO***Unable to determine foreground and background colors in DirArt image file.\nThe disk will be built without DirArt.\n";
        return false;
    }

    //Start with same colors
    FGCol = Col1;
    BGCol = Col1;

    //First let's try to find a SPACE character -> its color will be the background color

    for (size_t cy = 0; cy < ImgHeight; cy += ((size_t)Mplr * 8))
    {
        for (size_t cx = 0; cx < ImgWidth; cx += ((size_t)Mplr * 8))
        {
            int PixelCnt = 0;

            unsigned int FirstCol = GetPixel(cx, cy);

            for (size_t y = 0; y < (size_t)Mplr * 8; y += (size_t)Mplr)
            {
                for (size_t x = 0; x < (size_t)Mplr * 8; x += (size_t)Mplr)
                {
                    if (GetPixel(cx + x, cy + y) == FirstCol)
                    {
                        PixelCnt++;
                    }
                }
            }
            if (PixelCnt == 64)         //SPACE character found (i.e. all 64 pixels are the same color - the opposite (0xa0) is not allowed in dirart)
            {
                BGCol = FirstCol;
                FGCol = (BGCol == Col1) ? Col2 : Col1;
                return true;
            }
        }//End cx
    }//End cy

#ifdef PXDENSITY
    //No SPACE found in DirArt image, now check the distribution of low and high density pixels (i.e. pixels that have the highest chance to be eighter BG of FG color)
    //THIS IS NOT USED AS THE DISTRIBUTION OF THE INDIVIDUAL CHARACTERS CANNOT BE PREDICTED
    int LoPx = 0;
    int HiPx = 0;

    for (size_t cy = 0; cy < ImgHeight; cy += (size_t)Mplr * 8)
    {
        for (size_t cx = 0; cx < ImgWidth; cx += (size_t)Mplr * 8)
        {
            //Low density pixels = pixels most likely to be 0 (0:2, 7:2, 0:5. 7:5, 5:7)
            if (GetPixel(cx + 0, cy + 2) == Col1) LoPx++;
            if (GetPixel(cx + 7, cy + 2) == Col1) LoPx++;
            if (GetPixel(cx + 0, cy + 5) == Col1) LoPx++;
            if (GetPixel(cx + 7, cy + 5) == Col1) LoPx++;
            if (GetPixel(cx + 5, cy + 7) == Col1) LoPx++;

            //High density pixels = pixels most likely to be 1 (2:3, 3:3, 4:3, 3:6, 4:6)
            if (GetPixel(cx + 2, cy + 3) == Col1) HiPx++;
            if (GetPixel(cx + 3, cy + 3) == Col1) HiPx++;
            if (GetPixel(cx + 4, cy + 3) == Col1) HiPx++;
            if (GetPixel(cx + 3, cy + 6) == Col1) HiPx++;
            if (GetPixel(cx + 4, cy + 6) == Col1) HiPx++;

        }//End cx
    }//End cy

    if (LoPx > HiPx)
    {
        BGCol = Col1;
        FGCol = Col2;
    }
    else
    {
        BGCol = Col2;
        FGCol = Col1;
    }

    if (FGCol != BGCol)
    {
        return true;
    }
#endif // PXDENSITY

    //No SPACE character
    //Let's use the darker color as BGCol
    int R1 = Col1 / 0x1000000;
    int G1 = (Col1 & 0xff0000) / 0x10000;
    int B1 = (Col1 & 0x00ff00) / 0x100;
    int R2 = Col2 / 0x1000000;
    int G2 = (Col2 & 0xff0000) / 0x10000;
    int B2 = (Col2 & 0x00ff00) / 0x100;

    double C1 = sqrt((0.21 * R1 * R1) + (0.72 * G1 * G1) + (0.07 * B1 * B1));
    double C2 = sqrt((0.21 * R2 * R2) + (0.72 * G2 * G2) + (0.07 * B2 * B2));
    //Another formula: sqrt(0.299 * R ^ 2 + 0.587 * G ^ 2 + 0.114 * B ^ 2)

    BGCol = (C1 < C2) ? Col1 : Col2;
    FGCol = (BGCol == Col1) ? Col2 : Col1;

    if (BGCol == FGCol)
    {
        cerr << "***INFO***Unable to determine foreground and background colors in DirArt image file.\nThe disk will be built without DirArt.\n";
        return false;
    }

    return true;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool DecodeBmp()
{
    const size_t BIH = 0x0e;                                            //Offset of Bitmap Info Header within raw data
    const size_t DATA_OFFSET = 0x0a;                                    //Offset of Bitmap Data Start within raw data
    const size_t MINSIZE = sizeof(tagBITMAPINFOHEADER) + BIH;           //Minimum header size

    if (ImgRaw.size() < MINSIZE)
    {
        cerr << "***INFO***\tThe size of this BMP file is smaller than the minimum size allowed.\nThe disk will be built without DirArt.\n";
        return false;
    }

    memcpy(&BmpInfoHeader, &ImgRaw[BIH], sizeof(BmpInfoHeader));

    if (BmpInfoHeader.biWidth % 128 != 0)
    {
        cerr << "***INFO***\tUnsupported BMP size. The image must be 128 pixels (16 chars) wide or a multiple of it if resized.\nThe disk will be built without DirArt.\n";
        return false;
    }

    if ((BmpInfoHeader.biCompression != 0) && (BmpInfoHeader.biCompression != 3))
    {
        cerr << "***INFO***\tUnsupported BMP format. Sparkle can only work with uncompressed BMP files.\nThe disk will be built without DirArt.\n";
        return false;
    }

    int ColMax = (BmpInfoHeader.biBitCount < 24) ? (1 << BmpInfoHeader.biBitCount) : 0;

    PBITMAPINFO BmpInfo = (PBITMAPINFO)new char[sizeof(BITMAPINFOHEADER) + (ColMax * sizeof(RGBQUAD))];

    //Copy info header into structure
    memcpy(&BmpInfo->bmiHeader, &BmpInfoHeader, sizeof(BmpInfoHeader));

    //Copy palette into structure
    for (size_t i = 0; i < (size_t)ColMax; i++)
    {
        memcpy(&BmpInfo->bmiColors[i], &ImgRaw[0x36 + (i * 4)], sizeof(RGBQUAD));
    }

    //Calculate data offset
    size_t DataOffset = (size_t)ImgRaw[DATA_OFFSET] + (size_t)(ImgRaw[DATA_OFFSET + 1] * 0x100) + (size_t)(ImgRaw[DATA_OFFSET + 2] * 0x10000) + (size_t)(ImgRaw[DATA_OFFSET + 3] * 0x1000000);

    //Calculate length of pixel rows in bytes
    size_t RowLen = ((size_t)BmpInfo->bmiHeader.biWidth * (size_t)BmpInfo->bmiHeader.biBitCount) / 8;

    //BMP pads pixel rows to a multiple of 4 in bytes
    size_t PaddedRowLen = (RowLen % 4) == 0 ? RowLen : RowLen - (RowLen % 4) + 4;
    //size_t PaddedRowLen = ((RowLen + 3) / 4) * 4;
    
    size_t CalcSize = DataOffset + (BmpInfo->bmiHeader.biHeight * PaddedRowLen);

    if (ImgRaw.size() != CalcSize)
    {
        cerr << "***INFO***\tCorrupted BMP file size.\nThe disk will be built without DirArt.\n";

        delete[] BmpInfo;

        return false;
    }

    //Calculate size of our image vector (we will use 4 bytes per pixel in RGBA format)
    size_t BmpSize = (size_t)BmpInfo->bmiHeader.biWidth * 4 * (size_t)BmpInfo->bmiHeader.biHeight;

    //Resize image vector
    Image.resize(BmpSize, 0);

    size_t BmpOffset = 0;

    if (ColMax != 0)    //1 bit/pixel, 4 bits/pixel, 8 bits/pixel modes - use palette data
    {
        int BitsPerPx = BmpInfo->bmiHeader.biBitCount;   //1         4          8
        int bstart = 8 - BitsPerPx;                      //1-bit: 7; 4-bit:  4; 8-bit =  0;
        int mod = 1 << BitsPerPx;                        //1-bit: 2; 4-bit: 16; 8-bit: 256;

        for (int y = (BmpInfo->bmiHeader.biHeight - 1); y >= 0; y--)    //Pixel rows are read from last bitmap row to first
        {
            size_t RowOffset = DataOffset + (y * PaddedRowLen);

            for (size_t x = 0; x < RowLen; x++)                         //Pixel row are read left to right
            {
                unsigned int Pixel = ImgRaw[RowOffset + x];

                for (int b = bstart; b >= 0; b -= BitsPerPx)
                {
                    int PaletteIndex = (Pixel >> b) % mod;

                    Image[BmpOffset + 0] = BmpInfo->bmiColors[PaletteIndex].rgbRed;
                    Image[BmpOffset + 1] = BmpInfo->bmiColors[PaletteIndex].rgbGreen;
                    Image[BmpOffset + 2] = BmpInfo->bmiColors[PaletteIndex].rgbBlue;
                    BmpOffset += 4;
                }
            }
        }
    }
    else
    {
        int BytesPerPx = BmpInfo->bmiHeader.biBitCount / 8;    //24 bits/pixel: 3; 32 bits/pixel: 4

        for (int y = (BmpInfo->bmiHeader.biHeight - 1); y >= 0; y--)    //Pixel rows are read from last bitmap row to first
        {
            size_t RowOffset = DataOffset + (y * PaddedRowLen);

            for (size_t x = 0; x < RowLen; x += BytesPerPx)              //Pixel row are read left to right
            {
                Image[BmpOffset + 0] = ImgRaw[RowOffset + x + 2];
                Image[BmpOffset + 1] = ImgRaw[RowOffset + x + 1];
                Image[BmpOffset + 2] = ImgRaw[RowOffset + x + 0];
                BmpOffset += 4;
            }
        }
    }

    ImgWidth = BmpInfo->bmiHeader.biWidth;
    ImgHeight = BmpInfo->bmiHeader.biHeight;

    delete[] BmpInfo;

    return true;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

void ImportDirArtFromImage()
{
    ImgRaw.clear();
    Image.clear();

    if (ReadBinaryFile(DirArtName, ImgRaw) == -1)
    {
        cerr << "***INFO***\tUnable to open image DirArt file.\nThe disk will be built without DirArt.\n";
        return;
    }
    else if (ImgRaw.size() == 0)
    {
        cerr << "***INFO***\tThe DirArt file cannot be 0 bytes long.\nThe disk will be built without DirArt.\n";
        return;
    }

    if (DirArtType == "png")
    {
        //Decode PNG image using LodePNG (Copyright (c) 2005-2023 Lode Vandevenne)
        unsigned int error = lodepng::decode(Image, ImgWidth, ImgHeight, ImgRaw);

        if (error)
        {
            cout << "***INFO***\tPNG decode error: " << error << ": " << lodepng_error_text(error) << "\nThe disk will be built without DirArt.\n";
            return;
        }
    }
    else if (DirArtType == "bmp")
    {

        if (!DecodeBmp())
        {
            return;
        }
    }
    //Here we have the image decoded in Image vector of unsigned chars, 4 bytes representing a pixel in RGBA format

    if (ImgWidth % 128 != 0)
    {
        cerr << "***INFO***\tUnsupported image size. The image must be 128 pixels (16 chars) wide or a multiple of it if resized.\nThe disk will be built without DirArt.\n";
        return;
    }

    Mplr = ImgWidth / 128;

    if (!IdentifyColors())
    {
        return;
    }

    //DirTrack = 18;
    //DirSector = 1;

    int PixelCnt = 0;

    for (size_t cy = 0; cy < ImgHeight; cy += (size_t)Mplr * 8)
    {
        unsigned int Entry[16]{};
        for (size_t cx = 0; cx < ImgWidth; cx += (size_t)Mplr * 8)
        {
            PixelCnt = 0;
            for (size_t y = 0; y < (size_t)Mplr * 8; y += (size_t)Mplr)
            {
                for (size_t x = 0; x < (size_t)Mplr * 8; x += (size_t)Mplr)
                {
                    if (GetPixel(cx + x, cy + y) == FGCol)
                    {
                        PixelCnt++;
                    }
                }//End x
            }//End y
            Entry[cx / ((size_t)Mplr * 8)] = 32;

            for (size_t i = 0; i < 256; i++)
            {
                if (PixelCnt == pixelcnttab[i])
                {
                    size_t px = i % 16;
                    size_t py = i / 16;
                    bool Match = true;
                    for (size_t y = 0; y < 8; y++)
                    {
                        for (size_t x = 0; x < 8; x++)
                        {
                            size_t PixelX = cx + ((size_t)Mplr * x);
                            size_t PixelY = cy + ((size_t)Mplr * y);
                            unsigned int ThisCol = GetPixel(PixelX, PixelY);
                            unsigned char DACol = 0;
                            if (ThisCol == FGCol)
                            {
                                DACol = 1;
                            }

                            size_t CharSetPos = (py * 8 * 128) + (y * 128) + (px * 8) + x;

                            if (DACol != charsettab[CharSetPos])
                            {
                                Match = false;
                                break;
                            }
                        }//Next x
                        if (!Match)
                        {
                            break;
                        }//End if
                    }//Next y
                    if (Match)
                    {
                        Entry[cx / ((size_t)Mplr * 8)] = i;
                        break;
                    }//End if
                }//End if
            }//Next i
        }//Next cx

        FindNextDirPos();

        if (DirPos != 0)
        {
            Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 0] = 0x82;      //"PRG" -  all dir entries will point at first file in dir
            Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 1] = 18;        //Track 18 (track pointer of boot loader)
            Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 2] = 7;         //Sector 7 (sector pointer of boot loader)

            for (int i = 0; i < 16; i++)
            {
                unsigned int C = Entry[i];
                Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 3 + i] = petscii2dirart[C];
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
    }//Next cy

    return;

}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

void ImportFromJson()
{
    DirArt = ReadFileToString(DirArtName, false);

    if (DirArt.empty())
    {
        cerr << "***INFO***\tUnable to open the following .JSON DirArt file: " << DirArtName << "\nThe disk will be built without DirArt.\n";
        return;
    }

    //Convert the whole string to lower case for easier processing
    for (size_t i = 0; i < DirArt.length(); i++)
    {
        DirArt[i] = tolower(DirArt[i]);
    }
    int RowLen = 40;
    int RowCnt = 25;

    //DirTrack = 18;
    //DirSector = 1;

    if (DirArt.find("width") != string::npos)
    {
        //find numeric string between "width":XX,
        string W = DirArt.substr(DirArt.find("width"));
        W = W.substr(W.find(":") + 1);
        W = W.substr(0, W.find(","));
        while (W.find(" ") != string::npos)
        {
            //Remove space characters if there's any
            W.replace(W.find(" "), 1, "");
        }
        if ((W.length() > 0) && (IsNumeric(W)))
        {
            RowLen = ConvertStringToInt(W);
        }
    }

    if (DirArt.find("height") != string::npos)
    {
        //find numeric string between "height":XX,
        string H = DirArt.substr(DirArt.find("height"));
        H = H.substr(H.find(":") + 1);
        H = H.substr(0, H.find(","));
        while (H.find(" ") != string::npos)
        {
            //Remove space characters if there's any
            H.replace(H.find(" "), 1, "");
        }
        if ((H.length() > 0) && (IsNumeric(H)))
        {
            RowCnt = ConvertStringToInt(H);
        }
    }
    unsigned char* ScreenCodes = new unsigned char[RowLen * RowCnt] {};

    if (DirArt.find("screencodes") != string::npos)
    {
        //find numeric string between "height":XX,
        string S = DirArt.substr(DirArt.find("screencodes"));
        S = S.substr(S.find("[") + 1);
        S = S.substr(0, S.find("]"));
        while (S.find(" ") != string::npos)
        {
            //Remove space characters if there's any
            S.replace(S.find(" "), 1, "");
        }

        size_t p = 0;

        while (S.find(",") != string::npos)
        {
            string B = S.substr(0, S.find(","));
            S.erase(0, S.find(",") + 1);
            if (IsNumeric(B))
            {
                ScreenCodes[p++] = ConvertStringToInt(B);
            }
            else if (IsHexString(B))
            {
                ScreenCodes[p++] = ConvertHexStringToInt(B);
            }
        }

        //Last screen code, after asr comma
        if (IsNumeric(S))
        {
            ScreenCodes[p++] = ConvertStringToInt(S);
        }
        else if (IsHexString(S))
        {
            ScreenCodes[p++] = ConvertHexStringToInt(S);
        }

        for (int rc = 0; rc < RowCnt; rc++)
        {
            FindNextDirPos();

            if (DirPos != 0)
            {
                Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 0] = 0x82;      //"PRG" -  all dir entries will point at first file in dir
                Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 1] = 18;        //Track 18 (track pointer of boot loader)
                Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 2] = 7;         //Sector 7 (sector pointer of boot loader)

                for (int i = 0; i < 16; i++)
                {
                    Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 3 + i] = 0xa0;
                }

                for (int i = 0; i < min(16, RowLen); i++)
                {
                    Disk[Track[DirTrack] + (DirSector * 256) + DirPos + 3 + i] = petscii2dirart[ScreenCodes[(rc * RowLen) + i]];
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

    delete[] ScreenCodes;

    return;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

void AddDirArt()
{

    if (DirArtName.empty())
    {
        return;
    }

    if (!fs::exists(DirArtName))
    {
        cerr << "***INFO***\tThe following DirArt file does not exist: " << DirArtName << "\nThe disk will be built without DirArt.\n";
        return;
    }

    int ExtStart = DirArtName.length();

    for (int i = DirArtName.length() - 1; i >= 0; i--)
    {
#if _WIN32
        if ((DirArtName[i] == '\\') || (DirArtName[i] == '/'))
        {
            break;
    }
        else if (DirArtName[i] == '.')
        {
            ExtStart = i + 1;
            break;
        }
#elif __APPLE__ || __linux__
        if (DirArtName[i] == '/')
        {
            break;
        }
        else if (DirArtName[i] == '.')
        {
            ExtStart = i + 1;
            break;
        }
#endif
    }

    DirArtType = "";

    for (size_t i = ExtStart; i < DirArtName.length(); i++)
    {
        DirArtType += tolower(DirArtName[i]);
    }

    DirTrack = 18;
    DirSector = 1;

    if (DirArtType == "d64")                // Import from D64
    {
        cout << "Importing DirArt from D64 file...\n";
        ImportDirArtFromD64();
    }
    else if (DirArtType == "txt")           // Import from TXT file
    {
        cout << "Importing DirArt from text file...\n";
        ImportDirArtFromTxt();
    }
    else if (DirArtType == "prg")           // Import from PRG (binary file with header)
    {
        cout << "Importing DirArt from PRG file...\n";
        ImportDirArtFromBinary();
    }
    else if (DirArtType == "c")             // Import from Marq's PETSCII Editor C array file
    {
        cout << "Importing DirArt from C file...\n";
        ImportDirArtFromCArray();
    }
    else if (DirArtType == "asm")           // Import from Kick Assembler file parameter list
    {
        cout << "Importing DirArt from ASM file...\n";
        ImportDirArtFromAsm();
    }
    else if (DirArtType == "pet")           // Import from PET binary
    {
        cout << "Importing DirArt from PET file...\n";
        ImportDirArtFromPet();
    }
    else if (DirArtType == "json")          // Import from JSON file
    {
        cout << "Importing DirArt from JSON file...\n";
        ImportFromJson();
    }
    else if (DirArtType == "png")           // Import from PNG image using LodePNG (Copyright (c) 2005-2023 Lode Vandevenne)
    {
        cout << "Importing DirArt from PNG image...\n";
        ImportDirArtFromImage();
    }
    else if (DirArtType == "bmp")           // Import from BMP image
    {
        cout << "Importing DirArt from BMP image...\n";
        ImportDirArtFromImage();
    }
    else
    {
        cout << "Importing DirArt from unscpecified binary file...\n";
        ImportDirArtFromBinary();           // Import from any other binary file (same as PRG without header)
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

void AddDemoNameToDisk(unsigned char T, unsigned char S)
{
    if ((DemoName.empty()) && (!DirArtName.empty()))
    {
        //No DemoName defined, but we have a DirArt attached, we will add first dir entry there
        return;
    }
    else if (DemoName.empty())
    {
        //No DemoName, no DirArt - use the script's file name as DemoName
        DemoName = ScriptName.substr(ScriptPath.length());
        DemoName = DemoName.substr(0, DemoName.length() - 4);
        if (DemoName.length() > 16)
        {
            DemoName = DemoName.substr(0, 16);
        }
    }
/*
    //THIS IS NOT NEEDED, THE DIRECTORY IS EMPTY AT THIS POINT, WE ARE USING THE FIRST SLOT
    CT = 18;
    CS = 1;

    int Cnt = Track[CT] + (CS * 256);

    while (Disk[Cnt] != 0)
    {
        Cnt = Track[Disk[Cnt]] + (Disk[Cnt + 1] * 256);
    }

    size_t B = 2;
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
*/
    size_t DirEntryOffset = (size_t)Track[18] + 256 + 2;  //First directory entry's offset;

    Disk[DirEntryOffset] = 0x82;
    Disk[DirEntryOffset + 1] = T;
    Disk[DirEntryOffset + 2] = S;

    for (int W = 0; W < 16; W++)
    {
        Disk[DirEntryOffset + 3 + W] = 0xa0;
    }

    for (size_t W = 0; W < DemoName.length(); W++)
    {
        Disk[DirEntryOffset + 3 + W] = ascii2dirart[(size_t)DemoName[W]];
    }
    Disk[DirEntryOffset + 0x1c] = LoaderBlockCount;    //Length of boot loader in blocks

}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

void AddHeaderAndID()
{
    //unsigned char B;
    size_t BAM = Track[18];

    for (size_t i = 0x90; i <= 0xaa; i++)
    {
        Disk[BAM + i] = 0xa0;
    }

    if (DiskHeader.empty())
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
            Disk[BAM + 0x90 + i] = ascii2dirart[DH];
        }
    }

    if (DiskID.empty())
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
            Disk[BAM + 0xa2 + i] = ascii2dirart[DI];         //Overwrites Disk ID and DOS type (5 characters max.)
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------
#ifdef TESTDISK

bool InjectTestPlugin()
{
    //Calculate sector pointer on disk (FT.cpp takes one block)

    //cout << BufferCnt << "\n";      //3

    //cout << BundleCnt << "\n";      //1

    //cout << BlocksFree-1 << "\n";   //660

    BlocksUsedByPlugin = 1;

    int FirstBuffer = BufferCnt;
    int BlocksForTest = BlocksFree - BlocksUsedByPlugin;
    int NumTestBundles = 12;
    int TestBundleSize = BlocksForTest / NumTestBundles;
    if (TestBundleSize * NumTestBundles < BlocksForTest)
        TestBundleSize++;
    unsigned char v = 0;
    for (int b = 0; b < 256; b++)
    {
        Buffer[b] = EORtransform(v);
        if (v == 0)
        {
            v = 255;
        }
        else
        {
            v--;
        }
    }

    for (int i = 1; i <= NumTestBundles; i++)
    {

        DirBlocks[(BundleNo * 4) + 3] = 0;
        DirPtr[BundleNo] = BufferCnt;
        BundleNo++;

        for (int n = 0; n < TestBundleSize; n++)
        {
            if (n == 0)
            {
                if (i != NumTestBundles)
                {
                    Buffer[1] = EORtransform(TestBundleSize);
                }
                else
                {
                    Buffer[1] = EORtransform(TestBundleSize-1);
                }
            }
            else
            {
                Buffer[1] = EORtransform(255);
            }

            Buffer[0] = EORtransform(TabS[BufferCnt]);

            memcpy(&ByteSt[BufferCnt * 256], &Buffer[0], 256 * sizeof(Buffer[0]));
            BufferCnt++;
            //BlocksFree--; //Not needed - DeleteBit will update BlocksFree

        }

        BlocksForTest -= TestBundleSize;
        if (BlocksForTest < TestBundleSize)
            TestBundleSize = BlocksForTest;

    }

    InjectDirBlocks();

    for (int i = FirstBuffer; i < BufferCnt; i++)
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

    if (BlocksFree < BlocksUsedByPlugin)
    {
        cerr << "***CRITICAL***\tThe Disk Test Plugin cannot be added because there is not enough free space on the disk!\n";
        return false;
    }

    int SctPtr = SectorsPerDisk - 1;

    //Identify first T/S of the saver plugin
    CT = TabT[SctPtr];
    CS = TabS[SctPtr];

    unsigned char FTCode[256]{};

    for (int i = 0; i < sf_size; i++)
    {
        FTCode[i] = sf[i];
    }

    //Copy first block of saver plugin to disk
    for (int i = 0; i < sf_size; i++)
    {
        int j = 0 - i;
        if (j < 0)
            j += 256;
        Disk[Track[CT] + (CS * 256) + j] = EORtransform(FTCode[i]);
    }

    //Mark sector off in BAM
    DeleteBit(CT, CS);

    //Add plugin to directory
    Disk[Track[18] + (18 * 256) + 8] = EORtransform(CT);              //DirBlocks(0) = EORtransform(Track) = 35
    Disk[Track[18] + (18 * 256) + 7] = EORtransform(TabStartS[CT]);   //DirBlocks(1) = EORtransform(Sector) = First sector of Track(35) (not first sector of file!!!)
    Disk[Track[18] + (18 * 256) + 6] = EORtransform(TabSCnt[SctPtr]); //DirBlocks(2) = EORtransform(Remaining sectors on track)
    Disk[Track[18] + (18 * 256) + 5] = 0xfe;                                //DirBlocks(3) = BitPtr

    return true;

}

#endif // TESTDISK

//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool InjectSaverPlugin()
{
    if (HSFile.size() == 0)
    {
        cerr << "***CRITICAL***\tThe Hi-Score File's size must be a multiple of $100 bytes, but not greater than $f00 bytes.\n";
        return false;
    }
    if (HSFileName.empty())
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

    BlocksUsedByPlugin = (HSLength / 0x100) + 1 + 2;

    if (BlocksFree < BlocksUsedByPlugin)
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

        for(int i = 0; i < ssio_size; i++)
        {
            SaveCode[i] = ssio[i];
        }
        SaveCodeSize = ssio_size;
    }
    else
    {
        SaverSupportsIO = false;

        for (int i = 0; i < ss_size; i++)
        {
            SaveCode[i] = ss[i];
        }
        SaveCodeSize = ss_size;
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
    int SctPtr = SectorsPerDisk - 2;
    SctPtr -= ((HSLength / 256) + 1);

    //Identify first T/S of the saver plugin
    CT = TabT[SctPtr];
    CS = TabS[SctPtr];
    int FirstT = CT;
    int FirstS = CS;
    int LastT = TabT[SctPtr+1];
    int LastS = TabS[SctPtr+1];

    cout << "Adding Hi-score Saver Plugin...\t\t    ->    2 blocks\t\t\t";
    cout << ((FirstT < 10) ? "0" : "") << FirstT << ":" << ((FirstS < 10) ? "0" : "") << FirstS << " - " << ((LastT < 10) ? "0" : "") << LastT << ":" << ((LastS < 10) ? "0" : "") << LastS << "\n";

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
    FirstT = CT;
    FirstS = CS;
    
    int HSFBC = BlocksUsedByPlugin-2;
    
    LastT = TabT[SectorsPerDisk - 1];
    LastS = TabS[SectorsPerDisk - 1];

    TotalOrigSize += BlocksUsedByPlugin;

    cout << "Adding Hi-score File...\t\t\t    ->   " << (HSFBC < 10 ? " " : "") << HSFBC << " block" << ((HSFBC == 1) ? " \t\t\t" : "s\t\t\t");
    cout << ((FirstT < 10) ? "0" : "") << FirstT << ":" << ((FirstS < 10) ? "0" : "") << FirstS << " - " << ((LastT < 10) ? "0" : "") << LastT << ":" << ((LastS < 10) ? "0" : "") << LastS << "\n";

    Disk[Track[18] + (18 * 256) + 4] = EORtransform(CT);                //DirBlocks(0) = EORtransform(Track) = 35
    Disk[Track[18] + (18 * 256) + 3] = EORtransform(TabStartS[CT]);     //DirBlocks(1) = EORtransform(Sector) = First sector of Track(35) (not first sector of file!!!)
    Disk[Track[18] + (18 * 256) + 2] = EORtransform(TabSCnt[SctPtr]);   //DirBlocks(2) = EORtransform(Remaining sectors on track)
    Disk[Track[18] + (18 * 256) + 1] = 0xfe;                                 //DirBlocks(3) = BitPtr

    DeleteBit(CT, CS);

    unsigned char SBuffer[256]{};
    int HSStartAdd = HSAddress + HSLength - 1;
    unsigned char BlockCnt = HSLength / 256;


    //First block
    SBuffer[0] = 0;
    SBuffer[1] = EORtransform((HSLength / 256));                //Remaining block count (EOR transformed)
    SBuffer[255] = 0xfe;                                             //First byte of block
    SBuffer[254] = 0x81;                                             //Bit stream
    SBuffer[253] = HSStartAdd % 256;                                 //Last byte's address(Lo)
    if (SaverSupportsIO)
    {
        SBuffer[252] = 0;                                            //I/O flag
        SBuffer[251] = HSStartAdd / 256;                             //Last byte's address(Hi)
        SBuffer[250] = 0;                                            //LongLit flag
        SBuffer[249] = 0xf6;                                         //Number of literals - 1
    }
    else
    {
        SBuffer[252] = HSStartAdd / 256;                             //Last byte's address(Hi)
        SBuffer[251] = 0;                                            //LongLit flag
        SBuffer[250] = 0xf7;                                         //Number of literals - 1
    }

    for (int i = 2; i <= (SaverSupportsIO ? 248 : 249); i++)
    {
        SBuffer[i] = HSFile[HSLength - 1 - (SaverSupportsIO ? 248 : 249) + i];
    }

    for (int i = 0; i < 256; i++)
    {
        Disk[Track[CT] + (CS * 256) + i] = SBuffer[i];
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

        memset(SBuffer, 0, sizeof(SBuffer));
        //fill(SBuffer, SBuffer + sizeof(SBuffer), 0);

        SBuffer[0] = 0x81;                                   //Bit stream
        SBuffer[255] = HSStartAdd % 256;                     //Last byte's address(Lo)
        if (SaverSupportsIO)
        {
            SBuffer[254] = 0;                                //I/O flag
            SBuffer[253] = HSStartAdd / 256;                 //Last byte's address(hi)
            SBuffer[252] = 0;                                //LongLit flag
            SBuffer[251] = 0xf9;                             //Number of literals - 1
        }
        else
        {
            SBuffer[254] = HSStartAdd / 256;                 //Last byte's address(hi)
            SBuffer[253] = 0;                                //LongLit flag
            SBuffer[252] = 0xfa;                             //Number of literals - 1

        }

        for (int j = 1; j <= (SaverSupportsIO ? 250 : 251); j++)
        {
            SBuffer[j] = HSFile[HSLength - 1 - (SaverSupportsIO ? 250 : 251) + j];
        }

        for (int j = 0; j < 256; j++)
        {
            Disk[Track[CT] + (CS * 256) + j] = SBuffer[j];
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

    memset(SBuffer, 0, sizeof(SBuffer));
    //fill(SBuffer, SBuffer + sizeof(SBuffer), 0);


    SBuffer[0] = 0x81;                                           //Bit stream
    SBuffer[1] = EORtransform(0);                           //New block count = 0 (eor transformed)
    SBuffer[255] = HSStartAdd % 256;                             //Last byte's address(Lo)
    if (SaverSupportsIO)
    {
        SBuffer[254] = 0;                                        //I/O flag
        SBuffer[253] = HSStartAdd / 256;                         //Last byte's address(Hi)
        SBuffer[252] = 0;                                        //LongLit flag
        SBuffer[251] = HSLength - 1;                             //Number of remaining literals - 1

    }
    else
    {
        SBuffer[254] = HSStartAdd / 256;                         //Last byte's address(Hi)
        SBuffer[253] = 0;                                        //LongLit flag
        SBuffer[252] = HSLength - 1;                             //Number of remaining literals - 1

    }

    for (size_t i = 0; i < HSLength; i++)
    {
        SBuffer[(SaverSupportsIO ? 251 : 252) - HSLength + i] = HSFile[i];
    }

    SBuffer[(SaverSupportsIO ? 251 : 252) - HSLength - 1] = NearLongMatchTag;    //End of Bundle: 0x84
    SBuffer[(SaverSupportsIO ? 251 : 252) - HSLength - 2] = EndOfBundleTag;      //End of Bundle: 0x00


    for (int i = 0; i < 256; i++)
    {
        Disk[Track[CT] + (CS * 256) + i] = SBuffer[i];
    }

    return true;

}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool InjectDriveCode(unsigned char& idcSideID, char& idcFileCnt, unsigned char& idcNextID)
{
    int BAM = Track[18];

    unsigned char Drive[6 * 256]{};

    for (int i = 0; i < sd_size; i++)
    {
        Drive[i] = sd[i];
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
#ifdef TESTDISK
    if (bTestDisk)
        idcNextID = 0x00;
#endif // TESTDISK

    Drive[(3 * 256) + ZPILTabLoc + 0] = 256 - IL3;
    Drive[(3 * 256) + ZPILTabLoc + 1] = 256 - IL2;
    Drive[(3 * 256) + ZPILTabLoc + 2] = 256 - IL1;
    Drive[(3 * 256) + ZPILTabLoc + 3] = idcNextID % 256;
    Drive[(3 * 256) + ZPILTabLoc + 4] = 256 - IL0;

    //Add PlugIn to ZP (IncSaver = $74)
    int ZPIncPluginLoc = 0x74;

#ifdef TESTDISK
    if (bTestDisk)
    {
        Drive[(3 * 256) + ZPIncPluginLoc] = 1;
    }
    else
    {
#endif // TESTDISK
        if (bSaverPlugin)
        {
            Drive[(3 * 256) + ZPIncPluginLoc] = 2;
        }
        else
        {
            Drive[(3 * 256) + ZPIncPluginLoc] = 0;
        }
#ifdef TESTDISK
    }
#endif // TESTDISK

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

    BlocksUsedByPlugin = 0;
#ifdef TESTDISK
    if (bTestDisk)
    {
        bSaverPlugin = false;   //Can't have Saver and FetchTest at the same time

        //Add "IncludeFetchTest" flag and FetchTest plugin
        Disk[BAM + 249] = EORtransform(1);
        if (!InjectTestPlugin())
            return false;
    }
    else
    {
#endif // TESTDISK
        if (bSaverPlugin)
        {
            //Add "IncludeSaveCode" flag and Saver plugin
            Disk[BAM + 249] = EORtransform(2);
            if (!InjectSaverPlugin())
                return false;
        }
        else
        {
            Disk[BAM + 249] = EORtransform(0);
        }
#ifdef TESTDISK
    }
#endif // TESTDISK


    //Also add Product ID to BAM, EOR-transformed
    Disk[BAM + 248] = EORtransform((ProductID / 0x10000) & 0xff);
    Disk[BAM + 247] = EORtransform((ProductID / 0x100) & 0xff);
    Disk[BAM + 246] = EORtransform(ProductID & 0xff);

    return true;

}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool UpdateZP()
{
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
    for (int i = 0; i < loader_size - 1 - 2; i++)
    {
        if ((loader[i] == 0x4c) && (loader[i + 1] == 0x00) && (loader[i + 2] == 0x07))
        {
            LoaderBase = i + 3;
            break;
        }
    }

    if (LoaderBase == 0xffff)
    {
        cerr << "***CRITICAL***\tZeropage offset could not be updated.\n";
        return false;
    }

    const unsigned char OPC_STAZP = 0x85;
    const unsigned char OPC_RORZP = 0x66;
    const unsigned char OPC_ASLZP = 0x06;

    const unsigned char OPC_ADCZP = 0x65;
    const unsigned char OPC_STAZPY = 0x91;

    const unsigned char OPC_DECZP = 0xC6;
    //const unsigned char OPC_LDAZP = 0xA5;
    //const unsigned char OPC_EORIMM = 0x49;
    //unsigned char OPC_LDAZPY = 0xB1;

    const unsigned char ZPDstLo = 0x02;
    const unsigned char ZPDstHi = 0x03;
    const unsigned char ZPBits = 0x04;

    //Update STA ZPDst, ADC ZPDst, STA (ZPDst),Y
    for (int i = LoaderBase; i < loader_size - 1; i++)
    {
        if ((loader[i] == OPC_STAZP) || (loader[i] == OPC_ADCZP) || (loader[i] == OPC_STAZPY))
        {
            if (loader[i + 1] == ZPDstLo)
            {
                loader[i + 1] = ZP;
                i++;
            }
        }
    }

    //Update STA ZPDst+1, DEC ZPDst+1, ADC ZPDst+1
    for (int i = LoaderBase; i < loader_size - 1; i++)
    {
        if ((loader[i] == OPC_STAZP) || (loader[i] == OPC_DECZP) || (loader[i] == OPC_ADCZP))
        {
            if (loader[i + 1] == ZPDstHi)
            {
                loader[i + 1] = ZP + 1;
                i++;
            }
        }
    }

    //Update STA Bits, ROR Bits, ASL Bits,
    for (int i = LoaderBase; i < loader_size - 1; i++)
    {
        if ((loader[i] == OPC_STAZP) || (loader[i] == OPC_RORZP) || (loader[i] == OPC_ASLZP))
        {
            if (loader[i + 1] == (ZPBits))
            {
                loader[i + 1] = ZP + 2;
                i++;
            }
        }
    }

    return true;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool InjectLoader(unsigned char T, unsigned char S, unsigned char IL)
{
    int B{};
    unsigned char ST{}, SS{}, AdLo{}, AdHi{};


    if (!DemoStart.empty())                            //Check if we have a Demo Start Address
    {
        B = ConvertHexStringToInt(DemoStart);
    }
    else if (!FirstFileStart.empty())                  //No Demo Start Address, check if we have the first file's start address
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


    if (!UpdateZP())
        return false;

    unsigned char LDA_IMM = 0xa9;
    unsigned char PHA = 0x48;

    for (int i = 0; i < loader_size - 5; i++)      //Find STA $fffb instruction which is 14 bytes from AdLo
    {
        if ((loader[i] == LDA_IMM) && (loader[i + 2] == PHA) && (loader[i + 3] == LDA_IMM) && (loader[i + 5] == PHA))
        {
            loader[i + 1] = AdHi;                  //Hi Byte return address at the end of Loader
            loader[i + 4] = AdLo;                  //Lo Byte return address at the end of Loader
            break;
        }
    }

    //Number of blocks in Loader
    LoaderBlockCount = loader_size / 254;
    if (loader_size % 254 != 0)
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
            if ((i * 254) + c < loader_size)
            {
                Disk[Track[CT] + (CS * 256) + 2 + c] = loader[(i * 254) + c];
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
            if (loader_size % 254 == 0)
            {
                Disk[Track[ST] + (SS * 256) + 1] = 255;     //254+1
            }
            else
            {
                Disk[Track[ST] + (SS * 256) + 1] = ((loader_size) % 254) + 1;
            }
        }
    }

    //delete[] Loader;

    return true;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

void CalcTabs()
{
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

bool AddCompressedBundlesToDisk()
{
    if (BlocksFree < BufferCnt)
    {
        cerr << "***CRITICAL***\t" << D64Name << " cannot be built because it would require " << BufferCnt << " blocks.\n"
             << "This disk only has " << SectorsPerDisk << " blocks.\n";
        return false;
    }

#ifdef TESTDISK
    if (!bTestDisk)
    {
#endif // TESTDISK
        InjectDirBlocks();
#ifdef TESTDISK
    }
#endif // TESTDISK

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

void UpdateBlocksFree()
{
    if (TracksPerDisk == ExtTracksPerDisk)
    {
        unsigned char ExtBlocksFree = (BlocksFree > ExtSectorsPerDisk - StdSectorsPerDisk) ? ExtSectorsPerDisk - StdSectorsPerDisk - BlocksUsedByPlugin : BlocksFree;
        Disk[Track[18] + 4] += ExtBlocksFree;
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool FinishDisk(bool LastDisk)
{
    if ((BundleCnt == 0) && (FileCnt == -1))
    {
        cerr << "***CRITICAL***\tThis disk does not contain any files!\n";
        return false;
    }

    if (!BundleDone())
        return false;
    
    //BitsNeededForNextBundle is calculated during sorting the enxt bundle, but here we have no next bundle
    BitsNeededForNextBundle = 48;       //This is the last bundle on the disk, there is no next bundle under I/O
    
    if (!CompressBundle())
        return false;
    if (!CloseBundle(0, true))
        return false;
    if (!CloseBuffer())
        return false;

    if (MaxBundleNoExceeded)
    {
        cerr << "***INFO***\tThe number of file bundles is greater than 128 on this disk!\n"
             << "You can only access bundles 0-127 by bundle index. The rest can only be loaded using the LoadNext function.\n";
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
        << BlocksFree << " block" << (BlocksFree == 1 ? "" : "s") << " remaining free. Overall compression ratio: " << setprecision(2) << fixed << CompRatio << " %\n\n";

    if (!ParsedEntries.empty())
    {
        cout << "File entries with parameter expressions after evaluation:\n" << ParsedEntries << "\n";
    }

    if (D64Name.empty())
    {
        //No D64Name provided, use the script's path and file name, add _SideA etc. if multidisk
        D64Name = ScriptName.substr(0, ScriptName.length() - 4);
        if ((!LastDisk) || (DiskCnt > 0))
        {
            D64Name += "_Side";
            char DiskSide = 'A' + DiskCnt;
            D64Name += DiskSide;
            D64Name += ".d64";
        }
   }

    if (!WriteDiskImage(D64Name))
    {
        return false;
    }

    return true;

}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

bool Build()
{
    //initialize random seed
    srand(time(NULL));

    //generate a random number between 0 and 0Xffffff
    ProductID = (((rand() & 0xff0) << 16) + (rand() & 0xffff)) & 0xffffff;

    LineStart = 0;
    LineEnd = 0;

    //FindNextScriptEntry();

    //if (ScriptEntryType != EntryTypeScriptHeader)
    //{
    //    cerr << "***CRITICAL***\tInvalid script file!\n";
    //    return false;
    //}
    
    if (!ResetDiskVariables())
    {
        return false;
    }
        
    bool NewD = true;
    NewBundle = false;
    TmpSetNewBlock = false;

    while (LineEnd != string::npos)
    {
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
                    D64Name = FindAbsolutePath(ScriptEntryArray[0], ScriptPath);
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
                        ThisID = ConvertHexStringToInt(ScriptEntryArray[0]);
                        if (ThisID > 126)   //0x00-0x7e; called with A=$80+ID, 0x7f is reserved for disk reset (called with A=$ff)
                        {
                            cerr << "***CRITICAL***\tThis Side Index must be a hex number and cannot be greater than $7e!\n";
                            return false;
                        }
                    }
                    else
                    {
                        cerr << "***CRITICAL***\tThis Side Index must be a hex number betweem $00-$7e!\n";
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
                        NextID = ConvertHexStringToInt(ScriptEntryArray[0]);
                        if (NextID > 126)   //0x00-0x7e; called with A=$80+ID, 0x7f is reserved for disk reset (called with A=$ff)
                        {
                            cerr << "***CRITICAL***\tNext Side Index must be a hex number and cannot be greater than $7e!\n";
                            return false;
                        }
                    }
                    else
                    {
                        cerr << "***CRITICAL***\tNext Side Index must be a hex number between $00-$7e!\n";
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

                string DAN = FindAbsolutePath(ScriptEntryArray[0], ScriptPath);

                if (fs::exists(DAN))
                {
                    DirArtName = DAN;
                }
                else
                {
                    cerr << "***INFO***\tThe following DirArt file does not exist: " << DAN << "\nThe disk will be built without DirArt.\n";
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

                    if (ScriptEntryArray[0].size() > 0)
                    {
                        if (ScriptEntryArray[0].at(0) == '.')
                        {
                            string sTmp = ScriptEntryArray[0].substr(1);
                            if (IsNumeric(sTmp))
                            {
                                TmpIL = ConvertStringToInt(sTmp);
                            }
                        }
                        else if (IsHexString(ScriptEntryArray[0]))
                        {
                            TmpIL = ConvertHexStringToInt(ScriptEntryArray[0]);
                        }
                    }

                    if (TmpIL % 21 > 0)
                    {
                        IL0 = TmpIL % 21;
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

                    if (ScriptEntryArray[0].size() > 0)
                    {
                        if (ScriptEntryArray[0].at(0) == '.')
                        {
                            string sTmp = ScriptEntryArray[0].substr(1);
                            if (IsNumeric(sTmp))
                            {
                                TmpIL = ConvertStringToInt(sTmp);
                            }
                        }
                        else if (IsHexString(ScriptEntryArray[0]))
                        {
                            TmpIL = ConvertHexStringToInt(ScriptEntryArray[0]);
                        }
                    }

                    if (TmpIL % 19 > 0)
                    {
                        IL1 = TmpIL % 19;
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

                    if (ScriptEntryArray[0].size() > 0)
                    {
                        if (ScriptEntryArray[0].at(0) == '.')
                        {
                            string sTmp = ScriptEntryArray[0].substr(1);
                            if (IsNumeric(sTmp))
                            {
                                TmpIL = ConvertStringToInt(sTmp);
                            }
                        }
                        else if (IsHexString(ScriptEntryArray[0]))
                        {
                            TmpIL = ConvertHexStringToInt(ScriptEntryArray[0]);
                        }
                    }

                    if (TmpIL % 18 > 0)
                    {
                        IL2 = TmpIL % 18;
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

                    if (ScriptEntryArray[0].size() > 0)
                    {
                        if (ScriptEntryArray[0].at(0) == '.')
                        {
                            string sTmp = ScriptEntryArray[0].substr(1);
                            if (IsNumeric(sTmp))
                            {
                                TmpIL = ConvertStringToInt(sTmp);
                            }
                        }
                        else if (IsHexString(ScriptEntryArray[0]))
                        {
                            TmpIL = ConvertHexStringToInt(ScriptEntryArray[0]);
                        }
                    }

                    if (TmpIL % 17 > 0)
                    {
                        IL3 = TmpIL % 17;
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
                        
                        ProductID = ConvertHexStringToInt(ScriptEntryArray[0]);
                        if (ProductID > 0xffffff)
                        {
                            cerr << "***CRITICAL***\tThe Product ID must be a maximum 6-digit long hexadecimal number!\n";
                            return false;
                        }
                    }
                    else
                    {
                        cerr << "***CRITICAL***\tThe Product ID must be a maximum 6-digit long hexadecimal number!\n";
                        return false;
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
#ifdef TESTDISK
            else if (ScriptEntryType == EntryTypeTestDisk)
            {
                if (!NewD)
                {
                    NewD = true;
                    if ((!FinishDisk(false)) || (!ResetDiskVariables()))
                        return false;
                }

                bTestDisk = true;
                NewBundle = true;
                }
#endif // TESTDISK
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
            else if (ScriptEntryType == EntryTypeDirIndex)
            {
                if (NumScriptEntries > -1)
                {
                    if (IsHexString(ScriptEntryArray[0]))
                    {

                        int tmp = ConvertHexStringToInt(ScriptEntryArray[0]);

                        if ((tmp == 0) || (tmp > 127))
                        {
                            cerr << "***CRITICAL***\tThe DirIndex value must be a hex number between $01 and $7f!\n";
                            return false;
                        }
                        DirEntriesUsed = true;      //We have a valid DirEntry -> we will use the alternative directory
                        TmpDirEntryIndex = tmp;
                    }
                    else
                    {
                        cerr << "***CRITICAL***\tThe DirIndex value must be a hex number between $01 and $7f!\n";
                        return false;
                    }
                }
                else
                {
                    cerr << "***CRITICAL***\tThe DirIndex value must be a hex number between $01 and $7f!\n";
                    return false;
                }

                //NewBundle = true; //NOT NEEDED!!! This would result in merging to adjacent bundles...
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
            /*
                cout << "\n";

                if (ScriptEntry.size() != 0)
                {
                    cout << ScriptEntry << "\n";
                }
            */
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

#if _WIN32
    if ((sPath.size() < 3) || ((sPath.substr(1, 2) != ":\\") && (sPath.substr(1, 2) != ":/")))
    {
        sPath = aPath + sPath;                      //sPath is relative - use Sparkle's base folder to make it a full path
    }
#elif __APPLE__ || __linux__
    if (sPath[0] != '/')
    {
        if ((sPath.size() > 1) && (sPath[0] == '~') && (sPath[1] == '/'))
        {
            //Home directory (~/...) -> replace "~" with HomeDir if known
            if (!HomeDir.empty())
            {
                sPath = HomeDir + sPath.substr(1);
            }
            else
            {
                cerr << "***INFO***\tUnable to identify the user's Home directory...\n";
            }
        }
        else
        {
            //sPath is relative - use Sparkle's base folder to make it a full path
            sPath = aPath + sPath;
        }
    }
#endif

#if _WIN32
    for (size_t i = 0; i < sPath.length(); i++)
    {
        if (sPath[i] == '\\')
        {
            sPath.replace(i, 1, "/");
        }
    }
#endif

    ScriptName = sPath;                             //Absolute script path + file name

    ScriptPath = sPath;                             //Absolute script path

    int i = sPath.length() - 1;

    while ((i >= 0) && (sPath[i] != '/'))
    {
        ScriptPath.replace(i--, 1, "");
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

void PrintInfo()
{
    cout << "USAGE: sparkle script.sls\n\n";
    cout << "SCRIPT TEMPLATE:\n\n";
    //cout << "[Sparkle Loader Script]\n\n";
    cout << "Path:\t\tfilepath/diskname.d64\t\t\t\t\t<< path and file name of the D64 image <- THIS IS A COMMENT\n";
    cout << "Header:\t\tdisk header\t\t\t\t\t\t<< max. 16 characters\n";
    cout << "ID:\t\tdskid\t\t\t\t\t\t\t<< max. 5 characters\n";
    cout << "Name:\t\tdemo name\t\t\t\t\t\t<< max. 16 characters\n";
    cout << "Start:\t\tabcd\t\t\t\t\t\t\t<< 4-digit hex number, first part's start address\n";
    cout << "DirArt:\t\tfilepath/dirartfile.d64\t\t\t\t\t<< D64, TXT, PRG, BIN, C array, KickAss ASM, PET, JSON, PNG, BMP file formats accepted\n";
    cout << "Tracks:\t\t35\t\t\t\t\t\t\t<< 35 vs 40, default: 35 if entry is omitted\n";
    cout << "HSFile:\t\tfilepath/hsfile.bin\tabcd\tabcdabcd\tabcd\t<< (address) (offset) (length) can be omitted if not needed\n";
    cout << "ProdID:\t\tabcdef\t\t\t\t\t\t\t<< 6-digit hex number, once per script, must be the same for disks of a multidisk prod, autogenerated if omitted\n";
    cout << "ThisSide:\tab\t\t\t\t\t\t\t<< 2-digit hex number, only if disks of a multidisk prod are built separately\n";
    cout << "NextSide:\tab\t\t\t\t\t\t\t<< 2-digit hex number, only if disks of a multidisk prod are built separately\n";
    cout << "IL0:\t\t4\t\t\t\t\t\t\t<< 1-14 (hex) or .1-.20 (decimal), default: 4 if entry is omitted\n";
    cout << "IL1:\t\t3\t\t\t\t\t\t\t<< 1-12 (hex) or .1-.18 (decimal), default: 3 if entry is omitted\n";
    cout << "IL2:\t\t3\t\t\t\t\t\t\t<< 1-11 (hex) or .1-.17 (decimal), default: 3 if entry is omitted\n";
    cout << "IL3:\t\t3\t\t\t\t\t\t\t<< 1-10 (hex) or .1-.16 (decimal), default: 3 if entry is omitted\n";
    cout << "ZP:\t\t02\t\t\t\t\t\t\t<< 02-fd (hex), once per script, must be the same for disks of a multidisk prod, default: 02 if entry is omitted\n\n";
    cout << "<< Bundle 0 - bundles must be separated by at least one blank line!\n";
    cout << "File:\t\tfilepath/file0.prg\t\t\t\t\t<< (default address) (default offset) (default length)\n\n";
    cout << "<< Bundle 1 - files marked with * will be loaded under I/O ($d000-$dfff)\n";
    cout << "DirIndex:\tab\t\t\t\t\t\t\t<< 01-7f (hex), if used, then only bundles with dir index will be added to the internal directory!!!\n";
    cout << "File:\t\tfilepath/file1.kla\tabcd\t\t\t\t<< (address) (default offset) (default length)\n";
    cout << "File:\t\tfilepath/file2.bin\tabcd\tabcdabcd\t\t<< (address) (offset) (default length)\n";
    cout << "File:\t\tfilepath/file3.prg*\tabcd\tabcdabcd\tabcd\t<< (address) (offset) (length)\n\n";
    cout << "<< Bundle 2\n";
    cout << "Script:\t\tfilepath/scriptfile.sls\t\t\t\t\t<< this will import another script here...\n\n";
    cout << "<< Entries can be fully omitted if they are not needed or their default value is used.\n";
    cout << "<< Use TAB(s) to separate entry types and their values!\n";
    cout << "<< Unrecognized entries are interpreted as comments.\n";
    cout << "<< For more details please read the user manual!\n\n";

}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    auto cstart = std::chrono::system_clock::now();

    cout << "\n**************************************\n";
    cout << "Sparkle " << VersionMajor <<"." << VersionMinor << "." << hex << VersionBuild << dec << " by Sparta 2019-" << FullYear << "\n";
    cout << "**************************************\n\n";

#if _WIN32

    string AppPath{ fs::current_path().string() };
    if (AppPath[AppPath.size() - 1] != '\\')
        AppPath += "\\";

#elif __APPLE__ || __linux__

    //Identify user's Home directory
    char const* tmp = getenv("HOME");
    if (tmp != NULL)
    {
        HomeDir = tmp;
    }
 
    string AppPath{ fs::current_path().string() };
    if (AppPath[AppPath.size() - 1] != '/')
        AppPath += "/";

#else

    cerr << "***CRITICAL***\tUnsupported operating system!\n";
    return EXIT_FAILURE;

#endif

    if (argc < 2)
    {

#ifdef DEBUG

        string ScriptFileName = "c:/Sparkle3/Example/Sparkle3.sls";
        Script = ReadFileToString(ScriptFileName, true);
        SetScriptPath(ScriptFileName, AppPath);

        //cout << ScriptName << "\n" << ScriptPath << "\n";

#else

        PrintInfo();
        return EXIT_SUCCESS;

#endif

    }
    else
    {
        string ScriptFileName(argv[1]);
        Script = ReadFileToString(ScriptFileName, true);

        SetScriptPath(ScriptFileName, AppPath);
    }

    if (Script.empty())
    {
        cerr << "***CRITICAL***\tUnable to load script file or the file is empty!\n";
        cout << "\nPress Enter to continue...\n";
        cin.get();
        return EXIT_FAILURE;
    }

    CalcTabs();     //THIS IS NEEDED HERE!!!

    if (!Build())
    {
        cout << "\nPress Enter to continue...\n";
        cin.get();
        return EXIT_FAILURE;
    }

    auto cend = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = cend - cstart;

    std::cout << "Elapsed time: " << elapsed_seconds.count() << "s\n\n";

    return EXIT_SUCCESS;
}
