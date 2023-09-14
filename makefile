#define macros
DIR_OBJ = obj/release
DIR_BIN = bin/release
OBJFLAGS = -O3 -Wall -std=c++17 -fexceptions -c
LINKFLAGS = -s -std=c++17 -o
RESFLAGS = -J rc -O coff -i
create_dir = @mkdir -p $(@D)
OBJ = $(DIR_OBJ)/ascii2dirart.o $(DIR_OBJ)/loader.o $(DIR_OBJ)/packer.o $(DIR_OBJ)/petscii2dirart.o $(DIR_OBJ)/sd.o $(DIR_OBJ)/sf.o $(DIR_OBJ)/sparkle.o $(DIR_OBJ)/ss.o $(DIR_OBJ)/ssio.o $(DIR_OBJ)/thirdparty/tinyexpr.o

ifeq ($(OS),Windows_NT)
	OBJ += $(DIR_OBJ)/sparkle.res
	EXEC = $(DIR_BIN)/Sparkle2.exe
else
	EXEC = $(DIR_BIN)/sparkle2
endif

$(EXEC): $(OBJ)
	$(create_dir)
	g++ $(LINKFLAGS) $(EXEC) $(OBJ)

$(DIR_OBJ)/ascii2dirart.o: ascii2dirart.cpp common.h
	$(create_dir)
	g++ $(OBJFLAGS) ascii2dirart.cpp -o $(DIR_OBJ)/ascii2dirart.o

$(DIR_OBJ)/loader.o: loader.cpp common.h
	$(create_dir)
	g++ $(OBJFLAGS) loader.cpp -o $(DIR_OBJ)/loader.o

$(DIR_OBJ)/packer.o: packer.cpp common.h
	$(create_dir)
	g++ $(OBJFLAGS) packer.cpp -o $(DIR_OBJ)/packer.o

$(DIR_OBJ)/petscii2dirart.o: petscii2dirart.cpp common.h
	$(create_dir)
	g++ $(OBJFLAGS) petscii2dirart.cpp -o $(DIR_OBJ)/petscii2dirart.o

$(DIR_OBJ)/sd.o: sd.cpp common.h
	$(create_dir)
	g++ $(OBJFLAGS) sd.cpp -o $(DIR_OBJ)/sd.o

$(DIR_OBJ)/sf.o: sf.cpp common.h
	$(create_dir)
	g++ $(OBJFLAGS) sf.cpp -o $(DIR_OBJ)/sf.o

$(DIR_OBJ)/sparkle.o: sparkle.cpp common.h
	$(create_dir)
	g++ $(OBJFLAGS) sparkle.cpp -o $(DIR_OBJ)/sparkle.o

$(DIR_OBJ)/ss.o: ss.cpp common.h
	$(create_dir)
	g++ $(OBJFLAGS) ss.cpp -o $(DIR_OBJ)/ss.o

$(DIR_OBJ)/ssio.o: ssio.cpp common.h
	$(create_dir)
	g++ $(OBJFLAGS) ssio.cpp -o $(DIR_OBJ)/ssio.o

$(DIR_OBJ)/thirdparty/tinyexpr.o: thirdparty/tinyexpr.cpp thirdparty/tinyexpr.h common.h
	$(create_dir)
	g++ $(OBJFLAGS) thirdparty/tinyexpr.cpp -o $(DIR_OBJ)/thirdparty/tinyexpr.o

$(DIR_OBJ)/sparkle.res: sparkle.rc resource.h
	$(create_dir)
	windres $(RESFLAGS) sparkle.rc -o $(DIR_OBJ)/sparkle.res
