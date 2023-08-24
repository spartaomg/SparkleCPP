#define macros
DIR_TOOLS = Tools
DIR_KICKASS = $(DIR_TOOLS)\KickAss
KICKASS = Java -jar $(DIR_KICKASS)\KickAss.jar
DIR_B2A = $(DIR_TOOLS)\Bin2Array
B2A = $(DIR_B2A)\Bin2Array.exe
DIR_ZIP = $(DIR_TOOLS)\Zip
ZIP = $(DIR_ZIP)\zip.exe -j -X
STRIPZIP = $(DIR_ZIP)\stripzip.exe

cpp.zip: Ascii2DirArt.cpp Petscii2DirArt.cpp Loader.cpp SD.cpp SS.cpp SSIO.cpp SF.cpp
	$(ZIP) cpp.zip Ascii2DirArt.cpp Petscii2DirArt.cpp Loader.cpp SD.cpp SS.cpp SSIO.cpp SF.cpp
	$(STRIPZIP) cpp.zip


Ascii2DirArt.cpp: Resources\Ascii2DirArt.bin Resources\Petscii2DirArt.bin Resources\Loader.prg
	$(B2A) Resources\Ascii2DirArt.bin 0 0 Ascii2DirArt.cpp


Petscii2DirArt.cpp: Resources\Petscii2DirArt.bin
	$(B2A) Resources\Petscii2DirArt.bin 0 0 Petscii2DirArt.cpp


Loader.cpp: Resources\Loader.prg
	$(B2A) Resources\Loader.prg 0 0 Loader.cpp

Resources\Loader.prg: Resources\SL.asm
	$(KICKASS) Resources\SL.asm -o Resources\Loader.prg -afo


SD.cpp: Resources\SD.prg
	$(B2A) Resources\SD.prg 2 0 SD.cpp

Resources\SD.prg: Resources\SD.asm
	$(KICKASS) Resources\SD.asm -o Resources\SD.prg


SF.cpp: Resources\SF.prg
	$(B2A) Resources\SF.prg 2 0 SF.cpp

Resources\SF.prg: Resources\SF.asm Resources\SD.sym
	$(KICKASS) Resources\SF.asm -o Resources\SF.prg -afo


SS.cpp: Resources\SS.prg
	$(B2A) Resources\SS.prg 2 0 SS.cpp

Resources\SS.prg: Resources\SS.asm
	$(KICKASS) Resources\SS.asm -o Resources\SS.prg  :io=false


SSIO.cpp: Resources\SSIO.prg
	$(B2A) Resources\SSIO.prg 2 0 SSIO.cpp

Resources\SSIO.prg: Resources\SS.asm
	$(KICKASS) Resources\SS.asm -o Resources\SSIO.prg  :io=true