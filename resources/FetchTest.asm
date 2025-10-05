
//TAB=4
//----------------------------
//	SPARKLE
//	Fetch Test
//	DRIVE CODE
//----------------------------

.const OPC_NOP_IMM	= $80
.const OPC_NOP_ABS	= $0c

#import "SD.sym"				//Import labels from SD.asm

.pseudopc $0200 {				//Stack pointer =#$00 at start

			lda #<OPC_NOP_IMM	//Disable copying last block to second buffer to avoid overwriting this code at $0200
			sta ChkScnd+2
			sta NOP1

			lda #<OPC_NOP_ABS
			sta NOP2
			sta NOP3

			lda #$05			//Motor off delay down to 0.1 seconds
			sta Frames+1

			lda #<FTHeader
			sta HeaderJmp
			lda #>FTHeader
			sta HeaderJmp+1
			
			lda #<FTData		//Redirect indirect JMP from DT to DTF after fetching data block
			sta DataJmp
			lda #>FTData
			sta DataJmp+1

			lda #$00			// A=Y=#$00
			jmp TestRet			// Reload first Dir Sector and fetch Track 1 Sector 0.

//--------------------------------------
//		Check Header
//--------------------------------------

FTHeader:	sta HCSErr			// Checksum mismatch if HCSErr != 0

FTChkTrk18:	lda cT
			cmp #$12
			bne FTID1
			lda HCSErr			// This will set/clear Z according to the checksum 
			jmp Header			// We are on/need track 18, return to standard operation

			//------------------

			bne FetchError		//5b 5c	33	Checksum mismatch
		
			lda TabC,x			//5d 5e	37	X=00CCCCC0
			eor (ZPTabDm2),y	//5f 60	43	Y=DDDDD010
			tay					//64	51	Y = ID1

			lda.z (ZP0103,x)	//65 66	57	$0103 (Sector)
			jsr ShufToRaw		//67-69	77
			cmp MaxNumSct2+1	//6a-6c	80
			bcs FetchError		//6d 6e	82	Sector mismatch
			sta cS				//6f 70	85

			lda $0102			//		93	A = $0102 (Track)
			jsr ShufToRaw		//		113
			beq FetchError		//		115	Track mismatch
			cmp #$29			//		117	Max track: 40 ($28)
			bcs FetchError		//		119	Track mismatch
			
			ldx $0101			//		123 X = $0101 (ID2)
			cpx ZPHdrID2		//		126
			bne CheckIDs		//		128	ID2 mismatch
			
			cpy ZPHdrID1		//		131
			bne CheckIDs		//		133	ID1 mismatch

			cmp cT				//		134 Everything else checked out, check if we are on the requested track
			bne ToCorrTrack		//		136
						
			tsx					//		138	SP=0 here
			stx NewTrackFlag	//		142	Clear NewTrackFlag only after header block is successfully fetched and verified

			ldy cS				//		145
			ldx WList,y			//		149	Otherwise, only fetch sector data if this is a wanted sector
BplFetch:	bpl ReFetch			//		151	Sector is not on wanted list -> fetch next sector header
ToFetchData:
			jmp FetchData		//		154	Sector is on wanted list -> fetch data
















			 
FTID1:		lda TabC,x			// X=00CCCCC0
			eor (ZPTabDm2),y	// Y=DDDDD010
			tay					// Y = ID1

			lda.z (ZP0103,x)	// $0103 (Sector)
			jsr ShufToRaw
			cmp MaxNumSct2+1
			bcc StoreSct		// Sector mismatch
			inc SctErr
StoreSct:	sta cS

			lda $0102			// A = $0102 (Track)
			jsr ShufToRaw
			bne ChkMaxTrk		// Track mismatch
			inc TrkErr

ChkMaxTrk:	cmp #$29			// Max track: 40 ($28)
			bcc ChkID2			// Track mismatch
			inc TrkErr

ChkID2:		ldx $0101			// X = $0101 (ID2)
			cpx ZPHdrID2
			beq ChkID1			// ID2 mismatch
			inc ID2Err
			
ChkID1:		cpy ZPHdrID1
			beq ChkTrk			// ID1 mismatch
			inc ID1Err
			
ChkTrk:		cmp cT				// Everything else checked out, check if we are on the requested track
			bne FTCorrTrack

SkipTrkErr:	tsx					// SP=0 here
			stx NewTrackFlag	// Clear NewTrackFlag only after header block is successfully fetched and verified

			ldy SctErr
			bne FTFetchData

			ldy cS
			ldx WList,y
			bpl FTFetchHeader
			inc SctWtd
FTFetchData:
			jmp FetchData		// Sector is on wanted list -> fetch data
FTFetchHeader:
			jmp Fetch

//--------------------------------------

FTCorrTrack:tay					//Save A (actual track)
			lda #$00
			ldx #$06
!:			ora HCSErr,x
			dex
			bpl !-
			tax
			beq ToCorr
			inc TrkErr
			bne FTFetchData		//If there is any error then abort track correction and continue on the current track
ToCorr:		ldx #$07
!:			sta HCSErr,x		//A=#$00
			dey
			bpl !-
			tya					//Restore actual track to A
			jmp CorrTrack		//Y can be anything

//--------------------------------------
//		Check Data
//--------------------------------------

FTData:		ldx cT
			cpx #$12
			bne	NotTrack18
			jmp Data

NotTrack18:	bcs SkipFTCSL  

			ldy #$7e			//CSLoop takes 851 cycles (33 bytes passing under R/W head in zone 3)
			bne FTCSLEntry
FTCSL:		eor $0102,y
			eor $0103,y
			dey
			dey
FTCSLEntry:
			eor $0180,y
			eor $0181,y
			dey
			dey
			bne FTCSL

SkipFTCSL:	tay
			beq DCSOK
			inc DCSErr

DCSOK:		ldy #$00
DVfLoop:	tya
			cmp $0100,y
			beq DOK
			inc DVfErr
			bne SkipDVf
DOK:		iny
			bne DVfLoop

			lda #<ErrVal
			sta ErrCtr			//Reset Error Counter (only if both header and data are fetched correctly)

SkipDVf:	txa					//Current Track
			ldx cS				//Current Sector in Buffer
			ldy WList,x
			bpl SkipWanted
			inc SctWtd
			
//--------------------------------------
//		Update Wanted List
//--------------------------------------

			sta WList,x			//Sector loaded successfully, mark it off on Wanted list (A=Current Track - always greater than zero)			
			dec SCtr			//Update Sector Coun

SkipWanted:	ldx #$06
			lda #$00
!:			ldy HCSErr,x
			cpy #$01
			rol
			dex
			bpl !-
			tax
			beq	DontAdd			//no error, don't add it to the error list
			
StoreY:		ldy #$00
			jsr ShufToRaw
			sta $0700,y
			iny
			lda cS
			jsr ShufToRaw
			sta $0700,y
			iny
			sty StoreY+1
	
DontAdd:

			jmp ChkLastBlock
			
//--------------------------------------

HCSErr:
.byte $00
SctErr:
.byte $00
TrkErr:
.byte $00
ID2Err:
.byte $00
ID1Err:
.byte $00

DCSErr:
.byte $00
DVfErr:
.byte $00
SctWtd:
.byte $00
}