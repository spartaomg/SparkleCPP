#define macros
DIR_TOOLS = tools
DIR_KICKASS = $(DIR_TOOLS)\kickass
KICKASS = Java -jar $(DIR_KICKASS)\kickass.jar
DIR_B2A = $(DIR_TOOLS)\bin2array
B2A = $(DIR_B2A)\bin2array.exe
DIR_ZIP = $(DIR_TOOLS)\zip
ZIP = $(DIR_ZIP)\zip.exe -j -X
STRIPZIP = $(DIR_ZIP)\stripzip.exe

cpp.zip: ascii2dirart.cpp petscii2dirart.cpp loader.cpp sd.cpp ss.cpp ssio.cpp sf.cpp sc.cpp #st.cpp
	$(ZIP) cpp.zip ascii2dirart.cpp petscii2dirart.cpp loader.cpp sd.cpp ss.cpp ssio.cpp sf.cpp sc.cpp #st.cpp
	$(STRIPZIP) cpp.zip


ascii2dirart.cpp: resources\ascii2dirart.bin
	$(B2A) resources\ascii2dirart.bin 0 0 ascii2dirart.cpp


petscii2dirart.cpp: resources\petscii2dirart.bin
	$(B2A) resources\petscii2dirart.bin 0 0 petscii2dirart.cpp


loader.cpp: resources\loader.prg
	$(B2A) resources\loader.prg 0 0 loader.cpp

resources\loader.prg: resources\sl.asm
	$(KICKASS) resources\sl.asm -o resources\loader.prg -afo


sd.cpp: resources\sd.prg
	$(B2A) resources\sd.prg 2 0 sd.cpp

resources\sd.prg: resources\sd.asm
	$(KICKASS) resources\sd.asm -o resources\sd.prg


sf.cpp: resources\sf.prg
	$(B2A) resources\sf.prg 2 0 sf.cpp

resources\sf.prg: resources\sf.asm resources\sd.sym
	$(KICKASS) resources\sf.asm -o resources\sf.prg


#st.cpp: resources\st.prg
#	$(B2A) resources\st.prg 2 0 st.cpp

#resources\st.prg: resources\st.asm resources\sd.sym
#	$(KICKASS) resources\st.asm -o resources\st.prg


sc.cpp: resources\sc.prg
	$(B2A) resources\sc.prg 2 0 sc.cpp

resources\ss.prg: resources\sc.asm resources\sd.sym resources\sl.sym
	$(KICKASS) resources\sc.asm -o resources\sc.prg


ss.cpp: resources\ss.prg
	$(B2A) resources\ss.prg 2 0 ss.cpp

resources\ss.prg: resources\ss.asm resources\sd.sym resources\sl.sym
	$(KICKASS) resources\ss.asm -o resources\ss.prg  :io=false


ssio.cpp: resources\ssio.prg
	$(B2A) resources\ssio.prg 2 0 ssio.cpp

resources\ssio.prg: resources\ss.asm  resources\sd.sym resources\sl.sym
	$(KICKASS) resources\ss.asm -o resources\ssio.prg  :io=true