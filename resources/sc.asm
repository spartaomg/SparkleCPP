//TAB=4
//----------------------------
//	SPARKLE
//	Custom Drive Code Plugin
//	C64 CODE
//----------------------------
//	Version history
//
//	v1.0 	- initial version
//
//----------------------------

*=$2900 "C64 Code"

.pseudopc $0300
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
			tay
			jsr Sparkle_SendCmd		//Send number of blocks we want to receive (0-6)
			tya
			beq SC_ReceiveDone		//0 blocks to be received - exit

			sta Bits				//Save number of blocks to be received

SC_ReceiveNextBlock:			
			ldx #$00				//2
			ldy #ready				//2 Y=#$08, X=#$00
			sty $dd00				//4 Clear CO and DO to signal Ready-To-Receive
			bit $dd00				//Wait for Drive
			bvs *-3					//$dd00=#$cx - drive is busy, $0x - drive is ready	00,01	(BMI would also work)
			stx $dd00				//Release ATN										02-05
			jsr Delay				//Waste a few cycles... (drive takes 16 cycles)		06-28 (minimum needed here is 06-15, 10 cycles, including DEX!!!)

//----------------------------------
//		RECEIVE LOOP
//----------------------------------

SC_RcvLoop:
			lda $dd00				//4		W1-W2 = 18 cycles							25-28
			sty $dd00				//4 8	Y=#$08 -> ATN=1
			lsr						//2 10
			lsr						//2 12
			inx						//2 14
			nop						//2 16
			ldy #$c0				//2 (18)

			ora $dd00				//4		W2-W3 = 16 cycles
			sty $dd00				//4 8	Y=#$C0 -> ATN=0
			lsr						//2 10
			lsr						//2 12
SC_SpComp:	cpx #$ff				//2 14	Will be changed to #$ff in Spartan Step Delay
			beq SC_ChgJmp			//2/3	16/17 whith branch -------------|
SC_RcvCont:	ldy #$08				//2 (18/28) ATN=1						|
									//										|
			ora $dd00				//4		W3-W4 = 17 cycles				|
			sty $dd00				//4 8	Y=#$08 -> ATN=1					|
			lsr						//2 10									|
			lsr						//2 12									| C=1 here
			sta SC_LastBits+1		//4 16									|
			lda #$c0				//2 (18)								|
									//										|
			and $dd00				//4		W4-W1 = 16 cycles				|
			sta $dd00				//4 8	A=#$X0 -> ATN=0					|
SC_LastBits:ora #$00				//2 10									|
To:			sta $1000,x				//5 15									|
SC_JmpRcv:	bvc SC_RcvLoop			//3 (18)								|
									//										|
//----------------------------												|
									//										|
SC_ChgJmp:	ldy #<SC_BlockDone-<SC_ChgJmp	//2 19	<-----------------------|
			sty SC_JmpRcv+1					//4 23
			bne SC_RcvCont					//3 26 BRANCH ALWAYS

//----------------------------

SC_BlockDone:
			lda #<SC_RcvLoop-<SC_ChgJmp		//2 20 Restore Receive loop
			sta SC_JmpRcv+1					//4 24

			lda #$f8						//2	26

			inc To+2						//6 32
			dec Bits						//5 37
			bne SC_RcvLoop					//3 40 (+26 cycles)

			sta $dd00						//4	24

SC_ReceiveDone:
			rts

//----------------------------------
//		Send Code Blocks
//		X = SrcHi
//		A = NumBlocks
//----------------------------------

SC_SendBlocks:
			stx From+2
			tay
			jsr Sparkle_SendCmd		//Number of blocks to be sent (max. 7: ZP + $0200-$07ff)
			tya
			beq SC_SendDone
			sta NumBlocks
			ldy #$00
From:		lda $1000,y
			jsr Sparkle_SendCmd
			iny
			bne From
			inc From+2
			dec NumBlocks
			bne From
SC_SendDone:
			rts

}
*=$29f9 "C64 Close Sequence"		//Close Sequence of previous bundle
.pseudopc $03f9
{
.byte $00,$84,$00,$03,$fb,$01,$fe	//When the plugin is loaded, the depacker will process this sequence
}

//-----------------------------------------------------------------------------------------------------------------------------------------------

//----------------------------------
//	SPARKLE
//	Custom drive Code Plugin
//	DRIVE CODE
//----------------------------------

*=$2a00 "Drive Code"

.pseudopc $0100						//Stack pointer =#$00 at start
{

#import "SD.sym"					//Import labels from SD.asm

SC_DriveStart:
			//ldx #$ff
			//txs
			jsr SC_NewByte			//Number of Sparkle drive blocks to be sent
			beq SC_SkipSend

			jsr SC_SendDBlocks		//Send Sparkle drive blocks to C64 (they will arrive EOR-transformed)
SC_SkipSend:
			jsr SC_NewByte
			beq SC_DriveDone		//If nothing to receive -> we are done, back to Saprkle
			
			jsr SC_RcvCodeBlocks	//Receive drive code blocks from C64 (ZP + $0200-$07ff)

			jsr $0200				//Start custom drive code

			sei

			//lda #$7a
			//sta $1802
			lda #busy
			sta $1800

			jsr SC_NewByte
			beq SC_DriveDone		//Sparkle drive code is not being sent back
			//bmi SC_DriveReset

//----------------------------------
//		Retrieve Drive Blocks from C64
//----------------------------------

SC_RcvDriveBlocks:
			sta NumDriveBlocks
RcvDLoop:	jsr SC_NewByte
			ldx #$09				//Drive code blocks must be EOR-transformed
			axs #$00
			eor EorTab,x
RcvDTo:		sta.a $0000
			inc RcvDTo+1
			bne RcvDLoop
			lda RcvDTo+2
			bne *+4
			lda #$02				//Skip $0100-$02ff (stack/buffer [this page] and secondary buffer)
			clc
			adc #$01
			sta RcvDTo+2
			dec NumDriveBlocks
			bne RcvDLoop
SC_DriveDone:
			jmp CheckATN			//Back to Sparkle

EorTab:
.byte $7f,$76
NumDriveBlocks:
.byte $00
NumCodeBlocks:
.byte $00
SC_DriveReset:
			jmp ($fffc)
.byte $00
.byte $76,$7f

//----------------------------------
//		Receive a byte
//----------------------------------

SC_NewByte:
			ldx #$94				//Make sure C64 is ready to send (%10010100)
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
			tax
			rts

//----------------------------------------------------------------------------------------------------------------------------------------------
//		Everything below this line can be overwritten after custom code was uploaded to drive
//----------------------------------------------------------------------------------------------------------------------------------------------

//----------------------------------
//		Send Drive Blocks to C64
//----------------------------------

SC_SendDBlocks:
			sta NumDriveBlocks
			ldy #$00
			ldx #$ef
			lda #ready
			sta $1800				//Signal to C64 - drive is reasy to send

DLoop:		lda $0000,y				//4	11	33
			bit $1800				//4	15	37
			bmi *-3					//2	17	39 (C64 loop takes 44 cycles)
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

			lda DLoop+2				//4		10
			bne *+4					//2		12
			lda #$02				//2		14
			adc #$01				//2		16
			sta DLoop+2				//4		20
			dec NumDriveBlocks		//6		26
			bne DLoop				//3		29

			lda #busy			//16,17 		A=#$10
			bit $1800			//18,19,20,21 	Last bitpair received by C64?
			bmi *-3				//22,23
			sta $1800			//24,25,26,27	Transfer finished, send Busy Signal to C64
			
			bit $1800			//Make sure C64 pulls ATN before continuing
			bpl *-3				//Without this the next ATN check may fall through

			rts

//----------------------------------
//		Receive Code Blocks from C64
//----------------------------------

SC_RcvCodeBlocks:
			sta NumCodeBlocks
RcvCLoop:	jsr SC_NewByte
RcvCTo:		sta.a $0000
			inc RcvCTo+1
			bne RcvCLoop
			lda RcvCTo+2
			bne *+4
			lda #$01				//Skip $0100-$01ff
			clc
			adc #$01
			sta RcvCTo+2
			dec NumCodeBlocks
			bne RcvCLoop
			rts
}

