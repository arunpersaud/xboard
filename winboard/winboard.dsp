# Microsoft Developer Studio Project File - Name="winboard" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) External Target" 0x0106

CFG=winboard - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "winboard.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "winboard.mak" CFG="winboard - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "winboard - Win32 Release" (based on "Win32 (x86) External Target")
!MESSAGE "winboard - Win32 Debug" (based on "Win32 (x86) External Target")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""

!IF  "$(CFG)" == "winboard - Win32 Release"

# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Cmd_Line "NMAKE /f winboard.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "winboard.exe"
# PROP BASE Bsc_Name "winboard.bsc"
# PROP BASE Target_Dir ""
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release"
# PROP Intermediate_Dir ".\Release"
# PROP Cmd_Line "nmake /f msvc.mak"
# PROP Rebuild_Opt "/a"
# PROP Target_File "winboard.exe"
# PROP Bsc_Name "winboard.bsc"
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "winboard - Win32 Debug"

# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Cmd_Line "NMAKE /f winboard.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "winboard.exe"
# PROP BASE Bsc_Name "winboard.bsc"
# PROP BASE Target_Dir ""
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Cmd_Line "nmake /f msvc.mak"
# PROP Rebuild_Opt "/a"
# PROP Target_File "winboard.exe"
# PROP Bsc_Name "winboard.bsc"
# PROP Target_Dir ""

!ENDIF 

# Begin Target

# Name "winboard - Win32 Release"
# Name "winboard - Win32 Debug"

!IF  "$(CFG)" == "winboard - Win32 Release"

!ELSEIF  "$(CFG)" == "winboard - Win32 Debug"

!ENDIF 

# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\backend.c
# End Source File
# Begin Source File

SOURCE=.\common.c
# End Source File
# Begin Source File

SOURCE=.\gamelist.c
# End Source File
# Begin Source File

SOURCE=.\lists.c
# End Source File
# Begin Source File

SOURCE=.\moves.c
# End Source File
# Begin Source File

SOURCE=.\parser.c
# End Source File
# Begin Source File

SOURCE=.\parser.l
# End Source File
# Begin Source File

SOURCE=.\pgntags.c
# End Source File
# Begin Source File

SOURCE=.\wedittags.c
# End Source File
# Begin Source File

SOURCE=.\wgamelist.c
# End Source File
# Begin Source File

SOURCE=.\winboard.c
# End Source File
# Begin Source File

SOURCE=.\wsockerr.c
# End Source File
# Begin Source File

SOURCE=.\zippy.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\backend.h
# End Source File
# Begin Source File

SOURCE=.\backendz.h
# End Source File
# Begin Source File

SOURCE=.\common.h
# End Source File
# Begin Source File

SOURCE=.\config.h
# End Source File
# Begin Source File

SOURCE=.\defaults.h
# End Source File
# Begin Source File

SOURCE=.\frontend.h
# End Source File
# Begin Source File

SOURCE=.\lists.h
# End Source File
# Begin Source File

SOURCE=.\moves.h
# End Source File
# Begin Source File

SOURCE=.\parser.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\wedittags.h
# End Source File
# Begin Source File

SOURCE=.\wgamelist.h
# End Source File
# Begin Source File

SOURCE=.\winboard.h
# End Source File
# Begin Source File

SOURCE=.\wsockerr.h
# End Source File
# Begin Source File

SOURCE=.\zippy.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\bitmaps\B21o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\B21s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\B40o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\B40s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\B64s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\B80o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\B80s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\bepbeep.wav
# End Source File
# Begin Source File

SOURCE=.\bitmaps\board.ico
# End Source File
# Begin Source File

SOURCE=.\bitmaps\ching.wav
# End Source File
# Begin Source File

SOURCE=.\bitmaps\click.wav
# End Source File
# Begin Source File

SOURCE=.\bitmaps\cymbal.wav
# End Source File
# Begin Source File

SOURCE=.\bitmaps\Ding1.wav
# End Source File
# Begin Source File

SOURCE=.\bitmaps\doodloop.wav
# End Source File
# Begin Source File

SOURCE=.\bitmaps\drip.wav
# End Source File
# Begin Source File

SOURCE=.\bitmaps\galactic.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\gong.wav
# End Source File
# Begin Source File

SOURCE=.\bitmaps\honkhonk.wav
# End Source File
# Begin Source File

SOURCE=.\bitmaps\icon_b.ico
# End Source File
# Begin Source File

SOURCE=.\bitmaps\icon_ob.ico
# End Source File
# Begin Source File

SOURCE=.\bitmaps\icon_ow.ico
# End Source File
# Begin Source File

SOURCE=.\bitmaps\icon_whi.ico
# End Source File
# Begin Source File

SOURCE=.\bitmaps\K21o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\K21s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\laser.wav
# End Source File
# Begin Source File

SOURCE=.\bitmaps\move.wav
# End Source File
# Begin Source File

SOURCE=.\bitmaps\N21o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\N21s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\P21o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\P21s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\penalty.wav
# End Source File
# Begin Source File

SOURCE=.\bitmaps\phone.wav
# End Source File
# Begin Source File

SOURCE=.\bitmaps\pop.wav
# End Source File
# Begin Source File

SOURCE=.\bitmaps\pop2.wav
# End Source File
# Begin Source File

SOURCE=.\bitmaps\Q21o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\Q21s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\R21o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\R21s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\R80o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\R80s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\slap.wav
# End Source File
# Begin Source File

SOURCE=.\bitmaps\squeak.wav
# End Source File
# Begin Source File

SOURCE=.\bitmaps\swish.wav
# End Source File
# Begin Source File

SOURCE=.\bitmaps\thud.wav
# End Source File
# Begin Source File

SOURCE=.\bitmaps\tim.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\whipcrak.wav
# End Source File
# Begin Source File

SOURCE=.\winboard.rc
# End Source File
# Begin Source File

SOURCE=.\WINBOARD.rtf
# End Source File
# Begin Source File

SOURCE=.\bitmaps\zap.wav
# End Source File
# End Group
# Begin Source File

SOURCE=.\sounds\alarm.wav
# End Source File
# Begin Source File

SOURCE=.\sounds\challenge.wav
# End Source File
# Begin Source File

SOURCE=.\sounds\channel.wav
# End Source File
# Begin Source File

SOURCE=.\sounds\channel1.wav
# End Source File
# Begin Source File

SOURCE=.\sounds\ching.wav
# End Source File
# Begin Source File

SOURCE=.\sounds\click.wav
# End Source File
# Begin Source File

SOURCE=.\sounds\cymbal.wav
# End Source File
# Begin Source File

SOURCE=.\sounds\ding1.wav
# End Source File
# Begin Source File

SOURCE=.\sounds\draw.wav
# End Source File
# Begin Source File

SOURCE=.\sounds\drip.wav
# End Source File
# Begin Source File

SOURCE=.\sounds\gong.wav
# End Source File
# Begin Source File

SOURCE=.\sounds\honkhonk.wav
# End Source File
# Begin Source File

SOURCE=.\sounds\kibitz.wav
# End Source File
# Begin Source File

SOURCE=.\sounds\laser.wav
# End Source File
# Begin Source File

SOURCE=.\sounds\lose.wav
# End Source File
# Begin Source File

SOURCE=.\sounds\move.wav
# End Source File
# Begin Source File

SOURCE=.\sounds\penalty.wav
# End Source File
# Begin Source File

SOURCE=.\sounds\phone.wav
# End Source File
# Begin Source File

SOURCE=.\sounds\pop.wav
# End Source File
# Begin Source File

SOURCE=.\sounds\pop2.wav
# End Source File
# Begin Source File

SOURCE=.\sounds\request.wav
# End Source File
# Begin Source File

SOURCE=.\sounds\seek.wav
# End Source File
# Begin Source File

SOURCE=.\sounds\shout.wav
# End Source File
# Begin Source File

SOURCE=.\sounds\slap.wav
# End Source File
# Begin Source File

SOURCE=.\sounds\squeak.wav
# End Source File
# Begin Source File

SOURCE=.\sounds\sshout.wav
# End Source File
# Begin Source File

SOURCE=.\sounds\swish.wav
# End Source File
# Begin Source File

SOURCE=.\sounds\tell.wav
# End Source File
# Begin Source File

SOURCE=.\sounds\thud.wav
# End Source File
# Begin Source File

SOURCE=.\sounds\unfinished.wav
# End Source File
# Begin Source File

SOURCE=.\sounds\whipcrak.wav
# End Source File
# Begin Source File

SOURCE=.\sounds\win.wav
# End Source File
# End Target
# End Project
