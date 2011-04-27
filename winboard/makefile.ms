# Command line: nmake /f makefile.ms
# Set VCVER=6 for MSVC 4.x through 7.0(aka 2002 aka .NET).
# For MSVC 8 (aka 2005) set VCVER=8.  Beyond that try 8 first.
VCVER=6

# If using MSVC 6.0 or earlier, you will need the latest platform SDK supported.
# Set SDK_INC to the include directory after you install it.
# See: http://www.microsoft.com/msdownload/platformsdk/sdkupdate/psdk-full.htm
#SDK_INC="C:\Program Files\Microsoft SDK\include"


PROJ = winboard


OBJS=backend.obj book.obj gamelist.obj lists.obj moves.obj pgntags.obj uci.obj\
 zippy.obj parser.obj wclipbrd.obj wedittags.obj wengineoutput.obj wevalgraph.obj\
 wgamelist.obj whistory.obj history.obj winboard.obj wlayout.obj woptions.obj wsnap.obj\
 wsockerr.obj help.obj wsettings.obj wchat.obj engineoutput.obj evalgraph.obj


# Debugging?
USE_DEBUG=0


# JAWS support?
JAWS=0


CC = @cl
RC = @rc
LINK = link
HC="c:\program files\help workshop\hcrtf.exe" -xn
FLEX = "c:/mingw/mingw32/bin/flex.exe"
DEFS = -D_WIN32_IE=0x300 -DWIN32 -DWINVER=0x0500 -D_WIN32_WINDOWS=0x500 -D YY_NO_UNISTD_H
!if $(JAWS) == 1
DEFS = -DJAWS $(DEFS)
!endif
#WARN = -W3
ARCH =


!if $(VCVER) >= 8
CLIB= libcmt
LIBF= -MT
DEPRECATE=-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE -D_USE_32BIT_TIME_T
!else
VSO = -Gf -Og
CLIB = msvcrt
LIBF = -MD
!endif


!if $(USE_DEBUG) == 0
CFDEBUG = -DNDEBUG
OPTS = -Os -Oy -Gs -GA -Zl $(VSO) $(LIBF)
LFDEBUG = $(CLIB).lib
!else
CFDEBUG = -D_DEBUG -DDEBUG
OPTS = -Od -Zi -Fd$(PROJ).pdb $(LIBF)d
LFDEBUG = -DEBUG -PDB:$(PROJ).pdb -MAP:$(PROJ).map $(CLIB)d.lib
!endif
CF = $(CFDEBUG) $(DEFS) $(DEPRECATE) -I. -I..
!if DEFINED(SDK_INC)
CF = $(CF) -I$(SDK_INC)
!endif
CFLAGS = -c -nologo $(WARN) $(CF) $(ARCH) $(OPTS)
LFLAGS = -nologo $(LFDEBUG)


all: $(PROJ).exe


clean:
        -erase *~ $(PROJ).exe $(PROJ).err $(PROJ).rbj $(PROJ).res *.sbr *.bsc\
        *.o *.obj *.plg *.opt *.ncb *.debug *.bak *.gid *.map *.pdb *.ini


# Update the help file if necessary
$(PROJ).hlp : $(PROJ).rtf
        $(HC) $(PROJ).hpj
        -@type $(PROJ).err
        -@rename $(PROJ).hlp $(PROJ).hlp.tmp
        -@rename $(PROJ).hlp.tmp $(PROJ).hlp


# Update the resource if necessary
$(PROJ).res: $(PROJ).rc $(PROJ).h resource.h config.h
    $(RC) $(CF) -r -fo $(PROJ).res $(PROJ).rc

winboard.obj: winboard.c config.h winboard.h ../common.h ../frontend.h \
        ../backend.h ../moves.h defaults.h resource.h wclipbrd.h \
        wsockerr.h woptions.h wsnap.h ../lists.h help.h ../args.h
        $(CC) $(CFLAGS) winboard.c

backend.obj: ../backend.c config.h ../common.h ../frontend.h ../backend.h \
        ../parser.h ../moves.h ../zippy.h ../backendz.h ../gettext.h ../lists.h
        $(CC) $(CFLAGS) ../backend.c

parser.obj: ../parser.c config.h ../common.h ../backend.h ../parser.h \
        ../frontend.h ../moves.h ../lists.h
        $(CC) $(CFLAGS) ../parser.c

book.obj: ../book.c ../common.h ../backend.h ../lists.h
        $(CC) $(CFLAGS) ../book.c

gamelist.obj: ../gamelist.c config.h ../lists.h ../common.h ../frontend.h \
        ../backend.h ../parser.h
        $(CC) $(CFLAGS) ../gamelist.c

lists.obj: ../lists.c config.h ../lists.h ../common.h
        $(CC) $(CFLAGS) ../lists.c

moves.obj: ../moves.c config.h ../backend.h ../common.h ../parser.h \
        ../moves.h ../lists.h
        $(CC) $(CFLAGS) ../moves.c

wclipbrd.obj: wclipbrd.c config.h ../common.h ../frontend.h ../backend.h \
        winboard.h resource.h wclipbrd.h ../lists.h
        $(CC) $(CFLAGS) wclipbrd.c

wedittags.obj: wedittags.c config.h ../common.h winboard.h resource.h ../frontend.h \
        ../backend.h ../lists.h
        $(CC) $(CFLAGS) wedittags.c

wgamelist.obj: wgamelist.c config.h ../common.h winboard.h resource.h ../frontend.h \
        ../backend.h ../lists.h
        $(CC) $(CFLAGS) wgamelist.c

woptions.obj: woptions.c config.h ../common.h ../frontend.h ../backend.h \
	../lists.h defaults.h winboard.h resource.h
        $(CC) $(CFLAGS) woptions.c

wengineoutput.obj: wengineoutput.c config.h ../common.h ../frontend.h ../backend.h \
	../lists.h winboard.h resource.h wsnap.h
        $(CC) $(CFLAGS) wengineoutput.c

engineoutput.obj: ../engineoutput.c ../engineoutput.h config.h ../common.h \
	../frontend.h ../backend.h ../lists.h
        $(CC) $(CFLAGS) ../engineoutput.c

whistory.obj: whistory.c config.h ../common.h ../frontend.h ../backend.h \
	../lists.h winboard.h resource.h wsnap.h
        $(CC) $(CFLAGS) whistory.c

history.obj: ../history.c config.h ../common.h ../frontend.h ../backend.h \
	../lists.h
        $(CC) $(CFLAGS) ../history.c

wevalgraph.obj: wevalgraph.c config.h ../common.h ../frontend.h ../backend.h \
	../lists.h winboard.h resource.h wsnap.h
        $(CC) $(CFLAGS) wevalgraph.c

evalgraph.obj: ../evalgraph.c ../evalgraph.h config.h ../common.h ../frontend.h \
	../backend.h ../lists.h
        $(CC) $(CFLAGS) ../evalgraph.c

wlayout.obj: wlayout.c config.h ../common.h ../frontend.h winboard.h resource.h
        $(CC) $(CFLAGS) wlayout.c

wsockerr.obj: wsockerr.c wsockerr.h
        $(CC) $(CFLAGS) wsockerr.c

help.obj: help.c help.h
        $(CC) $(CFLAGS) help.c

wsnap.obj: wsnap.c wsnap.h
        $(CC) $(CFLAGS) wsnap.c

pgntags.obj: ../pgntags.c config.h ../common.h ../frontend.h ../backend.h \
        ../parser.h ../lists.h
        $(CC) $(CFLAGS) ../pgntags.c

zippy.obj: ../zippy.c config.h ../common.h ../zippy.h ../frontend.h \
	../backend.h ../backendz.h ../lists.h
        $(CC) $(CFLAGS) ../zippy.c

uci.obj: ../uci.c ../common.h ../backend.h ../frontend.h ../lists.h
        $(CC) $(CFLAGS) ../uci.c


wsettings.obj: wsettings.c ../common.h ../backend.h ../frontend.h ../lists.h
        $(CC) $(CFLAGS) wsettings.c

wchat.obj: wchat.c winboard.h wsnap.h ../common.h ../backend.h ../frontend.h ../lists.h
        $(CC) $(CFLAGS) wchat.c


$(PROJ).exe: $(OBJS) $(PROJ).res $(PROJ).hlp
    $(LINK) $(LFLAGS) $(OBJS) wsock32.lib comctl32.lib winmm.lib shell32.lib\
 oldnames.lib kernel32.lib advapi32.lib user32.lib gdi32.lib comdlg32.lib\
 $(PROJ).res -out:$(PROJ).exe


.c.obj:
        $(CC) $(CFLAGS) $< 
