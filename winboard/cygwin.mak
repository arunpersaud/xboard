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


# Currently WinBoard will build either with or without cygwin1.dll
cygwin= -mno-cygwin

VPATH= .:..
CFLAGS= $(cdebug)
INCLUDES= -I. -I..
CVARS= $(INCLUDES) -mwindows $(cygwin)
CC = gcc $(CVARS)
WCC = $(CC) -mwindows -Xlinker "-e" -Xlinker "_mainCRTStartup" \
  $(linkdebug) $(cygwin)
LD = ld
HC="/c/program files/help workshop/hcrtf.exe" -xn

all: $(proj).exe

depend:
	makedepend -f cygwin.mak $(INCLUDES) *.c *.rc

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
	-lwsock32 -lcomctl32 -lwinmm  -lkernel32 \
	-ladvapi32 -luser32 -lgdi32 -lcomdlg32 -lwinspool \
	-o $(proj).exe
# DO NOT DELETE

parser.o: /usr/include/stdio.h /usr/include/_ansi.h /usr/include/newlib.h
parser.o: /usr/include/sys/config.h /usr/include/machine/ieeefp.h
parser.o: /usr/include/sys/reent.h /usr/include/sys/_types.h
parser.o: /usr/include/sys/types.h /usr/include/machine/types.h
parser.o: /usr/include/sys/stdio.h config.h /usr/include/ctype.h
parser.o: /usr/include/stdlib.h /usr/include/machine/stdlib.h
parser.o: /usr/include/alloca.h /usr/include/string.h ../common.h
parser.o: ../backend.h ../lists.h ../frontend.h ../parser.h ../moves.h
wclipbrd.o: config.h /usr/include/w32api/windows.h
wclipbrd.o: /usr/include/w32api/windef.h /usr/include/w32api/winnt.h
wclipbrd.o: /usr/include/w32api/winerror.h /usr/include/string.h
wclipbrd.o: /usr/include/_ansi.h /usr/include/newlib.h
wclipbrd.o: /usr/include/sys/config.h /usr/include/machine/ieeefp.h
wclipbrd.o: /usr/include/sys/reent.h /usr/include/sys/_types.h
wclipbrd.o: /usr/include/w32api/basetsd.h /usr/include/w32api/pshpack4.h
wclipbrd.o: /usr/include/w32api/poppack.h /usr/include/w32api/wincon.h
wclipbrd.o: /usr/include/w32api/winbase.h /usr/include/w32api/wingdi.h
wclipbrd.o: /usr/include/w32api/winuser.h /usr/include/w32api/winnls.h
wclipbrd.o: /usr/include/w32api/winver.h /usr/include/w32api/winnetwk.h
wclipbrd.o: /usr/include/w32api/winreg.h /usr/include/w32api/winsvc.h
wclipbrd.o: /usr/include/w32api/cderr.h /usr/include/w32api/dde.h
wclipbrd.o: /usr/include/w32api/ddeml.h /usr/include/w32api/dlgs.h
wclipbrd.o: /usr/include/w32api/imm.h /usr/include/w32api/lzexpand.h
wclipbrd.o: /usr/include/w32api/mmsystem.h /usr/include/w32api/nb30.h
wclipbrd.o: /usr/include/w32api/rpc.h /usr/include/w32api/rpcdce.h
wclipbrd.o: /usr/include/w32api/basetyps.h /usr/include/w32api/rpcdcep.h
wclipbrd.o: /usr/include/w32api/rpcnsi.h /usr/include/w32api/rpcnterr.h
wclipbrd.o: /usr/include/w32api/shellapi.h /usr/include/w32api/pshpack2.h
wclipbrd.o: /usr/include/w32api/winperf.h /usr/include/w32api/commdlg.h
wclipbrd.o: /usr/include/w32api/winspool.h /usr/include/w32api/winsock2.h
wclipbrd.o: /usr/include/stdio.h /usr/include/sys/types.h
wclipbrd.o: /usr/include/machine/types.h /usr/include/sys/stdio.h
wclipbrd.o: /usr/include/stdlib.h /usr/include/machine/stdlib.h
wclipbrd.o: /usr/include/alloca.h /usr/include/malloc.h
wclipbrd.o: /usr/include/machine/malloc.h /usr/include/sys/stat.h
wclipbrd.o: /usr/include/time.h /usr/include/machine/time.h
wclipbrd.o: /usr/include/sys/features.h ../common.h winboard.h resource.h
wclipbrd.o: ../frontend.h ../backend.h ../lists.h wclipbrd.h
wedittags.o: config.h /usr/include/w32api/windows.h
wedittags.o: /usr/include/w32api/windef.h /usr/include/w32api/winnt.h
wedittags.o: /usr/include/w32api/winerror.h /usr/include/string.h
wedittags.o: /usr/include/_ansi.h /usr/include/newlib.h
wedittags.o: /usr/include/sys/config.h /usr/include/machine/ieeefp.h
wedittags.o: /usr/include/sys/reent.h /usr/include/sys/_types.h
wedittags.o: /usr/include/w32api/basetsd.h /usr/include/w32api/pshpack4.h
wedittags.o: /usr/include/w32api/poppack.h /usr/include/w32api/wincon.h
wedittags.o: /usr/include/w32api/winbase.h /usr/include/w32api/wingdi.h
wedittags.o: /usr/include/w32api/winuser.h /usr/include/w32api/winnls.h
wedittags.o: /usr/include/w32api/winver.h /usr/include/w32api/winnetwk.h
wedittags.o: /usr/include/w32api/winreg.h /usr/include/w32api/winsvc.h
wedittags.o: /usr/include/w32api/cderr.h /usr/include/w32api/dde.h
wedittags.o: /usr/include/w32api/ddeml.h /usr/include/w32api/dlgs.h
wedittags.o: /usr/include/w32api/imm.h /usr/include/w32api/lzexpand.h
wedittags.o: /usr/include/w32api/mmsystem.h /usr/include/w32api/nb30.h
wedittags.o: /usr/include/w32api/rpc.h /usr/include/w32api/rpcdce.h
wedittags.o: /usr/include/w32api/basetyps.h /usr/include/w32api/rpcdcep.h
wedittags.o: /usr/include/w32api/rpcnsi.h /usr/include/w32api/rpcnterr.h
wedittags.o: /usr/include/w32api/shellapi.h /usr/include/w32api/pshpack2.h
wedittags.o: /usr/include/w32api/winperf.h /usr/include/w32api/commdlg.h
wedittags.o: /usr/include/w32api/winspool.h /usr/include/w32api/winsock2.h
wedittags.o: /usr/include/stdio.h /usr/include/sys/types.h
wedittags.o: /usr/include/machine/types.h /usr/include/sys/stdio.h
wedittags.o: /usr/include/stdlib.h /usr/include/machine/stdlib.h
wedittags.o: /usr/include/alloca.h /usr/include/malloc.h
wedittags.o: /usr/include/machine/malloc.h /usr/include/fcntl.h
wedittags.o: /usr/include/sys/fcntl.h /usr/include/sys/stat.h
wedittags.o: /usr/include/time.h /usr/include/machine/time.h
wedittags.o: /usr/include/sys/features.h /usr/include/math.h ../common.h
wedittags.o: winboard.h resource.h ../frontend.h ../backend.h ../lists.h
wgamelist.o: config.h /usr/include/w32api/windows.h
wgamelist.o: /usr/include/w32api/windef.h /usr/include/w32api/winnt.h
wgamelist.o: /usr/include/w32api/winerror.h /usr/include/string.h
wgamelist.o: /usr/include/_ansi.h /usr/include/newlib.h
wgamelist.o: /usr/include/sys/config.h /usr/include/machine/ieeefp.h
wgamelist.o: /usr/include/sys/reent.h /usr/include/sys/_types.h
wgamelist.o: /usr/include/w32api/basetsd.h /usr/include/w32api/pshpack4.h
wgamelist.o: /usr/include/w32api/poppack.h /usr/include/w32api/wincon.h
wgamelist.o: /usr/include/w32api/winbase.h /usr/include/w32api/wingdi.h
wgamelist.o: /usr/include/w32api/winuser.h /usr/include/w32api/winnls.h
wgamelist.o: /usr/include/w32api/winver.h /usr/include/w32api/winnetwk.h
wgamelist.o: /usr/include/w32api/winreg.h /usr/include/w32api/winsvc.h
wgamelist.o: /usr/include/w32api/cderr.h /usr/include/w32api/dde.h
wgamelist.o: /usr/include/w32api/ddeml.h /usr/include/w32api/dlgs.h
wgamelist.o: /usr/include/w32api/imm.h /usr/include/w32api/lzexpand.h
wgamelist.o: /usr/include/w32api/mmsystem.h /usr/include/w32api/nb30.h
wgamelist.o: /usr/include/w32api/rpc.h /usr/include/w32api/rpcdce.h
wgamelist.o: /usr/include/w32api/basetyps.h /usr/include/w32api/rpcdcep.h
wgamelist.o: /usr/include/w32api/rpcnsi.h /usr/include/w32api/rpcnterr.h
wgamelist.o: /usr/include/w32api/shellapi.h /usr/include/w32api/pshpack2.h
wgamelist.o: /usr/include/w32api/winperf.h /usr/include/w32api/commdlg.h
wgamelist.o: /usr/include/w32api/winspool.h /usr/include/w32api/winsock2.h
wgamelist.o: /usr/include/stdio.h /usr/include/sys/types.h
wgamelist.o: /usr/include/machine/types.h /usr/include/sys/stdio.h
wgamelist.o: /usr/include/stdlib.h /usr/include/machine/stdlib.h
wgamelist.o: /usr/include/alloca.h /usr/include/malloc.h
wgamelist.o: /usr/include/machine/malloc.h /usr/include/fcntl.h
wgamelist.o: /usr/include/sys/fcntl.h /usr/include/sys/stat.h
wgamelist.o: /usr/include/time.h /usr/include/machine/time.h
wgamelist.o: /usr/include/sys/features.h /usr/include/math.h ../common.h
wgamelist.o: winboard.h resource.h ../frontend.h ../backend.h ../lists.h
winboard.o: config.h /usr/include/w32api/windows.h
winboard.o: /usr/include/w32api/windef.h /usr/include/w32api/winnt.h
winboard.o: /usr/include/w32api/winerror.h /usr/include/string.h
winboard.o: /usr/include/_ansi.h /usr/include/newlib.h
winboard.o: /usr/include/sys/config.h /usr/include/machine/ieeefp.h
winboard.o: /usr/include/sys/reent.h /usr/include/sys/_types.h
winboard.o: /usr/include/w32api/basetsd.h /usr/include/w32api/pshpack4.h
winboard.o: /usr/include/w32api/poppack.h /usr/include/w32api/wincon.h
winboard.o: /usr/include/w32api/winbase.h /usr/include/w32api/wingdi.h
winboard.o: /usr/include/w32api/winuser.h /usr/include/w32api/winnls.h
winboard.o: /usr/include/w32api/winver.h /usr/include/w32api/winnetwk.h
winboard.o: /usr/include/w32api/winreg.h /usr/include/w32api/winsvc.h
winboard.o: /usr/include/w32api/cderr.h /usr/include/w32api/dde.h
winboard.o: /usr/include/w32api/ddeml.h /usr/include/w32api/dlgs.h
winboard.o: /usr/include/w32api/imm.h /usr/include/w32api/lzexpand.h
winboard.o: /usr/include/w32api/mmsystem.h /usr/include/w32api/nb30.h
winboard.o: /usr/include/w32api/rpc.h /usr/include/w32api/rpcdce.h
winboard.o: /usr/include/w32api/basetyps.h /usr/include/w32api/rpcdcep.h
winboard.o: /usr/include/w32api/rpcnsi.h /usr/include/w32api/rpcnterr.h
winboard.o: /usr/include/w32api/shellapi.h /usr/include/w32api/pshpack2.h
winboard.o: /usr/include/w32api/winperf.h /usr/include/w32api/commdlg.h
winboard.o: /usr/include/w32api/winspool.h /usr/include/w32api/winsock2.h
winboard.o: /usr/include/stdio.h /usr/include/sys/types.h
winboard.o: /usr/include/machine/types.h /usr/include/sys/stdio.h
winboard.o: /usr/include/stdlib.h /usr/include/machine/stdlib.h
winboard.o: /usr/include/alloca.h /usr/include/malloc.h
winboard.o: /usr/include/machine/malloc.h /usr/include/io.h
winboard.o: /usr/include/sys/stat.h /usr/include/time.h
winboard.o: /usr/include/machine/time.h /usr/include/sys/features.h
winboard.o: /usr/include/fcntl.h /usr/include/sys/fcntl.h /usr/include/math.h
winboard.o: /usr/include/w32api/richedit.h /usr/include/errno.h
winboard.o: /usr/include/sys/errno.h ../common.h winboard.h resource.h
winboard.o: ../frontend.h ../backend.h ../lists.h ../moves.h wclipbrd.h
winboard.o: wgamelist.h wedittags.h woptions.h wsockerr.h defaults.h
winboard.o: /usr/include/w32api/winsock.h
woptions.o: config.h /usr/include/w32api/windows.h
woptions.o: /usr/include/w32api/windef.h /usr/include/w32api/winnt.h
woptions.o: /usr/include/w32api/winerror.h /usr/include/string.h
woptions.o: /usr/include/_ansi.h /usr/include/newlib.h
woptions.o: /usr/include/sys/config.h /usr/include/machine/ieeefp.h
woptions.o: /usr/include/sys/reent.h /usr/include/sys/_types.h
woptions.o: /usr/include/w32api/basetsd.h /usr/include/w32api/pshpack4.h
woptions.o: /usr/include/w32api/poppack.h /usr/include/w32api/wincon.h
woptions.o: /usr/include/w32api/winbase.h /usr/include/w32api/wingdi.h
woptions.o: /usr/include/w32api/winuser.h /usr/include/w32api/winnls.h
woptions.o: /usr/include/w32api/winver.h /usr/include/w32api/winnetwk.h
woptions.o: /usr/include/w32api/winreg.h /usr/include/w32api/winsvc.h
woptions.o: /usr/include/w32api/cderr.h /usr/include/w32api/dde.h
woptions.o: /usr/include/w32api/ddeml.h /usr/include/w32api/dlgs.h
woptions.o: /usr/include/w32api/imm.h /usr/include/w32api/lzexpand.h
woptions.o: /usr/include/w32api/mmsystem.h /usr/include/w32api/nb30.h
woptions.o: /usr/include/w32api/rpc.h /usr/include/w32api/rpcdce.h
woptions.o: /usr/include/w32api/basetyps.h /usr/include/w32api/rpcdcep.h
woptions.o: /usr/include/w32api/rpcnsi.h /usr/include/w32api/rpcnterr.h
woptions.o: /usr/include/w32api/shellapi.h /usr/include/w32api/pshpack2.h
woptions.o: /usr/include/w32api/winperf.h /usr/include/w32api/commdlg.h
woptions.o: /usr/include/w32api/winspool.h /usr/include/w32api/winsock2.h
woptions.o: /usr/include/stdio.h /usr/include/sys/types.h
woptions.o: /usr/include/machine/types.h /usr/include/sys/stdio.h
woptions.o: /usr/include/stdlib.h /usr/include/machine/stdlib.h
woptions.o: /usr/include/alloca.h ../common.h winboard.h resource.h
woptions.o: ../backend.h ../lists.h ../frontend.h woptions.h defaults.h
woptions.o: wedittags.h /usr/include/w32api/richedit.h /usr/include/errno.h
woptions.o: /usr/include/sys/errno.h
wsockerr.o: /usr/include/w32api/windows.h /usr/include/w32api/windef.h
wsockerr.o: /usr/include/w32api/winnt.h /usr/include/w32api/winerror.h
wsockerr.o: /usr/include/string.h /usr/include/_ansi.h /usr/include/newlib.h
wsockerr.o: /usr/include/sys/config.h /usr/include/machine/ieeefp.h
wsockerr.o: /usr/include/sys/reent.h /usr/include/sys/_types.h
wsockerr.o: /usr/include/w32api/basetsd.h /usr/include/w32api/pshpack4.h
wsockerr.o: /usr/include/w32api/poppack.h /usr/include/w32api/wincon.h
wsockerr.o: /usr/include/w32api/winbase.h /usr/include/w32api/wingdi.h
wsockerr.o: /usr/include/w32api/winuser.h /usr/include/w32api/winnls.h
wsockerr.o: /usr/include/w32api/winver.h /usr/include/w32api/winnetwk.h
wsockerr.o: /usr/include/w32api/winreg.h /usr/include/w32api/winsvc.h
wsockerr.o: /usr/include/w32api/cderr.h /usr/include/w32api/dde.h
wsockerr.o: /usr/include/w32api/ddeml.h /usr/include/w32api/dlgs.h
wsockerr.o: /usr/include/w32api/imm.h /usr/include/w32api/lzexpand.h
wsockerr.o: /usr/include/w32api/mmsystem.h /usr/include/w32api/nb30.h
wsockerr.o: /usr/include/w32api/rpc.h /usr/include/w32api/rpcdce.h
wsockerr.o: /usr/include/w32api/basetyps.h /usr/include/w32api/rpcdcep.h
wsockerr.o: /usr/include/w32api/rpcnsi.h /usr/include/w32api/rpcnterr.h
wsockerr.o: /usr/include/w32api/shellapi.h /usr/include/w32api/pshpack2.h
wsockerr.o: /usr/include/w32api/winperf.h /usr/include/w32api/commdlg.h
wsockerr.o: /usr/include/w32api/winspool.h /usr/include/w32api/winsock2.h
wsockerr.o: /usr/include/w32api/winsock.h wsockerr.h
winboard.o: resource.h /usr/include/w32api/windows.h
winboard.o: /usr/include/w32api/windef.h /usr/include/w32api/winnt.h
winboard.o: /usr/include/w32api/winerror.h /usr/include/string.h
winboard.o: /usr/include/_ansi.h /usr/include/newlib.h
winboard.o: /usr/include/sys/config.h /usr/include/machine/ieeefp.h
winboard.o: /usr/include/sys/reent.h /usr/include/sys/_types.h
winboard.o: /usr/include/w32api/basetsd.h /usr/include/w32api/pshpack4.h
winboard.o: /usr/include/w32api/poppack.h /usr/include/w32api/wincon.h
winboard.o: /usr/include/w32api/winbase.h /usr/include/w32api/wingdi.h
winboard.o: /usr/include/w32api/winuser.h /usr/include/w32api/winnls.h
winboard.o: /usr/include/w32api/winver.h /usr/include/w32api/winnetwk.h
winboard.o: /usr/include/w32api/winreg.h /usr/include/w32api/winsvc.h
winboard.o: /usr/include/w32api/cderr.h /usr/include/w32api/dde.h
winboard.o: /usr/include/w32api/ddeml.h /usr/include/w32api/dlgs.h
winboard.o: /usr/include/w32api/imm.h /usr/include/w32api/lzexpand.h
winboard.o: /usr/include/w32api/mmsystem.h /usr/include/w32api/nb30.h
winboard.o: /usr/include/w32api/rpc.h /usr/include/w32api/rpcdce.h
winboard.o: /usr/include/w32api/basetyps.h /usr/include/w32api/rpcdcep.h
winboard.o: /usr/include/w32api/rpcnsi.h /usr/include/w32api/rpcnterr.h
winboard.o: /usr/include/w32api/shellapi.h /usr/include/w32api/pshpack2.h
winboard.o: /usr/include/w32api/winperf.h /usr/include/w32api/commdlg.h
winboard.o: /usr/include/w32api/winspool.h /usr/include/w32api/winsock2.h
