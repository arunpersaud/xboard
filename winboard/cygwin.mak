#OS=WIN98
#ENV=WIN32
#CPU=i386
#!include <$(OS)$(ENV).MAK>

# Use up to date help compiler
#hc="c:\program files\help workshop\hcrtf.exe" -xn


# Uncomment both lines to turn on debugging symbols #######
cdebug= -g
linkdebug= -g
#######################################################

proj=winboard
allobj=  winboard.o backend.o parser.o moves.o lists.o \
	 gamelist.o pgntags.o wedittags.o wgamelist.o zippy.o \
         wsockerr.o wbres.o wclipbrd.o woptions.o


# 2 Dec 2001 - the mingw libraries that I have right now
#  (not updated for a few months) have bugs in fileno and stat
#  that prevent WinBoard from working.
#cygwin= -mno-cygwin

CFLAGS= $(cdebug)
CVARS= -I. -mwindows $(cygwin)
CC = gcc $(CVARS)
WCC = $(CC) -mwindows -Xlinker "-e" -Xlinker "_mainCRTStartup" \
  $(linkdebug) $(cygwin)
LD = ld

all: $(proj).exe

# Update the help file if necessary
#	$(proj).hlp : $(proj).rtf
#	$(hc) $(proj).hpj
#	cat $(proj).err

# Update the resource if necessary
wbres.o: $(proj).rc $(proj).h resource.h
	windres --use-temp-file $< -O coff -o $@

# Update the object files if necessary

parser.c: parser.l
	flex -L parser.l
	cp lex.yy.c parser.c

$(proj).exe: $(allobj) $(proj).hlp $(proj).rc
	$(WCC) $(guiflags) $(allobj) \
	-lwsock32 -lcomctl32 -lwinmm  -lkernel32 \
	-ladvapi32 -luser32 -lgdi32 -lcomdlg32 -lwinspool \
	-o $(proj).exe

clean:
	rm *.o parser.c
