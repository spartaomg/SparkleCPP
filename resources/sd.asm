//TAB=4
//----------------------------------------------------------------------------------------
//	SPARKLE
//	Inspired by Lft, Bitbreaker, and Krill
//	Drive Code
//	Tested on 1541, 1541-II, 1571, the 1541 Ultimate series, and Oceanic drives
//----------------------------------------------------------------------------------------
//	- 2-bit + ATN protocol, combined fixed-order and out-of-order loading
//	- 124-cycle GCR read-decode-verify loop with 1 BVC instruction
//	- tolerates disk rotation speeds between 272-314 rpm in VICE in all 4 disk zones
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
//	v09	- drive transfer loop modified to work with a 16-byte H2STab instead of a 256-byte tab
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
//	v21	- track check in header verification code
//		  if the R/W head lands on the wrong track, then Sparkle will correct it
//
//	v22	- reversing transfer loop direction in C64 resident code
//		  this allows transfer loop patches in both the drive and C64 codes
//		- simplified GCR loop patch code saving 14 bytes (patch tables on page 3)
//		- code modifications to allow track change during file saving
//		- reworked plugin detection
//		  number of plugin blocks is stored in the two most significant bits of the Track byte in the directory
//		  allows multiple plugins per disk side
//		  plugins must be loaded using index-based loader calls
//		- simplified track seek code
//		  eliminating early track change during sequential loading (stepping before ATN check)
//		- reworked fetch code
//		- adding multiple sanity checks to header block
//		  ID1, ID2, Track, and Sector checks
//		  ID1 and ID2 are updated when reading from track 18 (during init and disk flips)
//		- trailing 0 first nibble check
//		- checksum error counter
//		  if number of consecutive read errors is greater than a full disk spin then
//		  Sparkle will cycle through bitrates then take half track steps until fetching is successful
//
//----------------------------------------------------------------------------------------
//	Memory Layout
//
//	0000	007d	ZP GCR tables and variables
//	007c	00ff	GCR loop and loop patches
//	0100	01ff	Data Buffer on Stack
//	0200	02ff	Second Data Buffer
//	0300	03f1	GCR tables with code interleaved
//	0411	0472	GCR table with code interleaved
//	0300	06f4	Drive Code
//	0700	07ff	Directory (4 bytes per entry, 64 entries per dir block, 2 dir blocks on disk)
//
//	Layout at Start
//
//	0300	03f1	GCR tables					block 0
//	0300	05ff	Code						blocks 0-2
//	0600	06ff	ZP GCR tables and GCR loop	block 3
//	0700	0736	Installer					block 4
//
//	Layout in PRG
//
//	2300	23f1	GCR tables					block 0
//	2411	2472	GCR table					block 1
//	2300	26f4	Drive Code					blocks 0-3 3 -> block 5
//	2700	27ff	ZP GCR tables and GCR loop	block 	   4 -> block 3
//	2800	2836	Installer					block 	   5 -> block 4
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
//	18:00:$fb	$0105	BAM_NextID	(will be copied to $23 after flip, $ff if no more flips)
//	18:00:$fa	$0106	IL0R		(will be copied to $24)
//
//	18:00:$f9	$0107	BAM_IncSaver	(no longer used)
//
//	18:00:$f8	$0108	BAM_ProdID (0)
//	18:00:$f7	$0109	BAM_ProdID (1)
//	18:00:$f6	$010a	BAM_ProdID (2)
//
//	TODO (wound save 4 bytes in code but needs 4 bytes for ProdID and DiskID and we only have 3 ATM...)
//	18:00:$ff	$0101	IL3R		(will be copied to $20)
//	18:00:$fe	$0102	IL2R		(will be copied to $21)
//	18:00:$fd	$0103	IL1R		(will be copied to $22)
//	18:00:$fc	$0104	BAM_NextID	(will be copied to $23 after flip, $ff if no more flips)
//	18:00:$fb	$0105	IL0R		(will be copied to $24)
//
//	18:00:$fa	$0106	BAM_ProdID (0)
//	18:00:$f9	$0107	BAM_ProdID (1)
//	18:00:$f8	$0108	BAM_ProdID (2)
//	18:00:$f7	$0109	BAM_DiskID	(for flip detection, compare to NextID @ $23 on ZP)
//
//----------------------------------------------------------------------------------------
//	Directory Structure
//
//	4 bytes per dir entry
//	128 dir entries in 2 dir blocks
//
//	00	Track (two most significant bits indicate plugin blocks)
//	01	First sector on track after track change (to mark fetched sectors, NOT first sector of bundle)
//	02	Sector Counter (to mark fetched sectors and first sector of bundle)
//	03	Byte Pointer (used by the depacker to find start of stream, to be copied to the last byte of first block)
//
//----------------------------------------------------------------------------------------

//Constants:

.label CSV			=$07		//Checksum Verification Counter Default Value (3 1-bits, 3 data blocks to be verified)

.label DO			=$02
.label CO			=$08
.label AA			=$10
.label busy			=AA			//DO=0,CO=0,AA=1	$1800=#$10	dd00=010010xx (#$43)
.label ready		=CO			//DO=0,CO=1,AA=0	$1800=#$08	dd00=100000xx (#$83)

.label Sp			=$52		//Spartan Stepping constant (=82*72=5904=$1710=$17 bycles delay)
.const ErrVal		=$2a		//=42 missed consecutive sectors, two full rotations in zode 3, 2.5 full rotations in zone 0

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
.label ErrCtr		=$54		//Checksum error counter
.label DirSector	=$56		//Initial value=#$c5 (<>#$10 or #$11)
.label NBC			=$5c		//New Block Count temporary storage
.label TrackChg		=$5e		//Indicates whether Track change is needed AFTER CATN (last block of bundle=last sector of track)

.label ILTab		=$60		//Inverted Custom Interleave Table ($60-$64)
.label NextID		=$63		//Next Side's ID - will be updated from 18:00:$fd of next side

.label ReturnFlag	=$66		//Indicates whether StepTimer code is called in subroutine

.label Plugin		=$76		//Indicates whether fetch block is a plugin

.label ZPBAMPID		=$10		//$10/$11 = $0107
.label ZPHdrID1		=$68
.label ZPHdrID2		=$69
.label ZPILTab		=$71
.label ZPProdID		=$78		//$78/$79 = $031a (ProdID-1)

.label ZP7f			=$30		//BitShufTab
.label ZP3e			=$32		//TabC value
.label ZP12			=$3b		//TabF value
.label ZP07			=$57		//TabF value
.label ZP00			=$6b		//TabF value
.label ZP01			=$59
.label ZP02			=$7b
.label ZP01ff		=$58		//$58/$59 = $01ff
.label ZP0101		=$59		//$59/$5a = $0101
.label ZP0200		=$6b		//$6b/$6c = $0200
.label ZP0100		=$6e		//$6b/$6c = $0100
.label ZP02ff		=$7a		//$7a/$7b = $02ff
.label ZPTabDm2		=$7a		//$7a/$7b = $02ff = TabD-2

//BAM constants:
.label BAM_DiskID	=$0101
.label BAM_NextID	=$0105
.label BAM_ProdID	=$0108

//Other constants:
.label SF			=$0139		//SS drive code Fetch vector
.label SH			=$013e		//SS drive code Got Header vector

.label OPC_ALR		=$4b
.label OPC_BNE		=$d0
.const OPC_NOP_ABS	=$0c

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
ClearList:	sec					//00
JmpClrList:	ldx #$14			//01 02
ClrWList:	sty WList,x			//03 04	Y=00, clear Wanted Block List
			dex					//05
			bpl ClrWList		//06 07
			bcc ClrJmp			//08 09
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
//031b
Patch1:
.byte												<OPC_BNE,<OPC_BNE,<OPC_ALR
.byte															$07
.byte																<OPC_ALR
.byte	$a5,$a1,$a7,XX2,$ad,$a9,$67,$04,$05,$0d,$77,$00,$01,$09,$47,XX3	//PC tool stores Version info at $032f
.byte	$07,$0f,XX4,$0a,$03,$0b,$a7,$0c,$06,$0e,$b7,$08,$02,XX1,$27,XX2	//PC tool stores release YY/MM/DD at $0331/$033d/$033f
.byte	$af,$ab,$ae,$aa,$ac,$a8,$e7
//0347
HeaderJmp:
.byte								<HD,>HD
.byte										$d0,$f7
//034b
ToggleLED:	lda $1c00			//4b-4d
ToggleLD2:	eor #$08			//4e 4f
			nop #$d1			//50 51	SKIPPING TabD value
			sta $1c00			//52-54
			rts					//55
.byte							$87
//0357
DataJmp:
.byte								<DT,>DT
.byte										$d9,$97
//035b
Patch2:
.byte												<Mod2b-(LoopMod2+2),<Mod2a-(LoopMod2+2),$fc
.byte															$17
.byte																$fc
.byte	XX3,$a3,$a6,$a2,$a4,$a0,$c7
//0367
SHeaderJmp:
.byte								<SH,>SH
.byte										$d4,$d7
//036b
ProductID:
.byte												XX4,XX1,XX2,$57
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
FetchDir:	jsr ClearList		//94-96 C=1 after this
			bcs *+3				//97 98 SKIPPING TabD value
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
CorrTrack:	sec					//a7	CorrTrack needs Y=#$01
			nop #$de			//a8 a9 Skipping TabD
			sbc cT				//aa ab	Calculate Stepper Direction and number of Steps
			beq ResetVerif		//ac ad	We are staying on the same Track, skip track change
			bcs SkipStepDn		//ae af
			nop #$d7			//b0 b1 Skipping TabD
			eor #$ff			//b2 b3
			adc #$01			//b4 b5
			ldy #$03			//b6 b7	Y=#$03 -> Stepper moves Down/Outward
			nop #$df			//b8 b9 TabD
			sty StepDir			//ba bb	Only store stepper direction DOWN/OUTWARD here (Y=#$03)
SkipStepDn:	ldy #$02			//bc bd
			sty ReturnFlag		//be bf	->#$02 - signals need for RTS 
			asl					//c0
			tay					//c1	Y=Number of half-track changes
			jsr StepTmr			//c2-c4	Move head to track and update bitrate (also stores new Track number to cT and calculates SCtr but doesn't store it)

//--------------------------------------
//		Multi-track stepping	//A=bitrate, Y=SCtr here
//--------------------------------------

			sta $1c00			//c5-c7	Needed to update bitrate!!!
			nop	#$d8			//c8 c9	Skipping TabD = CLD
ResetVerif:	lda #CSV			//ca cb
			sta VerifCtr		//cc cd	Verify track after head movement

//--------------------------------------
//		Fetch Code
//--------------------------------------
//039e
FT:
Fetch:
FetchHeader:
			ldy #<HeaderJmp		//ce cf	Checksum verification after GCR loop will jump to Header Code
			nop #$d2			//d0 d1 Skipping TabD value $d2 (CMP izy)
FetchSHeader:
			lda #$52			//d2 d3	First 8 bits of Header ID (01010010)
			ldx #$04			//d4 d5	4 bytes to stack
			txs					//d6	Header: $0104,$0103..$0101
			bne Presync			//d7 d8 Skip Data Block fetch
	.byte	$da					//d9	NOP (TabD)
				
FetchData:	ldy #<DataJmp		//da db	Checksum verification after GCR loop will jump to Data Code
			lda #$55			//dc dd	First 8 bits of Data ID (01010101)
								//SP = $00 here (LDX #$00; TXS not needed)

Presync:	sty.z ModJmp+1		//de df
			ldy #$00			//e0 e1
			sty.z CSum+1		//e2 e3
			sta (ZP0102),y		//e4 e5
			sta (ZP0103),y		//e6 e7
			nop #$dc			//e8 e9 Skipping TabD value $dc (NOP ABS,X)
			
			bit $1c00			//ea-ec
			bmi *-3				//ed ee

			clv					//ef
			nop #$d6			//f0 f1	Skipping TabD value (DEC ZP,X)		
			nop $1c01			//f2-f4
			ldx #$c0			//f5 f6
			
			bvc *				//f7 f8|00-01
			cmp $1c01			//f9-fb|05	Read1 = AAAAABBB -> H: 01010|010(01), D: 01010|101(11)
			bne ReFetch			//fc fd|07

			ror					//fe   |09	C=1 before ROR -> H: 10101001|0, D: 10101010|1
			ror					//ff   |11				   -> H: 01010100,   D: 11010101
			sax AXS+1			//00-02|15				   -> H: 01000000,   D: 11000000
			clv					//03   |17

			bvc *				//04 05|00-01
			lda $1c01			//06-08|05*	*Read2 = BBCCCCCCD -> H: 01CCCCCD
AXS:		axs #$00			//09 0a|07						  D: 11CCCCCD
			bne ReFetch			//0b 0c|09	X = BB000000 - X1000000, if X = 0 then proper block type is fetched
			ldx #$3e			//0d 0e|11
			sax.z tC+1			//0f 10|14

	.byte	$fa					//11   |16	TabG (NOP)
			lsr					//12   |18
	.byte	$ea					//13   |20	TabG (NOP)

			ldx #$00			//14 15|22
			lda #$7f			//16 17|24	Z=0, needed for BNE in GCR loop
			jmp GCREntry		//18-1a|27	Same cycle count as in GCR loop before BNE
			
//--------------------------------------
//		Mark wanted sectors
//--------------------------------------

NxtSct:		inx					//1b
Build:		iny					//1c	Temporary increase as we will have an unwanted decrease after BNE
BuildSvr:	lda #$ff			//1d 1e	Needed if nS = last sector of track and it is already fetched
			bne MaxNumSct1		//1f 20	Branch ALWAYS

	.byte	$7a					//21	Skipping TabG value (NOP)
	.byte	$5a					//22   	Skipping TabG value (NOP)
	.byte	$da					//23   	TabG (NOP)

ChainLoop:	lda WList,x			//24 25	Check if sector is unfetched (=00)
			bne NxtSct			//26 27	If sector is not unfetched (it is either fetched or wanted), go to next sector

			lda #$ff			//28 29
MarkSct:	sta WList,x			//2a 2b	Mark Sector as wanted (or used in the case of random bundle, STA <=> STY)
			stx LastS			//2c 2d	Save Last Sector
IL:			axs #$00			//2e 2f	Calculate Next Sector using inverted interleave

	.byte	OPC_NOP_ABS			//30	Skip next two TabG values to avoid trashing A
	.byte	$6a					//31 	TabG (ROR)
	.byte 	$4a					//32	TabG (LSR)
	.byte	$ca					//33	TabG (DEX)
			inx					//34 	compensate for TabG DEX
			
MaxNumSct1:	cpx MaxNumSct2+1	//35-37	Reached Max?
			bcc SkipSub			//38 39	Has not reached Max yet, so skip adjustments
MaxNumSct2:	axs #$13			//3a 3b	Reached Max, so subtract Max
			beq SkipSub			//3c 3d
SubSct:		axs #$01			//3e 3f	Decrease if sector > 0
SkipSub:	dey					//40	Any more blocks to be put in chain?
			bne ChainLoop		//41 42
			stx nS				//43 44
			rts					//45	A=#$ff, X=next sector, Y=#$00, Z=0 here

//--------------------------------------

ReFetch:	jmp (FetchJmp)		//46-48	Refetch everything, vector is modified from SS!!!

//--------------------------------------
//		Flip Detection			//A/X/Y=#$00 here
//--------------------------------------

FlipDtct:	lda NextID			//49 4a	Side is done, check if there is a next side
			cmp (ZP0101),y		//4b 4c	DiskID, compare it to NextID
			bne ReFetch			//4d 4e ID mismatch, fetch again until flip detected

			ldy #$03			//4f 50

	.byte	$ba					//51	TabG (TSX) (X=#$00, no change)
	.byte	$8a					//52	TabG (TXA) (A=#$00)
	.byte	$aa					//53	TabG (TAX)

ProdIDLoop:	lda (ZPBAMPID),y	//54 55	Also compare Product ID, only continue if same ($0107 = BAMProdID-1)
			cmp (ZPProdID),y	//56 57
BneReFetch:	bne ReFetch			//58 59	Product ID mismatch, fetch again until same
			dey					//5a
			bne ProdIDLoop		//5b 5c

			ldy #$05			//5d 5e
			sty DirSector		//5f 60	Invalid value to trigger reload of the directory of the new disk side

	.byte	$3a					//61	TabG (NOP)
	.byte	$1a					//62	TabG (NOP)
	.byte	$9a					//63	TabG (TXS) no effect X=SP=#$00

CopyBAM:	lda (ZP0101),y		//64 65	($0101=DiskID), $102=IL3R, $103=IL2R, $104=IL1R, $0105=NextID, $106=IL0R
			sta (ZPILTab),y		//66 67
			dey					//68		
			bne CopyBAM			//69 6a
			tya					//6b
			jmp CheckDir		//6c-6e	Y=A=#$00 - Reload first directory sector

//--------------------------------------
//		Got Header
//--------------------------------------
//0473
HD:								//Mem	Cycles
Header:		bne FetchError		//6f 70	33	Checksum mismatch -> fetch next sector header

	.byte	$2a					//71	35	TabG (ROL) - A not needed here, no effect
	.byte	$0a					//72	37	TabG (ASL) - A not needed here, no effect

			lda (ZPTabDm2),y	//		43	Y=DDDDD010
			eor TabC,x			//		47	X=00CCCCC0
			tay					//		49	Y = ID1

			lda $0103			//		53	$0103 (Sector)
			jsr ShufToRaw		//		73
			cmp MaxNumSct2+1	//		77
			bcs FetchError		//		79	Sector mismatch
			sta cS				//		82

			lda $0102			//		86	A = $0102 (Track)
			jsr ShufToRaw		//		106
			beq FetchError		//		108	Track mismatch
			cmp #$29			//		110	Max track: 40 ($28)
			bcs FetchError		//		112	Track mismatch			
			
			ldx $0101			//		116 X = $0101 (ID2)
			cpx ZPHdrID2		//		119
			bne CheckIDs		//		121	ID2 mismatch
			
			cpy ZPHdrID1		//		124
			bne CheckIDs		//		126	ID1 mismatch

			ldx cT				//		129
			sta cT				//		132
			txa					//		134
			cmp cT				//		137
			bne ToCorrTrack		//		139	Wrong track -> go to wanted track
						
			ldy VerifCtr		//		142
			bne ToFetchData		//		144	Always fetch sector data if we are verifying the checksum

			ldy cS				//		147
			ldx WList,y			//		151	Otherwise, only fetch sector data if this is a wanted sector
BplFetch:	bpl ReFetch			//		153	Sector is not on wanted list -> fetch next sector header
ToFetchData:
			jmp FetchData		//		156	Sector is on wanted list -> fetch data

//--------------------------------------

ToCorrTrack:
			ldy #$01
			jmp CorrTrack

//--------------------------------------
//		Disk ID Check			//Y=#$00 here
//--------------------------------------

Track18:	txa					//BAM (Sector 0) or Drive Code Block 3 (Sector 16) or Dir Block (Sectors 17-18)?
			beq FlipDtct		//X=$00 (BAM)
ToCD:		jmp CopyCode		//Sector 16 (Block 3) - copy it from the Buffer to its place, will be changed to JMP CopyDir after Block 3 copied

//--------------------------------------
//		Check Disk IDs
//--------------------------------------
			
CheckIDs:	cmp cT
			bne FetchError
			cmp #$12
			bne FetchError

			sty ZPHdrID1		//Update disk ID1 and ID2 if we are on Track 18 (i.e. after disk flip)
			stx ZPHdrID2

//--------------------------------------
//		Fetch error
//--------------------------------------

FetchError:	dec ErrCtr
ToBneReFetch:
			bne	BneReFetch

			lda $1c00			//Based on Krill's track correction code
			and #$63
			clc
			adc #$e0			//Cycle through bitrates %11 -> %10 -> %01 -> %00 -> %11
			adc #$03			//Half track step down on bitrate wrap around
			ora #$0c			//Motor and LED on
			sta Spartan+1		//Update Spartan step $1c00 value (with LED on)
			jsr ToggleLD2		//Update $1c00 with LED off (LED is only on during transfer), JSR is OK here, we are discarding any fetched data on the stack

			lda #<ErrVal		//More than the max number of sectors per track
			sta ErrCtr	

			bne ToBneReFetch	//Refetch everything via ReFetch vector

//--------------------------------------
//		Got Data
//--------------------------------------
//04e0
DT:
Data:		ldx cT
			cpx #$12
			bcs SkipCSLoop  
/*
			ldy #$fc			//This loop saves 8 bytes but takes 1198 cycles (46 bytes passing under R/W head in zone 3)
CSLoop:		eor $0102,y
			eor $0103,y
			dey
			dey
			dey
			dey
			bne CSLoop
*/
			ldy #$7e			//This loop takes 856 cycles (33 bytes passing under R/W head in zone 3) but needs 8 more bytes
			bne CSLoopEntry
CSLoop:		eor $0102,y
			eor $0103,y
			dey
			dey
CSLoopEntry:
			//eor $0140,y		//ldx #$3e for this, takes 680 cycles (26 bytes passing under the R/W head), needs 12 more bytes
			//eor $0141,y
			eor $0180,y
			eor $0181,y
			//eor $01c0,y
			//eor $01c1,y
			dey
			dey
			bne CSLoop

SkipCSLoop:	tay
			bne FetchError		//Checksum mismatch, fetch header again
			
			lda #<ErrVal
			sta ErrCtr			//Reset Error Counter (only if both header and data are fetched correctly)

			lsr VerifCtr		//Checksum Verification Counter
			bcs BplFetch		//If counter<>0, go to verification loop (use BPL Fetch as trampoline, VerifCtr is always positive)

RegBlockRet:
			txa
			ldx cS				//Current Sector in Buffer
			cmp #$12			//If this is Track 18 then we are fetching Block 3 of the drive code or a Dir Block or checking Flip Info
			beq Track18			//We are on Track 18

//--------------------------------------
//		Update Wanted List
//--------------------------------------

			sta WList,x			//Sector loaded successfully, mark it off on Wanted list (A=Current Track - always positive)
			dec SCtr			//Update Sector Counter

//--------------------------------------
//		Check Saver Code
//--------------------------------------

			asl Plugin
			bcc ChkLastBlock
			jmp $0100			//Plugin code drive block fetched

//--------------------------------------

Reset:		jmp ($fffc)

//--------------------------------------
//		Store Transitional Block
//--------------------------------------

StoreLoop:	pla
			iny
			sta (ZP0200),y
			bne StoreLoop

			inc ScndBuff
			bne ToBneReFetch	//ALWAYS back to Fetch

//--------------------------------------
//		Check Last Block		//Y=#$00 here
//--------------------------------------

ChkLastBlock:
			dec WantedCtr

			cmp LastT
			bne CheckATN
			cpx LastS
			bne CheckATN

			lda (ZP0101),y		//Save new block count for later use
			sta NBC
			lda #$7f			//And delete it from the block
NOP1:		sta (ZP0101),y		//So that it does not confuse the depacker...

			lsr Random			//Check if this is also the first block of a randomly accessed bundle
			bcc ChkScnd

NOP2:		sta (ZP0100),y		//This is the first block of a random bundle, delete first byte
			lda BPtr			//Last byte of bundle will be pointer to first byte of new Bundle
NOP3:		sta (ZP01ff),y

ChkScnd:	lda WantedCtr
			bne StoreLoop

//--------------------------------------
//		Wait for C64
//--------------------------------------

CheckATN:	lda $1c00			//Fetch Motor and LED status
			ora #$0c			//Make sure LED will be on when we restart
			tax					//This needs to be done here, so that we do not affect Z flag at Restart

Frames:		ldy #$64			//100 frames (2 seconds) delay before turning motor off (#$fa for 5 sec)
DelayOut:	lda #$4f			//Approx. 1 frame delay (20000 cycles = $4e20 -> $4e+$01=$4f)
			sta $1c05			//Start timer, wait 2 seconds before turning motor off
DelayIn:	lda $1c05
			bne ChkLines
			dey
			bne DelayOut
			lda #$73			//Timer finished, turn motor off
			sta ErrCtr			//Reset Error Counter for motor spinup
			sax $1c00
			lda #<CSV			//Reset Checksum Verification Counter
			sta VerifCtr		//When motor restarts, first we verify proper read
			
ChkLines:	lda $1800
			bpl Reset			//ATN released - C64 got reset, reset the drive too
			alr #$05			//if C=1 then no change, A=#$00/#$02 after this
			bcs DelayIn
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

			asl
			bcs NewDiskID		//A=#$80-#$fe, Y=#$00 - flip disk
			beq CheckDir		//A=#$00, skip Random flag (first bundle on disk)
			inc Random
CheckDir:	ldx #$11			//A=#$00-#$7f, X=#$11 (dir sector 17) - DO NOT CHANGE TO INX, IT IS ALSO A JUMP TARGET!!!
			asl
			sta DirLoop+1		//Relative address within Dir block
			bcc CompareDir
			inx					//A=#$40-#$7f, X=#$12 (dir sector 18)
CompareDir:	cpx DirSector		//Dir Sector, initial value=#$c5
			beq DirFetchReturn	//Is the needed Dir block fetched?

			stx DirSector		//No, store new Dir block index and fetch directory sector
			stx LastS			//Also store it in LastS to be fetched
			jmp FetchDir		//ALWAYS, fetch directory, Y=#$00 here (needed)

DirFetchReturn:
			ldx #$03
DirLoop:	lda $0700,x
			sta LastT,x
			dex
			bpl DirLoop
			ldx #$c0
			sax Plugin			//A&X=#$40 if file is Saver or Custom Drive Code plugin, #$80 for transfer test, #$00 otherwise
			and #$3f
			sta LastT			//Delete upper 2 bits

			jsr ClearList		//Clear Wanted List, Y=00 here

			inc ReturnFlag

			tax					//X=A=LastT
			jsr BitRate			//Update Build loop, Y=MaxSct after this
								//Also find interleave and sector count for requested track

			ldx LastS			//This is actually the first sector on the track after track change (i.e. nS), always < MaxNumSct
			tya					//A=MaxSct
			//sec				//Not needed, C=1 after LSR ReturnFlag
			sbc SCtr			//Remaining sectors on track
			tay					//Y=already fetched sectors on track
			beq SkipUsed		//Y=0, we start with the first sector, skip marking used sectors (not essential)
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

ToFetchBAM:	jmp FetchBAM		//Go to Track 18 to fetch Sector 0 (BAM) for Next Side Info, Y=#$00

//--------------------------------------
//
//		Sequential Loading
//
//--------------------------------------

SeqLoad:	tay					//A=#$00 here -> Y=#$00
			lda BlockCtr		//End of Disk? BlockCtr never reaches zero here, only after trasfer, so if it is zero here then we have reached EoD
			beq ToFetchBAM		//If Yes, fetch BAM, otherwise start transfer

			lda SCtr			//Skip track change if either of these is true: (1) SCtr > 0 OR
			ora ScndBuff		//(2) SCtr = 0 but we have the last block of a bundle in the second buffer
			bne ToStartTr

			lda NBC				//Very last sector of the disk?
			beq ToStartTr		//Yes, skip track change, finish transfer

//--------------------------------------
//		Prepare track change
//--------------------------------------
								//Otherwise, clear wanted list and start seeking
			jmp JmpClrList		//C=0, Y=#$00 here, returns to NextTrack

//--------------------------------------

ToStartTr:	jmp StartTr

//--------------------------------------

NextTrack:	ldy #$81			//Prepare Y for half track step
NextTrkSvr:	ldx cT				//All blocks fetched in this track, so let's change track

			inx					//Go to next track

			cpx #$12			//next track = Track 18?, if yes, we need to skip it
			bne StepTmr			//half track step, (could skip setting timer here but we need it for the saver plugin)

			inx					//Skip track 18

			inc nS				//Skipping Dir Track will rotate disk a little bit more than a sector...
			inc nS				//...(12800 cycles to skip a track, 10526 cycles/sector on track 18)...
								//...so start sector of track 19 is increased by 2
			iny
			iny					//1.5-track seek, set timer at start

//--------------------------------------
//		Stepper Code			//X=Wanted Track
//--------------------------------------

StepTmr:	lda #$98
			sta $1c05

Seek:		lda $1c00
PreCalc:	and #$1b			//we keep AND+CLC instead of ANC because the original 1541U no longer receives firmware updates :(
			clc
			adc StepDir			//#$03 for stepping down, #$01 for stepping up
			ora #$04			//motor ON
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

RateDone:	sty MaxNumSct2+1

			ldx ILTab-$11,y		//Inverted Custom Interleave Tab
			stx IL+1

			ldx #$01			//Extra subtraction for Zone 3
			stx StepDir			//Reset stepper direction to Up/Inward here
			cpy #$15
			beq *+3
			dex
			stx SubSct+1

			lsr ReturnFlag
			bcc StoreBR
Return:		rts					//A=bitrate from both ReturnFlag checks

StoreBR:	sta Spartan+1		//Store bitrate for Spartan step

//--------------------------------------
//		GCR loop patch
//--------------------------------------

			ldx #<GCRLoop3-(GCREntry+2)
			cpy #$15						//Y=sector count (17, 18, 19, 21 for zones 0, 1, 2, 3, respectively)
			beq *+4
			ldx #<GCRLoop0_2-(GCREntry+2)	//GCR Loop patch for zones 0-2
			stx.z GCREntry+1

			ldx Patch1-17,y
			stx.z LoopMod2
			ldx Patch2-17,y
			stx.z LoopMod2+1

			lsr ReturnFlag
			bcs Return

			sty SCtr						//Reset Sector Counter, but only if this is not a random block which gets SCtr from Dir

//--------------------------------------

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
ByteCt:		cpy #$101-Sp		//04 05			Sending #$52 bytes before Spartan Stepping (#$59 for $1a bycles) CPY #$AF/#$01
			bit $1800			//06-09			#$52 x 72 = #$17 bycles*, #$31 bycles left for 2nd halftrack step and settling
			bpl *-3				//10 11			*actual time is #$18-$1a+ bycles, and can be (much) longer
W4:			sta $1800			//12-15			if C64 is not immediately ready to receive fetched block
								//(16 cycles)	thus, the drive may actually stop on halftracks before transfer continues
		
			bcs Loop			//00-02

//--------------------------------------
//	SPARTAN STEPPING (TM)				<< -	Uninterrupted data transfer across adjacent tracks - >>
//--------------------------------------		Transfer starts 1-2 bycles after first halftrack step initiated

Spartan:	lda #$00			//02 03			Last halftrack step is taken during data transfer
			sta $1c00			//04-07			Update bitrate and stepper with precalculated value
			tya					//08 09			Y=#$AE or #$00 here
			eor #$101-Sp		//10 11			#$31 bycles left for the head to take last halftrack step...
			sta ByteCt+1		//12-15			... and settle before new data is fetched
ChkPt:		bpl Loop			//16-18

.print ""
.print "Loop:  $0" + toHexString(Loop)
.print "ChkPt: $0" + toHexString(ChkPt)

.if ([>Loop] != [>ChkPt]) {
.error "ERROR!!! Transfer loop crosses pages!!!"
}

//--------------------------------------

TrSeqRet:	lda #busy			//16,17 		A=#$10
			bit $1800			//18,19,20,21 	Last bitpair received by C64?
			bmi *-3				//22,23
			sta $1800			//24,25,26,27	Transfer finished, send Busy Signal to C64
			
			bit $1800			//Make sure C64 pulls ATN before continuing
			bpl *-3				//Without this the next ATN check may fall through
								//resulting in early reset of the drive

			iny					//Y=#$01
			sty Loop+2			//Restore transfer loop
			
			jsr ToggleLED		//Transfer complete - turn LED off, leave motor on

//--------------------------------------
//		Update Block Counter
//--------------------------------------

			dec BlockCtr		//Decrease Block Counter
			bne ChkWCtr

UpdateBCtr:	lda NBC				//New Block Count
			sta BlockCtr
			bne ChkWCtr			//A = Block Count

JmpCATN:	jmp CheckATN		//No more blocks to fetch in sequence, wait for next loader call
								//If next loader call is sequential -> will go to BAM for flip check/reset
								//If next loader call is random -> will load requested file

//--------------------------------------

ChkWCtr:	lda WantedCtr		//If we just updated BlockCtr then WantedCtr will be 0
			bne JmpFetch2		//If there are more blocks on the list then fetch next one

			lsr ScndBuff		//No more blocks on wanted list, check if the last block has been stored...
			bcc CheckBCtr		//If we do not have the last block stored then check Bundle counter
								//Last block of Bundle stored, so transfer it
			inc Loop+2			//Modify transfer loop to transfer data from secondary buffer ($0100 -> $0200)
			bne JmpCATN

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
//		Initialization
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
			dec $f9				//Buffer pointer, it was #$04 before DEC
!:			jsr $d586			//Load 4 blocks to buffers 03,02,01,00
			and #$fe
			bne !-				//Error? -> try again
			dec $f9
			bpl !-

			sei

//--------------------------------------
//		Copy ZP code and tabs
//--------------------------------------

			ldx #$00
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
			sta $1800			//0  0  0  1  0  0  0  0  CO=0, DO=0, AA=1
								//AI|DN|DN|AA|CO|CI|DO|DI Signal C64 that the drive code has been installed

			lda #$01			//Shift register disabled, Port A ($1c01) latching enabled, Port B ($1c00) latching disabled
			sta $1c0b

			lda #$00			//Clear VIA #2 Timer 1 low byte
			sta $1c04

			lda #$7f			//Disable interrupts
			sta $180e
			sta $1c0e
			lda $180d			//Acknowledge pending interrupts
			lda $1c0d

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

.text "SPARKLE 3.2 BY OMG"

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
.byte	<BAM_ProdID-1,>BAM_ProdID-1
.byte			$2e,$1e,$ae,$1f,$be,$17,$00,CSV,$6e,$1a,$ee,$1b,$fe,$13	//1x
.byte	$01,$00,$14,$00,$8e,$1d,$9e,$15,$01,$00,$5e,$10,$ce,$19,$de,$11	//2x
.byte	$7f,$76,$3e,$16,$0e,$1c,$1e,$14,$76,$7f,$7e,$12,$4e,$18,$00,$00	//3x	Wanted List $3e-$52 (Sector 16 = #$ff)
.byte	$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$ff,$00	//4x	(0) unfetched, (+) fetched, (-) wanted
.byte	$00,$00,$00,$0e,$80,$0f,$c5,$07,$ff,$01,$01,$0a,$1e,$0b,$00,$03	//5x	
.byte	$fd,$fd,$fd,$00,$fc,$0d,$00,$05,$ff,$ff,XX1,$00,$02,$09,$00,$01	//6x	$60-$64 - ILTab, $63 - NextID, $68 - ZPHdrID1, $69 - ZPHdrID2
.byte	$1c,<ILTab-1,>ILTab-1
.byte				$06,$02,$0c,$00,$04,<ProductID-1,>ProductID-1
.byte											$ff,$02					//7x
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
								//						Zone 3	Zone 2	Zone 1	Zone 0
GCRLoop0_2:	eor ZP0102: $0102,x	//First pass: X=#$00	--		34		34		34		85-87
			eor ZP0103: $0103,x	//						--		38		38		38		88-8a

//------------------------------------------------------------------------------------------------------

								//						Zone 3	Zone 2	Zone 1	Zone 0
GCRLoop3:	sta.z PartialCSum+1	//						33		41		41		41		8b 8c

								//					   [26-51	28-55	30-59	32-63]
			lda $1c01			//DDDDEEEE				37/-14	45/-10	45/-14	45/+13	8d-8f
								//						211-413	187-366	200-420	213-420
			ldx #$0f			//						39		47		47		47		90 91
			sax.z tE+1			//tE+1=0000EEEE			42		50		50		50		92 93
			arr #$f0			//						44		52		52		52		94 95
			tay					//Y=DDDDD000			46		54		54		54		96

tC:			lda TabC			//00CCCCC0 (ZP)			49		57		57		57		97 98
tD:			eor TabD,y			//00000001,DDDDD000		53		61		61		61		99-9b
			pha					//$0104/$0100			56		64		64		64		9c
								//$0104 = Checksum
PartialCSum:
			eor #$7f			//						58		66		66		66		9d 9e
CSum:		eor #$00			//						60		68		68		68		9f a0
			sta.z CSum+1		//						63		71		71		71		a1 a2
								//				       [52-77	56-83	60-89	64-95]
			lda $1c01			//EFFFFFGG				67/-10	75/-8	75/-14	75/+11	a3-a5
								//						233-344	224-332	240-356	256-380
			ldx #$03			//						69		77		77		77		a6 a7
			sax.z tG+1			//tG+1=000000GG			72		80		80		80		a8 a9
LoopMod2:	alr #$fc			//					C=0	74		82		--		--		aa ab
			tax					//X=0EFFFFF0			76		84		94		102		ac

tE:			lda TabE			//0000EEEE (ZP)			79		87		97		105		ad ae
tF:			adc TabF,x			//00000001,0EFFFFF0 (ZP)83		91		101		109		af b0
			pha					//$0103/$01ff			86		94		104		112		b1
								//$0103 = Sector
								//				       [78-103	84-111	90-119	96-127]
			lax $1c01			//GGGHHHHH				90/-13	98/-13	108/-11	116/-11	b2-b4
								//						260-343	257-339	250-330	249-328
			alr #$e0			//A=0GGG0000			92		100		110		118		b5 b6
			tay					//Y=0GGG0000			94		102		112		120		b7
			lda #$1f			//						96		104		114		122		b8 b9
			axs #$00			//X=000HHHHH		C=1	98		106		116		124		ba bb

tG:			lda TabG,y			//000000GG,0GGG0000		102		110		120		128		bc-be
tH:			eor TabH,x			//10001011,000HHHHH		106		114		124		132		bf-c1
			pha					//$0102/$01fe			109		117		127		135		c2
								//$0102 = Track
			lax ZP07			//						112		120		130		138		c3 c4
								//					   [104-129	112-139	120-149	128-159]
			sbc $1c01			//AAAAABBB			V=0	116/-11	124/+12	134/+14	142/+14	c5-c7
								//						269-333	271-336	269-333	271-335
			sax.z tB+1			//tB+1=-00000BBB		119		127		137		145		c8 c9
			
			alr #$f8			//						121		129		139		147		ca cb
			tay					//Y=-0AAAAA00			123		131		141		149		cc

								//Total length (cycles):124		132		142		150
								//Max. RPM:				314.5	318.1	316.9	320.0
								
//------------------------------------------------------------------------------------------------------
								//						Zone 3	Zone 2	Zone 1	Zone 0
			bvc *				//						00-01							cd ce
								//					   [00-25	00-27	00-29	00-31]
			lda $1c01			//BBCCCCCD				05/-18	07/-20	07/-22	07/-24	cf-d1
			ldx #$3e			//						07								d2 d3
			sax.z tC+1			//tC+1=00CCCCC0			10								d4 d5
			alr #$c1			//						12								d6 d7
			tax					//X=0BB00000		C=D	14								d8

tA:			lda TabA,y			//00010010,-0AAAAA00	18								d9-da
tB:			eor TabB,x			//-00000BBB,0BB00000	22								db-de
			pha					//$0101/$01fd			25								df
								//$0101 = ID2
			tsx					//SP = $00/$fc ...		27								e0
GCREntry:	bne GCRLoop0_2		//We start on Track 18	30/29							e1 e2

//------------------------------------------------------------------------------------------------------

								//						Zone 3	Zone 2	Zone 1	Zone 0
			eor (ZP0102,x)		//						35								e3 e4
			tax					//						37								e5
								//					   [26-51	28-55	30-59	32-63]
			lda $1c01			//Final read = DDDD0101	41/-10	41/+13	41/+11	41/+9	e6-e8
			clv					//						43								e9
			ror					//A=DDDDD010|C=1		45								ea
			bvc *				//						00-01							eb ec
			tay					//						03								ed
			txa					//						05								ee
			eor TabD-2,y		//Checksum (D)/ID1 (H)	10								ef-f1
			ldx tC+1			//X=00CCCCC0			13								f2 f3
			eor TabC,x			//(ZP)					17								f4 f5
			eor $0103			//						21								f6-f8
			eor CSum+1			//						24								f9 fa
			sbc #$00			//Trailing 0 check		26								fb fc
ModJmp:		jmp (HeaderJmp)		//A = final checksum	31								fd-ff

//------------------------------------------------------------------------------------------------------
}
