#
# Makefile for WinBoard, using the GNU Cygwin toolset
#

# Uncomment both lines to turn on debugging symbols #######
cdebug= -g
linkdebug= -g
#######################################################

proj=winboard
allobj=  winboard.o backend.o parser.o moves.o lists.o \
	 gamelist.o pgntags.o wedittags.o wgamelist.o zippy.o \
         wsockerr.o wbres.o wclipbrd.o woptions.o

dotc=   winboard.c backend.c parser.c moves.c lists.c \
	 gamelist.c pgntags.c wedittags.c wgamelist.c zippy.c \
         wsockerr.c winboard.rc wclipbrd.c woptions.c

# Currently WinBoard will build either with or without -mno-cygwin
# however, a Cygwin bug in forming the command line to WinMain() is
# exposed if -mno-cygwin is not used.
cygwin= -mno-cygwin

VPATH= .:..
CFLAGS= $(cdebug)
INCLUDES= -I. -I..
CVARS= $(INCLUDES) $(cygwin)
CC = gcc $(CVARS)
WCC = $(CC) -mwindows $(linkdebug)
HC="/c/program files/help workshop/hcrtf.exe" -xn

all: $(proj).exe

depend: $(dotc)
	makedepend -Y -f cygwin.mak $(INCLUDES) $^

clean:
	rm -f *.obj *~ $(proj).exe $(proj).err $(proj).rbj \
		$(proj).RES $(proj).res $(proj).ini *.sbr *.bsc *.o \
		*.plg *.opt *.ncb *.debug *.bak *.gid *.GID

maintainer-clean: clean
	rm -f parser.c *.hlp *.HLP

# Update the help file if necessary
$(proj).hlp : $(proj).rtf
	$(HC) $(proj).hpj
	cat $(proj).err

# Update the resource if necessary
wbres.o: $(proj).rc $(proj).h resource.h
	windres --use-temp-file --include-dir .. $< -O coff -o $@

# Update the object files if necessary

parser.c: parser.l
	flex -oparser.c -L $<

$(proj).exe: $(allobj) $(proj).hlp $(proj).rc
	$(WCC) $(guiflags) $(allobj) \
	-lwsock32 -lwinmm \
	-o $(proj).exe

# DO NOT DELETE

winboard.o: config.h ../common.h winboard.h resource.h ../frontend.h
winboard.o: ../backend.h ../lists.h ../moves.h wclipbrd.h wgamelist.h
winboard.o: wedittags.h woptions.h wsockerr.h defaults.h
backend.o: config.h ../common.h ../frontend.h ../backend.h ../lists.h
backend.o: ../parser.h ../moves.h ../zippy.h ../backendz.h
parser.o: config.h ../common.h ../backend.h ../lists.h ../frontend.h
parser.o: ../parser.h ../moves.h
moves.o: config.h ../common.h ../backend.h ../lists.h ../frontend.h
moves.o: ../moves.h ../parser.h
lists.o: config.h ../common.h ../lists.h
gamelist.o: config.h ../common.h ../frontend.h ../backend.h ../lists.h
gamelist.o: ../parser.h
pgntags.o: config.h ../common.h ../frontend.h ../backend.h ../lists.h
pgntags.o: ../parser.h
wedittags.o: config.h ../common.h winboard.h resource.h ../frontend.h
wedittags.o: ../backend.h ../lists.h
wgamelist.o: config.h ../common.h winboard.h resource.h ../frontend.h
wgamelist.o: ../backend.h ../lists.h
zippy.o: config.h ../common.h ../zippy.h ../frontend.h ../backend.h
zippy.o: ../lists.h ../backendz.h
wsockerr.o: wsockerr.h
winboard.o: resource.h
wclipbrd.o: config.h ../common.h winboard.h resource.h ../frontend.h
wclipbrd.o: ../backend.h ../lists.h wclipbrd.h
woptions.o: config.h ../common.h winboard.h resource.h ../backend.h
woptions.o: ../lists.h ../frontend.h woptions.h defaults.h wedittags.h
