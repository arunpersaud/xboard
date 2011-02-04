# Microsoft Developer Studio Project File - Name="winboard" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=winboard - Win32 Jaws Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "winboard.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "winboard.mak" CFG="winboard - Win32 Jaws Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "winboard - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "winboard - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "winboard - Win32 Jaws Debug" (based on "Win32 (x86) Application")
!MESSAGE "winboard - Win32 Jaws Release" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "winboard - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Og /Os /Oy /Gf /I "." /I ".." /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D _WIN32_IE=0x300 /D WINVER=0x400 /D _WIN32_WINDOWS=0x500 /D "YY_NO_UNISTD_H" /YX /Zl /FD /Gs /GA /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /i ".." /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 wsock32.lib comctl32.lib winmm.lib shell32.lib oldnames.lib kernel32.lib advapi32.lib user32.lib gdi32.lib comdlg32.lib msvcrt.lib /nologo /subsystem:windows /pdb:none /machine:I386

!ELSEIF  "$(CFG)" == "winboard - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "." /I ".." /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D _WIN32_IE=0x300 /D WINVER=0x400 /D _WIN32_WINDOWS=0x500 /D "YY_NO_UNISTD_H" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i ".." /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 wsock32.lib comctl32.lib winmm.lib shell32.lib oldnames.lib kernel32.lib advapi32.lib user32.lib gdi32.lib comdlg32.lib msvcrtd.lib /nologo /subsystem:windows /map /debug /machine:I386 /pdbtype:sept

!ELSEIF  "$(CFG)" == "winboard - Win32 Jaws Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "winboard___Win32_Jaws_Debug"
# PROP BASE Intermediate_Dir "winboard___Win32_Jaws_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Jaws-Debug"
# PROP Intermediate_Dir "Jaws-Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "." /I ".." /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D _WIN32_IE=0x300 /D WINVER=0x400 /D _WIN32_WINDOWS=0x500 /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "." /I ".." /D "_DEBUG" /D "JAWS" /D "WIN32" /D "_WINDOWS" /D _WIN32_IE=0x300 /D WINVER=0x400 /D _WIN32_WINDOWS=0x500 /D "YY_NO_UNISTD_H" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /i ".." /d "_DEBUG"
# ADD RSC /l 0x409 /i ".." /d "_DEBUG" /d "JAWS"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wsock32.lib comctl32.lib winmm.lib shell32.lib oldnames.lib kernel32.lib advapi32.lib user32.lib gdi32.lib comdlg32.lib msvcrtd.lib /nologo /subsystem:windows /map /debug /machine:I386 /pdbtype:sept
# ADD LINK32 wsock32.lib comctl32.lib winmm.lib shell32.lib oldnames.lib kernel32.lib advapi32.lib user32.lib gdi32.lib comdlg32.lib msvcrtd.lib /nologo /subsystem:windows /map /debug /machine:I386 /pdbtype:sept

!ELSEIF  "$(CFG)" == "winboard - Win32 Jaws Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "winboard___Win32_Jaws_Release"
# PROP BASE Intermediate_Dir "winboard___Win32_Jaws_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Jaws-Release"
# PROP Intermediate_Dir "Jaws-Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /Og /Os /Oy /Gf /I "." /I ".." /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D _WIN32_IE=0x300 /D WINVER=0x400 /D _WIN32_WINDOWS=0x500 /YX /Zl /FD /Gs /GA /c
# ADD CPP /nologo /MD /W3 /GX /Og /Os /Oy /Gf /I "." /I ".." /D "NDEBUG" /D "JAWS" /D "WIN32" /D "_WINDOWS" /D _WIN32_IE=0x300 /D WINVER=0x400 /D _WIN32_WINDOWS=0x500 /D "YY_NO_UNISTD_H" /YX /Zl /FD /Gs /GA /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /i ".." /d "NDEBUG"
# ADD RSC /l 0x409 /i ".." /d "NDEBUG" /d "JAWS"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wsock32.lib comctl32.lib winmm.lib shell32.lib oldnames.lib kernel32.lib advapi32.lib user32.lib gdi32.lib comdlg32.lib msvcrt.lib /nologo /subsystem:windows /pdb:none /machine:I386
# ADD LINK32 wsock32.lib comctl32.lib winmm.lib shell32.lib oldnames.lib kernel32.lib advapi32.lib user32.lib gdi32.lib comdlg32.lib msvcrt.lib /nologo /subsystem:windows /pdb:none /machine:I386

!ENDIF 

# Begin Target

# Name "winboard - Win32 Release"
# Name "winboard - Win32 Debug"
# Name "winboard - Win32 Jaws Debug"
# Name "winboard - Win32 Jaws Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\backend.c
# End Source File
# Begin Source File

SOURCE=..\book.c
# End Source File
# Begin Source File

SOURCE=..\engineoutput.c
# End Source File
# Begin Source File

SOURCE=..\evalgraph.c
# End Source File
# Begin Source File

SOURCE=..\gamelist.c
# End Source File
# Begin Source File

SOURCE=.\help.c
# End Source File
# Begin Source File

SOURCE=..\history.c
# End Source File
# Begin Source File

SOURCE=..\lists.c
# End Source File
# Begin Source File

SOURCE=..\moves.c
# End Source File
# Begin Source File

SOURCE=..\parser.c
# End Source File
# Begin Source File

SOURCE=..\pgntags.c
# End Source File
# Begin Source File

SOURCE=..\uci.c
# End Source File
# Begin Source File

SOURCE=.\wchat.c
# End Source File
# Begin Source File

SOURCE=.\wclipbrd.c
# End Source File
# Begin Source File

SOURCE=.\wedittags.c
# End Source File
# Begin Source File

SOURCE=.\wengineoutput.c
# End Source File
# Begin Source File

SOURCE=.\wevalgraph.c
# End Source File
# Begin Source File

SOURCE=.\wgamelist.c
# End Source File
# Begin Source File

SOURCE=.\whistory.c
# End Source File
# Begin Source File

SOURCE=.\winboard.c

!IF  "$(CFG)" == "winboard - Win32 Release"

!ELSEIF  "$(CFG)" == "winboard - Win32 Debug"

!ELSEIF  "$(CFG)" == "winboard - Win32 Jaws Debug"

# ADD CPP /D WINVER=0x500
# SUBTRACT CPP /D WINVER=0x400

!ELSEIF  "$(CFG)" == "winboard - Win32 Jaws Release"

# ADD CPP /D WINVER=0x500
# SUBTRACT CPP /D WINVER=0x400

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\winboard.rc
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
# End Source File
# Begin Source File

SOURCE=.\wlayout.c
# End Source File
# Begin Source File

SOURCE=.\woptions.c
# End Source File
# Begin Source File

SOURCE=.\wsettings.c
# End Source File
# Begin Source File

SOURCE=.\wsnap.c
# End Source File
# Begin Source File

SOURCE=.\wsockerr.c
# End Source File
# Begin Source File

SOURCE=..\zippy.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
