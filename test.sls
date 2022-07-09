[Sparkle Loader Script]

Path:	..\..\D64\propaganda31.d64
Header:	propaganda
ID:		No.31
Name:	-= propaganda =-
#Tracks:	40
Start:	fe80
IL0:	4
IL1:	3
IL2:	3
IL3:	3
HSFile:	..\..\Build\Issue31\HSFile.bin															0f00	0000	0100
ProdID:	10ad31FF
DirArt:	..\DirArt\31DirArt.d64

#00
File:	..\..\Build\Issue31\Intro\Intro.prg			
#Comment
File:	..\..\Issue31\Intro\Propa31-intro.kla													$0c00	0x232a	&H03e8	#Comment	#COMMENT
File:	..\..\Issue31\Intro\Xiny6581-WigglyButter.sid
File:	..\..\Issue31\Intro\Propa31-intro.bin													2200	1a00	1e00
File:	..\..\Issue31\Intro\Propa31-intro.kla*													c000	0002	1f40
File:	..\..\Issue31\Intro\Propa31-intro.kla													e000	1f42	03e8
File:	..\..\Issue31\Intro\Propa31-intro.bin													e400	5c00	0580
File:	..\..\Issue31\Intro\Propa31-intro.bin													ea00	6200				#comment

#01
File:	..\..\Build\MAIN\Entry.prg																0400	0002
File:	..\..\Build\MAIN\FontWidths.bin															0500
File:	..\..\Issue31\Music\SIDVolumeAddresses.bin												05c0
File:	..\..\Build\MAIN\HeadlineNumbers.map													0600
File:	..\..\Build\MAIN\LoadingSine.bin														0680
File:	..\..\Build\MAIN\Font.bin																0700
File:	..\..\Build\MAIN\FontHeights.bin														0d00	0000	00c0
File:	..\..\Build\MAIN\FontHeights.bin														0e00	00c0	00c0
File:	..\..\Build\MAIN\Magazine.prg															2800	0002
File:	..\..\Build\Issue31\ChapterIndices.bin													4f00
File:	..\..\Build\MAIN\FontHeights.bin														a800	0180	00c0
File:	..\..\Build\MAIN\FontHeights.bin														a900	0240	00c0
File:	..\..\Build\Issue31\RLEMenus.map														ae00
File:	..\..\Build\MAIN\FontShifts.bin															bc00	0380	0300
File:	..\..\Build\MAIN\FontShifts.bin															bf00	0700

#02
File:	..\..\Build\Issue31\BundleCount.bin														5000
File:	..\..\Build\MAIN\REU.prg																5001	0002

'-------------------------------------------------------------------------------------------------------------------
'MUSIC FILES: #03 - #08
'-------------------------------------------------------------------------------------------------------------------

#03
File:	..\Music\Vincenzo-DowntownPorn.sid

#04
File:	..\Music\Proton-MistyMorning.sid

#05
File:	..\Music\Wiklund-Collapse.sid

#06
File:	..\Music\Jammer-OutOfMyBox.sid

#07
File:	..\Music\Drax-Propane.sid

#08
File:	..\Music\NecroPolo-Propaganda.sid

'-------------------------------------------------------------------------------------------------------------------
'ARTICLE FILES: #09 -
'-------------------------------------------------------------------------------------------------------------------

Script:	..\..\Build\Issue31\Articles.sls