//TAB=4
//----------------------------
//	Sparkle 2
//	Fetch Test
//	DRIVE CODE
//----------------------------

#import "SD.sym"				//Import labels from SD.asm

.const OPC_NOP_IMM	= $80
.const OPC_NOP_ABS	= $0c
.const ZPSum		= $68		//Save checksum on ZP

*=$2b00	"Drive Save Code"

.pseudopc $0100	{				//Stack pointer =#$00 at start

			lda #<OPC_NOP_IMM	//Disable copying last block to second buffer to avoid overwriting this code at $0200
			sta ChkScnd+2
			sta NOP1
			sta NOP3

			lda #<OPC_NOP_ABS
			sta NOP2

			lda #$05			//Motor delay down to 0.1 seconds
			sta Frames+1

			lda #<DTF			//Redirect indirect JMP from DT to DTF after fetching data block
			sta DataJmp
			lda #>DTF
			sta DataJmp+1

			ldy #$00			//Copy test code from stack to $0200
SF_CopyLoop:
			lda CopyFromHere,y
			sta $0200,y
			iny
			bne SF_CopyLoop
		
			tya					// A=Y=#$00
			jmp TestRet			// Reload first Dir Sector and fetch Track 1 Sector 0.
CopyFromHere:
}

//--------------------------

.pseudopc $0200	{

ToTrack18:	jmp SkipCSLoop		//Track 18 - return to drive code to fetch dir sector

//--------------------------

DTF:		stx X+1
			sty Y+1
			ldx cT				//A=checksum (partial in tracks 1-17)
			cpx #$12
			beq ToTrack18
			bcs SF_SkipCSLoop  
			ldx #$7e			//Tracks 1-17 - complete checksum here
			bne SF_CSLoopEntry
SF_CSLoop:	eor $0102,x
			eor $0103,x
			dex
			dex
SF_CSLoopEntry:
			eor $0180,x
			eor $0181,x
			dex
			dex
			bne SF_CSLoop
SF_SkipCSLoop:

//------------------------------

			tay					//Final checksum in Y
			beq !+
			lda #$40
!:			ora cS
			ldx VerifCtr
			beq Ctr
			ora #$80
Ctr:		ldx #$00
			sta SectorTable,x	//Add current sector to Sector Table: $0x = RegGood, #$4x = RegBad, $8x = VerifGood, $cx = VerifBad
			inx
			stx Ctr+1

			tya
			ora VerifCtr
			beq TransferBlock	//If (checksum == 0) && (VerifCtr == 0) -> transfer regular block

			tya
			bne !+
			lsr VerifCtr		//If (checksum == 0) -> decrement VerifCtr

			cpx MaxSct2+1
			beq TransferBlock	//SectorTable is full -> transfer meta block

!:			jmp Fetch			//Otherwise just fetch next block

//------------------------------

TransferBlock:
			lda Ctr+1
			clc
			adc #$05
			tax
X:			lda TabC
Y:			ldy #$00
			eor TabD,y			//CSum Stack     BCnt Ctr  Trck Sctr list
!:			eor $0100,x			//0100|0101-0102|0103|0104|0105|0106...
			dex
			bpl !-
			sta ZPSum

			ldx #$02
			txs					//Change stack pointer to avoid overwriting important data in block

			lda cT
			jsr ShufToRaw
			sta $0105			//Current Track	($03fb)

			ldy Ctr+1
			tya
			jsr ShufToRaw
			sta $0104			//Sector Table Counter	($03fc)

!:			lda SectorTable-1,y
			jsr ShufToRaw
			sta $0106-1,y		//Sector Table ($03fa, $03f9, ..)
			dey
			bne !-

			lda BlockCtr
			ldy Ctr+1			//Keep Ctr+1 in Y
			cpy MaxSct2+1		//Regular block: C=0, Meta block: C=1
			adc #$ff			//Regular block: decrement block count, Meta block: do not decrement block count
			jsr ShufToRaw
			sta $0103			//Block count ($03fd)

			tya
			clc
			adc #$04
			tax
			lda ZPSum
!:			eor $0101,x
			dex
			bpl !-
			sta $0100			//Final checksum

			inx
			stx Ctr+1
			cpy MaxSct2+1
			beq MetaBlock

			ldy #$00			//RegBlockRet needs Y=#$00

GoodBlock:	jmp RegBlockRet		//Continue with regular block - will be transferred normally

//------------------------------

MetaBlock:	jmp CheckATN

//-----------------------------------------------------------------------------------------------------------------------------------------------------------



/*
DTF:		ldx cT				//A=checksum (partial in tracks 1-17)
			cpx #$12
			beq ToTrack18
			bcs SF_SkipCSLoop  
			ldx #$7e			//Tracks 1-17 - complete checksum here
			bne SF_CSLoopEntry
SF_CSLoop:	eor $0102,x
			eor $0103,x
			dex
			dex
SF_CSLoopEntry:
			eor $0180,x
			eor $0181,x
			dex
			dex
			bne SF_CSLoop
SF_SkipCSLoop:

//------------------------------

			tay					//Final checksum in Y
			beq !+
			lda #$40
!:			ora cS
			ldx VerifCtr
			beq Ctr
			ora #$80
Ctr:		ldx #$00
			sta SectorTable,x	//Add current sector to Sector Table: $0x = RegGood, #$4x = RegBad, $8x = VerifGood, $cx = VerifBad
			inx
			stx Ctr+1

			tya
			ora VerifCtr
			beq GoodBlock		//If (checksum == 0) && (VerifCtr == 0) -> transfer regular block

			cpx MaxSct2+1
			beq MetaBlock		//SectorTable is full -> transfer meta block

			tya
			bne !+
			lsr VerifCtr		//If (checksum == 0) -> decrement VerifCtr

!:			jmp Fetch			//Fetch next block

//------------------------------

GoodBlock:	ldx #$10
			txs					//Change stack pointer to avoid overwriting important data in block

			jsr Sub

			sec
			sbc #01				//Decrement block count
			
			jsr SumBlockCnt
			
			jmp RegBlockRet		//Continue with regular block - will be transferred normally
								//TODO: only transfer 4 bytes if there is a checksum error???

//------------------------------

MetaBlock:
			tya
			bne !+
			lsr VerifCtr		//Decrement VerifCtr if checksum is good
			
			ldx #$10
			txs					//Change stack pointer to avoid overwriting important data in block

!:			jsr Sub

			jsr SumBlockCnt

			jmp CheckATN		//TODO: only transfer 4 bytes???

//------------------------------

Sub:
			lda cT
			jsr ShufToRaw
			sta $0180			//Current Track	($0380)

			ldy Ctr+1
			tya
			jsr ShufToRaw
			sta $017f			//Sector Table Counter	($0381)

!:			lda SectorTable-1,y
			jsr ShufToRaw
			sta $01c0-1,y		//Sector Table ($0340, $033f, ..)
			dey
			bne !-
			sty Ctr+1			//Reset Ctr

			lda BlockCtr
			rts

SumBlockCnt:
			jsr ShufToRaw
			sta $017e			//Block count ($0382)

			ldx #$0f
			lda #$00
!:			eor $0100,x
			eor $0120,x
			eor $0140,x
			eor $0160,x
			eor $0180,x
			eor $01a0,x
			eor $01c0,x
			eor $01e0,x
			eor $0110,x
			eor $0130,x
			eor $0150,x
			eor $0170,x
			eor $0190,x
			eor $01b0,x
			eor $01d0,x
			eor $01f0,x
			dex
			bpl !-
			eor $0100
			sta $0100

			rts
*/
SectorTable:
}
