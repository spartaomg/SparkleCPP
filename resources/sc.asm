//TAB=4

//----------------------------------
//	SPARKLE
//	Custom Drive Code Plugin
//----------------------------------
//	Version history
//
//	v1.0 	- initial version
//
//----------------------------------

//----------------------------------
//	SPARKLE
//	Custom Drive Code Plugin
//	C64 CODE
//----------------------------------

.const buslock	= $f8

.var myFile = createFile("SparkleCustom.inc")
.eval myFile.writeln("//--------------------------------------------")
.eval myFile.writeln("//	Sparkle custom drive code addresses")
.eval myFile.writeln("//	KickAss format")
.eval myFile.writeln("//--------------------------------------------")
.eval myFile.writeln("")
.eval myFile.writeln("#importonce")
.eval myFile.writeln("")

*=$2900 "C64 Code"

.pseudopc $0300
{
{
#import "SL.sym"					//Import labels from SL.asm

NumBlocks:
.byte $00,$77						//First 2 bytes of block - 00 and block count, will be overwritten by Byte counter

//----------------------------------
//		Receive Drive Code Blocks
//		X = DstHi
//		A = NumBlocks
//----------------------------------

SC_ReceiveBlocks:
			stx To+2				//Hi Byte of destination address of blocks to be received
			pha
			jsr SC_Send_NTSC		//Send number of blocks we want to receive (0-6) AND adjust loop for NTSC as needed
			pla
			beq SC_ReceiveDone		//0 blocks to be received - exit

			sta Bits				//Save number of blocks to be received

SC_ReceiveNextBlock:			
			ldy #$00				//2
			ldx #ready				//2 X=#$08, Y=#$00
			stx $dd00				//4 Clear CO and DO to signal Ready-To-Receive
			bit $dd00				//Wait for Drive
			bvs *-3					//$dd00=#$cx - drive is busy, $0x - drive is ready	00,01 (BMI would also work)
			sty $dd00				//Release ATN										02-05
			dey						//													06-07
			jsr SC_Set01			//Waste a few cycles... (drive takes 16 cycles)		08-24 (minimum needed here is 06-15, 10 cycles)

//----------------------------------
//		RECEIVE LOOP
//----------------------------------

SC_RcvLoop:
SC_R1:		lda $dd00				//4
			stx $dd00				//4 8	X=#$08 -> ATN=1
			lsr						//2 10
			lsr						//2 12
			iny						//2 14
			nop						//2 16
			ldx #$c0				//2 (18)

SC_R2:		ora $dd00				//4
			stx $dd00				//4 8	X=#$C0 -> ATN=0
			lsr						//2 10
			lsr						//2 12
SC_SpComp:	cpy #$ff				//2 14
			beq SC_ChgJmp			//2/3 16/17 with branch ----------------|
SC_RcvCont:	ldx #$08				//2 (18/28) ATN=1						|
									//										|
SC_R3:		ora $dd00				//4										|
			stx $dd00				//4 8	X=#$08 -> ATN=1					|
			lsr						//2 10									|
			lsr						//2 12									|
			sta SC_LastBits+1		//4 16									|
			lda #$c0				//2 (18)								|
									//										|
SC_R4:		and $dd00				//4										|
			sta $dd00				//4 8	A=#$X0 -> ATN=0					|
SC_LastBits:ora #$00				//2 10									|
			inx						//2 12									|
			axs #$00				//2 14									|
			eor EorTab,x			//4 18									|
To:			sta $1000,y				//5 23									|
			ldx #$08				//2	25									|
SC_JmpRcv:	bvc SC_RcvLoop			//3 (28/27)								|
									//										|
//----------------------------												|
									//										|
SC_ChgJmp:	ldx #<SC_BlockDone-<SC_ChgJmp	//2 19	<-----------------------|
			stx SC_JmpRcv+1					//4 23
			bne SC_RcvCont					//3 26 BRANCH ALWAYS

//----------------------------

SC_BlockDone:
			lda #<SC_RcvLoop-<SC_ChgJmp		//2 29 Restore Receive Loop
			sta SC_JmpRcv+1					//4 33

			lda #buslock					//2 35

			inc To+2						//6 41
			dec Bits						//5 46
			bne SC_RcvLoop					//3 49 (+21 cycles)

			sta $dd00						//4	24

SC_ReceiveDone:
			rts

EorTab:
.byte $7f,$76
SC_Set01:	lda #$35
			sta $01
			rts
			nop
.byte $76,$7f

//----------------------------------
//		Send Code Blocks
//		X = SrcHi
//		A = NumBlocks
//----------------------------------

SC_SendBlocks:
			stx From+2
			tay
			jsr SC_Sparkle_SendCmd		//Number of blocks to be sent (max. 7: $0200-$07ff + ZP)
			tya
			beq SC_SendDone				//0 blocks to be transferred, abort
			sta NumBlocks
			ldy #$00
From:		lda $1000,y
			jsr SC_Sparkle_SendCmd
			iny
			bne From
			inc From+2
			dec NumBlocks
			bne From
SC_SendDone:
			rts

//----------------------------------

SC_Send_NTSC:
			jsr SC_Sparkle_SendCmd
			ldy #$00
			ldx #$02
!:			tya
			ora Read1,x
			sta SC_R1,x
			tya
			ora Read2,x
			sta SC_R2,x
			tya
			ora Read3,x
			sta SC_R3,x
			tya
			ora Read4,x
			sta SC_R4,x
			dex
			bne *+4
			ldy #$04
			bpl !-
			rts

//----------------------------------

SC_Sparkle_SendCmd:
			sta Bits
			jsr SC_Set01
			ldx #sendbyte
			stx $dd00
			bit $dd00
			bmi *-3
			
SC_BitSLoop:
			adc #$e7
			sax $dd00
			and #$10
			eor #$10
			ror Bits
			bne SC_BitSLoop

			lda #buslock
			sta $dd00

			rts

//----------------------------------

.eval myFile.writeln(".const Sparkle_RcvDrvCode		=$" + toHexString(SC_ReceiveBlocks) + "	//Receive Sparkle drive code (A = number of pages, X = destination address high byte)")
.eval myFile.writeln(".const Sparkle_SendDrvCode		=$" + toHexString(SC_SendBlocks) + "	//Send drive code (A = number of pages, X = source address high byte)")

.print "Sparkle_RcvDrvCode:	" + toHexString(SC_ReceiveBlocks)
.print "Sparkle_SendDrvCode:	" + toHexString(SC_SendBlocks)
}
}
*=$29f9 "C64 Close Sequence"		//Close Sequence of previous bundle
.pseudopc $03f9
{
.byte $00,$84,$00,$03,$fb,$01,$fe	//When the plugin is loaded, the depacker will process this sequence
}

//-----------------------------------------------------------------------------------------------------------------------------------------------

//----------------------------------
//	SPARKLE
//	Custom Drive Code Plugin
//	DRIVE CODE
//----------------------------------

*=$2a00 "Drive Code"

.pseudopc $0100						//Stack pointer =#$00 at start
{
{
#import "SD.sym"					//Import labels from SD.asm

.const NumBlocks = $0102

SC_DriveStart:

//----------------------------------
//		Skip restore code at start
//----------------------------------

			jmp SC_SendRcv

//----------------------------------
//		Restore Sparkle's drive code
//----------------------------------

SC_Restore:
			sei

			ldx #$ee				//Read mode, Set Overflow enabled
			stx $1c0c

			txs						//Make sure stack pointer is outside restore code

			lda #$7a
			sta $1802
			lda #busy
			sta $1800

			lda #$7f				//Disable all interrupts
			sta $180e
			sta $1c0e
			lda $180d				//Acknowledge all pending interrupts
			lda $1c0d

			jsr SC_NewByte
			tax
			beq SC_DriveDone		//Sparkle drive code is not being sent back
			bpl SC_RestoreSpakle
SC_DriveReset:
			jmp ($fffc)

//----------------------------------
//		Retrieve Drive Blocks from C64
//----------------------------------

SC_RcvBlocks:
			sta NumBlocks
			bpl SC_DcrCtr			//Always
RcvLoop:	jsr SC_NewByte
RcvTo:		sta $0200
			inc RcvTo+1
			bne RcvLoop
			inc RcvTo+2
SC_DcrCtr:	dec NumBlocks
			bne !+
			lda #$00				//Last block (if NumBlocks > 0) always goes to ZP
			sta RcvTo+2
!:			bpl RcvLoop
			lda #$03				//Update the Receive Loop to start at $0300 for retreiving Sparkle's drive code
			sta RcvTo+2
			rts

//----------------------------------
//		Receive a byte
//----------------------------------

SC_NewByte:	ldx #$94				//Make sure C64 is ready to send (%10010100)
			cpx $1800
			bne *-3

			lda #$80				//$dd00=#$9b, $1800=#$94
			ldx #busy				//=#$10 (AA=1, CO=0, DO=0)
			ldy #$85
			sax $1800				//A&X = #$80 & #$10 = #$00, $dd00=#$1b, $1800=#$85

!:			cpy $1800
			beq *-3
			ldy $1800				//read: 6-12 cycles after $dd00 write (always within range)
			cpy #$80
			ror
			bcc !-
			stx $1800				//Drive busy

			ldx #$95				//Wait for C64 to signal transfer complete (%10010101)
			cpx $1800
			bne *-3
			rts

//----------------------------------
//		Restore Sparkle
//----------------------------------

SC_RestoreSpakle:
			jsr SC_RcvBlocks
SC_DriveDone:
			jmp CheckATN			//Back to Sparkle

//----------------------------------------------------------------------------------------------------------------------------------------------
//		Everything below this line can be overwritten after custom code was uploaded to drive
//----------------------------------------------------------------------------------------------------------------------------------------------

SC_SendRcv:	jsr SC_NewByte			//Number of Sparkle drive blocks to be sent
			tax
			beq SC_SkipSend

//----------------------------------
//		Send Drive Blocks to C64
//----------------------------------

			dex
			stx NumBlocks			//Convert counter from 1-base to 0-base
			bne	!+
			stx DLoop+2
!:			ldy #$00
			ldx #$ef
			lda #ready
			sta $1800				//Signal to C64 - drive is ready to send

DLoop:		lda $0300,y				//4	11	33
			bit $1800				//4	15	37
			bmi *-3					//2	17	39
			sax $1800				//4

			iny						//2	6
			asl						//2	8
			ora #$10				//2	10
			bit $1800				//4	14
			bpl *-3					//2	16
			sta $1800				//4

			ror						//2	6
			asr #$f0				//2	8
			bit $1800				//4	12
			bmi *-3					//2	14
			sta $1800				//4

			lsr						//2	6
			asr #$30				//2	8
			cpy #$01				//2	10
			bit $1800				//4	14
			bpl *-3					//2	16
			sta $1800				//4

			bcs DLoop				//2	7	6

			inc DLoop+2				//6		12
			dec NumBlocks			//6		18
			bne !+					//3/2	21/20
			lda #$00				//0/2	21/22
			sta DLoop+2				//0/4	21/26
!:			bpl DLoop				//3		24/29

			lda #busy				//16,17 		A=#$10
			bit $1800				//18,19,20,21 	Last bitpair received by C64?
			bmi *-3					//22,23
			sta $1800				//24,25,26,27	Transfer finished, send Busy Signal to C64
			
			bit $1800				//Make sure C64 pulls ATN before continuing
			bpl *-3					//Without this the next ATN check may fall through
SC_SkipSend:
			jsr SC_NewByte
			tax
			beq SC_DriveDone		//If nothing to receive -> we are done, back to Sparkle
			
			jsr SC_RcvBlocks		//Receive drive code blocks from C64 (ZP + $0200-$07ff)

//----------------------------------
//		Execute custom drive code
//----------------------------------

	        pla						//Stack pointer adjustment
			lda #>(SC_Restore-1)
		    pha
			lda #<(SC_Restore-1)
	        pha						//SP = $ff after this

			jmp $0200				//Start custom drive code, if stack remains untouched then RTS will return to SC_Restore
}
}