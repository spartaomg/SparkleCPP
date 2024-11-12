//TAB=4
//----------------------------------------------------------------------------------------
//	SPARKLE
//	Inspired by Lft, Bitbreaker, and Krill
//	Drive Code
//	Tested on 1541, 1541-II, 1571, the 1541 Ultimate series, and Oceanic drives
//----------------------------------------------------------------------------------------
//	- 2-bit + ATN protocol, combined fixed-order and out-of-order loading
//	- 124-cycle GCR read-decode-verify loop with 1 BVC instruction
//	- tolerates disk rotation speeds between 269-314 rpm in VICE in all 4 disk zones
//	- 72 bycles/block transfer
//	- Spartan Stepping (TM) for uninterrupted loading across neighboring tracks
//	- LZ blockwise back-to-back compression
//----------------------------------------------------------------------------------------
//	Revision history
//
//	v00	- initial version based on Sparkle 1 Drive Code
//		- 128-cycle on-the-fly GCR read-decode-verify loop
//		- introducing Spartan Stepping (TM)
//
//	v01	- 127-cycle GCR RDV loop
//		  tolerates disk speeds 291-307 rpm
//		- new disk sector layout
//		  tracks start with Sector 2
//		  zone 3 with IL4, zones 0-2 with IL3
//
//	v02	- improved 127-cycle GCR RDV loop
//		  tolerates disk speeds 289-307 rpm
//		- improved Spartan Stepping
//		  60 bycles left for second half-step allowing settling of the R/W head
//		- simplified bit shuffle and conversion
//		- optimized sector layout and interleave handling for Spartan Stepping
//
//	v03	- new 125-cycle GCR RDV loop with 1 BVC instruction
//		  tolerates disk speeds 289-309 rpm
//		- loader version used in OMG Got Balls!
//		- alternative 127-cycle GCR RDV loop with 2 BVC instructions
//		  tolerates disk speeds 286-307 rpm (not used)
//
//	v04	- speed improvements by eliminating motor stops in the middle of data transfer
//		- motor stop is delayed by 2 seconds after data transfer is complete
//		- updated Spartan Step code
//
//	v05	- updated stepper code
//		- bug fixes
//		  fixed a bug that prevented seeking to Track 1 after disk finished then reloaded
//		  fixed buggy motor stop delay
//
//	v06	- C64 reset detection
//		- new commmunication code: busy = #$02 (DO), ready = #$18 (CO|AA)
//		  allows the C64 to detect drive reset
//		  leaves DO/DI untouched when drive is busy, can detect C64 reset which turns DI=1
//		  no reset detection during data transfer
//		- improved flip detection
//		- updated seek code
//		- improved 125-cycle GCR RDV loop, now tolerates disk speeds 285-309 rpm
//		- ANC instruction replaced with AND+CLC for Ultimate-II+ compatibility in stepper code
//
//	v07	- lots of code optimization
//		  swapped unfetched (00), wanted (ff/negative) and fetched (01/positive) flags
//		- updated, faster wanted list building
//		  results in faster loading with 0-25% CPU load
//
//	v08	- drive code updated to work with back-to-back compression code: no half-blocks left unused
//		  the last block of a Bundle also contains the beginning of the next Bundle
//		  C64 buffer needs to be left untouched between loader calls
//
//	v09 	- drive transfer loop modified to work with a 16-byte H2STab instead of a 256-byte tab
//		  new transfer loop now takes 67 cycles (previous version was 65)
//		  C64 transfer loop remains 72 cycles long
//		  new 16-byte H2STab moved from $0200 to $0600
//		- $0200-$02ff is now used as a secondary buffer
//		  last block of a Bundle is fetched OOO and stored here until all other blocks are transferred
//		  thus, the last block (which also contains the beginning of the next Bundle) is always transferred last 
//		  results in even faster loading across all CPU loads
//		- simplified wanted list preparations
//		- end-of-disk detection
//
//	v10	- new 126-cycle GCR RDV loop with improved wobble tolerance based on THCM's tests
//		  tolerates disk speeds 291-309 rpm with max wobble in VICE, similar tolerance in Micro64
//		  passed THCM's 24-hour test without error
//		  previous 125-cycle GCR loop failed on THCM's SX-64 due to lack of wobble tolerance 
//		- bug fixes based on THCM's tests
//		- test disk code including zone tests
//
//	v11	- new 125-cycle GCR RDV loop
//		  tolerates disk speeds 289-311 rpm with max wobble in VICE across all 4 speed zones
//		  passed THCM's 24-hour test without error in over $0d00 (3328) full disk loads
//
//	v12	- new communication code
//		  inverts ATN check to allow bus lock
//		  no drive reset detection currently
//		- improved C64 reset detection
//		- final, v1.0 release code!
//
//	v13	- custom interleave
//		- updated wanted sector selection algorithm
//		- introduced in Sparkle v1.3
//
//	v14	- updated 125-cycle GCR RDV loop
//		  tolerates 284-311 rpm disk speeds with max wobble in VICE across all 4 disk zones
//		- GCR loop and sector parameters are only updated at zone changes
//		- reworked block count update to work with new block structure
//		- interleave bug fix (loader in infinite loop on track 19 if IL0=20)
//		- released with Sparkle v1.4
//
//	v15	- block chain building bug fixed
//		- released with Sparkle V1.5
//
//	v16	- major update with random file access
//		- new memory layout (see below)
//		- secondary buffer feature removed to give space for directory
//		- directory structure (max 128 files per disk, 64 files per block):
//		  00 - track (EOR transformed)
//		  01 - first sector on track (EOR transformed)
//		  02 - sector count remaining on track (EOR transformed)
//		  03 - block pointer, points at the first byte of bundle in block (NOT EOR transformed)
//		- updated communication code
//
//	v17	- high score file saver
//		- flip disk detection with selectable disk ID ($80-$ff)
//		- product ID check added to flip detection
//		- additional memory layout improvements
//		  code is now interleaved with tabs at $0300-#03ff
//		- reset drive with bundle ID #$ff
//		- released with Sparkle 2
//
//	v18	- new GCR loop patch
//		  better speed tolerance in zones 0-2
//		  zone 3 remains 282-312 at 0 wobble in VICE
//		- checking trailing zeros after fetching data block to improve reliability 
//		- bits of high nibble of fetched data are no longer shuffled, only EOR'd
//		  BitShufTab is now reduced to 4 bytes only
//		- more free memory
//		- ATNA-based transfer loop eliminating H2STab
//		- each track starts with sector 0 now
//
//	v19	- full GCR loop rewrite
//		  124-cycle loop with on-the-fly checksum verification for zones 0-2
//		  checksum verification is done partially outside the GCR loop for zone 3
//		  much wider rotation speed tolerance range of 269-314 rpm accross all 4 speed zones with max wobble in VICE
//		- re-introducing the second block buffer
//		  stores the transitional block until all other blocks are transferred
//		- back to Sparkle's original sector handling after track changes
//		  first sector of the next track depends on the last sector of the previouos track
//		- removed trailing zero and sync-in-progress checks as these make loading fail on Star Commander warp disks
//		- adding full block ID check to improve reliability and to avoid false data blocks on Star Commander warp disks
//		- to be released with Sparkle 2.1
//
//	v20	- adding sector header check to checksum verification
//
//----------------------------------------------------------------------------------------
//	Memory Layout
//
//	0000	007d	ZP GCR tables and variables
//	007c	00ff	GCR Loop and loop patches
//	0100	01ff	Data Buffer on Stack
//	0200	02ff	Second Data Buffer
//	0300	03f1	GCR tables with code interleaved
//	0411	0472	GCR table with code interleaved
//	0300	06f4	Drive Code
//	0700	07ff	Directory (4 bytes per entry, 64 entries per dir block, 2 dir blocks on disk)
//
//	Layout at Start
//
//	0300	03f1	GCR tables			block 0
//	0300	05ff	Code				blocks 0-2
//	0600	06ff	ZP GCR tables and GCR loop	block 3
//	0700	0736	Installer			block 4
//
//	Layout in PRG
//
//	2300	23f1	GCR tables			block 0
//	2411	2472	GCR table			block 1
//	2300	26f4	Drive Code			blocks 0-3 3 -> block 5
//	2700	27ff	ZP GCR tables and GCR loop	block 	   4 -> block 3
//	2800	2836	Installer			block 	   5 -> block 4
//
//----------------------------------------------------------------------------------------
//	Track 18
//
//	00		BAM
//	01-06	DirArt
//	07-10	C64 Code
//	11-16	Drive Code
//	17-18	Internal Directory
//
//----------------------------------------------------------------------------------------
//	Flip Info in BAM (EOR-transformed):
//
//	Disk:		Buffer:	Function:
//	18:00:$ff	$0101	BAM_DiskID	(for flip detection, compare to NextID @ $23 on ZP)
//	18:00:$fe	$0102	IL3R		(will be copied to $20)
//	18:00:$fd	$0103	IL2R		(will be copied to $21)
//	18:00:$fc	$0104	IL1R		(will be copied to $22)
//	18:00:$fb	$0105	BAM_NextID	(will be copied to $23 after flip, #$00 if no more flips)
//	18:00:$fa	$0106	IL0R		(will be copied to $24)
//
//	18:00:$f9	$0107	BAM_IncSaver	(will be copied to IncSaver)
//
//	18:00:$f8	$0108	BAM_ProdID (0)
//	18:00:$f7	$0109	BAM_ProdID (1)
//	18:00:$f6	$010a	BAM_ProdID (2)
//
//----------------------------------------------------------------------------------------
//	Directory Structure
//
//	4 bytes per dir entry
//	128 dir entries in 2 dir blocks
//
//	00	Track
//	01	First sector on track after track change (to mark fetched sectors, NOT first sector of bundle)
//	02	Sector Counter (to mark fetched sectors and first sector of bundle)
//	03	Byte Pointer (used by the depacker to find start of stream, to be copied to the last byte of first block)
//
//----------------------------------------------------------------------------------------

//Constants:
.const VerifSctHdrs	=true

.label CSV			=$07		//Checksum Verification Counter Default Value (3 1-bits, 3 data blocks to be verified)

.label DO			=$02
.label CO			=$08
.label AA			=$10
.label busy			=AA			//DO=0,CO=0,AA=1	$1800=#$10	dd00=010010xx (#$4b)
.label ready		=CO			//DO=0,CO=1,AA=0	$1800=#$08	dd00=100000xx (#$83)

.label Sp			=$52		//Spartan Stepping constant (=82*72=5904=$1710=$17 bycles delay)

//ZP Usage:
.label cT			=$00		//Current Track
.label cS			=$01		//Current Sector
.label nS			=$02		//Next Sector
.label BlockCtr		=$03		//No. of blocks in Bundle, stored as the last byte of first block
.label WantedCtr	=$08		//Wanted Sector Counter
.label Random		=$18		//Marks random file access
.label VerifCtr		=$19		//Checksum Verification Counter
.label LastT		=$20		//Track number of last block of a Bundle, initial value=#$01
.label LastS		=$21		//Sector number of last block of a Bundle, initial value=#$00
.label SCtr			=$22		//Sector Counter, sectors left unfetched in track
.label BPtr			=$23		//Byte Pointer within block for random access
.label StepDir		=$28		//Stepping  Direction
.label ScndBuff		=$29		//#$01 if last block of a Bundle is fetched, otherwise $00
.label WList		=$3e		//Wanted Sector list ($3e-$52) ([0]=unfetched, [-]=wanted, [+]=fetched)
.label NewBundle	=$54		//New Bundle Flag, #$00->#$01, stop motor if #$01
.label DirSector	=$56		//Initial value=#$c5 (<>#$10 or #$11)
.label NBC			=$5c		//New Block Count temporary storage
.label TrackChg		=$5e		//Indicates whether Track change is needed AFTER CATN (last block of bundle=last sector of track)

.label ILTab		=$60		//Inverted Custom Interleave Table ($60-$64)
.label NextID		=$63		//Next Side's ID - will be updated from 18:00:$fd of next side

.label StepTmrRet	=$66		//Indicates whether StepTimer code is called in subroutine
.label BitRateRet	=$6c		//Indicates whether Store code is called in subroutine
.label EoD			=$6e		//End of Disk flag, only used with sequential loading
.label IncSaver		=$74		//=#$02 if Saver Code is included, otherwise #$00
.label SaverCode	=$76		//Indicates whether Saver Code Drive code is in the buffer

.label ZPIncSav		=$10
.label ZPILTab		=$70
.label ZPProdID		=$78

.label ZP7f			=$30		//BitShufTab
.label ZP3e			=$32		//TabC value
.label ZP12			=$3b		//TabF value
.label ZP07			=$57		//TabF value
.label ZP00			=$6b		//TabF value
.label ZP01ff		=$58		//$58/$59 = $01ff
.label ZP0101		=$59		//$59/$5a = $0101
.label ZP0200		=$7a		//$7a/$7b = $0200

//BAM constants:
.label BAM_DiskID	=$0101
.label BAM_NextID	=$0105
.label BAM_IncSave	=$0107
.label BAM_ProdID	=$0108

//Other constants:
.label SF			=$012a		//SS drive code Fetch vector
.label SH			=$012f		//SS drive code Got Header vector

.label OPC_BNE		=$d0

//GCR Decoding Tables:
.label TabZP		=$00
.label BitShufTab	=TabZP+$30

.label TabA			=Tab300+$12
.label TabB			=Tab300
.label TabC			=TabZP
.label TabD			=Tab300+$01
.label TabE			=TabZP
.label TabF			=TabZP+$01
.label TabG			=Tab300+$100
.label TabH			=Tab300+$1e

.label XX1			=$c3
.label XX2			=$9d
.label XX3			=$e5
.label XX4			=$67

*=$2300 "Drive Code"
.pseudopc $0300 {
Tab300:
//0300
ClearList:	clc					//00
JmpClrList:	ldx #$14			//01 02
ClrWList:	sty WList,x			//03 04	Y=00, clear Wanted Block List
			dex					//05
			bpl ClrWList		//06 07
			bcs ClrJmp			//08 09
			rts					//0a
ClrJmp:		jmp NextTrack		//0b-0d

			//Y=#$00 before loop
CopyDir:
CDLoop:		pla					//0e		=LDA $0100,y
			iny					//0f		(TSX)
			sta $0700,y			//10-12		(STA $0700,X)
			bne CDLoop			//13 14
			jmp DirFetchReturn	//15-17

//		x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, xa, xb, xc, xd, xe, xf
//0318
FetchJmp:
.byte									<FT,>FT
.byte											$37
ProductID:
.byte												$ab,$cd,$ef,$07,XX1
.byte	$a5,$a1,$a7,XX2,$ad,$a9,$67,$04,$05,$0d,$77,$00,$01,$09,$47,XX3
.byte	$07,$0f,XX4,$0a,$03,$0b,$a7,$0c,$06,$0e,$b7,$08,$02,XX1,$27,XX2
.byte	$af,$ab,$ae,$aa,$ac,$a8,$e7
//0347
HeaderJmp:
.byte								<HD,>HD
.byte										$d0,$f7
//034b
ToggleLED:	lda #$08			//4b 4c
			eor $1c00			//4d-4f
			nop #$d1			//50 51	SKIPPING TabD value
			sta $1c00			//52-54
			rts					//55
.byte							$87
//0357
DataJmp:
.byte								<DT,>DT
.byte										$d9,$97,XX3,XX4,XX1,$17,XX2	//PC tool stores version info here
.byte	XX3,$a3,$a6,$a2,$a4,$a0,$c7
//0367
SHeaderJmp:
.byte								<SH,>SH
.byte										$d4,$d7,XX4,XX1,XX2,$57
//036f
SFetchJmp:
.byte																<SF
.byte	>SF
.byte	    $d5
//0372
ShufToRaw:	ldx #$09			//72 73	Fetched data are bit shuffled and
			axs #$00			//74 75	EOR transformed for fast transfer, sets C
			eor.z BitShufTab,x	//76 77
			rts					//78
//0379
.byte										$dd
//037a
RcvByte:	ldy #$85			//7a 7b
			sax $1800			//7c-7e		A&X = #$80 & #$10 = #$00, $dd00=#$1b, $1800=#$85
RBLoop:		cpy $1800			//7f-81	4
			beq *-3				//82 83	2/3	4+3+4+2+4+2+2+3 = 24 cycles worst case
			ldy $1800			//84-86	4	read: 6-12 cycles after $dd00 write (always within range)
			cpy #$80			//87 88	2
			ror					//89	2
			bcc RBLoop			//8a 8b	3	17/24 cycles/bit, 18 cycles per loop on C64 is enough
			stx $1800			//9c-9e		Drive busy
			rts					//9f		20 bytes total, A = Bundle Index
//0390
.byte	XX1,$d3

//----------------------------------------------
//		HERE STARTS THE FUN
//		Fetching BAM OR Dir
//----------------------------------------------

//0392
FetchBAM:	sty LastS			//92 93	Y=#$00
FetchDir:	jsr ClearList		//94-96 C=0 after this
			bcc *+3				//97 98 SKIPPING TabD value
	.byte	$db					//99	TabD
			ldx LastS			//9a 9b
			dec WList,x			//9c 9d	Mark sector as wanted
			lax ZP12			//9e 9f	Both FetchBAM and FetchDir need track 18
			sta LastT			//a0 a1	A=X=#$12

//--------------------------------------
//		Fetching any T:S		//A=X=wanted track, Y=#$00
//--------------------------------------

GotoTrack:	iny					//a2
ContCode:	sty WantedCtr		//a3 a4	Y=#$01 here
			sty BlockCtr		//a5 a6
			sec					//a7
			nop #$de			//a8 a9 TabD
			sbc cT				//aa ab	Calculate Stepper Direction and number of Steps
			beq ResetVerif		//ac ad	We are staying on the same Track, skip track change
			sty StepTmrRet		//ae af	->#$01 - signal need for RTS 
			nop #$d7			//b0 b1 TabD
			bcs SkipStepDn		//b2 b3
			eor #$ff			//b4 b5
			adc #$01			//b6 b7
			nop #$df			//b8 b9 TabD
			ldy #$03			//ba bb	Y=#$03 -> Stepper moves Down/Outward
			sty StepDir			//bc bd	Store stepper direction UP/INWARD (Y=#$01) or DOWN/OUTWARD (Y=#$03)
SkipStepDn:	asl					//be	Y=#$01 is not stored - it is the default value which is restored after every step
			tay					//bf	Y=Number of half-track changes
			jsr StepTmr			//c0-c2	Move head to track and update bitrate (also stores new Track number to cT and calculates SCtr)
			
//--------------------------------------
//		Multi-track stepping
//--------------------------------------

			lda Spartan+1		//c3-c5
			sta $1c00			//c6-c8	Store bitrate
	.byte	$d8					//c9	TabD = CLD
ResetVerif:	lda #CSV			//ca cb
			sta VerifCtr		//cc cd	Verify track after head movement

.if (VerifSctHdrs)
{
			nop #VerifCtr		//ce cf
			nop #$d2			//d0 d1 TabD (CMP izy)
			nop #FetchData		//d2 d3
}
//--------------------------------------
//		Fetch Code
//--------------------------------------
FT:
Fetch:
.if (!VerifSctHdrs)
{
			lda VerifCtr		//ce cf	If checksum verification needed at disk spin up...
			nop #$d2			//d0 d1 TabD (CMP izy)
			bne FetchData		//d2 d3...then fetch any data block instead of a Header
}
FetchHeader:
			ldy #<HeaderJmp		//d4 d5	Checksum verification after GCR loop will jump to Header Code
FetchSHeader:
			lda #$52			//d6 d7	First 8 bits of Header ID (01010010)
			nop #$da			//d8 d9	Skipping TabD
			ldx #$04			//da db	4 bytes to stack
			txs					//dc	Header: $0104,$0103..$0101
			ldx #$40			//dd de Last 2 bits of Header ID (01|000000)
			bne Presync			//df e0 Skip Data Block fetch
			
FetchData:	ldy #<DataJmp		//e1 e2	Checksum verification after GCR loop will jump to Data Code
			lda #$55			//e3 e4	First 8 bits of Data ID (01010101)
			ldx #$00			//e5 e6 256 bytes to stack
			txs					//e7	Data: $0100,$01ff..$0101
			nop #$dc			//e8 e9	Skipping TabD
			ldx #$c0			//ea eb Last 2 bits of Data ID (11|000000)

Presync:	sty.z ModJmp+1		//ec ed	Update Jump Address
			ldy #$00			//ee ef
			nop #$d6			//f0 f1	Skipping TabD
			stx AXS+1			//f2-f4
			sty.z CSum+1		//f5 f6 Reset checksum
			sta (GCRLoop0_2+1),y//f7 f8	Any value would do in A as long as $0102 and $0103 are the same
			sta (GCRLoop0_2+4),y//f9 fa
			ldx #$c0			//fb fc

Sync:		bit $1c00			//fd-ff	Wait for sync mark
			bmi *-3				//00 01
			nop $1c01			//02-04	Sync byte - MUST be read (VICE bug #582), not necessarily #$ff
			clv					//05	likely the last byte on the latch

								//Addr |Cycles
			bvc *				//06 07|00-01
			cmp $1c01			//08-0a|05	*Read1 = AAAAABBB  ->	01010|010(01) for Header
			clv					//0b   |07				01010|101(11) for Data
			beq SkipNOPs		//0c 0d|10
ReFetch:	jmp (FetchJmp)		//0e-10|..	Wrong block type, refetch everything, vector is modified from SS!!!
	.byte	$fa					//11   |..	TabG (NOP)
			nop					//12   |..
	.byte	$ea					//13   |..	TabG (NOP) 
SkipNOPs:	bvc *				//14 15|00-01
			lda $1c01			//16-18|05*	*Read2 = BBCCCCCCD ->	01|CCCCC|D for Header
AXS:		axs #$00			//19 1a|07							11|CCCCC|D for Data
			bne ReFetch			//1b 1c|09	X = BB000000 - X1000000, if X = 0 then proper block type is fetched
			ldx #$3e			//1d 1e|11
			sax.z tC+1			//1f 20|14
	.byte	$7a					//21   |16	TabG (NOP)
	.byte	$5a					//22   |18	TabG (NOP)
	.byte	$da					//23   |20	TabG (NOP)
			lsr					//24   |22
			ldx #$00			//25 26|24
			lda #$7f			//27 28|26	Z=0, needed for BNE in GCR loop
			jmp GCREntry		//29-2b|29	Same cycle count as in GCR loop before BNE

//--------------------------------------
//		Got Header
//--------------------------------------
//042c
			nop $0103			//2c-2e
HD:
Header:
BneFetch:	bne Fetch			//2f 30	Checksum mismatch -> fetch next sector header
	.byte	$6a					//31	TabG (ROR)
	.byte 	$4a					//32	TabG (LSR)
	.byte	$ca					//33	TabG (DEX)
			lda $0103			//34-36
			jsr ShufToRaw		//37-39
			sta cS				//3a-3b
.if (VerifSctHdrs)
{
			ldx VerifCtr		//3c 3d
			bne FetchData		//3e 3f	Always fetch sector data if we are verifying the checksum
}
else
{
			nop #VerifCtr		//3c 3d
			nop #FetchData		//3e 3f
}
			tay					//40
			ldx WList,y			//41 42	Otherwise, only fetch sector data if this is a wanted sector
BplFetch:	bpl Fetch			//43 44 Sector is not on wanted list -> fetch next sector headed
			bmi FetchData		//45 46	Sector is on wanted list, save sector number and fetch data

//--------------------------------------	Will be changed to JMP CopyDir after Block 3 copied

ToCD:		jmp CopyCode		//47-49 Sector 15 (Block 3) - copy it from the Buffer to its place

//--------------------------------------
//		Flip Detection			//Y=#$00 and X=#$00 here
//--------------------------------------

FlipDtct:	cmp (ZP0101),y		//4a 4b	DiskID, compare it to NextID
			bne BneFetch		//4c 4d ID mismatch, fetch again until flip detected
			ldy #$03			//4e 4f
			nop #$ba			//50 51 Skipping TabG value $ba (TSX)	//.byte	$ba					//51	TabG (TSX) (X=#$00)
	.byte	$8a					//52	TabG (TXA) (A=#$00)
	.byte	$aa					//53	TabG (TAX)
ProdIDLoop:	lda BAM_ProdID-1,y	//54-56	Also compare Product ID, only continue if same
			cmp (ZPProdID),y	//57 58
			bne BneFetch		//59 5a	Product ID mismatch, fetch again until same
			dey					//5b
			bne ProdIDLoop		//5c 5d
			ldy #$05			//5e 5f
			nop #$3a			//60 61	Skipping TabG (NOP)
	.byte	$1a					//62	TabG (NOP)
	.byte	$9a					//63	TabG (TXS) no effect X=SP=#$00
			sty DirSector		//64 65	Invalid value to trigger reload of the directory of the new disk side
CopyBAM:	lda (ZP0101),y		//66 67	= LDA $0101,y
			sta (ZPILTab),y		//68 69	($0100=DiskID), $101=IL3R, $102=IL2R, $103=IL1R, $0104=NextID, $105=IL0R
			dey					//6a		
			bne CopyBAM			//6b 6c

			lda (ZPIncSav),y	//6d 6e	Value (#$00 vs. #$02) indicates whether Saver Code is included on this disk side
			sta IncSaver		//6f 70
	.byte	$2a					//71	TabG (ROL) no effect
	.byte	$0a					//72	TabG (ASL) no effect
			tya					//73
			jmp CheckDir		//74-76	Y=A=#$00 - Reload first directory sector

//--------------------------------------
//		Disk ID Check		//Y=#$00 here
//--------------------------------------

Track18:	cpx #$10			//77 78	Drive Code Block 3 (Sector 16) or Dir Block (Sectors 17-18)?
			bcs ToCD			//79 7a
			lda NextID			//7b 7c	Side is done, check if there is a next side
			bpl FlipDtct		//7d 7e	//bmi ToReset		//49 4a	Disk ID = #$00 - #$7f, if NextID > #$7f -> no more disks -> reset drive 

			jmp ($fffc)			//7f-81

//--------------------------------------
//		Got Data
//--------------------------------------
DT:
Data:		ldx cT				//82 83
			cpx #$12
			bcs SkipCSLoop  
			ldx #$7e			//CSLoop takes 851 cycles (33 bytes passing under R/W head in zone 3)
			bne CSLoopEntry
CSLoop:		eor $0102,x
			eor $0103,x
			dex
			dex
CSLoopEntry:
			eor $0180,x
			eor $0181,x
			dex
			dex
			bne CSLoop
SkipCSLoop:	tay
ToBneFetch:	bne BneFetch		//Checksum mismatch, fetch header again
			
			lsr VerifCtr		//Checksum Verification Counter
			bcs BplFetch		//If counter<>0, go to verification loop (use BPL Fetch as trampoline, VerifCtr is always positive)

RegBlockRet:
			ldx cS				//Current Sector in Buffer
			lda cT
			cmp #$12			//If this is Track 18 then we are fetching Block 3 or a Dir Block or checking Flip Info
			beq Track18			//We are on Track 18

//--------------------------------------
//		Update Wanted List
//--------------------------------------

			sta WList,x			//Sector loaded successfully, mark it off on Wanted list (A=Current Track - always positive)
			dec SCtr			//Update Sector Counter

//--------------------------------------
//		Check Saver Code
//--------------------------------------

			lsr SaverCode
			bcc ChkLastBlock
			jmp $0100			//Saver Code fetched

//--------------------------------------
//		Store Transitional Block
//--------------------------------------

StoreLoop:	pla
			iny
			sta (ZP0200),y
			bne StoreLoop

			inc ScndBuff
			bne ToBneFetch		//ALWAYS

//--------------------------------------
//		Check Last Block		//Y=#$00 here
//--------------------------------------

ChkLastBlock:
			dec WantedCtr

			cmp LastT
			bne SkipLast
			cpx LastS
			bne SkipLast

			lda (ZP01ff),y		//Save new block count for later use
			sta NBC
			lda #$7f			//And delete it from the block
NOP1:		sta (ZP01ff),y		//So that it does not confuse the depacker...

			lsr Random			//Check if this is also the first block of a randomly accessed bundle
			bcc ChkScnd

NOP2:		sta $0100			//This is the first block of a random bundle, delete first byte
			lda BPtr			//Last byte of bundle will be pointer to first byte of new Bundle
NOP3:		sta (ZP0101),y

ChkScnd:	lda WantedCtr
			bne StoreLoop

//--------------------------------------//If this is the last sector on a track, track change can be started here...
//										//...except if this is the first block of a bundle...
//		Early Track change				//...because we don't know if the next call will be sequential or random
//										//...OR if we have the last sector of the bundle in the second buffer
//--------------------------------------

SkipLast:	lda SCtr			//Skip track change if either of these is not zero: SCtr > 0
			ora NewBundle		//NewBundle condition (this is the first full block of a new bundle and SCtr = 0)
			ora ScndBuff		//We have the last block of a bundle in the second buffer and SCtr = 0)
			bne ToCATN

			lda NBC				//Very last sector?
			beq ToCATN			//Yes, skip stepping, finish transfer

//--------------------------------------
//		Prepare seeking
//--------------------------------------
								//Otherwise, clear wanted list and start seeking
PrepStep:	sec					//Signal to use JMP instead of RTS -> returns to NextTrack
			jmp JmpClrList		//Y=#$00 here

//--------------------------------------

ToCATN:		jmp CheckATN

//--------------------------------------
//		Mark wanted sectors
//--------------------------------------

NxtSct:		inx
Build:		iny					//Temporary increase as we will have an unwanted decrease after BNE
			lda #$ff			//Needed if nS = last sector of track and it is already fetched
			bne MaxSct1			//Branch ALWAYS

ChainLoop:	lda WList,x			//Check if sector is unfetched (=00)
			bne NxtSct			//If sector is not unfetched (it is either fetched or wanted), go to next sector

			lda #$ff
MarkSct:	sta WList,x			//Mark Sector as wanted (or used in the case of random bundle, STA <=> STY)
			stx LastS			//Save Last Sector
IL:			axs #$00			//Calculate Next Sector using inverted interleave
MaxSct1:	cpx MaxSct2+1		//Reached Max?
			bcc SkipSub			//Has not reached Max yet, so skip adjustments
MaxSct2:	axs #$00			//Reached Max, so subtract Max
			beq SkipSub
SubSct:		axs #$01			//Decrease if sector > 0
SkipSub:	dey					//Any more blocks to be put in chain?
			bne ChainLoop
			stx nS
			rts					//A=#$ff, X=next sector, Y=#$00, Z=0 here

//--------------------------------------

NextTrack:	ldx cT				//All blocks fetched in this track, so let's change track
			ldy #$81			//Prepare Y for 0.5-track seek

			inx					//Go to next track

ChkDir:		cpx #$12			//next track = Track 18?, if yes, we need to skip it
			bne Seek			//0.5-track seek, skip setting timer

			inx					//Skip track 18

			inc nS				//Skipping Dir Track will rotate disk a little bit more than a sector...
			inc nS				//...(12800 cycles to skip a track, 10526 cycles/sector on track 18)...
								//...so start sector of track 19 is increased by 2

			ldy #$83			//1.5-track seek, set timer at start

//--------------------------------------
//		Stepper Code			//X=Wanted Track
//--------------------------------------

StepTmr:	lda #$98
			sta $1c05

Seek:		lda $1c00
PreCalc:	//anc #$1b			//ANC NOW WORKS ON THE ULTIMATE-II+ \o/
			and #$1b			//but we keep AND+CLC because the original 1541U no longer receives firmware updates :(
			clc
			adc StepDir			//#$03 for stepping down, #$01 for stepping up
			ora #$0c			//LED and motor ON
			cpy #$80
			beq StoreTrack		//This was the last half step precalc, leave Stepper Loop without updating $1c00
			sta $1c00

			dey
			cpy #$80
			beq PreCalc			//Ignore timer, precalculate last half step and leave Stepper Loop (after 0.5/1.5 track changes)

StepWait:	bit $1c05
			bmi StepWait

			cpy #$00
			bne StepTmr

StoreTrack:	stx cT

//--------------------------------------
//		Set Bitrate
//--------------------------------------

BitRate:	ldy #$11			//Sector count=17
			cpx #$1f			//Tracks 31-40, speed zone 0
			bcs RateDone		//Bitrate=%00

			iny					//Sector count=18
			cpx #$19			//Tracks 25-30, speed zone 1
			bcs BR20			//Bitrate=%01

			iny					//Sector count=19
			ora #$40			//Bitrate=%10
			cpx #$12			//Tracks 18-24, speed zone 2
			bcs RateDone
								//Tracks 01-17, speed zone 3
			ldy #$15			//Sector count=21
BR20:		ora #$20			//Bitrate=%11

//--------------------------------------
//		Update variables
//--------------------------------------

RateDone:	sty MaxSct2+1		//Three extra bytes here but faster loop later

			ldx ILTab-$11,y		//Inverted Custom Interleave Tab
			stx IL+1

			ldx #$01			//Extra subtraction for Zone 3
			stx StepDir			//Reset stepper direction to Up/Inward here
			cpy #$15
			beq *+3
			dex
			stx SubSct+1

			lsr BitRateRet
			bcc StoreBR
OPC_RTS:	rts

StoreBR:	sta Spartan+1		//Store bitrate for Spartan step
			lda Random
			bne *+4
			sty SCtr			//Reset Sector Counter, but only if this is not a random block which gets SCtr from Dir

//--------------------------------------
//		GCR loop patch
//--------------------------------------

			lda #<GCRLoop3-(GCREntry+2)
			cpy #$15						//Y=sector count (17, 18, 19, 21 for zones 0, 1, 2, 3, respectively)
			beq *+4
			lda #<GCRLoop0_2-(GCREntry+2)	//GCR Loop patch for zones 0-2
			sta.z GCREntry+1

			lda.z Mod2a+2					//Restore LoopMod2 for zones 2-3
			ldx.z Mod2a+3

			cpy #$13
			bcs UpdatePatch
			lda #<OPC_BNE
			ldx #<Mod2a-(LoopMod2+2)		//GCR Loop Patch for zone 1
			cpy #$11
			bne UpdatePatch
			ldx #<Mod2b-(LoopMod2+2)		//GCR Loop Patch for zone 0

UpdatePatch:
			sta.z LoopMod2
			stx.z LoopMod2+1

			lsr StepTmrRet
			bcs OPC_RTS

//--------------------------------------

			lsr TrackChg		//Are we changing tracks after CATN?
			bcc CheckATN		//No, goto CATN
			jmp StartTr			//Yes, go back to transfer (JSR/RTS cannot be used)

//--------------------------------------

Reset:		jmp ($fffc)

//--------------------------------------
//		Wait for C64
//--------------------------------------

CheckATN:	lda $1c00			//Fetch Motor and LED status
			ora #$08			//Make sure LED will be on when we restart
			tax					//This needs to be done here, so that we do not affect Z flag at Restart

Frames:		ldy #$64			//100 frames (2 seconds) delay before turning motor off (#$fa for 5 sec)
DelayOut:	lda #$4f			//Approx. 1 frame delay (20000 cycles = $4e20 -> $4e+$01=$4f)
			sta $1c05			//Start timer, wait 2 seconds before turning motor off
DelayIn:	lda $1c05
			bne ChkLines
			dey
			bne DelayOut
			lda #$73			//Timer finished, turn motor off
			sax $1c00
			lda #<CSV			//Reset Checksum Verification Counter
			sta VerifCtr		//I.e. when motor restarts, first we verify proper read
			
ChkLines:	lda $1800
			bpl Reset			//ATN released - C64 got reset, reset the drive too
			//lsr					//A=#$00/#$02 after this, if C=1 then no change
			alr #$05			//A=#$00/#$02 after this, if C=1 then no change
			bcs DelayIn
			//lda $1800
			//and #$04
								//C=0, file requested
Restart:	stx $1c00			//Restart Motor and turn LED on if they were turned off
			beq SeqLoad			//A=#$00 - sequential load
								//A=#$02 - random load

//--------------------------------------
//
//		Random File Access
//
//--------------------------------------

GetByte:	lda #$80			//$dd00=#$9b, $1800=#$94
			ldx #busy			//X=#$10 (AA=1, CO=0, DO=0) - WILL BE USED LATER IF SAVER CODE IS NEEDED

TrRnd:		jsr RcvByte			//OK to use stack here

TrRndRet:	cmp #$ff
			beq Reset			//C64 requests drive reset

TestRet:	ldy #$00			//Needed later (for FetchBAM if this is a flip request, and FetchDir too)
			sty EoD				//EoD needs to be cleared here
			sty NewBundle		//So does NewBundle
			sty ScndBuff		//And ScndBuff as well

			asl
			bcs NewDiskID		//A=#$80-#$fe, Y=#$00 - flip disk
			beq CheckDir		//A=#$00, skip Random flag (first bundle on disk)
			inc Random
CheckDir:	ldx #$11			//A=#$00-#$7f, X=#$11 (dir sector 17) - DO NOT CHANGE TO INX, IT IS ALSO A JUMP TARGET!!!
			asl
			sta DirLoop+1		//Relative address within Dir segment
			bcc CompareDir
			inx					//A=#$40-#$7f, X=#$12 (dir sector 18)
			cmp #$f8			//Index=#$7e - check if we are loading the Saver Code
			bne CompareDir
			lda IncSaver		//A=#$02 if Saver Code is included on Disk, #$01 for transfer test, #$00 otherwise
			sta SaverCode

CompareDir:	cpx DirSector		//Dir Sector, initial value=#$c5
			beq DirFetchReturn	//Is the needed Dir Sector fetched?

			stx DirSector		//No, store new Dir Segment index and fetch directory sector
			stx LastS			//Also store it in LastS to be fetched
			jmp FetchDir		//ALWAYS, fetch directory, Y=#$00 here (needed)

DirFetchReturn:
			ldx #$03
DirLoop:	lda $0700,x
			sta LastT,x
			dex
			bpl DirLoop

			jsr ClearList		//Clear Wanted List, Y=00 here

			inc BitRateRet

			tax					//X=A=LastT
			jsr BitRate			//Update Build loop, Y=MaxSct after this
								//Also find interleave and sector count for requested track

			ldx LastS			//This is actually the first sector on the track here
			tya					//A=MaxSct
			sec
			sbc SCtr			//Remaining sectors on track
			tay					//Y=already fetched sectors on track
			beq SkipUsed		//Y=0, we start with the first sector, skip marking used sectors
			dec MarkSct			//Change STA ZP,x to STY ZP,x ($95 -> $94) (A=$ff - wanted, Y>#$00 - used)
			jsr Build			//Mark all sectors as USED -before- first sector to be fetched
			inc MarkSct			//Restore Build loop

SkipUsed:	iny					//Mark the first sector of the new bundle as WANTED
			jsr Build			//A=#$ff, X=Next Sector, Y=#$00 after this call

			lax LastT
			jmp GotoTrack		//X=desired Track, Y=#$00

//--------------------------------------

NewDiskID:	lsr					//Next Disk's ID for flip detection
			sta NextID

ToFetchBAM:	jmp FetchBAM		//Go to Track 18 to fetch Sector 0 (BAM) for Next Side Info, A=#$12, Y=#$00

//--------------------------------------
//
//		Sequential Loading
//
//--------------------------------------

SeqLoad:	tay					//A=#$00 here -> Y=#$00
			lsr EoD				//End of Disk?
			bcs ToFetchBAM		//If Yes, load BAM, otherwise start transfer

//--------------------------------------
//
//		Check if Late Track Change is needed here
//
//--------------------------------------

			lsr NewBundle 
			bcc StartTr

			lda NBC
			beq StartTr
		
			lda SCtr
			bne StartTr
		
			//Last sector of track and not last sector of last bundle and not first sector of a new bundle 
		
			inc TrackChg
			jmp PrepStep		//Needs Y=#$00, JSR cannot be used

StartTr:	ldy #$00			//transfer loop counter
			ldx #$ef			//bit mask for SAX
			lda #ready			//A=#$08, ATN=0, AA not needed
TrSeq:		sta $1800

//--------------------------------------
//		Transfer loop
//--------------------------------------
								//			Spartan Loop:		Entry:
Loop:		lda $0100,y			//03-06			19-22			00-03
			bit $1800			//07-10			23-26			04-07
			bmi *-3				//11 12			27 28			08 09
W1:			sax $1800			//13-16			29-32			10-13
								//(17 cycles) 	(33 cycles)

			dey					//00 01
			asl					//02 03
			ora #$10			//04 05
			bit $1800			//06-09
			bpl *-3				//10 11
W2:			sta $1800			//12-15
								//(16 cycles)

			ror					//00 01
			alr #$f0			//02 03
			bit $1800			//04-07
			bmi *-3				//08 09
W3:			sta $1800			//10-13
								//(14 cycles)

			lsr					//00 01
			alr #$30			//02 03
ByteCt:		cpy #$101-Sp		//04 05			Sending #$52 bytes before Spartan Stepping (#$59 for $1a bycles)
			bit $1800			//06-09			#$52 x 72 = #$17 bycles*, #$31 bycles left for 2nd halftrack step and settling
			bpl *-3				//10 11			*actual time is #$18-$1a+ bycles, and can be (much) longer
W4:			sta $1800			//12-15			if C64 is not immediately ready to receive fetched block
								//(16 cycles)	thus, the drive may actually stop on halftracks before transfer continues
		
			bcs Loop			//00-02

//--------------------------------------
//	SPARTAN STEPPING (TM)				<< - Uninterrupted data transfer across adjacent tracks - >>
//--------------------------------------		Transfer starts 1-2 bycles after first halftrack step initiated

Spartan:	lda #$00			//02 03		Last halftrack step is taken during data transfer
			sta $1c00			//04-07		Update bitrate and stepper with precalculated value
			tya					//08 09		Y=#$ae or #$00 here
			eor #$101-Sp		//10 11		#$31 bycles left for the head to take last halftrack step...
			sta ByteCt+1		//12-15		... and settle before new data is fetched
ChkPt:		bpl Loop			//16-18

.print ""
.print "Loop:  $0" + toHexString(Loop)
.print "ChkPt: $0" + toHexString(ChkPt)

.if ([>Loop] != [>ChkPt]) {
.error "ERROR!!! Transfer loop crosses pages!!!"
}

//--------------------------------------

			lda #busy			//16,17 		A=#$10
			bit $1800			//18,19,20,21 	Last bitpair received by C64?
			bmi *-3				//22,23
			sta $1800			//24,25,26,27	Transfer finished, send Busy Signal to C64
			
			bit $1800			//Make sure C64 pulls ATN before continuing
			bpl *-3				//Without this the next ATN check may fall through
								//resulting in early reset of the drive

			iny					//Y=#$01
TrSeqRet:	sty Loop+2			//Restore transfer loop
			
			jsr ToggleLED		//Transfer complete - turn LED off, leave motor on

//--------------------------------------
//		Update Block Counter
//--------------------------------------

			dec BlockCtr		//Decrease Block Counter
			bne ChkWCtr

UpdateBCtr:	inc NewBundle		//#$00 -> #$01, next block will be first of next Bundle
			lda NBC				//New Block Count
			sta BlockCtr
			bne ChkWCtr			//A = Block Count

			inc EoD				//New BCtr=#$00 - this is the end of the disk
JmpCATN:	jmp CheckATN		//No more blocks to fetch in sequence, wait for next loader call
								//If next loader call is sequential -> will go to BAM for flip check/reset
								//If next loader call is random -> will load requested file
//--------------------------------------

ChkWCtr:
			lda WantedCtr		//If we just updated BlockCtr then WantedCtr will be 0
			bne JmpFetch2		//If there are more blocks on the list then fetch next

			lsr ScndBuff		//No more blocks on wanted list, check if the last block has been stored...
			bcc CheckBCtr		//If we do not have the last block stored then check Bundle counter
								//Last block of Bundle stored, so transfer it
			inc Loop+2			//Modify transfer loop to transfer data from secondary buffer ($0100 -> $0200)
			ldy SCtr
			bne JmpCATN			//If more sectors left in track, then transfer this block without seeking to next track
			jmp PrepStep		//Otherwise, seek to next track during transferring this block, A=#$00 here, needed after jump

//--------------------------------------
//		Build wanted list		//A=#$00, X=#$ef here
//--------------------------------------

CheckBCtr:	ldy SCtr			//Check if we have less unfetched sectors left on track than blocks left in Bundle
			cpy BlockCtr
			bcc NewWCtr			//Pick the smaller of the two for new Wanted Counter
			ldy BlockCtr
			ldx cT				//If SCtr>=BlockCtr then the Bundle will end on this track...
NewWCtr:	sty WantedCtr		//Store new Wanted Counter (SCtr vs BlockCtr whichever is smaller)
			stx LastT			//...so save current track to LastT, otherwise put #$ef to corrupt check

			ldx nS				//Preload Next Sector in chain
			jsr Build			//Build new wanted list (buffer transfer complete, JSR is safe)
JmpFetch2:	jmp Fetch			//then fetch

//--------------------------------------

EndOfDriveCode:

		.if (EndOfDriveCode > $0700)
		{
			.error "Error!!! Drive code too long!!!" + toHexString(EndOfDriveCode)
		}
}

//----------------------------------------------------------

*=$2800 "Installer"

.pseudopc $0700 {

//--------------------------------------
//		Initialization	//$0700
//--------------------------------------

CodeStart:	ldx #$06
			lda #$12			//Track 18
			ldy #$0e			//Sectors 14,13,12,11
!:			sta $06,x
			sty $07,x
			dey
			dex
			dex
			bpl !-
			dec $f9				//Buffer pointer, it was #$04 before dec
!:			jsr $d586			//Load 4 blocks to buffers 03,02,01,00
			dec $f9
			bpl !-

			sei

//--------------------------------------
//		Copy ZP code and tabs
//--------------------------------------

			ldx #$00			//Technically, this is not needed - X=$00 after loading all 5 blocks
ZPCopyLoop:	lda ZPTab,x			//Copy Tables C, E, F and GCR Loop from $0600 to ZP
			sta $00,x
			inx
			bne ZPCopyLoop

//--------------------------------------

			lda #$ee			//Read mode, Set Overflow enabled
			sta $1c0c			//could use JSR $fe00 here...

			lda $1c00			//Turn motor and LED on, set bitrate.
			and #$93
			ora #$4c			//1    1*   0*   1    1*   1*   1    0
			sta $1c00			//SYNC BITR BITR WRTP LED  MOTR STEP STEP

			lda #$7a
			sta $1802			//0  1  1  1  1  0  1  0  Set these 1800 bits to OUT (they read back as 0)
			lda #busy
			sta $1800			//0  0  0  1  0  0  1  0  CO=0, DO=1, AA=1 This reads as #$43 on $dd00
								//AI|DN|DN|AA|CO|CI|DO|DI This also signals that the drive code has been installed

.if (VerifSctHdrs)
{			lda #$01			//Shift register disabled, Port A ($1c01) latching enabled, Port B ($1c00) latching disabled
			sta $1c0b

			lda #$00			//Clear VIA #2 Timer 1 low byte
			sta $1c04

			lda #$7f			//Disable all interrupts
			sta $180e
			sta $1c0e
			lda $180d			//Acknowledge all pending interrupts
			lda $1c0d
}
			jmp Fetch			//Fetching block 3 (track 18, sector 16) WList+$10=#$ff, WantedCtr=1
								//A, X, Y can be anything here
//--------------------------------------
//		Copy block 3 to $0600
//--------------------------------------

CopyCode:
CCLoop:		pla					//=lda $0100,y
			iny					//Y=00 at start
			sta $0600,y			//Block 3 is EOR transformed and rearranged, just copy it
			bne CCLoop
			lda #<CopyDir		//Change JMP CopyCode to JMP CopyDir
			sta ToCD+1
			lda #>CopyDir
			sta ToCD+2
			tya					//A=#$00 - Bundle #$00 to be loaded
			jmp CheckDir		//Load 1st Dir Sector and then first Bundle, Y=A=#$00
CD:
}

//-----------------------------------------------------------------
//
//			ZP TABS AND CONSTANTS
//
//-----------------------------------------------------------------

*=$2700 "ZP Tabs and GCR Loop"
.pseudopc $0600 {
ZPTab:
//		 x0  x1  x2  x3  x4  x5  x6  x7  x8  x9  xa  xb  xc  xd  xe  xf
.byte	$12,$00,$04,$01,$f0,$60,$b0,$20,$01,$40,$80,$00,$e0,$c0,$a0,$80	//0x
.byte	<BAM_IncSave,>BAM_IncSave
.byte			$2e,$1e,$ae,$1f,$be,$17,$00,CSV,$6e,$1a,$ee,$1b,$fe,$13	//1x
.byte	$01,$00,$14,$00,$8e,$1d,$9e,$15,$01,$00,$5e,$10,$ce,$19,$de,$11	//2x
.byte	$7f,$76,$3e,$16,$0e,$1c,$1e,$14,$76,$7f,$7e,$12,$4e,$18,$00,$00	//3x	Wanted List $3e-$52 (Sector 16 = #$ff)
.byte	$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$ff,$00	//4x	(0) unfetched, (+) fetched, (-) wanted
.byte	$00,$00,$00,$0e,$00,$0f,$c5,$07,$ff,$01,$01,$0a,$1e,$0b,$00,$03	//5x
.byte	$fd,$fd,$fd,$00,$fc,$0d,$00,$05,XX3,XX4,XX1,$00,$00,$09,$00,$01	//6x	$60-$64 - ILTab, $63 - NextID
.byte	<ILTab-1,>ILTab-1
.byte			XX4,$06,$02,$0c,$00,$04,<ProductID-1,>ProductID-1
.byte											$00,$02					//7x
}
//007c
GCRLoop:
.pseudopc GCRLoop-$2700 {

//--------------------------------------
//		124-CYCLE GCR LOOP ON ZP
//--------------------------------------

//------------------------------------------------------------------------------------------------------
								//						Cycles							Address
								//						Zone 3	Zone 2	Zone 1	Zone 0
Mod2:		//bne Mod2b			//										--		85
Mod2b:		cmp ($08,x)			//$08 =TabF value						--		91		7c	7d
			nop					//										--		93		7e

			//bne Mod2a			//										85
Mod2a:		nop					//										87		95		7f
			nop					//										89		97		80
			alr #$fc			//										91		99		81	82
			bne LoopMod2+2		//										94		102		83	84
			//tax				//										96		104

//------------------------------------------------------------------------------------------------------

								//						Zone 3	Zone 2	Zone 1	Zone 0
GCRLoop0_2:	eor $0102,x			//First pass: X=#$00	--		36		36		36		85-87
			eor $0103,x			//						--		40		40		40		88-8a

//------------------------------------------------------------------------------------------------------

								//						Zone 3	Zone 2	Zone 1	Zone 0
GCRLoop3:	sta.z PartialCSum+1	//						35		43		43		43		8b 8c

								//					   [26-51	28-55	30-59	32-63]
			lda $1c01			//DDDDEEEE				39/-12	47/-8	47/-12	47/+15	8d-8f
			ldx #$0f			//						41		49		49		49		90 91
			sax.z tE+1			//tE+1=0000EEEE			44		52		52		52		92 93
			arr #$f0			//						46		54		54		54		94 95
			tay					//Y=DDDDD000			48		56		56		56		96

tC:			lda TabC			//00CCCCC0 (ZP)			51		59		59		59		97 98
tD:			eor TabD,y			//00000001,DDDDD000		55		63		63		63		99-9b
			pha					//$0104/$0100			58		66		66		66		9c
								//$0104 = Checksum
PartialCSum:
			eor #$7f			//						60		68		68		68		9d 9e
CSum:		eor #$00			//						62		70		70		70		9f a0
			sta.z CSum+1		//						65		73		73		73		a1 a2
								//				       [52-77	56-83	60-89	64-95]
			lda $1c01			//EFFFFFGG				69/-8	77/-6	77/-14	77/+11	a3-a5
			ldx #$03			//						71		79		79		79		a6 a7
			sax.z tG+1			//tG+1=000000GG			74		82		82		82		a8 a9
LoopMod2:	alr #$fc			//					C=0	76		84		--		--		aa ab
			tax					//X=0EFFFFF0			78		86		96		104		ac

tE:			lda TabE			//0000EEEE (ZP)			81		89		99		107		ad ae
tF:			adc TabF,x			//00000001,0EFFFFF0 (ZP)85		93		103		111		af b0
			pha					//$0103/$01ff			88		96		106		114		b1
								//$0103 = Sector
								//				       [78-103	84-111	90-119	96-127]
			lax $1c01			//GGGHHHHH				92/-11	100/-11	110/-9	118/-9	b2-b4
			alr #$e0			//A=0GGG0000			94		102		112		120		b5 b6
			tay					//Y=0GGG0000			96		104		114		122		b7
			lda #$1f			//						98		106		116		124		b8 b9
			axs #$00			//X=000HHHHH		C=1	100		108		118		126		ba bb

tG:			lda TabG,y			//000000GG,0GGG0000		104		112		122		130		bc-be
tH:			eor TabH,x			//10001011,000HHHHH		108		116		126		134		bf-c1
			pha					//$0102/$01fe			111		119		129		137		c2
								//$0102 = Track
			lax ZP07			//						114		122		132		140		c3 c4
								//					   [104-129	112-139	120-149	128-159]
			sbc $1c01			//AAAAABBB			V=0	118/-11	126/-13	136/-13	144/-15	c5-c7
			sax.z tB+1			//tB+1=-00000BBB		121		129		139		147		c8 c9
			
			alr #$f8			//						123		131		141		149		ca cb

								//Total length (cycles):124		132		142		150

//------------------------------------------------------------------------------------------------------

								//						Zone 3	Zone 2	Zone 1	Zone 0
			bvc *				//						00-01							cc cd

			tay					//Y=-0AAAAA00			03								ce
								//					   [00-25	00-27	00-29	00-31]	
			lda $1c01			//BBCCCCCD				07/-18	07/-20	07/-22	07/-24	cf-d1
			ldx #$3e			//						09								d2 d3
			sax.z tC+1			//tC+1=00CCCCC0			12								d4 d5
			alr #$c1			//						14								d6 d7
			tax					//X=0BB00000		C=D	16								d8

tA:			lda TabA,y			//00010010,-0AAAAA00	20								d9-da
tB:			eor TabB,x			//-00000BBB,0BB00000	24								db-de
			pha					//$0101,$01fd			27								df
								//$0101 = ID2
			tsx					//SP = $00/$fc ...		29								e0
GCREntry:	bne GCRLoop0_2		//We start on Track 18	32								e1 e2

//------------------------------------------------------------------------------------------------------

								//						Zone 3	Zone 2	Zone 1	Zone 0
			eor CSum+1			//						35		35		35		35		e3 e4
			tax					//Store checksum in X	37		37		37		37		e5
			clv					//						39		39		39		39		e6
								//					   [26-51	28-55	30-59	32-63]
			lda $1c01			//Final read = DDDDEEEE	43/-8	43/-12	43/+13	43/+11	e7-e9
			bvc *				//						01								ea eb
			arr #$f0			//A=DDDDD000			03								ec ed
			tay					//Y=DDDDD000			05								ee
			txa					//Return checksum to A	07								ef
			eor TabD,y			//Checksum (D)/ID1 (H)	11								f0-f2
			ldx tC+1			//X=00CCCCC0			14								f3 f4
			eor TabC,x			//(ZP)					18								f5 f6
			eor $0102			//						22								f7-f9
			eor $0103			//						26								fa-fc
ModJmp:		jmp (HeaderJmp)		//Calc final checksum	31								fd-ff

//------------------------------------------------------------------------------------------------------
}
