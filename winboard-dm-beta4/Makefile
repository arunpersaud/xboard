OS=NT
ENV=WIN32
CPU=i386

!include <$(OS)$(ENV).MAK>

proj = winboard
allobj = winboard.obj backend.obj parser.obj moves.obj lists.obj \
	 gamelist.obj pgntags.obj wedittags.obj wgamelist.obj zippy.obj \
         wsockerr.obj wclipbrd.obj woptions.obj

#### old stuff - disable by Daniel Mehrmann
#cvars = $(cvars) -I. -DWINVER=0x0400
#cflags = $(cflags) /FR

# delete .mak file We don't need that !

################ Mehrmann stuff ###########################

### EDIT HERE for compile optimze or debug ;-)

# new stuff by Daniel Mehrmann

# need this for a nomal run
# DO NOT CHANGE !!
cflags = -c -W3 -DCRTAPI1=_cdecl -DCRTAPI2=_cdecl -nologo -D_X86_=1 -D_WINNT -D_WIN32_WINNT=0x0400 -D_WIN32_IE=0x0300 -DWINVER=0x0400 /FRwinboard.bsc -DWIN32   

# optimize here !
# standard fast und pentium 
cflags = $(cflags) -O2 -G5


#!!! disable debug !!!
# comment this out if you want debug !
cdebug =
linkdebug =


############### Mehrmann stuff end ##########################

all: $(proj).exe

# Update the help file if necessary
$(proj).hlp : $(proj).rtf
    $(hc) $(proj).hpj
    cat $(proj).err

# Update the resource if necessary
$(proj).rbj: $(proj).rc $(proj).h $(proj).res resource.h
    $(rc) $(rcvars) -r -fo $(proj).res $(cvars) $(proj).rc
    cvtres -$(CPU) $(proj).res -o $(proj).rbj

# Update the object files if necessary
winboard.obj: winboard.c config.h winboard.h common.h frontend.h backend.h \
	moves.h wgamelist.h defaults.h resource.h wclipbrd.h wedittags.h \
	wsockerr.h lists.h
    $(cc) $(cflags) $(cvars) $(cdebug) winboard.c

backend.obj: backend.c config.h common.h frontend.h backend.h parser.h \
	moves.h zippy.h backendz.h lists.h
    $(cc) $(cflags) $(cvars) $(cdebug) backend.c

parser.obj: parser.c config.h common.h backend.h parser.h frontend.h moves.h \
	lists.h
    $(cc) $(cflags) $(cvars) $(cdebug) parser.c

parser.c: parser.l
    flex -L parser.l
    del parser.c
    rename lex.yy.c parser.c

moves.obj: moves.c config.h backend.h common.h parser.h moves.h lists.h \
	frontend.h
    $(cc) $(cflags) $(cvars) $(cdebug) moves.c

lists.obj: lists.c config.h lists.h common.h
    $(cc) $(cflags) $(cvars) $(cdebug) lists.c

gamelist.obj: gamelist.c config.h lists.h common.h frontend.h backend.h \
	parser.h lists.h
    $(cc) $(cflags) $(cvars) $(cdebug) gamelist.c

pgntags.obj: pgntags.c config.h common.h frontend.h backend.h parser.h lists.h
    $(cc) $(cflags) $(cvars) $(cdebug) pgntags.c

wclipbrd.obj: wclipbrd.c config.h common.h frontend.h backend.h winboard.h \
	wclipbrd.h lists.h resource.h
    $(cc) $(cflags) $(cvars) $(cdebug) wclipbrd.c

wedittags.obj: wedittags.c config.h common.h winboard.h frontend.h backend.h \
	lists.h resource.h
    $(cc) $(cflags) $(cvars) $(cdebug) wedittags.c

wgamelist.obj: wgamelist.c config.h. common.h winboard.h frontend.h backend.h \
	wgamelist.h lists.h resource.h
    $(cc) $(cflags) $(cvars) $(cdebug) wgamelist.c

woptions.obj: woptions.c config.h common.h frontend.h backend.h lists.h
    $(cc) $(cflags) $(cvars) $(cdebug) woptions.c

wsockerr.obj: wsockerr.c wsockerr.h
    $(cc) $(cflags) $(cvars) $(cdebug) wsockerr.c

zippy.obj: zippy.c config.h common.h zippy.h frontend.h backend.h backendz.h \
	lists.h
    $(cc) $(cflags) $(cvars) $(cdebug) zippy.c

$(proj).exe: $(allobj) $(proj).rbj $(proj).def $(proj).hlp $(proj).rc
    $(link) $(linkdebug) $(guiflags) $(allobj) \
	wsock32.lib shell32.lib comctl32.lib winmm.lib libc.lib oldnames.lib kernel32.lib \
	advapi32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib \
	$(proj).rbj -out:$(proj).exe
	bscmake *.sbr

test.exe: test.c
	$(cc) $(cflags) $(cvars) $(cdebug) test.c
	$(link) $(linkdebug) $(conflags) test.obj $(conlibs) -out:test.exe

