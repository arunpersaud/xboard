# Makefile for Borland C
# Contributed by Don Fong
# Modified for Winboard Plus by Mark Williams
ENV=WIN32
CPU=i386

cc=bcc32
rc=brc32 -w32 -Ic:/bc45/include;.
link=$(cc)

# Use up to date help compiler
hc="c:\program files\help workshop\hcrtf.exe" -xn

proj = winboard
allobj = winboard.obj backend.obj parser.obj moves.obj lists.obj \
         gamelist.obj pgntags.obj wedittags.obj wgamelist.obj zippy.obj \
         wsockerr.obj wclipbrd.obj woptions.obj
libs=wsock32.lib import32.lib
rm = c:\mksnt\rm -f

cvars = -DWINVER=0x0400

.c.obj:
        $(cc) -c $(cvars) $*.c



all: $(proj).exe

# Update the help file if necessary
$(proj).hlp : $(proj).rtf
        $(hc) $(proj).hpj
        cat $(proj).err

# Update the resource if necessary
$(proj).res: $(proj).rc
        $(rc) -r $(proj).rc

# Update the object files if necessary
winboard.obj: winboard.c config.h winboard.h common.h frontend.h backend.h \
        moves.h wgamelist.h defaults.h resource.h

backend.obj: backend.c config.h frontend.h backend.h common.h parser.h

parser.obj: parser.C config.h common.h backend.h parser.h

parser.C: parser.l
        flex -L parser.l
        del parser.C
        rename lex.yy.c parser.C

moves.obj: moves.c config.h backend.h common.h parser.h moves.h

lists.obj: lists.c config.h lists.h common.h

gamelist.obj: gamelist.c config.h lists.h common.h frontend.h backend.h \
        parser.h

pgntags.obj: pgntags.c config.h common.h frontend.h backend.h parser.h lists.h

wclipbrd.obj: wclipbrd.c config.h common.h winboard.h frontend.h backend.h

wedittags.obj: wedittags.c config.h common.h winboard.h frontend.h backend.h

wgamelist.obj: wgamelist.c config.h. common.h winboard.h frontend.h backend.h \
        wgamelist.h

woptions.obj: woptions.c config.h common.h winboard.h frontend.h backend.h

wsockerr.obj: wsockerr.c wsockerr.h

zippy.obj: zippy.c config.h common.h zippy.h frontend.h


$(proj).exe: $(proj).bin $(proj).res
        $(rc) -t -v -fe$(proj).exe $(proj).res $(proj).bin

$(proj).bin: $(allobj)
        $(link) -e$< @&&!
$(allobj)
$(libs)
!

clean:
        $(rm) *.obj


