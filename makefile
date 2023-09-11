#define macros
DIR_OBJ = Obj/Release
DIR_BIN = Bin/Release
OBJFLAGS = -O3 -Wall -std=c++17 -fexceptions -c
LINKFLAGS = -s -std=c++17 -o
RESFLAGS = -J rc -O coff -i
create_dir = @mkdir -p $(@D)

$(DIR_BIN)/Sparkle2.exe: $(DIR_OBJ)/Ascii2DirArt.o $(DIR_OBJ)/Loader.o $(DIR_OBJ)/Packer.o $(DIR_OBJ)/Petscii2DirArt.o $(DIR_OBJ)/SD.o $(DIR_OBJ)/SF.o $(DIR_OBJ)/Sparkle.o $(DIR_OBJ)/SS.o $(DIR_OBJ)/SSIO.o $(DIR_OBJ)/ThirdParty/TinyExpr.o $(DIR_OBJ)/Sparkle.res
	$(create_dir)
	g++ $(LINKFLAGS) $(DIR_BIN)/Sparkle2.exe $(DIR_OBJ)/Ascii2DirArt.o $(DIR_OBJ)/Loader.o $(DIR_OBJ)/Packer.o $(DIR_OBJ)/Petscii2DirArt.o $(DIR_OBJ)/SD.o $(DIR_OBJ)/SF.o $(DIR_OBJ)/Sparkle.o $(DIR_OBJ)/SS.o $(DIR_OBJ)/SSIO.o $(DIR_OBJ)/ThirdParty/TinyExpr.o $(DIR_OBJ)/Sparkle.res

$(DIR_OBJ)/Ascii2DirArt.o: Ascii2DirArt.cpp Common.h
	$(create_dir)
	g++ $(OBJFLAGS) Ascii2DirArt.cpp -o $(DIR_OBJ)/Ascii2DirArt.o

$(DIR_OBJ)/Loader.o: Loader.cpp Common.h
	$(create_dir)
	g++ $(OBJFLAGS) Loader.cpp -o $(DIR_OBJ)/Loader.o

$(DIR_OBJ)/Packer.o: Packer.cpp Common.h
	$(create_dir)
	g++ $(OBJFLAGS) Packer.cpp -o $(DIR_OBJ)/Packer.o

$(DIR_OBJ)/Petscii2DirArt.o: Petscii2DirArt.cpp Common.h
	$(create_dir)
	g++ $(OBJFLAGS) Petscii2DirArt.cpp -o $(DIR_OBJ)/Petscii2DirArt.o

$(DIR_OBJ)/SD.o: SD.cpp Common.h
	$(create_dir)
	g++ $(OBJFLAGS) SD.cpp -o $(DIR_OBJ)/SD.o

$(DIR_OBJ)/SF.o: SF.cpp Common.h
	$(create_dir)
	g++ $(OBJFLAGS) SF.cpp -o $(DIR_OBJ)/SF.o

$(DIR_OBJ)/Sparkle.o: Sparkle.cpp Common.h
	$(create_dir)
	g++ $(OBJFLAGS) Sparkle.cpp -o $(DIR_OBJ)/Sparkle.o

$(DIR_OBJ)/SS.o: SS.cpp Common.h
	$(create_dir)
	g++ $(OBJFLAGS) SS.cpp -o $(DIR_OBJ)/SS.o

$(DIR_OBJ)/SSIO.o: SSIO.cpp Common.h
	$(create_dir)
	g++ $(OBJFLAGS) SSIO.cpp -o $(DIR_OBJ)/SSIO.o

$(DIR_OBJ)/ThirdParty/TinyExpr.o: ThirdParty/TinyExpr.cpp ThirdParty/TinyExpr.h Common.h
	$(create_dir)
	g++ $(OBJFLAGS) ThirdParty/TinyExpr.cpp -o $(DIR_OBJ)/ThirdParty/TinyExpr.o

$(DIR_OBJ)/Sparkle.res: Sparkle.rc resource.h
	$(create_dir)
	windres $(RESFLAGS) Sparkle.rc -o $(DIR_OBJ)/Sparkle.res