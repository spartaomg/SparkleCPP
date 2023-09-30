#define macros
DIR_OBJ = obj/release
DIR_BIN = bin/release
CFLAGS = -O3 -Wall -std=c++17 -fexceptions -c
RESFLAGS = -J rc -O coff -i
create_dir = @mkdir -p $(@D)
OBJ = $(DIR_OBJ)/ascii2dirart.o $(DIR_OBJ)/loader.o $(DIR_OBJ)/packer.o $(DIR_OBJ)/petscii2dirart.o $(DIR_OBJ)/sd.o $(DIR_OBJ)/sf.o $(DIR_OBJ)/sparkle.o
OBJ += $(DIR_OBJ)/charsettab.o $(DIR_OBJ)/pixelcnttab.o $(DIR_OBJ)/ss.o $(DIR_OBJ)/ssio.o $(DIR_OBJ)/thirdparty/lodepng.o  $(DIR_OBJ)/thirdparty/tinyexpr.o

ifeq ($(OS),Windows_NT)
	OBJ += $(DIR_OBJ)/sparkle.res
	EXEC = $(DIR_BIN)/Sparkle2.exe
	LINKFLAGS = -static -static-libgcc -static-libstdc++ -s -std=c++17 -o
else
	EXEC = $(DIR_BIN)/sparkle2

	UNAME = $(shell uname)
    ifeq ($(UNAME),Linux)
		LINKFLAGS = -static -static-libgcc -static-libstdc++ -s -std=c++17 -o
    endif
    ifeq ($(UNAME),Darwin)
		LINKFLAGS = -std=c++17 -o
    endif
endif

$(EXEC): $(OBJ)
	$(create_dir)
	g++ $(LINKFLAGS) $(EXEC) $(OBJ)

$(DIR_OBJ)/%.o: %.cpp common.h
	$(create_dir)
	g++ $(CFLAGS) $< -o $@

$(DIR_OBJ)/thirdparty/%.o: thirdparty/%.cpp thirdparty/%.h
	$(create_dir)
	g++ $(CFLAGS) $< -o $@

$(DIR_OBJ)/sparkle.res: sparkle.rc resource.h
	$(create_dir)
	windres $(RESFLAGS) sparkle.rc -o $(DIR_OBJ)/sparkle.res
