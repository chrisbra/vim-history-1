# Project: gvimext
# Generates gvimext.dll with Cygwin's -mno-cygwin
#
CPP  := g++ -mno-cygwin
CC   := gcc -mno-cygwin
CXXFLAGS := -O2
LIBS :=  -luuid
WINDRES := windres
DLLWRAP = dllwrap.exe
RES  := gvimext.res
DEFFILE = gvimext_ming.def
STATICLIB = gvimext.a
EXPLIB = gvimext.exp
OBJ  := gvimext.o $(RES)

INCS :=
DLL  := gvimext.dll

.PHONY: all all-before all-after clean clean-custom

all: all-before $(DLL) all-after


clean: clean-custom
	${RM}  $(OBJ) $(DLL) ${RES} ${EXPLIB} $(STATICLIB)


$(DLL): $(OBJ)
	$(DLLWRAP) \
		--def $(DEFFILE) \
		--output-exp ${EXPLIB} \
		--image-base 0x1C000000 \
		--driver-name c++ \
		--implib $(STATICLIB) \
		$(OBJ) $(LIBS) \
		--target=i386-mingw32 -mno-cygwin \
		-o $(DLL) -s

gvimext.o: gvimext.cpp
	$(CPP) -c $? -o $@ $(CXXFLAGS) -DFEAT_GETTEXT

${RES}: gvimext_ming.rc
	$(WINDRES) $? -I rc -o $@ -O coff  -DMING
