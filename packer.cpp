#include "common.h"

using namespace std;
using namespace std::filesystem;

struct sequence {
    int Len;		//Length of the sequence in bytes (0 based)
    int Off;		//Offset of Match sequence in bytes (1 based), 0 if Literal Sequence
    int Nibbles;
    int TotalBits;		//Total Bits in Buffer
};

template <class T>
T FastMin(const T& left, const T& right)
{
    return left < right ? left : right;
}

sequence *Seq;

bool TransitionalBlock;
bool LastBlockOfBundle = false;

bool FirstLitOfBlock = false;                       //If true, this is the first block of next file in same buffer, Lit Selector Bit NOT NEEEDED
bool NextFileInBuffer = false;                      //Indicates whether the next file is added to the same buffer

int BlockUnderIO = 0;
unsigned char AdLoPos{}, AdHiPos{};

const int MatchSelector = 1;
const int LitSelector = 0;
const int ShortLitSelector = 0;
const int MidLitSelector = 1;


//Match offsets - stored 0-based
//const int MaxFarOffset = 65536;                   //0-based (257-65536, stored as 256-65535) UNUSED
const int MaxNearOffset = 256;                      //0-based (1-256, stored as 0-255)
const int MaxShortOffset = 64;                      //0-based (1-64, stored as 0-63)

int MaxOffset = MaxNearOffset * 8;                  //3 is most optimal for size, loading and disk creating speed, 8 is the new standard!!!

//Match lengths
const int MaxLongLen = 255;                         //1-based (32-255, stored the same)
const int MaxMidLen = 31;                           //1-based (2-31, stored the same)
const int MaxFarMidLen = 31;                        //(MaxMidLen * 2) + 1; minimal difference in efficacy, big difference in speed...
const int MaxShortLen = 3 + 1;                      //0-based (2-4, stored 1-3), cannot be 0 because it is preserved for EndTag

const unsigned char NearLongMatchTag = 0x84;
const unsigned char FarLongMatchTag = 0x04;
const unsigned char EndOfBundleTag = 0x00;
//const unsigned char EndOfBlockTag = 0x00;         //UNUSED
const unsigned char NextFileTag = 0x80;

const int MaxLitsPerNibble = 16;

int MatchBytes = 0;
int MatchBits = 0;
int LitBits = 0;
int MLen = 0;
int MOff = 0;
int LitCnt = -1;

//const int MaxBits = 256 * 8;                      //UNUSED
const int MaxLitPerBlock = 251 - 1;                 //Maximum number of literals that fits in a block, LitCnt is 0-based
                                                    //256 - (AdLo, AdHi , 1 Bit, 1 Nibble, Number of Lits)

int SI{};                                           //Sequence array index, Offset max and min for far matches
int LitSI{};                                        //Sequence array index of last literal sequence
int StartPtr{};

int CurrentFileIndex{};
int ReferenceFileIndex{};
int ReferenceFileStart{};
int ReferenceUnderIO{};

array<unsigned char, 256> Buffer;

int *SO, *NO, *FO, * FSO, * FNO, *FFO;

unsigned char* SL, * NL, * FL, * FSL, * FNL, * FFL;

int NibblePtr, BitPos, BitsLeft;
unsigned char LastByte;

//----------------------------------------------------------------------------------------------------------------------------------------------------------
/*
void WriteBinaryFile(const string& FileName, unsigned char* Buffer, streamsize Size) {

    ofstream myFile(FileName, ios::out | ios::binary);
    myFile.write((char*)&Buffer[0], Size);

}
*/
//----------------------------------------------------------------------------------------------------------------------------------------------------------------

bool UpdateByteStream()
{
    if (BufferCnt > (SectorsPerDisk - BlocksUsedByPlugin))
    {
        cerr << "***CRITICAL***\tUnable to add bundle. Disk is full!!!\n";
        return false;
    }

    memcpy(&ByteSt[(BufferCnt - 1) * 256], &Buffer[0], 256 * sizeof(Buffer[0]));

    //for (int i = 0; i < 256; i++)
    //{
        //ByteSt[((BufferCnt - 1) * 256) + i] = Buffer[i];
    //}

    return true;
}

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
        return ((Offset >= 0xd000) && (Offset <= 0xdfff) && (FileUnderIO)) ? 1 : 0;
    }

}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

void ResetBuffer()
{
    //Buffer.fill(0);     //Reset Buffer

    //Both of these would work:
    memset(Buffer.data(), 0, Buffer.size() * sizeof(Buffer[0]));    //memset() is faster than fill()
    //fill(Buffer.begin(), Buffer.end(), 0);  //New empty buffer

    //Initialize variables

    FilesInBuffer = 1;

    BitPos = 7;             //Reset Bit Position Counter (counts 8 bits backwards: 7 -> 0)

    BitPtr = 0;
    Buffer[BitPtr] = 0x01;
    BitsLeft = 7;
    NibblePtr = 0;
    BytePtr = 255;

    //DO NOT RESET LitCnt HERE!!! It is needed for match tag check
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

void AddNibble(int Bit)
{
    if (NibblePtr == 0)
    {
        NibblePtr = BytePtr--;                  //Also update BytPtr here
        Buffer[NibblePtr] = Bit;

    }
    else
    {
        Buffer[NibblePtr] += Bit * 16;
        NibblePtr = 0;
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

void AddLitBits(int Lits)
{

    //We are never adding more than MaxLitBit number of bits here

    if (Lits == -1)
    {
        return;         //Is this needed? We only call this routine with LitCnt>-1
    }

    if (!FirstLitOfBlock)
    {
        AddBits(LitSelector, 1);               //Add Literal Selector if this is not the first (Literal) byte in the buffer
    }
    else
    {
        FirstLitOfBlock = false;
    }

    if (Lits == 0)
    {
        AddBits(ShortLitSelector, 1);              //Add Literal Length Selector 0 - read no more bits
    }
    else if (Lits < MaxLitsPerNibble)
    {   //(Lits == 1 -> (MaxLitsPerNibble-1))
        AddBits(MidLitSelector, 1);              //Add Literal Length Selector 1 - read 4 more bits
        AddNibble(Lits);                //Add Literal Length: 01-0f, 4 bits (0001-1111)
    }
    else
    {   //(Lits == MaxLitsPerNibble)
        AddBits(MidLitSelector, 1);              //Add Literal Length Selector 1 - read 4 more bits
        AddNibble(0);                   //Add Literal Length: 0, 4 bits (0000) - we will have a longer literal sequence
    }

    //DO NOT RESET LitCnt HERE!!!

}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

void AddLitSequence()
{

    if (LitCnt == -1)
    {
        return;
    }

    int Lits = LitCnt;

    if (Lits >= MaxLitsPerNibble)
    {
        AddLitBits(MaxLitsPerNibble);
        //Then add number of literals as a byte
        Buffer[BytePtr--] = Lits;   //Also update BytePtr here
    }
    else
    {
        //Add literal bits for 1-15 literals
        AddLitBits(Lits);
    }

    //Then add literal bytes
    //for (int i = 0; i <= Lits; i++)
    //{
        //Buffer[BytePtr - i] = Prgs[CurrentFileIndex].Prg[LitSI - i];
    //}
    memcpy(&Buffer[BytePtr - Lits], &Prgs[CurrentFileIndex].Prg[LitSI - Lits], (Lits + 1) * sizeof(Buffer[0]));

    BytePtr -= Lits + 1;
    LitSI -= Lits + 1;
    Lits--;

    //TotLits++;

    //DO NOT RESET LITCNT HERE, IT IS NEEDED AT THE SUBSEQUENT MATCH TO SEE IF A MATCHTAG IS NEEDED!!!

}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

void AddMatchBit()
{
    if (LitCnt == -1)
    {
        AddBits(MatchSelector, 1);   //Last Literal Length was -1, we need the Match selector bit (1)
    }

    LitCnt = -1;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

void AddShortMatch()
{
    //				X-3	     X-2	  X-1      X		    OFFSET (STORED AS)  	LENGTH (STORED AS)
    //	SHORT:                                 ooooooLL	    $01-$40 ($00-$3F)	    $02-$04 ($01-$03)

    AddMatchBit();

    Buffer[BytePtr--] = ((MOff - 1) * 4) + (MLen - 1);  //Also update BytePtr here

}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

void AddNearMidMatch()
{
    //				X-3	     X-2	  X-1      X		    OFFSET (STORED AS)  	LENGTH (STORED AS)
    //	NEAR MID:	                  oooooooo 1LLLLL00	    $01-$100 ($00-$FF)   	$02-$1F ($02-$1F)

    AddMatchBit();

    Buffer[BytePtr--] = 0x80 + (MLen * 4);  //Length of match (#$02-#$1f))
    Buffer[BytePtr--] = MOff - 1;           //Offset (1-256, stored as 0-255)

}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

void AddFarMidMatch()
{
    //				X-3	     X-2	  X-1      X		    OFFSET (STORED AS)  	    LENGTH (STORED AS)
    //	FAR MID:		     oooooooo HHHHHHHH 0LLLLL00	    $0101-$FFFF ($0100-$FFFE) 	$03-$1F ($03-$1F)

    AddMatchBit();

    Buffer[BytePtr--] = MLen * 4;           //Length of match (#$02-#$1f))
    Buffer[BytePtr--] = (MOff - 1) / 256;
    Buffer[BytePtr--] = (MOff - 1) % 256;

}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

void AddNearLongMatch()
{
    //				X-3	     X-2	  X-1      X		    OFFSET (STORED AS)  	LENGTH (STORED AS)
    //	NEAR LONG:	         oooooooo LLLLLLLL 10000100	    $01-$100 ($00-$FF)	    $20-$FF ($20-$FF)

    AddMatchBit();

    Buffer[BytePtr--] = NearLongMatchTag;   //Near Long Match Tag = 0x84
    Buffer[BytePtr--] = MLen;
    Buffer[BytePtr--] = MOff - 1;

}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

void AddFarLongMatch()
{
    //				X-3	     X-2	  X-1      X		    OFFSET (STORED AS)  	LENGTH (STORED AS)
    //	FAR LONG:   oooooooo LLLLLLLL HHHHHHHH 00000100	    $0100-$FFFF	    	    $20-$FF ($20-$FF)

    AddMatchBit();

    Buffer[BytePtr--] = FarLongMatchTag;    //Far Long Match Tag = 0x04
    Buffer[BytePtr--] = (MOff - 1) / 256;
    Buffer[BytePtr--] = MLen;
    Buffer[BytePtr--] = (MOff - 1) % 256;

}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

int CalcLitBits(int Lits)
{
    if (Lits == -1)
    {
        LitBits = 0;                        //Lits = -1		no literals, 0 bit
    }
    else if (Lits == 0)
    {
        LitBits = 2;                        //Lits = 0		one literal, 1 bit
    }
    else if (Lits < MaxLitsPerNibble)
    {
        LitBits = 6;                        //Lits = 1-15	2-16 literals, 5 bits
    }
    else
    {
        LitBits = 14;                       //Lits = 15-250	17-251 literals, 13 bits
    }

    //BUGFIX: The very first literal sequence of a file or block does not need a type selector bit
    //As we always start with at least one literal byte
    if ((FirstLitOfBlock) && (LitBits > 0))
    {
        LitBits--;
    }

    return LitBits;

}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

void CalcMatchBytesAndBits(int Length, int Offset)
{

    if ((Length <= MaxShortLen) && (Offset <= MaxShortOffset))
    {
        MatchBytes = 1;
    }
    else if ((Length <= MaxMidLen) && (Offset <= MaxNearOffset))
    {
        MatchBytes = 2;
    }
    else if ((Length > MaxMidLen) && (Offset > MaxNearOffset))
    {
        MatchBytes = 4;
    }
    else
    {
        MatchBytes = 3;
    }

    MatchBits = (LitCnt == -1) ? 1 : 0;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

void CheckLitSeq(int Pos)
{
    //Continue previous Lit sequence or start new sequence
    LitCnt = (Seq[Pos].Off == 0) ? Seq[Pos].Len : -1;

    //Calculate literal bits for a presumtive LitCnt+1 value
    LitBits = ((LitCnt + 1) / MaxLitPerBlock) * 13;

    int LC = (LitCnt + 1) % MaxLitPerBlock;

    if (LC == 0)
    {
        LitBits++;              //Lits = 0	    1 literal       1 bit
    }
    else if (LC < MaxLitsPerNibble)
    {
        LitBits += 5;           //Lits = 1-15	2-16 literals   5 bits
    }
    else
    {
        LitBits += 13;          //Lits = 16-250	17-251 literals 13 bits
    }

    //LITERALS ARE ALWAYS FOLLOWED BY MATCHES, SO TYPE SELECTOR BIT IS NOT NEEDED AFTER LITERALS AT ALL

    int TotBits = Seq[Pos - (LitCnt + 1)].TotalBits + LitBits + ((LitCnt + 2) * 8);

    //See if total bit count is less than best version and save it to sequence at Pos+1 (position is 1 based)
    if (TotBits < Seq[Pos + 1].TotalBits)
    {
        Seq[Pos + 1].Len = LitCnt + 1;      //LitCnt is 0 based, LitLen is 0 based
        Seq[Pos + 1].Off = 0;               //An offset of 0 marks a literal sequence, match offset is 1 based
        Seq[Pos + 1].Nibbles = Seq[Pos - (LitCnt + 1)].Nibbles + ((LitBits > 1) ? 1 : 0);
        Seq[Pos + 1].TotalBits = TotBits;
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

void CheckMatchSeq(int SeqLen, int SeqOff, int Pos)
{
    //If this is a far match then min len = 3, otherwise min len = 2
    int MinLen = (SeqOff > MaxNearOffset) ? 3 : 2;

    //Check all possible lengths
    for (int L = SeqLen; L >= MinLen; L--)
    {
        //Calculate MatchBits
        if ((L <= MaxShortLen) && (SeqOff <= MaxShortOffset))
        {
            MatchBits = 9;
        }
        else if ((L <= MaxMidLen) && (SeqOff <= MaxNearOffset))
        {
            MatchBits = 17;
        }
        else if ((L > MaxMidLen) && (SeqOff > MaxNearOffset))
        {
            MatchBits = 33;
        }
        else
        {
            MatchBits = 25;
        }

        //Calculate total bit count, independently of nibble status
        int TotBits = Seq[Pos + 1 - L].TotalBits + MatchBits;

        if (TotBits < Seq[Pos + 1].TotalBits)
        {
            //If better, update best version
            Seq[Pos + 1].Len = L;            //MatchLen is 1 based
            Seq[Pos + 1].Off = SeqOff;       //Off is 1 based
            Seq[Pos + 1].Nibbles = Seq[Pos + 1 - L].Nibbles;
            Seq[Pos + 1].TotalBits = TotBits;
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

bool SequenceFits(int BytesToAdd, int BitsToAdd, int SequenceUnderIO) {

    int BytesFree = BytePtr;    //1,2,3,...,BytePtr-1,BytePtr

    //If this is a transitional block (including block 0 on disk) then we need 1 byte for block count (will be overwritten by Close Byte
    if ((TransitionalBlock) || (BufferCnt == 0))
    {
        BytesFree--;
    }

    int BitsFree = BitsLeft;    //0-8

    //Add IO Byte ONLY if this is the first sequence in the block that goes under IO
    BytesToAdd += ((BlockUnderIO == 0) && (SequenceUnderIO == 1)) ? 1 : 0;

    //Check if we have literal sequences >1 which have bits stored in nibbles
    //BUGFIX: first literal sequence of a block/file has one less bits than any other seuqences, so comparision must be made with 5 instead of 6
    if (BitsToAdd >= 5)
    {
        if (NibblePtr == 0)     //If NibblePtr Points at buffer(0) then we need to add 1 byte for a new NibblePtr position in the buffer
        {
            BytesFree--;
        }
        BitsToAdd -= 4;         //4 bits less to store in the BitPtr
    }

    //Add Match/Close Bit if the last sequence was a match
    BitsToAdd += (MLen > 0) ? 1 : 0;

    BytesToAdd += BitsToAdd / 8;
    BitsToAdd = BitsToAdd % 8;

    if (BitsFree - BitsToAdd < 0)
    {
        BytesToAdd++;
    }

    //Check if sequence will fit within block size limits
    if (BytesFree >= BytesToAdd)
    {
        if ((BlockUnderIO == 0) && (SequenceUnderIO == 1))
        {
            //This is the first byte in the block that will go UIO, so lets update the buffer to include the IO flag
            for (int i = BytePtr; i <= AdHiPos; i++)
            {
                Buffer[i - 1] = Buffer[i];                  //Move all data to the left in buffer, including AdHi
            }
            Buffer[AdHiPos] = 0;                            //IO Flag to previous AdHi Position
            BytePtr--;                                      //Update BytePtr to next empty position in buffer
            //Sparkle64k BUG - Don't move BitPtr and NibblePtr if they are in their startup positions(0 or outside file address bytes)
            if ((NibblePtr > 0) && (NibblePtr < AdHiPos))
            {
                NibblePtr--;                                //Only update Nibble Pointer if it does not point to Byte(0)
            }
            if ((BitPtr > 0) && (BitPtr < AdHiPos))
            {
                BitPtr--;                                   //BitPtr also needs to be moved BUT ONLY IF > 0 - BUG reported by Raistlin/G*P
            }
            AdHiPos--;                                          //Update AdHi Position in Buffer
            BlockUnderIO = 1;                                   //Set BlockUnderIO Flag
        }

        return true;
    }
    else
    {
        return false;
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

void FindFarMatches(int RefIndex, int SeqMaxIndex, int SeqMinIndex, int RefMaxAddress, int RefMinAddress)
{
    //unsigned char* Prg8 = &Prgs[CurrentFileIndex].Prg[0];
    //unsigned char* Ref8 = &Prgs[RefIndex].Prg[0];
    //uint16_t* Prg16 = (uint16_t*)&Prg8[0];
    //uint16_t* Ref16 = (uint16_t*)&Ref8[0];

    //bool MatchFound = false;

    //cout <<"\n" <<Prgs[CurrentFileIndex].FileName << "\t" << hex << Prg8[0] << "\t" << Prg16[0] << "\t" << Prgs[RefIndex].FileName << "\t" << Ref8[0] << "\t" << Ref16[0] << "\n";

    if (!FileUnderIO)
    {
        if ((PrgAdd + SeqMinIndex >= 0xd000) && (PrgAdd + SeqMinIndex <= 0xdfff))
        {
            if (PrgAdd + SeqMaxIndex <= 0xdfff)
            {
                //The entire file segment is under I/O -> skip it
                return;
            }
            else
            {
                //Not the entire file segment is under I/O -> only check the part that is not
                SeqMinIndex = 0xe000 - PrgAdd;

            }
        }
        else if ((PrgAdd + SeqMaxIndex >= 0xd000) && (PrgAdd + SeqMaxIndex <= 0xdfff))
        {
            SeqMaxIndex = 0xcfff - PrgAdd;
        }
    }

    //----------------------------------------------------------------------------------------------------------
    //FIND LONGEST SHORT AND NEAR MATCHES FOR EACH POSITION, AND FAR MATCHES WITH OFFSET < MAX. 1024
     //----------------------------------------------------------------------------------------------------------
    int RefMinAddressIndex = RefMinAddress - ReferenceFileStart;
    int RefMaxAddressIndex = RefMaxAddress - ReferenceFileStart;

    int OffsetBase = (ReferenceFileStart >= PrgAdd) ? ReferenceFileStart - PrgAdd : ReferenceFileStart + 0x10000 - PrgAdd;

    //Pos = Min>0 to Max value, direction of execution is arbitrary (could be Max to Min>0 Step -1)
    for (int Pos = SeqMaxIndex; Pos >= SeqMinIndex; Pos--)  //Pos cannot be 0, Prg(0) is always literal as there is always 1 byte left
    {
        unsigned char PrgValAtPos = Prgs[CurrentFileIndex].Prg[Pos];

        //int ShortMinO = RefMinAddressIndex;
        //int ShortMaxO = min(max(Pos + MaxShortOffset, OffsetBase + RefMinAddressIndex), OffsetBase + RefMaxAddressIndex) - OffsetBase;
/*
        for (int O = ShortMaxO; O >= ShortMinO; O--)
        {
            //Check if first byte matches at offset, if not go to next offset
            if (PrgValAtPos == Prgs[RefIndex].Prg[O])
            {
                //int MaxLL = FastMin(FastMin(Pos + 1, MaxLongLen),O - RefMinAddressIndex + 1);   //MaxLL = 255 or less
                int MaxLL = min(min(Pos + 1, MaxLongLen), O - RefMinAddressIndex + 1);   //MaxLL = 255 or less
                int MaxSL = min(min(Pos + 1, MaxShortLen), O - RefMinAddressIndex + 1);

                for (int L = 1; L <= MaxLL; L++)
                {
                    if ((L == MaxLL) || (Prgs[CurrentFileIndex].Prg[Pos - L] != Prgs[RefIndex].Prg[O - L]))
                    {
                        //Find the first position where there is NO match -> this will give us the absolute length of the match
                        //L=MatchLength + 1 here
                        if (L > 2)
                        {
                            if (FSL[Pos] < min(MaxSL, L))
                            {
                                FSL[Pos] = min(MaxSL, L); //If(L > MaxShortLen, MaxShortLen, L)   'Short matches cannot be longer than 4 bytes
                                FSO[Pos] = O - Pos + OffsetBase;                        //Keep Offset 1-based
                            }
                            if (FNL[Pos] < L)   //Allow short (2-byte) Mid Matches
                            {
                                FNL[Pos] = L;
                                FNO[Pos] = O - Pos + OffsetBase;
                            }
                        }
                        break;
                    }
                }
                if ((FSL[Pos] == MaxSL) && (FNL[Pos] == MaxLL))
                {
                    break;
                }
            }
        }
*/
//int NearMinO = min(RefMinAddressIndex, ShortMaxO);
//int NearMaxO = min(max(Pos + MaxNearOffset, OffsetBase + RefMinAddressIndex), OffsetBase + RefMaxAddressIndex) - OffsetBase;
/*
        for (int O = NearMaxO; O >= NearMinO; O--)
        {
            //Check if first byte matches at offset, if not go to next offset
            if (PrgValAtPos == Prgs[RefIndex].Prg[O])
            {
                //int MaxLL = FastMin(FastMin(Pos + 1, MaxLongLen),O - RefMinAddressIndex + 1);   //MaxLL = 255 or less
                int MaxLL = min(min(Pos + 1, MaxLongLen), O - RefMinAddressIndex + 1);   //MaxLL = 255 or less

                for (int L = 1; L <= MaxLL; L++)
                {
                    if ((L == MaxLL) || (Prgs[CurrentFileIndex].Prg[Pos - L] != Prgs[RefIndex].Prg[O - L]))
                    {
                        //Find the first position where there is NO match -> this will give us the absolute length of the match
                        //L=MatchLength + 1 here
                        if (L > 2)
                        {
                            if (FNL[Pos] < L)   //Allow short (2-byte) Mid Matches
                            {
                                FNL[Pos] = L;
                                FNO[Pos] = O - Pos + OffsetBase;
                            }
                        }
                        break;
                    }
                }
                //If far matches maxed out, we can leave the loop and go to the next Prg position
                if (FNL[Pos] == MaxLL)
                {
                    break;
                }
            }
        }
*/
//int FarMinO = max(RefMinAddressIndex, NearMaxO);
        int BestLength = FFL[Pos];
        int BestOffset = FFO[Pos];

        for (int O = RefMaxAddressIndex; O >= RefMinAddressIndex; O--)
        {
            //Check if first byte matches at offset, if not go to next offset
            if (PrgValAtPos == Prgs[RefIndex].Prg[O])
            {
                int MaxLL = min(min(Pos + 1, MaxLongLen), O - RefMinAddressIndex + 1);   //MaxLL = 255 or less

                for (int L = 1; L <= MaxLL; L++)
                {
                    if ((L == MaxLL) || (Prgs[CurrentFileIndex].Prg[Pos - L] != Prgs[RefIndex].Prg[O - L]))
                    {
                        //Find the first position where there is NO match -> this will give us the absolute length of the match
                        //L=MatchLength + 1 here
                        if (L > 2)
                        {
                            if (BestLength < L)
                            {
                                BestLength = L;
                                BestOffset = O - Pos + OffsetBase;
                                //MatchFound = true;
                            }
                        }
                        break;
                    }
                }
                //If far matches maxed out, we can leave the loop and go to the next Prg position
                if (BestLength >= min(min(Pos + 1, MaxFarMidLen), O - RefMinAddressIndex + 1))
                {
                    break;
                }
            }
        }

        FFL[Pos] = BestLength;
        FFO[Pos] = BestOffset;
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

void SearchReferenceFile(int PrgMaxIndex, int RefIndex, int RefMaxAddress, int RefMinAddress)
{
    //PrgMaxIndex = relative address within Prg
    //RefMaxAddress, RefMinAddress = absolute addresses within reference file
    if (!Prgs[RefIndex].FileIO)
    {
        //Reference file is not under I/O, check if it is IN I/O
        if (((RefMinAddress >= 0xd000) && (RefMinAddress < 0xe000)) || ((RefMaxAddress >= 0xd000) && (RefMaxAddress < 0xe000)))
        {
            //This reference file is IN the I/O registers, SKIP anything between $d000-$dfff
            if (RefMinAddress + 1 < 0xd000)
            {
                //Search from start to $cfff
                FindFarMatches(RefIndex, PrgMaxIndex, 1, 0xcfff, RefMinAddress + 1);
            }
            if (RefMaxAddress > 0xe000)
            {
                //Search from $e000 to end of file
                FindFarMatches(RefIndex, PrgMaxIndex, 1, RefMaxAddress, 0xe000 + 1);
            }
        }
        else
        {
            FindFarMatches(RefIndex, PrgMaxIndex, 1, RefMaxAddress, RefMinAddress + 1);
        }
    }
    else
    {
        FindFarMatches(RefIndex, PrgMaxIndex, 1, RefMaxAddress, RefMinAddress + 1);
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

void FindMatches(int SeqHighestIndex, int SeqLowestIndex, bool FirstRun)
{
    //----------------------------------------------------------------------------------------------------------
    //                                      TODO: TRY REVERSE VECTORS!!!
    //----------------------------------------------------------------------------------------------------------
    //FIND LONGEST SHORT AND NEAR MATCHES FOR EACH POSITION, AND FAR MATCHES WITH OFFSET < MAX. 1024
    //----------------------------------------------------------------------------------------------------------

    int CurrentMaxL;
    int L = 0;

    for (int Pos = SeqHighestIndex; Pos >= SeqLowestIndex; Pos--)   //Pos cannot be 0, Prg(0) is always literal as it is always 1 byte left
    {
        //Offset goes from 1 to max offset (cannot be 0)
        int MaxO = min(MaxOffset, SeqHighestIndex - Pos);
        //Match length goes from 1 to max length
        int MaxLL = min(Pos, MaxLongLen);
        int MaxSL = min(Pos, MaxShortLen);

        if ((FirstRun) || (FO[Pos] > MaxO) || (NO[Pos] > MaxO) || (SO[Pos] > MaxO))
        {
            //Only run search for this Pos if this is the first pass or the previously found match has an offset beyond block
            if (SO[Pos] > MaxO)
            {
                SO[Pos] = 0;
                SL[Pos] = 0;
            }
            if (NO[Pos] > MaxO)
            {
                NO[Pos] = 0;
                NL[Pos] = 0;
            }
            if (FO[Pos] > MaxO)
            {
                FO[Pos] = 0;
                FL[Pos] = 0;
            }

            unsigned char PrgValAtPos = Prgs[CurrentFileIndex].Prg[Pos];

            int ShortMaxO = min(MaxShortOffset, MaxO);

            int BestSL = SL[Pos];
            int BestSO = SO[Pos];

            int BestNL = NL[Pos];
            int BestNO = NO[Pos];

            for (int O = 1; O <= ShortMaxO; O++)
            {
                //If both short and long matches maxed out, we can leave the loop and go to the next Prg position
                if ((BestNL == MaxLL) && (BestSL == MaxSL))
                {
                    break;
                }
                //Check if first byte matches at offset, if not go to next offset
                if (PrgValAtPos == Prgs[CurrentFileIndex].Prg[Pos + O])
                {
                    L = 1;
                    do
                    {
                        if (Prgs[CurrentFileIndex].Prg[Pos - L] != Prgs[CurrentFileIndex].Prg[Pos + O - L])
                        {
                            break;
                        }
                        else
                        {
                            L++;
                        }
                    } while (L <= MaxLL);

                    if (L > 1)
                    {
                        if (BestSL < (CurrentMaxL = min(MaxSL, L)))
                        {
                            BestSL = CurrentMaxL; //If(L > MaxShortLen, MaxShortLen, L)   'Short matches cannot be longer than 4 bytes
                            BestSO = O;                        //Keep Offset 1-based
                        }
                        if (BestNL < (CurrentMaxL = min(L, MaxLL)))
                        {
                            BestNL = CurrentMaxL;
                            BestNO = O;
                        }
                    }
                }
            }

            SL[Pos] = BestSL;
            SO[Pos] = BestSO;

            int NearMaxO = min(MaxNearOffset, MaxO);

            for (int O = ShortMaxO + 1; O <= NearMaxO; O++)
            {
                //If both short and long matches maxed out, we can leave the loop and go to the next Prg position
                if (BestNL == MaxLL)
                {
                    break;
                }
                //Check if first byte matches at offset, if not go to next offset
                if (PrgValAtPos == Prgs[CurrentFileIndex].Prg[Pos + O])
                {
                    L = 1;
                    do {
                        if (Prgs[CurrentFileIndex].Prg[Pos - L] != Prgs[CurrentFileIndex].Prg[Pos + O - L])
                        {
                            break;
                        }
                        else
                        {
                            L++;
                        }
                    } while (L <= Pos);

                    if (L > (BestSL > 1 ? BestSL : 1))    // max(1, SL[Pos]))     //Only check match lengths > SL[Pos]
                    {
                        if (BestNL < (CurrentMaxL = min(L, MaxLL)))
                        {
                            BestNL = CurrentMaxL;
                            BestNO = O;
                        }
                        int I = Pos;
                        while ((L >= (CurrentMaxL = min(I, MaxLL))) && (L-- > 1))
                        {
                            NL[I] = CurrentMaxL;
                            NO[I--] = O;
                        }
                    }
                }
            }

            NL[Pos] = BestNL;
            NO[Pos] = BestNO;

            if (BestNL != MaxLL)
            {

                int BestFL = FL[Pos];
                int BestFO = FO[Pos];

                for (int O = NearMaxO + 1; O <= MaxO; O++)
                {
                    //If both short and long matches maxed out, we can leave the loop and go to the next Prg position
                    if (BestFL == MaxLL)
                    {
                        break;
                    }
                    //Check if first byte matches at offset, if not go to next offset
                    if (PrgValAtPos == Prgs[CurrentFileIndex].Prg[Pos + O])
                    {
                        L = 1;
                        do {
                            if (Prgs[CurrentFileIndex].Prg[Pos - L] != Prgs[CurrentFileIndex].Prg[Pos + O - L])
                            {
                                break;
                            }
                            else
                            {
                                L++;
                            }
                        } while (L <= Pos);

                        if (L > (BestNL > 2 ? BestNL : 2))    // max(2, NL[Pos])) //Only check lengths > NL[Pos]
                        {
                            if (BestFL < (CurrentMaxL = min(L, MaxLL)))
                            {
                                BestFL = CurrentMaxL;
                                BestFO = O;
                            }
                            int I = Pos;
                            while ((L >= (CurrentMaxL = min(I, MaxLL))) && (L-- > 2))
                            {
                                FL[I] = CurrentMaxL;
                                FO[I--] = O;
                            }
                        }
                    }
                }
                FL[Pos] = BestFL;
                FO[Pos] = BestFO;
            }
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

void CalcBestSequence(int SeqHighestIndex, int SeqLowestIndex, bool FirstRun)
{
    FindMatches(SeqHighestIndex, SeqLowestIndex, FirstRun);

    //----------------------------------------------------------------------------------------------------------
    //FIND BEST SEQUENCE FOR EACH POSITION
    //----------------------------------------------------------------------------------------------------------

    for (int Pos = SeqLowestIndex; Pos <= SeqHighestIndex; Pos++)
    {
        //Start with second element, first has been initialized above
        Seq[Pos + 1].TotalBits = 0xffffff;
        //Max block size=100 = $10000 bytes = $80000 bits, make default larger than this

        if ((FL[Pos] != 0) || (FFL[Pos] != 0))
        {
            if (FL[Pos] >= FFL[Pos])
            {
                CheckMatchSeq(FL[Pos], FO[Pos], Pos);
            }
            else if (Pos < SeqHighestIndex)     //The last byte of the block MUST be a literal, we can't use Far Matches there
            {
                //If a reference is under I/O then this block also must be under I/O to be able to copy the reference
                //If ((PrgAdd + Pos + FFO(Pos) >= &HD000) AndAlso (PrgAdd + Pos + FFO(Pos) < &HE000)) OrElse
                //((PrgAdd + Pos + FFO(Pos) - FFL(Pos) >= &HD000) AndAlso (PrgAdd + Pos + FFO(Pos) - FFL(Pos) < &HE000)) Then
                //BlockUnderIO = 1
                //End If
                CheckMatchSeq(FFL[Pos], FFO[Pos], Pos);
            }
        }
        if ((NL[Pos] > 0) || (FNL[Pos] > 0))
        {
            if (NL[Pos] >= FNL[Pos])
            {
                CheckMatchSeq(NL[Pos], NO[Pos], Pos);
            }
            else if (Pos < SeqHighestIndex)
            {
                CheckMatchSeq(FNL[Pos], FNO[Pos], Pos);
            }
        }
        if ((SL[Pos] > 0) || (FSL[Pos] > 0))
        {
            if (SL[Pos] >= FSL[Pos])
            {
                CheckMatchSeq(SL[Pos], SO[Pos], Pos);
            }
            else if (Pos < SeqHighestIndex)
            {
                CheckMatchSeq(FSL[Pos], FSO[Pos], Pos);
            }
        }
        CheckLitSeq(Pos);
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

bool CloseBuffer() {

    AddMatchBit();

    BlockCnt++;
    BufferCnt++;

    //This does not work here yet, Pack needs to be changed to a function
    //If BufferCnt > BlocksFree Then
    //MsgBox("Unable to add bundle to disk :(", vbOKOnly, "Not enough free space on disk")
    //GoTo NoDisk
    //End If

    if (!UpdateByteStream())
        return false;

    ResetBuffer();                      //Resets buffer variables

    NextFileInBuffer = false;           //Reset Next File flag

    TransitionalBlock = false;          //Only the first block of a bundle is a transitional block

    FirstLitOfBlock = true;

    if (SI < 0)                         //We have reached the end of the file -> exit
    {
        return true;
    }

    //If we have not reached the end of the file, then update buffer

    //------------------------------------------------------------------------------------------------------------------------------
    //"COLOR BUG"
    //Compression bug related to the transitional block (i.e. finding the last block of a bundle) - FIXED
    //Fix: add 5 or 6 bytes + 2 bits to the calculation to find the last block of a bundle
    //+2 new bundle tag, +2 NEXT Bundle address, +1 first literal byte of NEXT Bundle, +0/1 IO status of first literal byte of NEXT file
    //+1 literal bit, +1 match bit (may or may not be needed, but we don't know until the end...)
    //------------------------------------------------------------------------------------------------------------------------------

    //Check if the first literal byte of the NEXT Bundle will go under I/O
    //Bits needed for next bundle is calculated in ModDisk:SortPart
    //(Next block = Second block) or (remaining bits of Last File in Bundle + Needed Bits fit in this block)

    //LETHARGY BUG - Bits Left need to be calculated from Seq(SI+1) and NOT Seq(SI)
    //Add 4 bits if number of nibbles is odd
    int BitsLeftInBundle = Seq[SI + 1].TotalBits + ((Seq[SI + 1].Nibbles % 2) * 4);

    //If the next block is the first one on a new track, no need to recalculate the sequence
    //As all previous blocks will be loaded from the previous track before this block gets loaded
    bool NewTrack = false;

    if (BufferCnt < (17 * 21))
    {
        if (BufferCnt % 21 == 0) NewTrack = true;
    }
    else if (BufferCnt < ((17 * 21) + (6 * 19)))
    {
        if ((BufferCnt - (17 * 21)) % 19 == 0) NewTrack = true;
    }
    else if (BufferCnt < ((17 * 21) + (6 * 19) + (6 * 18)))
    {
        if ((BufferCnt - (17 * 21) - (6 * 19)) % 18 == 0) NewTrack = true;
    }
    else
    {
        if ((BufferCnt - (17 * 21) - (6 * 19) - (6 * 18)) % 17 == 0) NewTrack = true;
    }

    //KARAOKE BUG - fixed in Sparkle 2.2
    //BitsLeftInBundle = bits left to be compressed in bundle, add 1 bit if MLen>0, add 8 bits for Block Count (to simulate 'TransitionalBlock = True')
    //If the result is less than the bits remaining free in the remaining data + the first byte of the next bundle should fit in the buffer
    //I.e. we have identified the last block of the bundle (=transitional block)

    //LastBlockOfBundle = ((LastFileOfBundle) && (!NewBlock) && (BitsLeftInBundle + BitsNeededForNextBundle + ((MLen == 0) ? 0 : 1) + 8 <= ((LastByte - 1) * 8) + BitPos));
    
    if ((LastFileOfBundle) && (!NewBlock))
    {
        int BitsNeeded = BitsLeftInBundle + BitsNeededForNextBundle + ((MLen == 0) ? 0 : 1) + 8;
        int BitsAvailable = ((LastByte - 1) * 8) + BitPos;
        
        //if (BitsLeftInBundle + BitsNeededForNextBundle + ((MLen == 0) ? 0 : 1) + 8 <= ((LastByte - 1) * 8))
        if (BitsNeeded <= BitsAvailable)
        {
            LastBlockOfBundle = true;
        }
        //else
        //{
        //    LastBlockOfBundle = false;
        //}
    }

    /*
        if ((BitsLeftInBundle + BitsNeededForNextBundle + ((MLen == 0) ? 0 : 1) + 8 <= ((LastByte - 1) * 8) + BitPos) && (LastFileOfBundle) && (!NewBlock))
        {
            LastBlockOfBundle = true;
        }
        else
        {
            LastBlockOfBundle = false;
        }
    */

    if ((LastBlockOfBundle) || (NewTrack) || (BlockCnt == 1))
    {
        //Seq(SI+1).Bytes/Nibbles/Bits = to calculate remaining bits in file
        //BitsNeededForNextBundle (5-6 bytes + 1/2 bits)
        //+5/6 bytes +1/2 bits
        //LastByte-1: subtract close tag/block count = Byte(1)
        //Bits remaining in block: LastByte * 8 (+ remaining bits in last BitPtr (BitPos+1))
        //But we are trying to overcalculate here to avoid misidentification of the last block
        //Which would result in buggy decompression

        //This is the last block ONLY IF the remainder of the bundle + the next bundle's info fits!!!
        //AND THE NEXT Bundle IS NOT ALIGNED in which case the next block is the last one
        //Seg(SI).bit includes both the byte stream in bits and the bit stream (total bits needed to compress the remainder of the bundle)
        //+Close Tag: 8 bits
        //+BitsNeeded: 5-6 bytes for next bundle's info + 1 lit bit + / -1 match bit(may or may not be needed, but we wouldn't know until the end)
        //For the 2nd and last blocks of a bundle and the first blocks on a new track only recalculate the first byte's sequence
        //If BlockCnt <> 1 Then MsgBox((BitsLeftInBundle + BitsNeededForNextBundle).ToString + vbNewLine + (Seq(SI + 1).TotalBits + BitsNeededForNextBundle).ToString + vbNewLine + ((LastByte - 1) * 8 + BitPos).ToString)

        //Only recalculate the very first byte's sequence
        CalcBestSequence(max(SI, 1), max(SI, 1), false); //'(If(SI > 1, SI, 1), If(SI > 1, SI, 1))

        if (NewTrack)
        {

            if (CurrentFileIndex > PartialFileIndex)
            {

                if (PartialFileIndex > -1)
                {
                    //Search the finished segment of partial file
                        //ReferenceFile = Prgs(PartialFileIndex).ToArray
                    ReferenceFileStart = Prgs[PartialFileIndex].iFileAddr;

                    SearchReferenceFile(SI, PartialFileIndex, ReferenceFileStart + PartialFileOffset, ReferenceFileStart + 1);
                }

                //Search any finished files on track (partial file < finished file < current file)
                for (int I = PartialFileIndex + 1; I <= CurrentFileIndex - 1; I++)
                {
                    //ReferenceFile = Prgs(I).ToArray
                    ReferenceFileStart = Prgs[I].iFileAddr;

                    SearchReferenceFile(SI, I, ReferenceFileStart + Prgs[I].iFileLen - 1, ReferenceFileStart + 1);
                }

                //Search the finished segment of this file
                //ReferenceFile = Prgs(CurrentFileIndex).ToArray
                ReferenceFileStart = Prgs[CurrentFileIndex].iFileAddr;

                SearchReferenceFile(SI, CurrentFileIndex, ReferenceFileStart + Prgs[CurrentFileIndex].iFileLen - 1, ReferenceFileStart + SI + 1);
            }
            else
            {
                //Partial file = CurrentFile (same file, spanning multiple tracks)
                //ReferenceFile = Prgs(CurrentFileIndex).ToArray
                ReferenceFileStart = Prgs[CurrentFileIndex].iFileAddr;

                SearchReferenceFile(SI, CurrentFileIndex, ReferenceFileStart + PartialFileOffset, ReferenceFileStart + SI + 1);

            }

            //'The current file becomes the partial file, anything before it is now finished and can be used for search
            PartialFileIndex = CurrentFileIndex;
            PartialFileOffset = SI;

        }

        if ((BlockCnt != 1) && (!NewTrack))     //Do not recalculate the very first block and first blocks on each track
        {
            //Last/Transitional block
            BitsLeftInBundle = Seq[SI + 1].TotalBits + ((Seq[SI + 1].Nibbles % 2) * 4);
            //If the new bit count does not fit in the buffer then this is NOT the last block -> recalc sequence
            //KARAOKE BUG - fixed in Sparkle 2.2
            if (BitsLeftInBundle + BitsNeededForNextBundle + ((MLen == 0) ? 0 : 1) + 8 > ((LastByte - 1) * 8) + BitPos)
            {
                LastBlockOfBundle = false;
                goto CalcAll;
            }
        }
    }
    else
    {
        //For all other blocks recalculate the first 256*3 bytes' sequence (MaxNearOffset * 3)
    CalcAll:
        CalcBestSequence(max(SI, 1), max(SI - MaxOffset, 1), false);   //If(SI > 1, SI, 1), If(SI - MaxOffset > 1, SI - MaxOffset, 1))
    }

    //----------------------------------------------------------------------------------------------------------

    //if ((PrgAdd + SI) == 0x666f)        //SABREWULF
    //{
    //    PrgAdd += 0;
    //}

    AdLoPos = BytePtr;
    Buffer[BytePtr--] = (PrgAdd + SI) % 256;

    BlockUnderIO = CheckIO(SI, -1);          //Check if last byte of prg could go under IO

    if (BlockUnderIO == 1)
    {
        BytePtr--;
    }

    AdHiPos = BytePtr;
    Buffer[BytePtr--] = ((PrgAdd + SI) / 256) % 256;
    LastByte = BytePtr;             //LastByte = the first byte of the ByteStream after and Address Bytes (253 or 252 with BlockCnt)

    StartPtr = SI;

    return true;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

bool CloseFile() {

    //ADDS NEXT FILE TAG TO BUFFER

    //4-5 bytes and 1-2 bits needed for NextFileTag, Address Bytes and first Lit byte (+1 more if UIO)
    //BYTES NEEDED: (1)End Tag + (2)AdLo + (3)AdHi + (4)1st Literal +/- (5)I/O FLAG of NEW FILE's 1st literal
    //BUG reported by Raistlin/G*P

    int BytesNeededForNextFile = 4 + CheckIO(PrgLen - 1, -1);

    //THE FIRST LITERAL BYTE WILL ALSO NEED A LITERAL BIT
    //DO NOT check whether Match Bit is needed for new file - will be checked in SequenceFits()
    //BUG reported by Visage/Lethargy

    int BitsNeededForNextFile = 1;

    //Type selector bit (match vs literal) is not needed, the first byte of a file is always literal
    //So this is the literal length bit: 0 - 1 literal, 1 - more than 1 literals, would also need a Nibble...
    //...but here we only need to be able to fit 1 literal byte

    NextFileInBuffer = true;

    if (SequenceFits(BytesNeededForNextFile, BitsNeededForNextFile,0))
    {
        //Buffer has enough space for Next File Tag, New File Address and first Literal byte (and I/O flag if needed)

        //If last sequence was a match (no literals) then add a match bit
        if ((MLen > 0) || (LitCnt == -1)) AddBits(MatchSelector, 1);

        Buffer[BytePtr--] = NextFileTag;                            //Then add Next File Tag
        FirstLitOfBlock = true;
    }
    else
    {
        //Next File Info does not fit, so close buffer, next file will start in new block
        if (!CloseBuffer())
            return false;
    }

    return true;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

bool StartNewBundle()
{
    //New Bundle is Aligned OR next file start sequence DOES NOT fit, so close buffer
    if (!CloseBuffer())         //Adds EndTag and starts new buffer
        return false;
    
    //Then add 1 dummy literal byte to new block (blocks must start with 1 literal, next bundle tag is a match tag)
    Buffer[255] = 0xfd;         //Dummy Address ($03fd* - first literal's address in buffer... (*NextPart above, will reserve BlockCnt)
    Buffer[254] = 0x03;         //...we are overwriting it with the same value
    Buffer[253] = 0x00;         //Dummy value, will be overwritten with itself
    
    LitCnt = 0;
    
    AddLitBits(LitCnt);    //WE NEED THIS HERE, AS THIS IS THE BEGINNING OF THE BUFFER, AND 1ST BIT WILL BE CHANGED TO COMPRESSION BIT
    
    BytePtr = 252;
    LastBlockCnt++;

    if (LastBlockCnt > 255)
    {
        //Bundles cannot be larger than 255 blocks compressed
        cout << "***CRITICAL***\tBundle " << BundleCnt << " would need " << LastBlockCnt << " blocks on the disk.\nBundles cannot be larger than 255 blocks!\n";
        return false;
    }

    BlockCnt--;

    return true;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

void StartNextBundle(bool LastBundleOnDisk)
{
    //Buffer has enough space for New Bundle Tag and New Bundle Info and first Literal byte (and IO flag if needed)

    //Match Bit is not needed if this is the beginning of the next block
    FilesInBuffer++;  //There is going to be more than 1 file in the buffer

    Buffer[1] = EORtransform(0);

    NibblePtr = 0;
    Buffer[BytePtr--] = NearLongMatchTag;   //Then add New File Match Tag
    Buffer[BytePtr--] = EndOfBundleTag;
    BitPtr = BytePtr--;
    Buffer[BitPtr] = 0x01;

    BitPos = 7;
    BitsLeft = 7;

    if (LastBundleOnDisk)                   //This will finish the disk
    {
        Buffer[BytePtr] = BytePtr - 2;      //Finish disk with a dummy literal byte that overwrites itself to reset LastX for next disk side
        Buffer[BytePtr - 1] = 0x03;         //New address is the next byte in buffer
        Buffer[BytePtr - 2] = 0x00;         //Dummy $00 Literal that overwrites itself
        LitCnt = 0;                         //One (dummy) literal
        //AddLitBits()                      //NOT NEEDED, WE ARE IN THE MIDDLE OF THE BUFFER, 1ST BIT NEEDS TO BE OMITTED
        AddBits(0, 1);             //ADD 2ND BIT SEPARATELY (0-BIT, TECHNCALLY, THIS IS NOT NEEDED SINCE THIS IS THE LAST BIT)

        BytePtr -= 3;
        if (BundleNo < 128)
        {
            DirBlocks[(BundleNo * 4) + 3] = BitPtr;
            DirPtr[BundleNo] = BufferCnt;
        }
    }

    //DO NOT CLOSE LAST BUFFER HERE, WE ARE GOING TO ADD NEXT Bundle TO LAST BUFFER
    if ((BufferCnt * 256) > BlockPtr + 255)                                    //Only save block count if block is already added to ByteSt
    {
        ByteSt[BlockPtr + 1] = EORtransform(LastBlockCnt);   //New Block Count is ByteSt(BlockPtr+1) in buffer, not ByteSt(BlockPtr+255)
        LoaderBundles++;
    }

    LitCnt = -1;                        //Reset LitCnt here

}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

bool CloseBundle(int NextFileIO, bool LastBundleOnDisk)
{
    //Can't use BitsNeededForNextBundle here, this could be the last bundle on the disk!!!
    //So we re-calculate the number of bytes needed for the next (potentially dummy) bundle
    //BYTES NEEDED: (1)Long Match Tag + (2)End Tag + (3)BitPtr + (4)AdLo + (5)AdHi + (6)1st Literal +/- (7)I/O flag
    int BytesNeeded = 6 + NextFileIO;     //Next bundle is already sorted here, we know exactly which file is the next one
    
    //SABREWULF BUG - reported by Raistlin
    //Adding a Literal Bit here resulted in overcalculating the space needed for the next block -AFTER- we already identified and compressed it
    //and started a new block with a dummy write - this resulted in out-of-order loading of the last block with wrong match references
    
    //THE FIRST LITERAL OF THE NEXT BUNDLE DOES -NOT- NEED AN ADDITIONAL LITERAL BIT - it is already included in Bytes, see (3)BitPtr!!!
    //DO NOT ADD MATCH BIT HERE, IT WILL BE ADDED IN SequenceFits()
    
    int BitsNeeded = 0;

    TransitionalBlock = true;    //This is always a transitional block, unless close sequence does not fit, will add +1 for Block Count

    if ((NewBlock) || (!SequenceFits(BytesNeeded, BitsNeeded, 0)))
    {
        if (!StartNewBundle())
            return false;

    }
    else
    {

        //If last sequence was a match (no literals) then add a match bit
        if ((MLen > 0) || (LitCnt == -1))
        {
            AddBits(1, 1);
        }
    }

    //Then add next bundle
    StartNextBundle(LastBundleOnDisk);

    NewBlock = SetNewBlock;        //NewBlock is true at closing the previous bundle, so first it just sets NewBlock2
    SetNewBlock = false;            //And NewBlock2 will fire at the desired bundle

    return true;

}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

void FindVirtualFarMatches(int RefIndex, int SeqMaxIndex, int SeqMinIndex,int RefMaxAddress, int RefMinAddress)
{

    //Filter out virtual files overlapping packed file
    if (((RefMinAddress >= PrgAdd) && (RefMinAddress < PrgAdd + PrgLen)) || ((RefMaxAddress >= PrgAdd) && (RefMaxAddress < PrgAdd + PrgLen)) || ((RefMinAddress < PrgAdd) && (RefMaxAddress >= PrgAdd + PrgLen)))
    {
        return;
    }
    
    if (!FileUnderIO)
    {
        if ((PrgAdd + SeqMinIndex >= 0xd000) && (PrgAdd + SeqMinIndex <= 0xdfff))
        {
            if (PrgAdd + SeqMaxIndex <= 0xdfff)
            {
                //The entire file segment is under I/O -> skip it
                return;
            }
            else
            {
                //Not the entire file segment is under I/O -> only check the part that is not
                SeqMinIndex = 0xe000 - PrgAdd;

            }
        }
        else if ((PrgAdd + SeqMaxIndex >= 0xd000) && (PrgAdd + SeqMaxIndex <= 0xdfff))
        {
            SeqMaxIndex = 0xcfff - PrgAdd;
        }
    }

    //----------------------------------------------------------------------------------------------------------
    //FIND LONGEST MATCHES FOR EACH POSITION
    //----------------------------------------------------------------------------------------------------------
    int RefMinAddressIndex = RefMinAddress - ReferenceFileStart;
    int RefMaxAddressIndex = RefMaxAddress - ReferenceFileStart;

    int OffsetBase = (ReferenceFileStart >= PrgAdd) ? ReferenceFileStart - PrgAdd : ReferenceFileStart + 0x10000 - PrgAdd;

    for (int Pos = SeqMinIndex; Pos <= SeqMaxIndex; Pos++)  //Pos cannot be 0, Prg(0) is always literal as it is always 1 byte left
    {

        unsigned char PrgValAtPos = Prgs[CurrentFileIndex].Prg[Pos];

        int ShortMinO = RefMinAddressIndex;
        int ShortMaxO = min(max(Pos + MaxShortOffset, OffsetBase + RefMinAddressIndex), OffsetBase + RefMaxAddressIndex) - OffsetBase;

        for (int O = ShortMinO; O <= ShortMaxO; O++)
        {
            //Check if first byte matches at offset, if not go to next offset
            if (PrgValAtPos == VFiles[RefIndex].Prg[O])
            {
                //int MaxLL = FastMin(FastMin(Pos + 1, MaxLongLen),O - RefMinAddressIndex + 1);   //MaxLL = 255 or less
                int MaxLL = min(min(Pos + 1, MaxLongLen), O - RefMinAddressIndex + 1);   //MaxLL = 255 or less
                int MaxSL = min(min(Pos + 1, MaxShortLen), O - RefMinAddressIndex + 1);

                if ((FSL[Pos] == MaxSL) && (FNL[Pos] == min(min(Pos + 1, MaxFarMidLen), O - RefMinAddressIndex + 1)))
                {
                    break;
                }

                for (int L = 1; L <= MaxLL; L++)
                {
                    if ((L == MaxLL) || (Prgs[CurrentFileIndex].Prg[Pos - L] != VFiles[RefIndex].Prg[O - L]))
                    {
                        //Find the first position where there is NO match -> this will give us the absolute length of the match
                        //L=MatchLength + 1 here
                        if (L > 2)
                        {
                            if (FSL[Pos] < min(MaxSL, L))
                            {
                                FSL[Pos] = min(MaxSL, L); //If(L > MaxShortLen, MaxShortLen, L)   'Short matches cannot be longer than 4 bytes
                                FSO[Pos] = O - Pos + OffsetBase;                        //Keep Offset 1-based
                            }
                            if (FNL[Pos] < L)   //Allow short (2-byte) Mid Matches
                            {
                                FNL[Pos] = L;
                                FNO[Pos] = O - Pos + OffsetBase;
                            }
                        }
                        break;
                    }
                }
            }
        }

        int NearMinO = min(RefMinAddressIndex, ShortMaxO);
        int NearMaxO = min(max(Pos + MaxNearOffset, OffsetBase + RefMinAddressIndex), OffsetBase + RefMaxAddressIndex) - OffsetBase;

        for (int O = NearMinO; O <= NearMaxO; O++)
        {
            //Check if first byte matches at offset, if not go to next offset
            if (PrgValAtPos == VFiles[RefIndex].Prg[O])
            {
                //int MaxLL = FastMin(FastMin(Pos + 1, MaxLongLen),O - RefMinAddressIndex + 1);   //MaxLL = 255 or less
                int MaxLL = min(min(Pos + 1, MaxLongLen), O - RefMinAddressIndex + 1);   //MaxLL = 255 or less

                //If far matches maxed out, we can leave the loop and go to the next Prg position
                if (FNL[Pos] == min(min(Pos + 1, MaxFarMidLen), O - RefMinAddressIndex + 1))
                {
                    break;
                }

                for (int L = 1; L <= MaxLL; L++)
                {
                    if ((L == MaxLL) || (Prgs[CurrentFileIndex].Prg[Pos - L] != VFiles[RefIndex].Prg[O - L]))
                    {
                        //Find the first position where there is NO match -> this will give us the absolute length of the match
                        //L=MatchLength + 1 here
                        if (L > 2)
                        {
                            if (FNL[Pos] < L)   //Allow short (2-byte) Mid Matches
                            {
                                FNL[Pos] = L;
                                FNO[Pos] = O - Pos + OffsetBase;
                            }
                        }
                        break;
                    }
                }
            }
        }

        int FarMinO = max(RefMinAddressIndex, NearMaxO);

        for (int O = FarMinO; O <= RefMaxAddressIndex; O++)
        {
            //Check if first byte matches at offset, if not go to next offset
            if (PrgValAtPos == VFiles[RefIndex].Prg[O])
            {
                //int MaxLL = FastMin(FastMin(Pos + 1, MaxLongLen),O - RefMinAddressIndex + 1);   //MaxLL = 255 or less
                int MaxLL = min(min(Pos + 1, MaxLongLen), O - RefMinAddressIndex + 1);   //MaxLL = 255 or less
                //If far matches maxed out, we can leave the loop and go to the next Prg position
                if (FFL[Pos] == min(min(Pos + 1, MaxFarMidLen), O - RefMinAddressIndex + 1))
                {
                    break;
                }

                for (int L = 1; L <= MaxLL; L++)
                {
                    if ((L == MaxLL) || (Prgs[CurrentFileIndex].Prg[Pos - L] != VFiles[RefIndex].Prg[O - L]))
                    {
                        //Find the first position where there is NO match -> this will give us the absolute length of the match
                        //L=MatchLength + 1 here
                        if (L > 3)
                        {
                            if (FFL[Pos] < L)
                            {
                                FFL[Pos] = L;
                                FFO[Pos] = O - Pos + OffsetBase;
                                //cout << hex << FFL[Pos] << "\t" << FFO[Pos] << "\t" << Pos << dec << "\n";
                            }
                        }
                        break;
                    }
                }
            }
        }
        
/*        unsigned char PrgValAtPos = Prgs[CurrentFileIndex].Prg[Pos];

        //Offset goes from 1 to max offset (cannot be 0)
        //Match length goes from 1 to max length
        for (int O = RefMinAddressIndex; O <= RefMaxAddressIndex; O++)
        {
            //Check if first byte matches at offset, if not go to next offset
            if (PrgValAtPos == VFiles[RefIndex].Prg[O])
            {
                int MaxLL = min(min(Pos + 1, MaxLongLen), O - RefMinAddressIndex + 1);   //MaxLL = 255 or less
                int MaxSL = min(min(Pos + 1, MaxShortLen), O - RefMinAddressIndex + 1);

                for (int L = 1; L <= MaxLL; L++)
                {
                    if ((L == MaxLL) || (Prgs[CurrentFileIndex].Prg[Pos - L] != VFiles[RefIndex].Prg[O - L]))
                    {
                        //Find the first position where there is NO match -> this will give us the absolute length of the match
                        //L=MatchLength + 1 here
                        if (L > 2)
                        {
                            if ((O - Pos + OffsetBase <= MaxShortOffset) && (FSL[Pos] < MaxSL) && (FSL[Pos] < L))
                            {
                                FSL[Pos] = min(MaxSL, L); //If(L > MaxShortLen, MaxShortLen, L)   'Short matches cannot be longer than 4 bytes
                                FSO[Pos] = O - Pos + OffsetBase;                        //Keep Offset 1-based
                            }
                            if ((O - Pos + OffsetBase <= MaxNearOffset) && (FNL[Pos] < L))   //Allow short (2-byte) Mid Matches
                            {
                                FNL[Pos] = L;
                                FNO[Pos] = O - Pos + OffsetBase;
                            }
                            if (((O - Pos + OffsetBase) > MaxNearOffset) && (FFL[Pos] < L))
                            {
                                FFL[Pos] = L;
                                FFO[Pos] = O - Pos + OffsetBase;
                            }
                        }
                        break;
                    }
                }
                //If far matches maxed out, we can leave the loop and go to the next Prg position
                if (FFL[Pos] == MaxLL)
                {
                    break;
                }
            }
        }
 */
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

bool Pack()
{
    //Packing is done backwards

    bool BufferFull = false;

    SI = PrgLen - 1;
    StartPtr = SI;

Restart:
    while (SI >= 0)
    {
        if (Seq[SI + 1].Off == 0)
        {
            //--------------------------------------------------------------------
            //Literal sequence
            //--------------------------------------------------------------------
            LitCnt = Seq[SI + 1].Len;               //LitCnt is 0 based
            LitSI = SI;
            MLen = 0;                               //Reset MLen - this is needed for accurate bit counting in sequencefits

            //The max number of literals that fit in a single buffer is 249 bytes
            //This bypasses longer literal sequences and improves compression speed
            BufferFull = false;

            //Shortcut to bypass long literal sequences that wouldn't fit in the buffer anyway
            if (LitCnt > BytePtr)       //MaxLitPerBlock Then
            {
                BufferFull = true;
                LitCnt = BytePtr;       //MaxLitPerBlock
            }

            while (LitCnt > -1)
            {
                if (SequenceFits(LitCnt + 1, CalcLitBits(LitCnt), CheckIO(SI - LitCnt, -1)))
                {
                    break;
                }
                LitCnt--;
                BufferFull = true;
            }

            //Go to next element in sequence
            SI -= LitCnt + 1;       //If nothing added to the buffer, LitCnt=-1+1=0

            if (BufferFull)
            {
                AddLitSequence();
                if (!CloseBuffer())      //The whole literal sequence did not fit, buffer is full, close it
                    return false;
            }
        }
        else
        {
            //--------------------------------------------------------------------
            //Match sequence
            //--------------------------------------------------------------------

            BufferFull = false;

            MLen = Seq[SI + 1].Len;     //1 based
            MOff = Seq[SI + 1].Off;     //1 based
        //Match:
            CalcMatchBytesAndBits(MLen, MOff);

            if (MatchBytes == 4)
            {
                //--------------------------------------------------------------------
                //Far Long Match - 4 match bytes + 0/1 match bit
                //--------------------------------------------------------------------

                ReferenceUnderIO = 0;

                if ((((PrgAdd + SI + MOff) % 0x10000 >= 0xD000) && ((PrgAdd + SI + MOff) % 0x10000 < 0xE000)) ||
                    (((PrgAdd + SI + MOff - MLen + 1) % 0x10000 >= 0xD000) && ((PrgAdd + SI + MOff - MLen + 1) % 0x10000 < 0xE000)))
                {
                    if (((PrgAdd + SI >= 0xD000) && (PrgAdd + SI < 0xE000) && (!FileUnderIO)) ||
                        ((PrgAdd + SI - MLen + 1 >= 0xD000) && (PrgAdd + SI - MLen + 1 < 0xE000) && (!FileUnderIO)))
                    {

                    }
                    else
                    {
                        ReferenceUnderIO = 1;
                    }
                }

                if (SequenceFits(MatchBytes + LitCnt + 1, MatchBits + CalcLitBits(LitCnt), max(CheckIO(SI - MLen + 1, -1), ReferenceUnderIO)))
                {
                    AddLitSequence();
                    //Add far long match
                    AddFarLongMatch();
                }
                else
                {
                    MatchBytes = 3;
                    MLen = MaxMidLen;
                    BufferFull = true;   //Buffer if full, we will need to close it
                    goto Check3Bytes;

                }
            }
            else if (MatchBytes == 3)
            {
                //--------------------------------------------------------------------
                //Far Mid Match or Near Long Match - 3 match bytes + 0/1 match bit
                //--------------------------------------------------------------------
            Check3Bytes:

                ReferenceUnderIO = 0;

                if (MOff > MaxNearOffset)
                {
                    if ((((PrgAdd + SI + MOff) % 0x10000 >= 0xD000) && ((PrgAdd + SI + MOff) % 0x10000 < 0xE000)) ||
                        (((PrgAdd + SI + MOff - MLen + 1) % 0x10000 >= 0xD000) && ((PrgAdd + SI + MOff - MLen + 1) % 0x10000 < 0xE000)))
                    {
                        if (((PrgAdd + SI >= 0xD000) && (PrgAdd + SI < 0xE000) && (!FileUnderIO)) ||
                            ((PrgAdd + SI - MLen + 1 >= 0xD000) && (PrgAdd + SI - MLen + 1 < 0xE000) && (!FileUnderIO)))
                        {

                        }
                        else
                        {
                            ReferenceUnderIO = 1;
                        }
                    }
                }

                if (SequenceFits(MatchBytes + LitCnt + 1, MatchBits + CalcLitBits(LitCnt), max(CheckIO(SI - MLen + 1, -1), ReferenceUnderIO)))
                {
                    AddLitSequence();
                    //Add long match
                    if (MOff > MaxNearOffset)
                    {
                        //Add far mid match
                        AddFarMidMatch();
                    }
                    else
                    {
                        //Add near long match
                        AddNearLongMatch();
                    }
                }
                else
                {
                    BufferFull = true;   //Buffer if full, we will need to close it
                    if (MOff > MaxNearOffset)
                    {
                        if ((SL[SI] >= NL[SI]) && (SL[SI] != 0))
                        {
                            MatchBytes = 1;
                            MLen = SL[SI] < MaxShortLen ? SL[SI] : MaxShortLen; // min(SL[SI], MaxShortLen);  //SL is at least as good as NL, but uses 
                            MOff = SO[SI];  //SL and SO array indeces are 0 based
                            goto CheckShort;
                        }
                        else if (NL[SI] > 0)
                        {
                            MatchBytes = 2;
                            MLen = NL[SI] < MaxMidLen ? NL[SI] : MaxMidLen; // min(NL[SI], MaxMidLen);   //NL and NO array indeces are 0 based
                            MOff = NO[SI];
                            goto Check2Bytes;
                        }
                        else
                        {
                            goto CheckLit;
                        }
                    }
                    else
                    {
                        MLen = MaxMidLen;
                        goto Check2Bytes;
                    }
                }
            }
            else if (MatchBytes == 2)
            {
                //--------------------------------------------------------------------
                //Near Mid Match - 2 match bytes + 0/1 match bit
                //--------------------------------------------------------------------
            Check2Bytes:
                if (SequenceFits(MatchBytes + LitCnt + 1, MatchBits + CalcLitBits(LitCnt), CheckIO(SI - MLen + 1, -1)))
                {
                    AddLitSequence();
                    //Add mid match
                    AddNearMidMatch();
                }
                else
                {
                    BufferFull = true;
                    if (SO[SI] != 0)
                    {
                        MatchBytes = 1;
                        MLen = SL[SI];   //SL and SO array indeces are 0 based
                        MOff = SO[SI];
                        goto CheckShort;
                    }
                    else
                    {
                        goto CheckLit;
                    }
                }
            }
            else
            {
                //--------------------------------------------------------------------
                //Short Match - 1 match byte + 0/1 match bit
                //--------------------------------------------------------------------
            CheckShort:
                if (SequenceFits(MatchBytes + LitCnt + 1, MatchBits + CalcLitBits(LitCnt), CheckIO(SI - MLen + 1, -1)))
                {
                    AddLitSequence();
                    //Add short match
                    AddShortMatch();
                }
                else
                {
                    //--------------------------------------------------------------------
                    //Match does not fit, check if 1 literal byte fits
                    //--------------------------------------------------------------------
                    BufferFull = true;
                CheckLit:
                    MLen = 0;    //This is needed here for accurate Bit count calculation in sequencefits (indicates Literal, not Match)
                    int NewLit = 1;
                    if (SequenceFits(NewLit + LitCnt + 1, CalcLitBits(LitCnt + 1), CheckIO(SI - LitCnt, -1)))
                    {
                        if (LitCnt == -1)
                        {
                            //If no literals, current SI will be LitSI, else, do not change LitSI
                            LitSI = SI;

                        }
                        LitCnt++;    //0 based, now add 1 for an additional literal (first byte of match that did not fit)
                        SI--;         //Rest of LitCnt has been already subtracted from SI
                    }   //Literal vs nothing
                }   //Short match vs literal
            }   //Long, mid, or short match
        //Done:
            SI -= MLen;

            if (BufferFull)
            {
                AddLitSequence();
                if (!CloseBuffer())
                    return false;
            }
        }   //Lit vs match
    }

    AddLitSequence();        //See if any remaining literals need to be added, space has been previously reserved for them

    //KARAOKE BUG - fixed in Sparkle 2.2 - making sure that the first byte of the next bundle fits in the transitional block
    int BytesNeeded = BitsNeededForNextBundle / 8;      //6 or 7 bytes, depending on I/O status of first file in next bundle
    int BitsNeeded = 0;                                 //DO NOT ADD A LITERAL BIT HERE - WE INCLUDE A BitPtr IN BytesNeeded 
    if (LastBlockOfBundle)
    {
        LastBlockOfBundle = false;
        TransitionalBlock = true;
        if (!SequenceFits(BytesNeeded, BitsNeeded, 0))  //Bits=1, we need 1 literal bit MLen will be checked in SequenceFits
        {
            //We have miscalculated the last block of the bundle, let's recompress it the conventional way!
            SI = StartPtr;
            ResetBuffer();                       //Resets buffer variables
            NextFileInBuffer = false;            //Reset Next File flag
            TransitionalBlock = false;           //Only the first block of a bundle is a transitional block
            FirstLitOfBlock = true;

            Buffer[BytePtr] = (PrgAdd + SI) % 256;
            AdLoPos = BytePtr;
            BlockUnderIO = CheckIO(SI, -1);          //Check if last byte of prg could go under IO

            if (BlockUnderIO == 1)
            {
                BytePtr--;
            }

            Buffer[BytePtr - 1] = ((PrgAdd + SI) / 256) % 256;
            AdHiPos = BytePtr - 1;
            BytePtr -= 2;
            LastByte = BytePtr;              //LastByte = the first byte of the ByteStream after and Address Bytes (253 or 252 with blockCnt)
            CalcBestSequence(((SI > 1) ? SI : 1), ((SI - MaxNearOffset > 1) ? SI - MaxNearOffset : 1), false);
            goto Restart;
        }
    }
    return true;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

void SearchVirtualFile(int PrgMaxIndex, int RefIndex, int RefMaxAddress, int RefMinAddress)
{
    //PrgMaxIndex = relative address within Prg
    //RefMaxAddress, RefMinAddress = absolute addresses within reference file
    if (!VFiles[RefIndex].FileIO)
    {
        //Reference file is not under I/O, check if it is IN I/O
        if (((RefMinAddress >= 0xd000) && (RefMinAddress < 0xe000)) || ((RefMaxAddress >= 0xd000) && (RefMaxAddress < 0xe000)))
        {
            //This reference file is IN the I/O registers, SKIP anything between $d000-$dfff
            if (RefMinAddress + 1 < 0xd000)
            {
                //Search from start to $cfff
                FindVirtualFarMatches(RefIndex,PrgMaxIndex, 1, 0xcfff, RefMinAddress + 1);
            }
            if (RefMaxAddress > 0xe000)
            {
                //Search from $e000 to end of file
                FindVirtualFarMatches(RefIndex, PrgMaxIndex, 1, RefMaxAddress, 0xe000 + 1);
            }
        }
        else
        {
            FindVirtualFarMatches(RefIndex,PrgMaxIndex, 1, RefMaxAddress, RefMinAddress + 1);
        }
    }
    else
    {
        FindVirtualFarMatches(RefIndex,PrgMaxIndex, 1, RefMaxAddress, RefMinAddress + 1);
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

bool PackFile(int Index) {

    //----------------------------------------------------------------------------------------------------------
    //PROCESS FILE
    //----------------------------------------------------------------------------------------------------------

    FileUnderIO = Prgs[Index].FileIO;
    PrgAdd = Prgs[Index].iFileAddr;
    PrgLen = Prgs[Index].iFileLen;

    SL = new unsigned char[PrgLen] {};
    NL = new unsigned char[PrgLen] {};
    FL = new unsigned char[PrgLen] {};
    FSL = new unsigned char[PrgLen] {};
    FNL = new unsigned char[PrgLen] {};
    FFL = new unsigned char[PrgLen] {};
    SO = new int[PrgLen] {};
    NO = new int[PrgLen] {};
    FO = new int[PrgLen] {};
    FSO = new int[PrgLen] {};
    FNO = new int[PrgLen] {};
    FFO = new int[PrgLen] {};

    Seq = new sequence[PrgLen + 1]{};   //This is actually one element more in the array, to have starter element with 0 values

    //Initialize first element of sequence - WAS Seq[1]!!!
    //Seq[0].Len = 0;         //1 Literal byte, Len is 0 based
    //Seq[0].Off = 0;         //Offset = 0 -> literal sequence, Off is 1 based
    Seq[0].TotalBits = 10;  //LitLen bit + 8 bits, DO NOT CHANGE IT TO 9!!!

    //----------------------------------------------------------------------------------------------------------
    //SEARCH REFERENCE FILES FOR FAR MATCHES
    //----------------------------------------------------------------------------------------------------------

    CurrentFileIndex = Index;

    for (size_t RefIndex  = 0; RefIndex < VFiles.size(); RefIndex++)
    {
        ReferenceFileStart = VFiles[RefIndex].iFileAddr;
        SearchVirtualFile(PrgLen - 1, RefIndex, ReferenceFileStart + VFiles[RefIndex].iFileLen - 1, ReferenceFileStart + 1);
    }

    for (int RefIndex = 0; RefIndex < PartialFileIndex; RefIndex++)
    {
        ReferenceFileStart = Prgs[RefIndex].iFileAddr;
        SearchReferenceFile(PrgLen - 1, RefIndex, ReferenceFileStart + Prgs[RefIndex].iFileLen - 1, ReferenceFileStart + 1);
    }

    if (PartialFileIndex > -1)
    {
        //Search partial file from offset to end of file
        ReferenceFileStart = Prgs[PartialFileIndex].iFileAddr;
        SearchReferenceFile(PrgLen - 1, PartialFileIndex, ReferenceFileStart + Prgs[PartialFileIndex].iFileLen - 1, ReferenceFileStart + PartialFileOffset + 1);
    }

    //----------------------------------------------------------------------------------------------------------
    //CALCULATE BEST SEQUENCE
    //----------------------------------------------------------------------------------------------------------

    CalcBestSequence(PrgLen - 1, 1, true);     //SeqLowestIndex is 1 because Prg(0) is always 1 literal on its own, we need at lease 2 bytes for a match

    //----------------------------------------------------------------------------------------------------------
    //DETECT BUFFER STATUS AND INITIALIZE COMPRESSION
    //----------------------------------------------------------------------------------------------------------

    FirstLitOfBlock = true;                                     //First block of next file in same buffer, Lit Selector Bit NOT NEEEDED

    if (BytePtr == 255)
    {
        NextFileInBuffer = false;                               //This is the first file that is being added to an empty buffer
    }
    else
    {
        NextFileInBuffer = true;                                //Next file is being added to buffer that already has data
    }

    if (NewBundle)
    {
        TransitionalBlock = true;                               //New bundle, this is a transitional block
        //BlockPtr = ByteSt.Count                               //If this is a new bundle, store Block Counter Pointer
        BlockPtr = BufferCnt*256;                               //If this is a new bundle, store Block Counter Pointer
        NewBundle = false;
    }

    Buffer[BytePtr] = (PrgAdd + PrgLen - 1) % 256;              //Add Address Hi Byte
    AdLoPos = BytePtr;

    BlockUnderIO = CheckIO(PrgLen - 1,-1);   //Check if last byte of block is under IO or in ZP

    if (BlockUnderIO == 1)
    {
        BytePtr--;                                              //And skip 1 byte (=0) for IO Flag
    }

    Buffer[BytePtr - 1] = (PrgAdd + PrgLen - 1) / 256;          //Add Address Lo Byte
    AdHiPos = BytePtr - 1;

    BytePtr -= 2;
    LastByte = BytePtr;                                         //The first byte of the ByteStream after (BlockCnt and IO Flag and) Address Bytes (251..253)

    //----------------------------------------------------------------------------------------------------------
    //COMPRESS FILE
    //----------------------------------------------------------------------------------------------------------

    bool bSuccess = Pack();

    delete[] SL;
    delete[] SO;
    delete[] NL;
    delete[] NO;
    delete[] FL;
    delete[] FO;
    delete[] FSL;
    delete[] FSO;
    delete[] FNL;
    delete[] FNO;
    delete[] FFL;
    delete[] FFO;

    delete[] Seq;

    if (bSuccess)
    {
        return true;
    }
    else
    {
        return false;
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------
