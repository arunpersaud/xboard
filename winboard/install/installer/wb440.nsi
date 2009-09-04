; WinBoard-4.4.0 MUI
;
;

!define FILES "..\..\"
!define ROOT "${FILES}Chess\"
!define FNTDIR "${FILES}Chess\RePackage\"

; grab the FontName plugin from NSIS for these
!include FontRegAdv.nsh
!include FontName.nsh

;--------------------------------
;Include Modern UI
!include "MUI.nsh"

;--------------------------------

!define InstName "WinBoard"
!define InstVersion "4.4.0beta2"
!define InstBaseDir "WinBoard-4.4.0"

;--------------------------------
; General
;

Name "${InstName} ${InstVersion}"
Caption "WinBoard - Chessboard For Windows"
OutFile "WinBoard-4.4.0beta2.exe"
InstallDir $PROGRAMFILES\${InstBaseDir}

SetCompressor lzma
SetDateSave on
SetDatablockOptimize on
CRCCheck on
SilentInstall normal
;BGGradient 000000 4682b4 FFFFFF
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
    !insertmacro MUI_PAGE_LICENSE "${ROOT}WinBoard\doc\COPYRIGHTS.txt"
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


    SetOutPath "$INSTDIR\WinBoard\doc"
    File "${ROOT}WinBoard\doc\fonts.html"
    File "${ROOT}WinBoard\doc\manual.html"
    File "${ROOT}WinBoard\doc\UCIconfig.html"
    File "${ROOT}WinBoard\doc\shortcuts.html"
    File "${ROOT}WinBoard\doc\texture.html"
    File "${ROOT}WinBoard\doc\engine-intf.html"
    File "${ROOT}WinBoard\doc\FAQ.html"
    File "${ROOT}WinBoard\doc\mini.gif"
    File "${ROOT}WinBoard\doc\PG2fruit.png"
    File "${ROOT}WinBoard\doc\zippy.README"
    File "${ROOT}WinBoard\doc\README.html"
    File "${ROOT}WinBoard\doc\COPYRIGHTS.txt"
    File "${ROOT}WinBoard\doc\COPYRIGHT.txt"
    File "${ROOT}WinBoard\doc\COPYING.txt"

    ; logo bitmaps for ICS and users
    SetOutPath "$INSTDIR\WinBoard\logos"
    File "${ROOT}WinBoard\logos\chessclub.com.bmp"
    File "${ROOT}WinBoard\logos\freechess.org.bmp"
    File "${ROOT}WinBoard\logos\administrator.bmp"
    File "${ROOT}WinBoard\logos\user.bmp"
    File "${ROOT}WinBoard\logos\guest.bmp"
    File "${ROOT}WinBoard\logos\README.txt"

    ; Polyglot ini files; fruit.ini always supplied (even if Fruit not installed) as example
    SetOutPath "$INSTDIR\WinBoard\PG"
    File "${ROOT}WinBoard\PG\fruit.ini"

    ; bitmaps for board squares; xqboard is an entire (even-colored) board grid
    SetOutPath "$INSTDIR\Winboard\textures"
    File "${ROOT}WinBoard\textures\marble_d.bmp"
    File "${ROOT}WinBoard\textures\marble_l.bmp"
    File "${ROOT}WinBoard\textures\wood_d.bmp"
    File "${ROOT}WinBoard\textures\wood_l.bmp"
    File "${ROOT}WinBoard\textures\xqboard.bmp"

    StrCpy $FONT_DIR $FONTS
    !insertmacro InstallTTF '${FNTDIR}MARKFONT.TTF'
    SendMessage ${HWND_BROADCAST} ${WM_FONTCHANGE} 0 0 /TIMEOUT=5000

    ; the small ini files contain the command-line options used by the shortcuts
    SetOutPath "$INSTDIR\Winboard"
    File "${ROOT}Winboard\winboard.ini"
    File "${ROOT}Winboard\wood.ini"
    File "${ROOT}Winboard\marble.ini"
    File "${ROOT}WinBoard\ChessMark.ini"
    File "${ROOT}Winboard\fairy.ini"
    File "${ROOT}Winboard\FICS.ini"
    File "${ROOT}Winboard\fruit.ini"
    File "${ROOT}Winboard\Gothic.ini"
    File "${ROOT}Winboard\ICC.ini"
    File "${ROOT}Winboard\ICSbot.ini"
    File "${ROOT}Winboard\viewer.ini"
    File "${ROOT}Winboard\winboard.exe"
    File "${ROOT}Winboard\polyglot.exe"
    File "${ROOT}Winboard\timeseal.exe"
    File "${ROOT}Winboard\timestamp.exe"
    File "${ROOT}Winboard\winboard.chm"
    File "${ROOT}Winboard\winboard.hlp"
    File "${ROOT}Winboard\default_book.bin"

    ;Create uninstaller
    WriteUninstaller "$INSTDIR\UnInstall.exe"

      ; create some shortcuts in the WinBoard folder
	CreateShortCut "$INSTDIR\WinBoard\PGN Viewer.lnk" "$INSTDIR\WinBoard\winboard.exe" "@viewer" "$INSTDIR\WinBoard\winboard.exe" 0
	CreateShortCut "$INSTDIR\WinBoard\ICC.lnk" "$INSTDIR\WinBoard\winboard.exe" "@ICC" "$INSTDIR\WinBoard\winboard.exe" 0
	CreateShortCut "$INSTDIR\WinBoard\FICS.lnk" "$INSTDIR\WinBoard\winboard.exe" "@FICS" "$INSTDIR\WinBoard\winboard.exe" 0
	CreateShortCut "$INSTDIR\WinBoard\Fairy-Max ICS bot.lnk" "$INSTDIR\WinBoard\winboard.exe" "@ICSbot" "$INSTDIR\WinBoard\winboard.exe" 0
	CreateShortCut "$INSTDIR\WinBoard\my WinBoard.lnk" "$INSTDIR\WinBoard\winboard.exe" "@marble @ChessMark" "$INSTDIR\WinBoard\winboard.exe" 0
	CreateShortCut "$INSTDIR\WinBoard\Fruit.lnk" "$INSTDIR\WinBoard\winboard.exe" "@fruit" "$INSTDIR\WinBoard\winboard.exe" 0
	CreateShortCut "$INSTDIR\WinBoard\Fairy-Max.lnk" "$INSTDIR\WinBoard\winboard.exe" "@fairy" "$INSTDIR\WinBoard\winboard.exe" 0

	!insertmacro MUI_STARTMENU_WRITE_BEGIN Application
	CreateDirectory "$SMPROGRAMS\$START_MENU_FOLDER"
	CreateShortCut "$SMPROGRAMS\$START_MENU_FOLDER\Game Viewer.lnk" "$INSTDIR\WinBoard\winboard.exe" "@viewer" "$INSTDIR\WinBoard\winboard.exe" 1
	;CreateShortCut "$SMPROGRAMS\$START_MENU_FOLDER\WinBoard Help.lnk" "$INSTDIR\WinBoard\winboard.hlp"
	CreateShortCut "$SMPROGRAMS\$START_MENU_FOLDER\Frequently Asked Questions.lnk" "$INSTDIR\WinBoard\doc\FAQ.html"
	CreateShortCut "$SMPROGRAMS\$START_MENU_FOLDER\WinBoard Gold Pack README.lnk" "$INSTDIR\WinBoard\doc\README.html"
	CreateShortCut "$SMPROGRAMS\$START_MENU_FOLDER\WinBoard UnInstall.lnk" "$INSTDIR\UnInstall.exe"
	CreateShortCut "$SMPROGRAMS\$START_MENU_FOLDER\WinBoard Files.lnk" "$INSTDIR\WinBoard"
	CreateShortCut "$SMPROGRAMS\$START_MENU_FOLDER\Chess Server - chessclub.com.lnk" "$INSTDIR\WinBoard\winboard.exe"  "@ICC" "$INSTDIR\WinBoard\winboard.exe" 0
	CreateShortCut "$SMPROGRAMS\$START_MENU_FOLDER\Chess Server - freechess.org.lnk" "$INSTDIR\WinBoard\winboard.exe"  "@FICS" "$INSTDIR\WinBoard\winboard.exe" 0
	CreateShortCut "$SMPROGRAMS\$START_MENU_FOLDER\Fancy-Look WinBoard.lnk" "$INSTDIR\WinBoard\winboard.exe" "@marble @ChessMark" "$INSTDIR\WinBoard\winboard.exe" 2
	CreateShortCut "$SMPROGRAMS\$START_MENU_FOLDER\WinBoard Startup Dialog.lnk" "$INSTDIR\WinBoard\winboard.exe" "" "$INSTDIR\WinBoard\winboard.exe" 2
	!insertmacro MUI_STARTMENU_WRITE_END

SectionEnd

SectionGroup /e "Auxiliary Components and Engines" Profiles

    Section "Fairy-Max Demo Engine" fmax
        SectionIn 1 RO
        ; Fairy-Max is so small it can always be included, to have at least one working engine
        SetOutPath "$INSTDIR\Fairy-Max"
        File "${ROOT}Fairy-Max\fmax.exe"
        File "${ROOT}Fairy-Max\MaxQi.exe"
        File "${ROOT}Fairy-Max\fmax.ini"
        File "${ROOT}Fairy-Max\qmax.ini"
        File "${ROOT}Fairy-Max\logo.bmp"

        ; also create a menu item to play Xiangqi with MaxQi. It is put with the Chess Engines becase it uses western-style board
	  SetOutPath $INSTDIR\WinBoard
        CreateDirectory "$SMPROGRAMS\$START_MENU_FOLDER\Chess Engines"
        !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
        CreateShortCut "$SMPROGRAMS\$START_MENU_FOLDER\Chess Engines\Fairy-Max.lnk" "$INSTDIR\WinBoard\winboard.exe" "@fairy" "$INSTDIR\Fairy-Max\fmax.exe" 0
        CreateShortCut "$SMPROGRAMS\$START_MENU_FOLDER\Chess Engines\MaxQi (XQ).lnk" "$INSTDIR\WinBoard\winboard.exe" "@fairy -fcp MaxQi -scp MaxQi -variant xiangqi" "$INSTDIR\Fairy-Max\MaxQi.exe" 0
        !insertmacro MUI_STARTMENU_WRITE_END
    SectionEnd

    Section "Fruit 2.1" Fruit
        ; we include no separate book for Fruit, as it can use the GUI book. It is mostly included to provide a UCI example
        SetOutPath "$INSTDIR\Fruit"
        File "${ROOT}Fruit\copying.txt"
        File "${ROOT}Fruit\fruit_21.exe"
        File "${ROOT}Fruit\logo.bmp"
        File "${ROOT}Fruit\readme.txt"
        File "${ROOT}Fruit\technical_10.txt"

	  SetOutPath $INSTDIR\WinBoard
        CreateDirectory "$SMPROGRAMS\$START_MENU_FOLDER\Chess Engines"
        !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
        CreateShortCut "$SMPROGRAMS\$START_MENU_FOLDER\Chess Engines\Fruit 2.1.lnk" "$INSTDIR\WinBoard\winboard.exe" "@fruit" "$INSTDIR\WinBoard\winboard.exe" 2
        !insertmacro MUI_STARTMENU_WRITE_END
    SectionEnd

  Section "Tournament Manager" Tournaments
    SetOutPath "$INSTDIR\PSWBTM\doc"
    File "${ROOT}PSWBTM\doc\configure.html"
    File "${ROOT}PSWBTM\doc\install.html"
    File "${ROOT}PSWBTM\doc\running.html"
    File "${ROOT}PSWBTM\doc\tourney.html"
    File "${ROOT}PSWBTM\doc\menu.png"
    File "${ROOT}PSWBTM\doc\conf.png"
    File "${ROOT}PSWBTM\doc\eman.png"
    File "${ROOT}PSWBTM\doc\pswbtm.png"
    File "${ROOT}PSWBTM\doc\tour.png"

    CreateDirectory "$INSTDIR\PSWBTM\games"

    SetOutPath "$INSTDIR\PSWBTM\start positions"
    File "${ROOT}PSWBTM\start positions\nunn.pgn"
    File "${ROOT}PSWBTM\start positions\silver.epd"

    SetOutPath "$INSTDIR\PSWBTM"
    File "${ROOT}PSWBTM\config.pswbtm"
    File "${ROOT}PSWBTM\engines.pswbtm"
    File "${ROOT}PSWBTM\ntls.pswbtm"
    File "${ROOT}PSWBTM\PSWBTM.exe"
    File "${ROOT}PSWBTM\README.txt"
  SectionEnd

    SubSection "Xiangqi" Xiangqi

      Section "Graphics (required!)" XQgraphics
        ; the large bitmap of the wooden XQ board is optional, as is the XQ opening book
        SetOutPath "$INSTDIR\WinBoard\textures"
        File "${ROOT}WinBoard\textures\xqwood.bmp"
        
        StrCpy $FONT_DIR $FONTS
        !insertmacro InstallTTF '${FNTDIR}XIANGQI.TTF'
        SendMessage ${HWND_BROADCAST} ${WM_FONTCHANGE} 0 0 /TIMEOUT=5000

        SetOutPath "$INSTDIR\WinBoard"
        File "${ROOT}Winboard\xq.ini"
        File "${ROOT}Winboard\xq_book.bin"
        File "${ROOT}Winboard\UCCI2WB.exe"
        File "${ROOT}Winboard\QH2WB.exe"

        CreateDirectory "$SMPROGRAMS\$START_MENU_FOLDER\Xiangqi Engines"
        !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
        CreateShortCut "$SMPROGRAMS\$START_MENU_FOLDER\WinBoard\Xiangqi.lnk" "$INSTDIR\WinBoard\winboard.exe" "@xq" "$INSTDIR\WinBoard\UCCI2WB.exe" 0
        CreateShortCut "$SMPROGRAMS\$START_MENU_FOLDER\Xiangqi Engines\MaxQi.lnk" "$INSTDIR\WinBoard\winboard.exe" "@fairy @xq -fcp MaxQi -scp MaxQi" "$INSTDIR\Fairy-Max\MaxQi.exe" 0
        CreateShortCut "$SMPROGRAMS\$START_MENU_FOLDER\WinBoard XQ Startup (oriental).lnk" "$INSTDIR\WinBoard\winboard.exe" "@xq" "$INSTDIR\WinBoard\winboard.exe" 2
        !insertmacro MUI_STARTMENU_WRITE_END
      SectionEnd

      Section "HaQiKi D XQ-Engine" HaQi
        SetOutPath "$INSTDIR\HaQi"
        File "${ROOT}HaQi\haqikid.exe"
        File "${ROOT}HaQi\logo.bmp"
        CreateDirectory "$SMPROGRAMS\$START_MENU_FOLDER\Xiangqi Engines"

        SetOutPath $INSTDIR\WinBoard
        CreateDirectory "$SMPROGRAMS\$START_MENU_FOLDER\Xiangqi Engines"
        !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
        CreateShortCut "$SMPROGRAMS\$START_MENU_FOLDER\Xiangqi Engines\HaQiKi D.lnk" "$INSTDIR\WinBoard\winboard.exe" "@fairy @xq -fcp haqikid -fd ..\HaQi -scp haqikid -sd ..\HaQi" "$INSTDIR\HaQi\haqikid.exe" 0
        !insertmacro MUI_STARTMENU_WRITE_END
      SectionEnd

      Section "Elephant Eye XQ-Engine" EleEye
        SetOutPath "$INSTDIR\EleEye"
        File "${ROOT}EleEye\ATOM.DLL"
        File "${ROOT}EleEye\BOOK.DAT"
        File "${ROOT}EleEye\CCHESS.DLL"
        File "${ROOT}EleEye\ELEEYE.EXE"
        File "${ROOT}EleEye\EVALUATE.DLL"
        File "${ROOT}EleEye\logo.bmp"

        SetOutPath "$INSTDIR\WinBoard\QH"
        File "${ROOT}WinBoard\QH\eleeye.ini"

        CreateDirectory "$SMPROGRAMS\$START_MENU_FOLDER\Xiangqi Engines"
        SetOutPath $INSTDIR\WinBoard
        !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
        CreateShortCut "$SMPROGRAMS\$START_MENU_FOLDER\Xiangqi Engines\Elephant Eye.lnk" "$INSTDIR\WinBoard\winboard.exe" '@xq -cp -fcp "UCCI2WB QH\eleeye.ini" -firstLogo ..\EleEye\logo.bmp -scp "UCCI2WB QH\eleeye.ini -secondLogo ..\EleEye\logo.bmp"' "$INSTDIR\EleEye\ELEEYE.exe" 0
        !insertmacro MUI_STARTMENU_WRITE_END
      SectionEnd

    SubSectionEnd


    SubSection "Chess Variants" Variants

      Section "Pulsar Variant Engine (Mike Adams)" Pulsar
        SetOutPath "$INSTDIR\Pulsar"
        File "${ROOT}Pulsar\pulsar2009-9b.exe"
        File "${ROOT}Pulsar\bigbook.txt"
        File "${ROOT}Pulsar\openbk.txt"
        File "${ROOT}Pulsar\atomicBookBlack.txt"
        File "${ROOT}Pulsar\atomicBookWhite.txt"
        File "${ROOT}Pulsar\kingsBookBlack.txt"
        File "${ROOT}Pulsar\kingsBookWhite.txt"
        File "${ROOT}Pulsar\losersBlack.txt"
        File "${ROOT}Pulsar\losersWhite.txt"
        File "${ROOT}Pulsar\pulsarCrazyBlack.txt"
        File "${ROOT}Pulsar\pulsarCrazyWhite.txt"
        File "${ROOT}Pulsar\pulsarShatranjBlack.txt"
        File "${ROOT}Pulsar\pulsarShatranjWhite.txt"
        File "${ROOT}Pulsar\suicideBookBlack.txt"
        File "${ROOT}Pulsar\suicideBookWhite.txt"
        File "${ROOT}Pulsar\threeBookBlack.txt"
        File "${ROOT}Pulsar\threeBookWhite.txt"
        File "${ROOT}Pulsar\logo.bmp"

        SetOutPath $INSTDIR\WinBoard
        CreateDirectory "$SMPROGRAMS\$START_MENU_FOLDER\Chess Engines"
        !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
        CreateShortCut "$SMPROGRAMS\$START_MENU_FOLDER\Chess Engines\Pulsar.lnk" "$INSTDIR\WinBoard\winboard.exe" '@fairy -fcp "pulsar2009-9b 2" -fd ..\Pulsar -scp "pulsar2009-9b 2" -sd ..\Pulsar -usePolyglotBook false -variant atomic' "$INSTDIR\WinBoard\winboard.exe" 2
        !insertmacro MUI_STARTMENU_WRITE_END
      SectionEnd

      Section "Joker80 Variant Engine" Joker
        SetOutPath "$INSTDIR\Joker"
        File "${ROOT}Joker\joker80.exe"
        File "${ROOT}Joker\jokerKM.exe"
        File "${ROOT}Joker\logo.bmp"

        SetOutPath $INSTDIR\WinBoard
        CreateDirectory "$SMPROGRAMS\$START_MENU_FOLDER\Chess Engines"
        !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
        CreateShortCut "$SMPROGRAMS\$START_MENU_FOLDER\Chess Engines\Joker80 (Gothic).lnk" "$INSTDIR\WinBoard\winboard.exe" "@fairy -fcp Joker80.exe -fd ..\Joker -variant gothic" "$INSTDIR\Joker\Joker80.exe" 0
        CreateShortCut "$SMPROGRAMS\$START_MENU_FOLDER\Chess Engines\Joker Knightmate.lnk" "$INSTDIR\WinBoard\winboard.exe" "@fairy -fcp JokerKM.exe -fd ..\Joker -variant knightmate" "$INSTDIR\Joker\JokerKM.exe" 0
        !insertmacro MUI_STARTMENU_WRITE_END
      SectionEnd

      Section "Adapter for SMIRF Engine" Smirf
        SetOutPath $INSTDIR\SMIRF
        File "${ROOT}SMIRF\Smirfoglot.exe"
        File "${ROOT}SMIRF\logo.bmp"

        SetOutPath $INSTDIR\WinBoard
        CreateDirectory "$SMPROGRAMS\$START_MENU_FOLDER\Chess Engines"
        !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
        CreateShortCut "$SMPROGRAMS\$START_MENU_FOLDER\Chess Engines\SMIRF.lnk" "$INSTDIR\WinBoard\winboard.exe" "@fairy -fcp Smirfoglot.exe -fd ..\SMIRF" "$INSTDIR\SMIRF\Smirfoglot.exe" 0
        !insertmacro MUI_STARTMENU_WRITE_END
      SectionEnd

      Section "ShaMax Shatranj Engine" ShaMax
        SetOutPath $INSTDIR\Fairy-Max
        File ${ROOT}Fairy-Max\ShaMax.exe

        SetOutPath $INSTDIR\WinBoard
        !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
        CreateShortCut "$SMPROGRAMS\$START_MENU_FOLDER\Chess Engines\ShaMax.lnk" "$INSTDIR\WinBoard\winboard.exe" "@fairy -fcp ShaMax.exe -variant shatranj" "$INSTDIR\Fairy-Max\ShaMax.exe" 0
        !insertmacro MUI_STARTMENU_WRITE_END
      SectionEnd		

    SubSectionEnd
SectionGroupEnd

;Section "un.Fonts"
;    StrCpy $FONT_DIR $FONTS
;    !insertmacro RemoveTTF 'MARKFONT.TTF'
;    !insertmacro RemoveTTF 'XIANGI.TTF'
;    SendMessage ${HWND_BROADCAST} ${WM_FONTCHANGE} 0 0 /TIMEOUT=5000
;SectionEnd

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
    WriteRegStr HKCR "WinBoard.PGN\DefaultIcon" "" "$INSTDIR\WinBoard\WinBoard.exe,1"
    WriteRegStr HKCR "WinBoard.PGN\Shell\Open" "" "Open"
    WriteRegStr HKCR "WinBoard.PGN\Shell\Open\command" "" '"$INSTDIR\WinBoard\WinBoard.exe" -ini "$INSTDIR\WinBoard\WinBoard.ini" @viewer -lgf "%1"'

    CheckFEN:
    !insertmacro MUI_INSTALLOPTIONS_READ $INI_VALUE "FA.ini" "Field 3" "State"

    ;Display a messagebox if check box was checked
    StrCmp $INI_VALUE "1" "" Continue
    WriteRegStr HKCR ".fen" "" "WinBoard.FEN"
    WriteRegStr HKCR ".fen" "Content Type" "application/x-chess-fen"
    WriteRegStr HKCR "WinBoard.FEN" "" "Chess Position"
    WriteRegStr HKCR "WinBoard.FEN\DefaultIcon" "" "$INSTDIR\WinBoard\WinBoard.exe,1"
    WriteRegStr HKCR "WinBoard.FEN\Shell\Open" "" "Open"
    WriteRegStr HKCR "WinBoard.FEN\Shell\Open\command" "" '"$INSTDIR\WinBoard\WinBoard.exe" -ini "$INSTDIR\WinBoard\WinBoard.ini" @viewer -lpf "%1"'

    Continue:

FunctionEnd

;--------------------------------
;Descriptions

  ;Language strings

	LangString DESC_Core ${LANG_ENGLISH} "Winboard Core Components - Executable, Help Files, Protocol Adapters, Settings Files and Graphics"
	LangString DESC_Profiles ${LANG_ENGLISH} "Components only of Interest to Specific User Profiles"
	LangString DESC_Xiangqi ${LANG_ENGLISH} "Xiangqi (Chinese Chess) Engines and Graphics"
	LangString DESC_fmax ${LANG_ENGLISH} "Small Chess engine, also plays Gothic, Cylinder, Berolina, Capablanca, Superchess, Knightmate, Great Shatranj"
	LangString DESC_Fruit ${LANG_ENGLISH} "Very strong Chess engine suitable for analysis, by Fabien Letouzy"
	LangString DESC_Variants ${LANG_ENGLISH} "Engines for Chess-Variant Afficionados (e.g. Crazyhouse, Chess960, Gothic Chess)"
	LangString DESC_Tournaments ${LANG_ENGLISH} "PSWBTM Tournament Manager for running automated engine-engine tournaments with WinBoard"
	LangString DESC_XQgraphics ${LANG_ENGLISH} "Oriental-style board and pieces for WinBoard (the XQ-engine shortcuts won't work without it!)"
	LangString DESC_HaQi ${LANG_ENGLISH} "HaQiKi D 0.8, a strong Xiangqi engine by H.G. Muller"
	LangString DESC_EleEye ${LANG_ENGLISH} "Elephant Eye 3.1, a very strong Xiangqi engine by Morning Yellow"
	LangString DESC_Joker ${LANG_ENGLISH} "Joker80 Gothic-Chess engine and JokerKM Knightmate engine by H.G.Muller"
	LangString DESC_Pulsar ${LANG_ENGLISH} "Plays Chess960, Crazyhouse, Losers, Suicide, Giveway, Atomic, 3Check, TwoKings, Shatranj and standard Chess"
	LangString DESC_ShaMax ${LANG_ENGLISH} "A derivative of the Fairy-Max engine dedicated to playing Shatranj"
	LangString DESC_Smirf ${LANG_ENGLISH} "Smirfoglot adapter for Reinhard Scharnagl's SMIRF 10x8 and 8x8 Chess engine"


  ;Assign language strings to sections

  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
	!insertmacro MUI_DESCRIPTION_TEXT ${Core} $(DESC_Core)
	!insertmacro MUI_DESCRIPTION_TEXT ${Profiles} $(DESC_Profiles)
	!insertmacro MUI_DESCRIPTION_TEXT ${Xiangqi} $(DESC_Xiangqi)
	!insertmacro MUI_DESCRIPTION_TEXT ${fmax} $(DESC_fmax)
	!insertmacro MUI_DESCRIPTION_TEXT ${Fruit} $(DESC_Fruit)
	!insertmacro MUI_DESCRIPTION_TEXT ${Variants} $(DESC_Variants)
	!insertmacro MUI_DESCRIPTION_TEXT ${Tournaments} $(DESC_Tournaments)
	!insertmacro MUI_DESCRIPTION_TEXT ${XQgraphics} $(DESC_XQgraphics)
	!insertmacro MUI_DESCRIPTION_TEXT ${HaQi} $(DESC_HaQi)
	!insertmacro MUI_DESCRIPTION_TEXT ${EleEye} $(DESC_EleEye)
	!insertmacro MUI_DESCRIPTION_TEXT ${Joker} $(DESC_Joker)
	!insertmacro MUI_DESCRIPTION_TEXT ${Pulsar} $(DESC_Pulsar)
	!insertmacro MUI_DESCRIPTION_TEXT ${ShaMax} $(DESC_ShaMax)
	!insertmacro MUI_DESCRIPTION_TEXT ${Smirf} $(DESC_Smirf)
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

	Delete "$INSTDIR\WinBoard\PG\fruit.ini"
	Delete "$INSTDIR\WinBoard\logos\chessclub.com.bmp"
	Delete "$INSTDIR\WinBoard\logos\freechess.org.bmp"
	Delete "$INSTDIR\WinBoard\logos\administrator.bmp"
	Delete "$INSTDIR\WinBoard\logos\guest.bmp"
	Delete "$INSTDIR\WinBoard\logos\user.bmp"
	Delete "$INSTDIR\WinBoard\logos\README.txt"
	Delete "$INSTDIR\WinBoard\textures\marble_l.bmp"
	Delete "$INSTDIR\WinBoard\textures\marble_d.bmp"
	Delete "$INSTDIR\WinBoard\textures\wood_l.bmp"
	Delete "$INSTDIR\WinBoard\textures\wood_d.bmp"
	Delete "$INSTDIR\WinBoard\textures\xqboard.bmp"
	Delete "$INSTDIR\Fairy-Max\fmax.exe"
	Delete "$INSTDIR\Fairy-Max\MaxQi.exe"
	Delete "$INSTDIR\Fairy-Max\fmax.ini"
	Delete "$INSTDIR\Fairy-Max\qmax.ini"
	Delete "$INSTDIR\Fairy-Max\logo.bmp"
	Delete "$INSTDIR\WinBoard\doc\engine-intf.html"
	Delete "$INSTDIR\WinBoard\doc\FAQ.html"
	Delete "$INSTDIR\WinBoard\doc\fonts.html"
	Delete "$INSTDIR\WinBoard\doc\manual.html"
	Delete "$INSTDIR\WinBoard\doc\UCIconfig.html"
	Delete "$INSTDIR\WinBoard\doc\shortcuts.html"
	Delete "$INSTDIR\WinBoard\doc\texture.html"
	Delete "$INSTDIR\WinBoard\doc\mini.gif"
	Delete "$INSTDIR\WinBoard\doc\PG2fruit.png"
	Delete "$INSTDIR\WinBoard\doc\zippy.README"
	Delete "$INSTDIR\WinBoard\doc\COPYING.txt"
	Delete "$INSTDIR\WinBoard\doc\COPYRIGHT.txt"
	Delete "$INSTDIR\WinBoard\doc\COPYRIGHTS.txt"
	Delete "$INSTDIR\WinBoard\doc\README.html"
	;Delete "$FONTS\ChessMark.ttf"
	Delete "$INSTDIR\WinBoard\polyglot.exe"
	Delete "$INSTDIR\WinBoard\UCCI2WB.exe"
	Delete "$INSTDIR\WinBoard\timeseal.exe"
	Delete "$INSTDIR\WinBoard\timestamp.exe"
	Delete "$INSTDIR\WinBoard\winboard.exe"
	Delete "$INSTDIR\WinBoard\winboard.hlp"
	Delete "$INSTDIR\WinBoard\winboard.chm"
	Delete "$INSTDIR\WinBoard\FICS.ini"
	Delete "$INSTDIR\WinBoard\ICC.ini"
	Delete "$INSTDIR\WinBoard\fairy.ini"
	Delete "$INSTDIR\WinBoard\Gothic.ini"
	Delete "$INSTDIR\WinBoard\viewer.ini"
	Delete "$INSTDIR\WinBoard\marble.ini"
	Delete "$INSTDIR\WinBoard\wood.ini"
	Delete "$INSTDIR\WinBoard\ICSbot.ini"
	Delete "$INSTDIR\WinBoard\fruit.ini"
	Delete "$INSTDIR\WinBoard\winboard.ini"
	Delete "$INSTDIR\WinBoard\ChessMark.ini"
	Delete "$INSTDIR\WinBoard\default_book.bin"
	Delete "$INSTDIR\WinBoard\zippy.lines"
	Delete "$INSTDIR\WinBoard\textures\xqwood.bmp"
	Delete "$INSTDIR\WinBoard\FICS.lnk"
	Delete "$INSTDIR\WinBoard\ICC.lnk"
	Delete "$INSTDIR\WinBoard\Fairy-Max.lnk"
	Delete "$INSTDIR\WinBoard\PGN Viewer.lnk"
	Delete "$INSTDIR\WinBoard\my WinBoard.lnk"
	Delete "$INSTDIR\WinBoard\Fairy-Max ICS bot.lnk"
	Delete "$INSTDIR\WinBoard\Fruit.lnk"
	Delete "$INSTDIR\WinBoard\polyglot_1st.ini"
	Delete "$INSTDIR\WinBoard\polyglot_2nd.ini"

	Delete "$INSTDIR\Fruit\fruit_21.exe"
	Delete "$INSTDIR\Fruit\copying.txt"
	Delete "$INSTDIR\Fruit\readme.txt"
	Delete "$INSTDIR\Fruit\technical_10.txt"
	Delete "$INSTDIR\Fruit\logo.bmp"

	Delete "$INSTDIR\HaQi\haqikid.exe"
	Delete "$INSTDIR\HaQi\logo.bmp"
	Delete "$INSTDIR\EleEye\ELEEYE.exe"
	Delete "$INSTDIR\EleEye\ATOM.DLL"
	Delete "$INSTDIR\EleEye\CCHESS.DLL"
	Delete "$INSTDIR\EleEye\EVALUATE.DLL"
	Delete "$INSTDIR\EleEye\BOOK.DAT"
	Delete "$INSTDIR\EleEye\logo.bmp"
	Delete "$INSTDIR\WinBoard\QH\eleeye.ini"
	;Delete "$FONTS\XIANGQI.ttf"
	Delete "$INSTDIR\WinBoard\xq_book.bin"
	Delete "$INSTDIR\WinBoard\QH2WB.exe"
	Delete "$INSTDIR\WinBoard\xq.ini"

	Delete "$INSTDIR\Pulsar\Pulsar2009-9b.exe"
	Delete "$INSTDIR\Pulsar\atomicBookBlack.txt"
	Delete "$INSTDIR\Pulsar\atomicBookWhite.txt"
	Delete "$INSTDIR\Pulsar\kingsBookBlack.txt"
	Delete "$INSTDIR\Pulsar\kingsBookWhite.txt"
	Delete "$INSTDIR\Pulsar\losersBlack.txt"
	Delete "$INSTDIR\Pulsar\losersWhite.txt"
	Delete "$INSTDIR\Pulsar\pulsarCrazyBlack.txt"
	Delete "$INSTDIR\Pulsar\pulsarCrazyWhite.txt"
	Delete "$INSTDIR\Pulsar\pulsarShatranjBlack.txt"
	Delete "$INSTDIR\Pulsar\pulsarShatranjWhite.txt"
	Delete "$INSTDIR\Pulsar\suicideBookBlack.txt"
	Delete "$INSTDIR\Pulsar\suicideBookWhite.txt"
	Delete "$INSTDIR\Pulsar\threeBookBlack.txt"
	Delete "$INSTDIR\Pulsar\threeBookWhite.txt"
	Delete "$INSTDIR\Pulsar\bigbook.txt"
	Delete "$INSTDIR\Pulsar\openbk.txt"
	Delete "$INSTDIR\Pulsar\logo.bmp"
	Delete "$INSTDIR\Joker\joker80.exe"
	Delete "$INSTDIR\Joker\jokerKM.exe"
	Delete "$INSTDIR\Joker\logo.bmp"
	Delete "$INSTDIR\SMIRF\Smirfoglot.exe"
	Delete "$INSTDIR\SMIRF\logo.bmp"
	Delete "$INSTDIR\Fairy-Max\ShaMax.exe"

	Delete "$INSTDIR\PSWBTM\PSWBTM.exe"
	Delete "$INSTDIR\PSWBTM\README.txt"
	Delete "$INSTDIR\PSWBTM\config.pswbtm"
	Delete "$INSTDIR\PSWBTM\engines.pswbtm"
	Delete "$INSTDIR\PSWBTM\ntls.pswbtm"
	Delete "$INSTDIR\PSWBTM\start positions\nunn.pgn"
	Delete "$INSTDIR\PSWBTM\start positions\silver.epd"
	Delete "$INSTDIR\PSWBTM\doc\configure.html"
	Delete "$INSTDIR\PSWBTM\doc\install.html"
	Delete "$INSTDIR\PSWBTM\doc\running.html"
	Delete "$INSTDIR\PSWBTM\doc\tourney.html"
	Delete "$INSTDIR\PSWBTM\doc\conf.png"
	Delete "$INSTDIR\PSWBTM\doc\eman.png"
	Delete "$INSTDIR\PSWBTM\doc\menu.png"
	Delete "$INSTDIR\PSWBTM\doc\pswbtm.png"
	Delete "$INSTDIR\PSWBTM\doc\tour.png"

	RMDir "$INSTDIR\WinBoard\doc"
	RMDir "$INSTDIR\WinBoard\logos"
	RMDir "$INSTDIR\WinBoard\textures"
	RMDir "$INSTDIR\WinBoard\PG"
	RMDir "$INSTDIR\WinBoard\QH"
	RMDir "$INSTDIR\WinBoard"
	RMDir "$INSTDIR\Fairy-Max"
	RMDir "$INSTDIR\Pulsar"
	RMDir "$INSTDIR\Joker"
	RMDir "$INSTDIR\Fruit"
	RMDir "$INSTDIR\PSWBTM\doc"
	RMDir "$INSTDIR\PSWBTM\games"
	RMDir "$INSTDIR\PSWBTM\start positions"
	RMDir "$INSTDIR\PSWBTM"
	RMDir "$INSTDIR\SMIRF"
	RMDir "$INSTDIR\HaQi"
	RMDir "$INSTDIR\EleEye"
	Delete "$INSTDIR\uninstall.exe"
	RMDir "$INSTDIR"


    !insertmacro MUI_STARTMENU_GETFOLDER Application $MUI_TEMP

    RMDir /r "$SMPROGRAMS\$MUI_TEMP"

    ReadRegStr $1 HKCR ".pgn" ""
    StrCmp $1 "WinBoard.PGN" "" DelFEN
    ReadRegStr $1 HKCR "WinBoard.PGN\Shell\Open\command" ""
    StrCmp $1 '"$INSTDIR\WinBoard\WinBoard.exe" -ini "$INSTDIR\WinBoard\WinBoard.ini" @viewer -lgf "%1"' "" DelFEN
    DeleteRegKey HKCR ".pgn"
    DeleteRegKey HKCR "WinBoard.PGN"

    DelFEN:

    ReadRegStr $1 HKCR ".fen" ""
    StrCmp $1 "WinBoard.FEN" "" ContDelFEN
    ReadRegStr $1 HKCR "WinBoard.FEN\Shell\Open\command" ""
    StrCmp $1 '"$INSTDIR\WinBoard\WinBoard.exe" -ini "$INSTDIR\WinBoard\WinBoard.ini" @viewer -lpf "%1"' "" ContDelFEN
    DeleteRegKey HKCR ".fen"
    DeleteRegKey HKCR "WinBoard.FEN"

    ContDelFEN:

    DeleteRegKey HKCU "Software\WinBoard"


    IfFileExists "$INSTDIR\*.*" GoDirDel Continue

    GoDirDel:
    Call un.ForceDirectoryDelete

    Continue:


SectionEnd
