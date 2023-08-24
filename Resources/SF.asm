//TAB=4
//----------------------------
//	Sparkle 2
//	Fetch Test
//	DRIVE CODE
//----------------------------

#import "SD.sym"				//Import labels from SD.asm

.const OPC_NOP_IMM	= $80

*=$2b00	"Drive Save Code"

.pseudopc $0200	{				//Stack pointer =#$00 at start

			lda #<OPC_NOP_IMM	//Disable copying last block to second buffer to avoid overwriting this code at $0200
			sta ChkScnd+2

			lda #$05			//Motor delay down to 0.1 seconds
			sta Frames+1

			lda #<DTF			//Redirect indirect JMP from DT to DTF after fetching data block
			sta DataJmp
			lda #>DTF
			sta DataJmp+1

			ldy #$00			//Copy test code from stack to $0200
SF_CopyLoop:
			pla
			iny
			sta $0200,y
			bne SF_CopyLoop
		
			tya					// A=Y=#$00
			jmp TestRet			// Reload first Dir Sector and fetch Track 1 Sector 0.

//--------------------------

ToTrack18:	jmp SkipCSLoop		//Track 18 - return to drive code to fetch dir sector

//--------------------------

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

			lda cT
			jsr ShufToRaw
			sta $0180			//Current Track	($0380)

			ldy Ctr+1
			tya
			jsr ShufToRaw
			sta $017f			//Sector Table Counter ($0381)

!:			lda SectorTable-1,y
			jsr ShufToRaw
			sta $01c0-1,y		//Sector Table ($0340, $033f, ..)
			dey
			bne !-
			sty Ctr+1			//Reset Ctr

			lda BlockCtr
			sec
			sbc #01				//Decrement block count
			jsr ShufToRaw
			sta $017e			//Block count ($0382)
			
			jmp RegBlockRet		//Continue with regular block - will be transferred normally
								//TODO: only transfer 4 bytes if there is a checksum error???

//------------------------------

MetaBlock:	ldx #$10
			txs					//Change stack pointer to avoid overwriting important data in block

			tya
			bne !+
			lsr VerifCtr		//Decrement VerifCtr if checksum is good

!:			lda cT
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
			jsr ShufToRaw
			sta $017e			//Block count ($0382)

			jmp CheckATN		//TODO: only transfer 4 bytes???

SectorTable:
}
