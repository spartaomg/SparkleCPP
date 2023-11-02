
#---------------------------------------------------
# This makefile requires GNU make, mkdir, and g++.
# On Windows please use MinGW.
#---------------------------------------------------

ifeq ($(OS),Windows_NT)
	DIR_BIN = bin/windows
	DIR_OBJ = obj/windows
	EXEC = $(DIR_BIN)/Sparkle.exe
	LINKFLAGS = -static -static-libgcc -static-libstdc++ -s -std=c++17 -o
else
	EXEC = $(DIR_BIN)/sparkle

	UNAME = $(shell uname)
    ifeq ($(UNAME),Linux)
		DIR_BIN = bin/linux
		DIR_OBJ = obj/linux
		LINKFLAGS = -static -static-libgcc -static-libstdc++ -s -std=c++17 -o
    endif
    ifeq ($(UNAME),Darwin)
		DIR_BIN = bin/macos
		DIR_OBJ = obj/macos
		LINKFLAGS = -std=c++17 -o
    endif
endif

CC = g++
CFLAGS = -O3 -Wall -std=c++17 -fexceptions -c
RESFLAGS = -J rc -O coff -i

OBJ = $(DIR_OBJ)/ascii2dirart.o $(DIR_OBJ)/charsettab.o $(DIR_OBJ)/loader.o $(DIR_OBJ)/packer.o $(DIR_OBJ)/petscii2dirart.o $(DIR_OBJ)/pixelcnttab.o
OBJ += $(DIR_OBJ)/sd.o $(DIR_OBJ)/sf.o $(DIR_OBJ)/sparkle.o $(DIR_OBJ)/ss.o $(DIR_OBJ)/ssio.o $(DIR_OBJ)/thirdparty/lodepng.o  $(DIR_OBJ)/thirdparty/tinyexpr.o

# "mkdir" (with quotes) will force Windows to search for the executable (GNU) mkdir instead of the CMD command. Just make sure that its folder is in the system PATH.
# It also works on Linux and macOS.

create_dir = @"mkdir" -p $(@D)

ifeq ($(OS),Windows_NT)
	OBJ += $(DIR_OBJ)/sparkle.res
endif

$(EXEC): $(OBJ)
	$(create_dir)
	$(CC) $(LINKFLAGS) $(EXEC) $(OBJ)

$(DIR_OBJ)/%.o: %.cpp common.h
	$(create_dir)
	$(CC) $(CFLAGS) $< -o $@

$(DIR_OBJ)/thirdparty/%.o: thirdparty/%.cpp thirdparty/%.h
	$(create_dir)
	$(CC) $(CFLAGS) $< -o $@

$(DIR_OBJ)/sparkle.res: sparkle.rc resource.h
	$(create_dir)
	windres $(RESFLAGS) sparkle.rc -o $(DIR_OBJ)/sparkle.res
