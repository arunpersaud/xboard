; WinBoard-4.2.7 MUI
;
;

;--------------------------------
;Include Modern UI

  !include "MUI.nsh"

;--------------------------------

!define InstName "WinBoard"
!define InstVersion "4.2.7"
!define InstBaseDir "WinBoard-4.2.7"

;--------------------------------
; General
;

Name "${InstName} ${InstVersion}"
Caption "WinBoard - Chessboard For Windows"
OutFile "WinBoard-4.2.7_full.exe"
InstallDir $PROGRAMFILES\${InstBaseDir}

SetCompressor lzma
SetDateSave on
SetDatablockOptimize on
CRCCheck on
SilentInstall normal
BGGradient 000000 4682b4 FFFFFF
;XPStyle on

;--------------------------------
;Variables

  Var INI_VALUE
  Var START_MENU_FOLDER
  Var MUI_TEMP

;--------------------------------


;!define MUI_ICON "knight.ico"
;!define MUI_UNICON "wc_uninst.ico"
!define MUI_WELCOMEFINISHPAGE_BITMAP "welcome_chess.bmp"

;--------------------------------
;Interface Settings

	!define MUI_ABORTWARNING
	;!define MUI_COMPONENTSPAGE_NODESC
	!define MUI_COMPONENTSPAGE_SMALLDESC
	!define MUI_WELCOMEPAGE_TEXT "This wizard will guide you through the installation of ${InstName} ${InstVersion}.\n\n\n\n\nClick Next to continue."
	!define MUI_LICENSEPAGE_TEXT_BOTTOM "$\nPress Continue to proceed with the installation."
	!define MUI_LICENSEPAGE_BUTTON "Continue"

;--------------------------------

;--------------------------------
; Pages

	; Install Section
	;--------------------------------------------------------------------
	!insertmacro MUI_PAGE_WELCOME
	!insertmacro MUI_PAGE_LICENSE "..\READ_ME.txt"
	!insertmacro MUI_PAGE_COMPONENTS
	Page custom FileAssoc
	!insertmacro MUI_PAGE_DIRECTORY
	
	;Start Menu Folder Page Configuration
        !define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKCU"
        !define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\WinBoard"
        !define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
	!insertmacro MUI_PAGE_STARTMENU Application $START_MENU_FOLDER
	
	!insertmacro MUI_PAGE_INSTFILES
	!insertmacro MUI_PAGE_FINISH
	;--------------------------------------------------------------------
	
	; Uninstall Section
	;
	;!define MUI_WELCOMEPAGE_TEXT "WARNING! THIS UNINSTALLER WILL COMPLETELY DELETE THE INSTALLATION DIRECTORY\n\n$INSTDIR !!!\n\nIF THERE ARE ANY FILES YOU WISH TO SAVE, MOVE THEM FROM THE INSTALLATION DIRECTORY FIRST!\n\n\n\nPress Next to Continue."
	!define MUI_WELCOMEPAGE_TEXT "This will uninstall WinBoard from directory:\n\n$INSTDIR\n\n\n\nPress Next to Continue."
	!insertmacro MUI_UNPAGE_WELCOME
	!insertmacro MUI_UNPAGE_CONFIRM
	!insertmacro MUI_UNPAGE_INSTFILES
	!insertmacro MUI_UNPAGE_FINISH

;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"

;--------------------------------

;--------------------------------
;Reserve Files
  
  ;These files should be inserted before other files in the data block
  ;Keep these lines before any File command
  ;Only for solid compression (by default, solid compression is enabled for BZIP2 and LZMA)
  
  ReserveFile "FA.ini"
  !insertmacro MUI_RESERVEFILE_INSTALLOPTIONS

;--------------------------------

Section "WinBoard Core Components" Core

SectionIn 1 RO

	SetOutPath $INSTDIR

	File ..\bughouse.bat
	File ..\ChangeLog
	File ..\COPYING
	File ..\COPYRIGHT
	File ..\cygncurses7.dll
	File ..\cygreadline5.dll
	File ..\cygwin1.dll
	File ..\FAQ.html
	File ..\kk13.pgn
	File ..\NEWS
	File ..\READ_ME.txt
	File ..\RJF60.pgn
	File ..\timeseal.exe
	File ..\timestamp.exe
	File ..\winboard.exe
	File ..\winboard.hlp
	File ..\zippy.lines
	File ..\zippy.README
	
	;Create uninstaller
	WriteUninstaller "$INSTDIR\UnInstall.exe"
	
	!insertmacro MUI_STARTMENU_WRITE_BEGIN Application

	CreateDirectory "$SMPROGRAMS\$START_MENU_FOLDER"
	CreateShortCut "$SMPROGRAMS\$START_MENU_FOLDER\Game Viewer.lnk" "$INSTDIR\winboard.exe" "-ncp" "$INSTDIR\winboard.exe" 1
	CreateShortCut "$SMPROGRAMS\$START_MENU_FOLDER\Game Viewer - Bobby Fischer.lnk" "$INSTDIR\winboard.exe" "-ncp -lgf RJF60.pgn" "$INSTDIR\winboard.exe" 1
	CreateShortCut "$SMPROGRAMS\$START_MENU_FOLDER\Game Viewer - Karpov vs Kasparov.lnk" "$INSTDIR\winboard.exe" "-ncp -lgf kk13.pgn" "$INSTDIR\winboard.exe" 1
        CreateShortCut "$SMPROGRAMS\$START_MENU_FOLDER\WinBoard Help.lnk" "$INSTDIR\winboard.hlp"
        CreateShortCut "$SMPROGRAMS\$START_MENU_FOLDER\Frequently Asked Questions.lnk" "$INSTDIR\FAQ.html"
        CreateShortCut "$SMPROGRAMS\$START_MENU_FOLDER\WinBoard READ_ME.lnk" "$INSTDIR\READ_ME.txt"
	CreateShortCut "$SMPROGRAMS\$START_MENU_FOLDER\WinBoard UnInstall.lnk" "$INSTDIR\UnInstall.exe"
	CreateShortCut "$SMPROGRAMS\$START_MENU_FOLDER\Chess Server - chessclub.com.lnk" "$INSTDIR\winboard.exe"  "-ics -icshost chessclub.com -icshelper timestamp" "$INSTDIR\winboard.exe" 0
	CreateShortCut "$SMPROGRAMS\$START_MENU_FOLDER\Chess Server - freechess.org.lnk" "$INSTDIR\winboard.exe"  "-ics -icshost freechess.org -icshelper timeseal" "$INSTDIR\winboard.exe" 0
	CreateShortCut "$SMPROGRAMS\$START_MENU_FOLDER\Chess Server - Other.lnk" "$INSTDIR\winboard.exe" "-ics" "$INSTDIR\winboard.exe" 0
	CreateShortCut "$SMPROGRAMS\$START_MENU_FOLDER\WinBoard Startup Dialog.lnk" "$INSTDIR\winboard.exe" "" "$INSTDIR\winboard.exe" 2
	
        !insertmacro MUI_STARTMENU_WRITE_END

SectionEnd

SubSection /e "Chess Engines" Engines

	Section "GNU Chess 5.0" GNUChess5

		SetOutPath "$INSTDIR"

		File ..\book.dat
		File ..\GNUChes5.exe
		File ..\gnuches5.txt
		File ..\gnuchess.dat
		File ..\gnuchess.lan
		File ..\gnuchess.README
		
		CreateDirectory "$SMPROGRAMS\$START_MENU_FOLDER\Chess Engines"

                !insertmacro MUI_STARTMENU_WRITE_BEGIN Application

		CreateShortCut "$SMPROGRAMS\$START_MENU_FOLDER\Chess Engines\GNU Chess 5.0 Documentation.lnk" "$INSTDIR\gnuches5.txt"
		CreateShortCut "$SMPROGRAMS\$START_MENU_FOLDER\Chess Engines\GNU Chess 5.0.lnk" "$INSTDIR\winboard.exe" "-cp -fcp 'GNUChes5 xboard' -scp 'GNUChes5 xboard'" "$INSTDIR\winboard.exe" 2
		
		!insertmacro MUI_STARTMENU_WRITE_END

	SectionEnd
	
	Section "GNU Chess 4.0" GNUChess4
	
                SetOutPath "$INSTDIR"
                
                File ..\gnuchesr.exe
		File ..\GNUChess.exe
		File ..\gnuchess.txt

		CreateDirectory "$SMPROGRAMS\$START_MENU_FOLDER\Chess Engines"

		!insertmacro MUI_STARTMENU_WRITE_BEGIN Application

		CreateShortCut "$SMPROGRAMS\$START_MENU_FOLDER\Chess Engines\GNU Chess 4.0 Documentation.lnk" "$INSTDIR\gnuchess.txt"
		CreateShortCut "$SMPROGRAMS\$START_MENU_FOLDER\Chess Engines\GNU Chess 4.0.lnk" "$INSTDIR\winboard.exe" "-cp -fcp GNUChess -scp GNUChess" "$INSTDIR\winboard.exe" 2
		
		!insertmacro MUI_STARTMENU_WRITE_END
		
         SectionEnd

	Section "Crafty 19.3" Crafty

		SetOutPath "$INSTDIR\Crafty"

		File ..\Crafty-WinBoard\wcrafty.exe
		File ..\Crafty-WinBoard\book.bin
		File ..\Crafty-WinBoard\books.bin
		File ..\Crafty-WinBoard\Crafty.rc
		File ..\Crafty-WinBoard\crafty.doc.txt
		
 		CreateDirectory "$SMPROGRAMS\$START_MENU_FOLDER\Chess Engines"
		
                !insertmacro MUI_STARTMENU_WRITE_BEGIN Application

		CreateShortCut "$SMPROGRAMS\$START_MENU_FOLDER\Chess Engines\Crafty Documentation.lnk" "$INSTDIR\Crafty\crafty.doc.txt"
		CreateShortCut "$SMPROGRAMS\$START_MENU_FOLDER\Chess Engines\Crafty 19.3.lnk" "$INSTDIR\winboard.exe" "-cp -fcp Crafty\wcrafty.exe -fd Crafty -scp Crafty\wcrafty.exe -sd Crafty" "$INSTDIR\winboard.exe" 2

		!insertmacro MUI_STARTMENU_WRITE_END


	SectionEnd

SubSectionEnd


;--------------------------------
;Installer Functions
Function .onInit

  ;Extract InstallOptions INI files
  !insertmacro MUI_INSTALLOPTIONS_EXTRACT "FA.ini"
  
FunctionEnd

Function FileAssoc

	!insertmacro MUI_HEADER_TEXT "Windows File Associations" "Do you want to use Winboard as your viewer for the following file types?"
	!insertmacro MUI_INSTALLOPTIONS_DISPLAY "FA.ini"

	
	;Read a value from an InstallOptions INI file

	!insertmacro MUI_INSTALLOPTIONS_READ $INI_VALUE "FA.ini" "Field 2" "State"
  
	;Display a messagebox if check box was checked
	StrCmp $INI_VALUE "1" "" CheckFEN
	WriteRegStr HKCR ".pgn" "" "WinBoard.PGN"
	WriteRegStr HKCR ".pgn" "Content Type" "application/x-chess-pgn"
	WriteRegStr HKCR "WinBoard.PGN" "" "Chess Game"
	WriteRegStr HKCR "WinBoard.PGN\DefaultIcon" "" "$INSTDIR\WinBoard.exe,1"
	WriteRegStr HKCR "WinBoard.PGN\Shell\Open" "" "Open"
	WriteRegStr HKCR "WinBoard.PGN\Shell\Open\command" "" '"$INSTDIR\WinBoard.exe" -ini "$INSTDIR\WinBoard.ini" -ncp -lgf "%1"'
	
	CheckFEN:
	!insertmacro MUI_INSTALLOPTIONS_READ $INI_VALUE "FA.ini" "Field 3" "State"
  
	;Display a messagebox if check box was checked
	StrCmp $INI_VALUE "1" "" Continue
	WriteRegStr HKCR ".fen" "" "WinBoard.FEN"
	WriteRegStr HKCR ".fen" "Content Type" "application/x-chess-fen"
	WriteRegStr HKCR "WinBoard.FEN" "" "Chess Position"
	WriteRegStr HKCR "WinBoard.FEN\DefaultIcon" "" "$INSTDIR\WinBoard.exe,1"
	WriteRegStr HKCR "WinBoard.FEN\Shell\Open" "" "Open"
	WriteRegStr HKCR "WinBoard.FEN\Shell\Open\command" "" '"$INSTDIR\WinBoard.exe" -ini "$INSTDIR\WinBoard.ini" -ncp -lpf "%1"'
	
	Continue:

FunctionEnd

;--------------------------------
;Descriptions

  ;Language strings

	LangString DESC_Core ${LANG_ENGLISH} "Winboard Core Components - Executable and Help Files"
	LangString DESC_Engines ${LANG_ENGLISH} "Chess Engines to play against using the WinBoard Interface"
	LangString DESC_GNUChess5 ${LANG_ENGLISH} "GNU Chess 5.0 Engine"
	LangString DESC_GNUChess4 ${LANG_ENGLISH} "GNU Chess 4.0 Engine"
	LangString DESC_Crafty ${LANG_ENGLISH} "Crafty 19.3 Chess Engine, by Robert Hyatt"


  ;Assign language strings to sections

  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
	!insertmacro MUI_DESCRIPTION_TEXT ${Core} $(DESC_Core)
	!insertmacro MUI_DESCRIPTION_TEXT ${Engines} $(DESC_Engines)
	!insertmacro MUI_DESCRIPTION_TEXT ${GNUChess5} $(DESC_GNUChess5)
	!insertmacro MUI_DESCRIPTION_TEXT ${GNUChess4} $(DESC_GNUChess4)
	!insertmacro MUI_DESCRIPTION_TEXT ${Crafty} $(DESC_Crafty)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------

;--------------------------------
;Uninstaller Section

; Function to delete install directory if non-empty on user request
Function un.ForceDirectoryDelete

         MessageBox MB_YESNO "The uninstaller was unable to delete the directory '$INSTDIR'. This is usually$\r$\ndue to user-created files such as WinBoard.ini, or other configuration files. If$\r$\nyou wish to keep your old configuration files, select No.$\r$\n$\r$\nForce deletion of install directory and all files in it?" IDNO End
         
         RMDir /r "$INSTDIR"
         
         End:

FunctionEnd


Section "Uninstall"

        Delete "$INSTDIR\bughouse.bat"
	Delete "$INSTDIR\ChangeLog"
	Delete "$INSTDIR\COPYING"
	Delete "$INSTDIR\COPYRIGHT"
	Delete "$INSTDIR\cygncurses7.dll"
	Delete "$INSTDIR\cygreadline5.dll"
	Delete "$INSTDIR\cygwin1.dll"
	Delete "$INSTDIR\FAQ.html"
	Delete "$INSTDIR\kk13.pgn"
	Delete "$INSTDIR\NEWS"
	Delete "$INSTDIR\READ_ME.txt"
	Delete "$INSTDIR\RJF60.pgn"
	Delete "$INSTDIR\timeseal.exe"
	Delete "$INSTDIR\timestamp.exe"
	Delete "$INSTDIR\winboard.exe"
	Delete "$INSTDIR\winboard.hlp"
	Delete "$INSTDIR\zippy.lines"
	Delete "$INSTDIR\zippy.README"
        Delete "$INSTDIR\book.dat"
	Delete "$INSTDIR\GNUChes5.exe"
	Delete "$INSTDIR\gnuches5.txt"
	Delete "$INSTDIR\gnuchess.dat"
	Delete "$INSTDIR\gnuchess.lan"
	Delete "$INSTDIR\gnuchess.README"
        Delete "$INSTDIR\gnuchesr.exe"
	Delete "$INSTDIR\GNUChess.exe"
	Delete "$INSTDIR\gnuchess.txt"
	Delete "$INSTDIR\UnInstall.exe"
	Delete "$INSTDIR\Crafty\wcrafty.exe"
	Delete "$INSTDIR\Crafty\book.bin"
	Delete "$INSTDIR\Crafty\books.bin"
	Delete "$INSTDIR\Crafty\Crafty.rc"
	Delete "$INSTDIR\Crafty\crafty.doc.txt"
	RMDir "$INSTDIR\Crafty"
	RMDir "$INSTDIR"

 	
	!insertmacro MUI_STARTMENU_GETFOLDER Application $MUI_TEMP
	
	RMDir /r "$SMPROGRAMS\$MUI_TEMP"
	
	ReadRegStr $1 HKCR ".pgn" ""
	StrCmp $1 "WinBoard.PGN" "" DelFEN
	ReadRegStr $1 HKCR "WinBoard.PGN\Shell\Open\command" ""
	StrCmp $1 '"$INSTDIR\WinBoard.exe" -ini "$INSTDIR\WinBoard.ini" -ncp -lgf "%1"' "" DelFEN
	DeleteRegKey HKCR ".pgn"
	DeleteRegKey HKCR "WinBoard.PGN"
	
	DelFEN:
	
	ReadRegStr $1 HKCR ".fen" ""
	StrCmp $1 "WinBoard.FEN" "" ContDelFEN
	ReadRegStr $1 HKCR "WinBoard.FEN\Shell\Open\command" ""
	StrCmp $1 '"$INSTDIR\WinBoard.exe" -ini "$INSTDIR\WinBoard.ini" -ncp -lpf "%1"' "" ContDelFEN
	DeleteRegKey HKCR ".fen"
	DeleteRegKey HKCR "WinBoard.FEN"
	
	ContDelFEN:
	
	DeleteRegKey HKCU "Software\WinBoard"
	
	
	IfFileExists "$INSTDIR\*.*" GoDirDel Continue

	GoDirDel:
	Call un.ForceDirectoryDelete

	Continue:


SectionEnd
