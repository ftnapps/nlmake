OSTYPE=LINUX
SRC_DIR=./
OBJ=.o
EXE=
GPP=g++
CFLAGS=-g -Wall -pedantic
LFLAGS=


CDEFS= -DINCS_NEED_DOT_H -DNOEXCEPTIONS -D$(OSTYPE) $(ADDCDEFS)

.SUFFIXES:
.SUFFIXES: .c .o

%$(OBJ): $(SRC_DIR)%.c
	$(GPP) -c $(CFLAGS) $(CDEFS) $(SRC_DIR)$*.c

OBJFILES = \
 compress$(OBJ) \
 crc2$(OBJ) \
 diff$(OBJ) \
 doslinux$(OBJ) \
 file$(OBJ) \
 filefind$(OBJ) \
 flags$(OBJ) \
 jdate$(OBJ) \
 log$(OBJ) \
 nlmake$(OBJ) \
 netmail$(OBJ) \
 segment$(OBJ) \
 stats$(OBJ)

default: nlmake$(EXE)

all: nlmake$(EXE)

nlmake$(EXE): $(OBJFILES)
	$(GPP) $(LFLAGS) -o nlmake$(EXE) $(OBJFILES)

install:
	$(INSTALL) $(IBOPT) nlmake$(EXE) $(BINDIR)

uninstall:
	-$(RM) $(RMOPT) $(BINDIR)$(DIRSEP)nlmake$(EXE)

clean:
	-$(RM) $(RMOPT) *$(OBJ)
	-$(RM) $(RMOPT) core
	-$(RM) $(RMOPT) *~

distclean: clean
	-$(RM) $(RMOPT) nlmake$(EXE)

