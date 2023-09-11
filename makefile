#define macros
DIR_OBJ = Obj/Release
DIR_BIN = bin/Release
GPPFLAGS = -O3 -Wall -std=c++17 -fexceptions -c
RESFLAGS = -J rc -O coff -i

Sparkle.exe: $(DIR_OBJ)/Ascii2DirArt.o $(DIR_OBJ)/Loader.o $(DIR_OBJ)/Packer.o $(DIR_OBJ)/Petscii2DirArt.o $(DIR_OBJ)/SD.o $(DIR_OBJ)/SF.o $(DIR_OBJ)/Sparkle.o $(DIR_OBJ)/SS.o $(DIR_OBJ)/SSIO.o $(DIR_OBJ)/ThirdParty/TinyExpr.o $(DIR_OBJ)/Sparkle.res
	g++ -s -std=c++17 -lstdc++fs -o $(DIR_BIN)/Sparkle2.exe $(DIR_OBJ)/Ascii2DirArt.o $(DIR_OBJ)/Loader.o $(DIR_OBJ)/Packer.o $(DIR_OBJ)/Petscii2DirArt.o $(DIR_OBJ)/SD.o $(DIR_OBJ)/SF.o $(DIR_OBJ)/Sparkle.o $(DIR_OBJ)/SS.o $(DIR_OBJ)/SSIO.o $(DIR_OBJ)/ThirdParty/TinyExpr.o $(DIR_OBJ)/Sparkle.res

$(DIR_OBJ)/Ascii2DirArt.o: Ascii2DirArt.cpp Common.h
	g++ $(GPPFLAGS) Ascii2DirArt.cpp -o $(DIR_OBJ)/Ascii2DirArt.o

$(DIR_OBJ)/Loader.o: Loader.cpp Common.h
	g++ $(GPPFLAGS) Loader.cpp -o $(DIR_OBJ)/Loader.o

$(DIR_OBJ)/Packer.o: Packer.cpp Common.h
	g++ $(GPPFLAGS) Packer.cpp -o $(DIR_OBJ)/Packer.o

$(DIR_OBJ)/Petscii2DirArt.o: Petscii2DirArt.cpp Common.h
	g++ $(GPPFLAGS) Petscii2DirArt.cpp -o $(DIR_OBJ)/Petscii2DirArt.o

$(DIR_OBJ)/SD.o: SD.cpp Common.h
	g++ $(GPPFLAGS) SD.cpp -o $(DIR_OBJ)/SD.o

$(DIR_OBJ)/SF.o: SF.cpp Common.h
	g++ $(GPPFLAGS) SF.cpp -o $(DIR_OBJ)/SF.o

$(DIR_OBJ)/Sparkle.o: Sparkle.cpp Common.h
	g++ $(GPPFLAGS) Sparkle.cpp -o $(DIR_OBJ)/Sparkle.o

$(DIR_OBJ)/SS.o: SS.cpp Common.h
	g++ $(GPPFLAGS) SS.cpp -o $(DIR_OBJ)/SS.o

$(DIR_OBJ)/SSIO.o: SSIO.cpp Common.h
	g++ $(GPPFLAGS) SSIO.cpp -o $(DIR_OBJ)/SSIO.o

$(DIR_OBJ)/ThirdParty/TinyExpr.o: ThirdParty/TinyExpr.cpp ThirdParty/TinyExpr.h Common.h
	g++ $(GPPFLAGS) ThirdParty/TinyExpr.cpp -o $(DIR_OBJ)/ThirdParty/TinyExpr.o

$(DIR_OBJ)/Sparkle.res: Sparkle.rc
	windres $(RESFLAGS) Sparkle.rc -o $(DIR_OBJ)/Sparkle.res