#include "common.h"

using namespace std;
using namespace std::filesystem;

struct sequence {
    int Len;		//Length of the sequence in bytes (0 based)
    int Off;		//Offset of Match sequence in bytes (1 based), 0 if Literal Sequence
    int Nibbles;
    int TotalBits;		//Total Bits in Buffer
};

bool TransitionalBlock;
bool LastBlockOfBundle = false;

bool FirstLitOfBlock = false;                   //If true, this is the first block of next file in same buffer, Lit Selector Bit NOT NEEEDED
bool NextFileInBuffer = false;                   //Indicates whether the next file is added to the same buffer

int BlockUnderIO = 0;
unsigned char AdLoPos{}, AdHiPos{};

const int MatchSelector = 1;
const int LitSelector = 0;

//Match offsets - stored 0-based
const int MaxFarOffset = 65536;                     //0-based (257-65536, stored as 256-65535)
const int MaxNearOffset = 256;                      //0-based (1-256, stored as 0-255)
const int MaxShortOffset = 64;                      //0-based (1-64, stored as 0-63)

int MaxOffset = MaxNearOffset * 3;                  //3 is most optimal for size, loading and disk creating speed

//Match lengths
const unsigned char MaxLongLen = 255;               //1-based (32-255, stored the same)
const unsigned char MaxMidLen = 31;                 //1-based (2-31, stored the same)
const unsigned char MaxShortLen = 3 + 1;            //0-based (2-4, stored 1-3), cannot be 0 because it is preserved for EndTag

const unsigned char NearLongMatchTag = 0x84;
const unsigned char FarLongMatchTag = 0x04;
const unsigned char EndOfBundleTag = 0x00;
const unsigned char EndOfBlockTag = 0x00;
const unsigned char NextFileTag = 0x80;

const int MaxLitLen = 16;

int MatchBytes = 0;
int MatchBits = 0;
int LitBits = 0;
int MLen = 0;
int MOff = 0;
int LitCnt = -1;

const int MaxBits = 256 * 8;
const int MaxLitPerBlock = 251 - 1;                 //Maximum number of literals that fits in a block, LitCnt is 0-based
                                                    //256 - (AdLo, AdHi , 1 Bit, 1 Nibble, Number of Lits)

//Private Seq() As Sequence           'Sequence array, to find the best sequence

//Private SL(), SO(), NL(), NO(), FL(), FO(), FFL(), FFO() As Integer   'Short, Near, and Far Lengths and Offsets

int SI{};                                           //Sequence array index, Offset max and min for far matches
int LitSI{};                                        //Sequence array index of last literal sequence
int StartPtr{};

//Public PartialFileIndex, PartialFileOffset As Integer
int CurrentFileIndex{};
int ReferenceFileStart{};
int ReferenceUnderIO{};

//----------------------------------------------------------------------------------------------------------------------------------------------------------------
int CheckIO(int Offset, int NextFileUnderIO) {
    
    Offset += PrgAdd;

    if (Offset < 256)           //Are we loading to the Zero Page? If yes, we need to signal it by adding IO Flag
    {
        return 1;
    }
    else if (NextFileUnderIO > -1)
    {
        return ((Offset >= 0xd000) && (Offset <= 0xdfff) && (NextFileUnderIO == 1)) ? 1 : 0;
    }
    else
    {
        return  ((Offset >= 0xd000) && (Offset <= 0xdfff) && (FileUnderIO == true)) ? 1 : 0;
    }

}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

void AddBits(int Bit, unsigned char BCnt) {

    for (int i = BCnt - 1; i >= 0; i--)
    {
        if (BitPos < 0)
        {
            BitPos += 8;
            BitsLeft = 8;
            BitPtr = BytePtr--;     //New BitPtr pos and BytePtr pos
        }
        if ((Bit & (1 << i)) != 0)
        {
            Buffer[BitPtr] |= (1 << BitPos);
        }

        BitPos--;
        BitsLeft--;
        if (BitPos == 0)
        {
            if (Buffer[BitPtr] % 2 == 1)
            {
                BitPos = -1;
                BitsLeft = 0;
            }
        }
        //Very first BitPtr in buffer has a 1 in BitPos=0 (Token Bit) -> Skip It!!!
        //If (BitPtr = 0) And (BitPos = 0) Then BitPos = -1
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

bool SequenceFits(int BytesToAdd, int BitstoAdd, int SequenceunderIO) {

}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

bool CloseFile() {

    //ADDS NEXT FILE TAG TO BUFFER

    //4-5 bytes and 1-2 bits needed for NextFileTag, Address Bytes and first Lit byte (+1 more if UIO)
    //BYTES NEEDED: (1)End Tag + (2)AdLo + (3)AdHi + (4)1st Literal +/- (5)I/O FLAG of NEW FILE's 1st literal
    //BUG reported by Raistlin/G*P

    int BytesNeededForNextFile = 4 + CheckIO(PrgLen - 1, -1);

    //THE FIRST LITERAL BYTE WILL ALSO NEED A LITERAL BIT
    //DO NOT check whether Match Bit is needed for new file - will be checked in Sequencefits()
    //BUG reported by Visage/Lethargy

    int BitsNeededForNextFile = 1;

    //Type selector bit  (match vs literal) is not needed, the first byte of a file is always literal
    //So this is the literal length bit: 0 - 1 literal, 1 - more than 1 literals, would also need a Nibble...
    //...but here we only need to be able to fit 1 literal byte

    NextFileInBuffer = true;

    if (SequenceFits(BytesNeededForNextFile, BitsNeededForNextFile,0))
    {
        //Buffer has enough space for New File Match Tag and New File Info and first Literal byte (and I/O flag if needed)

        //If last sequence was a match (no literals) then add a match bit
        if ((MLen > 0) || (LitCnt == -1)) AddBits(MatchSelector, 1);
            
        Buffer[BytePtr--] = NextFileTag;                            //Then add New File Match Tag
        FirstLitOfBlock = true;
    }
    else
    {
        //Next File Info does not fit, so close buffer, next file will start in new block
        CloseBuffer();
    }

    return true;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

bool CloseBuffer() {

    return true;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

void PackFile(int Index) {
    
    //----------------------------------------------------------------------------------------------------------
    //PROCESS FILE
    //----------------------------------------------------------------------------------------------------------

    //MaxOffset = MaxNearOffset * 3;

    FileUnderIO = Prgs[Index].FileIO;
    PrgAdd = Prgs[Index].iFileAddr;
    PrgLen = Prgs[Index].iFileLen;

    int* SL = new int[PrgLen - 1]{};
    int* SO = new int[PrgLen - 1]{};
    int* NL = new int[PrgLen - 1]{};
    int* NO = new int[PrgLen - 1]{};
    int* FL = new int[PrgLen - 1]{};
    int* FO = new int[PrgLen - 1]{};
    int* FFL = new int[PrgLen - 1]{};
    int* FFO = new int[PrgLen - 1]{};

    sequence* Seq = new sequence[PrgLen]{};

    Seq[PrgLen - 1].Len = -1;
    //Seq[PrgLen - 1].TotalBits = 10;

    //----------------------------------------------------------------------------------------------------------
    //SEARCH REFERENCE FILES FOR FAR MATCHES
    //----------------------------------------------------------------------------------------------------------

    CurrentFileIndex = Index;

    for (int RefIndex  = 0; RefIndex < VFiles.size(); RefIndex++)
    {
        //SearchVirtualFile(RefIndex);
    }

    for (int PIndex = 0; PIndex < PartialFileIndex; PIndex++)
    {
    }

    if (PartialFileIndex > -1)
    {
    }
/*
    CurrentFileIndex = FileIndex

    If Packer = PackerTypes.Better Then
        For I As Integer = 0 To VFiles.Count - 1
            ReferenceFile = VFiles(I).ToArray
            ReferenceFileStart = Convert.ToInt32(VFileAddrA(I), 16)

            SearchVirtualFile(PrgLen - 1, I, ReferenceFileStart + ReferenceFile.Length - 1, ReferenceFileStart + 1)
        Next

        For I As Integer = 0 To PartialFileIndex - 1
            ReferenceFile = Prgs(I).ToArray
            ReferenceFileStart = Convert.ToInt32(FileAddrA(I), 16)

            SearchReferenceFile(PrgLen - 1, I, ReferenceFileStart + ReferenceFile.Length - 1, ReferenceFileStart + 1)
        Next

        If PartialFileIndex > -1 Then
            ReferenceFile = Prgs(PartialFileIndex).ToArray
            ReferenceFileStart = Convert.ToInt32(FileAddrA(PartialFileIndex), 16)

            'Search partial file from offset to end of file
            SearchReferenceFile(PrgLen - 1, PartialFileIndex, ReferenceFileStart + ReferenceFile.Length - 1, ReferenceFileStart + PartialFileOffset + 1)
        End If
    End If
*/

}

//----------------------------------------------------------------------------------------------------------------------------------------------------------
