#define macros
DIR_TOOLS = Tools
DIR_KICKASS = $(DIR_TOOLS)\KickAss
KICKASS = Java -jar $(DIR_KICKASS)\KickAss.jar
DIR_B2A = $(DIR_TOOLS)\Bin2Array
B2A = $(DIR_B2A)\Bin2Array.exe
DIR_ZIP = $(DIR_TOOLS)\Zip
ZIP = $(DIR_ZIP)\zip.exe -j -X
STRIPZIP = $(DIR_ZIP)\stripzip.exe
DIR_OBJ = Obj\Release
#DIR_GPP = $(DIR_TOOLS)\GPP
#GPP: $(DIR_GPP)\g++.exe

Sparkle.exe: Obj\Release\Ascii2DirArt.o Obj\Release\Loader.o Obj\Release\Packer.o Obj\Release\Petscii2DirArt.o Obj\Release\SD.o Obj\Release\SF.o Obj\Release\Sparkle.o Obj\Release\SS.o Obj\Release\SSIO.o Obj\Release\Sparkle.res
	g++.exe -s -std=c++17 -lstdc++fs -o bin\Release\Sparkle2.exe obj\Release\Ascii2DirArt.o obj\Release\Loader.o obj\Release\Packer.o obj\Release\Petscii2DirArt.o obj\Release\SD.o obj\Release\SF.o obj\Release\Sparkle.o obj\Release\SS.o obj\Release\SSIO.o  obj\Release\Sparkle.res

#$(GPP) -o Sparkle.exe Obj\Release\Ascii2DirArt.o Obj\Release\Loader.o Obj\Release\Packer.o Obj\Release\Petscii2DirArt.o Obj\Release\SD.o Obj\Release\SF.o Obj\Release\Sparkle.o Obj\Release\SS.o Obj\Release\SSIO.o Obj\Release\Sparkle.res

#Obj\Release\Ascii2DirArt.o: Ascii2DirArt.cpp Common.h
#	g++.exe -O3 -Wall -std=c++17 -fexceptions -c C:\Users\Tamas\OneDrive\C64\C++\Sparkle\Ascii2DirArt.cpp -o obj\Release\Ascii2DirArt.o

#$(GPP) Ascii2DirArt.cpp -o Obj\Release\Ascii2DirArt.o

#Obj\Release\Loader.o: Loader.cpp Common.h
#	$(GPP) Loader.cpp -o Obj\Release\Loader.o

#Obj\Release\Packer.o: Packer.cpp Common.h
#	$(GPP) Packer.cpp -o Obj\Release\Packer.o

#Obj\Release\Petscii2DirArt.o: Petscii2DirArt.cpp Common.h
#	$(GPP) Petscii2DirArt.cpp -o Obj\Release\Petscii2DirArt.o

#Obj\Release\SD.o: SD.cpp Common.h
#	$(GPP) SD.cpp -o Obj\Release\SD.o

#Obj\Release\SF.o: SF.cpp Common.h
#	$(GPP) SF.cpp -o Obj\Release\SF.o

#Obj\Release\Sparkle.o: Sparkle.cpp Common.h
#	$(GPP) Sparkle.cpp -o Obj\Release\Sparkle.o

#Obj\Release\SS.o: SS.cpp Common.h
#	$(GPP) SS.cpp -o Obj\Release\SS.o

#Obj\Release\SSIO.o: SSIO.cpp Common.h
#	$(GPP) SSIO.cpp -o Obj\Release\SSIO.o
