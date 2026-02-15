//TAB=4
.const DriveNo		=$fb
.const DriveCt		=$fc

.const Sp			=$52		//Spartan Stepping constant
.const InvSp		=Sp^$ff		//#$ae

.label ZPDst		=$02		//$02/$03
.label Bits			=$04

.const busy			=$f8		//DO NOT CHANGE IT TO #$FF!!!
.const ready		=$08		//AO=1, CO=0, DO=0 on C64 -> $1800=#90
.label sendbyte		=$18		//AO-1, CO=1, DO=0 on C64 -> $1800=$94
.const drivebusy	=$12		//AA=1, CO=0, DO=1 on Drive

.const Buffer		=$0300

.const Listen		=$ed0c
.const ListenSA		=$edb9
.const Unlisten		=$edfe
.const SetFLP		=$fe00
.const SetFN		=$fdf9
.const Open			=$ffc0

.const LDA_ABSY		=$b9
.const ORA_ABSY		=$19
.const AND_ABSY		=$39
.const NTSC_CLRATN	=$c0
.const NTSC_DD00_1	=$dd00-ready
.const NTSC_DD00_2	=$dd00-NTSC_CLRATN
.const NTSC			=Read1+2

//C64
//Write	 0  0  X  X  X  0  0  0
//Read	 X  X  0  0  0  1  X  X
//dd00	80 40 20 10 08 04 02 01		Value after C64 reset:   #$97 = 10010111 (DI=1 CI=0 DO=0 CO=1 AO=0)
//		DI|CI|DO|CO|AO|RS|VICII		Value after drive reset: #$c3 = 11000011 (DI=1 CI=1 DO=0 CO=0 AO=0)

//Drive
//1800	80 40 20 10 08 04 02 01		Value after C64 reset:   #$04 = 00000100 (D0=0 CO=0 DI=0 CI=1 AI=0 AA=0)
//		AI|DN|DN|AA|CO|CI|DO|DI		Value after drive reset: #$00 = 00000000 (D0=0 CO=0 DI=0 CI=0 AI=0 AA=0)

*=$0801 "Basic"						//Prg starts @ $0810
BasicUpstart(Start)

*=$0810 "Installer"

Start:		lda #$ff				//Check IEC bus for multiple drives
			sta DriveCt
			ldx #$04
			lda #$08
			sta $ba

DriveLoop:	lda $ba
			jsr Listen
			lda #$6f
			jsr ListenSA			//Return value of A=#$17 (drive present) vs #$c7 (drive not present)
			bmi SkipWarn			//check next drive # if not present

			lda $ba					//Drive present
			sta DriveNo				//This will be the active drive if there is only one drive on the bus
			jsr Unlisten
			inc DriveCt
			beq SkipWarn			//Skip warning if only one drive present

			lda $d018				//More than one drive present, show warning
			bmi Start				//Warning is already on, start bus check again

			ldy #$03
			ldx #$00
			lda #$20
ClrScrn:	sta $3c00,x				//Clear screen RAM @ $3c00
			inx						//JSR $e544 does not work properly on old Kernal ROM versions
			bne ClrScrn
			inc ClrScrn+2
			dey
			bpl ClrScrn

			ldx #<WEnd-Warning-1
TxtLoop:	lda Warning,x			//Copy warning
			sta $3db9,x
			lda $286				//Foreground color
			sta $d9b9,x				//Needed for old Kernal ROMs
			dex
			bpl TxtLoop

			lda #$f5				//Screen RAM: $3c00
			sta $d018
			bmi Start				//Warning turned on, start bus check again

SkipWarn:	inc $ba
			dex
			bne DriveLoop

			//Here, DriveCt can only be $ff or $00

			lda #$15				//Restore Screen RAM to $0400
			sta $d018

			lda DriveCt
			beq ChkDone				//One drive only, continue

			ldx #<NDWEnd-NDW
NDLoop:		lda NDW-1,x				//No drive, show message and finish
			jsr $ffd2
			dex
			bne NDLoop
			stx $0801				//Delete basic line to force reload
			stx $0802
			rts

//----------------------------

ChkDone:	ldx #<Cmd
			ldy #>Cmd
			lda #CmdEnd-Cmd
			jsr SetFN				//Filename = drive install code in command buffer

			lda #$0f
			tay
			ldx DriveNo
			jsr SetFLP				//Logical parameters
			jsr Open				//Open vector

			sei

			lda #$35
			sta $01

			ldx #$5f
			txs						//Loader starts @ $160, so reduce stack to $100-$15f

			inx
			stx LTo+1
			txa
			clc
			adc #<LoaderCode - $60
			sta LCopyLoop+1
			lda #>LoaderCode - $60
			adc #$00
			sta LCopyLoop+2

			lda #$3c				// 0  0  1  1  1  1  0  0
			sta $dd02				//DI|CI|DO|CO|AO|RS|VC|VC
			ldx #$00				//Clear the lines
			stx $dd00

LCopyLoop:	lda LoaderCode,x
LTo:		sta $0160,x
			lda LoaderCode+$a0,x
			sta $0200,x
			inx
			bne LCopyLoop

//----------------------------------
//		NTSC fix
//----------------------------------
		
NextLine:	lda $d012				//Based on J0x's solution for NTSC detection from CodeBase64.org
SameLine:	cmp $d012
			beq SameLine
			bmi NextLine
			cmp #$20
			bcs SkipNTSC

			lda #<NTSC_DD00_2
			sta Read2+1
			lda #>NTSC_DD00_1		//=NTSC_DD00_2
			sta Read2+2
			ldx #$02
NTSCLoop:	sta Read1,x
			sta Read3,x
			sta Read4,x
			lda #<NTSC_DD00_1
			dex
			bne NTSCLoop
			lda #LDA_ABSY
			sta Read1
			lda #ORA_ABSY
			sta Read2
			sta Read3
			lda #AND_ABSY
			sta Read4

SkipNTSC:
//----------------------------------

			lda #busy				//=#$f8
			bit $dd00				//Wait for "drive busy" signal (DI=0 CI=1 dd00=#$4b)
			bmi *-3
			sta $dd00				//Lock bus

			//First loader call, returns with I=1

			lda #>$10ad				//#>PrgStart-1 (Hi Byte)
			pha
			lda #<$10ad				//#<PrgStart-1 (Lo Byte)
			pha
			jmp Sparkle_LoadFetched	//Load first Bundle, it may overwrite installer, so we use an alternative approach here

//-----------------------------------------------------------------------------------

Warning:
       //0123456789012345678901234567890123456789
.text	 "sparkle supports only one active drive "
.text	"pls turn off everything else on the bus!"
WEnd:
NDW:
      //0123456789012345678901234567890123456789
//.text	"please turn your drive on and load again"
.byte	$4e,$49,$41,$47,$41,$20,$44,$41,$4f,$4c,$20,$44
.byte	$4e,$41,$20,$4e,$4f,$20,$45,$56,$49,$52,$44,$20,$52,$55,$4f,$59
.byte	$20,$4e,$52,$55,$54,$20,$45,$53,$41,$45,$4c,$50
//.text	"niaga daol dna no evird ruoy nrut esaelp"
NDWEnd:

//-----------------------------------------------------------------------------------

Cmd:
//Load drive code blocks into buffers 0-4

.const ROMLoadBAM	= $c647
.const ROMSeekBlock	= $d038
.const ROMReadBlock	= $d586

.byte	'M','-','E',$05,$02			//-0204 Command buffer: $0200-$0228
			
!:			jsr ROMLoadBAM			//-0207 always start on track 18 by loading BAM - this will also initialize ZP variables (disk ID, current track)
			bne !-					//-0209 if error then try again
			lda #$03				//-020b
			sta $f9					//-020d buffer #3
			ldx #$0f				//-020f
			stx $81					//-0211
			inx						//-0212
			jsr ROMSeekBlock		//-0215 find track 16 sector 15, to make sure stepper bits are aligned
			lsr						//-0216
			bne !-					//-0218 if error then start over
			lda #$12				//-021a
			sta $0c					//-021c success, back to track 18
!:			jsr ROMReadBlock		//-021f load track 18 sector 15 (init code) to buffer #3 ($0600)
			lsr						//-0220
			bne !-					//-0222 if error then try block again
			jmp $0600				//-0225
CmdEnd:

//-----------------------------------------------------------------------------------

//----------------------------
//	C64 RESIDENT CODE
//	$0160-$02ff
//----------------------------

LoaderCode:

*=LoaderCode "Loader"

.pseudopc $0160 {

//----------------------------
//		FALLBACK IRQ
//		Address: $0160
//----------------------------

Sparkle_IRQ:
			pha
			txa
			pha
			tya
			pha
			lda $01
			pha

			jsr Set01
			inc $d019
Sparkle_IRQ_JSR:
			jsr Done				//Music player or IRQ subroutine, installer @ $02d1

			pla
			sta $01
			pla
			tay
			pla
			tax
			pla
Sparkle_IRQ_RTI:
			rti

//----------------------------

Sparkle_SendCmd:
			sta Bits				//Store Bundle Number on ZP
			jsr Set01				//$dd00=#$3b, $1800=#$95, Bus Lock, A=#$35
SS_Send:	ldx #sendbyte			//CO=1, AO=1 => C64 is ready to send a byte, X=#$18
			stx $dd00				//Signal to Drive that we want to send Bundle Index
			bit $dd00				//$dd00=#$9b, $1800=#$94
			bmi *-3					//Wait for Drive response ($1800->00 => $dd00=#$1b, $1800=#$85)

									//Sending bits via AO, flipping CO to signal new bit, C can be anything at start
BitSLoop:	adc #$e7				//2 A=#$35+#$e7+C=#$1c/#$1d A&X=#$18 and C=1 after addition in first pass, C=0 in all other passes
			sax $dd00				//4 subsequent passes: A&X=#$00/#$08/#$10/#$18 and C=0
			and #$10				//2 Clear AO
			eor #$10				//2 A=#$18 in first pass (CO=1, AO=1) reads #$85 on $1800 - no change, first pass falls through
			ror Bits				//5 C=1 in first pass, C=0 in all other passes before ROR
			bne BitSLoop			//3
									//18 cycles/bit - drive loop needs 17 cycles/bit (should work for both PAL and NTSC)

BusLock:	lda #busy				//2 (A=#$f8) worst case, last bit is read by the drive on the first cycle of LDA
			sta $dd00				//4 Bus lock

			rts						//6

//----------------------------

Sparkle_LoadA:
			jsr Sparkle_SendCmd

//----------------------------

Sparkle_LoadFetched:
			jsr Set01				//17
			ldx #$00				//2
			ldy #ready				//2 Y=#$08, X=#$00
			sty $dd00				//4 Clear CO and DO to signal Ready-To-Receive
			bit $dd00				//Wait for Drive
			bvs *-3					//$dd00=#$cx - drive is busy, $0x - drive is ready	00,01	(BMI would also work)
			stx $dd00				//Release ATN										02-05
			inx						//To compensate for first DEX in loop
			jsr Set01				//Waste a few cycles... (drive takes 16 cycles)		06-28 (minimum needed here is 06-15, 10 cycles, including INX!!!)

//-------------------------------------
//
//		RECEIVE LOOP
//
//-------------------------------------

RcvLoop:
Read1:		lda $dd00				//4		W1-W2 = 18 cycles							25-28
			sty $dd00				//4 8	Y=#$08 -> ATN=1
			lsr						//2 10
			lsr						//2 12
			dex						//2 14
			nop						//2 16
			ldy #$c0				//2 (18)

Read2:		ora $dd00				//4		W2-W3 = 16 cycles
			sty $dd00				//4 8	Y=#$C0 -> ATN=0
			lsr						//2 10
			lsr						//2 12
SpComp:		cpx #<$101-Sp			//2 14	X=#$AF/#$01
			beq ChgJmp				//2/3	16/17 with branch --------------|
RcvCont:	ldy #$08				//2 (18/28) ATN=1						|
									//										|
Read3:		ora $dd00				//4		W3-W4 = 17 cycles				|
			sty $dd00				//4 8	Y=#$08 -> ATN=1					|
			lsr						//2 10									|
			lsr						//2 12									| C=1 here
			sta LastBits+1			//4 16									|
			lda #$c0				//2 (18)								|
									//										|
Read4:		and $dd00				//4		W4-W1 = 16 cycles				|
			sta $dd00				//4 8	A=#$X0 -> ATN=0					|
LastBits:	ora #$00				//2 10									|
			sta Buffer,x			//5 15									|
JmpRcv:		bvc RcvLoop				//3 (18)								|
									//										|
//----------------------------												|
									//										|
ChgJmp:		ldy #<SpSDelay-<ChgJmp	//2 19	<-------------------------------|
			sty JmpRcv+1			//4 23
			bne RcvCont				//3 26 BRANCH ALWAYS

//----------------------------
//		FAR MATCH					//A=#$00-#$7c, N=0
//----------------------------

FarMatch:	beq Sparkle_LoadFetched	//A=#$00 -> End of Block, load next one
			dex						//A=#$0c-#$7c ($08 is not used, $04 - long match tag)
			ldy Buffer,x			//=OffsetHi
			bcc FarConv

//----------------------------
//		LONG MATCH					//A=#$00-#$01
//----------------------------

LongMatch:	lsr
			bcc NextFile			//A was #$00 -> Next File in block, C=0
			dex						//A was #$01 -> Long Match, C=1
			lda Buffer,x			//Read next byte for Match Length (#$20-#$ff) or End of Bundle (#$00)
			bne LongConv			//Converge with Mid Match (C=1)

//----------------------------
//		END OF BUNDLE
//----------------------------

EndOfBundle:
Delay:		dex						//Or finish bundle (A=#$00) - Also used as Delay for LoadFetched and Set01
			stx Buffer+$ff			//This write doesn't make any difference in Delay, but used in EndOfBundle
Set01:		lda #$35
			sta $01
Done:		rts

//----------------------------
//		SPARTAN STEP DELAY
//----------------------------

SpSDelay:	lda #<RcvLoop-<ChgJmp	//2 20 Restore Receive loop
			sta JmpRcv+1			//4 24
			txa						//2 26
			eor #<$100-Sp			//2 28 Invert byte counter (EOR #$AE)
			sta SpComp+1			//4 32 SpComp+1=(#$AF <-> #$01)
			bpl RcvLoop				//3 (35) (Drive loop takes 34 cycles)

			lda #busy				//Bus lock
			sta $dd00

//------------------------------------------------------------
//		BLOCK STRUCTURE FOR DEPACKER
//------------------------------------------------------------
//		$00		- First Bitstream byte -> will be changed to #$00 (end of block)
//		$01		- last data byte vs #$00 (block count on drive side)
//		$ff		- Dest Address Lo
//		($fe	- IO Flag)
//		$fe/$fd	- Dest Address Hi
//		$fd/$fc	- Byte stream backwards with bit stream interleaved
//------------------------------------------------------------

Sparkle_LoadNext:					//Entry point for next bundle in block
			ldx #$ff
			stx MidLitSeq+1

			inx
GetBits:	lda Buffer,x			//First bitstream value
			bne StoreBits
			ldx Buffer+$ff			//=LastX
			bne GetBits				//BRA
StoreBits:	sta Bits				//Store it on ZP for faster processing

NextFile:	dex						//Entry point for next file in block, C must be 0 here for subsequent files
			lda Buffer,x			//Lo Byte of Dest Address
			sta ZPDst

			ldy #$35				//Default value for $01, IO=on
			dex
			lda Buffer,x			//Hi Byte vs IO Flag=#$00
			bne SkipIO
			dex
			lda Buffer,x			//This version can also load to zeropage!!!
			beq SkipIO				//We are loading to ZP -> do not turn off IO!!!
			dey						//Y=#$34, turn IO off

SkipIO:		sta ZPDst+1				//Hi Byte of Dest Address
			sty $01					//Update $01

			dex

			ldy #$00				//Needed for Literals
			sty Buffer

			jmp LitCheck

//----------------------------
//	DECODING MATCH BYTES
//----------------------------
//					X-3		 X-2	  X-1	   X		OFFSET (STORED AS)	LENGTH (STORED AS)
//	SHORT:									ooooooLL	$01-$40 ($00-$3F)	$02-$04 ($01-$03)
//	NEAR MID:				       oooooooo 1LLLLL00	$01-$FF ($01-$FF)	$02-$1F ($02-$1F)
//	NEAR LONG:			  oooooooo LLLLLLLL 10000100	$01-$FF ($01-$FF)	$20-$FF ($20-$FF)
//	FAR MID:		      oooooooo HHHHHHHH 0LLLLL00	$0100-$FFFF			$03-$1F ($03-$1F)
//	FAR LONG:	 oooooooo LLLLLLLL HHHHHHHH 00000100	$0100-$FFFF			$20-$FF ($20-$FF)
//
//	END OF BLOCK:							00000000
//	END OF FILE:							10000000
//	END OF BUNDLE:				   00000000 10000100	(USE 00001000 instead??? far mid length of $02 is not used)
//														(far match check would take 4 more cycles...)
//														Not worth it - would save max 64 bytes per disk side...
//														...and lose 4 cycles per far match (thousands per disk side)
//	L - length
//	o - Offset LO
//	H - Offset HI

//----------------------------
//		MID MATCH
//----------------------------

MidMatch:	lda Buffer,x			//C=0
			bpl FarMatch			//HLLLLL00 => $00 - Next Block, N=1 - Near Match, N=0 - Far Match, $x4 - Long Match, $80 - Next File
FarConv:	sty MatchHi+1			//=#$00 vs OffsetHi
			alr #$7c				//Clear MSB (Near Match Flag)
			lsr
			cmp #$02
			bcc LongMatch			//$00 -> Next File or Block, $01 -> Long Match
LongConv:	tay						//Mid Match Length=#$02-#$1f, Long Match Length=#$20-#$ff (both 1-based)
			eor #$ff
			adc ZPDst				//C=1 here, add +1, as Mid and Long Matches are 1-based, Y will not be increased
			sta ZPDst
			bcs MidMCont			//This could be branched off to save 1 cycle here, would need 2 more free bytes...
			dec ZPDst+1
			sec
MidMCont:	dex
			adc Buffer,x			//Match Offset=$00-$ff+(C=1)=$01-$100
			sta MatchCopy+1			//MatchCopy+1=ZP+(Buffer)+(C=1)
MatchHi:	lda #$00				//=#$00 vs. OffsetHi
			jmp MidConv				//46-50 cycles

//----------------------------
//		LITERALS
//----------------------------

NextBit:	lda Buffer,x			//C=1, Z=1, Bits=#$00, token bit in C, update Bits
			rol
			sta Bits

LongLit:	dex						//Saves 1 byte and adds 2 cycles per LongLit sequence, C=0 for LongLit
			bcs MidLitSeq			//C=1, we have more than 1 literal, LongLit (C=0) falls through

ShortLit:	tya						//Y=00, C=0
MidLit:		iny						//Y+Lit-1, C=0
			sty SubX+1				//Y+Lit, C=0
			eor #$ff				//ZP=ZP+(A^#$FF)+(C=1) = ZP=ZP-A (e.g. A=#$0e -> ZP=ZP-0e)
			adc ZPDst
			sta ZPDst
			bcc ShortLHi

ShortLCont:	txa
SubX:		axs #$00				//X=X-1-Literal (e.g. Lit=#$00 -> X=A-1-0)
			stx LitCopy+1

LitCopy:	lda Buffer,y
			sta (ZPDst),y
			dey
			bne LitCopy				//Literal sequence is ALWAYS followed by a match sequence

//----------------------------
//		SHORT MATCH
//----------------------------

Match:		lda Buffer,x
			anc #$03
			beq MidMatch			//C=0

ShortMatch:	tay						//Short Match Length=#$01-#$03 (corresponds to a match length of 2-4)
			iny						//Only Short Match Lengths are 0-based, Mid and Long Match Lengths are 1-based
			eor #$ff
			adc ZPDst
			sta ZPDst
			bcc ShortMHi

ShortMCont:	lda Buffer,x			//Short Match Offset=($00-$3f)+1=$01-$40
			lsr
			lsr
			sec
			adc ZPDst
			sta MatchCopy+1			//MatchCopy+1=ZP+(Buffer)+(C=1)
			lda #$00				//This is always #$00 here, no need to reset

MidConv:	adc ZPDst+1
			sta MatchCopy+2			//C=0 after this
			dex						//DEX needs to be after MidConv - 55-59 cycles here min. from MidMatch
MatchCopy:	lda $10ad,y				//Y=#$02-#$04 (short) vs #$02-#$1f (mid) vs #$20-#$ff (long)
			sta (ZPDst),y
			dey
			bne MatchCopy

//----------------------------
//		BITCHECK					//Y=#$00 here
//----------------------------

BitCheck:	asl Bits
			bcc LitCheck			//C=0, literals
			bne Match				//C=1, Z=0, this is a match (control bits: 1)

//----------------------------

			lda Buffer,x			//C=1, Z=1, Bits=#$00, token bit in C, update Bits
			dex
			rol
			sta Bits
			bcs Match

//----------------------------

LitCheck:	asl Bits
			bcc ShortLit			//C=0, we have 1 literal (control bits: 00)
			beq NextBit				//C=1, Z=1, this is the token bit in C (Bits=#$00), get next bit stream byte

//----------------------------
//		LITERALS 2-16
//----------------------------

MidLitSeq:	ldy #$f8				//C=1, more than 1 literal (control bits: 01)
			bpl SkipML
			ldy Buffer,x
			tya
			dex
			alr #$f0				//0xxxx000
			lsr						//00xxxx00
			lsr						//000xxxx0		C=0 after this -> 
SkipML:		ror						//0000xxxx vs 1xxxxxxx	C=0, N=0 vs N=1 here depending on the branch taken
			sta MidLitSeq+1
			tya
			anc #$0f
			tay
			bne MidLit

//----------------------------
//		LITERALS 17-251
//----------------------------

			ldy Buffer,x			//Literal lengths 17-251 (Bits: 11|0000|xxxxxxxx)
			bcc LongLit				//ALWAYS, C=0, we have 17-251 literals

//----------------------------

ShortLHi:	dec ZPDst+1
			bcc ShortLCont

//----------------------------

ShortMHi:	dec ZPDst+1
			bcc ShortMCont

/*
//----------------------------
//		IRQ INSTALLER
//		Call: jsr $02d1
//		X/Y=Player Hi/Lo
//		A=Raster
//----------------------------

Sparkle_InstallIRQ:
			sty Sparkle_IRQ_JSR+1	//Installs a subroutine vector
			stx Sparkle_IRQ_JSR+2
Sparkle_RestoreIRQ:
			sta $d012				//Sets raster for IRQ
			lda #<Sparkle_IRQ		//Installs Fallback IRQ vector
			sta $fffe
			lda #>Sparkle_IRQ
			sta $ffff
			rts						//20 bytes
*/

//----------------------------

//.text "<OMG>"

EndLoader:

.var myFile = createFile("Sparkle.inc")
.eval myFile.writeln("//--------------------------------")
.eval myFile.writeln("//	Sparkle loader addresses	")
.eval myFile.writeln("//	KickAss format		")
.eval myFile.writeln("//--------------------------------")
.eval myFile.writeln("#importonce")
.eval myFile.writeln("")
.eval myFile.writeln(".const Sparkle_SendCmd		=$" + toHexString(Sparkle_SendCmd) + "	//Requests a bundle (A=#$00-#$7f) and prefetches its first sector, or")
.eval myFile.writeln("					//Requests a new disk (A=#$80-#$fe [#$80 + disk index]) without loading its first bundle, or")
.eval myFile.writeln("					//Resets drive (A=#$ff)")
.eval myFile.writeln(".const Sparkle_LoadA		=$" + toHexString(Sparkle_LoadA) + "	//Index-based loader call (A=#$00-#$7f), or")
.eval myFile.writeln("					//Requests a new disk & loads first bundle (A=#$80-#$fe [#$80 + disk index])")
.eval myFile.writeln(".const Sparkle_LoadFetched	=$" + toHexString(Sparkle_LoadFetched) + "	//Loads prefetched bundle, use only after Sparkle_SendCmd (A=bundle index)")
.eval myFile.writeln(".const Sparkle_LoadNext		=$" + toHexString(Sparkle_LoadNext) + "	//Sequential loader call, parameterless, loads next bundle in sequence")
//.eval myFile.writeln(".const Sparkle_InstallIRQ	=$" + toHexString(Sparkle_InstallIRQ) + "	//Installs fallback IRQ (A=raster line, X/Y=subroutine/music player vector high/low bytes)") 
//.eval myFile.writeln(".const Sparkle_RestoreIRQ	=$" + toHexString(Sparkle_RestoreIRQ) + "	//Restores fallback IRQ without changing subroutine vector (A=raster line)")
.eval myFile.writeln(".const Sparkle_IRQ		=$" + toHexString(Sparkle_IRQ) + "	//Fallback IRQ vector")
.eval myFile.writeln(".const Sparkle_IRQ_JSR		=$" + toHexString(Sparkle_IRQ_JSR) + "	//Fallback IRQ subroutine/music player JSR instruction")
.eval myFile.writeln(".const Sparkle_IRQ_RTI		=$" + toHexString(Sparkle_IRQ_RTI) + "	//Fallback IRQ RTI instruction")
.eval myFile.writeln(".const Sparkle_NTSC_Check	=$" + toHexString(NTSC) + "	//Value = $dd if PAL, $dc if NTSC")
.eval myFile.writeln(".const Sparkle_Save		=$302	//Hi-score file saver (A=#$01-#$0f, high byte of file size, A=#$00 to abort), only if hi-score file is included on disk")

.print "Sparkle_SendCmd:	" + toHexString(Sparkle_SendCmd)
.print "Sparkle_LoadA:	" + toHexString(Sparkle_LoadA)
.print "Sparkle_LoadFetched:	" + toHexString(Sparkle_LoadFetched)
.print "Sparkle_LoadNext:	" + toHexString(Sparkle_LoadNext)
//.print "Sparkle_InstallIRQ:	" + toHexString(Sparkle_InstallIRQ)
//.print "Sparkle_RestoreIRQ:	" + toHexString(Sparkle_RestoreIRQ)
.print "Sparkle_IRQ:		" + toHexString(Sparkle_IRQ)
.print "Sparkle_IRQ_JSR:	" + toHexString(Sparkle_IRQ_JSR)
.print "Sparkle_IRQ_RTI:	" + toHexString(Sparkle_IRQ_RTI)
.print "Loader End:		" + toHexString(EndLoader-1)
}