# Microsoft Developer Studio Project File - Name="winboard" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) External Target" 0x0106

CFG=winboard - Win32 Release
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "winboard.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "winboard.mak" CFG="winboard - Win32 Release"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "winboard - Win32 Release" (basierend auf  "Win32 (x86) External Target")
!MESSAGE "winboard - Win32 Debug" (basierend auf  "Win32 (x86) External Target")
!MESSAGE "winboard - Win32 Opt" (basierend auf  "Win32 (x86) External Target")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
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
# PROP Cmd_Line "NMAKE"
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
# PROP Cmd_Line "NMAKE"
# PROP Rebuild_Opt ""
# PROP Target_File "winboard.exe"
# PROP Bsc_Name "winboard.bsc"
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "winboard - Win32 Opt"

# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "winboard___Win32_Opt"
# PROP BASE Intermediate_Dir "winboard___Win32_Opt"
# PROP BASE Cmd_Line "NMAKE"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "winboard.exe"
# PROP BASE Bsc_Name "winboard.bsc"
# PROP BASE Target_Dir ""
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "winboard___Win32_Opt"
# PROP Intermediate_Dir "winboard___Win32_Opt"
# PROP Cmd_Line "NMAKE"
# PROP Rebuild_Opt "/a"
# PROP Target_File "winboard.exe"
# PROP Bsc_Name "winboard.bsc"
# PROP Target_Dir ""

!ENDIF 

# Begin Target

# Name "winboard - Win32 Release"
# Name "winboard - Win32 Debug"
# Name "winboard - Win32 Opt"

!IF  "$(CFG)" == "winboard - Win32 Release"

!ELSEIF  "$(CFG)" == "winboard - Win32 Debug"

!ELSEIF  "$(CFG)" == "winboard - Win32 Opt"

!ENDIF 

# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\backend.c
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

SOURCE=.\woptions.c
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

SOURCE=.\woptions.h
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

SOURCE=.\bitmaps\b108o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\b108s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\b108w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\b116o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\b116s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\b116w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\b129o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\b129s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\b129w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\B21o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\B21s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\b21w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\b25o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\b25s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\b25w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\b29o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\b29s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\b29w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\b33o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\b33s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\b33w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\b37o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\b37s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\b37w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\B40o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\B40s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\b40w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\b45o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\b45s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\b45w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\b49o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\b49s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\b49w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\b54o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\b54s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\b54w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\b58o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\b58s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\b58w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\b64o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\B64s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\b64w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\b72o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\b72s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\b72w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\B80o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\B80s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\b80w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\b87o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\b87s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\b87w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\b95o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\b95s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\b95w.bmp
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

SOURCE=.\bitmaps\k108o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\k108s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\k108w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\k116o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\k116s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\k116w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\k129o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\k129s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\k129w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\K21o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\K21s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\k21w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\k25o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\k25s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\k25w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\k29o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\k29s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\k29w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\k33o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\k33s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\k33w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\k37o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\k37s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\k37w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\k40o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\k40s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\k40w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\k45o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\k45s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\k45w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\k49o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\k49s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\k49w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\k54o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\k54s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\k54w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\k58o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\k58s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\k58w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\k64o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\k64s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\k64w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\k72o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\k72s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\k72w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\k80o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\k80s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\k80w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\k87o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\k87s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\k87w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\k95o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\k95s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\k95w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\laser.wav
# End Source File
# Begin Source File

SOURCE=.\bitmaps\move.wav
# End Source File
# Begin Source File

SOURCE=.\bitmaps\n108o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\n108s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\n108w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\n116o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\n116s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\n116w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\n129o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\n129s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\n129w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\N21o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\N21s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\n21w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\n25o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\n25s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\n25w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\n29o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\n29s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\n29w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\n33o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\n33s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\n33w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\n37o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\n37s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\n37w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\n40o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\n40s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\n40w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\n45o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\n45s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\n45w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\n49o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\n49s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\n49w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\n54o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\n54s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\n54w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\n58o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\n58s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\n58w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\n64o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\n64s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\n64w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\n72o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\n72s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\n72w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\n80o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\n80s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\n80w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\n87o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\n87s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\n87w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\n95o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\n95s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\n95w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\p108o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\p108s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\p108w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\p116o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\p116s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\p116w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\p129o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\p129s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\p129w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\P21o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\P21s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\p21w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\p25o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\p25s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\p25w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\p29o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\p29s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\p29w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\p33o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\p33s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\p33w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\p37o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\p37s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\p37w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\p40o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\p40s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\p40w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\p45o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\p45s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\p45w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\p49o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\p49s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\p49w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\p54o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\p54s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\p54w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\p58o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\p58s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\p58w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\p64o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\p64s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\p64w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\p72o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\p72s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\p72w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\p80o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\p80s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\p80w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\p87o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\p87s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\p87w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\p95o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\p95s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\p95w.bmp
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

SOURCE=.\bitmaps\q108o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\q108s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\q108w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\q116o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\q116s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\q116w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\q129o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\q129s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\q129w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\Q21o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\Q21s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\q21w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\q25o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\q25s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\q25w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\q29o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\q29s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\q29w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\q33o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\q33s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\q33w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\q37o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\q37s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\q37w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\q40o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\q40s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\q40w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\q45o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\q45s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\q45w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\q49o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\q49s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\q49w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\q54o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\q54s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\q54w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\q58o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\q58s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\q58w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\q64o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\q64s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\q64w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\q72o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\q72s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\q72w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\q80o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\q80s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\q80w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\q87o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\q87s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\q87w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\q95o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\q95s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\q95w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\r108o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\r108s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\r108w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\r116o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\r116s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\r116w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\r129o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\r129s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\r129w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\R21o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\R21s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\r21w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\r25o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\r25s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\r25w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\r29o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\r29s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\r29w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\r33o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\r33s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\r33w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\r37o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\r37s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\r37w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\r40o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\r40s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\r40w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\r45o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\r45s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\r45w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\r49o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\r49s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\r49w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\r54o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\r54s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\r54w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\r58o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\r58s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\r58w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\r64o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\r64s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\r64w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\r72o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\r72s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\r72w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\R80o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\R80s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\r80w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\r87o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\r87s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\r87w.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\r95o.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\r95s.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\r95w.bmp
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
