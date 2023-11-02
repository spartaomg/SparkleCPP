//TAB=4
//----------------------------
//	SPARKLE
//	Fetch and Transfer Test
//	DRIVE CODE
//----------------------------

#import "SD.sym"				//Import labels from SD.asm

.const OPC_NOP_IMM	= $80
.const OPC_NOP_ABS	= $0c
.const ZPSum		= $68		//Save checksum on ZP
.const ZPTmp		= $69

*=$2b00	"Drive Save Code"

.pseudopc $0100	{				//Stack pointer =#$00 at start

			lda #<OPC_NOP_IMM	//Disable copying last block to second buffer to avoid overwriting this code at $0200
			sta ChkScnd+2
			sta NOP1
			sta NOP3

			lda #<OPC_NOP_ABS
			sta NOP2

			lda #$05			//Motor off delay down to 0.1 seconds
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

DTF:		stx X+1				//Save X - will be used to retrieve checksum (final read in GCR loop)
			sty Y+1				//Save Y - will be used to retrieve checksum (final read in GCR loop)
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
			lda #$40			//Bad checksum - mark sector with %01000000
!:			ora cS
			ldx VerifCtr
			beq Ctr
			ora #$80			//Sector check during spinup - mark sector with %10000000
Ctr:		ldx #$00
			sta SectorTable,x	//Add current sector to Sector Table: $0x = RegGood, #$4x = RegBad, $8x = VerifGood, $cx = VerifBad
			inx
			stx Ctr+1

			tya
			ora VerifCtr		//ORA must be done before LSR!!!
			beq TransferBlock	//If (checksum == 0) && (VerifCtr == 0) -> transfer regular block

			tya
			bne !+				//Bad checksum -> fetch next sector
			lsr VerifCtr		//Good checksum -> decrement VerifCtr

!:			cpx MaxSct2+1
			beq TransferBlock	//SectorTable is full -> transfer meta block

			jmp Fetch			//Otherwise just fetch next sector

//------------------------------

TransferBlock:
			lax Ctr+1
			axs #$fa			//X=(sector count in SectorTable)+6, number of bytes we need to "un-EOR" from final checksum ($0100-$0106 + sector list)

X:			lda TabC
Y:			ldy #$00
			eor TabD,y			//Retrieve original checksum (read #257 in GCR loop)
			tay					//Save GCR loop checksum to Y

								//CSum|Stack    |BCnt|Ctr |Trck|CVer|Sctr list
!:			eor $0100,x			//0100|0101-0102|0103|0104|0105|0106|0107......
			dex
			bpl !-
			sta ZPSum

			cpy #$7f
			bne DEX				//If original checksum != #$7f then last byte of buffer is block count, skip it in the loop to avoid false positives
!:			txa
			eor $0100,x
			bne !+
DEX:		dex
			bne !-
			lda $0100			//Value should be #$00 (EOR check is not needed)
			eor cS

!:			ldx #$02
			txs					//Change stack pointer to avoid overwriting important data in block

			jsr ShufToRaw
			sta $0106			//Data block verification = $00 -> checksum is OK, any other values -> false positive checksum ($03fa)

			lda cT
			jsr ShufToRaw
			sta $0105			//Current Track	($03fb)

			ldy Ctr+1
			tya
			jsr ShufToRaw
			sta $0104			//Sector Table Counter	($03fc)

!:			lda SectorTable-1,y
			jsr ShufToRaw
			sta $0107-1,y		//Sector Table ($03fa, $03f9, ..)
			dey
			bne !-

			lda BlockCtr
			ldy Ctr+1			//Keep Ctr+1 in Y
			cpy MaxSct2+1		//Regular block: C=0, Meta block: C=1
			adc #$ff			//Regular block: decrement block count, Meta block: do not decrement block count
			jsr ShufToRaw
			sta $0103			//Block count ($03fd)

			tya
			tax
			axs #$fb
			lda ZPSum
!:			eor $0101,x
			dex
			bpl !-
			sta $0100			//Final checksum

			inx
			stx Ctr+1			//Clear Sector Counter
			cpy MaxSct2+1
			beq MetaBlock

			ldy #$00			//RegBlockRet needs Y=#$00

GoodBlock:	jmp RegBlockRet		//Continue with regular block - will be transferred normally

//------------------------------

MetaBlock:	jmp CheckATN

//-----------------------------------------------------------------------------------------------------------------------------------------------------------

SectorTable:
}
