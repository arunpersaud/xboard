/*
 * WinBoard.c -- Windows NT front end to XBoard
 *
 * Copyright 1991 by Digital Equipment Corporation, Maynard,
 * Massachusetts.
 *
 * Enhancements Copyright 1992-2001, 2002, 2003, 2004, 2005, 2006,
 * 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014 Free Software Foundation, Inc.
 *
 * Enhancements Copyright 2005 Alessandro Scotti
 *
 * XBoard borrows its colors and the bitmaps.xchess bitmap set from XChess,
 * which was written and is copyrighted by Wayne Christopher.
 *
 * The following terms apply to Digital Equipment Corporation's copyright
 * interest in XBoard:
 * ------------------------------------------------------------------------
 * All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of Digital not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 * ------------------------------------------------------------------------
 *
 * The following terms apply to the enhanced version of XBoard
 * distributed by the Free Software Foundation:
 * ------------------------------------------------------------------------
 *
 * GNU XBoard is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 *
 * GNU XBoard is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.  *
 *
 *------------------------------------------------------------------------
 ** See the file ChangeLog for a revision history.  */

#include "config.h"

#include <windows.h>
#include <winuser.h>
#include <winsock.h>
#include <commctrl.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <malloc.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include <commdlg.h>
#include <dlgs.h>
#include <richedit.h>
#include <mmsystem.h>
#include <ctype.h>
#include <io.h>

#if __GNUC__
#include <errno.h>
#include <string.h>
#endif

#include "common.h"
#include "frontend.h"
#include "backend.h"
#include "winboard.h"
#include "moves.h"
#include "wclipbrd.h"
#include "woptions.h"
#include "wsockerr.h"
#include "defaults.h"
#include "help.h"
#include "wsnap.h"

#define SLASH '/'
#define DATADIR "~~"

//void InitEngineUCI( const char * iniDir, ChessProgramState * cps );

  int myrandom(void);
  void mysrandom(unsigned int seed);

extern int whiteFlag, blackFlag;
Boolean flipClock = FALSE;
extern HANDLE chatHandle[];
extern enum ICS_TYPE ics_type;

int  MySearchPath P((char *installDir, char *name, char *fullname));
int  MyGetFullPathName P((char *name, char *fullname));
void DisplayHoldingsCount(HDC hdc, int x, int y, int align, int copyNumber);
VOID NewVariantPopup(HWND hwnd);
int FinishMove P((ChessMove moveType, int fromX, int fromY, int toX, int toY,
		   /*char*/int promoChar));
void DisplayMove P((int moveNumber));
void ChatPopUp P((char *s));
typedef struct {
  ChessSquare piece;  
  POINT pos;      /* window coordinates of current pos */
  POINT lastpos;  /* window coordinates of last pos - used for clipping */
  POINT from;     /* board coordinates of the piece's orig pos */
  POINT to;       /* board coordinates of the piece's new pos */
} AnimInfo;

static AnimInfo animInfo = { EmptySquare, {-1,-1}, {-1,-1}, {-1,-1} };

typedef struct {
  POINT start;    /* window coordinates of start pos */
  POINT pos;      /* window coordinates of current pos */
  POINT lastpos;  /* window coordinates of last pos - used for clipping */
  POINT from;     /* board coordinates of the piece's orig pos */
  ChessSquare piece;
} DragInfo;

static DragInfo dragInfo = { {-1,-1}, {-1,-1}, {-1,-1}, {-1,-1}, EmptySquare };

typedef struct {
  POINT sq[2];	  /* board coordinates of from, to squares */
} HighlightInfo;

static HighlightInfo highlightInfo        = { {{-1, -1}, {-1, -1}} };
static HighlightInfo premoveHighlightInfo = { {{-1, -1}, {-1, -1}} };
static HighlightInfo partnerHighlightInfo = { {{-1, -1}, {-1, -1}} };
static HighlightInfo oldPartnerHighlight  = { {{-1, -1}, {-1, -1}} };

typedef struct { // [HGM] atomic
  int fromX, fromY, toX, toY, radius;
} ExplodeInfo;

static ExplodeInfo explodeInfo;

/* Window class names */
char szAppName[] = "WinBoard";
char szConsoleName[] = "WBConsole";

/* Title bar text */
char szTitle[] = "WinBoard";
char szConsoleTitle[] = "I C S Interaction";

char *programName;
char *settingsFileName;
Boolean saveSettingsOnExit;
char installDir[MSG_SIZ];
int errorExitStatus;

BoardSize boardSize;
Boolean chessProgram;
//static int boardX, boardY;
int  minX, minY; // [HGM] placement: volatile limits on upper-left corner
int squareSize, lineGap, minorSize;
static int winW, winH;
static RECT messageRect, whiteRect, blackRect, leftLogoRect, rightLogoRect; // [HGM] logo
static int logoHeight = 0;
static char messageText[MESSAGE_TEXT_MAX];
static int clockTimerEvent = 0;
static int loadGameTimerEvent = 0;
static int analysisTimerEvent = 0;
static DelayedEventCallback delayedTimerCallback;
static int delayedTimerEvent = 0;
static int buttonCount = 2;
char *icsTextMenuString;
char *icsNames;
char *firstChessProgramNames;
char *secondChessProgramNames;

#define PALETTESIZE 256

HINSTANCE hInst;          /* current instance */
Boolean alwaysOnTop = FALSE;
RECT boardRect;
COLORREF lightSquareColor, darkSquareColor, whitePieceColor, 
  blackPieceColor, highlightSquareColor, premoveHighlightColor;
COLORREF markerColor[8] = { 0x00FFFF, 0x0000FF, 0x00FF00, 0xFF0000, 0xFFFF00, 0xFF00FF, 0xFFFFFF, 0x000000 };
HPALETTE hPal;
ColorClass currentColorClass;

static HWND savedHwnd;
HWND hCommPort = NULL;    /* currently open comm port */
static HWND hwndPause;    /* pause button */
static HBITMAP pieceBitmap[3][(int) BlackPawn]; /* [HGM] nr of bitmaps referred to bP in stead of wK */
static HBRUSH lightSquareBrush, darkSquareBrush,
  blackSquareBrush, /* [HGM] for band between board and holdings */
  explodeBrush,     /* [HGM] atomic */
  markerBrush[8],   /* [HGM] markers */
  whitePieceBrush, blackPieceBrush, iconBkgndBrush /*, outlineBrush*/;
static POINT gridEndpoints[(BOARD_RANKS + BOARD_FILES + 2) * 2];
static DWORD gridVertexCounts[BOARD_RANKS + BOARD_FILES + 2];
static HPEN gridPen = NULL;
static HPEN highlightPen = NULL;
static HPEN premovePen = NULL;
static NPLOGPALETTE pLogPal;
static BOOL paletteChanged = FALSE;
static HICON iconWhite, iconBlack, iconCurrent;
static int doingSizing = FALSE;
static int lastSizing = 0;
static int prevStderrPort;
static HBITMAP userLogo;

static HBITMAP liteBackTexture = NULL;
static HBITMAP darkBackTexture = NULL;
static int liteBackTextureMode = BACK_TEXTURE_MODE_PLAIN;
static int darkBackTextureMode = BACK_TEXTURE_MODE_PLAIN;
static int backTextureSquareSize = 0;
static struct { int x; int y; int mode; } backTextureSquareInfo[BOARD_RANKS][BOARD_FILES];

#if __GNUC__ && !defined(_winmajor)
#define oldDialog 0 /* cygwin doesn't define _winmajor; mingw does */
#else

#if defined(_winmajor)
#define oldDialog (_winmajor < 4)
#else
#define oldDialog 0
#endif
#endif

#define INTERNATIONAL

#ifdef INTERNATIONAL
#  define _(s) T_(s)
#  define N_(s) s
#else
#  define _(s) s
#  define N_(s) s
#  define T_(s) s
#  define Translate(x, y)
#  define LoadLanguageFile(s)
#endif

#ifdef INTERNATIONAL

Boolean barbaric; // flag indicating if translation is needed

// list of item numbers used in each dialog (used to alter language at run time)

#define ABOUTBOX -1  /* not sure why these are needed */
#define ABOUTBOX2 -1

int dialogItems[][42] = {
{ ABOUTBOX, IDOK, OPT_MESS, 400 }, 
{ DLG_TimeControl, IDC_Babble, OPT_TCUseMoves, OPT_TCUseInc, OPT_TCUseFixed, 
  OPT_TCtext1, OPT_TCtext2, OPT_TCitext1, OPT_TCitext2, OPT_TCftext, GPB_Factors,   IDC_Factor1, IDC_Factor2, IDOK, IDCANCEL }, 
{ DLG_LoadOptions, OPT_Autostep, OPT_AStext1, OPT_Exact, OPT_Subset, OPT_Struct, OPT_Material, OPT_Range, OPT_Difference,
  OPT_elo1t, OPT_elo2t, OPT_datet, OPT_Stretch, OPT_Stretcht, OPT_Reversed, OPT_SearchMode, OPT_Mirror, OPT_thresholds,
  OPT_Ranget, IDOK, IDCANCEL }, 
{ DLG_SaveOptions, OPT_Autosave, OPT_AVPrompt, OPT_AVToFile, OPT_AVBrowse,
  801, OPT_PGN, OPT_Old, OPT_OutOfBookInfo, IDOK, IDCANCEL }, 
{ 1536, 1090, IDC_Directories, 1089, 1091, IDOK, IDCANCEL, 1038, IDC_IndexNr, 1037 }, 
{ DLG_CommPort, IDOK, IDCANCEL, IDC_Port, IDC_Rate, IDC_Bits, IDC_Parity,
  IDC_Stop, IDC_Flow, OPT_SerialHelp }, 
{ DLG_EditComment, IDOK, OPT_CancelComment, OPT_ClearComment, OPT_EditComment }, 
{ DLG_PromotionKing, PB_Chancellor, PB_Archbishop, PB_Queen, PB_Rook, 
  PB_Bishop, PB_Knight, PB_King, IDCANCEL, IDC_Yes, IDC_No, IDC_Centaur }, 
{ ABOUTBOX2, IDC_ChessBoard }, 
{ DLG_GameList, OPT_GameListLoad, OPT_GameListPrev, OPT_GameListNext, 
  OPT_GameListClose, IDC_GameListDoFilter }, 
{ DLG_EditTags, IDOK, OPT_TagsCancel, OPT_EditTags }, 
{ DLG_Error, IDOK }, 
{ DLG_Colorize, IDOK, IDCANCEL, OPT_ChooseColor, OPT_Bold, OPT_Italic,
  OPT_Underline, OPT_Strikeout, OPT_Sample }, 
{ DLG_Question, IDOK, IDCANCEL, OPT_QuestionText }, 
{ DLG_Startup, IDC_Welcome, OPT_ChessEngine, OPT_ChessServer, OPT_View,
  IDC_SPECIFY_ENG_STATIC, IDC_SPECIFY_SERVER_STATIC, OPT_AnyAdditional,
  IDOK, IDCANCEL, IDM_HELPCONTENTS }, 
{ DLG_IndexNumber, IDC_Index }, 
{ DLG_TypeInMove, IDOK, IDCANCEL }, 
{ DLG_TypeInName, IDOK, IDCANCEL }, 
{ DLG_Sound, IDC_Event, OPT_NoSound, OPT_DefaultBeep, OPT_BuiltInSound,
  OPT_WavFile, OPT_BrowseSound, OPT_DefaultSounds, IDOK, IDCANCEL, OPT_PlaySound }, 
{ DLG_GeneralOptions, IDOK, IDCANCEL, OPT_AlwaysOnTop, OPT_HighlightLastMove,
  OPT_AlwaysQueen, OPT_PeriodicUpdates, OPT_AnimateDragging, OPT_PonderNextMove,
  OPT_AnimateMoving, OPT_PopupExitMessage, OPT_AutoFlag, OPT_PopupMoveErrors,
  OPT_AutoFlipView, OPT_ShowButtonBar, OPT_AutoRaiseBoard, OPT_ShowCoordinates,
  OPT_Blindfold, OPT_ShowThinking, OPT_HighlightDragging, OPT_TestLegality,
  OPT_SaveExtPGN, OPT_HideThinkFromHuman, OPT_ExtraInfoInMoveHistory,
  OPT_HighlightMoveArrow, OPT_AutoLogo ,OPT_SmartMove }, 
{ DLG_IcsOptions, IDOK, IDCANCEL, OPT_AutoComment, OPT_AutoKibitz, OPT_AutoObserve,
  OPT_GetMoveList, OPT_LocalLineEditing, OPT_QuietPlay, OPT_SeekGraph, OPT_AutoRefresh,
  OPT_BgObserve, OPT_DualBoard, OPT_Premove, OPT_PremoveWhite, OPT_PremoveBlack,
  OPT_SmartMove, OPT_IcsAlarm, IDC_Sec, OPT_ChooseShoutColor, OPT_ChooseSShoutColor,
  OPT_ChooseChannel1Color, OPT_ChooseChannelColor, OPT_ChooseKibitzColor,
  OPT_ChooseTellColor, OPT_ChooseChallengeColor, OPT_ChooseRequestColor,
  OPT_ChooseSeekColor, OPT_ChooseNormalColor, OPT_ChooseBackgroundColor,
  OPT_DefaultColors, OPT_DontColorize, IDC_Boxes, GPB_Colors, GPB_Premove,
  GPB_General, GPB_Alarm, OPT_AutoCreate }, 
{ DLG_BoardOptions, IDOK, IDCANCEL, OPT_SizeTiny, OPT_SizeTeeny, OPT_SizeDinky,
  OPT_SizePetite, OPT_SizeSlim, OPT_SizeSmall, OPT_SizeMediocre, OPT_SizeMiddling,
  OPT_SizeAverage, OPT_SizeModerate, OPT_SizeMedium, OPT_SizeBulky, OPT_SizeLarge,
  OPT_SizeBig, OPT_SizeHuge, OPT_SizeGiant, OPT_SizeColossal, OPT_SizeTitanic,
  OPT_ChooseLightSquareColor, OPT_ChooseDarkSquareColor, OPT_ChooseWhitePieceColor,
  OPT_ChooseBlackPieceColor, OPT_ChooseHighlightSquareColor, OPT_ChoosePremoveHighlightColor,
  OPT_Monochrome, OPT_AllWhite, OPT_UpsideDown, OPT_DefaultBoardColors, GPB_Colors,
  IDC_Light, IDC_Dark, IDC_White, IDC_Black, IDC_High, IDC_PreHigh, GPB_Size, OPT_Bitmaps, OPT_PieceFont, OPT_Grid }, 
{ DLG_NewVariant, IDOK, IDCANCEL, OPT_VariantNormal, OPT_VariantFRC, OPT_VariantWildcastle,
  OPT_VariantNocastle, OPT_VariantLosers, OPT_VariantGiveaway, OPT_VariantSuicide,
  OPT_Variant3Check, OPT_VariantTwoKings, OPT_VariantAtomic, OPT_VariantCrazyhouse,
  OPT_VariantBughouse, OPT_VariantTwilight, OPT_VariantShogi, OPT_VariantSuper,
  OPT_VariantKnightmate, OPT_VariantBerolina, OPT_VariantCylinder, OPT_VariantFairy,
  OPT_VariantMakruk, OPT_VariantGothic, OPT_VariantCapablanca, OPT_VariantJanus,
  OPT_VariantCRC, OPT_VariantFalcon, OPT_VariantCourier, OPT_VariantGreat, OPT_VariantSChess,
  OPT_VariantShatranj, OPT_VariantXiangqi, GPB_Variant, GPB_Board, IDC_Height,
  IDC_Width, IDC_Hand, IDC_Pieces, IDC_Def }, 
{ DLG_Fonts, IDOK, IDCANCEL, OPT_ChooseClockFont, OPT_ChooseMessageFont,
  OPT_ChooseCoordFont, OPT_ChooseTagFont, OPT_ChooseCommentsFont,  OPT_ChooseConsoleFont, OPT_ChooseMoveHistoryFont, OPT_DefaultFonts,
  OPT_ClockFont, OPT_MessageFont, OPT_CoordFont, OPT_EditTagsFont, OPT_ChoosePieceFont, OPT_MessageFont8,
  OPT_SampleGameListFont, OPT_ChooseGameListFont, OPT_MessageFont7, 
  OPT_CommentsFont, OPT_MessageFont5, GPB_Current, GPB_All, OPT_MessageFont6 }, 
{ DLG_NewGameFRC, IDC_NFG_Label, IDC_NFG_Random, IDOK, IDCANCEL }, 
{ DLG_GameListOptions, IDC_GLT, IDC_GLT_Up, IDC_GLT_Down, IDC_GLT_Restore,
  IDC_GLT_Default, IDOK, IDCANCEL, IDC_GLT_RestoreTo }, 
{ DLG_MoveHistory }, 
{ DLG_EvalGraph }, 
{ DLG_EngineOutput, IDC_EngineLabel1, IDC_Engine1_NPS, IDC_EngineLabel2, IDC_Engine2_NPS }, 
{ DLG_Chat, IDC_Partner, IDC_Clear, IDC_Send,  }, 
{ DLG_EnginePlayOptions, IDC_EpPonder, IDC_EpShowThinking, IDC_EpHideThinkingHuman,
  IDC_EpPeriodicUpdates, GPB_Adjudications, IDC_Draw, IDC_Moves, IDC_Threshold,
  IDC_Centi, IDC_TestClaims, IDC_DetectMates, IDC_MaterialDraws, IDC_TrivialDraws,
  GPB_Apply, IDC_Rule, IDC_Repeats, IDC_ScoreAbs1, IDC_ScoreAbs2, IDOK, IDCANCEL }, 
{ DLG_OptionsUCI, IDC_PolyDir, IDC_BrowseForPolyglotDir, IDC_Hash, IDC_Path,
  IDC_BrowseForEGTB, IDC_Cache, IDC_UseBook, IDC_BrowseForBook, IDC_CPU, IDC_OwnBook1,
  IDC_OwnBook2, IDC_Depth, IDC_Variation, IDC_DefGames, IDOK, IDCANCEL },
{ 0 }
};

static char languageBuf[70000], *foreign[1000], *english[1000], *languageFile[MSG_SIZ];
static int lastChecked;
static char oldLanguage[MSG_SIZ], *menuText[10][30];
extern int tinyLayout;
extern char * menuBarText[][10];

void
LoadLanguageFile(char *name)
{   //load the file with translations, and make a list of the strings to be translated, and their translations
    FILE *f;
    int i=0, j=0, n=0, k;
    char buf[MSG_SIZ];

    if(!name || name[0] == NULLCHAR) return;
      snprintf(buf, MSG_SIZ, "%s%s", name, strchr(name, '.') ? "" : ".lng"); // auto-append lng extension
    appData.language = oldLanguage;
    if(!strcmp(buf, oldLanguage)) { barbaric = 1; return; } // this language already loaded; just switch on
    if((f = fopen(buf, "r")) == NULL) return;
    while((k = fgetc(f)) != EOF) {
        if(i >= sizeof(languageBuf)) { DisplayError("Language file too big", 0); return; }
        languageBuf[i] = k;
        if(k == '\n') {
            if(languageBuf[n] == '"' && languageBuf[i-1] == '"') {
                char *p;
                if(p = strstr(languageBuf + n + 1, "\" === \"")) {
                    if(p > languageBuf+n+2 && p+8 < languageBuf+i) {
                        if(j >= sizeof(english)) { DisplayError("Too many translated strings", 0); return; }
                        english[j] = languageBuf + n + 1; *p = 0;
                        foreign[j++] = p + 7; languageBuf[i-1] = 0;
//if(appData.debugMode) fprintf(debugFP, "translation: replace '%s' by '%s'\n", english[j-1], foreign[j-1]);
                    }
                }
            }
            n = i + 1;
        } else if(i > 0 && languageBuf[i-1] == '\\') {
            switch(k) {
              case 'n': k = '\n'; break;
              case 'r': k = '\r'; break;
              case 't': k = '\t'; break;
            }
            languageBuf[--i] = k;
        }
        i++;
    }
    fclose(f);
    barbaric = (j != 0);
    safeStrCpy(oldLanguage, buf, sizeof(oldLanguage)/sizeof(oldLanguage[0]) );
}

char *
T_(char *s)
{   // return the translation of the given string
    // efficiency can be improved a lot...
    int i=0;
    static char buf[MSG_SIZ];
//if(appData.debugMode) fprintf(debugFP, "T_(%s)\n", s);
    if(!barbaric) return s;
    if(!s) return ""; // sanity
    while(english[i]) {
        if(!strcmp(s, english[i])) return foreign[i];
	if(english[i][0] == '%' && strstr(s, english[i]+1) == s) { // allow translation of strings with variable ending
	    snprintf(buf, MSG_SIZ, "%s%s", foreign[i], s + strlen(english[i]+1)); // keep unmatched portion
	    return buf;
	}
        i++;
    }
    return s;
}

void
Translate(HWND hDlg, int dialogID)
{   // translate all text items in the given dialog
    int i=0, j, k;
    char buf[MSG_SIZ], *s;
    if(!barbaric) return;
    while(dialogItems[i][0] && dialogItems[i][0] != dialogID) i++; // find the dialog description
    if(dialogItems[i][0] != dialogID) return; // unknown dialog, should not happen
    GetWindowText( hDlg, buf, MSG_SIZ );
    s = T_(buf);
    if(strcmp(buf, s)) SetWindowText(hDlg, s); // replace by translated string (if different)
    for(j=1; k=dialogItems[i][j]; j++) { // translate all listed dialog items
        GetDlgItemText(hDlg, k, buf, MSG_SIZ);
        if(strlen(buf) == 0) continue;
        s = T_(buf);
        if(strcmp(buf, s)) SetDlgItemText(hDlg, k, s); // replace by translated string (if different)
    }
}

HMENU
TranslateOneMenu(int i, HMENU subMenu)
{
    int j;
    static MENUITEMINFO info;

    info.cbSize = sizeof(MENUITEMINFO);
    info.fMask = MIIM_STATE | MIIM_TYPE;
          for(j=GetMenuItemCount(subMenu)-1; j>=0; j--){
            char buf[MSG_SIZ];
            info.dwTypeData = buf;
            info.cch = sizeof(buf);
            GetMenuItemInfo(subMenu, j, TRUE, &info);
            if(i < 10) {
                if(menuText[i][j]) safeStrCpy(buf, menuText[i][j], sizeof(buf)/sizeof(buf[0]) );
                else menuText[i][j] = strdup(buf); // remember original on first change
            }
            if(buf[0] == NULLCHAR) continue;
            info.dwTypeData = T_(buf);
            info.cch = strlen(buf)+1;
            SetMenuItemInfo(subMenu, j, TRUE, &info);
          }
    return subMenu;
}

void
TranslateMenus(int addLanguage)
{
    int i;
    WIN32_FIND_DATA fileData;
    HANDLE hFind;
#define IDM_English 1970
    if(1) {
        HMENU mainMenu = GetMenu(hwndMain);
        for (i=GetMenuItemCount(mainMenu)-1; i>=0; i--) {
          HMENU subMenu = GetSubMenu(mainMenu, i);
          ModifyMenu(mainMenu, i, MF_STRING|MF_BYPOSITION|MF_POPUP|EnableMenuItem(mainMenu, i, MF_BYPOSITION),
                                                                  (UINT) subMenu, T_(menuBarText[tinyLayout][i]));
          TranslateOneMenu(i, subMenu);
        }
        DrawMenuBar(hwndMain);
    }

    if(!addLanguage) return;
    if((hFind = FindFirstFile("*.LNG", &fileData)) != INVALID_HANDLE_VALUE) {
        HMENU mainMenu = GetMenu(hwndMain);
        HMENU subMenu = GetSubMenu(mainMenu, GetMenuItemCount(mainMenu)-1);
        AppendMenu(subMenu, MF_SEPARATOR, (UINT_PTR) 0, NULL);
        AppendMenu(subMenu, MF_ENABLED|MF_STRING|(barbaric?MF_UNCHECKED:MF_CHECKED), (UINT_PTR) IDM_English, (LPCTSTR) "English");
        i = 0; lastChecked = IDM_English;
        do {
            char *p, *q = fileData.cFileName;
            int checkFlag = MF_UNCHECKED;
            languageFile[i] = strdup(q);
            if(barbaric && !strcmp(oldLanguage, q)) {
                checkFlag = MF_CHECKED;
                lastChecked = IDM_English + i + 1;
                CheckMenuItem(mainMenu, IDM_English, MF_BYCOMMAND|MF_UNCHECKED);
            }
            *q = ToUpper(*q); while(*++q) *q = ToLower(*q);
            p = strstr(fileData.cFileName, ".lng");
            if(p) *p = 0;
            AppendMenu(subMenu, MF_ENABLED|MF_STRING|checkFlag, (UINT_PTR) IDM_English + ++i, (LPCTSTR) fileData.cFileName);
        } while(FindNextFile(hFind, &fileData));
        FindClose(hFind);
    }
}

#endif

#define IDM_RecentEngines 3000

void
RecentEngineMenu (char *s)
{
    if(appData.icsActive) return;
    if(appData.recentEngines > 0 && *s) { // feature is on, and list non-empty
	HMENU mainMenu = GetMenu(hwndMain);
	HMENU subMenu = GetSubMenu(mainMenu, 5); // Engine menu
	int i=IDM_RecentEngines;
	recentEngines = strdup(appData.recentEngineList); // remember them as they are in menu
	AppendMenu(subMenu, MF_SEPARATOR, (UINT_PTR) 0, NULL);
	while(*s) {
	  char *p = strchr(s, '\n');
	  if(p == NULL) return; // malformed!
	  *p = NULLCHAR;
	  AppendMenu(subMenu, MF_ENABLED|MF_STRING|MF_UNCHECKED, (UINT_PTR) i++, (LPCTSTR) s);
	  *p = '\n';
	  s = p+1;
	}
    }
}


typedef struct {
  char *name;
  int squareSize;
  int lineGap;
  int smallLayout;
  int tinyLayout;
  int cliWidth, cliHeight;
} SizeInfo;

SizeInfo sizeInfo[] = 
{
  { "tiny",     21, 0, 1, 1, 0, 0 },
  { "teeny",    25, 1, 1, 1, 0, 0 },
  { "dinky",    29, 1, 1, 1, 0, 0 },
  { "petite",   33, 1, 1, 1, 0, 0 },
  { "slim",     37, 2, 1, 0, 0, 0 },
  { "small",    40, 2, 1, 0, 0, 0 },
  { "mediocre", 45, 2, 1, 0, 0, 0 },
  { "middling", 49, 2, 0, 0, 0, 0 },
  { "average",  54, 2, 0, 0, 0, 0 },
  { "moderate", 58, 3, 0, 0, 0, 0 },
  { "medium",   64, 3, 0, 0, 0, 0 },
  { "bulky",    72, 3, 0, 0, 0, 0 },
  { "large",    80, 3, 0, 0, 0, 0 },
  { "big",      87, 3, 0, 0, 0, 0 },
  { "huge",     95, 3, 0, 0, 0, 0 },
  { "giant",    108, 3, 0, 0, 0, 0 },
  { "colossal", 116, 4, 0, 0, 0, 0 },
  { "titanic",  129, 4, 0, 0, 0, 0 },
  { NULL, 0, 0, 0, 0, 0, 0 }
};

#define MF(x) {x, {{0,}, 0. }, {0, }, 0}
MyFont fontRec[NUM_SIZES][NUM_FONTS] =
{
  { MF(CLOCK_FONT_TINY), MF(MESSAGE_FONT_TINY), MF(COORD_FONT_TINY), MF(CONSOLE_FONT_TINY), MF(COMMENT_FONT_TINY), MF(EDITTAGS_FONT_TINY), MF(MOVEHISTORY_FONT_ALL), MF(GAMELIST_FONT_ALL) },
  { MF(CLOCK_FONT_TEENY), MF(MESSAGE_FONT_TEENY), MF(COORD_FONT_TEENY), MF(CONSOLE_FONT_TEENY), MF(COMMENT_FONT_TEENY), MF(EDITTAGS_FONT_TEENY), MF(MOVEHISTORY_FONT_ALL), MF(GAMELIST_FONT_ALL) },
  { MF(CLOCK_FONT_DINKY), MF(MESSAGE_FONT_DINKY), MF(COORD_FONT_DINKY), MF(CONSOLE_FONT_DINKY), MF(COMMENT_FONT_DINKY), MF(EDITTAGS_FONT_DINKY), MF(MOVEHISTORY_FONT_ALL), MF(GAMELIST_FONT_ALL) },
  { MF(CLOCK_FONT_PETITE), MF(MESSAGE_FONT_PETITE), MF(COORD_FONT_PETITE), MF(CONSOLE_FONT_PETITE), MF(COMMENT_FONT_PETITE), MF(EDITTAGS_FONT_PETITE), MF(MOVEHISTORY_FONT_ALL), MF(GAMELIST_FONT_ALL) },
  { MF(CLOCK_FONT_SLIM), MF(MESSAGE_FONT_SLIM), MF(COORD_FONT_SLIM), MF(CONSOLE_FONT_SLIM), MF(COMMENT_FONT_SLIM), MF(EDITTAGS_FONT_SLIM), MF(MOVEHISTORY_FONT_ALL), MF(GAMELIST_FONT_ALL) },
  { MF(CLOCK_FONT_SMALL), MF(MESSAGE_FONT_SMALL), MF(COORD_FONT_SMALL), MF(CONSOLE_FONT_SMALL), MF(COMMENT_FONT_SMALL), MF(EDITTAGS_FONT_SMALL), MF(MOVEHISTORY_FONT_ALL), MF(GAMELIST_FONT_ALL) },
  { MF(CLOCK_FONT_MEDIOCRE), MF(MESSAGE_FONT_MEDIOCRE), MF(COORD_FONT_MEDIOCRE), MF(CONSOLE_FONT_MEDIOCRE), MF(COMMENT_FONT_MEDIOCRE), MF(EDITTAGS_FONT_MEDIOCRE), MF(MOVEHISTORY_FONT_ALL), MF(GAMELIST_FONT_ALL) },
  { MF(CLOCK_FONT_MIDDLING), MF(MESSAGE_FONT_MIDDLING), MF(COORD_FONT_MIDDLING), MF(CONSOLE_FONT_MIDDLING), MF(COMMENT_FONT_MIDDLING), MF(EDITTAGS_FONT_MIDDLING), MF(MOVEHISTORY_FONT_ALL), MF(GAMELIST_FONT_ALL) },
  { MF(CLOCK_FONT_AVERAGE), MF(MESSAGE_FONT_AVERAGE), MF(COORD_FONT_AVERAGE), MF(CONSOLE_FONT_AVERAGE), MF(COMMENT_FONT_AVERAGE), MF(EDITTAGS_FONT_AVERAGE), MF(MOVEHISTORY_FONT_ALL), MF(GAMELIST_FONT_ALL) },
  { MF(CLOCK_FONT_MODERATE), MF(MESSAGE_FONT_MODERATE), MF(COORD_FONT_MODERATE), MF(CONSOLE_FONT_MODERATE), MF(COMMENT_FONT_MODERATE), MF(EDITTAGS_FONT_MODERATE), MF(MOVEHISTORY_FONT_ALL), MF(GAMELIST_FONT_ALL) },
  { MF(CLOCK_FONT_MEDIUM), MF(MESSAGE_FONT_MEDIUM), MF(COORD_FONT_MEDIUM), MF(CONSOLE_FONT_MEDIUM), MF(COMMENT_FONT_MEDIUM), MF(EDITTAGS_FONT_MEDIUM), MF(MOVEHISTORY_FONT_ALL),  MF(GAMELIST_FONT_ALL) },
  { MF(CLOCK_FONT_BULKY), MF(MESSAGE_FONT_BULKY), MF(COORD_FONT_BULKY), MF(CONSOLE_FONT_BULKY), MF(COMMENT_FONT_BULKY), MF(EDITTAGS_FONT_BULKY), MF(MOVEHISTORY_FONT_ALL),  MF(GAMELIST_FONT_ALL) },
  { MF(CLOCK_FONT_LARGE), MF(MESSAGE_FONT_LARGE), MF(COORD_FONT_LARGE), MF(CONSOLE_FONT_LARGE), MF(COMMENT_FONT_LARGE), MF(EDITTAGS_FONT_LARGE), MF(MOVEHISTORY_FONT_ALL),  MF(GAMELIST_FONT_ALL) },
  { MF(CLOCK_FONT_BIG), MF(MESSAGE_FONT_BIG), MF(COORD_FONT_BIG), MF(CONSOLE_FONT_BIG), MF(COMMENT_FONT_BIG), MF(EDITTAGS_FONT_BIG), MF(MOVEHISTORY_FONT_ALL),  MF(GAMELIST_FONT_ALL) },
  { MF(CLOCK_FONT_HUGE), MF(MESSAGE_FONT_HUGE), MF(COORD_FONT_HUGE), MF(CONSOLE_FONT_HUGE), MF(COMMENT_FONT_HUGE), MF(EDITTAGS_FONT_HUGE), MF(MOVEHISTORY_FONT_ALL), MF(GAMELIST_FONT_ALL) },
  { MF(CLOCK_FONT_GIANT), MF(MESSAGE_FONT_GIANT), MF(COORD_FONT_GIANT), MF(CONSOLE_FONT_GIANT), MF(COMMENT_FONT_GIANT), MF(EDITTAGS_FONT_GIANT), MF(MOVEHISTORY_FONT_ALL), MF(GAMELIST_FONT_ALL) },
  { MF(CLOCK_FONT_COLOSSAL), MF(MESSAGE_FONT_COLOSSAL), MF(COORD_FONT_COLOSSAL), MF(CONSOLE_FONT_COLOSSAL), MF(COMMENT_FONT_COLOSSAL), MF(EDITTAGS_FONT_COLOSSAL), MF(MOVEHISTORY_FONT_ALL), MF (GAMELIST_FONT_ALL) },
  { MF(CLOCK_FONT_TITANIC), MF(MESSAGE_FONT_TITANIC), MF(COORD_FONT_TITANIC), MF(CONSOLE_FONT_TITANIC), MF(COMMENT_FONT_TITANIC), MF(EDITTAGS_FONT_TITANIC), MF(MOVEHISTORY_FONT_ALL), MF(GAMELIST_FONT_ALL) },
};

MyFont *font[NUM_SIZES][NUM_FONTS];

typedef struct {
  char *label;
  int id;
  HWND hwnd;
  WNDPROC wndproc;
} MyButtonDesc;

#define BUTTON_WIDTH (tinyLayout ? 16 : 32)
#define N_BUTTONS 5

MyButtonDesc buttonDesc[N_BUTTONS] =
{
  {"<<", IDM_ToStart, NULL, NULL},
  {"<", IDM_Backward, NULL, NULL},
  {"P", IDM_Pause, NULL, NULL},
  {">", IDM_Forward, NULL, NULL},
  {">>", IDM_ToEnd, NULL, NULL},
};

int tinyLayout = 0, smallLayout = 0;
#define MENU_BAR_ITEMS 9
char *menuBarText[2][MENU_BAR_ITEMS+1] = {
  { N_("&File"), N_("&Edit"), N_("&View"), N_("&Mode"), N_("&Action"), N_("E&ngine"), N_("&Options"), N_("&Help"), NULL },
  { N_("&F"), N_("&E"), N_("&V"), N_("&M"), N_("&A"), N_("&N"), N_("&O"), N_("&H"), NULL },
};


MySound sounds[(int)NSoundClasses];
MyTextAttribs textAttribs[(int)NColorClasses];

MyColorizeAttribs colorizeAttribs[] = {
  { (COLORREF)0, 0, N_("Shout Text") },
  { (COLORREF)0, 0, N_("SShout/CShout") },
  { (COLORREF)0, 0, N_("Channel 1 Text") },
  { (COLORREF)0, 0, N_("Channel Text") },
  { (COLORREF)0, 0, N_("Kibitz Text") },
  { (COLORREF)0, 0, N_("Tell Text") },
  { (COLORREF)0, 0, N_("Challenge Text") },
  { (COLORREF)0, 0, N_("Request Text") },
  { (COLORREF)0, 0, N_("Seek Text") },
  { (COLORREF)0, 0, N_("Normal Text") },
  { (COLORREF)0, 0, N_("None") }
};



static char *commentTitle;
static char *commentText;
static int commentIndex;
static Boolean editComment = FALSE;


char errorTitle[MSG_SIZ];
char errorMessage[2*MSG_SIZ];
HWND errorDialog = NULL;
BOOLEAN moveErrorMessageUp = FALSE;
BOOLEAN consoleEcho = TRUE;
CHARFORMAT consoleCF;
COLORREF consoleBackgroundColor;

char *programVersion;

#define CPReal 1
#define CPComm 2
#define CPSock 3
#define CPRcmd 4
typedef int CPKind;

typedef struct {
  CPKind kind;
  HANDLE hProcess;
  DWORD pid;
  HANDLE hTo;
  HANDLE hFrom;
  SOCKET sock;
  SOCKET sock2;  /* stderr socket for OpenRcmd */
} ChildProc;

#define INPUT_SOURCE_BUF_SIZE 4096

typedef struct _InputSource {
  CPKind kind;
  HANDLE hFile;
  SOCKET sock;
  int lineByLine;
  HANDLE hThread;
  DWORD id;
  char buf[INPUT_SOURCE_BUF_SIZE];
  char *next;
  DWORD count;
  int error;
  InputCallback func;
  struct _InputSource *second;  /* for stderr thread on CPRcmd */
  VOIDSTAR closure;
} InputSource;

InputSource *consoleInputSource;

DCB dcb;

/* forward */
VOID ConsoleOutput(char* data, int length, int forceVisible);
VOID ConsoleCreate();
LRESULT CALLBACK
  ConsoleWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
VOID ColorizeTextPopup(HWND hwnd, ColorClass cc);
VOID PrintCommSettings(FILE *f, char *name, DCB *dcb);
VOID ParseCommSettings(char *arg, DCB *dcb);
LRESULT CALLBACK
  StartupDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
VOID APIENTRY MenuPopup(HWND hwnd, POINT pt, HMENU hmenu, UINT def);
void ParseIcsTextMenu(char *icsTextMenuString);
VOID PopUpNameDialog(char firstchar);
VOID UpdateSampleText(HWND hDlg, int id, MyColorizeAttribs *mca);

/* [AS] */
int NewGameFRC();
int GameListOptions();

int dummy; // [HGM] for obsolete args

HWND hwndMain = NULL;        /* root window*/
HWND hwndConsole = NULL;
HWND commentDialog = NULL;
HWND moveHistoryDialog = NULL;
HWND evalGraphDialog = NULL;
HWND engineOutputDialog = NULL;
HWND gameListDialog = NULL;
HWND editTagsDialog = NULL;

int commentUp = FALSE;

WindowPlacement wpMain;
WindowPlacement wpConsole;
WindowPlacement wpComment;
WindowPlacement wpMoveHistory;
WindowPlacement wpEvalGraph;
WindowPlacement wpEngineOutput;
WindowPlacement wpGameList;
WindowPlacement wpTags;

VOID EngineOptionsPopup(); // [HGM] settings

VOID GothicPopUp(char *title, VariantClass variant);
/*
 * Setting "frozen" should disable all user input other than deleting
 * the window.  We do this while engines are initializing themselves.
 */
static int frozen = 0;
static int oldMenuItemState[MENU_BAR_ITEMS];
void FreezeUI()
{
  HMENU hmenu;
  int i;

  if (frozen) return;
  frozen = 1;
  hmenu = GetMenu(hwndMain);
  for (i=0; i<MENU_BAR_ITEMS; i++) {
    oldMenuItemState[i] = EnableMenuItem(hmenu, i, MF_BYPOSITION|MF_GRAYED);
  }
  DrawMenuBar(hwndMain);
}

/* Undo a FreezeUI */
void ThawUI()
{
  HMENU hmenu;
  int i;

  if (!frozen) return;
  frozen = 0;
  hmenu = GetMenu(hwndMain);
  for (i=0; i<MENU_BAR_ITEMS; i++) {
    EnableMenuItem(hmenu, i, MF_BYPOSITION|oldMenuItemState[i]);
  }
  DrawMenuBar(hwndMain);
}

/*static*/ int fromX = -1, fromY = -1, toX, toY; // [HGM] moved upstream, so JAWS can use them

/* JAWS preparation patch (WinBoard for the sight impaired). Define required insertions as empty */
#ifdef JAWS
#include "jaws.c"
#else
#define JAWS_INIT
#define JAWS_ARGS
#define JAWS_ALT_INTERCEPT
#define JAWS_KBUP_NAVIGATION
#define JAWS_KBDOWN_NAVIGATION
#define JAWS_MENU_ITEMS
#define JAWS_SILENCE
#define JAWS_REPLAY
#define JAWS_ACCEL
#define JAWS_COPYRIGHT
#define JAWS_DELETE(X) X
#define SAYMACHINEMOVE()
#define SAY(X)
#endif

/*---------------------------------------------------------------------------*\
 *
 * WinMain
 *
\*---------------------------------------------------------------------------*/

static void HandleMessage P((MSG *message));
static HANDLE hAccelMain, hAccelNoAlt, hAccelNoICS;

int APIENTRY
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
  MSG msg;
//  INITCOMMONCONTROLSEX ex;

  debugFP = stderr;

  LoadLibrary("RICHED32.DLL");
  consoleCF.cbSize = sizeof(CHARFORMAT);

  if (!InitApplication(hInstance)) {
    return (FALSE);
  }
  if (!InitInstance(hInstance, nCmdShow, lpCmdLine)) {
    return (FALSE);
  }

  JAWS_INIT
  TranslateMenus(1);

//  InitCommonControlsEx(&ex);
  InitCommonControls();

  hAccelMain = LoadAccelerators (hInstance, szAppName);
  hAccelNoAlt = LoadAccelerators (hInstance, "NO_ALT");
  hAccelNoICS = LoadAccelerators( hInstance, "NO_ICS"); /* [AS] No Ctrl-V on ICS!!! */

  /* Acquire and dispatch messages until a WM_QUIT message is received. */

  while (GetMessage(&msg, /* message structure */
		    NULL, /* handle of window receiving the message */
		    0,    /* lowest message to examine */
		    0))   /* highest message to examine */
    {
	HandleMessage(&msg);
    }


  return (msg.wParam);	/* Returns the value from PostQuitMessage */
}

static void
HandleMessage (MSG *message)
{
    MSG msg = *message;

      if(msg.message == WM_CHAR && msg.wParam == '\t') {
	// [HGM] navigate: switch between all windows with tab
	HWND e1 = NULL, e2 = NULL, mh = NULL, hInput = NULL, hText = NULL;
	int i, currentElement = 0;

	// first determine what element of the chain we come from (if any)
	if(appData.icsActive) {
	    hInput = GetDlgItem(hwndConsole, OPT_ConsoleInput);
	    hText  = GetDlgItem(hwndConsole, OPT_ConsoleText);
	}
	if(engineOutputDialog && EngineOutputIsUp()) {
	    e1 = GetDlgItem(engineOutputDialog, IDC_EngineMemo1);
	    e2 = GetDlgItem(engineOutputDialog, IDC_EngineMemo2);
	}
	if(moveHistoryDialog && MoveHistoryIsUp()) {
	    mh = GetDlgItem(moveHistoryDialog, IDC_MoveHistory);
	}
	if(msg.hwnd == hwndMain) currentElement = 7 ; else
	if(msg.hwnd == engineOutputDialog) currentElement = 2; else
	if(msg.hwnd == e1)                 currentElement = 2; else
	if(msg.hwnd == e2)                 currentElement = 3; else
	if(msg.hwnd == moveHistoryDialog) currentElement = 4; else
	if(msg.hwnd == mh)                currentElement = 4; else
	if(msg.hwnd == evalGraphDialog)    currentElement = 6; else
	if(msg.hwnd == hText)  currentElement = 5; else
	if(msg.hwnd == hInput) currentElement = 6; else
	for (i = 0; i < N_BUTTONS; i++) {
	    if (buttonDesc[i].hwnd == msg.hwnd) { currentElement = 1; break; }
	}

	// determine where to go to
	if(currentElement) { HWND h = NULL; int direction = GetKeyState(VK_SHIFT) < 0 ? -1 : 1;
	  do {
	    currentElement = (currentElement + direction) % 7;
	    switch(currentElement) {
		case 0:
		  h = hwndMain; break; // passing this case always makes the loop exit
		case 1:
		  h = buttonDesc[0].hwnd; break; // could be NULL
		case 2:
		  if(!EngineOutputIsUp()) continue; // skip closed auxiliary windows
		  h = e1; break;
		case 3:
		  if(!EngineOutputIsUp()) continue;
		  h = e2; break;
		case 4:
		  if(!MoveHistoryIsUp()) continue;
		  h = mh; break;
//		case 6: // input to eval graph does not seem to get here!
//		  if(!EvalGraphIsUp()) continue;
//		  h = evalGraphDialog; break;
		case 5:
		  if(!appData.icsActive) continue;
		  SAY("display");
		  h = hText; break;
		case 6:
		  if(!appData.icsActive) continue;
		  SAY("input");
		  h = hInput; break;
	    }
	  } while(h == 0);

	  if(currentElement > 4 && IsIconic(hwndConsole)) ShowWindow(hwndConsole, SW_RESTORE);
	  if(currentElement < 5 && IsIconic(hwndMain))    ShowWindow(hwndMain, SW_RESTORE); // all open together
	  SetFocus(h);

	  return; // this message now has been processed
	}
      }

      if (!(commentDialog && IsDialogMessage(commentDialog, &msg)) &&
          !(moveHistoryDialog && IsDialogMessage(moveHistoryDialog, &msg)) &&
          !(evalGraphDialog && IsDialogMessage(evalGraphDialog, &msg)) &&
          !(engineOutputDialog && IsDialogMessage(engineOutputDialog, &msg)) &&
	  !(editTagsDialog && IsDialogMessage(editTagsDialog, &msg)) &&
	  !(gameListDialog && IsDialogMessage(gameListDialog, &msg)) &&
	  !(errorDialog && IsDialogMessage(errorDialog, &msg)) &&
	  !(!frozen && TranslateAccelerator(hwndMain, hAccelMain, &msg)) && JAWS_ACCEL
          !(!hwndConsole && TranslateAccelerator(hwndMain, hAccelNoICS, &msg)) &&
	  !(!hwndConsole && TranslateAccelerator(hwndMain, hAccelNoAlt, &msg))) {
	int done = 0, i; // [HGM] chat: dispatch cat-box messages
	for(i=0; i<MAX_CHAT; i++) 
	    if(chatHandle[i] && IsDialogMessage(chatHandle[i], &msg)) {
		done = 1; break;
	}
	if(done) return; // [HGM] chat: end patch
	TranslateMessage(&msg);	/* Translates virtual key codes */
	DispatchMessage(&msg);	/* Dispatches message to window */
      }
}

void
DoEvents ()
{ /* Dispatch pending messages */
  MSG msg;
  while (PeekMessage(&msg, /* message structure */
		     NULL, /* handle of window receiving the message */
		     0,    /* lowest message to examine */
		     0,    /* highest message to examine */
		     PM_REMOVE))
    {
	HandleMessage(&msg);
    }
}

/*---------------------------------------------------------------------------*\
 *
 * Initialization functions
 *
\*---------------------------------------------------------------------------*/

void
SetUserLogo()
{   // update user logo if necessary
    static char oldUserName[MSG_SIZ], dir[MSG_SIZ], *curName;

    if(appData.autoLogo) {
	  curName = UserName();
	  if(strcmp(curName, oldUserName)) {
		GetCurrentDirectory(MSG_SIZ, dir);
		SetCurrentDirectory(installDir);
		snprintf(oldUserName, MSG_SIZ, "logos\\%s.bmp", curName);
		userLogo = LoadImage( 0, oldUserName, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE );	
		safeStrCpy(oldUserName, curName, sizeof(oldUserName)/sizeof(oldUserName[0]) );
		if(userLogo == NULL)
		    userLogo = LoadImage( 0, "logos\\dummy.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE );	
		SetCurrentDirectory(dir); /* return to prev directory */
	  }
    }
}

BOOL
InitApplication(HINSTANCE hInstance)
{
  WNDCLASS wc;

  /* Fill in window class structure with parameters that describe the */
  /* main window. */

  wc.style         = CS_HREDRAW | CS_VREDRAW; /* Class style(s). */
  wc.lpfnWndProc   = (WNDPROC)WndProc;	/* Window Procedure */
  wc.cbClsExtra    = 0;			/* No per-class extra data. */
  wc.cbWndExtra    = 0;			/* No per-window extra data. */
  wc.hInstance     = hInstance;		/* Owner of this class */
  wc.hIcon         = LoadIcon(hInstance, "icon_white");
  wc.hCursor       = LoadCursor(NULL, IDC_ARROW);	/* Cursor */
  wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);	/* Default color */
  wc.lpszMenuName  = szAppName;			/* Menu name from .RC */
  wc.lpszClassName = szAppName;			/* Name to register as */

  /* Register the window class and return success/failure code. */
  if (!RegisterClass(&wc)) return FALSE;

  wc.style         = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc   = (WNDPROC)ConsoleWndProc;
  wc.cbClsExtra    = 0;
  wc.cbWndExtra    = DLGWINDOWEXTRA;
  wc.hInstance     = hInstance;
  wc.hIcon         = LoadIcon(hInstance, "icon_white");
  wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)(COLOR_MENU+1);
  wc.lpszMenuName  = NULL;
  wc.lpszClassName = szConsoleName;

  if (!RegisterClass(&wc)) return FALSE;
  return TRUE;
}


/* Set by InitInstance, used by EnsureOnScreen */
int screenHeight, screenWidth;
RECT screenGeometry;

void
EnsureOnScreen(int *x, int *y, int minX, int minY)
{
//  int gap = GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CYCAPTION);
  /* Be sure window at (x,y) is not off screen (or even mostly off screen) */
  if (*x > screenGeometry.right - 32) *x = screenGeometry.left;
  if (*y > screenGeometry.bottom - 32) *y = screenGeometry.top;
  if (*x < screenGeometry.left + minX) *x = screenGeometry.left + minX;
  if (*y < screenGeometry.top + minY) *y = screenGeometry.top + minY;
}

VOID
LoadLogo(ChessProgramState *cps, int n, Boolean ics)
{
  char buf[MSG_SIZ], dir[MSG_SIZ];
  GetCurrentDirectory(MSG_SIZ, dir);
  SetCurrentDirectory(installDir);
  if( appData.logo[n] && appData.logo[n][0] != NULLCHAR) {
      cps->programLogo = LoadImage( 0, appData.logo[n], IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE );

      if (cps->programLogo == NULL && appData.debugMode) {
          fprintf( debugFP, "Unable to load logo bitmap '%s'\n", appData.logo[n] );
      }
  } else if(appData.autoLogo) {
      if(ics) { // [HGM] logo: in ICS mode second can be used for ICS
	char *opponent = "";
	if(gameMode == IcsPlayingWhite) opponent = gameInfo.black;
	if(gameMode == IcsPlayingBlack) opponent = gameInfo.white;
	sprintf(buf, "logos\\%s\\%s.bmp", appData.icsHost, opponent);
	if(!*opponent || !(cps->programLogo = LoadImage( 0, buf, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE ))) {
	    sprintf(buf, "logos\\%s.bmp", appData.icsHost);
	    cps->programLogo = LoadImage( 0, buf, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE );
	}
      } else
      if(appData.directory[n] && appData.directory[n][0]) {
        SetCurrentDirectory(appData.directory[n]);
	cps->programLogo = LoadImage( 0, "logo.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE );	
      }
  }
  SetCurrentDirectory(dir); /* return to prev directory */
}

VOID
InitTextures()
{
  ZeroMemory( &backTextureSquareInfo, sizeof(backTextureSquareInfo) );
  backTextureSquareSize = 0; // kludge to force recalculation of texturemode
  
  if( appData.liteBackTextureFile && appData.liteBackTextureFile[0] != NULLCHAR && appData.liteBackTextureFile[0] != '*' ) {
      if(liteBackTexture) DeleteObject(liteBackTexture);
      liteBackTexture = LoadImage( 0, appData.liteBackTextureFile, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE );
      liteBackTextureMode = appData.liteBackTextureMode;

      if (liteBackTexture == NULL && appData.debugMode) {
          fprintf( debugFP, "Unable to load lite texture bitmap '%s'\n", appData.liteBackTextureFile );
      }
  }
  
  if( appData.darkBackTextureFile && appData.darkBackTextureFile[0] != NULLCHAR && appData.darkBackTextureFile[0] != '*' ) {
      if(darkBackTexture) DeleteObject(darkBackTexture);
      darkBackTexture = LoadImage( 0, appData.darkBackTextureFile, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE );
      darkBackTextureMode = appData.darkBackTextureMode;

      if (darkBackTexture == NULL && appData.debugMode) {
          fprintf( debugFP, "Unable to load dark texture bitmap '%s'\n", appData.darkBackTextureFile );
      }
  }
}

#ifndef SM_CXVIRTUALSCREEN
#define SM_CXVIRTUALSCREEN 78
#endif
#ifndef SM_CYVIRTUALSCREEN
#define SM_CYVIRTUALSCREEN 79
#endif
#ifndef SM_XVIRTUALSCREEN 
#define SM_XVIRTUALSCREEN 76
#endif
#ifndef SM_YVIRTUALSCREEN 
#define SM_YVIRTUALSCREEN 77
#endif

VOID
InitGeometry()
{
  screenHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);
  if( !screenHeight ) screenHeight = GetSystemMetrics(SM_CYSCREEN);
  screenWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
  if( !screenWidth ) screenWidth = GetSystemMetrics(SM_CXSCREEN);
  screenGeometry.left = GetSystemMetrics(SM_XVIRTUALSCREEN);
  screenGeometry.top = GetSystemMetrics(SM_YVIRTUALSCREEN);
  screenGeometry.right = screenGeometry.left + screenWidth;
  screenGeometry.bottom = screenGeometry.top + screenHeight;
}

BOOL
InitInstance(HINSTANCE hInstance, int nCmdShow, LPSTR lpCmdLine)
{
  HWND hwnd; /* Main window handle. */
  int ibs;
  WINDOWPLACEMENT wp;
  char *filepart;

  hInst = hInstance;	/* Store instance handle in our global variable */
  programName = szAppName;

  if (SearchPath(NULL, "WinBoard.exe", NULL, MSG_SIZ, installDir, &filepart)) {
    *filepart = NULLCHAR;
    SetCurrentDirectory(installDir);
  } else {
    GetCurrentDirectory(MSG_SIZ, installDir);
  }
  gameInfo.boardWidth = gameInfo.boardHeight = 8; // [HGM] won't have open window otherwise
  InitGeometry();
  InitAppData(lpCmdLine);      /* Get run-time parameters */
  /* xboard, and older WinBoards, controlled the move sound with the
     appData.ringBellAfterMoves option.  In the current WinBoard, we
     always turn the option on (so that the backend will call us),
     then let the user turn the sound off by setting it to silence if
     desired.  To accommodate old winboard.ini files saved by old
     versions of WinBoard, we also turn off the sound if the option
     was initially set to false. [HGM] taken out of InitAppData */
  if (!appData.ringBellAfterMoves) {
    sounds[(int)SoundMove].name = strdup("");
    appData.ringBellAfterMoves = TRUE;
  }
  if (appData.debugMode) {
    debugFP = fopen(appData.nameOfDebugFile, "w");
    setbuf(debugFP, NULL);
  }

  LoadLanguageFile(appData.language);

  InitBackEnd1();

//  InitEngineUCI( installDir, &first ); // [HGM] incorporated in InitBackEnd1()
//  InitEngineUCI( installDir, &second );

  /* Create a main window for this application instance. */
  hwnd = CreateWindow(szAppName, szTitle,
		      (WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX),
		      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
		      NULL, NULL, hInstance, NULL);
  hwndMain = hwnd;

  /* If window could not be created, return "failure" */
  if (!hwnd) {
    return (FALSE);
  }

  /* [HGM] logo: Load logos if specified (must be done before InitDrawingSizes) */
  LoadLogo(&first, 0, FALSE);
  LoadLogo(&second, 1, appData.icsActive);

  SetUserLogo();

  iconWhite = LoadIcon(hInstance, "icon_white");
  iconBlack = LoadIcon(hInstance, "icon_black");
  iconCurrent = iconWhite;
  InitDrawingColors();

  InitPosition(0); // to set nr of ranks and files, which might be non-default through command-line args
  for (ibs = (int) NUM_SIZES - 1; ibs >= 0; ibs--) {
    /* Compute window size for each board size, and use the largest
       size that fits on this screen as the default. */
    InitDrawingSizes((BoardSize)(ibs+1000), 0);
    if (boardSize == (BoardSize)-1 &&
        winH <= screenHeight
           - GetSystemMetrics(SM_CYFRAME) - GetSystemMetrics(SM_CYCAPTION) - 10
        && winW <= screenWidth) {
      boardSize = (BoardSize)ibs;
    }
  }

  InitDrawingSizes(boardSize, 0);
  RecentEngineMenu(appData.recentEngineList);
  InitMenuChecks();
  buttonCount = GetSystemMetrics(SM_CMOUSEBUTTONS);

  /* [AS] Load textures if specified */
  InitTextures();

  mysrandom( (unsigned) time(NULL) );

  /* [AS] Restore layout */
  if( wpMoveHistory.visible ) {
      MoveHistoryPopUp();
  }

  if( wpEvalGraph.visible ) {
      EvalGraphPopUp();
  }

  if( wpEngineOutput.visible ) {
      EngineOutputPopUp();
  }

  /* Make the window visible; update its client area; and return "success" */
  EnsureOnScreen(&wpMain.x, &wpMain.y, minX, minY);
  wp.length = sizeof(WINDOWPLACEMENT);
  wp.flags = 0;
  wp.showCmd = nCmdShow;
  wp.ptMaxPosition.x = wp.ptMaxPosition.y = 0;
  wp.rcNormalPosition.left = wpMain.x;
  wp.rcNormalPosition.right = wpMain.x + wpMain.width;
  wp.rcNormalPosition.top = wpMain.y;
  wp.rcNormalPosition.bottom = wpMain.y + wpMain.height;
  SetWindowPlacement(hwndMain, &wp);

  InitBackEnd2(); // [HGM] moved until after all windows placed, to save correct position if fatal error on engine start

  if(!appData.noGUI) SetWindowPos(hwndMain, alwaysOnTop ? HWND_TOPMOST : HWND_NOTOPMOST,
               0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);

  if (hwndConsole) {
#if AOT_CONSOLE
    SetWindowPos(hwndConsole, alwaysOnTop ? HWND_TOPMOST : HWND_NOTOPMOST,
                 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
#endif
    ShowWindow(hwndConsole, nCmdShow);
    SetActiveWindow(hwndConsole);
  }
  if(!appData.noGUI)   UpdateWindow(hwnd);  else ShowWindow(hwnd, SW_MINIMIZE);
  if(gameListDialog) SetFocus(gameListDialog); // [HGM] jaws: for if we clicked multi-game game file

  return TRUE;

}

VOID
InitMenuChecks()
{
  HMENU hmenu = GetMenu(hwndMain);

  (void) EnableMenuItem(hmenu, IDM_CommPort,
			MF_BYCOMMAND|((appData.icsActive &&
				       *appData.icsCommPort != NULLCHAR) ?
				      MF_ENABLED : MF_GRAYED));
  (void) CheckMenuItem(hmenu, IDM_SaveSettingsOnExit,
		       MF_BYCOMMAND|(saveSettingsOnExit ?
				     MF_CHECKED : MF_UNCHECKED));
  EnableMenuItem(hmenu, IDM_SaveSelected, MF_GRAYED);
}

//---------------------------------------------------------------------------------------------------------

#define ICS_TEXT_MENU_SIZE (IDM_CommandXLast - IDM_CommandX + 1)
#define XBOARD FALSE

#define OPTCHAR "/"
#define SEPCHAR "="
#define TOPLEVEL 0

#include "args.h"

// front-end part of option handling

VOID
LFfromMFP(LOGFONT* lf, MyFontParams *mfp)
{
  HDC hdc = CreateDC("DISPLAY", NULL, NULL, NULL);
  lf->lfHeight = -(int)(mfp->pointSize * GetDeviceCaps(hdc, LOGPIXELSY) / 72.0 + 0.5);
  DeleteDC(hdc);
  lf->lfWidth = 0;
  lf->lfEscapement = 0;
  lf->lfOrientation = 0;
  lf->lfWeight = mfp->bold ? FW_BOLD : FW_NORMAL;
  lf->lfItalic = mfp->italic;
  lf->lfUnderline = mfp->underline;
  lf->lfStrikeOut = mfp->strikeout;
  lf->lfCharSet = mfp->charset;
  lf->lfOutPrecision = OUT_DEFAULT_PRECIS;

  lf->lfClipPrecision = CLIP_DEFAULT_PRECIS;
  lf->lfQuality = DEFAULT_QUALITY;
  lf->lfPitchAndFamily = DEFAULT_PITCH|FF_DONTCARE;
    safeStrCpy(lf->lfFaceName, mfp->faceName, sizeof(lf->lfFaceName)/sizeof(lf->lfFaceName[0]) );
}

void
CreateFontInMF(MyFont *mf)
{ 
  LFfromMFP(&mf->lf, &mf->mfp);
  if (mf->hf) DeleteObject(mf->hf);
  mf->hf = CreateFontIndirect(&mf->lf);
}

// [HGM] This platform-dependent table provides the location for storing the color info
void *
colorVariable[] = {
  &whitePieceColor, 
  &blackPieceColor, 
  &lightSquareColor,
  &darkSquareColor, 
  &highlightSquareColor,
  &premoveHighlightColor,
  NULL,
  &consoleBackgroundColor,
  &appData.fontForeColorWhite,
  &appData.fontBackColorWhite,
  &appData.fontForeColorBlack,
  &appData.fontBackColorBlack,
  &appData.evalHistColorWhite,
  &appData.evalHistColorBlack,
  &appData.highlightArrowColor,
};

/* Command line font name parser.  NULL name means do nothing.
   Syntax like "Courier New:10.0 bi" or "Arial:10" or "Arial:10b"
   For backward compatibility, syntax without the colon is also
   accepted, but font names with digits in them won't work in that case.
*/
VOID
ParseFontName(char *name, MyFontParams *mfp)
{
  char *p, *q;
  if (name == NULL) return;
  p = name;
  q = strchr(p, ':');
  if (q) {
    if (q - p >= sizeof(mfp->faceName))
      ExitArgError(_("Font name too long:"), name, TRUE);
    memcpy(mfp->faceName, p, q - p);
    mfp->faceName[q - p] = NULLCHAR;
    p = q + 1;
  } else {
    q = mfp->faceName;

    while (*p && !isdigit(*p)) {
      *q++ = *p++;
      if (q - mfp->faceName >= sizeof(mfp->faceName))
	ExitArgError(_("Font name too long:"), name, TRUE);
    }
    while (q > mfp->faceName && q[-1] == ' ') q--;
    *q = NULLCHAR;
  }
  if (!*p) ExitArgError(_("Font point size missing:"), name, TRUE);
  mfp->pointSize = (float) atof(p);
  mfp->bold = (strchr(p, 'b') != NULL);
  mfp->italic = (strchr(p, 'i') != NULL);
  mfp->underline = (strchr(p, 'u') != NULL);
  mfp->strikeout = (strchr(p, 's') != NULL);
  mfp->charset = DEFAULT_CHARSET;
  q = strchr(p, 'c');
  if (q)
    mfp->charset = (BYTE) atoi(q+1);
}

void
ParseFont(char *name, int number)
{ // wrapper to shield back-end from 'font'
  ParseFontName(name, &font[boardSize][number]->mfp);
}

void
SetFontDefaults()
{ // in WB  we have a 2D array of fonts; this initializes their description
  int i, j;
  /* Point font array elements to structures and
     parse default font names */
  for (i=0; i<NUM_FONTS; i++) {
    for (j=0; j<NUM_SIZES; j++) {
      font[j][i] = &fontRec[j][i];
      ParseFontName(font[j][i]->def, &font[j][i]->mfp);
    }
  }
}

void
CreateFonts()
{ // here we create the actual fonts from the selected descriptions
  int i, j;
  for (i=0; i<NUM_FONTS; i++) {
    for (j=0; j<NUM_SIZES; j++) {
      CreateFontInMF(font[j][i]);
    }
  }
}
/* Color name parser.
   X version accepts X color names, but this one
   handles only the #rrggbb form (hex) or rrr,ggg,bbb (decimal) */
COLORREF
ParseColorName(char *name)
{
  int red, green, blue, count;
  char buf[MSG_SIZ];

  count = sscanf(name, "#%2x%2x%2x", &red, &green, &blue);
  if (count != 3) {
    count = sscanf(name, "%3d%*[^0-9]%3d%*[^0-9]%3d", 
      &red, &green, &blue);
  }
  if (count != 3) {
    snprintf(buf, MSG_SIZ, _("Can't parse color name %s"), name);
    DisplayError(buf, 0);
    return RGB(0, 0, 0);
  }
  return PALETTERGB(red, green, blue);
}

void
ParseColor(int n, char *name)
{ // for WinBoard the color is an int, which needs to be derived from the string
  if(colorVariable[n]) *(int*)colorVariable[n] = ParseColorName(name);
}

void
ParseAttribs(COLORREF *color, int *effects, char* argValue)
{
  char *e = argValue;
  int eff = 0;

  while (*e) {
    if (*e == 'b')      eff |= CFE_BOLD;
    else if (*e == 'i') eff |= CFE_ITALIC;
    else if (*e == 'u') eff |= CFE_UNDERLINE;
    else if (*e == 's') eff |= CFE_STRIKEOUT;
    else if (*e == '#' || isdigit(*e)) break;
    e++;
  }
  *effects = eff;
  *color   = ParseColorName(e);
}

void
ParseTextAttribs(ColorClass cc, char *s)
{   // [HGM] front-end wrapper that does the platform-dependent call
    // for XBoard we would set (&appData.colorShout)[cc] = strdup(s);
    ParseAttribs(&textAttribs[cc].color, &textAttribs[cc].effects, s);
}

void
ParseBoardSize(void *addr, char *name)
{ // [HGM] rewritten with return-value ptr to shield back-end from BoardSize
  BoardSize bs = SizeTiny;
  while (sizeInfo[bs].name != NULL) {
    if (StrCaseCmp(name, sizeInfo[bs].name) == 0) {
	*(BoardSize *)addr = bs;
	return;
    }
    bs++;
  }
  ExitArgError(_("Unrecognized board size value"), name, TRUE);
}

void
LoadAllSounds()
{ // [HGM] import name from appData first
  ColorClass cc;
  SoundClass sc;
  for (cc = (ColorClass)0; cc < ColorNormal; cc++) {
    textAttribs[cc].sound.name = strdup((&appData.soundShout)[cc]);
    textAttribs[cc].sound.data = NULL;
    MyLoadSound(&textAttribs[cc].sound);
  }
  for (cc = ColorNormal; cc < NColorClasses; cc++) {
    textAttribs[cc].sound.name = strdup("");
    textAttribs[cc].sound.data = NULL;
  }
  for (sc = (SoundClass)0; sc < NSoundClasses; sc++) {
    sounds[sc].name = strdup((&appData.soundMove)[sc]);
    sounds[sc].data = NULL;
    MyLoadSound(&sounds[sc]);
  }
}

void
SetCommPortDefaults()
{
   memset(&dcb, 0, sizeof(DCB)); // required by VS 2002 +
  dcb.DCBlength = sizeof(DCB);
  dcb.BaudRate = 9600;
  dcb.fBinary = TRUE;
  dcb.fParity = FALSE;
  dcb.fOutxCtsFlow = FALSE;
  dcb.fOutxDsrFlow = FALSE;
  dcb.fDtrControl = DTR_CONTROL_ENABLE;
  dcb.fDsrSensitivity = FALSE;
  dcb.fTXContinueOnXoff = TRUE;
  dcb.fOutX = FALSE;
  dcb.fInX = FALSE;
  dcb.fNull = FALSE;
  dcb.fRtsControl = RTS_CONTROL_ENABLE;
  dcb.fAbortOnError = FALSE;
  dcb.ByteSize = 7;
  dcb.Parity = SPACEPARITY;
  dcb.StopBits = ONESTOPBIT;
}

// [HGM] args: these three cases taken out to stay in front-end
void
SaveFontArg(FILE *f, ArgDescriptor *ad)
{	// in WinBoard every board size has its own font, and the "argLoc" identifies the table,
	// while the curent board size determines the element. This system should be ported to XBoard.
	// What the table contains pointers to, and how to print the font description, remains platform-dependent
        int bs;
	for (bs=0; bs<NUM_SIZES; bs++) {
	  MyFontParams *mfp = &font[bs][(int) ad->argLoc]->mfp;
          fprintf(f, "/size=%s ", sizeInfo[bs].name);
	  fprintf(f, "/%s=\"%s:%g%s%s%s%s%sc%d\"\n",
	    ad->argName, mfp->faceName, mfp->pointSize,
            mfp->bold || mfp->italic || mfp->underline || mfp->strikeout ? " " : "",
	    mfp->bold ? "b" : "",
	    mfp->italic ? "i" : "",
	    mfp->underline ? "u" : "",
	    mfp->strikeout ? "s" : "",
            (int)mfp->charset);
	}
      }

void
ExportSounds()
{ // [HGM] copy the names from the internal WB variables to appData
  ColorClass cc;
  SoundClass sc;
  for (cc = (ColorClass)0; cc < ColorNormal; cc++)
    (&appData.soundShout)[cc] = textAttribs[cc].sound.name;
  for (sc = (SoundClass)0; sc < NSoundClasses; sc++)
    (&appData.soundMove)[sc] = sounds[sc].name;
}

void
SaveAttribsArg(FILE *f, ArgDescriptor *ad)
{	// here the "argLoc" defines a table index. It could have contained the 'ta' pointer itself, though
	MyTextAttribs* ta = &textAttribs[(ColorClass)ad->argLoc];
	fprintf(f, "/%s=\"%s%s%s%s%s#%02lx%02lx%02lx\"\n", ad->argName,
          (ta->effects & CFE_BOLD) ? "b" : "",
          (ta->effects & CFE_ITALIC) ? "i" : "",
          (ta->effects & CFE_UNDERLINE) ? "u" : "",
          (ta->effects & CFE_STRIKEOUT) ? "s" : "",
          (ta->effects) ? " " : "",
	  ta->color&0xff, (ta->color >> 8)&0xff, (ta->color >> 16)&0xff);
      }

void
SaveColor(FILE *f, ArgDescriptor *ad)
{	// in WinBoard the color is an int and has to be converted to text. In X it would be a string already?
	COLORREF color = *(COLORREF *)colorVariable[(int)ad->argLoc];
	fprintf(f, "/%s=#%02lx%02lx%02lx\n", ad->argName, 
	  color&0xff, (color>>8)&0xff, (color>>16)&0xff);
}

void
SaveBoardSize(FILE *f, char *name, void *addr)
{ // wrapper to shield back-end from BoardSize & sizeInfo
  fprintf(f, "/%s=%s\n", name, sizeInfo[*(BoardSize *)addr].name);
}

void
ParseCommPortSettings(char *s)
{ // wrapper to keep dcb from back-end
  ParseCommSettings(s, &dcb);
}

void
GetWindowCoords()
{ // wrapper to shield use of window handles from back-end (make addressible by number?)
  GetActualPlacement(hwndMain, &wpMain);
  GetActualPlacement(hwndConsole, &wpConsole);
  GetActualPlacement(commentDialog, &wpComment);
  GetActualPlacement(editTagsDialog, &wpTags);
  GetActualPlacement(gameListDialog, &wpGameList);
  GetActualPlacement(moveHistoryDialog, &wpMoveHistory);
  GetActualPlacement(evalGraphDialog, &wpEvalGraph);
  GetActualPlacement(engineOutputDialog, &wpEngineOutput);
}

void
PrintCommPortSettings(FILE *f, char *name)
{ // wrapper to shield back-end from DCB
      PrintCommSettings(f, name, &dcb);
}

int
MySearchPath(char *installDir, char *name, char *fullname)
{
  char *dummy, buf[MSG_SIZ], *p = name, *q;
  if(name[0]== '%') {
    fullname[0] = 0; // [HGM] first expand any environment variables in the given name
    while(*p == '%' && (q = strchr(p+1, '%'))) { // [HGM] recognize %*% as environment variable
      safeStrCpy(buf, p+1, sizeof(buf)/sizeof(buf[0]) );
      *strchr(buf, '%') = 0;
      strcat(fullname, getenv(buf));
      p = q+1; while(*p == '\\') { strcat(fullname, "\\"); p++; }
    }
    strcat(fullname, p); // after environment variables (if any), take the remainder of the given name
    if(appData.debugMode) fprintf(debugFP, "name = '%s', expanded name = '%s'\n", name, fullname);
    return (int) strlen(fullname);
  }
  return (int) SearchPath(installDir, name, NULL, MSG_SIZ, fullname, &dummy);
}

int
MyGetFullPathName(char *name, char *fullname)
{
  char *dummy;
  return (int) GetFullPathName(name, MSG_SIZ, fullname, &dummy);
}

int
MainWindowUp()
{ // [HGM] args: allows testing if main window is realized from back-end
  return hwndMain != NULL;
}

void
PopUpStartupDialog()
{
    FARPROC lpProc;
    
    LoadLanguageFile(appData.language);
    lpProc = MakeProcInstance((FARPROC)StartupDialog, hInst);
    DialogBox(hInst, MAKEINTRESOURCE(DLG_Startup), NULL, (DLGPROC)lpProc);
    FreeProcInstance(lpProc);
}

/*---------------------------------------------------------------------------*\
 *
 * GDI board drawing routines
 *
\*---------------------------------------------------------------------------*/

/* [AS] Draw square using background texture */
static void DrawTile( int dx, int dy, int dw, int dh, HDC dst, HDC src, int mode, int sx, int sy )
{
    XFORM   x;

    if( mode == 0 ) {
        return; /* Should never happen! */
    }

    SetGraphicsMode( dst, GM_ADVANCED );

    switch( mode ) {
    case 1:
        /* Identity */
        break;
    case 2:
        /* X reflection */
        x.eM11 = -1.0;
        x.eM12 = 0;
        x.eM21 = 0;
        x.eM22 = 1.0;
        x.eDx = (FLOAT) dw + dx - 1;
        x.eDy = 0;
        dx = 0;
        SetWorldTransform( dst, &x );
        break;
    case 3:
        /* Y reflection */
        x.eM11 = 1.0;
        x.eM12 = 0;
        x.eM21 = 0;
        x.eM22 = -1.0;
        x.eDx = 0;
        x.eDy = (FLOAT) dh + dy - 1;
        dy = 0;
        SetWorldTransform( dst, &x );
        break;
    case 4:
        /* X/Y flip */
        x.eM11 = 0;
        x.eM12 = 1.0;
        x.eM21 = 1.0;
        x.eM22 = 0;
        x.eDx = (FLOAT) dx;
        x.eDy = (FLOAT) dy;
        dx = 0;
        dy = 0;
        SetWorldTransform( dst, &x );
        break;
    }

    BitBlt( dst, dx, dy, dw, dh, src, sx, sy, SRCCOPY );

    x.eM11 = 1.0;
    x.eM12 = 0;
    x.eM21 = 0;
    x.eM22 = 1.0;
    x.eDx = 0;
    x.eDy = 0;
    SetWorldTransform( dst, &x );

    ModifyWorldTransform( dst, 0, MWT_IDENTITY );
}

/* [AS] [HGM] Make room for more piece types, so all pieces can be different */
enum {
    PM_WP = (int) WhitePawn, 
    PM_WN = (int) WhiteKnight, 
    PM_WB = (int) WhiteBishop, 
    PM_WR = (int) WhiteRook, 
    PM_WQ = (int) WhiteQueen, 
    PM_WF = (int) WhiteFerz, 
    PM_WW = (int) WhiteWazir, 
    PM_WE = (int) WhiteAlfil, 
    PM_WM = (int) WhiteMan, 
    PM_WO = (int) WhiteCannon, 
    PM_WU = (int) WhiteUnicorn, 
    PM_WH = (int) WhiteNightrider, 
    PM_WA = (int) WhiteAngel, 
    PM_WC = (int) WhiteMarshall, 
    PM_WAB = (int) WhiteCardinal, 
    PM_WD = (int) WhiteDragon, 
    PM_WL = (int) WhiteLance, 
    PM_WS = (int) WhiteCobra, 
    PM_WV = (int) WhiteFalcon, 
    PM_WSG = (int) WhiteSilver, 
    PM_WG = (int) WhiteGrasshopper, 
    PM_WK = (int) WhiteKing,
    PM_BP = (int) BlackPawn, 
    PM_BN = (int) BlackKnight, 
    PM_BB = (int) BlackBishop, 
    PM_BR = (int) BlackRook, 
    PM_BQ = (int) BlackQueen, 
    PM_BF = (int) BlackFerz, 
    PM_BW = (int) BlackWazir, 
    PM_BE = (int) BlackAlfil, 
    PM_BM = (int) BlackMan,
    PM_BO = (int) BlackCannon, 
    PM_BU = (int) BlackUnicorn, 
    PM_BH = (int) BlackNightrider, 
    PM_BA = (int) BlackAngel, 
    PM_BC = (int) BlackMarshall, 
    PM_BG = (int) BlackGrasshopper, 
    PM_BAB = (int) BlackCardinal,
    PM_BD = (int) BlackDragon,
    PM_BL = (int) BlackLance,
    PM_BS = (int) BlackCobra,
    PM_BV = (int) BlackFalcon,
    PM_BSG = (int) BlackSilver,
    PM_BK = (int) BlackKing
};

static HFONT hPieceFont = NULL;
static HBITMAP hPieceMask[(int) EmptySquare];
static HBITMAP hPieceFace[(int) EmptySquare];
static int fontBitmapSquareSize = 0;
static char pieceToFontChar[(int) EmptySquare] =
                              { 'p', 'n', 'b', 'r', 'q', 
                      'n', 'b', 'p', 'n', 'b', 'r', 'b', 'r', 'q', 'k',
                      'k', 'o', 'm', 'v', 't', 'w', 
                      'v', 't', 'o', 'm', 'v', 't', 'v', 't', 'w', 'l',
                                                              'l' };

extern BOOL SetCharTable( char *table, const char * map );
/* [HGM] moved to backend.c */

static void SetPieceBackground( HDC hdc, COLORREF color, int mode )
{
    HBRUSH hbrush;
    BYTE r1 = GetRValue( color );
    BYTE g1 = GetGValue( color );
    BYTE b1 = GetBValue( color );
    BYTE r2 = r1 / 2;
    BYTE g2 = g1 / 2;
    BYTE b2 = b1 / 2;
    RECT rc;

    /* Create a uniform background first */
    hbrush = CreateSolidBrush( color );
    SetRect( &rc, 0, 0, squareSize, squareSize );
    FillRect( hdc, &rc, hbrush );
    DeleteObject( hbrush );
    
    if( mode == 1 ) {
        /* Vertical gradient, good for pawn, knight and rook, less for queen and king */
        int steps = squareSize / 2;
        int i;

        for( i=0; i<steps; i++ ) {
            BYTE r = r1 - (r1-r2) * i / steps;
            BYTE g = g1 - (g1-g2) * i / steps;
            BYTE b = b1 - (b1-b2) * i / steps;

            hbrush = CreateSolidBrush( RGB(r,g,b) );
            SetRect( &rc, i + squareSize - steps, 0, i + squareSize - steps + 1, squareSize );
            FillRect( hdc, &rc, hbrush );
            DeleteObject(hbrush);
        }
    }
    else if( mode == 2 ) {
        /* Diagonal gradient, good more or less for every piece */
        POINT triangle[3];
        HPEN hpen = SelectObject( hdc, GetStockObject(NULL_PEN) );
        HBRUSH hbrush_old;
        int steps = squareSize;
        int i;

        triangle[0].x = squareSize - steps;
        triangle[0].y = squareSize;
        triangle[1].x = squareSize;
        triangle[1].y = squareSize;
        triangle[2].x = squareSize;
        triangle[2].y = squareSize - steps;

        for( i=0; i<steps; i++ ) {
            BYTE r = r1 - (r1-r2) * i / steps;
            BYTE g = g1 - (g1-g2) * i / steps;
            BYTE b = b1 - (b1-b2) * i / steps;

            hbrush = CreateSolidBrush( RGB(r,g,b) );
            hbrush_old = SelectObject( hdc, hbrush );
            Polygon( hdc, triangle, 3 );
            SelectObject( hdc, hbrush_old );
            DeleteObject(hbrush);
            triangle[0].x++;
            triangle[2].y++;
        }

        SelectObject( hdc, hpen );
    }
}

/*
    [AS] The method I use to create the bitmaps it a bit tricky, but it
    seems to work ok. The main problem here is to find the "inside" of a chess
    piece: follow the steps as explained below.
*/
static void CreatePieceMaskFromFont( HDC hdc_window, HDC hdc, int index )
{
    HBITMAP hbm;
    HBITMAP hbm_old;
    COLORREF chroma = RGB(0xFF,0x00,0xFF);
    RECT rc;
    SIZE sz;


    POINT pt;
    int backColor = whitePieceColor; 
    int foreColor = blackPieceColor;
    
    if( index < (int)BlackPawn && appData.fontBackColorWhite != appData.fontForeColorWhite ) {
        backColor = appData.fontBackColorWhite;
        foreColor = appData.fontForeColorWhite;
    }
    else if( index >= (int)BlackPawn && appData.fontBackColorBlack != appData.fontForeColorBlack ) {
        backColor = appData.fontBackColorBlack;
        foreColor = appData.fontForeColorBlack;
    }

    /* Mask */
    hbm = CreateCompatibleBitmap( hdc_window, squareSize, squareSize );

    hbm_old = SelectObject( hdc, hbm );

    rc.left = 0;
    rc.top = 0;
    rc.right = squareSize;
    rc.bottom = squareSize;

    /* Step 1: background is now black */
    FillRect( hdc, &rc, GetStockObject(BLACK_BRUSH) );

    GetTextExtentPoint32( hdc, &pieceToFontChar[index], 1, &sz );

    pt.x = (squareSize - sz.cx) / 2;
    pt.y = (squareSize - sz.cy) / 2;

    SetBkMode( hdc, TRANSPARENT );
    SetTextColor( hdc, chroma );
    /* Step 2: the piece has been drawn in purple, there are now black and purple in this bitmap */
    TextOut( hdc, pt.x, pt.y, &pieceToFontChar[appData.allWhite && index >= (int)BlackPawn ? index - (int)BlackPawn : index], 1 );

    SelectObject( hdc, GetStockObject(WHITE_BRUSH) );
    /* Step 3: the area outside the piece is filled with white */
//    FloodFill( hdc, 0, 0, chroma );
    ExtFloodFill( hdc, 0, 0, 0, FLOODFILLSURFACE );
    ExtFloodFill( hdc, 0, squareSize-1, 0, FLOODFILLSURFACE ); // [HGM] fill from all 4 corners, for if piece too big
    ExtFloodFill( hdc, squareSize-1, 0, 0, FLOODFILLSURFACE );
    ExtFloodFill( hdc, squareSize-1, squareSize-1, 0, FLOODFILLSURFACE );
    SelectObject( hdc, GetStockObject(BLACK_BRUSH) );
    /* 
        Step 4: this is the tricky part, the area inside the piece is filled with black,
        but if the start point is not inside the piece we're lost!
        There should be a better way to do this... if we could create a region or path
        from the fill operation we would be fine for example.
    */
//    FloodFill( hdc, squareSize / 2, squareSize / 2, RGB(0xFF,0xFF,0xFF) );
    ExtFloodFill( hdc, squareSize / 2, squareSize / 2, RGB(0xFF,0xFF,0xFF), FLOODFILLBORDER );

    {   /* [HGM] shave off edges of mask, in an attempt to correct for the fact that FloodFill does not work correctly under Win XP */
        HDC dc2 = CreateCompatibleDC( hdc_window );
        HBITMAP bm2 = CreateCompatibleBitmap( hdc_window, squareSize, squareSize );

        SelectObject( dc2, bm2 );
        BitBlt( dc2, 0, 0, squareSize, squareSize, hdc, 0, 0, SRCCOPY ); // make copy
        BitBlt( hdc, 0, 1, squareSize-2, squareSize-2, dc2, 1, 1, SRCPAINT );
        BitBlt( hdc, 2, 1, squareSize-2, squareSize-2, dc2, 1, 1, SRCPAINT );
        BitBlt( hdc, 1, 0, squareSize-2, squareSize-2, dc2, 1, 1, SRCPAINT );
        BitBlt( hdc, 1, 2, squareSize-2, squareSize-2, dc2, 1, 1, SRCPAINT );

        DeleteDC( dc2 );
        DeleteObject( bm2 );
    }

    SetTextColor( hdc, 0 );
    /* 
        Step 5: some fonts have "disconnected" areas that are skipped by the fill:
        draw the piece again in black for safety.
    */
    TextOut( hdc, pt.x, pt.y, &pieceToFontChar[appData.allWhite && index >= (int)BlackPawn ? index - (int)BlackPawn : index], 1 );

    SelectObject( hdc, hbm_old );

    if( hPieceMask[index] != NULL ) {
        DeleteObject( hPieceMask[index] );
    }

    hPieceMask[index] = hbm;

    /* Face */
    hbm = CreateCompatibleBitmap( hdc_window, squareSize, squareSize );

    SelectObject( hdc, hbm );

    {
        HDC dc1 = CreateCompatibleDC( hdc_window );
        HDC dc2 = CreateCompatibleDC( hdc_window );
        HBITMAP bm2 = CreateCompatibleBitmap( hdc_window, squareSize, squareSize );

        SelectObject( dc1, hPieceMask[index] );
        SelectObject( dc2, bm2 );
        FillRect( dc2, &rc, GetStockObject(WHITE_BRUSH) );
        BitBlt( dc2, 0, 0, squareSize, squareSize, dc1, 0, 0, SRCINVERT );
        
        /* 
            Now dc2 contains the inverse of the piece mask, i.e. a mask that preserves
            the piece background and deletes (makes transparent) the rest.
            Thanks to that mask, we are free to paint the background with the greates
            freedom, as we'll be able to mask off the unwanted parts when finished.
            We use this, to make gradients and give the pieces a "roundish" look.
        */
        SetPieceBackground( hdc, backColor, 2 );
        BitBlt( hdc, 0, 0, squareSize, squareSize, dc2, 0, 0, SRCAND );

        DeleteDC( dc2 );
        DeleteDC( dc1 );
        DeleteObject( bm2 );
    }

    SetTextColor( hdc, foreColor );
    TextOut( hdc, pt.x, pt.y, &pieceToFontChar[appData.allWhite && index >= (int)BlackPawn ? index - (int)BlackPawn : index], 1 );

    SelectObject( hdc, hbm_old );

    if( hPieceFace[index] != NULL ) {
        DeleteObject( hPieceFace[index] );
    }

    hPieceFace[index] = hbm;
}

static int TranslatePieceToFontPiece( int piece )
{
    switch( piece ) {
    case BlackPawn:
        return PM_BP;
    case BlackKnight:
        return PM_BN;
    case BlackBishop:
        return PM_BB;
    case BlackRook:
        return PM_BR;
    case BlackQueen:
        return PM_BQ;
    case BlackKing:
        return PM_BK;
    case WhitePawn:
        return PM_WP;
    case WhiteKnight:
        return PM_WN;
    case WhiteBishop:
        return PM_WB;
    case WhiteRook:
        return PM_WR;
    case WhiteQueen:
        return PM_WQ;
    case WhiteKing:
        return PM_WK;

    case BlackAngel:
        return PM_BA;
    case BlackMarshall:
        return PM_BC;
    case BlackFerz:
        return PM_BF;
    case BlackNightrider:
        return PM_BH;
    case BlackAlfil:
        return PM_BE;
    case BlackWazir:
        return PM_BW;
    case BlackUnicorn:
        return PM_BU;
    case BlackCannon:
        return PM_BO;
    case BlackGrasshopper:
        return PM_BG;
    case BlackMan:
        return PM_BM;
    case BlackSilver:
        return PM_BSG;
    case BlackLance:
        return PM_BL;
    case BlackFalcon:
        return PM_BV;
    case BlackCobra:
        return PM_BS;
    case BlackCardinal:
        return PM_BAB;
    case BlackDragon:
        return PM_BD;

    case WhiteAngel:
        return PM_WA;
    case WhiteMarshall:
        return PM_WC;
    case WhiteFerz:
        return PM_WF;
    case WhiteNightrider:
        return PM_WH;
    case WhiteAlfil:
        return PM_WE;
    case WhiteWazir:
        return PM_WW;
    case WhiteUnicorn:
        return PM_WU;
    case WhiteCannon:
        return PM_WO;
    case WhiteGrasshopper:
        return PM_WG;
    case WhiteMan:
        return PM_WM;
    case WhiteSilver:
        return PM_WSG;
    case WhiteLance:
        return PM_WL;
    case WhiteFalcon:
        return PM_WV;
    case WhiteCobra:
        return PM_WS;
    case WhiteCardinal:
        return PM_WAB;
    case WhiteDragon:
        return PM_WD;
    }

    return 0;
}

void CreatePiecesFromFont()
{
    LOGFONT lf;
    HDC hdc_window = NULL;
    HDC hdc = NULL;
    HFONT hfont_old;
    int fontHeight;
    int i;

    if( fontBitmapSquareSize < 0 ) {
        /* Something went seriously wrong in the past: do not try to recreate fonts! */
        return;
    }

    if( !appData.useFont || appData.renderPiecesWithFont == NULL ||
            appData.renderPiecesWithFont[0] == NULLCHAR || appData.renderPiecesWithFont[0] == '*' ) {
        fontBitmapSquareSize = -1;
        return;
    }

    if( fontBitmapSquareSize != squareSize ) {
        hdc_window = GetDC( hwndMain );
        hdc = CreateCompatibleDC( hdc_window );

        if( hPieceFont != NULL ) {
            DeleteObject( hPieceFont );
        }
        else {
            for( i=0; i<=(int)BlackKing; i++ ) {
                hPieceMask[i] = NULL;
                hPieceFace[i] = NULL;
            }
        }

        fontHeight = 75;

        if( appData.fontPieceSize >= 50 && appData.fontPieceSize <= 150 ) {
            fontHeight = appData.fontPieceSize;
        }

        fontHeight = (fontHeight * squareSize) / 100;

        lf.lfHeight = -MulDiv( fontHeight, GetDeviceCaps(hdc, LOGPIXELSY), 72 );
        lf.lfWidth = 0;
        lf.lfEscapement = 0;
        lf.lfOrientation = 0;
        lf.lfWeight = FW_NORMAL;
        lf.lfItalic = 0;
        lf.lfUnderline = 0;
        lf.lfStrikeOut = 0;
        lf.lfCharSet = DEFAULT_CHARSET;
        lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
        lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
        lf.lfQuality = PROOF_QUALITY;
        lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
        strncpy( lf.lfFaceName, appData.renderPiecesWithFont, sizeof(lf.lfFaceName) );
        lf.lfFaceName[ sizeof(lf.lfFaceName) - 1 ] = '\0';

        hPieceFont = CreateFontIndirect( &lf );

        if( hPieceFont == NULL ) {
            fontBitmapSquareSize = -2;
        }
        else {
            /* Setup font-to-piece character table */
            if( ! SetCharTable(pieceToFontChar, appData.fontToPieceTable) ) {
                /* No (or wrong) global settings, try to detect the font */
                if( strstr(lf.lfFaceName,"Alpha") != NULL ) {
                    /* Alpha */
                    SetCharTable(pieceToFontChar, "phbrqkojntwl");
                }
                else if( strstr(lf.lfFaceName,"DiagramTT") != NULL ) {
                    /* DiagramTT* family */
                    SetCharTable(pieceToFontChar, "PNLRQKpnlrqk");
                }
                else if( strstr(lf.lfFaceName,"WinboardF") != NULL ) {
                    /* Fairy symbols */
                     SetCharTable(pieceToFontChar, "PNBRQFEACWMOHIJGDVSLUKpnbrqfeacwmohijgdvsluk");
                }
                else if( strstr(lf.lfFaceName,"GC2004D") != NULL ) {
                    /* Good Companion (Some characters get warped as literal :-( */
                    char s[] = "1cmWG0??S??oYI23wgQU";
                    s[0]=0xB9; s[1]=0xA9; s[6]=0xB1; s[11]=0xBB; s[12]=0xAB; s[17]=0xB3;
                    SetCharTable(pieceToFontChar, s);
                }
                else {
                    /* Cases, Condal, Leipzig, Lucena, Marroquin, Merida, Usual */
                    SetCharTable(pieceToFontChar, "pnbrqkomvtwl");
                }
            }

            /* Create bitmaps */
            hfont_old = SelectObject( hdc, hPieceFont );
	    for(i=(int)WhitePawn; i<(int)EmptySquare; i++) /* [HGM] made a loop for this */
		if(PieceToChar((ChessSquare)i) != '.')     /* skip unused pieces         */
		    CreatePieceMaskFromFont( hdc_window, hdc, i );

            SelectObject( hdc, hfont_old );

            fontBitmapSquareSize = squareSize;
        }
    }

    if( hdc != NULL ) {
        DeleteDC( hdc );
    }

    if( hdc_window != NULL ) {
        ReleaseDC( hwndMain, hdc_window );
    }
}

HBITMAP
DoLoadBitmap(HINSTANCE hinst, char *piece, int squareSize, char *suffix)
{
  char name[128], buf[MSG_SIZ];

    snprintf(name, sizeof(name)/sizeof(name[0]), "%s%d%s", piece, squareSize, suffix);
  if(appData.pieceDirectory[0]) {
    HBITMAP res;
    snprintf(buf, MSG_SIZ, "%s\\%s.bmp", appData.pieceDirectory, name);
    res = LoadImage( 0, buf, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE );
    if(res) return res;
  }
  if (gameInfo.event &&
      strcmp(gameInfo.event, "Easter Egg Hunt") == 0 &&
      strcmp(name, "k80s") == 0) {
    safeStrCpy(name, "tim", sizeof(name)/sizeof(name[0]) );
  }
  return LoadBitmap(hinst, name);
}


/* Insert a color into the program's logical palette
   structure.  This code assumes the given color is
   the result of the RGB or PALETTERGB macro, and it
   knows how those macros work (which is documented).
*/
VOID
InsertInPalette(COLORREF color)
{
  LPPALETTEENTRY pe = &(pLogPal->palPalEntry[pLogPal->palNumEntries]);

  if (pLogPal->palNumEntries++ >= PALETTESIZE) {
    DisplayFatalError(_("Too many colors"), 0, 1);
    pLogPal->palNumEntries--;
    return;
  }

  pe->peFlags = (char) 0;
  pe->peRed = (char) (0xFF & color);
  pe->peGreen = (char) (0xFF & (color >> 8));
  pe->peBlue = (char) (0xFF & (color >> 16));
  return;
}


VOID
InitDrawingColors()
{
  int i;
  if (pLogPal == NULL) {
    /* Allocate enough memory for a logical palette with
     * PALETTESIZE entries and set the size and version fields
     * of the logical palette structure.
     */
    pLogPal = (NPLOGPALETTE)
      LocalAlloc(LMEM_FIXED, (sizeof(LOGPALETTE) +
			      (sizeof(PALETTEENTRY) * (PALETTESIZE))));
    pLogPal->palVersion    = 0x300;
  }
  pLogPal->palNumEntries = 0;

  InsertInPalette(lightSquareColor);
  InsertInPalette(darkSquareColor);
  InsertInPalette(whitePieceColor);
  InsertInPalette(blackPieceColor);
  InsertInPalette(highlightSquareColor);
  InsertInPalette(premoveHighlightColor);

  /*  create a logical color palette according the information
   *  in the LOGPALETTE structure.
   */
  hPal = CreatePalette((LPLOGPALETTE) pLogPal);

  lightSquareBrush = CreateSolidBrush(lightSquareColor);
  blackSquareBrush = CreateSolidBrush(blackPieceColor);
  darkSquareBrush = CreateSolidBrush(darkSquareColor);
  whitePieceBrush = CreateSolidBrush(whitePieceColor);
  blackPieceBrush = CreateSolidBrush(blackPieceColor);
  iconBkgndBrush = CreateSolidBrush(GetSysColor(COLOR_BACKGROUND));
  explodeBrush = CreateSolidBrush(highlightSquareColor); // [HGM] atomic
    for(i=0; i<8;i++) markerBrush[i] = CreateSolidBrush(markerColor[i]); // [HGM] markers

   /* [AS] Force rendering of the font-based pieces */
  if( fontBitmapSquareSize > 0 ) {
    fontBitmapSquareSize = 0;
  }
}


int
BoardWidth(int boardSize, int n)
{ /* [HGM] argument n added to allow different width and height */
  int lineGap = sizeInfo[boardSize].lineGap;

  if( appData.overrideLineGap >= 0 && appData.overrideLineGap <= 5 ) {
      lineGap = appData.overrideLineGap;
  }

  return (n + 1) * lineGap +
          n * sizeInfo[boardSize].squareSize;
}

/* Respond to board resize by dragging edge */
VOID
ResizeBoard(int newSizeX, int newSizeY, int flags)
{
  BoardSize newSize = NUM_SIZES - 1;
  static int recurse = 0;
  if (IsIconic(hwndMain)) return;
  if (recurse > 0) return;
  recurse++;
  while (newSize > 0) {
	InitDrawingSizes(newSize+1000, 0); // [HGM] kludge to update sizeInfo without visible effects
	if(newSizeX >= sizeInfo[newSize].cliWidth &&
	   newSizeY >= sizeInfo[newSize].cliHeight) break;
    newSize--;
  } 
  boardSize = newSize;
  InitDrawingSizes(boardSize, flags);
  recurse--;
}


extern Boolean twoBoards, partnerUp; // [HGM] dual

VOID
InitDrawingSizes(BoardSize boardSize, int flags)
{
  int i, boardWidth, boardHeight; /* [HGM] height treated separately */
  ChessSquare piece;
  static int oldBoardSize = -1, oldTinyLayout = 0;
  HDC hdc;
  SIZE clockSize, messageSize;
  HFONT oldFont;
  char buf[MSG_SIZ];
  char *str;
  HMENU hmenu = GetMenu(hwndMain);
  RECT crect, wrect, oldRect;
  int offby;
  LOGBRUSH logbrush;
  VariantClass v = gameInfo.variant;

  int suppressVisibleEffects = 0; // [HGM] kludge to request updating sizeInfo only
  if((int)boardSize >= 1000 ) { boardSize -= 1000; suppressVisibleEffects = 1; }

  /* [HGM] call with -2 uses old size (for if nr of files, ranks changes) */
  if(boardSize == (BoardSize)(-2) ) boardSize = oldBoardSize;
  if(boardSize == -1) return;     // no size defined yet; abort (to allow early call of InitPosition)
  oldBoardSize = boardSize;

  if(boardSize != SizeMiddling && boardSize != SizePetite && boardSize != SizeBulky && !appData.useFont)
  { // correct board size to one where built-in pieces exist
    if((v == VariantCapablanca || v == VariantGothic || v == VariantGrand || v == VariantCapaRandom || v == VariantJanus || v == VariantSuper)
       && (boardSize < SizePetite || boardSize > SizeBulky) // Archbishop and Chancellor available in entire middle range

      || (v == VariantShogi && boardSize != SizeModerate)   // Japanese-style Shogi
      ||  v == VariantKnightmate || v == VariantSChess || v == VariantXiangqi || v == VariantSpartan
      ||  v == VariantShatranj || v == VariantMakruk || v == VariantGreat || v == VariantFairy || v == VariantLion ) {
      if(boardSize < SizeMediocre) boardSize = SizePetite; else
      if(boardSize > SizeModerate) boardSize = SizeBulky;  else
                                   boardSize = SizeMiddling;
    }
  }
  if(!appData.useFont && boardSize == SizePetite && (v == VariantKnightmate)) boardSize = SizeMiddling; // no Unicorn in Petite

  oldRect.left = wpMain.x; //[HGM] placement: remember previous window params
  oldRect.top = wpMain.y;
  oldRect.right = wpMain.x + wpMain.width;
  oldRect.bottom = wpMain.y + wpMain.height;

  tinyLayout = sizeInfo[boardSize].tinyLayout;
  smallLayout = sizeInfo[boardSize].smallLayout;
  squareSize = sizeInfo[boardSize].squareSize;
  lineGap = sizeInfo[boardSize].lineGap;
  minorSize = 0; /* [HGM] Kludge to see if demagnified pieces need to be shifted  */
  border = appData.useBorder && appData.border[0] ? squareSize/2 : 0;

  if( appData.overrideLineGap >= 0 && appData.overrideLineGap <= 5 ) {
      lineGap = appData.overrideLineGap;
  }

  if (tinyLayout != oldTinyLayout) {
    long style = GetWindowLongPtr(hwndMain, GWL_STYLE);
    if (tinyLayout) {
      style &= ~WS_SYSMENU;
      InsertMenu(hmenu, IDM_Exit, MF_BYCOMMAND, IDM_Minimize,
		 "&Minimize\tCtrl+F4");
    } else {
      style |= WS_SYSMENU;
      RemoveMenu(hmenu, IDM_Minimize, MF_BYCOMMAND);
    }
    SetWindowLongPtr(hwndMain, GWL_STYLE, style);

    for (i=0; menuBarText[tinyLayout][i]; i++) {
      ModifyMenu(hmenu, i, MF_STRING|MF_BYPOSITION|MF_POPUP, 
	(UINT)GetSubMenu(hmenu, i), T_(menuBarText[tinyLayout][i]));
    }
    DrawMenuBar(hwndMain);
  }

  boardWidth  = BoardWidth(boardSize, BOARD_WIDTH) + 2*border;
  boardHeight = BoardWidth(boardSize, BOARD_HEIGHT) + 2*border;

  /* Get text area sizes */
  hdc = GetDC(hwndMain);
  if (appData.clockMode) {
    snprintf(buf, MSG_SIZ, _("White: %s"), TimeString(23*60*60*1000L));
  } else {
    snprintf(buf, MSG_SIZ, _("White"));
  }
  oldFont = SelectObject(hdc, font[boardSize][CLOCK_FONT]->hf);
  GetTextExtentPoint(hdc, buf, strlen(buf), &clockSize);
  SelectObject(hdc, font[boardSize][MESSAGE_FONT]->hf);
  str = _("We only care about the height here");
  GetTextExtentPoint(hdc, str, strlen(str), &messageSize);
  SelectObject(hdc, oldFont);
  ReleaseDC(hwndMain, hdc);

  /* Compute where everything goes */
  if((first.programLogo || second.programLogo) && !tinyLayout) {
        /* [HGM] logo: if either logo is on, reserve space for it */
	logoHeight =  2*clockSize.cy;
	leftLogoRect.left   = OUTER_MARGIN;
	leftLogoRect.right  = leftLogoRect.left + 4*clockSize.cy;
	leftLogoRect.top    = OUTER_MARGIN;
	leftLogoRect.bottom = OUTER_MARGIN + logoHeight;

	rightLogoRect.right  = OUTER_MARGIN + boardWidth;
	rightLogoRect.left   = rightLogoRect.right - 4*clockSize.cy;
	rightLogoRect.top    = OUTER_MARGIN;
	rightLogoRect.bottom = OUTER_MARGIN + logoHeight;


    whiteRect.left = leftLogoRect.right;
    whiteRect.right = OUTER_MARGIN + boardWidth/2 - INNER_MARGIN/2;
    whiteRect.top = OUTER_MARGIN;
    whiteRect.bottom = whiteRect.top + logoHeight;

    blackRect.right = rightLogoRect.left;
    blackRect.left = whiteRect.right + INNER_MARGIN;
    blackRect.top = whiteRect.top;
    blackRect.bottom = whiteRect.bottom;
  } else {
    whiteRect.left = OUTER_MARGIN;
    whiteRect.right = whiteRect.left + boardWidth/2 - INNER_MARGIN/2;
    whiteRect.top = OUTER_MARGIN;
    whiteRect.bottom = whiteRect.top + clockSize.cy;

    blackRect.left = whiteRect.right + INNER_MARGIN;
    blackRect.right = blackRect.left + boardWidth/2 - 1;
    blackRect.top = whiteRect.top;
    blackRect.bottom = whiteRect.bottom;

    logoHeight = 0; // [HGM] logo: suppress logo after change to tiny layout!
  }

  messageRect.left = OUTER_MARGIN + MESSAGE_LINE_LEFTMARGIN;
  if (appData.showButtonBar) {
    messageRect.right = OUTER_MARGIN + boardWidth         // [HGM] logo: expressed independent of clock placement
      - N_BUTTONS*BUTTON_WIDTH - MESSAGE_LINE_LEFTMARGIN;
  } else {
    messageRect.right = OUTER_MARGIN + boardWidth;
  }
  messageRect.top = whiteRect.bottom + INNER_MARGIN;
  messageRect.bottom = messageRect.top + messageSize.cy;

  boardRect.left = OUTER_MARGIN;
  boardRect.right = boardRect.left + boardWidth;
  boardRect.top = messageRect.bottom + INNER_MARGIN;
  boardRect.bottom = boardRect.top + boardHeight;

  sizeInfo[boardSize].cliWidth = boardRect.right + OUTER_MARGIN;
  sizeInfo[boardSize].cliHeight = boardRect.bottom + OUTER_MARGIN;
  oldTinyLayout = tinyLayout;
  winW = 2 * GetSystemMetrics(SM_CXFRAME) + boardRect.right + OUTER_MARGIN;
  winH = 2 * GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CYMENU) +
    GetSystemMetrics(SM_CYCAPTION) + boardRect.bottom + OUTER_MARGIN;
  winW *= 1 + twoBoards;
  if(suppressVisibleEffects) return; // [HGM] when called for filling sizeInfo only
  wpMain.width = winW;  // [HGM] placement: set through temporary which can used by initial sizing choice
  wpMain.height = winH; //       without disturbing window attachments
  GetWindowRect(hwndMain, &wrect);
  SetWindowPos(hwndMain, NULL, 0, 0, wpMain.width, wpMain.height,
	       SWP_NOCOPYBITS|SWP_NOZORDER|SWP_NOMOVE);

  // [HGM] placement: let attached windows follow size change.
  ReattachAfterSize( &oldRect, wpMain.width, wpMain.height, moveHistoryDialog, &wpMoveHistory );
  ReattachAfterSize( &oldRect, wpMain.width, wpMain.height, evalGraphDialog, &wpEvalGraph );
  ReattachAfterSize( &oldRect, wpMain.width, wpMain.height, engineOutputDialog, &wpEngineOutput );
  ReattachAfterSize( &oldRect, wpMain.width, wpMain.height, gameListDialog, &wpGameList );
  ReattachAfterSize( &oldRect, wpMain.width, wpMain.height, hwndConsole, &wpConsole );

  /* compensate if menu bar wrapped */
  GetClientRect(hwndMain, &crect);
  offby = boardRect.bottom + OUTER_MARGIN - crect.bottom;
  wpMain.height += offby;
  switch (flags) {
  case WMSZ_TOPLEFT:
    SetWindowPos(hwndMain, NULL, 
                 wrect.right - wpMain.width, wrect.bottom - wpMain.height, 
                 wpMain.width, wpMain.height, SWP_NOCOPYBITS|SWP_NOZORDER);
    break;

  case WMSZ_TOPRIGHT:
  case WMSZ_TOP:
    SetWindowPos(hwndMain, NULL, 
                 wrect.left, wrect.bottom - wpMain.height, 
                 wpMain.width, wpMain.height, SWP_NOCOPYBITS|SWP_NOZORDER);
    break;

  case WMSZ_BOTTOMLEFT:
  case WMSZ_LEFT:
    SetWindowPos(hwndMain, NULL, 
                 wrect.right - wpMain.width, wrect.top, 
                 wpMain.width, wpMain.height, SWP_NOCOPYBITS|SWP_NOZORDER);
    break;

  case WMSZ_BOTTOMRIGHT:
  case WMSZ_BOTTOM:
  case WMSZ_RIGHT:
  default:
    SetWindowPos(hwndMain, NULL, 0, 0, wpMain.width, wpMain.height,
               SWP_NOCOPYBITS|SWP_NOZORDER|SWP_NOMOVE);
    break;
  }

  hwndPause = NULL;
  for (i = 0; i < N_BUTTONS; i++) {
    if (buttonDesc[i].hwnd != NULL) {
      DestroyWindow(buttonDesc[i].hwnd);
      buttonDesc[i].hwnd = NULL;
    }
    if (appData.showButtonBar) {
      buttonDesc[i].hwnd =
	CreateWindow("BUTTON", buttonDesc[i].label,
		     WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		     boardRect.right - BUTTON_WIDTH*(N_BUTTONS-i),
		     messageRect.top, BUTTON_WIDTH, messageSize.cy, hwndMain,
		     (HMENU) buttonDesc[i].id,
		     (HINSTANCE) GetWindowLongPtr(hwndMain, GWLP_HINSTANCE), NULL);
      if (tinyLayout) {
	SendMessage(buttonDesc[i].hwnd, WM_SETFONT, 
		    (WPARAM)font[boardSize][MESSAGE_FONT]->hf,
		    MAKELPARAM(FALSE, 0));
      }
      if (buttonDesc[i].id == IDM_Pause)
	hwndPause = buttonDesc[i].hwnd;
      buttonDesc[i].wndproc = (WNDPROC)
	SetWindowLongPtr(buttonDesc[i].hwnd, GWLP_WNDPROC, (LONG_PTR) ButtonProc);
    }
  }
  if (gridPen != NULL) DeleteObject(gridPen);
  if (highlightPen != NULL) DeleteObject(highlightPen);
  if (premovePen != NULL) DeleteObject(premovePen);
  if (lineGap != 0) {
    logbrush.lbStyle = BS_SOLID;
    logbrush.lbColor = RGB(0, 0, 0); /* grid pen color = black */
    gridPen =
      ExtCreatePen(PS_GEOMETRIC|PS_SOLID|PS_ENDCAP_FLAT|PS_JOIN_MITER,
                   lineGap, &logbrush, 0, NULL);
    logbrush.lbColor = highlightSquareColor;
    highlightPen =
      ExtCreatePen(PS_GEOMETRIC|PS_SOLID|PS_ENDCAP_FLAT|PS_JOIN_MITER,
                   lineGap, &logbrush, 0, NULL);

    logbrush.lbColor = premoveHighlightColor; 
    premovePen =
      ExtCreatePen(PS_GEOMETRIC|PS_SOLID|PS_ENDCAP_FLAT|PS_JOIN_MITER,
                   lineGap, &logbrush, 0, NULL);

    /* [HGM] Loop had to be split in part for vert. and hor. lines */
    for (i = 0; i < BOARD_HEIGHT + 1; i++) {
      gridEndpoints[i*2].x = boardRect.left + lineGap / 2 + border;
      gridEndpoints[i*2].y = gridEndpoints[i*2 + 1].y =
	boardRect.top + lineGap / 2 + (i * (squareSize + lineGap)) + border;
      gridEndpoints[i*2 + 1].x = boardRect.left + lineGap / 2 +
        BOARD_WIDTH * (squareSize + lineGap) + border;
      gridVertexCounts[i*2] = gridVertexCounts[i*2 + 1] = 2;
    }
    for (i = 0; i < BOARD_WIDTH + 1; i++) {
      gridEndpoints[i*2 + BOARD_HEIGHT*2 + 2].y = boardRect.top + lineGap / 2 + border;
      gridEndpoints[i*2 + BOARD_HEIGHT*2 + 2].x =
        gridEndpoints[i*2 + 1 + BOARD_HEIGHT*2 + 2].x = boardRect.left +
	lineGap / 2 + (i * (squareSize + lineGap)) + border;
      gridEndpoints[i*2 + 1 + BOARD_HEIGHT*2 + 2].y =
        boardRect.top + BOARD_HEIGHT * (squareSize + lineGap) + border;
      gridVertexCounts[i*2] = gridVertexCounts[i*2 + 1] = 2;
    }
  }

  /* [HGM] Licensing requirement */
#ifdef GOTHIC
  if(gameInfo.variant == VariantGothic) GothicPopUp( GOTHIC, VariantGothic); else
#endif
#ifdef FALCON
  if(gameInfo.variant == VariantFalcon) GothicPopUp( FALCON, VariantFalcon); else
#endif
  GothicPopUp( "", VariantNormal);


/*  if (boardSize == oldBoardSize) return; [HGM] variant might have changed */

  /* Load piece bitmaps for this board size */
  for (i=0; i<=2; i++) {
    for (piece = WhitePawn;
         (int) piece < (int) BlackPawn;
	 piece = (ChessSquare) ((int) piece + 1)) {
      if (pieceBitmap[i][piece] != NULL)
	DeleteObject(pieceBitmap[i][piece]);
    }
  }

  fontBitmapSquareSize = 0; /* [HGM] render: make sure pieces will be recreated, as we might need others now */
  // Orthodox Chess pieces
  pieceBitmap[0][WhitePawn] = DoLoadBitmap(hInst, "p", squareSize, "s");
  pieceBitmap[0][WhiteKnight] = DoLoadBitmap(hInst, "n", squareSize, "s");
  pieceBitmap[0][WhiteBishop] = DoLoadBitmap(hInst, "b", squareSize, "s");
  pieceBitmap[0][WhiteRook] = DoLoadBitmap(hInst, "r", squareSize, "s");
  pieceBitmap[0][WhiteKing] = DoLoadBitmap(hInst, "k", squareSize, "s");
  pieceBitmap[1][WhitePawn] = DoLoadBitmap(hInst, "p", squareSize, "o");
  pieceBitmap[1][WhiteKnight] = DoLoadBitmap(hInst, "n", squareSize, "o");
  pieceBitmap[1][WhiteBishop] = DoLoadBitmap(hInst, "b", squareSize, "o");
  pieceBitmap[1][WhiteRook] = DoLoadBitmap(hInst, "r", squareSize, "o");
  pieceBitmap[1][WhiteKing] = DoLoadBitmap(hInst, "k", squareSize, "o");
  pieceBitmap[2][WhitePawn] = DoLoadBitmap(hInst, "p", squareSize, "w");
  pieceBitmap[2][WhiteKnight] = DoLoadBitmap(hInst, "n", squareSize, "w");
  pieceBitmap[2][WhiteBishop] = DoLoadBitmap(hInst, "b", squareSize, "w");
  pieceBitmap[2][WhiteRook] = DoLoadBitmap(hInst, "r", squareSize, "w");
  pieceBitmap[2][WhiteKing] = DoLoadBitmap(hInst, "k", squareSize, "w");
  if( gameInfo.variant == VariantShogi && squareSize <= 72 && squareSize >= 33) {
    // in Shogi, Hijack the unused Queen for Lance
    pieceBitmap[0][WhiteQueen] = DoLoadBitmap(hInst, "l", squareSize, "s");
    pieceBitmap[1][WhiteQueen] = DoLoadBitmap(hInst, "l", squareSize, "o");
    pieceBitmap[2][WhiteQueen] = DoLoadBitmap(hInst, "l", squareSize, "w");
  } else {
    pieceBitmap[0][WhiteQueen] = DoLoadBitmap(hInst, "q", squareSize, "s");
    pieceBitmap[1][WhiteQueen] = DoLoadBitmap(hInst, "q", squareSize, "o");
    pieceBitmap[2][WhiteQueen] = DoLoadBitmap(hInst, "q", squareSize, "w");
  }

  if(squareSize <= 72 && squareSize >= 33) { 
    /* A & C are available in most sizes now */
    if(squareSize != 49 && squareSize != 72 && squareSize != 33) { // Vortex-like
      pieceBitmap[0][WhiteAngel] = DoLoadBitmap(hInst, "a", squareSize, "s");
      pieceBitmap[1][WhiteAngel] = DoLoadBitmap(hInst, "a", squareSize, "o");
      pieceBitmap[2][WhiteAngel] = DoLoadBitmap(hInst, "a", squareSize, "w");
      pieceBitmap[0][WhiteSilver] = DoLoadBitmap(hInst, "cv", squareSize, "s");
      pieceBitmap[1][WhiteSilver] = DoLoadBitmap(hInst, "cv", squareSize, "o");
      pieceBitmap[2][WhiteSilver] = DoLoadBitmap(hInst, "cv", squareSize, "w");
      pieceBitmap[0][WhiteCobra] = DoLoadBitmap(hInst, "cv", squareSize, "s");
      pieceBitmap[1][WhiteCobra] = DoLoadBitmap(hInst, "cv", squareSize, "o");
      pieceBitmap[2][WhiteCobra] = DoLoadBitmap(hInst, "cv", squareSize, "w");
      pieceBitmap[0][WhiteLance] = DoLoadBitmap(hInst, "l", squareSize, "s");
      pieceBitmap[1][WhiteLance] = DoLoadBitmap(hInst, "l", squareSize, "o");
      pieceBitmap[2][WhiteLance] = DoLoadBitmap(hInst, "l", squareSize, "w");
    } else { // Smirf-like
      if(gameInfo.variant == VariantSChess) {
        pieceBitmap[0][WhiteAngel] = DoLoadBitmap(hInst, "v", squareSize, "s");
        pieceBitmap[1][WhiteAngel] = DoLoadBitmap(hInst, "v", squareSize, "o");
        pieceBitmap[2][WhiteAngel] = DoLoadBitmap(hInst, "v", squareSize, "w");
      } else {
        pieceBitmap[0][WhiteAngel] = DoLoadBitmap(hInst, "aa", squareSize, "s");
        pieceBitmap[1][WhiteAngel] = DoLoadBitmap(hInst, "aa", squareSize, "o");
        pieceBitmap[2][WhiteAngel] = DoLoadBitmap(hInst, "aa", squareSize, "w");
      }
    }
    if(gameInfo.variant == VariantGothic) { // Vortex-like
      pieceBitmap[0][WhiteMarshall] = DoLoadBitmap(hInst, "cv", squareSize, "s");
      pieceBitmap[1][WhiteMarshall] = DoLoadBitmap(hInst, "cv", squareSize, "o");
      pieceBitmap[2][WhiteMarshall] = DoLoadBitmap(hInst, "cv", squareSize, "w");
    } else if(gameInfo.variant == VariantSChess && (squareSize == 49 || squareSize == 72)) {
      pieceBitmap[0][WhiteMarshall] = DoLoadBitmap(hInst, "e", squareSize, "s");
      pieceBitmap[1][WhiteMarshall] = DoLoadBitmap(hInst, "e", squareSize, "o");
      pieceBitmap[2][WhiteMarshall] = DoLoadBitmap(hInst, "e", squareSize, "w");
    } else { // WinBoard standard
      pieceBitmap[0][WhiteMarshall] = DoLoadBitmap(hInst, "c", squareSize, "s");
      pieceBitmap[1][WhiteMarshall] = DoLoadBitmap(hInst, "c", squareSize, "o");
      pieceBitmap[2][WhiteMarshall] = DoLoadBitmap(hInst, "c", squareSize, "w");
    }
  }


  if(squareSize==72 || squareSize==49 || squareSize==33) { /* experiment with some home-made bitmaps */
    pieceBitmap[0][WhiteFerz] = DoLoadBitmap(hInst, "f", squareSize, "s");
    pieceBitmap[1][WhiteFerz] = DoLoadBitmap(hInst, "f", squareSize, "o");
    pieceBitmap[2][WhiteFerz] = DoLoadBitmap(hInst, "f", squareSize, "w");
    pieceBitmap[0][WhiteWazir] = DoLoadBitmap(hInst, "w", squareSize, "s");
    pieceBitmap[1][WhiteWazir] = DoLoadBitmap(hInst, "w", squareSize, "o");
    pieceBitmap[2][WhiteWazir] = DoLoadBitmap(hInst, "w", squareSize, "w");
    pieceBitmap[0][WhiteAlfil] = DoLoadBitmap(hInst, "e", squareSize, "s");
    pieceBitmap[1][WhiteAlfil] = DoLoadBitmap(hInst, "e", squareSize, "o");
    pieceBitmap[2][WhiteAlfil] = DoLoadBitmap(hInst, "e", squareSize, "w");
    pieceBitmap[0][WhiteMan] = DoLoadBitmap(hInst, "m", squareSize, "s");
    pieceBitmap[1][WhiteMan] = DoLoadBitmap(hInst, "m", squareSize, "o");
    pieceBitmap[2][WhiteMan] = DoLoadBitmap(hInst, "m", squareSize, "w");
    pieceBitmap[0][WhiteCardinal] = DoLoadBitmap(hInst, "a", squareSize, "s");
    pieceBitmap[1][WhiteCardinal] = DoLoadBitmap(hInst, "a", squareSize, "o");
    pieceBitmap[2][WhiteCardinal] = DoLoadBitmap(hInst, "a", squareSize, "w");
    pieceBitmap[0][WhiteDragon] = DoLoadBitmap(hInst, "dk", squareSize, "s");
    pieceBitmap[1][WhiteDragon] = DoLoadBitmap(hInst, "dk", squareSize, "o");
    pieceBitmap[2][WhiteDragon] = DoLoadBitmap(hInst, "dk", squareSize, "w");
    pieceBitmap[0][WhiteFalcon] = DoLoadBitmap(hInst, "v", squareSize, "s");
    pieceBitmap[1][WhiteFalcon] = DoLoadBitmap(hInst, "v", squareSize, "o");
    pieceBitmap[2][WhiteFalcon] = DoLoadBitmap(hInst, "v", squareSize, "w");
    pieceBitmap[0][WhiteCobra] = DoLoadBitmap(hInst, "s", squareSize, "s");
    pieceBitmap[1][WhiteCobra] = DoLoadBitmap(hInst, "s", squareSize, "o");
    pieceBitmap[2][WhiteCobra] = DoLoadBitmap(hInst, "s", squareSize, "w");
    pieceBitmap[0][WhiteLance] = DoLoadBitmap(hInst, "l", squareSize, "s");
    pieceBitmap[1][WhiteLance] = DoLoadBitmap(hInst, "l", squareSize, "o");
    pieceBitmap[2][WhiteLance] = DoLoadBitmap(hInst, "l", squareSize, "w");
    pieceBitmap[0][WhiteUnicorn] = DoLoadBitmap(hInst, "u", squareSize, "s");
    pieceBitmap[1][WhiteUnicorn] = DoLoadBitmap(hInst, "u", squareSize, "o");
    pieceBitmap[2][WhiteUnicorn] = DoLoadBitmap(hInst, "u", squareSize, "w");
    pieceBitmap[0][WhiteLion] = DoLoadBitmap(hInst, "ln", squareSize, "s");
    pieceBitmap[1][WhiteLion] = DoLoadBitmap(hInst, "ln", squareSize, "o");
    pieceBitmap[2][WhiteLion] = DoLoadBitmap(hInst, "ln", squareSize, "w");

    if(gameInfo.variant == VariantShogi && BOARD_HEIGHT != 7) { /* promoted Gold representations (but not in Tori!)*/
      pieceBitmap[0][WhiteCannon] = DoLoadBitmap(hInst, "wp", squareSize, "s");
      pieceBitmap[1][WhiteCannon] = DoLoadBitmap(hInst, "wp", squareSize, "o");
      pieceBitmap[2][WhiteCannon] = DoLoadBitmap(hInst, "w", squareSize, "w");
      pieceBitmap[0][WhiteNightrider] = DoLoadBitmap(hInst, "wn", squareSize, "s");
      pieceBitmap[1][WhiteNightrider] = DoLoadBitmap(hInst, "wn", squareSize, "o");
      pieceBitmap[2][WhiteNightrider] = DoLoadBitmap(hInst, "w", squareSize, "w");
      pieceBitmap[0][WhiteSilver] = DoLoadBitmap(hInst, "ws", squareSize, "s");
      pieceBitmap[1][WhiteSilver] = DoLoadBitmap(hInst, "ws", squareSize, "o");
      pieceBitmap[2][WhiteSilver] = DoLoadBitmap(hInst, "w", squareSize, "w");
      pieceBitmap[0][WhiteGrasshopper] = DoLoadBitmap(hInst, "wl", squareSize, "s");
      pieceBitmap[1][WhiteGrasshopper] = DoLoadBitmap(hInst, "wl", squareSize, "o");
      pieceBitmap[2][WhiteGrasshopper] = DoLoadBitmap(hInst, "w", squareSize, "w");
    } else {
      pieceBitmap[0][WhiteCannon] = DoLoadBitmap(hInst, "o", squareSize, "s");
      pieceBitmap[1][WhiteCannon] = DoLoadBitmap(hInst, "o", squareSize, "o");
      pieceBitmap[2][WhiteCannon] = DoLoadBitmap(hInst, "o", squareSize, "w");
      pieceBitmap[0][WhiteNightrider] = DoLoadBitmap(hInst, "h", squareSize, "s");
      pieceBitmap[1][WhiteNightrider] = DoLoadBitmap(hInst, "h", squareSize, "o");
      pieceBitmap[2][WhiteNightrider] = DoLoadBitmap(hInst, "h", squareSize, "w");
      pieceBitmap[0][WhiteSilver] = DoLoadBitmap(hInst, "cv", squareSize, "s");
      pieceBitmap[1][WhiteSilver] = DoLoadBitmap(hInst, "cv", squareSize, "o");
      pieceBitmap[2][WhiteSilver] = DoLoadBitmap(hInst, "cv", squareSize, "w");
      pieceBitmap[0][WhiteGrasshopper] = DoLoadBitmap(hInst, "g", squareSize, "s");
      pieceBitmap[1][WhiteGrasshopper] = DoLoadBitmap(hInst, "g", squareSize, "o");
      pieceBitmap[2][WhiteGrasshopper] = DoLoadBitmap(hInst, "g", squareSize, "w");
    }

  } else { /* other size, no special bitmaps available. Use smaller symbols */
    if((int)boardSize < 2) minorSize = sizeInfo[0].squareSize;
    else  minorSize = sizeInfo[(int)boardSize - 2].squareSize;
    pieceBitmap[0][WhiteNightrider] = DoLoadBitmap(hInst, "n", minorSize, "s");
    pieceBitmap[1][WhiteNightrider] = DoLoadBitmap(hInst, "n", minorSize, "o");
    pieceBitmap[2][WhiteNightrider] = DoLoadBitmap(hInst, "n", minorSize, "w");
    pieceBitmap[0][WhiteCardinal]   = DoLoadBitmap(hInst, "b", minorSize, "s");
    pieceBitmap[1][WhiteCardinal]   = DoLoadBitmap(hInst, "b", minorSize, "o");
    pieceBitmap[2][WhiteCardinal]   = DoLoadBitmap(hInst, "b", minorSize, "w");
    pieceBitmap[0][WhiteDragon]   = DoLoadBitmap(hInst, "r", minorSize, "s");
    pieceBitmap[1][WhiteDragon]   = DoLoadBitmap(hInst, "r", minorSize, "o");
    pieceBitmap[2][WhiteDragon]   = DoLoadBitmap(hInst, "r", minorSize, "w");
    pieceBitmap[0][WhiteGrasshopper] = DoLoadBitmap(hInst, "q", minorSize, "s");
    pieceBitmap[1][WhiteGrasshopper] = DoLoadBitmap(hInst, "q", minorSize, "o");
    pieceBitmap[2][WhiteGrasshopper] = DoLoadBitmap(hInst, "q", minorSize, "w");
  }


  if(gameInfo.variant == VariantShogi && squareSize == 58)
  /* special Shogi support in this size */
  { for (i=0; i<=2; i++) { /* replace all bitmaps */
      for (piece = WhitePawn;
           (int) piece < (int) BlackPawn;
           piece = (ChessSquare) ((int) piece + 1)) {
        if (pieceBitmap[i][piece] != NULL)
          DeleteObject(pieceBitmap[i][piece]);
      }
    }
  pieceBitmap[0][WhitePawn] = DoLoadBitmap(hInst, "sp", squareSize, "o");
  pieceBitmap[0][WhiteKnight] = DoLoadBitmap(hInst, "sn", squareSize, "o");
  pieceBitmap[0][WhiteBishop] = DoLoadBitmap(hInst, "sb", squareSize, "o");
  pieceBitmap[0][WhiteRook] = DoLoadBitmap(hInst, "sr", squareSize, "o");
  pieceBitmap[0][WhiteQueen] = DoLoadBitmap(hInst, "sl", squareSize, "o");
  pieceBitmap[0][WhiteKing] = DoLoadBitmap(hInst, "sk", squareSize, "o");
  pieceBitmap[0][WhiteFerz] = DoLoadBitmap(hInst, "sf", squareSize, "o");
  pieceBitmap[0][WhiteWazir] = DoLoadBitmap(hInst, "sw", squareSize, "o");
  pieceBitmap[0][WhiteCannon] = DoLoadBitmap(hInst, "su", squareSize, "o");
  pieceBitmap[0][WhiteNightrider] = DoLoadBitmap(hInst, "sh", squareSize, "o");
  pieceBitmap[0][WhiteCardinal] = DoLoadBitmap(hInst, "sa", squareSize, "o");
  pieceBitmap[0][WhiteDragon] = DoLoadBitmap(hInst, "sc", squareSize, "o");
  pieceBitmap[0][WhiteGrasshopper] = DoLoadBitmap(hInst, "sg", squareSize, "o");
  pieceBitmap[0][WhiteSilver] = DoLoadBitmap(hInst, "ss", squareSize, "o");
  pieceBitmap[1][WhitePawn] = DoLoadBitmap(hInst, "sp", squareSize, "o");
  pieceBitmap[1][WhiteKnight] = DoLoadBitmap(hInst, "sn", squareSize, "o");
  pieceBitmap[1][WhiteBishop] = DoLoadBitmap(hInst, "sb", squareSize, "o");
  pieceBitmap[1][WhiteRook] = DoLoadBitmap(hInst, "sr", squareSize, "o");
  pieceBitmap[1][WhiteQueen] = DoLoadBitmap(hInst, "sl", squareSize, "o");
  pieceBitmap[1][WhiteKing] = DoLoadBitmap(hInst, "sk", squareSize, "o");
  pieceBitmap[1][WhiteFerz] = DoLoadBitmap(hInst, "sf", squareSize, "o");
  pieceBitmap[1][WhiteWazir] = DoLoadBitmap(hInst, "sw", squareSize, "o");
  pieceBitmap[1][WhiteCannon] = DoLoadBitmap(hInst, "su", squareSize, "o");
  pieceBitmap[1][WhiteNightrider] = DoLoadBitmap(hInst, "sh", squareSize, "o");
  pieceBitmap[1][WhiteCardinal] = DoLoadBitmap(hInst, "sa", squareSize, "o");
  pieceBitmap[1][WhiteDragon] = DoLoadBitmap(hInst, "sc", squareSize, "o");
  pieceBitmap[1][WhiteGrasshopper] = DoLoadBitmap(hInst, "sg", squareSize, "o");
  pieceBitmap[1][WhiteSilver] = DoLoadBitmap(hInst, "ss", squareSize, "o");
  pieceBitmap[2][WhitePawn] = DoLoadBitmap(hInst, "sp", squareSize, "w");
  pieceBitmap[2][WhiteKnight] = DoLoadBitmap(hInst, "sn", squareSize, "w");
  pieceBitmap[2][WhiteBishop] = DoLoadBitmap(hInst, "sr", squareSize, "w");
  pieceBitmap[2][WhiteRook] = DoLoadBitmap(hInst, "sr", squareSize, "w");
  pieceBitmap[2][WhiteQueen] = DoLoadBitmap(hInst, "sl", squareSize, "w");
  pieceBitmap[2][WhiteKing] = DoLoadBitmap(hInst, "sk", squareSize, "w");
  pieceBitmap[2][WhiteFerz] = DoLoadBitmap(hInst, "sw", squareSize, "w");
  pieceBitmap[2][WhiteWazir] = DoLoadBitmap(hInst, "sw", squareSize, "w");
  pieceBitmap[2][WhiteCannon] = DoLoadBitmap(hInst, "sp", squareSize, "w");
  pieceBitmap[2][WhiteNightrider] = DoLoadBitmap(hInst, "sn", squareSize, "w");
  pieceBitmap[2][WhiteCardinal] = DoLoadBitmap(hInst, "sr", squareSize, "w");
  pieceBitmap[2][WhiteDragon] = DoLoadBitmap(hInst, "sr", squareSize, "w");
  pieceBitmap[2][WhiteGrasshopper] = DoLoadBitmap(hInst, "sl", squareSize, "w");
  pieceBitmap[2][WhiteSilver] = DoLoadBitmap(hInst, "sw", squareSize, "w");
  minorSize = 0;
  }
}

HBITMAP
PieceBitmap(ChessSquare p, int kind)
{
  if ((int) p >= (int) BlackPawn)
    p = (ChessSquare) ((int) p - (int) BlackPawn + (int) WhitePawn);

  return pieceBitmap[kind][(int) p];
}

/***************************************************************/

#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
/*
#define MIN3(a,b,c) (((a) < (b) && (a) < (c)) ? (a) : (((b) < (a) && (b) < (c)) ? (b) : (c)))
#define MAX3(a,b,c) (((a) > (b) && (a) > (c)) ? (a) : (((b) > (a) && (b) > (c)) ? (b) : (c)))
*/

VOID
SquareToPos(int row, int column, int * x, int * y)
{
  if (flipView) {
    *x = boardRect.left + lineGap + ((BOARD_WIDTH-1)-column) * (squareSize + lineGap) + border;
    *y = boardRect.top + lineGap + row * (squareSize + lineGap) + border;
  } else {
    *x = boardRect.left + lineGap + column * (squareSize + lineGap) + border;
    *y = boardRect.top + lineGap + ((BOARD_HEIGHT-1)-row) * (squareSize + lineGap) + border;
  }
}

VOID
DrawCoordsOnDC(HDC hdc)
{
  static char files[] = "0123456789012345678901221098765432109876543210";
  static char ranks[] = "wvutsrqponmlkjihgfedcbaabcdefghijklmnopqrstuvw";
  char str[2] = { NULLCHAR, NULLCHAR };
  int oldMode, oldAlign, x, y, start, i;
  HFONT oldFont;
  HBRUSH oldBrush;

  if (!appData.showCoords)
    return;

  start = flipView ? 1-(ONE!='1') : 45+(ONE!='1')-BOARD_HEIGHT;

  oldBrush = SelectObject(hdc, GetStockObject(BLACK_BRUSH));
  oldMode = SetBkMode(hdc, (appData.monoMode ? OPAQUE : TRANSPARENT));
  oldAlign = GetTextAlign(hdc);
  oldFont = SelectObject(hdc, font[boardSize][COORD_FONT]->hf);

  y = boardRect.top + lineGap;
  x = boardRect.left + lineGap + gameInfo.holdingsWidth*(squareSize + lineGap);

  if(border) {
    SetTextAlign(hdc, TA_RIGHT|TA_TOP);
    x += border - lineGap - 4; y += squareSize - 6;
  } else
  SetTextAlign(hdc, TA_LEFT|TA_TOP);
  for (i = 0; i < BOARD_HEIGHT; i++) {
    str[0] = files[start + i];
    ExtTextOut(hdc, x + 2 - (border ? gameInfo.holdingsWidth * (squareSize + lineGap) : 0), y + 1, 0, NULL, str, 1, NULL);
    y += squareSize + lineGap;
  }

  start = flipView ? 23-(BOARD_RGHT-BOARD_LEFT) : 23;

  if(border) {
    SetTextAlign(hdc, TA_LEFT|TA_TOP);
    x += -border + 4; y += border - squareSize + 6;
  } else
  SetTextAlign(hdc, TA_RIGHT|TA_BOTTOM);
  for (i = 0; i < BOARD_RGHT - BOARD_LEFT; i++) {
    str[0] = ranks[start + i];
    ExtTextOut(hdc, x + squareSize - 2, y - 1, 0, NULL, str, 1, NULL);
    x += squareSize + lineGap;
  }    

  SelectObject(hdc, oldBrush);
  SetBkMode(hdc, oldMode);
  SetTextAlign(hdc, oldAlign);
  SelectObject(hdc, oldFont);
}

VOID
DrawGridOnDC(HDC hdc)
{
  HPEN oldPen;
 
  if (lineGap != 0) {
    oldPen = SelectObject(hdc, gridPen);
    PolyPolyline(hdc, gridEndpoints, gridVertexCounts, BOARD_WIDTH+BOARD_HEIGHT + 2);
    SelectObject(hdc, oldPen);
  }
}

#define HIGHLIGHT_PEN 0
#define PREMOVE_PEN   1

VOID
DrawHighlightOnDC(HDC hdc, BOOLEAN on, int x, int y, int pen)
{
  int x1, y1;
  HPEN oldPen, hPen;
  if (lineGap == 0) return;
  if (flipView) {
    x1 = boardRect.left +
      lineGap/2 + ((BOARD_WIDTH-1)-x) * (squareSize + lineGap) + border;
    y1 = boardRect.top +
      lineGap/2 + y * (squareSize + lineGap) + border;
  } else {
    x1 = boardRect.left +
      lineGap/2 + x * (squareSize + lineGap) + border;
    y1 = boardRect.top +
      lineGap/2 + ((BOARD_HEIGHT-1)-y) * (squareSize + lineGap) + border;
  }
  hPen = pen ? premovePen : highlightPen;
  oldPen = SelectObject(hdc, on ? hPen : gridPen);
  MoveToEx(hdc, x1, y1, NULL);
  LineTo(hdc, x1 + squareSize + lineGap, y1);
  LineTo(hdc, x1 + squareSize + lineGap, y1 + squareSize + lineGap);
  LineTo(hdc, x1, y1 + squareSize + lineGap);
  LineTo(hdc, x1, y1);
  SelectObject(hdc, oldPen);
}

VOID
DrawHighlightsOnDC(HDC hdc, HighlightInfo *h, int pen)
{
  int i;
  for (i=0; i<2; i++) {
    if (h->sq[i].x >= 0 && h->sq[i].y >= 0) 
      DrawHighlightOnDC(hdc, TRUE,
			h->sq[i].x, h->sq[i].y,
			pen);
  }
}

/* Note: sqcolor is used only in monoMode */
/* Note that this code is largely duplicated in woptions.c,
   function DrawSampleSquare, so that needs to be updated too */
VOID
DrawPieceOnDC(HDC hdc, ChessSquare piece, int color, int sqcolor, int x, int y, HDC tmphdc)
{
  HBITMAP oldBitmap;
  HBRUSH oldBrush;
  int tmpSize;

  if (appData.blindfold) return;

  /* [AS] Use font-based pieces if needed */
  if( fontBitmapSquareSize >= 0 && (squareSize > 32 || gameInfo.variant >= VariantShogi)) {
    /* Create piece bitmaps, or do nothing if piece set is up to date */
    CreatePiecesFromFont();

    if( fontBitmapSquareSize == squareSize ) {
        int index = TranslatePieceToFontPiece(piece);

        SelectObject( tmphdc, hPieceMask[ index ] );

      if(appData.upsideDown ? color==flipView : (flipView && gameInfo.variant == VariantShogi))
        StretchBlt(hdc, x+squareSize, y+squareSize, -squareSize, -squareSize, tmphdc, 0, 0, squareSize, squareSize, SRCAND);
      else
        BitBlt( hdc,
            x, y,
            squareSize, squareSize,
            tmphdc,
            0, 0,
            SRCAND );

        SelectObject( tmphdc, hPieceFace[ index ] );

      if(appData.upsideDown ? color==flipView : (flipView && gameInfo.variant == VariantShogi))
        StretchBlt(hdc, x+squareSize, y+squareSize, -squareSize, -squareSize, tmphdc, 0, 0, squareSize, squareSize, SRCPAINT);
      else
        BitBlt( hdc,
            x, y,
            squareSize, squareSize,
            tmphdc,
            0, 0,
            SRCPAINT );

        return;
    }
  }

  if (appData.monoMode) {
    SelectObject(tmphdc, PieceBitmap(piece, 
      color == sqcolor ? OUTLINE_PIECE : SOLID_PIECE));
    BitBlt(hdc, x, y, squareSize, squareSize, tmphdc, 0, 0,
	   sqcolor ? SRCCOPY : NOTSRCCOPY);
  } else {
    HBRUSH xBrush = whitePieceBrush;
    tmpSize = squareSize;
    if(appData.pieceDirectory[0]) xBrush = GetStockObject(WHITE_BRUSH);
    if(minorSize &&
        ((piece >= (int)WhiteNightrider && piece <= WhiteGrasshopper) ||
         (piece >= (int)BlackNightrider && piece <= BlackGrasshopper))  ) {
      /* [HGM] no bitmap available for promoted pieces in Crazyhouse        */
      /* Bitmaps of smaller size are substituted, but we have to align them */
      x += (squareSize - minorSize)>>1;
      y += squareSize - minorSize - 2;
      tmpSize = minorSize;
    }
    if (color || appData.allWhite ) {
      oldBitmap = SelectObject(tmphdc, PieceBitmap(piece, WHITE_PIECE));
      if( color )
              oldBrush = SelectObject(hdc, xBrush);
      else    oldBrush = SelectObject(hdc, blackPieceBrush);
      if(appData.upsideDown && color==flipView)
        StretchBlt(hdc, x+tmpSize, y+tmpSize, -tmpSize, -tmpSize, tmphdc, 0, 0, tmpSize, tmpSize, 0x00B8074A);
      else
        BitBlt(hdc, x, y, tmpSize, tmpSize, tmphdc, 0, 0, 0x00B8074A);
      /* Use black for outline of white pieces */
      SelectObject(tmphdc, PieceBitmap(piece, OUTLINE_PIECE));
      if(appData.upsideDown && color==flipView)
        StretchBlt(hdc, x+tmpSize, y+tmpSize, -tmpSize, -tmpSize, tmphdc, 0, 0, tmpSize, tmpSize, SRCAND);
      else
        BitBlt(hdc, x, y, tmpSize, tmpSize, tmphdc, 0, 0, SRCAND);
    } else if(appData.pieceDirectory[0]) {
      oldBitmap = SelectObject(tmphdc, PieceBitmap(piece, WHITE_PIECE));
      oldBrush = SelectObject(hdc, xBrush);
      if(appData.upsideDown && color==flipView)
        StretchBlt(hdc, x+tmpSize, y+tmpSize, -tmpSize, -tmpSize, tmphdc, 0, 0, tmpSize, tmpSize, 0x00B8074A);
      else
        BitBlt(hdc, x, y, tmpSize, tmpSize, tmphdc, 0, 0, 0x00B8074A);
      SelectObject(tmphdc, PieceBitmap(piece, SOLID_PIECE));
      if(appData.upsideDown && color==flipView)
        StretchBlt(hdc, x+tmpSize, y+tmpSize, -tmpSize, -tmpSize, tmphdc, 0, 0, tmpSize, tmpSize, SRCAND);
      else
        BitBlt(hdc, x, y, tmpSize, tmpSize, tmphdc, 0, 0, SRCAND);
    } else {
      /* Use square color for details of black pieces */
      oldBitmap = SelectObject(tmphdc, PieceBitmap(piece, SOLID_PIECE));
      oldBrush = SelectObject(hdc, blackPieceBrush);
      if(appData.upsideDown && !flipView)
        StretchBlt(hdc, x+tmpSize, y+tmpSize, -tmpSize, -tmpSize, tmphdc, 0, 0, tmpSize, tmpSize, 0x00B8074A);
      else
        BitBlt(hdc, x, y, tmpSize, tmpSize, tmphdc, 0, 0, 0x00B8074A);
    }
    SelectObject(hdc, oldBrush);
    SelectObject(tmphdc, oldBitmap);
  }
}

/* [AS] Compute a drawing mode for a square, based on specified settings (see DrawTile) */
int GetBackTextureMode( int algo )
{
    int result = BACK_TEXTURE_MODE_DISABLED;

    switch( algo ) 
    {
        case BACK_TEXTURE_MODE_PLAIN:
            result = 1; /* Always use identity map */
            break;
        case BACK_TEXTURE_MODE_FULL_RANDOM:
            result = 1 + (myrandom() % 3); /* Pick a transformation at random */
            break;
    }

    return result;
}

/* 
    [AS] Compute and save texture drawing info, otherwise we may not be able
    to handle redraws cleanly (as random numbers would always be different).
*/
VOID RebuildTextureSquareInfo()
{
    BITMAP bi;
    int lite_w = 0;
    int lite_h = 0;
    int dark_w = 0;
    int dark_h = 0;
    int row;
    int col;

    ZeroMemory( &backTextureSquareInfo, sizeof(backTextureSquareInfo) );

    if( liteBackTexture != NULL ) {
        if( GetObject( liteBackTexture, sizeof(bi), &bi ) > 0 ) {
            lite_w = bi.bmWidth;
            lite_h = bi.bmHeight;
        }
    }

    if( darkBackTexture != NULL ) {
        if( GetObject( darkBackTexture, sizeof(bi), &bi ) > 0 ) {
            dark_w = bi.bmWidth;
            dark_h = bi.bmHeight;
        }
    }

    for( row=0; row<BOARD_HEIGHT; row++ ) {
        for( col=0; col<BOARD_WIDTH; col++ ) {
            if( (col + row) & 1 ) {
                /* Lite square */
                if( lite_w >= squareSize && lite_h >= squareSize ) {
                  if( lite_w >= squareSize*BOARD_WIDTH )
                    backTextureSquareInfo[row][col].x = (2*col+1)*lite_w/(2*BOARD_WIDTH) - squareSize/2;  /* [HGM] cut out of center of virtual square */
                  else
                    backTextureSquareInfo[row][col].x = col * (lite_w - squareSize) / (BOARD_WIDTH-1);  /* [HGM] divide by size-1 in stead of size! */
                  if( lite_h >= squareSize*BOARD_HEIGHT )
                    backTextureSquareInfo[row][col].y = (2*(BOARD_HEIGHT-row)-1)*lite_h/(2*BOARD_HEIGHT) - squareSize/2;
                  else
                    backTextureSquareInfo[row][col].y = (BOARD_HEIGHT-1-row) * (lite_h - squareSize) / (BOARD_HEIGHT-1);
                    backTextureSquareInfo[row][col].mode = GetBackTextureMode(liteBackTextureMode);
                }
            }
            else {
                /* Dark square */
                if( dark_w >= squareSize && dark_h >= squareSize ) {
                  if( dark_w >= squareSize*BOARD_WIDTH )
                    backTextureSquareInfo[row][col].x = (2*col+1) * dark_w / (2*BOARD_WIDTH) - squareSize/2;
                  else
                    backTextureSquareInfo[row][col].x = col * (dark_w - squareSize) / (BOARD_WIDTH-1);
                  if( dark_h >= squareSize*BOARD_HEIGHT )
                    backTextureSquareInfo[row][col].y = (2*(BOARD_HEIGHT-row)-1) * dark_h / (2*BOARD_HEIGHT) - squareSize/2;
                  else
                    backTextureSquareInfo[row][col].y = (BOARD_HEIGHT-1-row) * (dark_h - squareSize) / (BOARD_HEIGHT-1);
                    backTextureSquareInfo[row][col].mode = GetBackTextureMode(darkBackTextureMode);
                }
            }
        }
    }
}

/* [AS] Arrow highlighting support */

static double A_WIDTH = 5; /* Width of arrow body */

#define A_HEIGHT_FACTOR 6   /* Length of arrow "point", relative to body width */
#define A_WIDTH_FACTOR  3   /* Width of arrow "point", relative to body width */

static double Sqr( double x )
{
    return x*x;
}

static int Round( double x )
{
    return (int) (x + 0.5);
}

/* Draw an arrow between two points using current settings */
VOID DrawArrowBetweenPoints( HDC hdc, int s_x, int s_y, int d_x, int d_y )
{
    POINT arrow[7];
    double dx, dy, j, k, x, y;

    if( d_x == s_x ) {
        int h = (d_y > s_y) ? +A_WIDTH*A_HEIGHT_FACTOR : -A_WIDTH*A_HEIGHT_FACTOR;

        arrow[0].x = s_x + A_WIDTH + 0.5;
        arrow[0].y = s_y;

        arrow[1].x = s_x + A_WIDTH + 0.5;
        arrow[1].y = d_y - h;

        arrow[2].x = arrow[1].x + A_WIDTH*(A_WIDTH_FACTOR-1) + 0.5;
        arrow[2].y = d_y - h;

        arrow[3].x = d_x;
        arrow[3].y = d_y;

        arrow[5].x = arrow[1].x - 2*A_WIDTH + 0.5;
        arrow[5].y = d_y - h;

        arrow[4].x = arrow[5].x - A_WIDTH*(A_WIDTH_FACTOR-1) + 0.5;
        arrow[4].y = d_y - h;

        arrow[6].x = arrow[1].x - 2*A_WIDTH + 0.5;
        arrow[6].y = s_y;
    }
    else if( d_y == s_y ) {
        int w = (d_x > s_x) ? +A_WIDTH*A_HEIGHT_FACTOR : -A_WIDTH*A_HEIGHT_FACTOR;

        arrow[0].x = s_x;
        arrow[0].y = s_y + A_WIDTH + 0.5;

        arrow[1].x = d_x - w;
        arrow[1].y = s_y + A_WIDTH + 0.5;

        arrow[2].x = d_x - w;
        arrow[2].y = arrow[1].y + A_WIDTH*(A_WIDTH_FACTOR-1) + 0.5;

        arrow[3].x = d_x;
        arrow[3].y = d_y;

        arrow[5].x = d_x - w;
        arrow[5].y = arrow[1].y - 2*A_WIDTH + 0.5;

        arrow[4].x = d_x - w;
        arrow[4].y = arrow[5].y - A_WIDTH*(A_WIDTH_FACTOR-1) + 0.5;

        arrow[6].x = s_x;
        arrow[6].y = arrow[1].y - 2*A_WIDTH + 0.5;
    }
    else {
        /* [AS] Needed a lot of paper for this! :-) */
        dy = (double) (d_y - s_y) / (double) (d_x - s_x);
        dx = (double) (s_x - d_x) / (double) (s_y - d_y);
  
        j = sqrt( Sqr(A_WIDTH) / (1.0 + Sqr(dx)) );

        k = sqrt( Sqr(A_WIDTH*A_HEIGHT_FACTOR) / (1.0 + Sqr(dy)) );

        x = s_x;
        y = s_y;

        arrow[0].x = Round(x - j);
        arrow[0].y = Round(y + j*dx);

        arrow[1].x = Round(arrow[0].x + 2*j);   // [HGM] prevent width to be affected by rounding twice
        arrow[1].y = Round(arrow[0].y - 2*j*dx);

        if( d_x > s_x ) {
            x = (double) d_x - k;
            y = (double) d_y - k*dy;
        }
        else {
            x = (double) d_x + k;
            y = (double) d_y + k*dy;
        }

        x = Round(x); y = Round(y); // [HGM] make sure width of shaft is rounded the same way on both ends

        arrow[6].x = Round(x - j);
        arrow[6].y = Round(y + j*dx);

        arrow[2].x = Round(arrow[6].x + 2*j);
        arrow[2].y = Round(arrow[6].y - 2*j*dx);

        arrow[3].x = Round(arrow[2].x + j*(A_WIDTH_FACTOR-1));
        arrow[3].y = Round(arrow[2].y - j*(A_WIDTH_FACTOR-1)*dx);

        arrow[4].x = d_x;
        arrow[4].y = d_y;

        arrow[5].x = Round(arrow[6].x - j*(A_WIDTH_FACTOR-1));
        arrow[5].y = Round(arrow[6].y + j*(A_WIDTH_FACTOR-1)*dx);
    }

    Polygon( hdc, arrow, 7 );
}

/* [AS] Draw an arrow between two squares */
VOID DrawArrowBetweenSquares( HDC hdc, int s_col, int s_row, int d_col, int d_row )
{
    int s_x, s_y, d_x, d_y;
    HPEN hpen;
    HPEN holdpen;
    HBRUSH hbrush;
    HBRUSH holdbrush;
    LOGBRUSH stLB;

    if( s_col == d_col && s_row == d_row ) {
        return;
    }

    /* Get source and destination points */
    SquareToPos( s_row, s_col, &s_x, &s_y);
    SquareToPos( d_row, d_col, &d_x, &d_y);

    if( d_y > s_y ) {
        d_y += squareSize / 2 - squareSize / 4; // [HGM] round towards same centers on all sides!
    }
    else if( d_y < s_y ) {
        d_y += squareSize / 2 + squareSize / 4;
    }
    else {
        d_y += squareSize / 2;
    }

    if( d_x > s_x ) {
        d_x += squareSize / 2 - squareSize / 4;
    }
    else if( d_x < s_x ) {
        d_x += squareSize / 2 + squareSize / 4;
    }
    else {
        d_x += squareSize / 2;
    }

    s_x += squareSize / 2;
    s_y += squareSize / 2;

    /* Adjust width */
    A_WIDTH = squareSize / 14.; //[HGM] make float

    /* Draw */
    stLB.lbStyle = BS_SOLID;
    stLB.lbColor = appData.highlightArrowColor;
    stLB.lbHatch = 0;

    hpen = CreatePen( PS_SOLID, 2, RGB(0x00,0x00,0x00) );
    holdpen = SelectObject( hdc, hpen );
    hbrush = CreateBrushIndirect( &stLB );
    holdbrush = SelectObject( hdc, hbrush );

    DrawArrowBetweenPoints( hdc, s_x, s_y, d_x, d_y );

    SelectObject( hdc, holdpen );
    SelectObject( hdc, holdbrush );
    DeleteObject( hpen );
    DeleteObject( hbrush );
}

BOOL HasHighlightInfo()
{
    BOOL result = FALSE;

    if( highlightInfo.sq[0].x >= 0 && highlightInfo.sq[0].y >= 0 &&
        highlightInfo.sq[1].x >= 0 && highlightInfo.sq[1].y >= 0 )
    {
        result = TRUE;
    }

    return result;



}

BOOL IsDrawArrowEnabled()
{
    BOOL result = FALSE;

    if( appData.highlightMoveWithArrow && squareSize >= 32 ) {
        result = TRUE;
    }

    return result;
}

VOID DrawArrowHighlight( HDC hdc )
{
    if( IsDrawArrowEnabled() && HasHighlightInfo() ) {
        DrawArrowBetweenSquares( hdc,
            highlightInfo.sq[0].x, highlightInfo.sq[0].y,
            highlightInfo.sq[1].x, highlightInfo.sq[1].y );
    }
}

HRGN GetArrowHighlightClipRegion( HDC hdc )
{
    HRGN result = NULL;

    if( HasHighlightInfo() ) {
        int x1, y1, x2, y2;
        int sx, sy, dx, dy;

        SquareToPos(highlightInfo.sq[0].y, highlightInfo.sq[0].x, &x1, &y1 );
        SquareToPos(highlightInfo.sq[1].y, highlightInfo.sq[1].x, &x2, &y2 );

        sx = MIN( x1, x2 );
        sy = MIN( y1, y2 );
        dx = MAX( x1, x2 ) + squareSize;
        dy = MAX( y1, y2 ) + squareSize;

        result = CreateRectRgn( sx, sy, dx, dy );
    }

    return result;
}

/*
    Warning: this function modifies the behavior of several other functions. 
    
    Basically, Winboard is optimized to avoid drawing the whole board if not strictly
    needed. Unfortunately, the decision whether or not to perform a full or partial
    repaint is scattered all over the place, which is not good for features such as
    "arrow highlighting" that require a full repaint of the board.

    So, I've tried to patch the code where I thought it made sense (e.g. after or during
    user interaction, when speed is not so important) but especially to avoid errors
    in the displayed graphics.

    In such patched places, I always try refer to this function so there is a single
    place to maintain knowledge.
    
    To restore the original behavior, just return FALSE unconditionally.
*/
BOOL IsFullRepaintPreferrable()
{
    BOOL result = FALSE;

    if( (appData.highlightLastMove || appData.highlightDragging) && IsDrawArrowEnabled() ) {
        /* Arrow may appear on the board */
        result = TRUE;
    }

    return result;
}

/* 
    This function is called by DrawPosition to know whether a full repaint must
    be forced or not.

    Only DrawPosition may directly call this function, which makes use of 
    some state information. Other function should call DrawPosition specifying 
    the repaint flag, and can use IsFullRepaintPreferrable if needed.
*/
BOOL DrawPositionNeedsFullRepaint()
{
    BOOL result = FALSE;

    /* 
        Probably a slightly better policy would be to trigger a full repaint
        when animInfo.piece changes state (i.e. empty -> non-empty and viceversa),
        but animation is fast enough that it's difficult to notice.
    */
    if( animInfo.piece == EmptySquare ) {
        if( (appData.highlightLastMove || appData.highlightDragging) && IsDrawArrowEnabled() /*&& HasHighlightInfo()*/ ) {
            result = TRUE;
        }
    }

    return result;
}

static HBITMAP borderBitmap;

VOID
DrawBackgroundOnDC(HDC hdc)
{
  
  BITMAP bi;
  HDC tmphdc;
  HBITMAP hbm;
  static char oldBorder[MSG_SIZ];
  int w = 600, h = 600, mode;

  if(strcmp(appData.border, oldBorder)) { // load new one when old one no longer valid
    strncpy(oldBorder, appData.border, MSG_SIZ-1);
    borderBitmap = LoadImage( 0, appData.border, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE );	
  }
  if(borderBitmap == NULL) { // loading failed, use white
    FillRect( hdc, &boardRect, whitePieceBrush );
    return;
  }
  tmphdc = CreateCompatibleDC(hdc);
  hbm = SelectObject(tmphdc, borderBitmap);
  if( GetObject( borderBitmap, sizeof(bi), &bi ) > 0 ) {
            w = bi.bmWidth;
            h = bi.bmHeight;
  }
  mode = SetStretchBltMode(hdc, COLORONCOLOR);
  StretchBlt(hdc, boardRect.left, boardRect.top, boardRect.right - boardRect.left, 
                  boardRect.bottom - boardRect.top, tmphdc, 0, 0, w, h, SRCCOPY);
  SetStretchBltMode(hdc, mode);
  SelectObject(tmphdc, hbm);
  DeleteDC(tmphdc);
}

VOID
DrawBoardOnDC(HDC hdc, Board board, HDC tmphdc)
{
  int row, column, x, y, square_color, piece_color;
  ChessSquare piece;
  HBRUSH oldBrush;
  HDC texture_hdc = NULL;

  /* [AS] Initialize background textures if needed */
  if( liteBackTexture != NULL || darkBackTexture != NULL ) {
      static int backTextureBoardSize; /* [HGM] boardsize: also new texture if board format changed */
      if( backTextureSquareSize != squareSize 
       || backTextureBoardSize != BOARD_WIDTH+BOARD_FILES*BOARD_HEIGHT) {
	  backTextureBoardSize = BOARD_WIDTH+BOARD_FILES*BOARD_HEIGHT;
          backTextureSquareSize = squareSize;
          RebuildTextureSquareInfo();
      }

      texture_hdc = CreateCompatibleDC( hdc );
  }

  for (row = 0; row < BOARD_HEIGHT; row++) {
    for (column = 0; column < BOARD_WIDTH; column++) {
  
      SquareToPos(row, column, &x, &y);

      piece = board[row][column];

      square_color = ((column + row) % 2) == 1;
      if( gameInfo.variant == VariantXiangqi ) {
          square_color = !InPalace(row, column);
          if(BOARD_HEIGHT&1) { if(row==BOARD_HEIGHT/2) square_color ^= 1; }
          else if(row < BOARD_HEIGHT/2) square_color ^= 1;
      }
      piece_color = (int) piece < (int) BlackPawn;


      /* [HGM] holdings file: light square or black */
      if(column == BOARD_LEFT-2) {
            if( row > BOARD_HEIGHT - gameInfo.holdingsSize - 1 )
                square_color = 1;
            else {
                DisplayHoldingsCount(hdc, x, y, 0, 0); /* black out */
                continue;
            }
      } else
      if(column == BOARD_RGHT + 1 ) {
            if( row < gameInfo.holdingsSize )
                square_color = 1;
            else {
                DisplayHoldingsCount(hdc, x, y, 0, 0); 
                continue;
            }
      }
      if(column == BOARD_LEFT-1 ) /* left align */
            DisplayHoldingsCount(hdc, x, y, flipView, (int) board[row][column]);
      else if( column == BOARD_RGHT) /* right align */
            DisplayHoldingsCount(hdc, x, y, !flipView, (int) board[row][column]);
      else if( piece == DarkSquare) DisplayHoldingsCount(hdc, x, y, 0, 0);
      else
      if (appData.monoMode) {
        if (piece == EmptySquare) {
          BitBlt(hdc, x, y, squareSize, squareSize, 0, 0, 0,
		 square_color ? WHITENESS : BLACKNESS);
        } else {
          DrawPieceOnDC(hdc, piece, piece_color, square_color, x, y, tmphdc);
        }
      } 
      else if( appData.useBitmaps && backTextureSquareInfo[row][column].mode > 0 ) {
          /* [AS] Draw the square using a texture bitmap */
          HBITMAP hbm = SelectObject( texture_hdc, square_color ? liteBackTexture : darkBackTexture );
	  int r = row, c = column; // [HGM] do not flip board in flipView
	  if(flipView) { r = BOARD_HEIGHT-1 - r; c = BOARD_WIDTH-1 - c; }

          DrawTile( x, y, 
              squareSize, squareSize, 
              hdc, 
              texture_hdc,
              backTextureSquareInfo[r][c].mode,
              backTextureSquareInfo[r][c].x,
              backTextureSquareInfo[r][c].y );

          SelectObject( texture_hdc, hbm );

          if (piece != EmptySquare) {
              DrawPieceOnDC(hdc, piece, piece_color, -1, x, y, tmphdc);
          }
      }
      else {
        HBRUSH brush = square_color ? lightSquareBrush : darkSquareBrush;

        oldBrush = SelectObject(hdc, brush );
        BitBlt(hdc, x, y, squareSize, squareSize, 0, 0, 0, PATCOPY);
        SelectObject(hdc, oldBrush);
        if (piece != EmptySquare)
          DrawPieceOnDC(hdc, piece, piece_color, -1, x, y, tmphdc);
      }
    }
  }

  if( texture_hdc != NULL ) {
    DeleteDC( texture_hdc );
  }
}

int saveDiagFlag = 0; FILE *diagFile; // [HGM] diag
void fputDW(FILE *f, int x)
{
	fputc(x     & 255, f);
	fputc(x>>8  & 255, f);
	fputc(x>>16 & 255, f);
	fputc(x>>24 & 255, f);
}

#define MAX_CLIPS 200   /* more than enough */

VOID
DrawLogoOnDC(HDC hdc, RECT logoRect, HBITMAP logo)
{
//  HBITMAP bufferBitmap;
  BITMAP bi;
//  RECT Rect;
  HDC tmphdc;
  HBITMAP hbm;
  int w = 100, h = 50;

  if(logo == NULL) {
    if(!logoHeight) return;
    FillRect( hdc, &logoRect, whitePieceBrush );
  }
//  GetClientRect(hwndMain, &Rect);
//  bufferBitmap = CreateCompatibleBitmap(hdc, Rect.right-Rect.left+1,
//					Rect.bottom-Rect.top+1);
  tmphdc = CreateCompatibleDC(hdc);
  hbm = SelectObject(tmphdc, logo);
  if( GetObject( logo, sizeof(bi), &bi ) > 0 ) {
            w = bi.bmWidth;
            h = bi.bmHeight;
  }
  StretchBlt(hdc, logoRect.left, logoRect.top, logoRect.right - logoRect.left, 
                  logoRect.bottom - logoRect.top, tmphdc, 0, 0, w, h, SRCCOPY);
  SelectObject(tmphdc, hbm);
  DeleteDC(tmphdc);
}

VOID
DisplayLogos()
{
  if(logoHeight) {
	HDC hdc = GetDC(hwndMain);
	HBITMAP whiteLogo = (HBITMAP) first.programLogo, blackLogo = (HBITMAP) second.programLogo;
	if(appData.autoLogo) {
	  
	  switch(gameMode) { // pick logos based on game mode
	    case IcsObserving:
		whiteLogo = second.programLogo; // ICS logo
		blackLogo = second.programLogo;
	    default:
		break;
	    case IcsPlayingWhite:
		if(!appData.zippyPlay) whiteLogo = userLogo;
		blackLogo = second.programLogo; // ICS logo
		break;
	    case IcsPlayingBlack:
		whiteLogo = second.programLogo; // ICS logo
		blackLogo = appData.zippyPlay ? first.programLogo : userLogo;
		break;
	    case TwoMachinesPlay:
	        if(first.twoMachinesColor[0] == 'b') {
		    whiteLogo = second.programLogo;
		    blackLogo = first.programLogo;
		}
		break;
	    case MachinePlaysWhite:
		blackLogo = userLogo;
		break;
	    case MachinePlaysBlack:
		whiteLogo = userLogo;
		blackLogo = first.programLogo;
	  }
	}
	DrawLogoOnDC(hdc, leftLogoRect, flipClock ? blackLogo : whiteLogo);
	DrawLogoOnDC(hdc, rightLogoRect, flipClock ? whiteLogo : blackLogo);
	ReleaseDC(hwndMain, hdc);
  }
}

void
UpdateLogos(int display)
{ // called after loading new engine(s), in tourney or from menu
  LoadLogo(&first, 0, FALSE);
  LoadLogo(&second, 1, appData.icsActive);
  InitDrawingSizes(-2, 0); // adapt layout of board window to presence/absence of logos
  if(display) DisplayLogos();
}

static HDC hdcSeek;

// [HGM] seekgraph
void DrawSeekAxis( int x, int y, int xTo, int yTo )
{
    POINT stPt;
    HPEN hp = SelectObject( hdcSeek, gridPen );
    MoveToEx( hdcSeek, boardRect.left+x, boardRect.top+y, &stPt );
    LineTo( hdcSeek, boardRect.left+xTo, boardRect.top+yTo );
    SelectObject( hdcSeek, hp );
}

// front-end wrapper for drawing functions to do rectangles
void DrawSeekBackground( int left, int top, int right, int bottom )
{
    HPEN hp;
    RECT rc;

    if (hdcSeek == NULL) {
    hdcSeek = GetDC(hwndMain);
      if (!appData.monoMode) {
        SelectPalette(hdcSeek, hPal, FALSE);
        RealizePalette(hdcSeek);
      }
    }
    hp = SelectObject( hdcSeek, gridPen );
    rc.top = boardRect.top+top; rc.left = boardRect.left+left; 
    rc.bottom = boardRect.top+bottom; rc.right = boardRect.left+right;
    FillRect( hdcSeek, &rc, lightSquareBrush );
    SelectObject( hdcSeek, hp );
}

// front-end wrapper for putting text in graph
void DrawSeekText(char *buf, int x, int y)
{
        SIZE stSize;
	SetBkMode( hdcSeek, TRANSPARENT );
        GetTextExtentPoint32( hdcSeek, buf, strlen(buf), &stSize );
        TextOut( hdcSeek, boardRect.left+x-3, boardRect.top+y-stSize.cy/2, buf, strlen(buf) );
}

void DrawSeekDot(int x, int y, int color)
{
	int square = color & 0x80;
	HBRUSH oldBrush = SelectObject(hdcSeek, 
			color == 0 ? markerBrush[1] : color == 1 ? darkSquareBrush : explodeBrush);
	color &= 0x7F;
	if(square)
	    Rectangle(hdcSeek, boardRect.left+x - squareSize/9, boardRect.top+y - squareSize/9,
			       boardRect.left+x + squareSize/9, boardRect.top+y + squareSize/9);
	else
	    Ellipse(hdcSeek, boardRect.left+x - squareSize/8, boardRect.top+y - squareSize/8,
			     boardRect.left+x + squareSize/8, boardRect.top+y + squareSize/8);
	    SelectObject(hdcSeek, oldBrush);
}

void DrawSeekOpen()
{
}

void DrawSeekClose()
{
}

VOID
HDCDrawPosition(HDC hdc, BOOLEAN repaint, Board board)
{
  static Board lastReq[2], lastDrawn[2];
  static HighlightInfo lastDrawnHighlight, lastDrawnPremove;
  static int lastDrawnFlipView = 0;
  static int lastReqValid[2] = {0, 0}, lastDrawnValid[2] = {0, 0};
  int releaseDC, x, y, x2, y2, row, column, num_clips = 0, i;
  HDC tmphdc;
  HDC hdcmem;
  HBITMAP bufferBitmap;
  HBITMAP oldBitmap;
  RECT Rect;
  HRGN clips[MAX_CLIPS];
  ChessSquare dragged_piece = EmptySquare;
  int nr = twoBoards*partnerUp;

  /* I'm undecided on this - this function figures out whether a full
   * repaint is necessary on its own, so there's no real reason to have the
   * caller tell it that.  I think this can safely be set to FALSE - but
   * if we trust the callers not to request full repaints unnessesarily, then
   * we could skip some clipping work.  In other words, only request a full
   * redraw when the majority of pieces have changed positions (ie. flip, 
   * gamestart and similar)  --Hawk
   */
  Boolean fullrepaint = repaint;

  if(DrawSeekGraph()) return; // [HG} seekgraph: suppress printing board if seek graph up

  if( DrawPositionNeedsFullRepaint() ) {
      fullrepaint = TRUE;
  }

  if (board == NULL) {
    if (!lastReqValid[nr]) {
      return;
    }
    board = lastReq[nr];
  } else {
    CopyBoard(lastReq[nr], board);
    lastReqValid[nr] = 1;
  }

  if (doingSizing) {
    return;
  }

  if (IsIconic(hwndMain)) {
    return;
  }

  if (hdc == NULL) {
    hdc = GetDC(hwndMain);
    if (!appData.monoMode) {
      SelectPalette(hdc, hPal, FALSE);
      RealizePalette(hdc);
    }
    releaseDC = TRUE;
  } else {
    releaseDC = FALSE;
  }

  /* Create some work-DCs */
  hdcmem = CreateCompatibleDC(hdc);
  tmphdc = CreateCompatibleDC(hdc);

  /* If dragging is in progress, we temporarely remove the piece */
  /* [HGM] or temporarily decrease count if stacked              */
  /*       !! Moved to before board compare !!                   */
  if (dragInfo.from.x >= 0 && dragInfo.pos.x >= 0) {
    dragged_piece = board[dragInfo.from.y][dragInfo.from.x];
    if(dragInfo.from.x == BOARD_LEFT-2 ) {
            if(--board[dragInfo.from.y][dragInfo.from.x+1] == 0 )
        board[dragInfo.from.y][dragInfo.from.x] = EmptySquare;
    } else 
    if(dragInfo.from.x == BOARD_RGHT+1) {
            if(--board[dragInfo.from.y][dragInfo.from.x-1] == 0 )
        board[dragInfo.from.y][dragInfo.from.x] = EmptySquare;
    } else 
        board[dragInfo.from.y][dragInfo.from.x] = gatingPiece;
  }

  /* Figure out which squares need updating by comparing the 
   * newest board with the last drawn board and checking if
   * flipping has changed.
   */
  if (!fullrepaint && lastDrawnValid[nr] && (nr == 1 || lastDrawnFlipView == flipView)) {
    for (row = 0; row < BOARD_HEIGHT; row++) { /* [HGM] true size, not 8 */
      for (column = 0; column < BOARD_WIDTH; column++) {
	if (lastDrawn[nr][row][column] != board[row][column]) {
	  SquareToPos(row, column, &x, &y);
	  clips[num_clips++] =
	    CreateRectRgn(x, y, x + squareSize, y + squareSize);
	}
      }
    }
   if(nr == 0) { // [HGM] dual: no highlights on second board
    for (i=0; i<2; i++) {
      if (lastDrawnHighlight.sq[i].x != highlightInfo.sq[i].x ||
	  lastDrawnHighlight.sq[i].y != highlightInfo.sq[i].y) {
	if (lastDrawnHighlight.sq[i].x >= 0 &&
	    lastDrawnHighlight.sq[i].y >= 0) {
	  SquareToPos(lastDrawnHighlight.sq[i].y,
		      lastDrawnHighlight.sq[i].x, &x, &y);
	  clips[num_clips++] =
	    CreateRectRgn(x - lineGap, y - lineGap, 
	                  x + squareSize + lineGap, y + squareSize + lineGap);
	}
	if (highlightInfo.sq[i].x >= 0 && highlightInfo.sq[i].y >= 0) {
	  SquareToPos(highlightInfo.sq[i].y, highlightInfo.sq[i].x, &x, &y);
	  clips[num_clips++] =
	    CreateRectRgn(x - lineGap, y - lineGap, 
	                  x + squareSize + lineGap, y + squareSize + lineGap);
	}
      }
    }
    for (i=0; i<2; i++) {
      if (lastDrawnPremove.sq[i].x != premoveHighlightInfo.sq[i].x ||
	  lastDrawnPremove.sq[i].y != premoveHighlightInfo.sq[i].y) {
	if (lastDrawnPremove.sq[i].x >= 0 &&
	    lastDrawnPremove.sq[i].y >= 0) {
	  SquareToPos(lastDrawnPremove.sq[i].y,
		      lastDrawnPremove.sq[i].x, &x, &y);
	  clips[num_clips++] =
	    CreateRectRgn(x - lineGap, y - lineGap, 
	                  x + squareSize + lineGap, y + squareSize + lineGap);
	}
	if (premoveHighlightInfo.sq[i].x >= 0 && 
	    premoveHighlightInfo.sq[i].y >= 0) {
	  SquareToPos(premoveHighlightInfo.sq[i].y, 
		      premoveHighlightInfo.sq[i].x, &x, &y);
	  clips[num_clips++] =
	    CreateRectRgn(x - lineGap, y - lineGap, 
	                  x + squareSize + lineGap, y + squareSize + lineGap);
	}
      }
    }
   } else { // nr == 1
	partnerHighlightInfo.sq[0].y = board[EP_STATUS-4];
	partnerHighlightInfo.sq[0].x = board[EP_STATUS-3];
	partnerHighlightInfo.sq[1].y = board[EP_STATUS-2];
	partnerHighlightInfo.sq[1].x = board[EP_STATUS-1];
      for (i=0; i<2; i++) {
	if (partnerHighlightInfo.sq[i].x >= 0 &&
	    partnerHighlightInfo.sq[i].y >= 0) {
	  SquareToPos(partnerHighlightInfo.sq[i].y,
		      partnerHighlightInfo.sq[i].x, &x, &y);
	  clips[num_clips++] =
	    CreateRectRgn(x - lineGap, y - lineGap, 
	                  x + squareSize + lineGap, y + squareSize + lineGap);
	}
	if (oldPartnerHighlight.sq[i].x >= 0 && 
	    oldPartnerHighlight.sq[i].y >= 0) {
	  SquareToPos(oldPartnerHighlight.sq[i].y, 
		      oldPartnerHighlight.sq[i].x, &x, &y);
	  clips[num_clips++] =
	    CreateRectRgn(x - lineGap, y - lineGap, 
	                  x + squareSize + lineGap, y + squareSize + lineGap);
	}
      }
   }
  } else {
    fullrepaint = TRUE;
  }

  /* Create a buffer bitmap - this is the actual bitmap
   * being written to.  When all the work is done, we can
   * copy it to the real DC (the screen).  This avoids
   * the problems with flickering.
   */
  GetClientRect(hwndMain, &Rect);
  bufferBitmap = CreateCompatibleBitmap(hdc, Rect.right-Rect.left+1,
					Rect.bottom-Rect.top+1);
  oldBitmap = SelectObject(hdcmem, bufferBitmap);
  if (!appData.monoMode) {
    SelectPalette(hdcmem, hPal, FALSE);
  }

  /* Create clips for dragging */
  if (!fullrepaint) {
    if (dragInfo.from.x >= 0) {
      SquareToPos(dragInfo.from.y, dragInfo.from.x, &x, &y);
      clips[num_clips++] = CreateRectRgn(x, y, x+squareSize, y+squareSize);
    }
    if (dragInfo.start.x >= 0) {
      SquareToPos(dragInfo.start.y, dragInfo.start.x, &x, &y);
      clips[num_clips++] = CreateRectRgn(x, y, x+squareSize, y+squareSize);
    }
    if (dragInfo.pos.x >= 0) {
      x = dragInfo.pos.x - squareSize / 2;
      y = dragInfo.pos.y - squareSize / 2;
      clips[num_clips++] = CreateRectRgn(x, y, x+squareSize, y+squareSize);
    }
    if (dragInfo.lastpos.x >= 0) {
      x = dragInfo.lastpos.x - squareSize / 2;
      y = dragInfo.lastpos.y - squareSize / 2;
      clips[num_clips++] = CreateRectRgn(x, y, x+squareSize, y+squareSize);
    }
  }

  /* Are we animating a move?  
   * If so, 
   *   - remove the piece from the board (temporarely)
   *   - calculate the clipping region
   */
  if (!fullrepaint) {
    if (animInfo.piece != EmptySquare) {
      board[animInfo.from.y][animInfo.from.x] = EmptySquare;
      x = boardRect.left + animInfo.lastpos.x;
      y = boardRect.top + animInfo.lastpos.y;
      x2 = boardRect.left + animInfo.pos.x;
      y2 = boardRect.top + animInfo.pos.y;
      clips[num_clips++] = CreateRectRgn(MIN(x,x2), MIN(y,y2), MAX(x,x2)+squareSize, MAX(y,y2)+squareSize);
      /* Slight kludge.  The real problem is that after AnimateMove is
	 done, the position on the screen does not match lastDrawn.
	 This currently causes trouble only on e.p. captures in
	 atomic, where the piece moves to an empty square and then
	 explodes.  The old and new positions both had an empty square
	 at the destination, but animation has drawn a piece there and
	 we have to remember to erase it. [HGM] moved until after setting lastDrawn */
      lastDrawn[0][animInfo.to.y][animInfo.to.x] = animInfo.piece;
    }
  }

  /* No clips?  Make sure we have fullrepaint set to TRUE */
  if (num_clips == 0)
    fullrepaint = TRUE;

  /* Set clipping on the memory DC */
  if (!fullrepaint) {
    SelectClipRgn(hdcmem, clips[0]);
    for (x = 1; x < num_clips; x++) {
      if (ExtSelectClipRgn(hdcmem, clips[x], RGN_OR) == ERROR)
        abort();  // this should never ever happen!
    }
  }

  /* Do all the drawing to the memory DC */
  if(explodeInfo.radius) { // [HGM] atomic
	HBRUSH oldBrush;
	int x, y, r=(explodeInfo.radius * squareSize)/100;
        ChessSquare piece = board[explodeInfo.fromY][explodeInfo.fromX];
        board[explodeInfo.fromY][explodeInfo.fromX] = EmptySquare; // suppress display of capturer
	SquareToPos(explodeInfo.toY, explodeInfo.toX, &x, &y);
	x += squareSize/2;
	y += squareSize/2;
        if(!fullrepaint) {
	  clips[num_clips] = CreateRectRgn(x-r, y-r, x+r, y+r);
	  ExtSelectClipRgn(hdcmem, clips[num_clips++], RGN_OR);
	}
	DrawGridOnDC(hdcmem);
	DrawHighlightsOnDC(hdcmem, &highlightInfo, HIGHLIGHT_PEN);
	DrawHighlightsOnDC(hdcmem, &premoveHighlightInfo, PREMOVE_PEN);
	DrawBoardOnDC(hdcmem, board, tmphdc);
        board[explodeInfo.fromY][explodeInfo.fromX] = piece;
	oldBrush = SelectObject(hdcmem, explodeBrush);
	Ellipse(hdcmem, x-r, y-r, x+r, y+r);
	SelectObject(hdcmem, oldBrush);
  } else {
    if(border) DrawBackgroundOnDC(hdcmem);
    DrawGridOnDC(hdcmem);
    if(nr == 0) { // [HGM] dual: decide which highlights to draw
	DrawHighlightsOnDC(hdcmem, &highlightInfo, HIGHLIGHT_PEN);
	DrawHighlightsOnDC(hdcmem, &premoveHighlightInfo, PREMOVE_PEN);
    } else {
	DrawHighlightsOnDC(hdcmem, &partnerHighlightInfo, HIGHLIGHT_PEN);
	oldPartnerHighlight = partnerHighlightInfo;
    }
    DrawBoardOnDC(hdcmem, board, tmphdc);
  }
  if(nr == 0) // [HGM] dual: markers only on left board
  for (row = 0; row < BOARD_HEIGHT; row++) {
    for (column = 0; column < BOARD_WIDTH; column++) {
	if (marker[row][column]) { // marker changes only occur with full repaint!
	    HBRUSH oldBrush = SelectObject(hdcmem, markerBrush[marker[row][column]-1]);
	    SquareToPos(row, column, &x, &y);
	    Ellipse(hdcmem, x + squareSize/4, y + squareSize/4,
		  	  x + 3*squareSize/4, y + 3*squareSize/4);
	    SelectObject(hdcmem, oldBrush);
	}
    }
  }

  if( appData.highlightMoveWithArrow ) {
    DrawArrowHighlight(hdcmem);
  }

  DrawCoordsOnDC(hdcmem);

  CopyBoard(lastDrawn[nr], board); /* [HGM] Moved to here from end of routine, */
                 /* to make sure lastDrawn contains what is actually drawn */

  /* Put the dragged piece back into place and draw it (out of place!) */
    if (dragged_piece != EmptySquare) {
    /* [HGM] or restack */
    if(dragInfo.from.x == BOARD_LEFT-2 )
                 board[dragInfo.from.y][dragInfo.from.x+1]++;
    else
    if(dragInfo.from.x == BOARD_RGHT+1 )
                 board[dragInfo.from.y][dragInfo.from.x-1]++;

    board[dragInfo.from.y][dragInfo.from.x] = dragged_piece;
    x = dragInfo.pos.x - squareSize / 2;
    y = dragInfo.pos.y - squareSize / 2;
    DrawPieceOnDC(hdcmem, dragInfo.piece,
		  ((int) dragInfo.piece < (int) BlackPawn), 
                  (dragInfo.from.y + dragInfo.from.x) % 2, x, y, tmphdc);
  }   
  
  /* Put the animated piece back into place and draw it */
  if (animInfo.piece != EmptySquare) {
    board[animInfo.from.y][animInfo.from.x]  = animInfo.piece;
    x = boardRect.left + animInfo.pos.x;
    y = boardRect.top + animInfo.pos.y;
    DrawPieceOnDC(hdcmem, animInfo.piece,
		  ((int) animInfo.piece < (int) BlackPawn),
                  (animInfo.from.y + animInfo.from.x) % 2, x, y, tmphdc);
  }

  /* Release the bufferBitmap by selecting in the old bitmap 
   * and delete the memory DC
   */
  SelectObject(hdcmem, oldBitmap);
  DeleteDC(hdcmem);

  /* Set clipping on the target DC */
  if (!fullrepaint) {
    if(nr == 1) for (x = 0; x < num_clips; x++) { // [HGM] dual: translate clips
	RECT rect;
	GetRgnBox(clips[x], &rect);
	DeleteObject(clips[x]);
	clips[x] = CreateRectRgn(rect.left + wpMain.width/2, rect.top, 
	                  rect.right + wpMain.width/2, rect.bottom);
    }
    SelectClipRgn(hdc, clips[0]);
    for (x = 1; x < num_clips; x++) {
      if (ExtSelectClipRgn(hdc, clips[x], RGN_OR) == ERROR)
        abort();   // this should never ever happen!
    } 
  }

  /* Copy the new bitmap onto the screen in one go.
   * This way we avoid any flickering
   */
  oldBitmap = SelectObject(tmphdc, bufferBitmap);
  BitBlt(hdc, boardRect.left + twoBoards*partnerUp*wpMain.width/2, boardRect.top, // [HGM] dual
	 boardRect.right - boardRect.left,
	 boardRect.bottom - boardRect.top,
	 tmphdc, boardRect.left, boardRect.top, SRCCOPY);
  if(saveDiagFlag) { 
    BITMAP b; int i, j=0, m, w, wb, fac=0; char *pData; 
    BITMAPINFOHEADER bih; int color[16], nrColors=0;

    GetObject(bufferBitmap, sizeof(b), &b);
    if(pData = malloc(b.bmWidthBytes*b.bmHeight + 10000)) {
	bih.biSize = sizeof(BITMAPINFOHEADER);
	bih.biWidth = b.bmWidth;
	bih.biHeight = b.bmHeight;
	bih.biPlanes = 1;
	bih.biBitCount = b.bmBitsPixel;
	bih.biCompression = 0;
	bih.biSizeImage = b.bmWidthBytes*b.bmHeight;
	bih.biXPelsPerMeter = 0;
	bih.biYPelsPerMeter = 0;
	bih.biClrUsed = 0;
	bih.biClrImportant = 0;
//	fprintf(diagFile, "t=%d\nw=%d\nh=%d\nB=%d\nP=%d\nX=%d\n", 
//		b.bmType,  b.bmWidth,  b.bmHeight, b.bmWidthBytes,  b.bmPlanes,  b.bmBitsPixel);
	GetDIBits(tmphdc,bufferBitmap,0,b.bmHeight,pData,(BITMAPINFO*)&bih,DIB_RGB_COLORS);
//	fprintf(diagFile, "%8x\n", (int) pData);

	wb = b.bmWidthBytes;
	// count colors
	for(i=0; i<wb*(b.bmHeight - boardRect.top + OUTER_MARGIN)>>2; i++) {
		int k = ((int*) pData)[i];
		for(j=0; j<nrColors; j++) if(color[j] == k) break;
		if(j >= 16) break;
		color[j] = k;
		if(j >= nrColors) nrColors = j+1;
	}
	if(j<16) { // 16 colors is enough. Compress to 4 bits per pixel
		INT p = 0;
		for(i=0; i<b.bmHeight - boardRect.top + OUTER_MARGIN; i++) {
		    for(w=0; w<(wb>>2); w+=2) {
			int k = ((int*) pData)[(wb*i>>2) + w];
			for(j=0; j<nrColors; j++) if(color[j] == k) break;
			k = ((int*) pData)[(wb*i>>2) + w + 1];
			for(m=0; m<nrColors; m++) if(color[m] == k) break;
			pData[p++] = m | j<<4;
		    }
		    while(p&3) pData[p++] = 0;
		}
		fac = 3;
		wb = ((wb+31)>>5)<<2;
	}
	// write BITMAPFILEHEADER
	fprintf(diagFile, "BM");
        fputDW(diagFile, wb*(b.bmHeight - boardRect.top + OUTER_MARGIN)+0x36 + (fac?64:0));
        fputDW(diagFile, 0);
        fputDW(diagFile, 0x36 + (fac?64:0));
	// write BITMAPINFOHEADER
        fputDW(diagFile, 40);
        fputDW(diagFile, b.bmWidth);
        fputDW(diagFile, b.bmHeight - boardRect.top + OUTER_MARGIN);
	if(fac) fputDW(diagFile, 0x040001);   // planes and bits/pixel
        else    fputDW(diagFile, 0x200001);   // planes and bits/pixel
        fputDW(diagFile, 0);
        fputDW(diagFile, 0);
        fputDW(diagFile, 0);
        fputDW(diagFile, 0);
        fputDW(diagFile, 0);
        fputDW(diagFile, 0);
	// write color table
	if(fac)
	for(i=0; i<16; i++) fputDW(diagFile, color[i]);
	// write bitmap data
	for(i=0; i<wb*(b.bmHeight - boardRect.top + OUTER_MARGIN); i++) 
		fputc(pData[i], diagFile);
	free(pData);
     }
  }

  SelectObject(tmphdc, oldBitmap);

  /* Massive cleanup */
  for (x = 0; x < num_clips; x++)
    DeleteObject(clips[x]);

  DeleteDC(tmphdc);
  DeleteObject(bufferBitmap);

  if (releaseDC) 
    ReleaseDC(hwndMain, hdc);
  
  if (lastDrawnFlipView != flipView && nr == 0) {
    if (flipView)
      CheckMenuItem(GetMenu(hwndMain),IDM_FlipView, MF_BYCOMMAND|MF_CHECKED);
    else
      CheckMenuItem(GetMenu(hwndMain),IDM_FlipView, MF_BYCOMMAND|MF_UNCHECKED);
  }

/*  CopyBoard(lastDrawn, board);*/
  lastDrawnHighlight = highlightInfo;
  lastDrawnPremove   = premoveHighlightInfo;
  lastDrawnFlipView = flipView;
  lastDrawnValid[nr] = 1;
}

/* [HGM] diag: Save the current board display to the given open file and close the file */
int
SaveDiagram(f)
     FILE *f;
{
    saveDiagFlag = 1; diagFile = f;
    HDCDrawPosition(NULL, TRUE, NULL);
    saveDiagFlag = 0;

    fclose(f);
    return TRUE;
}


/*---------------------------------------------------------------------------*\
| CLIENT PAINT PROCEDURE
|   This is the main event-handler for the WM_PAINT message.
|
\*---------------------------------------------------------------------------*/
VOID
PaintProc(HWND hwnd)
{
  HDC         hdc;
  PAINTSTRUCT ps;
  HFONT       oldFont;

  if((hdc = BeginPaint(hwnd, &ps))) {
    if (IsIconic(hwnd)) {
      DrawIcon(hdc, 2, 2, iconCurrent);
    } else {
      if (!appData.monoMode) {
	SelectPalette(hdc, hPal, FALSE);
	RealizePalette(hdc);
      }
      HDCDrawPosition(hdc, 1, NULL);
      if(twoBoards) { // [HGM] dual: also redraw other board in other orientation
	flipView = !flipView; partnerUp = !partnerUp;
	HDCDrawPosition(hdc, 1, NULL);
	flipView = !flipView; partnerUp = !partnerUp;
      }
      oldFont =
	SelectObject(hdc, font[boardSize][MESSAGE_FONT]->hf);
      ExtTextOut(hdc, messageRect.left, messageRect.top,
		 ETO_CLIPPED|ETO_OPAQUE,
		 &messageRect, messageText, strlen(messageText), NULL);
      SelectObject(hdc, oldFont);
      DisplayBothClocks();
      DisplayLogos();
    }
    EndPaint(hwnd,&ps);
  }

  return;
}


/*
 * If the user selects on a border boundary, return -1; if off the board,
 *   return -2.  Otherwise map the event coordinate to the square.
 * The offset boardRect.left or boardRect.top must already have been
 *   subtracted from x.
 */
int EventToSquare(x, limit)
     int x, limit;
{
  if (x <= border)
    return -2;
  if (x < lineGap + border)
    return -1;
  x -= lineGap + border;
  if ((x % (squareSize + lineGap)) >= squareSize)
    return -1;
  x /= (squareSize + lineGap);
    if (x >= limit)
    return -2;
  return x;
}

typedef struct {
  char piece;
  int command;
  char* name;
} DropEnable;

DropEnable dropEnables[] = {
  { 'P', DP_Pawn, N_("Pawn") },
  { 'N', DP_Knight, N_("Knight") },
  { 'B', DP_Bishop, N_("Bishop") },
  { 'R', DP_Rook, N_("Rook") },
  { 'Q', DP_Queen, N_("Queen") },
};

VOID
SetupDropMenu(HMENU hmenu)
{
  int i, count, enable;
  char *p;
  extern char white_holding[], black_holding[];
  char item[MSG_SIZ];

  for (i=0; i<sizeof(dropEnables)/sizeof(DropEnable); i++) {
    p = strchr(gameMode == IcsPlayingWhite ? white_holding : black_holding,
	       dropEnables[i].piece);
    count = 0;
    while (p && *p++ == dropEnables[i].piece) count++;
      snprintf(item, MSG_SIZ, "%s  %d", T_(dropEnables[i].name), count);
    enable = count > 0 || !appData.testLegality
      /*!!temp:*/ || (gameInfo.variant == VariantCrazyhouse
		      && !appData.icsActive);
    ModifyMenu(hmenu, dropEnables[i].command,
	       MF_BYCOMMAND | (enable ? MF_ENABLED : MF_GRAYED) | MF_STRING,
	       dropEnables[i].command, item);
  }
}

void DragPieceBegin(int x, int y, Boolean instantly)
{
      dragInfo.lastpos.x = boardRect.left + x;
      dragInfo.lastpos.y = boardRect.top + y;
      if(instantly) dragInfo.pos = dragInfo.lastpos;
      dragInfo.from.x = fromX;
      dragInfo.from.y = fromY;
      dragInfo.piece = boards[currentMove][fromY][fromX];
      dragInfo.start = dragInfo.from;
      SetCapture(hwndMain);
}

void DragPieceEnd(int x, int y)
{
    ReleaseCapture();
    dragInfo.start.x = dragInfo.start.y = -1;
    dragInfo.from = dragInfo.start;
    dragInfo.pos = dragInfo.lastpos = dragInfo.start;
}

void ChangeDragPiece(ChessSquare piece)
{
    dragInfo.piece = piece;
}

/* Event handler for mouse messages */
VOID
MouseEvent(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  int x, y, menuNr;
  POINT pt;
  static int recursive = 0;
  HMENU hmenu;
  BOOLEAN forceFullRepaint = IsFullRepaintPreferrable(); /* [AS] */

  if (recursive) {
    if (message == WM_MBUTTONUP) {
      /* Hideous kludge to fool TrackPopupMenu into paying attention
	 to the middle button: we simulate pressing the left button too!
	 */
      PostMessage(hwnd, WM_LBUTTONDOWN, wParam, lParam);
      PostMessage(hwnd, WM_LBUTTONUP, wParam, lParam);
    }
    return;
  }
  recursive++;
  
  pt.x = LOWORD(lParam);
  pt.y = HIWORD(lParam);
  x = EventToSquare(pt.x - boardRect.left, BOARD_WIDTH);
  y = EventToSquare(pt.y - boardRect.top, BOARD_HEIGHT);
  if (!flipView && y >= 0) {
    y = BOARD_HEIGHT - 1 - y;
  }
  if (flipView && x >= 0) {
    x = BOARD_WIDTH - 1 - x;
  }

  shiftKey = GetKeyState(VK_SHIFT) < 0; // [HGM] remember last shift status
  controlKey = GetKeyState(VK_CONTROL) < 0; // [HGM] remember last shift status

  switch (message) {
  case WM_LBUTTONDOWN:
      if (PtInRect((LPRECT) &whiteRect, pt)) {
        ClockClick(flipClock); break;
      } else if (PtInRect((LPRECT) &blackRect, pt)) {
	ClockClick(!flipClock); break;
      }
    if(dragging) { // [HGM] lion: don't destroy dragging info if we are already dragging
      dragInfo.start.x = dragInfo.start.y = -1;
      dragInfo.from = dragInfo.start;
    }
    if(fromX == -1 && frozen) { // not sure where this is for
		fromX = fromY = -1; 
      DrawPosition(forceFullRepaint || FALSE, NULL); /* [AS] */
      break;
    }
      LeftClick(Press, pt.x - boardRect.left, pt.y - boardRect.top);
      DrawPosition(TRUE, NULL);
    break;

  case WM_LBUTTONUP:
      LeftClick(Release, pt.x - boardRect.left, pt.y - boardRect.top);
      DrawPosition(TRUE, NULL);
    break;

  case WM_MOUSEMOVE:
    if(SeekGraphClick(Press, pt.x - boardRect.left, pt.y - boardRect.top, 1)) break;
    if(PromoScroll(pt.x - boardRect.left, pt.y - boardRect.top)) break;
    MovePV(pt.x - boardRect.left, pt.y - boardRect.top, boardRect.bottom - boardRect.top);
    if ((appData.animateDragging || appData.highlightDragging)
	&& (wParam & MK_LBUTTON || dragging == 2)
	&& dragInfo.from.x >= 0) 
    {
      BOOL full_repaint = FALSE;

      if (appData.animateDragging) {
	dragInfo.pos = pt;
      }
      if (appData.highlightDragging) {
	HoverEvent(highlightInfo.sq[1].x, highlightInfo.sq[1].y, x, y);
        if( IsDrawArrowEnabled() && (x < 0 || x >= BOARD_WIDTH || y < 0 || y >= BOARD_HEIGHT) ) {
            full_repaint = TRUE;
        }
      }
      
      DrawPosition( full_repaint, NULL);
      
      dragInfo.lastpos = dragInfo.pos;
    }
    break;

  case WM_MOUSEWHEEL: // [DM]
    {  static int lastDir = 0; // [HGM] build in some hysteresis to avoid spurious events
       /* Mouse Wheel is being rolled forward
        * Play moves forward
        */
       if((short)HIWORD(wParam) < 0 && currentMove < forwardMostMove) 
		{ if(lastDir == 1) ForwardEvent(); else lastDir = 1; } // [HGM] suppress first event in direction
       /* Mouse Wheel is being rolled backward
        * Play moves backward
        */
       if((short)HIWORD(wParam) > 0 && currentMove > backwardMostMove) 
		{ if(lastDir == -1) BackwardEvent(); else lastDir = -1; }
    }
    break;

  case WM_MBUTTONUP:
  case WM_RBUTTONUP:
    ReleaseCapture();
    RightClick(Release, pt.x - boardRect.left, pt.y - boardRect.top, &fromX, &fromY);
    break;
 
  case WM_MBUTTONDOWN:
  case WM_RBUTTONDOWN:
    ErrorPopDown();
    ReleaseCapture();
    fromX = fromY = -1;
    dragInfo.pos.x = dragInfo.pos.y = -1;
    dragInfo.start.x = dragInfo.start.y = -1;
    dragInfo.from = dragInfo.start;
    dragInfo.lastpos = dragInfo.pos;
    if (appData.highlightDragging) {
      ClearHighlights();
    }
    if(y == -2) {
      /* [HGM] right mouse button in clock area edit-game mode ups clock */
      if (PtInRect((LPRECT) &whiteRect, pt)) {
          if (GetKeyState(VK_SHIFT) < 0) AdjustClock(flipClock, 1);
      } else if (PtInRect((LPRECT) &blackRect, pt)) {
          if (GetKeyState(VK_SHIFT) < 0) AdjustClock(!flipClock, 1);
      }
      break;
    }
    DrawPosition(TRUE, NULL);

    menuNr = RightClick(Press, pt.x - boardRect.left, pt.y - boardRect.top, &fromX, &fromY);
    switch (menuNr) {
    case 0:
      if (message == WM_MBUTTONDOWN) {
	buttonCount = 3;  /* even if system didn't think so */
	if (wParam & MK_SHIFT) 
	  MenuPopup(hwnd, pt, LoadMenu(hInst, "BlackPieceMenu"), -1);
	else
	  MenuPopup(hwnd, pt, LoadMenu(hInst, "WhitePieceMenu"), -1);
      } else { /* message == WM_RBUTTONDOWN */
	/* Just have one menu, on the right button.  Windows users don't
	   think to try the middle one, and sometimes other software steals
	   it, or it doesn't really exist. */
        if(gameInfo.variant != VariantShogi)
            MenuPopup(hwnd, pt, LoadMenu(hInst, "PieceMenu"), -1);
        else
            MenuPopup(hwnd, pt, LoadMenu(hInst, "ShogiPieceMenu"), -1);
      }
      break;
    case 2:
      SetCapture(hwndMain);
      break;
    case 1:
      hmenu = LoadMenu(hInst, "DropPieceMenu");
      SetupDropMenu(hmenu);
      MenuPopup(hwnd, pt, hmenu, -1);
    default:
      break;
    }
    break;
  }

  recursive--;
}

/* Preprocess messages for buttons in main window */
LRESULT CALLBACK
ButtonProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  int id = GetWindowLongPtr(hwnd, GWLP_ID);
  int i, dir;

  for (i=0; i<N_BUTTONS; i++) {
    if (buttonDesc[i].id == id) break;
  }
  if (i == N_BUTTONS) return 0;
  switch (message) {
  case WM_KEYDOWN:
    switch (wParam) {
    case VK_LEFT:
    case VK_RIGHT:
      dir = (wParam == VK_LEFT) ? -1 : 1;
      SetFocus(buttonDesc[(i + dir + N_BUTTONS) % N_BUTTONS].hwnd);
      return TRUE;
    }
    break;
  case WM_CHAR:
    switch (wParam) {
    case '\r':
      SendMessage(hwndMain, WM_COMMAND, MAKEWPARAM(buttonDesc[i].id, 0), 0);
      return TRUE;
    default:
      if (appData.icsActive && (isalpha((char)wParam) || wParam == '0')) {
	// [HGM] movenum: only letters or leading zero should go to ICS input
        HWND h = GetDlgItem(hwndConsole, OPT_ConsoleInput);
	if (IsIconic(hwndConsole)) ShowWindow(hwndConsole, SW_RESTORE);
	SetFocus(h);
	SendMessage(h, WM_CHAR, wParam, lParam);
	return TRUE;
      } else if (isalpha((char)wParam) || isdigit((char)wParam)){
	TypeInEvent((char)wParam);
      }
      break;
    }
    break;
  }
  return CallWindowProc(buttonDesc[i].wndproc, hwnd, message, wParam, lParam);
}

static int promoStyle;

/* Process messages for Promotion dialog box */
LRESULT CALLBACK
Promotion(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  char promoChar;

  switch (message) {

  case WM_INITDIALOG: /* message: initialize dialog box */
    /* Center the dialog over the application window */
    CenterWindow(hDlg, GetWindow(hDlg, GW_OWNER));
    Translate(hDlg, DLG_PromotionKing);
    ShowWindow(GetDlgItem(hDlg, PB_King), 
      (!appData.testLegality || gameInfo.variant == VariantSuicide ||
       gameInfo.variant == VariantSpartan && !WhiteOnMove(currentMove) ||
       gameInfo.variant == VariantGiveaway || gameInfo.variant == VariantSuper ) ?
	       SW_SHOW : SW_HIDE);
    /* [HGM] Only allow C & A promotions if these pieces are defined */
    ShowWindow(GetDlgItem(hDlg, PB_Archbishop),
       ((PieceToChar(WhiteAngel) >= 'A' && WhiteOnMove(currentMove) &&
         PieceToChar(WhiteAngel) != '~') ||
        (PieceToChar(BlackAngel) >= 'A' && !WhiteOnMove(currentMove) &&
         PieceToChar(BlackAngel) != '~')   ) ?
	       SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hDlg, PB_Chancellor), 
       ((PieceToChar(WhiteMarshall) >= 'A' && WhiteOnMove(currentMove) &&
         PieceToChar(WhiteMarshall) != '~') ||
        (PieceToChar(BlackMarshall) >= 'A' && !WhiteOnMove(currentMove) &&
         PieceToChar(BlackMarshall) != '~')   ) ?
	       SW_SHOW : SW_HIDE);
    /* [HGM] Hide B & R button in Shogi, use Q as promote, N as defer */
    ShowWindow(GetDlgItem(hDlg, PB_Rook),   !promoStyle ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hDlg, PB_Bishop), !promoStyle ? SW_SHOW : SW_HIDE);
    if(promoStyle) {
        SetDlgItemText(hDlg, PB_Queen, "YES");
        SetDlgItemText(hDlg, PB_Knight, "NO");
        SetWindowText(hDlg, "Promote?");
    }
    ShowWindow(GetDlgItem(hDlg, IDC_Centaur), 
       gameInfo.variant == VariantSuper ?
	       SW_SHOW : SW_HIDE);
    return TRUE;

  case WM_COMMAND: /* message: received a command */
    switch (LOWORD(wParam)) {
    case IDCANCEL:
      EndDialog(hDlg, TRUE); /* Exit the dialog */
      ClearHighlights();
      DrawPosition(FALSE, NULL);
      return TRUE;
    case PB_King:
      promoChar = gameInfo.variant == VariantSuper ? PieceToChar(BlackSilver) : PieceToChar(BlackKing);
      break;
    case PB_Queen:
      promoChar = promoStyle ? '+' : ToLower(PieceToChar(WhiteOnMove(currentMove) ? WhiteQueen : BlackQueen));
      break;
    case PB_Rook:
      promoChar = ToLower(PieceToChar(WhiteOnMove(currentMove) ? WhiteRook : BlackRook));
      if(gameInfo.variant == VariantSpartan && !WhiteOnMove(currentMove)) promoChar = PieceToChar(BlackDragon);
      break;
    case PB_Bishop:
      promoChar = ToLower(PieceToChar(WhiteOnMove(currentMove) ? WhiteBishop : BlackBishop));
      if(gameInfo.variant == VariantSpartan && !WhiteOnMove(currentMove)) promoChar = PieceToChar(BlackAlfil);
      break;
    case PB_Chancellor:
      promoChar = ToLower(PieceToChar(WhiteOnMove(currentMove) ? WhiteMarshall : BlackMarshall));
      break;
    case PB_Archbishop:
      promoChar = ToLower(PieceToChar(WhiteOnMove(currentMove) ? WhiteAngel : BlackAngel));
      break;
    case PB_Knight:
      promoChar = gameInfo.variant == VariantShogi ? '=' : promoStyle ? NULLCHAR : 
                  ToLower(PieceToChar(WhiteOnMove(currentMove) ? WhiteKnight : BlackKnight));
      break;
    default:
      return FALSE;
    }
    if(promoChar == '.') return FALSE; // invalid piece chosen 
    EndDialog(hDlg, TRUE); /* Exit the dialog */
    UserMoveEvent(fromX, fromY, toX, toY, promoChar);
    fromX = fromY = -1;
    if (!appData.highlightLastMove) {
      ClearHighlights();
      DrawPosition(FALSE, NULL);
    }
    return TRUE;
  }
  return FALSE;
}

/* Pop up promotion dialog */
VOID
PromotionPopup(HWND hwnd)
{
  FARPROC lpProc;

  lpProc = MakeProcInstance((FARPROC)Promotion, hInst);
  DialogBox(hInst, MAKEINTRESOURCE(DLG_PromotionKing),
    hwnd, (DLGPROC)lpProc);
  FreeProcInstance(lpProc);
}

void
PromotionPopUp(char choice)
{
  promoStyle = (choice == '+');
  DrawPosition(TRUE, NULL);
  PromotionPopup(hwndMain);
}

VOID
LoadGameDialog(HWND hwnd, char* title)
{
  UINT number = 0;
  FILE *f;
  char fileTitle[MSG_SIZ];
  f = OpenFileDialog(hwnd, "rb", "",
 	             appData.oldSaveStyle ? "gam" : "pgn",
		     GAME_FILT,
		     title, &number, fileTitle, NULL);
  if (f != NULL) {
    cmailMsgLoaded = FALSE;
    if (number == 0) {
      int error = GameListBuild(f);
      if (error) {
        DisplayError(_("Cannot build game list"), error);
      } else if (!ListEmpty(&gameList) &&
                 ((ListGame *) gameList.tailPred)->number > 1) {
	GameListPopUp(f, fileTitle);
        return;
      }
      GameListDestroy();
      number = 1;
    }
    LoadGame(f, number, fileTitle, FALSE);
  }
}

int get_term_width()
{
    HDC hdc;
    TEXTMETRIC tm;
    RECT rc;
    HFONT hfont, hold_font;
    LOGFONT lf;
    HWND hText;

    if (hwndConsole)
        hText = GetDlgItem(hwndConsole, OPT_ConsoleText);
    else
        return 79;

    // get the text metrics
    hdc = GetDC(hText);
    lf = font[boardSize][CONSOLE_FONT]->lf;
    if (consoleCF.dwEffects & CFE_BOLD)
        lf.lfWeight = FW_BOLD;
    if (consoleCF.dwEffects & CFE_ITALIC)
        lf.lfItalic = TRUE;
    if (consoleCF.dwEffects & CFE_STRIKEOUT)
        lf.lfStrikeOut = TRUE;
    if (consoleCF.dwEffects & CFE_UNDERLINE)
        lf.lfUnderline = TRUE;
    hfont = CreateFontIndirect(&lf);
    hold_font = SelectObject(hdc, hfont);
    GetTextMetrics(hdc, &tm);
    SelectObject(hdc, hold_font);
    DeleteObject(hfont);
    ReleaseDC(hText, hdc);

    // get the rectangle
    SendMessage(hText, EM_GETRECT, 0, (LPARAM)&rc);

    return (rc.right-rc.left) / tm.tmAveCharWidth;
}

void UpdateICSWidth(HWND hText)
{
    LONG old_width, new_width;

    new_width = get_term_width(hText, FALSE);
    old_width = GetWindowLongPtr(hText, GWLP_USERDATA);
    if (new_width != old_width)
    {
        ics_update_width(new_width);
        SetWindowLongPtr(hText, GWLP_USERDATA, new_width);
    }
}

VOID
ChangedConsoleFont()
{
  CHARFORMAT cfmt;
  CHARRANGE tmpsel, sel;
  MyFont *f = font[boardSize][CONSOLE_FONT];
  HWND hText = GetDlgItem(hwndConsole, OPT_ConsoleText);
  HWND hInput = GetDlgItem(hwndConsole, OPT_ConsoleInput);
  PARAFORMAT paraf;

  cfmt.cbSize = sizeof(CHARFORMAT);
  cfmt.dwMask = CFM_FACE|CFM_SIZE|CFM_CHARSET;
    safeStrCpy(cfmt.szFaceName, font[boardSize][CONSOLE_FONT]->mfp.faceName,
	       sizeof(cfmt.szFaceName)/sizeof(cfmt.szFaceName[0]) );
  /* yHeight is expressed in twips.  A twip is 1/20 of a font's point
   * size.  This was undocumented in the version of MSVC++ that I had
   * when I wrote the code, but is apparently documented now.
   */
  cfmt.yHeight = (int)(f->mfp.pointSize * 20.0 + 0.5);
  cfmt.bCharSet = f->lf.lfCharSet;
  cfmt.bPitchAndFamily = f->lf.lfPitchAndFamily;
  SendMessage(hText, EM_SETCHARFORMAT, SCF_ALL, (LPARAM) &cfmt); 
  SendMessage(hInput, EM_SETCHARFORMAT, SCF_ALL, (LPARAM) &cfmt); 
  /* Why are the following seemingly needed too? */
  SendMessage(hText, EM_SETCHARFORMAT, SCF_DEFAULT, (LPARAM) &cfmt); 
  SendMessage(hInput, EM_SETCHARFORMAT, SCF_DEFAULT, (LPARAM) &cfmt); 
  SendMessage(hText, EM_EXGETSEL, 0, (LPARAM)&sel);
  tmpsel.cpMin = 0;
  tmpsel.cpMax = -1; /*999999?*/
  SendMessage(hText, EM_EXSETSEL, 0, (LPARAM)&tmpsel);
  SendMessage(hText, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM) &cfmt); 
  /* Trying putting this here too.  It still seems to tickle a RichEdit
   *  bug: sometimes RichEdit indents the first line of a paragraph too.
   */
  paraf.cbSize = sizeof(paraf);
  paraf.dwMask = PFM_OFFSET | PFM_STARTINDENT;
  paraf.dxStartIndent = 0;
  paraf.dxOffset = WRAP_INDENT;
  SendMessage(hText, EM_SETPARAFORMAT, 0, (LPARAM) &paraf);
  SendMessage(hText, EM_EXSETSEL, 0, (LPARAM)&sel);
  UpdateICSWidth(hText);
}

/*---------------------------------------------------------------------------*\
 *
 * Window Proc for main window
 *
\*---------------------------------------------------------------------------*/

/* Process messages for main window, etc. */
LRESULT CALLBACK
WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  FARPROC lpProc;
  int wmId;
  char *defName;
  FILE *f;
  UINT number;
  char fileTitle[MSG_SIZ];
  static SnapData sd;
  static int peek=0;

  switch (message) {

  case WM_PAINT: /* message: repaint portion of window */
    PaintProc(hwnd);
    break;

  case WM_ERASEBKGND:
    if (IsIconic(hwnd)) {
      /* Cheat; change the message */
      return (DefWindowProc(hwnd, WM_ICONERASEBKGND, wParam, lParam));
    } else {
      return (DefWindowProc(hwnd, message, wParam, lParam));
    }
    break;

  case WM_LBUTTONDOWN:
  case WM_MBUTTONDOWN:
  case WM_RBUTTONDOWN:
  case WM_LBUTTONUP:
  case WM_MBUTTONUP:
  case WM_RBUTTONUP:
  case WM_MOUSEMOVE:
  case WM_MOUSEWHEEL:
    MouseEvent(hwnd, message, wParam, lParam);
    break;

  case WM_KEYUP:
    if((char)wParam == '\b') {
      ForwardEvent(); peek = 0;
    }

    JAWS_KBUP_NAVIGATION

    break;

  case WM_KEYDOWN:
    if((char)wParam == '\b') {
      if(!peek) BackwardEvent(), peek = 1;
    }

    JAWS_KBDOWN_NAVIGATION

    break;

  case WM_CHAR:
    
    JAWS_ALT_INTERCEPT

    if (appData.icsActive && ((char)wParam == '\r' || (char)wParam > ' ' && !((char)wParam >= '1' && (char)wParam <= '9'))) { 
	// [HGM] movenum: for non-zero digits we always do type-in dialog
	HWND h = GetDlgItem(hwndConsole, OPT_ConsoleInput);
	if (IsIconic(hwndConsole)) ShowWindow(hwndConsole, SW_RESTORE);
	SetFocus(h);
	SendMessage(h, message, wParam, lParam);
    } else if(lParam != KF_REPEAT) {
	if (isalpha((char)wParam) || isdigit((char)wParam)) {
		TypeInEvent((char)wParam);
	} else if((char)wParam == 003) CopyGameToClipboard();
	 else if((char)wParam == 026) PasteGameOrFENFromClipboard();
    }

    break;

  case WM_PALETTECHANGED:
    if (hwnd != (HWND)wParam && !appData.monoMode) {
      int nnew;
      HDC hdc = GetDC(hwndMain);
      SelectPalette(hdc, hPal, TRUE);
      nnew = RealizePalette(hdc);
      if (nnew > 0) {
	paletteChanged = TRUE;

        InvalidateRect(hwnd, &boardRect, FALSE);
      }
      ReleaseDC(hwnd, hdc);
    }
    break;

  case WM_QUERYNEWPALETTE:
    if (!appData.monoMode /*&& paletteChanged*/) {
      int nnew;
      HDC hdc = GetDC(hwndMain);
      paletteChanged = FALSE;
      SelectPalette(hdc, hPal, FALSE);
      nnew = RealizePalette(hdc);
      if (nnew > 0) {
	InvalidateRect(hwnd, &boardRect, FALSE);
      }
      ReleaseDC(hwnd, hdc);
      return TRUE;
    }
    return FALSE;

  case WM_COMMAND: /* message: command from application menu */
    wmId    = LOWORD(wParam);

    switch (wmId) {
    case IDM_NewGame:
      ResetGameEvent();
      SAY("new game enter a move to play against the computer with white");
      break;

    case IDM_NewGameFRC:
      if( NewGameFRC() == 0 ) {
        ResetGameEvent();
      }
      break;

    case IDM_NewVariant:
      NewVariantPopup(hwnd);
      break;

    case IDM_LoadGame:
      LoadGameDialog(hwnd, _("Load Game from File"));
      break;

    case IDM_LoadNextGame:
      ReloadGame(1);
      break;

    case IDM_LoadPrevGame:
      ReloadGame(-1);
      break;

    case IDM_ReloadGame:
      ReloadGame(0);
      break;

    case IDM_LoadPosition:
      if (gameMode == AnalyzeMode || gameMode == AnalyzeFile) {
        Reset(FALSE, TRUE);
      }
      number = 1;
      f = OpenFileDialog(hwnd, "rb", "",
			 appData.oldSaveStyle ? "pos" : "fen",
			 POSITION_FILT,
			 _("Load Position from File"), &number, fileTitle, NULL);
      if (f != NULL) {
	LoadPosition(f, number, fileTitle);
      }
      break;

    case IDM_LoadNextPosition:
      ReloadPosition(1);
      break;

    case IDM_LoadPrevPosition:
      ReloadPosition(-1);
      break;

    case IDM_ReloadPosition:
      ReloadPosition(0);
      break;

    case IDM_SaveGame:
      defName = DefaultFileName(appData.oldSaveStyle ? "gam" : "pgn");
      f = OpenFileDialog(hwnd, "a", defName,
			 appData.oldSaveStyle ? "gam" : "pgn",
			 GAME_FILT,
			 _("Save Game to File"), NULL, fileTitle, NULL);
      if (f != NULL) {
	SaveGame(f, 0, "");
      }
      break;

    case IDM_SavePosition:
      defName = DefaultFileName(appData.oldSaveStyle ? "pos" : "fen");
      f = OpenFileDialog(hwnd, "a", defName,
			 appData.oldSaveStyle ? "pos" : "fen",
			 POSITION_FILT,
			 _("Save Position to File"), NULL, fileTitle, NULL);
      if (f != NULL) {
	SavePosition(f, 0, "");
      }
      break;

    case IDM_SaveDiagram:
      defName = "diagram";
      f = OpenFileDialog(hwnd, "wb", defName,
			 "bmp",
			 DIAGRAM_FILT,
			 _("Save Diagram to File"), NULL, fileTitle, NULL);
      if (f != NULL) {
	SaveDiagram(f);
      }
      break;

    case IDM_SaveSelected:
      f = OpenFileDialog(hwnd, "a", "",
			 "pgn",
			 GAME_FILT,
			 _("Save Game to File"), NULL, fileTitle, NULL);
      if (f != NULL) {
	SaveSelected(f, 0, "");
      }
      break;

    case IDM_CreateBook:
      CreateBookEvent();
      break;

    case IDM_CopyGame:
      CopyGameToClipboard();
      break;

    case IDM_PasteGame:
      PasteGameFromClipboard();
      break;

    case IDM_CopyGameListToClipboard:
      CopyGameListToClipboard();
      break;

    /* [AS] Autodetect FEN or PGN data */
    case IDM_PasteAny:
      PasteGameOrFENFromClipboard();
      break;

    /* [AS] Move history */
    case IDM_ShowMoveHistory:
        if( MoveHistoryIsUp() ) {
            MoveHistoryPopDown();
        }
        else {
            MoveHistoryPopUp();
        }
        break;

    /* [AS] Eval graph */
    case IDM_ShowEvalGraph:
        if( EvalGraphIsUp() ) {
            EvalGraphPopDown();
        }
        else {
            EvalGraphPopUp();
	    SetFocus(hwndMain);
        }
        break;

    /* [AS] Engine output */
    case IDM_ShowEngineOutput:
        if( EngineOutputIsUp() ) {
            EngineOutputPopDown();
        }
        else {
            EngineOutputPopUp();
        }
        break;

    /* [AS] User adjudication */
    case IDM_UserAdjudication_White:
        UserAdjudicationEvent( +1 );
        break;

    case IDM_UserAdjudication_Black:
        UserAdjudicationEvent( -1 );
        break;

    case IDM_UserAdjudication_Draw:
        UserAdjudicationEvent( 0 );
        break;

    /* [AS] Game list options dialog */
    case IDM_GameListOptions:
      GameListOptions();
      break;

    case IDM_NewChat:
      ChatPopUp(NULL);
      break;

    case IDM_CopyPosition:
      CopyFENToClipboard();
      break;

    case IDM_PastePosition:
      PasteFENFromClipboard();
      break;

    case IDM_MailMove:
      MailMoveEvent();
      break;

    case IDM_ReloadCMailMsg:
      Reset(TRUE, TRUE);
      ReloadCmailMsgEvent(FALSE);
      break;

    case IDM_Minimize:
      ShowWindow(hwnd, SW_MINIMIZE);
      break;

    case IDM_Exit:
      ExitEvent(0);
      break;

    case IDM_MachineWhite:
      MachineWhiteEvent();
      /*
       * refresh the tags dialog only if it's visible
       */
      if (gameMode == MachinePlaysWhite && IsWindowVisible(editTagsDialog)) {
	  char *tags;
	  tags = PGNTags(&gameInfo);
	  TagsPopUp(tags, CmailMsg());
	  free(tags);
      }
      SAY("computer starts playing white");
      break;

    case IDM_MachineBlack:
      MachineBlackEvent();
      /*
       * refresh the tags dialog only if it's visible
       */
      if (gameMode == MachinePlaysBlack && IsWindowVisible(editTagsDialog)) {
	  char *tags;
	  tags = PGNTags(&gameInfo);
	  TagsPopUp(tags, CmailMsg());
	  free(tags);
      }
      SAY("computer starts playing black");
      break;

    case IDM_Match: // [HGM] match: flows into next case, after setting Match Mode and nr of Games
      MatchEvent(2); // distinguish from command-line-triggered case (matchMode=1)
      break;

    case IDM_TwoMachines:
      TwoMachinesEvent();
      /*
       * refresh the tags dialog only if it's visible
       */
      if (gameMode == TwoMachinesPlay && IsWindowVisible(editTagsDialog)) {
	  char *tags;
	  tags = PGNTags(&gameInfo);
	  TagsPopUp(tags, CmailMsg());
	  free(tags);
      }
      SAY("computer starts playing both sides");
      break;

    case IDM_AnalysisMode:
      if(AnalyzeModeEvent()) {
	SAY("analyzing current position");
      }
      break;

    case IDM_AnalyzeFile:
      AnalyzeFileEvent();
      break;

    case IDM_IcsClient:
      IcsClientEvent();
      break;

    case IDM_EditGame:
    case IDM_EditGame2:
      EditGameEvent();
      SAY("edit game");
      break;

    case IDM_EditPosition:
    case IDM_EditPosition2:
      EditPositionEvent();
      SAY("enter a FEN string or setup a position on the board using the control R pop up menu");
      break;

    case IDM_Training:
      TrainingEvent();
      break;

    case IDM_ShowGameList:
      ShowGameListProc();
      break;

    case IDM_EditProgs1:
      EditTagsPopUp(firstChessProgramNames, &firstChessProgramNames);
      break;

    case IDM_LoadProg1:
     LoadEnginePopUp(hwndMain, 0);
      break;

    case IDM_LoadProg2:
     LoadEnginePopUp(hwndMain, 1);
      break;

    case IDM_EditServers:
      EditTagsPopUp(icsNames, &icsNames);
      break;

    case IDM_EditTags:
    case IDM_Tags:
      EditTagsProc();
      break;

    case IDM_EditBook:
      EditBookEvent();
      break;

    case IDM_EditComment:
    case IDM_Comment:
      if (commentUp && editComment) {
	CommentPopDown();
      } else {
	EditCommentEvent();
      }
      break;

    case IDM_Pause:
      PauseEvent();
      break;

    case IDM_Accept:
      AcceptEvent();
      break;

    case IDM_Decline:
      DeclineEvent();
      break;

    case IDM_Rematch:

      RematchEvent();
      break;

    case IDM_CallFlag:
      CallFlagEvent();
      break;

    case IDM_Draw:
      DrawEvent();
      break;

    case IDM_Adjourn:
      AdjournEvent();
      break;

    case IDM_Abort:
      AbortEvent();
      break;

    case IDM_Resign:
      ResignEvent();
      break;

    case IDM_StopObserving:
      StopObservingEvent();
      break;

    case IDM_StopExamining:
      StopExaminingEvent();
      break;

    case IDM_Upload:
      UploadGameEvent();
      break;

    case IDM_TypeInMove:
      TypeInEvent('\000');
      break;

    case IDM_TypeInName:
      PopUpNameDialog('\000');
      break;

    case IDM_Backward:
      BackwardEvent();
      SetFocus(hwndMain);
      break;

    JAWS_MENU_ITEMS

    case IDM_Forward:
      ForwardEvent();
      SetFocus(hwndMain);
      break;

    case IDM_ToStart:
      ToStartEvent();
      SetFocus(hwndMain);
      break;

    case IDM_ToEnd:
      ToEndEvent();
      SetFocus(hwndMain);
      break;

    case OPT_GameListNext: // [HGM] forward these two accelerators to Game List
    case OPT_GameListPrev:
      if(gameListDialog) SendMessage(gameListDialog, WM_COMMAND, wmId, 0);
      break;

    case IDM_Revert:
      RevertEvent(FALSE);
      break;

    case IDM_Annotate: // [HGM] vari: revert with annotation
      RevertEvent(TRUE);
      break;

    case IDM_TruncateGame:
      TruncateGameEvent();
      break;

    case IDM_MoveNow:
      MoveNowEvent();
      break;

    case IDM_RetractMove:
      RetractMoveEvent();
      break;

    case IDM_FlipView:
      flipView = !flipView;
      DrawPosition(FALSE, NULL);
      break;

    case IDM_FlipClock:
      flipClock = !flipClock;
      DisplayBothClocks();
      DisplayLogos();
      break;

    case IDM_MuteSounds:
      mute = !mute; // [HGM] mute: keep track of global muting variable
      CheckMenuItem(GetMenu(hwndMain),IDM_MuteSounds, 
				MF_BYCOMMAND|(mute?MF_CHECKED:MF_UNCHECKED));
      break;

    case IDM_GeneralOptions:
      GeneralOptionsPopup(hwnd);
      DrawPosition(TRUE, NULL);
      break;

    case IDM_BoardOptions:
      BoardOptionsPopup(hwnd);
      break;

    case IDM_ThemeOptions:
      ThemeOptionsPopup(hwnd);
      break;

    case IDM_EnginePlayOptions:
      EnginePlayOptionsPopup(hwnd);
      break;

    case IDM_Engine1Options:
      EngineOptionsPopup(hwnd, &first);
      break;

    case IDM_Engine2Options:
      savedHwnd = hwnd;
      if(WaitForEngine(&second, SettingsMenuIfReady)) break;
      EngineOptionsPopup(hwnd, &second);
      break;

    case IDM_OptionsUCI:
      UciOptionsPopup(hwnd);
      break;

    case IDM_Tourney:
      TourneyPopup(hwnd);
      break;

    case IDM_IcsOptions:
      IcsOptionsPopup(hwnd);
      break;

    case IDM_Fonts:
      FontsOptionsPopup(hwnd);
      break;

    case IDM_Sounds:
      SoundOptionsPopup(hwnd);
      break;

    case IDM_CommPort:
      CommPortOptionsPopup(hwnd);
      break;

    case IDM_LoadOptions:
      LoadOptionsPopup(hwnd);
      break;

    case IDM_SaveOptions:
      SaveOptionsPopup(hwnd);
      break;

    case IDM_TimeControl:
      TimeControlOptionsPopup(hwnd);
      break;

    case IDM_SaveSettings:
      SaveSettings(settingsFileName);
      break;

    case IDM_SaveSettingsOnExit:
      saveSettingsOnExit = !saveSettingsOnExit;
      (void) CheckMenuItem(GetMenu(hwndMain), IDM_SaveSettingsOnExit,
			   MF_BYCOMMAND|(saveSettingsOnExit ?
					 MF_CHECKED : MF_UNCHECKED));
      break;

    case IDM_Hint:
      HintEvent();
      break;

    case IDM_Book:
      BookEvent();
      break;

    case IDM_AboutGame:
      AboutGameEvent();
      break;

    case IDM_Debug:
      appData.debugMode = !appData.debugMode;
      if (appData.debugMode) {
	char dir[MSG_SIZ];
	GetCurrentDirectory(MSG_SIZ, dir);
	SetCurrentDirectory(installDir);
	debugFP = fopen(appData.nameOfDebugFile, "w");
        SetCurrentDirectory(dir);
        setbuf(debugFP, NULL);
      } else {
	fclose(debugFP);
        debugFP = NULL;
      }
      break;

    case IDM_HELPCONTENTS:
      if (!MyHelp (hwnd, "winboard.hlp", HELP_KEY,(DWORD)(LPSTR)"CONTENTS") &&
	  !HtmlHelp(hwnd, "winboard.chm", 0, 0)	) {
	  MessageBox (GetFocus(),
		    _("Unable to activate help"),
		    szAppName, MB_SYSTEMMODAL|MB_OK|MB_ICONHAND);
      }
      break;

    case IDM_HELPSEARCH:
        if (!MyHelp (hwnd, "winboard.hlp", HELP_PARTIALKEY, (DWORD)(LPSTR)"") &&
	    !HtmlHelp(hwnd, "winboard.chm", 0, 0)	) {
	MessageBox (GetFocus(),
		    _("Unable to activate help"),
		    szAppName, MB_SYSTEMMODAL|MB_OK|MB_ICONHAND);
      }
      break;

    case IDM_HELPHELP:
      if(!WinHelp(hwnd, (LPSTR)NULL, HELP_HELPONHELP, 0)) {
	MessageBox (GetFocus(),
		    _("Unable to activate help"),
		    szAppName, MB_SYSTEMMODAL|MB_OK|MB_ICONHAND);
      }
      break;

    case IDM_ABOUT:
      lpProc = MakeProcInstance((FARPROC)About, hInst);
      DialogBox(hInst, 
	(gameInfo.event && strcmp(gameInfo.event, "Easter Egg Hunt") == 0) ?
	"AboutBox2" : "AboutBox", hwnd, (DLGPROC)lpProc);
      FreeProcInstance(lpProc);
      break;

    case IDM_DirectCommand1:
      AskQuestionEvent(_("Direct Command"),
		       _("Send to chess program:"), "", "1");
      break;
    case IDM_DirectCommand2:
      AskQuestionEvent(_("Direct Command"),
		       _("Send to second chess program:"), "", "2");
      break;

    case EP_WhitePawn:
      EditPositionMenuEvent(WhitePawn, fromX, fromY);
      fromX = fromY = -1;
      break;

    case EP_WhiteKnight:
      EditPositionMenuEvent(WhiteKnight, fromX, fromY);
      fromX = fromY = -1;
      break;

    case EP_WhiteBishop:
      EditPositionMenuEvent(WhiteBishop, fromX, fromY);
      fromX = fromY = -1;
      break;

    case EP_WhiteRook:
      EditPositionMenuEvent(WhiteRook, fromX, fromY);
      fromX = fromY = -1;
      break;

    case EP_WhiteQueen:
      EditPositionMenuEvent(WhiteQueen, fromX, fromY);
      fromX = fromY = -1;
      break;

    case EP_WhiteFerz:
      EditPositionMenuEvent(WhiteFerz, fromX, fromY);
      fromX = fromY = -1;
      break;

    case EP_WhiteWazir:
      EditPositionMenuEvent(WhiteWazir, fromX, fromY);
      fromX = fromY = -1;
      break;

    case EP_WhiteAlfil:
      EditPositionMenuEvent(WhiteAlfil, fromX, fromY);
      fromX = fromY = -1;
      break;

    case EP_WhiteCannon:
      EditPositionMenuEvent(WhiteCannon, fromX, fromY);
      fromX = fromY = -1;
      break;

    case EP_WhiteCardinal:
      EditPositionMenuEvent(WhiteAngel, fromX, fromY);
      fromX = fromY = -1;
      break;

    case EP_WhiteMarshall:
      EditPositionMenuEvent(WhiteMarshall, fromX, fromY);
      fromX = fromY = -1;
      break;

    case EP_WhiteKing:
      EditPositionMenuEvent(WhiteKing, fromX, fromY);
      fromX = fromY = -1;
      break;

    case EP_BlackPawn:
      EditPositionMenuEvent(BlackPawn, fromX, fromY);
      fromX = fromY = -1;
      break;

    case EP_BlackKnight:
      EditPositionMenuEvent(BlackKnight, fromX, fromY);
      fromX = fromY = -1;
      break;

    case EP_BlackBishop:
      EditPositionMenuEvent(BlackBishop, fromX, fromY);
      fromX = fromY = -1;
      break;

    case EP_BlackRook:
      EditPositionMenuEvent(BlackRook, fromX, fromY);
      fromX = fromY = -1;
      break;

    case EP_BlackQueen:
      EditPositionMenuEvent(BlackQueen, fromX, fromY);
      fromX = fromY = -1;
      break;

    case EP_BlackFerz:
      EditPositionMenuEvent(BlackFerz, fromX, fromY);
      fromX = fromY = -1;
      break;

    case EP_BlackWazir:
      EditPositionMenuEvent(BlackWazir, fromX, fromY);
      fromX = fromY = -1;
      break;

    case EP_BlackAlfil:
      EditPositionMenuEvent(BlackAlfil, fromX, fromY);
      fromX = fromY = -1;
      break;

    case EP_BlackCannon:
      EditPositionMenuEvent(BlackCannon, fromX, fromY);
      fromX = fromY = -1;
      break;

    case EP_BlackCardinal:
      EditPositionMenuEvent(BlackAngel, fromX, fromY);
      fromX = fromY = -1;
      break;

    case EP_BlackMarshall:
      EditPositionMenuEvent(BlackMarshall, fromX, fromY);
      fromX = fromY = -1;
      break;

    case EP_BlackKing:
      EditPositionMenuEvent(BlackKing, fromX, fromY);
      fromX = fromY = -1;
      break;

    case EP_EmptySquare:
      EditPositionMenuEvent(EmptySquare, fromX, fromY);
      fromX = fromY = -1;
      break;

    case EP_ClearBoard:
      EditPositionMenuEvent(ClearBoard, fromX, fromY);
      fromX = fromY = -1;
      break;

    case EP_White:
      EditPositionMenuEvent(WhitePlay, fromX, fromY);
      fromX = fromY = -1;
      break;

    case EP_Black:
      EditPositionMenuEvent(BlackPlay, fromX, fromY);
      fromX = fromY = -1;
      break;

    case EP_Promote:
      EditPositionMenuEvent(PromotePiece, fromX, fromY);
      fromX = fromY = -1;
      break;

    case EP_Demote:
      EditPositionMenuEvent(DemotePiece, fromX, fromY);
      fromX = fromY = -1;
      break;

    case DP_Pawn:
      DropMenuEvent(WhitePawn, fromX, fromY);
      fromX = fromY = -1;
      break;

    case DP_Knight:
      DropMenuEvent(WhiteKnight, fromX, fromY);
      fromX = fromY = -1;
      break;

    case DP_Bishop:
      DropMenuEvent(WhiteBishop, fromX, fromY);
      fromX = fromY = -1;
      break;

    case DP_Rook:
      DropMenuEvent(WhiteRook, fromX, fromY);
      fromX = fromY = -1;
      break;

    case DP_Queen:
      DropMenuEvent(WhiteQueen, fromX, fromY);
      fromX = fromY = -1;
      break;

    case IDM_English:
      barbaric = 0; appData.language = "";
      TranslateMenus(0);
      CheckMenuItem(GetMenu(hwndMain), lastChecked, MF_BYCOMMAND|MF_UNCHECKED);
      CheckMenuItem(GetMenu(hwndMain), IDM_English, MF_BYCOMMAND|MF_CHECKED);
      lastChecked = wmId;
      break;

    default:
      if(wmId >= IDM_RecentEngines && wmId < IDM_RecentEngines + appData.recentEngines)
          RecentEngineEvent(wmId - IDM_RecentEngines);
      else
      if(wmId > IDM_English && wmId < IDM_English+20) {
          LoadLanguageFile(languageFile[wmId - IDM_English - 1]);
          TranslateMenus(0);
          CheckMenuItem(GetMenu(hwndMain), lastChecked, MF_BYCOMMAND|MF_UNCHECKED);
          CheckMenuItem(GetMenu(hwndMain), wmId, MF_BYCOMMAND|MF_CHECKED);
          lastChecked = wmId;
          break;
      }
      return (DefWindowProc(hwnd, message, wParam, lParam));
    }
    break;

  case WM_TIMER:
    switch (wParam) {
    case CLOCK_TIMER_ID:
      KillTimer(hwnd, clockTimerEvent);  /* Simulate one-shot timer as in X */
      clockTimerEvent = 0;
      DecrementClocks(); /* call into back end */
      break;
    case LOAD_GAME_TIMER_ID:
      KillTimer(hwnd, loadGameTimerEvent); /* Simulate one-shot timer as in X*/
      loadGameTimerEvent = 0;
      AutoPlayGameLoop(); /* call into back end */
      break;
    case ANALYSIS_TIMER_ID:
      if ((gameMode == AnalyzeMode || gameMode == AnalyzeFile
                 || appData.icsEngineAnalyze) && appData.periodicUpdates) {
	AnalysisPeriodicEvent(0);
      } else {
	KillTimer(hwnd, analysisTimerEvent);
	analysisTimerEvent = 0;
      }
      break;
    case DELAYED_TIMER_ID:
      KillTimer(hwnd, delayedTimerEvent);
      delayedTimerEvent = 0;
      delayedTimerCallback();
      break;
    }
    break;

  case WM_USER_Input:
    InputEvent(hwnd, message, wParam, lParam);
    break;

  /* [AS] Also move "attached" child windows */
  case WM_WINDOWPOSCHANGING:

    if( hwnd == hwndMain && appData.useStickyWindows ) {
        LPWINDOWPOS lpwp = (LPWINDOWPOS) lParam;

        if( ((lpwp->flags & SWP_NOMOVE) == 0) /*&& ((lpwp->flags & SWP_NOSIZE) != 0)*/ ) { // [HGM] in Win8 size always accompanies move?
            /* Window is moving */
            RECT rcMain;

//            GetWindowRect( hwnd, &rcMain ); //[HGM] sticky: in XP this returned new position, not old
	    rcMain.left   = wpMain.x;           //              replace by these 4 lines to reconstruct old rect
	    rcMain.right  = wpMain.x + wpMain.width;
	    rcMain.top    = wpMain.y;
	    rcMain.bottom = wpMain.y + wpMain.height;
            
            ReattachAfterMove( &rcMain, lpwp->x, lpwp->y, moveHistoryDialog, &wpMoveHistory );
            ReattachAfterMove( &rcMain, lpwp->x, lpwp->y, evalGraphDialog, &wpEvalGraph );
            ReattachAfterMove( &rcMain, lpwp->x, lpwp->y, engineOutputDialog, &wpEngineOutput );
            ReattachAfterMove( &rcMain, lpwp->x, lpwp->y, gameListDialog, &wpGameList );
            ReattachAfterMove( &rcMain, lpwp->x, lpwp->y, hwndConsole, &wpConsole );
	    wpMain.x = lpwp->x;
            wpMain.y = lpwp->y;
        }
    }
    break;

  /* [AS] Snapping */
  case WM_ENTERSIZEMOVE:
    if(appData.debugMode) { fprintf(debugFP, "size-move\n"); }
    if (hwnd == hwndMain) {
      doingSizing = TRUE;
      lastSizing = 0;
    }
    return OnEnterSizeMove( &sd, hwnd, wParam, lParam );
    break;

  case WM_SIZING:
    if(appData.debugMode) { fprintf(debugFP, "sizing\n"); }
    if (hwnd == hwndMain) {
      lastSizing = wParam;
    }
    break;

  case WM_MOVING:
    if(appData.debugMode) { fprintf(debugFP, "moving\n"); }
      return OnMoving( &sd, hwnd, wParam, lParam );

  case WM_EXITSIZEMOVE:
    if(appData.debugMode) { fprintf(debugFP, "exit size-move, size = %d\n", squareSize); }
    if (hwnd == hwndMain) {
      RECT client;
      doingSizing = FALSE;
      InvalidateRect(hwnd, &boardRect, FALSE);
      GetClientRect(hwnd, &client);
      ResizeBoard(client.right, client.bottom, lastSizing);
      lastSizing = 0;
      if(appData.debugMode) { fprintf(debugFP, "square size = %d\n", squareSize); }
    }
    return OnExitSizeMove( &sd, hwnd, wParam, lParam );
    break;

  case WM_DESTROY: /* message: window being destroyed */
    PostQuitMessage(0);
    break;

  case WM_CLOSE:
    if (hwnd == hwndMain) {
      ExitEvent(0);
    }
    break;

  default:	/* Passes it on if unprocessed */
    return (DefWindowProc(hwnd, message, wParam, lParam));
  }


  return 0;
}

/*---------------------------------------------------------------------------*\
 *
 * Misc utility routines
 *
\*---------------------------------------------------------------------------*/

/*
 * Decent random number generator, at least not as bad as Windows
 * standard rand, which returns a value in the range 0 to 0x7fff.
 */
unsigned int randstate;

int
myrandom(void)
{
  randstate = randstate * 1664525 + 1013904223;
  return (int) randstate & 0x7fffffff;
}

void
mysrandom(unsigned int seed)
{
  randstate = seed;
}


/* 
 * returns TRUE if user selects a different color, FALSE otherwise 
 */

BOOL
ChangeColor(HWND hwnd, COLORREF *which)
{
  static BOOL firstTime = TRUE;
  static DWORD customColors[16];
  CHOOSECOLOR cc;
  COLORREF newcolor;
  int i;
  ColorClass ccl;

  if (firstTime) {
    /* Make initial colors in use available as custom colors */
    /* Should we put the compiled-in defaults here instead? */
    i = 0;
    customColors[i++] = lightSquareColor & 0xffffff;
    customColors[i++] = darkSquareColor & 0xffffff;
    customColors[i++] = whitePieceColor & 0xffffff;
    customColors[i++] = blackPieceColor & 0xffffff;
    customColors[i++] = highlightSquareColor & 0xffffff;
    customColors[i++] = premoveHighlightColor & 0xffffff;

    for (ccl = (ColorClass) 0; ccl < NColorClasses && i < 16; ccl++) {
      customColors[i++] = textAttribs[ccl].color;
    }
    while (i < 16) customColors[i++] = RGB(255, 255, 255);
    firstTime = FALSE;
  }

  cc.lStructSize = sizeof(cc);
  cc.hwndOwner = hwnd;
  cc.hInstance = NULL;
  cc.rgbResult = (DWORD) (*which & 0xffffff);
  cc.lpCustColors = (LPDWORD) customColors;
  cc.Flags = CC_RGBINIT|CC_FULLOPEN;

  if (!ChooseColor(&cc)) return FALSE;

  newcolor = (COLORREF) (0x2000000 | cc.rgbResult);
  if (newcolor == *which) return FALSE;
  *which = newcolor;
  return TRUE;

  /*
  InitDrawingColors();
  InvalidateRect(hwnd, &boardRect, FALSE);
  */
}

BOOLEAN
MyLoadSound(MySound *ms)
{
  BOOL ok = FALSE;
  struct stat st;
  FILE *f;

  if (ms->data && ms->flag) free(ms->data);
  ms->data = NULL;

  switch (ms->name[0]) {
  case NULLCHAR:
    /* Silence */
    ok = TRUE;
    break;
  case '$':
    /* System sound from Control Panel.  Don't preload here. */
    ok = TRUE;
    break;
  case '!':
    if (ms->name[1] == NULLCHAR) {
      /* "!" alone = silence */
      ok = TRUE;
    } else {
      /* Builtin wave resource.  Error if not found. */
      HANDLE h = FindResource(hInst, ms->name + 1, "WAVE");
      if (h == NULL) break;
      ms->data = (void *)LoadResource(hInst, h);
      ms->flag = 0; // not maloced, so cannot be freed!
      if (h == NULL) break;
      ok = TRUE;
    }
    break;
  default:
    /* .wav file.  Error if not found. */
    f = fopen(ms->name, "rb");
    if (f == NULL) break;
    if (fstat(fileno(f), &st) < 0) break;
    ms->data = malloc(st.st_size);
    ms->flag = 1;
    if (fread(ms->data, st.st_size, 1, f) < 1) break;
    fclose(f);
    ok = TRUE;
    break;
  }
  if (!ok) {
    char buf[MSG_SIZ];
      snprintf(buf, MSG_SIZ, _("Error loading sound %s"), ms->name);
    DisplayError(buf, GetLastError());
  }
  return ok;
}

BOOLEAN
MyPlaySound(MySound *ms)
{
  BOOLEAN ok = FALSE;

  if(mute) return TRUE; // [HGM] mute: suppress all sound play when muted
  switch (ms->name[0]) {
  case NULLCHAR:
	if(appData.debugMode) fprintf(debugFP, "silence\n");
    /* Silence */
    ok = TRUE;
    break;
  case '$':
    /* System sound from Control Panel (deprecated feature).
       "$" alone or an unset sound name gets default beep (still in use). */
    if (ms->name[1]) {
      ok = PlaySound(ms->name + 1, NULL, SND_ALIAS|SND_ASYNC);
    }
    if (!ok) ok = MessageBeep(MB_OK);
    break; 
  case '!':
    /* Builtin wave resource, or "!" alone for silence */
    if (ms->name[1]) {
      if (ms->data == NULL) return FALSE;
      ok = PlaySound(ms->data, NULL, SND_MEMORY|SND_ASYNC);
    } else {
      ok = TRUE;
    }
    break;
  default:
    /* .wav file.  Error if not found. */
    if (ms->data == NULL) return FALSE;
    ok = PlaySound(ms->data, NULL, SND_MEMORY|SND_ASYNC);
    break;
  }
  /* Don't print an error: this can happen innocently if the sound driver
     is busy; for instance, if another instance of WinBoard is playing
     a sound at about the same time. */
  return ok;
}


LRESULT CALLBACK
OldOpenFileHook(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  BOOL ok;
  OPENFILENAME *ofn;
  static UINT *number; /* gross that this is static */

  switch (message) {
  case WM_INITDIALOG: /* message: initialize dialog box */
    /* Center the dialog over the application window */
    ofn = (OPENFILENAME *) lParam;
    if (ofn->Flags & OFN_ENABLETEMPLATE) {
      number = (UINT *) ofn->lCustData;
      SendMessage(GetDlgItem(hDlg, edt2), WM_SETTEXT, 0, (LPARAM) "");
    } else {
      number = NULL;
    }
    CenterWindow (hDlg, GetWindow (hDlg, GW_OWNER));
    Translate(hDlg, 1536);
    return FALSE;  /* Allow for further processing */

  case WM_COMMAND:
    if ((LOWORD(wParam) == IDOK) && (number != NULL)) {
      *number = GetDlgItemInt(hDlg, OPT_IndexNumberOld, &ok, FALSE);
    }
    return FALSE;  /* Allow for further processing */
  }
  return FALSE;
}

UINT APIENTRY
OpenFileHook(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
  static UINT *number;
  OPENFILENAME *ofname;
  OFNOTIFY *ofnot;
  switch (uiMsg) {
  case WM_INITDIALOG:
    Translate(hdlg, DLG_IndexNumber);
    ofname = (OPENFILENAME *)lParam;
    number = (UINT *)(ofname->lCustData);
    break;
  case WM_NOTIFY:
    ofnot = (OFNOTIFY *)lParam;
    if (ofnot->hdr.code == CDN_FILEOK) {
      *number = GetDlgItemInt(hdlg, OPT_IndexNumber, NULL, FALSE);
    }
    break;
  }
  return 0;
}


FILE *
OpenFileDialog(HWND hwnd, char *write, char *defName, char *defExt, // [HGM] diag: type of 'write' now string
	       char *nameFilt, char *dlgTitle, UINT *number,
	       char fileTitle[MSG_SIZ], char fileName[MSG_SIZ])
{
  OPENFILENAME openFileName;
  char buf1[MSG_SIZ];
  FILE *f;

  if (fileName == NULL) fileName = buf1;
  if (defName == NULL) {
    safeStrCpy(fileName, "*.", 3 );
    strcat(fileName, defExt);
  } else {
    safeStrCpy(fileName, defName, MSG_SIZ );
  }
    if (fileTitle) safeStrCpy(fileTitle, "", MSG_SIZ );
  if (number) *number = 0;

  openFileName.lStructSize       = sizeof(OPENFILENAME);
  openFileName.hwndOwner         = hwnd;
  openFileName.hInstance         = (HANDLE) hInst;
  openFileName.lpstrFilter       = nameFilt;
  openFileName.lpstrCustomFilter = (LPSTR) NULL;
  openFileName.nMaxCustFilter    = 0L;
  openFileName.nFilterIndex      = 1L;
  openFileName.lpstrFile         = fileName;
  openFileName.nMaxFile          = MSG_SIZ;
  openFileName.lpstrFileTitle    = fileTitle;
  openFileName.nMaxFileTitle     = fileTitle ? MSG_SIZ : 0;
  openFileName.lpstrInitialDir   = NULL;
  openFileName.lpstrTitle        = dlgTitle;
  openFileName.Flags = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY 
    | (write[0] != 'r' ? 0 : OFN_FILEMUSTEXIST) 
    | (number ? OFN_ENABLETEMPLATE | OFN_ENABLEHOOK: 0)
    | (oldDialog ? 0 : OFN_EXPLORER);
  openFileName.nFileOffset       = 0;
  openFileName.nFileExtension    = 0;
  openFileName.lpstrDefExt       = defExt;
  openFileName.lCustData         = (LONG) number;
  openFileName.lpfnHook          = oldDialog ?
    (LPOFNHOOKPROC) OldOpenFileHook : (LPOFNHOOKPROC) OpenFileHook;
  openFileName.lpTemplateName    = (LPSTR)(oldDialog ? 1536 : DLG_IndexNumber);

  if (write[0] != 'r' ? GetSaveFileName(&openFileName) : 
                        GetOpenFileName(&openFileName)) {
    /* open the file */
    f = fopen(openFileName.lpstrFile, write);
    if (f == NULL) {
      MessageBox(hwnd, _("File open failed"), NULL,
		 MB_OK|MB_ICONEXCLAMATION);
      return NULL;
    }
  } else {
    int err = CommDlgExtendedError();
    if (err != 0) DisplayError(_("Internal error in file dialog box"), err);
    return FALSE;
  }
  return f;
}



VOID APIENTRY
MenuPopup(HWND hwnd, POINT pt, HMENU hmenu, UINT def)
{
  HMENU hmenuTrackPopup;	/* floating pop-up menu  */

  /*
   * Get the first pop-up menu in the menu template. This is the
   * menu that TrackPopupMenu displays.
   */
  hmenuTrackPopup = GetSubMenu(hmenu, 0);
  TranslateOneMenu(10, hmenuTrackPopup);

  SetMenuDefaultItem(hmenuTrackPopup, def, FALSE);

  /*
   * TrackPopup uses screen coordinates, so convert the
   * coordinates of the mouse click to screen coordinates.
   */
  ClientToScreen(hwnd, (LPPOINT) &pt);

  /* Draw and track the floating pop-up menu. */
  TrackPopupMenu(hmenuTrackPopup, TPM_CENTERALIGN | TPM_RIGHTBUTTON,
		 pt.x, pt.y, 0, hwnd, NULL);

  /* Destroy the menu.*/
  DestroyMenu(hmenu);
}
   
typedef struct {
  HWND hDlg, hText;
  int sizeX, sizeY, newSizeX, newSizeY;
  HDWP hdwp;
} ResizeEditPlusButtonsClosure;

BOOL CALLBACK
ResizeEditPlusButtonsCallback(HWND hChild, LPARAM lparam)
{
  ResizeEditPlusButtonsClosure *cl = (ResizeEditPlusButtonsClosure *)lparam;
  RECT rect;
  POINT pt;

  if (hChild == cl->hText) return TRUE;
  GetWindowRect(hChild, &rect); /* gives screen coords */
  pt.x = rect.left + (cl->newSizeX - cl->sizeX)/2;
  pt.y = rect.top + cl->newSizeY - cl->sizeY;
  ScreenToClient(cl->hDlg, &pt);
  cl->hdwp = DeferWindowPos(cl->hdwp, hChild, NULL, 
    pt.x, pt.y, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER);
  return TRUE;
}

/* Resize a dialog that has a (rich) edit field filling most of
   the top, with a row of buttons below */
VOID
ResizeEditPlusButtons(HWND hDlg, HWND hText, int sizeX, int sizeY, int newSizeX, int newSizeY)
{
  RECT rectText;
  int newTextHeight, newTextWidth;
  ResizeEditPlusButtonsClosure cl;
  
  /*if (IsIconic(hDlg)) return;*/
  if (newSizeX == sizeX && newSizeY == sizeY) return;
  
  cl.hdwp = BeginDeferWindowPos(8);

  GetWindowRect(hText, &rectText); /* gives screen coords */
  newTextWidth = rectText.right - rectText.left + newSizeX - sizeX;
  newTextHeight = rectText.bottom - rectText.top + newSizeY - sizeY;
  if (newTextHeight < 0) {
    newSizeY += -newTextHeight;
    newTextHeight = 0;
  }
  cl.hdwp = DeferWindowPos(cl.hdwp, hText, NULL, 0, 0, 
    newTextWidth, newTextHeight, SWP_NOZORDER|SWP_NOMOVE);

  cl.hDlg = hDlg;
  cl.hText = hText;
  cl.sizeX = sizeX;
  cl.sizeY = sizeY;
  cl.newSizeX = newSizeX;
  cl.newSizeY = newSizeY;
  EnumChildWindows(hDlg, ResizeEditPlusButtonsCallback, (LPARAM)&cl);

  EndDeferWindowPos(cl.hdwp);
}

BOOL CenterWindowEx(HWND hwndChild, HWND hwndParent, int mode)
{
    RECT    rChild, rParent;
    int     wChild, hChild, wParent, hParent;
    int     wScreen, hScreen, xNew, yNew;
    HDC     hdc;

    /* Get the Height and Width of the child window */
    GetWindowRect (hwndChild, &rChild);
    wChild = rChild.right - rChild.left;
    hChild = rChild.bottom - rChild.top;

    /* Get the Height and Width of the parent window */
    GetWindowRect (hwndParent, &rParent);
    wParent = rParent.right - rParent.left;
    hParent = rParent.bottom - rParent.top;

    /* Get the display limits */
    hdc = GetDC (hwndChild);
    wScreen = GetDeviceCaps (hdc, HORZRES);
    hScreen = GetDeviceCaps (hdc, VERTRES);
    ReleaseDC(hwndChild, hdc);

    /* Calculate new X position, then adjust for screen */
    xNew = rParent.left + ((wParent - wChild) /2);
    if (xNew < 0) {
	xNew = 0;
    } else if ((xNew+wChild) > wScreen) {
	xNew = wScreen - wChild;
    }

    /* Calculate new Y position, then adjust for screen */
    if( mode == 0 ) {
        yNew = rParent.top  + ((hParent - hChild) /2);
    }
    else {
        yNew = rParent.top + GetSystemMetrics( SM_CYCAPTION ) * 2 / 3;
    }

    if (yNew < 0) {
	yNew = 0;
    } else if ((yNew+hChild) > hScreen) {
	yNew = hScreen - hChild;
    }

    /* Set it, and return */
    return SetWindowPos (hwndChild, NULL,
			 xNew, yNew, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

/* Center one window over another */
BOOL CenterWindow (HWND hwndChild, HWND hwndParent)
{
    return CenterWindowEx( hwndChild, hwndParent, 0 );
}

/*---------------------------------------------------------------------------*\
 *
 * Startup Dialog functions
 *
\*---------------------------------------------------------------------------*/
void
InitComboStrings(HANDLE hwndCombo, char **cd)
{
  SendMessage(hwndCombo, CB_RESETCONTENT, 0, 0);

  while (*cd != NULL) {
    SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM) T_(*cd));
    cd++;
  }
}

void
InitComboStringsFromOption(HANDLE hwndCombo, char *str)
{
  char buf1[MAX_ARG_LEN];
  int len;

  if (str[0] == '@') {
    FILE* f = fopen(str + 1, "r");
    if (f == NULL) {
      DisplayFatalError(str + 1, errno, 2);
      return;
    }
    len = fread(buf1, 1, sizeof(buf1)-1, f);
    fclose(f);
    buf1[len] = NULLCHAR;
    str = buf1;
  }

  SendMessage(hwndCombo, CB_RESETCONTENT, 0, 0);

  for (;;) {
    char buf[MSG_SIZ];
    char *end = strchr(str, '\n');
    if (end == NULL) return;
    memcpy(buf, str, end - str);
    buf[end - str] = NULLCHAR;
    SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM) buf);
    str = end + 1;
  }
}

void
SetStartupDialogEnables(HWND hDlg)
{
  EnableWindow(GetDlgItem(hDlg, OPT_ChessEngineName),
    IsDlgButtonChecked(hDlg, OPT_ChessEngine) ||
    (appData.zippyPlay && IsDlgButtonChecked(hDlg, OPT_ChessServer)));
  EnableWindow(GetDlgItem(hDlg, OPT_SecondChessEngineName),
    IsDlgButtonChecked(hDlg, OPT_ChessEngine));
  EnableWindow(GetDlgItem(hDlg, OPT_ChessServerName),
    IsDlgButtonChecked(hDlg, OPT_ChessServer));
  EnableWindow(GetDlgItem(hDlg, OPT_AdditionalOptions),
    IsDlgButtonChecked(hDlg, OPT_AnyAdditional));
  EnableWindow(GetDlgItem(hDlg, IDOK),
    IsDlgButtonChecked(hDlg, OPT_ChessEngine) ||
    IsDlgButtonChecked(hDlg, OPT_ChessServer) ||
    IsDlgButtonChecked(hDlg, OPT_View));
}

char *
QuoteForFilename(char *filename)
{
  int dquote, space;
  dquote = strchr(filename, '"') != NULL;
  space = strchr(filename, ' ') != NULL;
  if (dquote || space) {
    if (dquote) {
      return "'";
    } else {
      return "\"";
    }
  } else {
    return "";
  }
}

VOID
InitEngineBox(HWND hDlg, HWND hwndCombo, char* nthcp, char* nthd, char* nthdir, char *nthnames)
{
  char buf[MSG_SIZ];
  char *q;

  InitComboStringsFromOption(hwndCombo, nthnames);
  q = QuoteForFilename(nthcp);
    snprintf(buf, MSG_SIZ, "%s%s%s", q, nthcp, q);
  if (*nthdir != NULLCHAR) {
    q = QuoteForFilename(nthdir);
      snprintf(buf + strlen(buf), MSG_SIZ, " /%s=%s%s%s", nthd, q, nthdir, q);
  }
  if (*nthcp == NULLCHAR) {
    SendMessage(hwndCombo, CB_SETCURSEL, (WPARAM) 0, (LPARAM) 0);
  } else if (SendMessage(hwndCombo, CB_SELECTSTRING, (WPARAM) -1, (LPARAM) buf) == CB_ERR) {
    SendMessage(hwndCombo, CB_SETCURSEL, (WPARAM) -1, (LPARAM) 0);
    SendMessage(hwndCombo, WM_SETTEXT, (WPARAM) 0, (LPARAM) buf);
  }
}

LRESULT CALLBACK
StartupDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  char buf[MSG_SIZ];
  HANDLE hwndCombo;
  char *p;

  switch (message) {
  case WM_INITDIALOG:
    /* Center the dialog */
    CenterWindow (hDlg, GetDesktopWindow());
    Translate(hDlg, DLG_Startup);
    /* Initialize the dialog items */
    InitEngineBox(hDlg, GetDlgItem(hDlg, OPT_ChessEngineName),
	          appData.firstChessProgram, "fd", appData.firstDirectory,
		  firstChessProgramNames);
    InitEngineBox(hDlg, GetDlgItem(hDlg, OPT_SecondChessEngineName),
	          appData.secondChessProgram, singleList ? "fd" : "sd", appData.secondDirectory,
		  singleList ? firstChessProgramNames : secondChessProgramNames); //[HGM] single: use first list in second combo
    hwndCombo = GetDlgItem(hDlg, OPT_ChessServerName);
    InitComboStringsFromOption(hwndCombo, icsNames);    
      snprintf(buf, MSG_SIZ, "%s /icsport=%s", appData.icsHost, appData.icsPort);
    if (*appData.icsHelper != NULLCHAR) {
      char *q = QuoteForFilename(appData.icsHelper);
      sprintf(buf + strlen(buf), " /icshelper=%s%s%s", q, appData.icsHelper, q);
    }
    if (*appData.icsHost == NULLCHAR) {
      SendMessage(hwndCombo, CB_SETCURSEL, (WPARAM) 0, (LPARAM) 0);
      /*SendMessage(hwndCombo, CB_SHOWDROPDOWN, (WPARAM) TRUE, (LPARAM) 0); !!too soon */
    } else if (SendMessage(hwndCombo, CB_SELECTSTRING, (WPARAM) -1, (LPARAM) buf) == CB_ERR) {
      SendMessage(hwndCombo, CB_SETCURSEL, (WPARAM) -1, (LPARAM) 0);
      SendMessage(hwndCombo, WM_SETTEXT, (WPARAM) 0, (LPARAM) buf);
    }

    if (appData.icsActive) {
      CheckDlgButton(hDlg, OPT_ChessServer, BST_CHECKED);
    }
    else if (appData.noChessProgram) {
      CheckDlgButton(hDlg, OPT_View, BST_CHECKED);
    }
    else {
      CheckDlgButton(hDlg, OPT_ChessEngine, BST_CHECKED);
    }

    SetStartupDialogEnables(hDlg);
    return TRUE;

  case WM_COMMAND:
    switch (LOWORD(wParam)) {
    case IDOK:
      if (IsDlgButtonChecked(hDlg, OPT_ChessEngine)) {
        safeStrCpy(buf, "/fcp=", sizeof(buf)/sizeof(buf[0]) );
	GetDlgItemText(hDlg, OPT_ChessEngineName, buf + strlen(buf), sizeof(buf) - strlen(buf));
        p = buf;
	comboLine = strdup(p+5); // [HGM] recent: remember complete line of first combobox
	ParseArgs(StringGet, &p);
	safeStrCpy(buf, singleList ? "/fcp=" : "/scp=", sizeof(buf)/sizeof(buf[0]) );
	GetDlgItemText(hDlg, OPT_SecondChessEngineName, buf + strlen(buf), sizeof(buf) - strlen(buf));
        p = buf;
	SwapEngines(singleList); // temporarily swap first and second, to load a second 'first', ...
	ParseArgs(StringGet, &p);
	SwapEngines(singleList); // ... and then make it 'second'

	appData.noChessProgram = FALSE;
	appData.icsActive = FALSE;
      } else if (IsDlgButtonChecked(hDlg, OPT_ChessServer)) {
        safeStrCpy(buf, "/ics /icshost=", sizeof(buf)/sizeof(buf[0]) );
	GetDlgItemText(hDlg, OPT_ChessServerName, buf + strlen(buf), sizeof(buf) - strlen(buf));
        p = buf;
	ParseArgs(StringGet, &p);
	if (appData.zippyPlay) {
	  safeStrCpy(buf, "/fcp=", sizeof(buf)/sizeof(buf[0]) );
  	  GetDlgItemText(hDlg, OPT_ChessEngineName, buf + strlen(buf), sizeof(buf) - strlen(buf));
	  p = buf;
	  ParseArgs(StringGet, &p);
	}
      } else if (IsDlgButtonChecked(hDlg, OPT_View)) {
	appData.noChessProgram = TRUE;
	appData.icsActive = FALSE;
      } else {
	MessageBox(hDlg, _("Choose an option, or cancel to exit"),
		   _("Option Error"), MB_OK|MB_ICONEXCLAMATION);
	return TRUE;
      }
      if (IsDlgButtonChecked(hDlg, OPT_AnyAdditional)) {
	GetDlgItemText(hDlg, OPT_AdditionalOptions, buf, sizeof(buf));
	p = buf;
	ParseArgs(StringGet, &p);
      }
      EndDialog(hDlg, TRUE);
      return TRUE;

    case IDCANCEL:
      ExitEvent(0);
      return TRUE;

    case IDM_HELPCONTENTS:
      if (!WinHelp (hDlg, "winboard.hlp", HELP_KEY,(DWORD)(LPSTR)"CONTENTS")) {
	MessageBox (GetFocus(),
		    _("Unable to activate help"),
		    szAppName, MB_SYSTEMMODAL|MB_OK|MB_ICONHAND);
      }
      break;

    default:
      SetStartupDialogEnables(hDlg);
      break;
    }
    break;
  }
  return FALSE;
}

/*---------------------------------------------------------------------------*\
 *
 * About box dialog functions
 *
\*---------------------------------------------------------------------------*/

/* Process messages for "About" dialog box */
LRESULT CALLBACK
About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message) {
  case WM_INITDIALOG: /* message: initialize dialog box */
    /* Center the dialog over the application window */
    CenterWindow (hDlg, GetWindow (hDlg, GW_OWNER));
    SetDlgItemText(hDlg, ABOUTBOX_Version, programVersion);
    Translate(hDlg, ABOUTBOX);
    JAWS_COPYRIGHT
    return (TRUE);

  case WM_COMMAND: /* message: received a command */
    if (LOWORD(wParam) == IDOK /* "OK" box selected? */
	|| LOWORD(wParam) == IDCANCEL) { /* System menu close command? */
      EndDialog(hDlg, TRUE); /* Exit the dialog */
      return (TRUE);
    }
    break;
  }
  return (FALSE);
}

/*---------------------------------------------------------------------------*\
 *
 * Comment Dialog functions
 *
\*---------------------------------------------------------------------------*/

LRESULT CALLBACK
CommentDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  static HANDLE hwndText = NULL;
  int len, newSizeX, newSizeY;
  static int sizeX, sizeY;
  char *str;
  RECT rect;
  MINMAXINFO *mmi;

  switch (message) {
  case WM_INITDIALOG: /* message: initialize dialog box */
    /* Initialize the dialog items */
    Translate(hDlg, DLG_EditComment);
    hwndText = GetDlgItem(hDlg, OPT_CommentText);
    SetDlgItemText(hDlg, OPT_CommentText, commentText);
    EnableWindow(GetDlgItem(hDlg, OPT_CancelComment), editComment);
    EnableWindow(GetDlgItem(hDlg, OPT_ClearComment), editComment);
    EnableWindow(GetDlgItem(hDlg, OPT_EditComment), !editComment);
    SendMessage(hwndText, EM_SETREADONLY, !editComment, 0);
    SetWindowText(hDlg, commentTitle);
    if (editComment) {
      SetFocus(hwndText);
    } else {
      SetFocus(GetDlgItem(hDlg, IDOK));
    }
    SendMessage(GetDlgItem(hDlg, OPT_CommentText),
		WM_SETFONT, (WPARAM)font[boardSize][COMMENT_FONT]->hf,
		MAKELPARAM(FALSE, 0));
    /* Size and position the dialog */
    if (!commentDialog) {
      commentDialog = hDlg;
      GetClientRect(hDlg, &rect);
      sizeX = rect.right;
      sizeY = rect.bottom;
      if (wpComment.x != CW_USEDEFAULT && wpComment.y != CW_USEDEFAULT &&
	  wpComment.width != CW_USEDEFAULT && wpComment.height != CW_USEDEFAULT) {
	WINDOWPLACEMENT wp;
	EnsureOnScreen(&wpComment.x, &wpComment.y, 0, 0);
	wp.length = sizeof(WINDOWPLACEMENT);
	wp.flags = 0;
	wp.showCmd = SW_SHOW;
	wp.ptMaxPosition.x = wp.ptMaxPosition.y = 0;
	wp.rcNormalPosition.left = wpComment.x;
	wp.rcNormalPosition.right = wpComment.x + wpComment.width;
	wp.rcNormalPosition.top = wpComment.y;
	wp.rcNormalPosition.bottom = wpComment.y + wpComment.height;
	SetWindowPlacement(hDlg, &wp);

	GetClientRect(hDlg, &rect);
	newSizeX = rect.right;
	newSizeY = rect.bottom;
        ResizeEditPlusButtons(hDlg, hwndText, sizeX, sizeY,
			      newSizeX, newSizeY);
	sizeX = newSizeX;
	sizeY = newSizeY;
      }
    }
    SendDlgItemMessage( hDlg, OPT_CommentText, EM_SETEVENTMASK, 0, ENM_MOUSEEVENTS | ENM_KEYEVENTS );
    return FALSE;

  case WM_COMMAND: /* message: received a command */
    switch (LOWORD(wParam)) {
    case IDOK:
      if (editComment) {
	char *p, *q;
	/* Read changed options from the dialog box */
	hwndText = GetDlgItem(hDlg, OPT_CommentText);
	len = GetWindowTextLength(hwndText);
	str = (char *) malloc(len + 1);
	GetWindowText(hwndText, str, len + 1);
	p = q = str;
	while (*q) {
	  if (*q == '\r')
	    q++;
	  else
	    *p++ = *q++;
	}
	*p = NULLCHAR;
	ReplaceComment(commentIndex, str);
	free(str);
      }
      CommentPopDown();
      return TRUE;

    case IDCANCEL:
    case OPT_CancelComment:
      CommentPopDown();
      return TRUE;

    case OPT_ClearComment:
      SetDlgItemText(hDlg, OPT_CommentText, "");
      break;

    case OPT_EditComment:
      EditCommentEvent();
      return TRUE;

    default:
      break;
    }
    break;

  case WM_NOTIFY: // [HGM] vari: cloned from whistory.c
        if( wParam == OPT_CommentText ) {
            MSGFILTER * lpMF = (MSGFILTER *) lParam;

            if( lpMF->msg == WM_RBUTTONDOWN && (lpMF->wParam & (MK_CONTROL | MK_SHIFT)) == 0 ||
                lpMF->msg == WM_CHAR && lpMF->wParam == '\022' ) {
                POINTL pt;
                LRESULT index;

                pt.x = LOWORD( lpMF->lParam );
                pt.y = HIWORD( lpMF->lParam );

                if(lpMF->msg == WM_CHAR) {
                        CHARRANGE sel;
                        SendDlgItemMessage( hDlg, OPT_CommentText, EM_EXGETSEL, 0, (LPARAM) &sel );
                        index = sel.cpMin;
                } else
                index = SendDlgItemMessage( hDlg, OPT_CommentText, EM_CHARFROMPOS, 0, (LPARAM) &pt );

		hwndText = GetDlgItem(hDlg, OPT_CommentText); // cloned from above
		len = GetWindowTextLength(hwndText);
		str = (char *) malloc(len + 1);
		GetWindowText(hwndText, str, len + 1);
		ReplaceComment(commentIndex, str);
		if(commentIndex != currentMove) ToNrEvent(commentIndex);
                LoadVariation( index, str ); // [HGM] also does the actual moving to it, now
		free(str);

                /* Zap the message for good: apparently, returning non-zero is not enough */
                lpMF->msg = WM_USER;

                return TRUE;
            }
        }
        break;

  case WM_SIZE:
    newSizeX = LOWORD(lParam);
    newSizeY = HIWORD(lParam);
    ResizeEditPlusButtons(hDlg, hwndText, sizeX, sizeY, newSizeX, newSizeY);
    sizeX = newSizeX;
    sizeY = newSizeY;
    break;

  case WM_GETMINMAXINFO:
    /* Prevent resizing window too small */
    mmi = (MINMAXINFO *) lParam;
    mmi->ptMinTrackSize.x = 100;
    mmi->ptMinTrackSize.y = 100;
    break;
  }
  return FALSE;
}

VOID
EitherCommentPopUp(int index, char *title, char *str, BOOLEAN edit)
{
  FARPROC lpProc;
  char *p, *q;

  CheckMenuItem(GetMenu(hwndMain), IDM_Comment, edit ? MF_CHECKED : MF_UNCHECKED);

  if (str == NULL) str = "";
  p = (char *) malloc(2 * strlen(str) + 2);
  q = p;
  while (*str) {
    if (*str == '\n') *q++ = '\r';
    *q++ = *str++;
  }
  *q = NULLCHAR;
  if (commentText != NULL) free(commentText);

  commentIndex = index;
  commentTitle = title;
  commentText = p;
  editComment = edit;

  if (commentDialog) {
    SendMessage(commentDialog, WM_INITDIALOG, 0, 0);
    if (!commentUp) ShowWindow(commentDialog, SW_SHOW);
  } else {
    lpProc = MakeProcInstance((FARPROC)CommentDialog, hInst);
    CreateDialog(hInst, MAKEINTRESOURCE(DLG_EditComment),
		 hwndMain, (DLGPROC)lpProc);
    FreeProcInstance(lpProc);
  }
  commentUp = TRUE;
}


/*---------------------------------------------------------------------------*\
 *
 * Type-in move dialog functions
 * 
\*---------------------------------------------------------------------------*/

LRESULT CALLBACK
TypeInMoveDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  char move[MSG_SIZ];
  HWND hInput;

  switch (message) {
  case WM_INITDIALOG:
    move[0] = (char) lParam;
    move[1] = NULLCHAR;
    CenterWindowEx(hDlg, GetWindow(hDlg, GW_OWNER), 1 );
    Translate(hDlg, DLG_TypeInMove);
    hInput = GetDlgItem(hDlg, OPT_Move);
    SetWindowText(hInput, move);
    SetFocus(hInput);
    SendMessage(hInput, EM_SETSEL, (WPARAM)9999, (LPARAM)9999);
    return FALSE;

  case WM_COMMAND:
    switch (LOWORD(wParam)) {
    case IDOK:

      shiftKey = GetKeyState(VK_SHIFT) < 0; // [HGM] remember last shift status
      GetDlgItemText(hDlg, OPT_Move, move, sizeof(move));
      TypeInDoneEvent(move);
      EndDialog(hDlg, TRUE);
      return TRUE;
    case IDCANCEL:
      EndDialog(hDlg, FALSE);
      return TRUE;
    default:
      break;
    }
    break;
  }
  return FALSE;
}

VOID
PopUpMoveDialog(char firstchar)
{
    FARPROC lpProc;

      lpProc = MakeProcInstance((FARPROC)TypeInMoveDialog, hInst);
      DialogBoxParam(hInst, MAKEINTRESOURCE(DLG_TypeInMove),
	hwndMain, (DLGPROC)lpProc, (LPARAM)firstchar);
      FreeProcInstance(lpProc);
}

/*---------------------------------------------------------------------------*\
 *
 * Type-in name dialog functions
 * 
\*---------------------------------------------------------------------------*/

LRESULT CALLBACK
TypeInNameDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  char move[MSG_SIZ];
  HWND hInput;

  switch (message) {
  case WM_INITDIALOG:
    move[0] = (char) lParam;
    move[1] = NULLCHAR;
    CenterWindowEx(hDlg, GetWindow(hDlg, GW_OWNER), 1 );
    Translate(hDlg, DLG_TypeInName);
    hInput = GetDlgItem(hDlg, OPT_Name);
    SetWindowText(hInput, move);
    SetFocus(hInput);
    SendMessage(hInput, EM_SETSEL, (WPARAM)9999, (LPARAM)9999);
    return FALSE;

  case WM_COMMAND:
    switch (LOWORD(wParam)) {
    case IDOK:
      GetDlgItemText(hDlg, OPT_Name, move, sizeof(move));
      appData.userName = strdup(move);
      SetUserLogo();
      SetGameInfo();
      if(gameMode == MachinePlaysWhite || gameMode == MachinePlaysBlack) {
	snprintf(move, MSG_SIZ, "%s vs. %s", gameInfo.white, gameInfo.black);
	DisplayTitle(move);
      }


      EndDialog(hDlg, TRUE);
      return TRUE;
    case IDCANCEL:
      EndDialog(hDlg, FALSE);
      return TRUE;
    default:
      break;
    }
    break;
  }
  return FALSE;
}

VOID
PopUpNameDialog(char firstchar)
{
    FARPROC lpProc;
    
      lpProc = MakeProcInstance((FARPROC)TypeInNameDialog, hInst);
      DialogBoxParam(hInst, MAKEINTRESOURCE(DLG_TypeInName),
	hwndMain, (DLGPROC)lpProc, (LPARAM)firstchar);
      FreeProcInstance(lpProc);
}

/*---------------------------------------------------------------------------*\
 *
 *  Error dialogs
 * 
\*---------------------------------------------------------------------------*/

/* Nonmodal error box */
LRESULT CALLBACK ErrorDialog(HWND hDlg, UINT message,
			     WPARAM wParam, LPARAM lParam);

VOID
ErrorPopUp(char *title, char *content)
{
  FARPROC lpProc;
  char *p, *q;
  BOOLEAN modal = hwndMain == NULL;

  p = content;
  q = errorMessage;
  while (*p) {
    if (*p == '\n') {
      if (modal) {
	*q++ = ' ';
	p++;
      } else {
	*q++ = '\r';
	*q++ = *p++;
      }
    } else {
      *q++ = *p++;
    }
  }
  *q = NULLCHAR;
  strncpy(errorTitle, title, sizeof(errorTitle));
  errorTitle[sizeof(errorTitle) - 1] = '\0';
  
  if (modal) {
    MessageBox(NULL, errorMessage, errorTitle, MB_OK|MB_ICONEXCLAMATION);
  } else {
    lpProc = MakeProcInstance((FARPROC)ErrorDialog, hInst);
    CreateDialog(hInst, MAKEINTRESOURCE(DLG_Error),
		 hwndMain, (DLGPROC)lpProc);
    FreeProcInstance(lpProc);
  }
}

VOID
ErrorPopDown()
{
  if (!appData.popupMoveErrors && moveErrorMessageUp) DisplayMessage("", "");
  if (errorDialog == NULL) return;
  DestroyWindow(errorDialog);
  errorDialog = NULL;
  if(errorExitStatus) ExitEvent(errorExitStatus);
}

LRESULT CALLBACK
ErrorDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  RECT rChild;

  switch (message) {
  case WM_INITDIALOG:
    GetWindowRect(hDlg, &rChild);

    /*
    SetWindowPos(hDlg, NULL, rChild.left,
      rChild.top + boardRect.top - (rChild.bottom - rChild.top), 
      0, 0, SWP_NOZORDER|SWP_NOSIZE);
    */

    /* 
        [AS] It seems that the above code wants to move the dialog up in the "caption
        area" of the main window, but it uses the dialog height as an hard-coded constant,
        and it doesn't work when you resize the dialog.
        For now, just give it a default position.
    */
    SetWindowPos(hDlg, NULL, boardRect.left+8, boardRect.top+8, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
    Translate(hDlg, DLG_Error);

    errorDialog = hDlg;
    SetWindowText(hDlg, errorTitle);
    SetDlgItemText(hDlg, OPT_ErrorText, errorMessage);
    return FALSE;

  case WM_COMMAND:
    switch (LOWORD(wParam)) {
    case IDOK:
    case IDCANCEL:
      if (errorDialog == hDlg) errorDialog = NULL;
      DestroyWindow(hDlg);
      return TRUE;

    default:
      break;
    }
    break;
  }
  return FALSE;
}

#ifdef GOTHIC
HWND gothicDialog = NULL;

LRESULT CALLBACK
GothicDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  RECT rChild;
  int height = GetSystemMetrics(SM_CYCAPTION)+GetSystemMetrics(SM_CYFRAME);

  switch (message) {
  case WM_INITDIALOG:
    GetWindowRect(hDlg, &rChild);

    SetWindowPos(hDlg, NULL, wpMain.x, wpMain.y-height, wpMain.width, height,
                                                             SWP_NOZORDER);

    /* 
        [AS] It seems that the above code wants to move the dialog up in the "caption
        area" of the main window, but it uses the dialog height as an hard-coded constant,
        and it doesn't work when you resize the dialog.
        For now, just give it a default position.
    */
    gothicDialog = hDlg;
    SetWindowText(hDlg, errorTitle);
    SetDlgItemText(hDlg, OPT_ErrorText, errorMessage);
    return FALSE;

  case WM_COMMAND:
    switch (LOWORD(wParam)) {
    case IDOK:
    case IDCANCEL:
      if (errorDialog == hDlg) errorDialog = NULL;
      DestroyWindow(hDlg);
      return TRUE;

    default:
      break;
    }
    break;
  }
  return FALSE;
}

VOID
GothicPopUp(char *title, VariantClass variant)
{
  FARPROC lpProc;
  static char *lastTitle;

  strncpy(errorTitle, title, sizeof(errorTitle));
  errorTitle[sizeof(errorTitle) - 1] = '\0';

  if(lastTitle != title && gothicDialog != NULL) {
    DestroyWindow(gothicDialog);
    gothicDialog = NULL;
  }
  if(variant != VariantNormal && gothicDialog == NULL) {
    title = lastTitle;
    lpProc = MakeProcInstance((FARPROC)GothicDialog, hInst);
    CreateDialog(hInst, MAKEINTRESOURCE(DLG_Error),
		 hwndMain, (DLGPROC)lpProc);
    FreeProcInstance(lpProc);
  }
}
#endif

/*---------------------------------------------------------------------------*\
 *
 *  Ics Interaction console functions
 *
\*---------------------------------------------------------------------------*/

#define HISTORY_SIZE 64
static char *history[HISTORY_SIZE];
int histIn = 0, histP = 0;


VOID
SaveInHistory(char *cmd)
{
  if (history[histIn] != NULL) {
    free(history[histIn]);
    history[histIn] = NULL;
  }
  if (*cmd == NULLCHAR) return;
  history[histIn] = StrSave(cmd);
  histIn = (histIn + 1) % HISTORY_SIZE;
  if (history[histIn] != NULL) {
    free(history[histIn]);

    history[histIn] = NULL;
  }
  histP = histIn;
}

char *
PrevInHistory(char *cmd)
{
  int newhp;
  if (histP == histIn) {
    if (history[histIn] != NULL) free(history[histIn]);
    history[histIn] = StrSave(cmd);
  }
  newhp = (histP - 1 + HISTORY_SIZE) % HISTORY_SIZE;
  if (newhp == histIn || history[newhp] == NULL) return NULL;
  histP = newhp;
  return history[histP];
}

char *
NextInHistory()
{
  if (histP == histIn) return NULL;
  histP = (histP + 1) % HISTORY_SIZE;
  return history[histP];   
}

HMENU
LoadIcsTextMenu(IcsTextMenuEntry *e)
{
  HMENU hmenu, h;
  int i = 0;
  hmenu = LoadMenu(hInst, "TextMenu");
  h = GetSubMenu(hmenu, 0);
  while (e->item) {
    if (strcmp(e->item, "-") == 0) {
      AppendMenu(h, MF_SEPARATOR, 0, 0);
    } else { // [HGM] re-written a bit to use only one AppendMenu call for both cases (| or no |)
      int flags = MF_STRING, j = 0;
      if (e->item[0] == '|') {
	flags |= MF_MENUBARBREAK;
        j++;
      }
      if(!strcmp(e->command, "none")) flags |= MF_GRAYED; // [HGM] chatclick: provide inactive dummy
      AppendMenu(h, flags, IDM_CommandX + i, e->item + j);
    }
    e++;
    i++;
  } 
  return hmenu;
}

WNDPROC consoleTextWindowProc;

void
CommandX(HWND hwnd, char *command, BOOLEAN getname, BOOLEAN immediate)
{
  char buf[MSG_SIZ], name[MSG_SIZ];
  HWND hInput = GetDlgItem(hwndConsole, OPT_ConsoleInput);
  CHARRANGE sel;

  if (!getname) {
    SetWindowText(hInput, command);
    if (immediate) {
      SendMessage(hInput, WM_CHAR, '\r', 0);
    } else {
      sel.cpMin = 999999;
      sel.cpMax = 999999;
      SendMessage(hInput, EM_EXSETSEL, 0, (LPARAM)&sel);
      SetFocus(hInput);
    }
    return;
  }    
  SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM)&sel);
  if (sel.cpMin == sel.cpMax) {
    /* Expand to surrounding word */
    TEXTRANGE tr;
    do {
      tr.chrg.cpMax = sel.cpMin;
      tr.chrg.cpMin = --sel.cpMin;
      if (sel.cpMin < 0) break;
      tr.lpstrText = name;
      SendMessage(hwnd, EM_GETTEXTRANGE, 0, (LPARAM) &tr);
    } while (isalpha(name[0]) || isdigit(name[0]) || name[0] == '-');
    sel.cpMin++;

    do {
      tr.chrg.cpMin = sel.cpMax;
      tr.chrg.cpMax = ++sel.cpMax;
      tr.lpstrText = name;
      if (SendMessage(hwnd, EM_GETTEXTRANGE, 0, (LPARAM) &tr) < 1) break;
    } while (isalpha(name[0]) || isdigit(name[0]) || name[0] == '-');
    sel.cpMax--;

    if (sel.cpMax == sel.cpMin || sel.cpMax - sel.cpMin > MSG_SIZ/2) {
      MessageBeep(MB_ICONEXCLAMATION);
      return;
    }
    tr.chrg = sel;
    tr.lpstrText = name;
    SendMessage(hwnd, EM_GETTEXTRANGE, 0, (LPARAM) &tr);
  } else {
    if (sel.cpMax - sel.cpMin > MSG_SIZ/2) {
      MessageBeep(MB_ICONEXCLAMATION);
      return;
    }
    SendMessage(hwnd, EM_GETSELTEXT, 0, (LPARAM) name);
  }
  if (immediate) {
    if(strstr(command, "%s")) snprintf(buf, MSG_SIZ, command, name); else
    snprintf(buf, MSG_SIZ, "%s %s", command, name);
    SetWindowText(hInput, buf);
    SendMessage(hInput, WM_CHAR, '\r', 0);
  } else {
    if(!strcmp(command, "chat")) { ChatPopUp(name); return; }
      snprintf(buf, MSG_SIZ, "%s %s ", command, name); /* trailing space */
    SetWindowText(hInput, buf);
    sel.cpMin = 999999;
    sel.cpMax = 999999;
    SendMessage(hInput, EM_EXSETSEL, 0, (LPARAM)&sel);
    SetFocus(hInput);
  }
}

LRESULT CALLBACK 
ConsoleTextSubclass(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  HWND hInput;
  CHARRANGE sel;

  switch (message) {
  case WM_KEYDOWN:
    if (!(GetKeyState(VK_CONTROL) & ~1)) break;
    if(wParam=='R') return 0;
    switch (wParam) {
    case VK_PRIOR:
      SendMessage(hwnd, EM_LINESCROLL, 0, -999999);
      return 0;
    case VK_NEXT:
      sel.cpMin = 999999;
      sel.cpMax = 999999;
      SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM)&sel);
      SendMessage(hwnd, EM_SCROLLCARET, 0, 0);
      return 0;
    }
    break;
  case WM_CHAR:
   if(wParam != '\022') {
    if (wParam == '\t') {
      if (GetKeyState(VK_SHIFT) < 0) {
	/* shifted */
	if (IsIconic(hwndMain)) ShowWindow(hwndMain, SW_RESTORE);
	if (buttonDesc[0].hwnd) {
	  SetFocus(buttonDesc[0].hwnd);
	} else {
	  SetFocus(hwndMain);
	}
      } else {
	/* unshifted */
	SetFocus(GetDlgItem(hwndConsole, OPT_ConsoleInput));
      }
    } else {
      hInput = GetDlgItem(hwndConsole, OPT_ConsoleInput);
      JAWS_DELETE( SetFocus(hInput); )
      SendMessage(hInput, message, wParam, lParam);
    }
    return 0;
   } // [HGM] navigate: for Ctrl+R, flow into next case (moved up here) to summon up menu
   lParam = -1;
  case WM_RBUTTONDOWN:
    if (!(GetKeyState(VK_SHIFT) & ~1)) {
      /* Move selection here if it was empty */
      POINT pt;
      pt.x = LOWORD(lParam);
      pt.y = HIWORD(lParam);
      SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM)&sel);
      if (sel.cpMin == sel.cpMax) {
        if(lParam != -1) sel.cpMin = SendMessage(hwnd, EM_CHARFROMPOS, 0, (LPARAM)&pt); /*doc is wrong*/
	sel.cpMax = sel.cpMin;
	SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM)&sel);
      }
      SendMessage(hwnd, EM_HIDESELECTION, FALSE, FALSE);
{ // [HGM] chatclick: code moved here from WM_RBUTTONUP case, to have menu appear on down-click
      POINT pt;
      HMENU hmenu = LoadIcsTextMenu(icsTextMenuEntry);
      SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM)&sel);
      if (sel.cpMin == sel.cpMax) {
        EnableMenuItem(hmenu, IDM_Copy, MF_BYCOMMAND|MF_GRAYED);
        EnableMenuItem(hmenu, IDM_QuickPaste, MF_BYCOMMAND|MF_GRAYED);
      }
      if (!IsClipboardFormatAvailable(CF_TEXT)) {
        EnableMenuItem(hmenu, IDM_Paste, MF_BYCOMMAND|MF_GRAYED);
      }
      pt.x = LOWORD(lParam)-30; // [HGM] chatclick: make menu pop up with pointer above upper-right item
      pt.y = HIWORD(lParam)-10; //       make it appear as if mouse moved there, so it will be selected on up-click
      PostMessage(hwnd, WM_MOUSEMOVE, wParam, lParam+5);
      MenuPopup(hwnd, pt, hmenu, -1);
}
    }
    return 0;
  case WM_RBUTTONUP:
    if (GetKeyState(VK_SHIFT) & ~1) {
      SendDlgItemMessage(hwndConsole, OPT_ConsoleText, 
        WM_COMMAND, MAKEWPARAM(IDM_QuickPaste, 0), 0);
    }
    return 0;
  case WM_PASTE:
    hInput = GetDlgItem(hwndConsole, OPT_ConsoleInput);
    SetFocus(hInput);
    return SendMessage(hInput, message, wParam, lParam);
  case WM_MBUTTONDOWN:
    return SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDM_QuickPaste, 0), 0);
  case WM_COMMAND:
    switch (LOWORD(wParam)) {
    case IDM_QuickPaste:
      {
        SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM)&sel);
	if (sel.cpMin == sel.cpMax) {
	  MessageBeep(MB_ICONEXCLAMATION);
          return 0;
	}
	SendMessage(hwnd, WM_COPY, 0, 0);
	hInput = GetDlgItem(hwndConsole, OPT_ConsoleInput);
        SendMessage(hInput, WM_PASTE, 0, 0);
        SetFocus(hInput);
        return 0;
      }
    case IDM_Cut:
      SendMessage(hwnd, WM_CUT, 0, 0);
      return 0;
    case IDM_Paste:
      SendMessage(hwnd, WM_PASTE, 0, 0);
      return 0;
    case IDM_Copy:
      SendMessage(hwnd, WM_COPY, 0, 0);
      return 0;
    default:
      {
	int i = LOWORD(wParam) - IDM_CommandX;
	if (i >= 0 && i < ICS_TEXT_MENU_SIZE &&
	    icsTextMenuEntry[i].command != NULL) {
	  CommandX(hwnd, icsTextMenuEntry[i].command,
		   icsTextMenuEntry[i].getname,
		   icsTextMenuEntry[i].immediate);
	  return 0;
	}
      }
      break;
    }
    break;
  }
  return (*consoleTextWindowProc)(hwnd, message, wParam, lParam);
}

WNDPROC consoleInputWindowProc;

LRESULT CALLBACK
ConsoleInputSubclass(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  char buf[MSG_SIZ];
  char *p;
  static BOOL sendNextChar = FALSE;
  static BOOL quoteNextChar = FALSE;
  InputSource *is = consoleInputSource;
  CHARFORMAT cf;
  CHARRANGE sel;

  switch (message) {
  case WM_CHAR:
    if (!appData.localLineEditing || sendNextChar) {
      is->buf[0] = (CHAR) wParam;
      is->count = 1;
      SendMessage(hwndMain, WM_USER_Input, 0, (LPARAM) is);
      sendNextChar = FALSE;
      return 0;
    }
    if (quoteNextChar) {
      buf[0] = (char) wParam;
      buf[1] = NULLCHAR;
      SendMessage(hwnd, EM_REPLACESEL, TRUE, (LPARAM) buf);
      quoteNextChar = FALSE;
      return 0;
    }
    switch (wParam) {
    case '\r':   /* Enter key */
      is->count = GetWindowText(hwnd, is->buf, INPUT_SOURCE_BUF_SIZE-1);     
      if (consoleEcho) SaveInHistory(is->buf);
      is->buf[is->count++] = '\n';
      is->buf[is->count] = NULLCHAR;
      SendMessage(hwndMain, WM_USER_Input, 0, (LPARAM) is);
      if (consoleEcho) {
	ConsoleOutput(is->buf, is->count, TRUE);
      } else if (appData.localLineEditing) {
	ConsoleOutput("\n", 1, TRUE);
      }
      /* fall thru */
    case '\033': /* Escape key */
      SetWindowText(hwnd, "");
      cf.cbSize = sizeof(CHARFORMAT);
      cf.dwMask = CFM_COLOR|CFM_BOLD|CFM_ITALIC|CFM_UNDERLINE|CFM_STRIKEOUT;
      if (consoleEcho) {
        cf.crTextColor = textAttribs[ColorNormal].color;
      } else {
	cf.crTextColor = COLOR_ECHOOFF;
      }
      cf.dwEffects = textAttribs[ColorNormal].effects;
      SendMessage(hwnd, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
      return 0;
    case '\t':   /* Tab key */
      if (GetKeyState(VK_SHIFT) < 0) {
	/* shifted */
	SetFocus(GetDlgItem(hwndConsole, OPT_ConsoleText));
      } else {
	/* unshifted */
	if (IsIconic(hwndMain)) ShowWindow(hwndMain, SW_RESTORE);
	if (buttonDesc[0].hwnd) {
	  SetFocus(buttonDesc[0].hwnd);
	} else {
	  SetFocus(hwndMain);
	}
      }
      return 0;
    case '\023': /* Ctrl+S */
      sendNextChar = TRUE;
      return 0;
    case '\021': /* Ctrl+Q */
      quoteNextChar = TRUE;
      return 0;
    JAWS_REPLAY
    default:
      break;
    }
    break;
  case WM_KEYDOWN:
    switch (wParam) {
    case VK_UP:
      GetWindowText(hwnd, buf, MSG_SIZ);
      p = PrevInHistory(buf);
      if (p != NULL) {
	SetWindowText(hwnd, p);
	sel.cpMin = 999999;
	sel.cpMax = 999999;
	SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM)&sel);
        return 0;
      }
      break;
    case VK_DOWN:
      p = NextInHistory();
      if (p != NULL) {
	SetWindowText(hwnd, p);
	sel.cpMin = 999999;
	sel.cpMax = 999999;
	SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM)&sel);
        return 0;
      }
      break;
    case VK_HOME:
    case VK_END:
      if (!(GetKeyState(VK_CONTROL) & ~1)) break;
      /* fall thru */
    case VK_PRIOR:
    case VK_NEXT:
      SendDlgItemMessage(hwndConsole, OPT_ConsoleText, message, wParam, lParam);
      return 0;
    }
    break;
  case WM_MBUTTONDOWN:
    SendDlgItemMessage(hwndConsole, OPT_ConsoleText, 
      WM_COMMAND, MAKEWPARAM(IDM_QuickPaste, 0), 0);
    break;
  case WM_RBUTTONUP:
    if (GetKeyState(VK_SHIFT) & ~1) {
      SendDlgItemMessage(hwndConsole, OPT_ConsoleText, 
        WM_COMMAND, MAKEWPARAM(IDM_QuickPaste, 0), 0);
    } else {
      POINT pt;
      HMENU hmenu;
      hmenu = LoadMenu(hInst, "InputMenu");
      SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM)&sel);
      if (sel.cpMin == sel.cpMax) {
        EnableMenuItem(hmenu, IDM_Copy, MF_BYCOMMAND|MF_GRAYED);
        EnableMenuItem(hmenu, IDM_Cut, MF_BYCOMMAND|MF_GRAYED);
      }
      if (!IsClipboardFormatAvailable(CF_TEXT)) {
        EnableMenuItem(hmenu, IDM_Paste, MF_BYCOMMAND|MF_GRAYED);
      }
      pt.x = LOWORD(lParam);
      pt.y = HIWORD(lParam);
      MenuPopup(hwnd, pt, hmenu, -1);
    }
    return 0;
  case WM_COMMAND:
    switch (LOWORD(wParam)) { 
    case IDM_Undo:
      SendMessage(hwnd, EM_UNDO, 0, 0);
      return 0;
    case IDM_SelectAll:
      sel.cpMin = 0;
      sel.cpMax = -1; /*999999?*/
      SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM)&sel);
      return 0;
    case IDM_Cut:
      SendMessage(hwnd, WM_CUT, 0, 0);
      return 0;
    case IDM_Paste:
      SendMessage(hwnd, WM_PASTE, 0, 0);
      return 0;
    case IDM_Copy:
      SendMessage(hwnd, WM_COPY, 0, 0);
      return 0;
    }
    break;
  }
  return (*consoleInputWindowProc)(hwnd, message, wParam, lParam);
}

#define CO_MAX  100000
#define CO_TRIM   1000

LRESULT CALLBACK
ConsoleWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  static SnapData sd;
  HWND hText, hInput;
  RECT rect;
  static int sizeX, sizeY;
  int newSizeX, newSizeY;
  MINMAXINFO *mmi;
  WORD wMask;

  hText = GetDlgItem(hDlg, OPT_ConsoleText);
  hInput = GetDlgItem(hDlg, OPT_ConsoleInput);

  switch (message) {
  case WM_NOTIFY:
    if (((NMHDR*)lParam)->code == EN_LINK)
    {
      ENLINK *pLink = (ENLINK*)lParam;
      if (pLink->msg == WM_LBUTTONUP)
      {
        TEXTRANGE tr;

        tr.chrg = pLink->chrg;
        tr.lpstrText = malloc(1+tr.chrg.cpMax-tr.chrg.cpMin);
        SendMessage(hText, EM_GETTEXTRANGE, 0, (LPARAM)&tr);
        ShellExecute(NULL, "open", tr.lpstrText, NULL, NULL, SW_SHOW);
        free(tr.lpstrText);
      }
    }
    break;
  case WM_INITDIALOG: /* message: initialize dialog box */
    hwndConsole = hDlg;
    SetFocus(hInput);
    consoleTextWindowProc = (WNDPROC)
      SetWindowLongPtr(hText, GWLP_WNDPROC, (LONG_PTR) ConsoleTextSubclass);
    SendMessage(hText, EM_SETBKGNDCOLOR, FALSE, consoleBackgroundColor);
    consoleInputWindowProc = (WNDPROC)
      SetWindowLongPtr(hInput, GWLP_WNDPROC, (LONG_PTR) ConsoleInputSubclass);
    SendMessage(hInput, EM_SETBKGNDCOLOR, FALSE, consoleBackgroundColor);
    Colorize(ColorNormal, TRUE);
    SendMessage(hInput, EM_SETCHARFORMAT, SCF_ALL, (LPARAM) &consoleCF);
    ChangedConsoleFont();
    GetClientRect(hDlg, &rect);
    sizeX = rect.right;
    sizeY = rect.bottom;
    if (wpConsole.x != CW_USEDEFAULT && wpConsole.y != CW_USEDEFAULT &&
	wpConsole.width != CW_USEDEFAULT && wpConsole.height != CW_USEDEFAULT) {
      WINDOWPLACEMENT wp;
      EnsureOnScreen(&wpConsole.x, &wpConsole.y, 0, 0);
      wp.length = sizeof(WINDOWPLACEMENT);
      wp.flags = 0;
      wp.showCmd = SW_SHOW;
      wp.ptMaxPosition.x = wp.ptMaxPosition.y = 0;
      wp.rcNormalPosition.left = wpConsole.x;
      wp.rcNormalPosition.right = wpConsole.x + wpConsole.width;
      wp.rcNormalPosition.top = wpConsole.y;
      wp.rcNormalPosition.bottom = wpConsole.y + wpConsole.height;
      SetWindowPlacement(hDlg, &wp);
    }

   // [HGM] Chessknight's change 2004-07-13
   else { /* Determine Defaults */
       WINDOWPLACEMENT wp;
       wpConsole.x = wpMain.width + 1;
       wpConsole.y = wpMain.y;
       wpConsole.width = screenWidth -  wpMain.width;
       wpConsole.height = wpMain.height;
       EnsureOnScreen(&wpConsole.x, &wpConsole.y, 0, 0);
       wp.length = sizeof(WINDOWPLACEMENT);
       wp.flags = 0;
       wp.showCmd = SW_SHOW;
       wp.ptMaxPosition.x = wp.ptMaxPosition.y = 0;
       wp.rcNormalPosition.left = wpConsole.x;
       wp.rcNormalPosition.right = wpConsole.x + wpConsole.width;
       wp.rcNormalPosition.top = wpConsole.y;
       wp.rcNormalPosition.bottom = wpConsole.y + wpConsole.height;
       SetWindowPlacement(hDlg, &wp);
    }

   // Allow hText to highlight URLs and send notifications on them
   wMask = (WORD) SendMessage(hText, EM_GETEVENTMASK, 0, 0L);
   SendMessage(hText, EM_SETEVENTMASK, 0, wMask | ENM_LINK);
   SendMessage(hText, EM_AUTOURLDETECT, TRUE, 0L);
   SetWindowLongPtr(hText, GWLP_USERDATA, 79); // initialize the text window's width

    return FALSE;

  case WM_SETFOCUS:
    SetFocus(hInput);
    return 0;

  case WM_CLOSE:
    ExitEvent(0);
    /* not reached */
    break;

  case WM_SIZE:
    if (IsIconic(hDlg)) break;
    newSizeX = LOWORD(lParam);
    newSizeY = HIWORD(lParam);
    if (sizeX != newSizeX || sizeY != newSizeY) {
      RECT rectText, rectInput;
      POINT pt;
      int newTextHeight, newTextWidth;
      GetWindowRect(hText, &rectText);
      newTextWidth = rectText.right - rectText.left + newSizeX - sizeX;
      newTextHeight = rectText.bottom - rectText.top + newSizeY - sizeY;
      if (newTextHeight < 0) {
	newSizeY += -newTextHeight;
        newTextHeight = 0;
      }
      SetWindowPos(hText, NULL, 0, 0,
	newTextWidth, newTextHeight, SWP_NOZORDER|SWP_NOMOVE);
      GetWindowRect(hInput, &rectInput); /* gives screen coords */
      pt.x = rectInput.left;
      pt.y = rectInput.top + newSizeY - sizeY;
      ScreenToClient(hDlg, &pt);
      SetWindowPos(hInput, NULL, 
	pt.x, pt.y, /* needs client coords */	
	rectInput.right - rectInput.left + newSizeX - sizeX,
	rectInput.bottom - rectInput.top, SWP_NOZORDER);
    }
    sizeX = newSizeX;
    sizeY = newSizeY;
    break;

  case WM_GETMINMAXINFO:
    /* Prevent resizing window too small */
    mmi = (MINMAXINFO *) lParam;
    mmi->ptMinTrackSize.x = 100;
    mmi->ptMinTrackSize.y = 100;
    break;

  /* [AS] Snapping */
  case WM_ENTERSIZEMOVE:
    return OnEnterSizeMove( &sd, hDlg, wParam, lParam );

  case WM_SIZING:
    return OnSizing( &sd, hDlg, wParam, lParam );

  case WM_MOVING:
    return OnMoving( &sd, hDlg, wParam, lParam );

  case WM_EXITSIZEMOVE:
  	UpdateICSWidth(hText);
    return OnExitSizeMove( &sd, hDlg, wParam, lParam );
  }

  return DefWindowProc(hDlg, message, wParam, lParam);
}


VOID
ConsoleCreate()
{
  HWND hCons;
  if (hwndConsole) return;
  hCons = CreateDialog(hInst, szConsoleName, 0, NULL);
  SendMessage(hCons, WM_INITDIALOG, 0, 0);
}


VOID
ConsoleOutput(char* data, int length, int forceVisible)
{
  HWND hText;
  int trim, exlen;
  char *p, *q;
  char buf[CO_MAX+1];
  POINT pEnd;
  RECT rect;
  static int delayLF = 0;
  CHARRANGE savesel, sel;

  if (hwndConsole == NULL || length > CO_MAX-100 || length == 0) return;
  p = data;
  q = buf;
  if (delayLF) {
    *q++ = '\r';
    *q++ = '\n';
    delayLF = 0;
  }
  while (length--) {
    if (*p == '\n') {
      if (*++p) {
	*q++ = '\r';
	*q++ = '\n';
      } else {
	delayLF = 1;
      }
    } else if (*p == '\007') {
       MyPlaySound(&sounds[(int)SoundBell]);
       p++;
    } else {
      *q++ = *p++;
    }
  }
  *q = NULLCHAR;
  hText = GetDlgItem(hwndConsole, OPT_ConsoleText);
  SendMessage(hText, EM_HIDESELECTION, TRUE, FALSE);
  /* Save current selection */
  SendMessage(hText, EM_EXGETSEL, 0, (LPARAM)&savesel);
  exlen = GetWindowTextLength(hText);
  /* Find out whether current end of text is visible */
  SendMessage(hText, EM_GETRECT, 0, (LPARAM) &rect);
  SendMessage(hText, EM_POSFROMCHAR, (WPARAM) &pEnd, exlen);
  /* Trim existing text if it's too long */
  if (exlen + (q - buf) > CO_MAX) {
    trim = (CO_TRIM > (q - buf)) ? CO_TRIM : (q - buf);
    sel.cpMin = 0;
    sel.cpMax = trim;
    SendMessage(hText, EM_EXSETSEL, 0, (LPARAM)&sel);
    SendMessage(hText, EM_REPLACESEL, 0, (LPARAM)"");
    exlen -= trim;
    savesel.cpMin -= trim;
    savesel.cpMax -= trim;
    if (exlen < 0) exlen = 0;
    if (savesel.cpMin < 0) savesel.cpMin = 0;
    if (savesel.cpMax < savesel.cpMin) savesel.cpMax = savesel.cpMin;
  }
  /* Append the new text */
  sel.cpMin = exlen;
  sel.cpMax = exlen;
  SendMessage(hText, EM_EXSETSEL, 0, (LPARAM)&sel);
  SendMessage(hText, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&consoleCF);
  SendMessage(hText, EM_REPLACESEL, 0, (LPARAM) buf);
  if (forceVisible || exlen == 0 ||
      (rect.left <= pEnd.x && pEnd.x < rect.right &&
       rect.top <= pEnd.y && pEnd.y < rect.bottom)) {
    /* Scroll to make new end of text visible if old end of text
       was visible or new text is an echo of user typein */
    sel.cpMin = 9999999;
    sel.cpMax = 9999999;
    SendMessage(hText, EM_EXSETSEL, 0, (LPARAM)&sel);
    SendMessage(hText, EM_HIDESELECTION, FALSE, FALSE);
    SendMessage(hText, EM_SCROLLCARET, 0, 0);
    SendMessage(hText, EM_HIDESELECTION, TRUE, FALSE);
  }
  if (savesel.cpMax == exlen || forceVisible) {
    /* Move insert point to new end of text if it was at the old
       end of text or if the new text is an echo of user typein */
    sel.cpMin = 9999999;
    sel.cpMax = 9999999;
    SendMessage(hText, EM_EXSETSEL, 0, (LPARAM)&sel);
  } else {
    /* Restore previous selection */
    SendMessage(hText, EM_EXSETSEL, 0, (LPARAM)&savesel);
  }
  SendMessage(hText, EM_HIDESELECTION, FALSE, FALSE);
}

/*---------*/


void
DisplayHoldingsCount(HDC hdc, int x, int y, int rightAlign, int copyNumber)
{
  char buf[100];
  char *str;
  COLORREF oldFg, oldBg;
  HFONT oldFont;
  RECT rect;

  if(copyNumber > 1)
    snprintf(buf, sizeof(buf)/sizeof(buf[0]),"%d", copyNumber); else buf[0] = 0;

  oldFg = SetTextColor(hdc, RGB(255, 255, 255)); /* white */
  oldBg = SetBkColor(hdc, RGB(0, 0, 0)); /* black */
  oldFont = SelectObject(hdc, font[boardSize][CLOCK_FONT]->hf);

  rect.left = x;
  rect.right = x + squareSize;
  rect.top  = y;
  rect.bottom = y + squareSize;
  str = buf;

  ExtTextOut(hdc, x + MESSAGE_LINE_LEFTMARGIN
                    + (rightAlign ? (squareSize*2)/3 : 0),
             y, ETO_CLIPPED|ETO_OPAQUE,
             &rect, str, strlen(str), NULL);

  (void) SetTextColor(hdc, oldFg);
  (void) SetBkColor(hdc, oldBg);
  (void) SelectObject(hdc, oldFont);
}

void
DisplayAClock(HDC hdc, int timeRemaining, int highlight,
              RECT *rect, char *color, char *flagFell)
{
  char buf[100];
  char *str;
  COLORREF oldFg, oldBg;
  HFONT oldFont;

  if (twoBoards && partnerUp) return;
  if (appData.clockMode) {
    if (tinyLayout)
      snprintf(buf, sizeof(buf)/sizeof(buf[0]), "%c %s %s", color[0], TimeString(timeRemaining), flagFell);
    else
      snprintf(buf, sizeof(buf)/sizeof(buf[0]), "%s:%c%s %s", color, (logoHeight>0 ? 0 : ' '), TimeString(timeRemaining), flagFell);
    str = buf;
  } else {
    str = color;
  }

  if (highlight) {
    oldFg = SetTextColor(hdc, RGB(255, 255, 255)); /* white */
    oldBg = SetBkColor(hdc, RGB(0, 0, 0)); /* black */
  } else {
    oldFg = SetTextColor(hdc, RGB(0, 0, 0)); /* black */
    oldBg = SetBkColor(hdc, RGB(255, 255, 255)); /* white */
  }
  oldFont = SelectObject(hdc, font[boardSize][CLOCK_FONT]->hf);

  JAWS_SILENCE

  ExtTextOut(hdc, rect->left + MESSAGE_LINE_LEFTMARGIN,
	     rect->top, ETO_CLIPPED|ETO_OPAQUE,
	     rect, str, strlen(str), NULL);
  if(logoHeight > 0 && appData.clockMode) {
      RECT r;
      str += strlen(color)+2;
      r.top = rect->top + logoHeight/2;
      r.left = rect->left;
      r.right = rect->right;
      r.bottom = rect->bottom;
      ExtTextOut(hdc, rect->left + MESSAGE_LINE_LEFTMARGIN,
	         r.top, ETO_CLIPPED|ETO_OPAQUE,
	         &r, str, strlen(str), NULL);
  }
  (void) SetTextColor(hdc, oldFg);
  (void) SetBkColor(hdc, oldBg);
  (void) SelectObject(hdc, oldFont);
}


int
DoReadFile(HANDLE hFile, char *buf, int count, DWORD *outCount,
	   OVERLAPPED *ovl)
{
  int ok, err;

  /* [AS]  */
  if( count <= 0 ) {
    if (appData.debugMode) {
      fprintf( debugFP, "DoReadFile: trying to read past end of buffer, overflow = %d\n", count );
    }

    return ERROR_INVALID_USER_BUFFER;
  }

  ResetEvent(ovl->hEvent);
  ovl->Offset = ovl->OffsetHigh = 0;
  ok = ReadFile(hFile, buf, count, outCount, ovl);
  if (ok) {
    err = NO_ERROR;
  } else {
    err = GetLastError();
    if (err == ERROR_IO_PENDING) {
      ok = GetOverlappedResult(hFile, ovl, outCount, TRUE);
      if (ok)
	err = NO_ERROR;
      else
	err = GetLastError();
    }
  }
  return err;
}

int
DoWriteFile(HANDLE hFile, char *buf, int count, DWORD *outCount,
	    OVERLAPPED *ovl)
{
  int ok, err;

  ResetEvent(ovl->hEvent);
  ovl->Offset = ovl->OffsetHigh = 0;
  ok = WriteFile(hFile, buf, count, outCount, ovl);
  if (ok) {
    err = NO_ERROR;
  } else {
    err = GetLastError();
    if (err == ERROR_IO_PENDING) {
      ok = GetOverlappedResult(hFile, ovl, outCount, TRUE);
      if (ok)
	err = NO_ERROR;
      else
	err = GetLastError();
    }

  }
  return err;
}

/* [AS] If input is line by line and a line exceed the buffer size, force an error */
void CheckForInputBufferFull( InputSource * is )
{
    if( is->lineByLine && (is->next - is->buf) >= INPUT_SOURCE_BUF_SIZE ) {
        /* Look for end of line */
        char * p = is->buf;
        
        while( p < is->next && *p != '\n' ) {
            p++;
        }

        if( p >= is->next ) {
            if (appData.debugMode) {
                fprintf( debugFP, "Input line exceeded buffer size (source id=%lu)\n", is->id );
            }

            is->error = ERROR_BROKEN_PIPE; /* [AS] Just any non-successful code! */
            is->count = (DWORD) -1;
            is->next = is->buf;
        }
    }
}

DWORD
InputThread(LPVOID arg)
{
  InputSource *is;
  OVERLAPPED ovl;

  is = (InputSource *) arg;
  ovl.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
  ovl.Internal = ovl.InternalHigh = ovl.Offset = ovl.OffsetHigh = 0;
  while (is->hThread != NULL) {
    is->error = DoReadFile(is->hFile, is->next,
			   INPUT_SOURCE_BUF_SIZE - (is->next - is->buf),
			   &is->count, &ovl);
    if (is->error == NO_ERROR) {
      is->next += is->count;
    } else {
      if (is->error == ERROR_BROKEN_PIPE) {
	/* Correct for MS brain damage.  EOF reading a pipe is not an error. */
	is->count = 0;
      } else {
	is->count = (DWORD) -1;
        /* [AS] The (is->count <= 0) check below is not useful for unsigned values! */
        break; 
      }
    }

    CheckForInputBufferFull( is );

    SendMessage(hwndMain, WM_USER_Input, 0, (LPARAM) is);

    if( is->count == ((DWORD) -1) ) break; /* [AS] */

    if (is->count <= 0) break;  /* Quit on EOF or error */
  }

  CloseHandle(ovl.hEvent);
  CloseHandle(is->hFile);

  if (appData.debugMode) {
    fprintf( debugFP, "Input thread terminated (id=%lu, error=%d, count=%ld)\n", is->id, is->error, is->count );
  }

  return 0;
}


/* Windows 95 beta 2 won't let you do overlapped i/o on a console or pipe */
DWORD
NonOvlInputThread(LPVOID arg)
{
  InputSource *is;
  char *p, *q;
  int i;
  char prev;

  is = (InputSource *) arg;
  while (is->hThread != NULL) {
    is->error = ReadFile(is->hFile, is->next,
			 INPUT_SOURCE_BUF_SIZE - (is->next - is->buf),
			 &is->count, NULL) ? NO_ERROR : GetLastError();
    if (is->error == NO_ERROR) {
      /* Change CRLF to LF */
      if (is->next > is->buf) {
	p = is->next - 1;
	i = is->count + 1;
      } else {
	p = is->next;
	i = is->count;
      }
      q = p;
      prev = NULLCHAR;
      while (i > 0) {
	if (prev == '\r' && *p == '\n') {
	  *(q-1) = '\n';
	  is->count--;
	} else { 
	  *q++ = *p;
	}
	prev = *p++;
	i--;
      }
      *q = NULLCHAR;
      is->next = q;
    } else {
      if (is->error == ERROR_BROKEN_PIPE) {
	/* Correct for MS brain damage.  EOF reading a pipe is not an error. */
	is->count = 0; 
      } else {
	is->count = (DWORD) -1;
      }
    }

    CheckForInputBufferFull( is );

    SendMessage(hwndMain, WM_USER_Input, 0, (LPARAM) is);

    if( is->count == ((DWORD) -1) ) break; /* [AS] */

    if (is->count < 0) break;  /* Quit on error */
  }
  CloseHandle(is->hFile);
  return 0;
}

DWORD
SocketInputThread(LPVOID arg)
{
  InputSource *is;

  is = (InputSource *) arg;
  while (is->hThread != NULL) {
    is->count = recv(is->sock, is->buf, INPUT_SOURCE_BUF_SIZE, 0);
    if ((int)is->count == SOCKET_ERROR) {
      is->count = (DWORD) -1;
      is->error = WSAGetLastError();
    } else {
      is->error = NO_ERROR;
      is->next += is->count;
      if (is->count == 0 && is->second == is) {
	/* End of file on stderr; quit with no message */
	break;
      }
    }
    SendMessage(hwndMain, WM_USER_Input, 0, (LPARAM) is);

    if( is->count == ((DWORD) -1) ) break; /* [AS] */

    if (is->count <= 0) break;  /* Quit on EOF or error */
  }
  return 0;
}

VOID
InputEvent(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  InputSource *is;

  is = (InputSource *) lParam;
  if (is->lineByLine) {
    /* Feed in lines one by one */
    char *p = is->buf;
    char *q = p;
    while (q < is->next) {
      if (*q++ == '\n') {
	(is->func)(is, is->closure, p, q - p, NO_ERROR);
	p = q;
      }
    }
    
    /* Move any partial line to the start of the buffer */
    q = is->buf;
    while (p < is->next) {
      *q++ = *p++;
    }
    is->next = q;

    if (is->error != NO_ERROR || is->count == 0) {
      /* Notify backend of the error.  Note: If there was a partial
	 line at the end, it is not flushed through. */
      (is->func)(is, is->closure, is->buf, is->count, is->error);   
    }
  } else {
    /* Feed in the whole chunk of input at once */
    (is->func)(is, is->closure, is->buf, is->count, is->error);
    is->next = is->buf;
  }
}

/*---------------------------------------------------------------------------*\
 *
 *  Menu enables. Used when setting various modes.
 *
\*---------------------------------------------------------------------------*/

typedef struct {
  int item;
  int flags;
} Enables;

VOID
GreyRevert(Boolean grey)
{ // [HGM] vari: for retracting variations in local mode
  HMENU hmenu = GetMenu(hwndMain);
  EnableMenuItem(hmenu, IDM_Revert, MF_BYCOMMAND|(grey ? MF_GRAYED : MF_ENABLED));
  EnableMenuItem(hmenu, IDM_Annotate, MF_BYCOMMAND|(grey ? MF_GRAYED : MF_ENABLED));
}

VOID
SetMenuEnables(HMENU hmenu, Enables *enab)
{
  while (enab->item > 0) {
    (void) EnableMenuItem(hmenu, enab->item, enab->flags);
    enab++;
  }
}

Enables gnuEnables[] = {
  { IDM_MailMove, MF_BYCOMMAND|MF_GRAYED },
  { IDM_ReloadCMailMsg, MF_BYCOMMAND|MF_GRAYED },
  { IDM_IcsClient, MF_BYCOMMAND|MF_GRAYED },
  { IDM_Accept, MF_BYCOMMAND|MF_GRAYED },
  { IDM_Decline, MF_BYCOMMAND|MF_GRAYED },
  { IDM_Rematch, MF_BYCOMMAND|MF_GRAYED },
  { IDM_Adjourn, MF_BYCOMMAND|MF_GRAYED },
  { IDM_StopExamining, MF_BYCOMMAND|MF_GRAYED },
  { IDM_StopObserving, MF_BYCOMMAND|MF_GRAYED },
  { IDM_Upload, MF_BYCOMMAND|MF_GRAYED },
  { IDM_Revert, MF_BYCOMMAND|MF_GRAYED },
  { IDM_Annotate, MF_BYCOMMAND|MF_GRAYED },
  { IDM_NewChat, MF_BYCOMMAND|MF_GRAYED },

  // Needed to switch from ncp to GNU mode on Engine Load
  { ACTION_POS, MF_BYPOSITION|MF_ENABLED },
  { IDM_MachineWhite, MF_BYCOMMAND|MF_ENABLED },
  { IDM_MachineBlack, MF_BYCOMMAND|MF_ENABLED },
  { IDM_TwoMachines, MF_BYCOMMAND|MF_ENABLED },
  { IDM_Match, MF_BYCOMMAND|MF_ENABLED },
  { IDM_AnalysisMode, MF_BYCOMMAND|MF_ENABLED },
  { IDM_AnalyzeFile, MF_BYCOMMAND|MF_ENABLED },
  { IDM_Engine1Options, MF_BYCOMMAND|MF_ENABLED },
  { IDM_Engine2Options, MF_BYCOMMAND|MF_ENABLED },
  { IDM_TimeControl, MF_BYCOMMAND|MF_ENABLED },
  { IDM_RetractMove, MF_BYCOMMAND|MF_ENABLED },
  { IDM_MoveNow, MF_BYCOMMAND|MF_ENABLED },
  { IDM_Hint, MF_BYCOMMAND|MF_ENABLED },
  { IDM_Book, MF_BYCOMMAND|MF_ENABLED },
  { -1, -1 }
};

Enables icsEnables[] = {
  { IDM_MailMove, MF_BYCOMMAND|MF_GRAYED },
  { IDM_ReloadCMailMsg, MF_BYCOMMAND|MF_GRAYED },
  { IDM_MachineWhite, MF_BYCOMMAND|MF_GRAYED },
  { IDM_MachineBlack, MF_BYCOMMAND|MF_GRAYED },
  { IDM_TwoMachines, MF_BYCOMMAND|MF_GRAYED },
  { IDM_Match, MF_BYCOMMAND|MF_GRAYED },
  { IDM_MachineBoth, MF_BYCOMMAND|MF_GRAYED },
  { IDM_AnalysisMode, MF_BYCOMMAND|MF_ENABLED },
  { IDM_AnalyzeFile, MF_BYCOMMAND|MF_GRAYED },
  { IDM_TimeControl, MF_BYCOMMAND|MF_GRAYED },
  { IDM_MoveNow, MF_BYCOMMAND|MF_GRAYED },
  { IDM_Hint, MF_BYCOMMAND|MF_GRAYED },
  { IDM_Book, MF_BYCOMMAND|MF_GRAYED },
  { IDM_LoadProg1, MF_BYCOMMAND|MF_GRAYED },
  { IDM_LoadProg2, MF_BYCOMMAND|MF_GRAYED },
  { IDM_IcsOptions, MF_BYCOMMAND|MF_ENABLED },
  { IDM_Engine1Options, MF_BYCOMMAND|MF_GRAYED },
  { IDM_Engine2Options, MF_BYCOMMAND|MF_GRAYED },
  { IDM_Annotate, MF_BYCOMMAND|MF_GRAYED },
  { IDM_Tourney, MF_BYCOMMAND|MF_GRAYED },
  { -1, -1 }
};

#if ZIPPY
Enables zippyEnables[] = {
  { IDM_MoveNow, MF_BYCOMMAND|MF_ENABLED },
  { IDM_Hint, MF_BYCOMMAND|MF_ENABLED },
  { IDM_Book, MF_BYCOMMAND|MF_ENABLED },
  { IDM_Engine1Options, MF_BYCOMMAND|MF_ENABLED },
  { -1, -1 }
};
#endif

Enables ncpEnables[] = {
  { IDM_MailMove, MF_BYCOMMAND|MF_GRAYED },
  { IDM_ReloadCMailMsg, MF_BYCOMMAND|MF_GRAYED },
  { IDM_MachineWhite, MF_BYCOMMAND|MF_GRAYED },
  { IDM_MachineBlack, MF_BYCOMMAND|MF_GRAYED },
  { IDM_TwoMachines, MF_BYCOMMAND|MF_GRAYED },
  { IDM_Match, MF_BYCOMMAND|MF_GRAYED },
  { IDM_AnalysisMode, MF_BYCOMMAND|MF_GRAYED },
  { IDM_AnalyzeFile, MF_BYCOMMAND|MF_GRAYED },
  { IDM_IcsClient, MF_BYCOMMAND|MF_GRAYED },
  { ACTION_POS, MF_BYPOSITION|MF_GRAYED },
  { IDM_Revert, MF_BYCOMMAND|MF_GRAYED },
  { IDM_Annotate, MF_BYCOMMAND|MF_GRAYED },
  { IDM_MoveNow, MF_BYCOMMAND|MF_GRAYED },
  { IDM_RetractMove, MF_BYCOMMAND|MF_GRAYED },
  { IDM_TimeControl, MF_BYCOMMAND|MF_GRAYED },
  { IDM_Hint, MF_BYCOMMAND|MF_GRAYED },
  { IDM_Book, MF_BYCOMMAND|MF_GRAYED },
  { IDM_MachineBoth, MF_BYCOMMAND|MF_GRAYED },
  { IDM_NewChat, MF_BYCOMMAND|MF_GRAYED },
  { IDM_Engine1Options, MF_BYCOMMAND|MF_GRAYED },
  { IDM_Engine2Options, MF_BYCOMMAND|MF_GRAYED },
  { IDM_Sounds, MF_BYCOMMAND|MF_GRAYED },
  { -1, -1 }
};

Enables trainingOnEnables[] = {
  { IDM_EditComment, MF_BYCOMMAND|MF_GRAYED },
  { IDM_Comment, MF_BYCOMMAND|MF_GRAYED },
  { IDM_Pause, MF_BYCOMMAND|MF_GRAYED },
  { IDM_Forward, MF_BYCOMMAND|MF_GRAYED },
  { IDM_Backward, MF_BYCOMMAND|MF_GRAYED },
  { IDM_ToEnd, MF_BYCOMMAND|MF_GRAYED },
  { IDM_ToStart, MF_BYCOMMAND|MF_GRAYED },
  { IDM_MoveNow, MF_BYCOMMAND|MF_GRAYED },
  { IDM_TruncateGame, MF_BYCOMMAND|MF_GRAYED },
  { -1, -1 }
};

Enables trainingOffEnables[] = {
  { IDM_EditComment, MF_BYCOMMAND|MF_ENABLED },
  { IDM_Comment, MF_BYCOMMAND|MF_ENABLED },
  { IDM_Pause, MF_BYCOMMAND|MF_ENABLED },
  { IDM_Forward, MF_BYCOMMAND|MF_ENABLED },
  { IDM_Backward, MF_BYCOMMAND|MF_ENABLED },
  { IDM_ToEnd, MF_BYCOMMAND|MF_ENABLED },
  { IDM_ToStart, MF_BYCOMMAND|MF_ENABLED },
  { IDM_MoveNow, MF_BYCOMMAND|MF_ENABLED },
  { IDM_TruncateGame, MF_BYCOMMAND|MF_ENABLED },
  { -1, -1 }
};

/* These modify either ncpEnables or gnuEnables */
Enables cmailEnables[] = {
  { IDM_MailMove, MF_BYCOMMAND|MF_ENABLED },
  { IDM_ReloadCMailMsg, MF_BYCOMMAND|MF_ENABLED },
  { ACTION_POS, MF_BYPOSITION|MF_ENABLED },
  { IDM_CallFlag, MF_BYCOMMAND|MF_GRAYED },
  { IDM_Draw, MF_BYCOMMAND|MF_ENABLED },
  { IDM_Adjourn, MF_BYCOMMAND|MF_GRAYED },
  { IDM_Abort, MF_BYCOMMAND|MF_GRAYED },
  { -1, -1 }
};

Enables machineThinkingEnables[] = {
  { IDM_LoadGame, MF_BYCOMMAND|MF_GRAYED },
  { IDM_LoadNextGame, MF_BYCOMMAND|MF_GRAYED },
  { IDM_LoadPrevGame, MF_BYCOMMAND|MF_GRAYED },
  { IDM_ReloadGame, MF_BYCOMMAND|MF_GRAYED },
  { IDM_PasteGame, MF_BYCOMMAND|MF_GRAYED },
  { IDM_LoadPosition, MF_BYCOMMAND|MF_GRAYED },
  { IDM_LoadNextPosition, MF_BYCOMMAND|MF_GRAYED },
  { IDM_LoadPrevPosition, MF_BYCOMMAND|MF_GRAYED },
  { IDM_ReloadPosition, MF_BYCOMMAND|MF_GRAYED },
  { IDM_PastePosition, MF_BYCOMMAND|MF_GRAYED },
  { IDM_MachineWhite, MF_BYCOMMAND|MF_GRAYED },
  { IDM_MachineBlack, MF_BYCOMMAND|MF_GRAYED },
  { IDM_TwoMachines, MF_BYCOMMAND|MF_GRAYED },
//  { IDM_Match, MF_BYCOMMAND|MF_GRAYED },
  { IDM_TypeInMove, MF_BYCOMMAND|MF_GRAYED },
  { IDM_RetractMove, MF_BYCOMMAND|MF_GRAYED },
  { -1, -1 }
};

Enables userThinkingEnables[] = {
  { IDM_LoadGame, MF_BYCOMMAND|MF_ENABLED },
  { IDM_LoadNextGame, MF_BYCOMMAND|MF_ENABLED },
  { IDM_LoadPrevGame, MF_BYCOMMAND|MF_ENABLED },
  { IDM_ReloadGame, MF_BYCOMMAND|MF_ENABLED },
  { IDM_PasteGame, MF_BYCOMMAND|MF_ENABLED },
  { IDM_LoadPosition, MF_BYCOMMAND|MF_ENABLED },
  { IDM_LoadNextPosition, MF_BYCOMMAND|MF_ENABLED },
  { IDM_LoadPrevPosition, MF_BYCOMMAND|MF_ENABLED },
  { IDM_ReloadPosition, MF_BYCOMMAND|MF_ENABLED },
  { IDM_PastePosition, MF_BYCOMMAND|MF_ENABLED },
  { IDM_MachineWhite, MF_BYCOMMAND|MF_ENABLED },
  { IDM_MachineBlack, MF_BYCOMMAND|MF_ENABLED },
  { IDM_TwoMachines, MF_BYCOMMAND|MF_ENABLED },
//  { IDM_Match, MF_BYCOMMAND|MF_ENABLED },
  { IDM_TypeInMove, MF_BYCOMMAND|MF_ENABLED },
  { IDM_RetractMove, MF_BYCOMMAND|MF_ENABLED },
  { -1, -1 }
};

/*---------------------------------------------------------------------------*\
 *
 *  Front-end interface functions exported by XBoard.
 *  Functions appear in same order as prototypes in frontend.h.
 * 
\*---------------------------------------------------------------------------*/
VOID
CheckMark(UINT item, int state)
{
    if(item) CheckMenuItem(GetMenu(hwndMain), item, MF_BYCOMMAND|state);
}

VOID
ModeHighlight()
{
  static UINT prevChecked = 0;
  static int prevPausing = 0;
  UINT nowChecked;

  if (pausing != prevPausing) {
    prevPausing = pausing;
    (void) CheckMenuItem(GetMenu(hwndMain), IDM_Pause,
			 MF_BYCOMMAND|(pausing ? MF_CHECKED : MF_UNCHECKED));
    if (hwndPause) SetWindowText(hwndPause, pausing ? "C" : "P");
  }

  switch (gameMode) {
  case BeginningOfGame:
    if (appData.icsActive)
      nowChecked = IDM_IcsClient;
    else if (appData.noChessProgram)
      nowChecked = IDM_EditGame;
    else
      nowChecked = IDM_MachineBlack;
    break;
  case MachinePlaysBlack:
    nowChecked = IDM_MachineBlack;
    break;
  case MachinePlaysWhite:
    nowChecked = IDM_MachineWhite;
    break;
  case TwoMachinesPlay:
    nowChecked = IDM_TwoMachines;
    break;
  case AnalyzeMode:
    nowChecked = IDM_AnalysisMode;
    break;
  case AnalyzeFile:
    nowChecked = IDM_AnalyzeFile;
    break;
  case EditGame:
    nowChecked = IDM_EditGame;
    break;
  case PlayFromGameFile:
    nowChecked = IDM_LoadGame;
    break;
  case EditPosition:
    nowChecked = IDM_EditPosition;
    break;
  case Training:
    nowChecked = IDM_Training;
    break;
  case IcsPlayingWhite:
  case IcsPlayingBlack:
  case IcsObserving:
  case IcsIdle:
    nowChecked = IDM_IcsClient;
    break;
  default:
  case EndOfGame:
    nowChecked = 0;
    break;
  }
  CheckMark(prevChecked, MF_UNCHECKED);
  CheckMark(nowChecked, MF_CHECKED);
  CheckMark(IDM_Match, matchMode && matchGame < appData.matchGames ? MF_CHECKED : MF_UNCHECKED);

  if (nowChecked == IDM_LoadGame || nowChecked == IDM_Training) {
    (void) EnableMenuItem(GetMenu(hwndMain), IDM_Training, 
			  MF_BYCOMMAND|MF_ENABLED);
  } else {
    (void) EnableMenuItem(GetMenu(hwndMain), 
			  IDM_Training, MF_BYCOMMAND|MF_GRAYED);
  }

  prevChecked = nowChecked;

  /* [DM] icsEngineAnalyze - Do a sceure check too */
  if (appData.icsActive) {
       if (appData.icsEngineAnalyze) {
               CheckMark(IDM_AnalysisMode, MF_CHECKED);
       } else {
               CheckMark(IDM_AnalysisMode, MF_UNCHECKED);
       }
  }
  DisplayLogos(); // [HGM] logos: mode change could have altered logos
}

VOID
SetICSMode()
{
  HMENU hmenu = GetMenu(hwndMain);
  SetMenuEnables(hmenu, icsEnables);
  EnableMenuItem(GetSubMenu(hmenu, OPTIONS_POS), IDM_IcsOptions,
    MF_BYCOMMAND|MF_ENABLED);
#if ZIPPY
  if (appData.zippyPlay) {
    SetMenuEnables(hmenu, zippyEnables);
    if (!appData.noChessProgram)     /* [DM] icsEngineAnalyze */
         (void) EnableMenuItem(GetMenu(hwndMain), IDM_AnalysisMode,
          MF_BYCOMMAND|MF_ENABLED);
  }
#endif
}

VOID
SetGNUMode()
{
  SetMenuEnables(GetMenu(hwndMain), gnuEnables);
}

VOID
SetNCPMode()
{
  HMENU hmenu = GetMenu(hwndMain);
  SetMenuEnables(hmenu, ncpEnables);
    DrawMenuBar(hwndMain);
}

VOID
SetCmailMode()
{
  SetMenuEnables(GetMenu(hwndMain), cmailEnables);
}

VOID 
SetTrainingModeOn()
{
  int i;
  SetMenuEnables(GetMenu(hwndMain), trainingOnEnables);
  for (i = 0; i < N_BUTTONS; i++) {
    if (buttonDesc[i].hwnd != NULL)
      EnableWindow(buttonDesc[i].hwnd, FALSE);
  }
  CommentPopDown();
}

VOID SetTrainingModeOff()
{
  int i;
  SetMenuEnables(GetMenu(hwndMain), trainingOffEnables);
  for (i = 0; i < N_BUTTONS; i++) {
    if (buttonDesc[i].hwnd != NULL)
      EnableWindow(buttonDesc[i].hwnd, TRUE);
  }
}


VOID
SetUserThinkingEnables()
{
  SetMenuEnables(GetMenu(hwndMain), userThinkingEnables);
}

VOID
SetMachineThinkingEnables()
{
  HMENU hMenu = GetMenu(hwndMain);
  int flags = MF_BYCOMMAND|MF_ENABLED;

  SetMenuEnables(hMenu, machineThinkingEnables);

  if (gameMode == MachinePlaysBlack) {
    (void)EnableMenuItem(hMenu, IDM_MachineBlack, flags);
  } else if (gameMode == MachinePlaysWhite) {
    (void)EnableMenuItem(hMenu, IDM_MachineWhite, flags);
  } else if (gameMode == TwoMachinesPlay) {
    (void)EnableMenuItem(hMenu, matchMode ? IDM_Match : IDM_TwoMachines, flags); // [HGM] match
  }
}


VOID
DisplayTitle(char *str)
{
  char title[MSG_SIZ], *host;
  if (str[0] != NULLCHAR) {
    safeStrCpy(title, str, sizeof(title)/sizeof(title[0]) );
  } else if (appData.icsActive) {
    if (appData.icsCommPort[0] != NULLCHAR)
      host = "ICS";
    else 
      host = appData.icsHost;
      snprintf(title, MSG_SIZ, "%s: %s", szTitle, host);
  } else if (appData.noChessProgram) {
    safeStrCpy(title, szTitle, sizeof(title)/sizeof(title[0]) );
  } else {
    safeStrCpy(title, szTitle, sizeof(title)/sizeof(title[0]) );
    strcat(title, ": ");
    strcat(title, first.tidy);
  }
  SetWindowText(hwndMain, title);
}


VOID
DisplayMessage(char *str1, char *str2)
{
  HDC hdc;
  HFONT oldFont;
  int remain = MESSAGE_TEXT_MAX - 1;
  int len;

  moveErrorMessageUp = FALSE; /* turned on later by caller if needed */
  messageText[0] = NULLCHAR;
  if (*str1) {
    len = strlen(str1);
    if (len > remain) len = remain;
    strncpy(messageText, str1, len);
    messageText[len] = NULLCHAR;
    remain -= len;
  }
  if (*str2 && remain >= 2) {
    if (*str1) {
      strcat(messageText, "  ");
      remain -= 2;
    }
    len = strlen(str2);
    if (len > remain) len = remain;
    strncat(messageText, str2, len);
  }
  messageText[MESSAGE_TEXT_MAX - 1] = NULLCHAR;
  safeStrCpy(lastMsg, messageText, MSG_SIZ);

  if (hwndMain == NULL || IsIconic(hwndMain)) return;

  SAYMACHINEMOVE();

  hdc = GetDC(hwndMain);
  oldFont = SelectObject(hdc, font[boardSize][MESSAGE_FONT]->hf);
  ExtTextOut(hdc, messageRect.left, messageRect.top, ETO_CLIPPED|ETO_OPAQUE,
	     &messageRect, messageText, strlen(messageText), NULL);
  (void) SelectObject(hdc, oldFont);
  (void) ReleaseDC(hwndMain, hdc);
}

VOID
DisplayError(char *str, int error)
{
  char buf[MSG_SIZ*2], buf2[MSG_SIZ];
  int len;

  if (error == 0) {
    safeStrCpy(buf, str, sizeof(buf)/sizeof(buf[0]) );
  } else {
    len = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
			NULL, error, LANG_NEUTRAL,
			(LPSTR) buf2, MSG_SIZ, NULL);
    if (len > 0) {
      snprintf(buf, 2*MSG_SIZ, "%s:\n%s", str, buf2);
    } else {
      ErrorMap *em = errmap;
      while (em->err != 0 && em->err != error) em++;
      if (em->err != 0) {
	snprintf(buf, 2*MSG_SIZ, "%s:\n%s", str, em->msg);
      } else {
	snprintf(buf, 2*MSG_SIZ, "%s:\nError code %d", str, error);
      }
    }
  }
  
  ErrorPopUp(_("Error"), buf);
}


VOID
DisplayMoveError(char *str)
{
  fromX = fromY = -1;
  ClearHighlights();
  DrawPosition(FALSE, NULL);
  if (appData.popupMoveErrors) {
    ErrorPopUp(_("Error"), str);
  } else {
    DisplayMessage(str, "");
    moveErrorMessageUp = TRUE;
  }
}

VOID
DisplayFatalError(char *str, int error, int exitStatus)
{
  char buf[2*MSG_SIZ], buf2[MSG_SIZ];
  int len;
  char *label = exitStatus ? _("Fatal Error") : _("Exiting");

  if (error != 0) {
    len = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
			NULL, error, LANG_NEUTRAL,
			(LPSTR) buf2, MSG_SIZ, NULL);
    if (len > 0) {
      snprintf(buf, 2*MSG_SIZ, "%s:\n%s", str, buf2);
    } else {
      ErrorMap *em = errmap;
      while (em->err != 0 && em->err != error) em++;
      if (em->err != 0) {
	snprintf(buf, 2*MSG_SIZ, "%s:\n%s", str, em->msg);
      } else {
	snprintf(buf, 2*MSG_SIZ, "%s:\nError code %d", str, error);
      }
    }
    str = buf;
  }
  if (appData.debugMode) {
    fprintf(debugFP, "%s: %s\n", label, str);
  }
  if (appData.popupExitMessage) {
    (void) MessageBox(hwndMain, str, label, MB_OK|
		      (exitStatus ? MB_ICONSTOP : MB_ICONINFORMATION));
  }
  ExitEvent(exitStatus);
}


VOID
DisplayInformation(char *str)
{
  (void) MessageBox(hwndMain, str, _("Information"), MB_OK|MB_ICONINFORMATION);
}


VOID
DisplayNote(char *str)
{
  ErrorPopUp(_("Note"), str);
}


typedef struct {
  char *title, *question, *replyPrefix;
  ProcRef pr;
} QuestionParams;

LRESULT CALLBACK
QuestionDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  static QuestionParams *qp;
  char reply[MSG_SIZ];
  int len, err;

  switch (message) {
  case WM_INITDIALOG:
    qp = (QuestionParams *) lParam;
    CenterWindow(hDlg, GetWindow(hDlg, GW_OWNER));
    Translate(hDlg, DLG_Question);
    SetWindowText(hDlg, qp->title);
    SetDlgItemText(hDlg, OPT_QuestionText, qp->question);
    SetFocus(GetDlgItem(hDlg, OPT_QuestionInput));
    return FALSE;

  case WM_COMMAND:
    switch (LOWORD(wParam)) {
    case IDOK:
      safeStrCpy(reply, qp->replyPrefix, sizeof(reply)/sizeof(reply[0]) );
      if (*reply) strcat(reply, " ");
      len = strlen(reply);
      GetDlgItemText(hDlg, OPT_QuestionInput, reply + len, sizeof(reply) - len);
      strcat(reply, "\n");
      OutputToProcess(qp->pr, reply, strlen(reply), &err);
      EndDialog(hDlg, TRUE);
      if (err) DisplayFatalError(_("Error writing to chess program"), err, 1);
      return TRUE;
    case IDCANCEL:
      EndDialog(hDlg, FALSE);
      return TRUE;
    default:
      break;
    }
    break;
  }
  return FALSE;
}

VOID
AskQuestion(char* title, char *question, char *replyPrefix, ProcRef pr)
{
    QuestionParams qp;
    FARPROC lpProc;
    
    qp.title = title;
    qp.question = question;
    qp.replyPrefix = replyPrefix;
    qp.pr = pr;
    lpProc = MakeProcInstance((FARPROC)QuestionDialog, hInst);
    DialogBoxParam(hInst, MAKEINTRESOURCE(DLG_Question),
      hwndMain, (DLGPROC)lpProc, (LPARAM)&qp);
    FreeProcInstance(lpProc);
}

/* [AS] Pick FRC position */
LRESULT CALLBACK NewGameFRC_Proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static int * lpIndexFRC;
    BOOL index_is_ok;
    char buf[16];

    switch( message )
    {
    case WM_INITDIALOG:
        lpIndexFRC = (int *) lParam;

        CenterWindow(hDlg, GetWindow(hDlg, GW_OWNER));
        Translate(hDlg, DLG_NewGameFRC);

        SendDlgItemMessage( hDlg, IDC_NFG_Edit, EM_SETLIMITTEXT, sizeof(buf)-1, 0 );
        SetDlgItemInt( hDlg, IDC_NFG_Edit, *lpIndexFRC, TRUE );
        SendDlgItemMessage( hDlg, IDC_NFG_Edit, EM_SETSEL, 0, -1 );
        SetFocus(GetDlgItem(hDlg, IDC_NFG_Edit));

        break;

    case WM_COMMAND:
        switch( LOWORD(wParam) ) {
        case IDOK:
            *lpIndexFRC = GetDlgItemInt(hDlg, IDC_NFG_Edit, &index_is_ok, TRUE );
            EndDialog( hDlg, 0 );
	    shuffleOpenings = TRUE; /* [HGM] shuffle: switch shuffling on for as long as we stay in current variant */
            return TRUE;
        case IDCANCEL:
            EndDialog( hDlg, 1 );   
            return TRUE;
        case IDC_NFG_Edit:
            if( HIWORD(wParam) == EN_CHANGE ) {
                GetDlgItemInt(hDlg, IDC_NFG_Edit, &index_is_ok, TRUE );

                EnableWindow( GetDlgItem(hDlg, IDOK), index_is_ok );
            }
            return TRUE;
        case IDC_NFG_Random:
	  snprintf( buf, sizeof(buf)/sizeof(buf[0]), "%d", myrandom() ); /* [HGM] shuffle: no longer limit to 960 */
            SetDlgItemText(hDlg, IDC_NFG_Edit, buf );
            return TRUE;
        }

        break;
    }

    return FALSE;
}

int NewGameFRC()
{
    int result;
    int index = appData.defaultFrcPosition;
    FARPROC lpProc = MakeProcInstance( (FARPROC) NewGameFRC_Proc, hInst );

    result = DialogBoxParam( hInst, MAKEINTRESOURCE(DLG_NewGameFRC), hwndMain, (DLGPROC)lpProc, (LPARAM)&index );

    if( result == 0 ) {
        appData.defaultFrcPosition = index;
    }

    return result;
}

/* [AS] Game list options. Refactored by HGM */

HWND gameListOptionsDialog;

// low-level front-end: clear text edit / list widget
void

GLT_ClearList()
{
    SendDlgItemMessage( gameListOptionsDialog, IDC_GameListTags, LB_RESETCONTENT, 0, 0 );
}

// low-level front-end: clear text edit / list widget
void
GLT_DeSelectList()
{
    SendDlgItemMessage( gameListOptionsDialog, IDC_GameListTags, LB_SETCURSEL, 0, 0 );
}

// low-level front-end: append line to text edit / list widget
void
GLT_AddToList( char *name )
{
    if( name != 0 ) {
            SendDlgItemMessage( gameListOptionsDialog, IDC_GameListTags, LB_ADDSTRING, 0, (LPARAM) name );
    }
}

// low-level front-end: get line from text edit / list widget
Boolean
GLT_GetFromList( int index, char *name )
{
    if( name != 0 ) {
	    if( SendDlgItemMessage( gameListOptionsDialog, IDC_GameListTags, LB_GETTEXT, index, (LPARAM) name ) != LB_ERR )
		return TRUE;
    }
    return FALSE;
}

void GLT_MoveSelection( HWND hDlg, int delta )
{
    int idx1 = (int) SendDlgItemMessage( hDlg, IDC_GameListTags, LB_GETCURSEL, 0, 0 );
    int idx2 = idx1 + delta;
    int count = (int) SendDlgItemMessage( hDlg, IDC_GameListTags, LB_GETCOUNT, 0, 0 );

    if( idx1 >=0 && idx1 < count && idx2 >= 0 && idx2 < count ) {
        char buf[128];

        SendDlgItemMessage( hDlg, IDC_GameListTags, LB_GETTEXT, idx1, (LPARAM) buf );
        SendDlgItemMessage( hDlg, IDC_GameListTags, LB_DELETESTRING, idx1, 0 );
        SendDlgItemMessage( hDlg, IDC_GameListTags, LB_INSERTSTRING, idx2, (LPARAM) buf );
        SendDlgItemMessage( hDlg, IDC_GameListTags, LB_SETCURSEL, idx2, 0 );
    }
}

LRESULT CALLBACK GameListOptions_Proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch( message )
    {
    case WM_INITDIALOG:
	gameListOptionsDialog = hDlg; // [HGM] pass through global to keep out off back-end
        
        CenterWindow(hDlg, GetWindow(hDlg, GW_OWNER));
        Translate(hDlg, DLG_GameListOptions);

        /* Initialize list */
        GLT_TagsToList( lpUserGLT );

        SetFocus( GetDlgItem(hDlg, IDC_GameListTags) );

        break;

    case WM_COMMAND:
        switch( LOWORD(wParam) ) {
        case IDOK:
	    GLT_ParseList();
            EndDialog( hDlg, 0 );
            return TRUE;
        case IDCANCEL:
            EndDialog( hDlg, 1 );
            return TRUE;

        case IDC_GLT_Default:
            GLT_TagsToList( GLT_DEFAULT_TAGS );
            return TRUE;

        case IDC_GLT_Restore:
            GLT_TagsToList( appData.gameListTags );
            return TRUE;

        case IDC_GLT_Up:
            GLT_MoveSelection( hDlg, -1 );
            return TRUE;

        case IDC_GLT_Down:
            GLT_MoveSelection( hDlg, +1 );
            return TRUE;
        }

        break;
    }

    return FALSE;
}

int GameListOptions()
{
    int result;
    FARPROC lpProc = MakeProcInstance( (FARPROC) GameListOptions_Proc, hInst );

      safeStrCpy( lpUserGLT, appData.gameListTags ,LPUSERGLT_SIZE ); 

    result = DialogBoxParam( hInst, MAKEINTRESOURCE(DLG_GameListOptions), hwndMain, (DLGPROC)lpProc, (LPARAM)lpUserGLT );

    if( result == 0 ) {
        char *oldTags = appData.gameListTags;
        /* [AS] Memory leak here! */
        appData.gameListTags = strdup( lpUserGLT ); 
        if(strcmp(oldTags, appData.gameListTags)) // [HGM] redo Game List when we changed something
            GameListToListBox(NULL, TRUE, ".", NULL, FALSE, FALSE); // "." as filter is kludge to select all
    }

    return result;
}

VOID
DisplayIcsInteractionTitle(char *str)
{
  char consoleTitle[MSG_SIZ];

    snprintf(consoleTitle, MSG_SIZ, "%s: %s", szConsoleTitle, str);
    SetWindowText(hwndConsole, consoleTitle);

    if(appData.chatBoxes) { // [HGM] chat: open chat boxes
      char buf[MSG_SIZ], *p = buf, *q;
	safeStrCpy(buf, appData.chatBoxes, sizeof(buf)/sizeof(buf[0]) );
      do {
	q = strchr(p, ';');
	if(q) *q++ = 0;
	if(*p) ChatPopUp(p);
      } while(p=q);
    }

    SetActiveWindow(hwndMain);
}

void
DrawPosition(int fullRedraw, Board board)
{
  HDCDrawPosition(NULL, (BOOLEAN) fullRedraw, board); 
}

void NotifyFrontendLogin()
{
	if (hwndConsole)
		UpdateICSWidth(GetDlgItem(hwndConsole, OPT_ConsoleText));
}

VOID
ResetFrontEnd()
{
  fromX = fromY = -1;
  if (dragInfo.pos.x != -1 || dragInfo.pos.y != -1) {
    dragInfo.pos.x = dragInfo.pos.y = -1;
    dragInfo.pos.x = dragInfo.pos.y = -1;
    dragInfo.lastpos = dragInfo.pos;
    dragInfo.start.x = dragInfo.start.y = -1;
    dragInfo.from = dragInfo.start;
    ReleaseCapture();
    DrawPosition(TRUE, NULL);
  }
  TagsPopDown();
}


VOID
CommentPopUp(char *title, char *str)
{
  HWND hwnd = GetActiveWindow();
  EitherCommentPopUp(currentMove, title, str, FALSE); // [HGM] vari: fake move index, rather than 0
  SAY(str);
  SetActiveWindow(hwnd);
}

VOID
CommentPopDown(void)
{
  CheckMenuItem(GetMenu(hwndMain), IDM_Comment, MF_UNCHECKED);
  if (commentDialog) {
    ShowWindow(commentDialog, SW_HIDE);
  }
  commentUp = FALSE;
}

VOID
EditCommentPopUp(int index, char *title, char *str)
{
  EitherCommentPopUp(index, title, str, TRUE);
}


int
Roar()
{
  MyPlaySound(&sounds[(int)SoundRoar]);
  return 1;
}

VOID
RingBell()
{
  MyPlaySound(&sounds[(int)SoundMove]);
}

VOID PlayIcsWinSound()
{
  MyPlaySound(&sounds[(int)SoundIcsWin]);
}

VOID PlayIcsLossSound()
{
  MyPlaySound(&sounds[(int)SoundIcsLoss]);
}

VOID PlayIcsDrawSound()
{
  MyPlaySound(&sounds[(int)SoundIcsDraw]);
}

VOID PlayIcsUnfinishedSound()
{
  MyPlaySound(&sounds[(int)SoundIcsUnfinished]);
}

VOID
PlayAlarmSound()
{
  MyPlaySound(&sounds[(int)SoundAlarm]);
}

VOID
PlayTellSound()
{
  MyPlaySound(&textAttribs[ColorTell].sound);
}


VOID
EchoOn()
{
  HWND hInput;
  consoleEcho = TRUE;
  hInput = GetDlgItem(hwndConsole, OPT_ConsoleInput);
  SendMessage(hInput, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&consoleCF);
  SendMessage(hInput, EM_SETBKGNDCOLOR, FALSE, consoleBackgroundColor);
}


VOID
EchoOff()
{
  CHARFORMAT cf;
  HWND hInput;
  consoleEcho = FALSE;
  hInput = GetDlgItem(hwndConsole, OPT_ConsoleInput);
  /* This works OK: set text and background both to the same color */
  cf = consoleCF;
  cf.crTextColor = COLOR_ECHOOFF;
  SendMessage(hInput, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
  SendMessage(hInput, EM_SETBKGNDCOLOR, FALSE, cf.crTextColor);
}

/* No Raw()...? */

void Colorize(ColorClass cc, int continuation)
{
  currentColorClass = cc;
  consoleCF.dwMask = CFM_COLOR|CFM_BOLD|CFM_ITALIC|CFM_UNDERLINE|CFM_STRIKEOUT;
  consoleCF.crTextColor = textAttribs[cc].color;
  consoleCF.dwEffects = textAttribs[cc].effects;
  if (!continuation) MyPlaySound(&textAttribs[cc].sound);
}

char *
UserName()
{
  static char buf[MSG_SIZ];
  DWORD bufsiz = MSG_SIZ;

  if(appData.userName != NULL && appData.userName[0] != 0) { 
	return appData.userName; /* [HGM] username: prefer name selected by user over his system login */
  }
  if (!GetUserName(buf, &bufsiz)) {
    /*DisplayError("Error getting user name", GetLastError());*/
    safeStrCpy(buf, _("User"), sizeof(buf)/sizeof(buf[0]) );
  }
  return buf;
}

char *
HostName()
{
  static char buf[MSG_SIZ];
  DWORD bufsiz = MSG_SIZ;

  if (!GetComputerName(buf, &bufsiz)) {
    /*DisplayError("Error getting host name", GetLastError());*/
    safeStrCpy(buf, _("Unknown"), sizeof(buf)/sizeof(buf[0]) );
  }
  return buf;
}


int
ClockTimerRunning()
{
  return clockTimerEvent != 0;
}

int
StopClockTimer()
{
  if (clockTimerEvent == 0) return FALSE;
  KillTimer(hwndMain, clockTimerEvent);
  clockTimerEvent = 0;
  return TRUE;
}

void
StartClockTimer(long millisec)
{
  clockTimerEvent = SetTimer(hwndMain, (UINT) CLOCK_TIMER_ID,
			     (UINT) millisec, NULL);
}

void
DisplayWhiteClock(long timeRemaining, int highlight)
{
  HDC hdc;
  char *flag = whiteFlag && gameMode == TwoMachinesPlay ? "(!)" : "";

  if(appData.noGUI) return;
  hdc = GetDC(hwndMain);
  if (!IsIconic(hwndMain)) {
    DisplayAClock(hdc, timeRemaining, highlight, 
			flipClock ? &blackRect : &whiteRect, _("White"), flag);
  }
  if (highlight && iconCurrent == iconBlack) {
    iconCurrent = iconWhite;
    PostMessage(hwndMain, WM_SETICON, (WPARAM) TRUE, (LPARAM) iconCurrent);
    if (IsIconic(hwndMain)) {
      DrawIcon(hdc, 2, 2, iconCurrent);
    }
  }
  (void) ReleaseDC(hwndMain, hdc);
  if (hwndConsole)
    PostMessage(hwndConsole, WM_SETICON, (WPARAM) TRUE, (LPARAM) iconCurrent);
}

void
DisplayBlackClock(long timeRemaining, int highlight)
{
  HDC hdc;
  char *flag = blackFlag && gameMode == TwoMachinesPlay ? "(!)" : "";


  if(appData.noGUI) return;
  hdc = GetDC(hwndMain);
  if (!IsIconic(hwndMain)) {
    DisplayAClock(hdc, timeRemaining, highlight, 
			flipClock ? &whiteRect : &blackRect, _("Black"), flag);
  }
  if (highlight && iconCurrent == iconWhite) {
    iconCurrent = iconBlack;
    PostMessage(hwndMain, WM_SETICON, (WPARAM) TRUE, (LPARAM) iconCurrent);
    if (IsIconic(hwndMain)) {
      DrawIcon(hdc, 2, 2, iconCurrent);
    }
  }
  (void) ReleaseDC(hwndMain, hdc);
  if (hwndConsole)
    PostMessage(hwndConsole, WM_SETICON, (WPARAM) TRUE, (LPARAM) iconCurrent);
}


int
LoadGameTimerRunning()
{
  return loadGameTimerEvent != 0;
}

int
StopLoadGameTimer()
{
  if (loadGameTimerEvent == 0) return FALSE;
  KillTimer(hwndMain, loadGameTimerEvent);
  loadGameTimerEvent = 0;
  return TRUE;
}

void
StartLoadGameTimer(long millisec)
{
  loadGameTimerEvent = SetTimer(hwndMain, (UINT) LOAD_GAME_TIMER_ID,
				(UINT) millisec, NULL);
}

void
AutoSaveGame()
{
  char *defName;
  FILE *f;
  char fileTitle[MSG_SIZ];

  defName = DefaultFileName(appData.oldSaveStyle ? "gam" : "pgn");
  f = OpenFileDialog(hwndMain, "a", defName,
		     appData.oldSaveStyle ? "gam" : "pgn",
		     GAME_FILT, 
		     _("Save Game to File"), NULL, fileTitle, NULL);
  if (f != NULL) {
    SaveGame(f, 0, "");
    fclose(f);
  }
}


void
ScheduleDelayedEvent(DelayedEventCallback cb, long millisec)
{
  if (delayedTimerEvent != 0) {
    if (appData.debugMode && cb != delayedTimerCallback) { // [HGM] alive: not too much debug
      fprintf(debugFP, "ScheduleDelayedEvent: event already scheduled\n");
    }
    KillTimer(hwndMain, delayedTimerEvent);
    delayedTimerEvent = 0;
    if(delayedTimerCallback != cb) // [HGM] alive: do not "flush" same event, just postpone it
    delayedTimerCallback();
  }
  delayedTimerCallback = cb;
  delayedTimerEvent = SetTimer(hwndMain, (UINT) DELAYED_TIMER_ID,
				(UINT) millisec, NULL);
}

DelayedEventCallback
GetDelayedEvent()
{
  if (delayedTimerEvent) {
    return delayedTimerCallback;
  } else {
    return NULL;
  }
}

void
CancelDelayedEvent()
{
  if (delayedTimerEvent) {
    KillTimer(hwndMain, delayedTimerEvent);
    delayedTimerEvent = 0;
  }
}

DWORD GetWin32Priority(int nice)
{ // [HGM] nice: translate Unix nice() value to indows priority class. (Code stolen from Polyglot 1.4w11)
/*
REALTIME_PRIORITY_CLASS     0x00000100
HIGH_PRIORITY_CLASS         0x00000080
ABOVE_NORMAL_PRIORITY_CLASS 0x00008000
NORMAL_PRIORITY_CLASS       0x00000020
BELOW_NORMAL_PRIORITY_CLASS 0x00004000
IDLE_PRIORITY_CLASS         0x00000040
*/
        if (nice < -15) return 0x00000080;
        if (nice < 0)   return 0x00008000;
        if (nice == 0)  return 0x00000020;
        if (nice < 15)  return 0x00004000;
        return 0x00000040;
}

void RunCommand(char *cmdLine)
{
  /* Now create the child process. */
  STARTUPINFO siStartInfo;
  PROCESS_INFORMATION piProcInfo;

  siStartInfo.cb = sizeof(STARTUPINFO);
  siStartInfo.lpReserved = NULL;
  siStartInfo.lpDesktop = NULL;
  siStartInfo.lpTitle = NULL;
  siStartInfo.dwFlags = STARTF_USESTDHANDLES;
  siStartInfo.cbReserved2 = 0;
  siStartInfo.lpReserved2 = NULL;
  siStartInfo.hStdInput = NULL;
  siStartInfo.hStdOutput = NULL;
  siStartInfo.hStdError = NULL;

  CreateProcess(NULL,
		cmdLine,	   /* command line */
		NULL,	   /* process security attributes */
		NULL,	   /* primary thread security attrs */
		TRUE,	   /* handles are inherited */
		DETACHED_PROCESS|CREATE_NEW_PROCESS_GROUP,
		NULL,	   /* use parent's environment */
		NULL,
		&siStartInfo, /* STARTUPINFO pointer */
		&piProcInfo); /* receives PROCESS_INFORMATION */

  CloseHandle(piProcInfo.hThread);
}

/* Start a child process running the given program.
   The process's standard output can be read from "from", and its
   standard input can be written to "to".
   Exit with fatal error if anything goes wrong.
   Returns an opaque pointer that can be used to destroy the process
   later.
*/
int
StartChildProcess(char *cmdLine, char *dir, ProcRef *pr)
{
#define BUFSIZE 4096

  HANDLE hChildStdinRd, hChildStdinWr,
    hChildStdoutRd, hChildStdoutWr;
  HANDLE hChildStdinWrDup, hChildStdoutRdDup;
  SECURITY_ATTRIBUTES saAttr;
  BOOL fSuccess;
  PROCESS_INFORMATION piProcInfo;
  STARTUPINFO siStartInfo;
  ChildProc *cp;
  char buf[MSG_SIZ];
  DWORD err;

  if (appData.debugMode) {
    fprintf(debugFP, "StartChildProcess (dir=\"%s\") %s\n", dir, cmdLine);
  }

  *pr = NoProc;

  /* Set the bInheritHandle flag so pipe handles are inherited. */
  saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
  saAttr.bInheritHandle = TRUE;
  saAttr.lpSecurityDescriptor = NULL;

  /*
   * The steps for redirecting child's STDOUT:
   *     1. Create anonymous pipe to be STDOUT for child.
   *     2. Create a noninheritable duplicate of read handle,
   *         and close the inheritable read handle.
   */

  /* Create a pipe for the child's STDOUT. */
  if (! CreatePipe(&hChildStdoutRd, &hChildStdoutWr, &saAttr, 0)) {
    return GetLastError();
  }

  /* Duplicate the read handle to the pipe, so it is not inherited. */
  fSuccess = DuplicateHandle(GetCurrentProcess(), hChildStdoutRd,
			     GetCurrentProcess(), &hChildStdoutRdDup, 0,
			     FALSE,	/* not inherited */
			     DUPLICATE_SAME_ACCESS);
  if (! fSuccess) {
    return GetLastError();
  }
  CloseHandle(hChildStdoutRd);

  /*
   * The steps for redirecting child's STDIN:
   *     1. Create anonymous pipe to be STDIN for child.
   *     2. Create a noninheritable duplicate of write handle,
   *         and close the inheritable write handle.
   */

  /* Create a pipe for the child's STDIN. */
  if (! CreatePipe(&hChildStdinRd, &hChildStdinWr, &saAttr, 0)) {
    return GetLastError();
  }

  /* Duplicate the write handle to the pipe, so it is not inherited. */
  fSuccess = DuplicateHandle(GetCurrentProcess(), hChildStdinWr,
			     GetCurrentProcess(), &hChildStdinWrDup, 0,
			     FALSE,	/* not inherited */
			     DUPLICATE_SAME_ACCESS);
  if (! fSuccess) {
    return GetLastError();
  }
  CloseHandle(hChildStdinWr);

  /* Arrange to (1) look in dir for the child .exe file, and
   * (2) have dir be the child's working directory.  Interpret
   * dir relative to the directory WinBoard loaded from. */
  GetCurrentDirectory(MSG_SIZ, buf);
  SetCurrentDirectory(installDir);
  SetCurrentDirectory(dir);

  /* Now create the child process. */

  siStartInfo.cb = sizeof(STARTUPINFO);
  siStartInfo.lpReserved = NULL;
  siStartInfo.lpDesktop = NULL;
  siStartInfo.lpTitle = NULL;
  siStartInfo.dwFlags = STARTF_USESTDHANDLES;
  siStartInfo.cbReserved2 = 0;
  siStartInfo.lpReserved2 = NULL;
  siStartInfo.hStdInput = hChildStdinRd;
  siStartInfo.hStdOutput = hChildStdoutWr;
  siStartInfo.hStdError = hChildStdoutWr;

  fSuccess = CreateProcess(NULL,
			   cmdLine,	   /* command line */
			   NULL,	   /* process security attributes */
			   NULL,	   /* primary thread security attrs */
			   TRUE,	   /* handles are inherited */
			   DETACHED_PROCESS|CREATE_NEW_PROCESS_GROUP,
			   NULL,	   /* use parent's environment */
			   NULL,
			   &siStartInfo, /* STARTUPINFO pointer */
			   &piProcInfo); /* receives PROCESS_INFORMATION */

  err = GetLastError();
  SetCurrentDirectory(buf); /* return to prev directory */
  if (! fSuccess) {
    return err;
  }

  if (appData.niceEngines){ // [HGM] nice: adjust engine proc priority
    if(appData.debugMode) fprintf(debugFP, "nice engine proc to %d\n", appData.niceEngines);
    SetPriorityClass(piProcInfo.hProcess, GetWin32Priority(appData.niceEngines));
  }

  /* Close the handles we don't need in the parent */
  CloseHandle(piProcInfo.hThread);
  CloseHandle(hChildStdinRd);
  CloseHandle(hChildStdoutWr);

  /* Prepare return value */
  cp = (ChildProc *) calloc(1, sizeof(ChildProc));
  cp->kind = CPReal;
  cp->hProcess = piProcInfo.hProcess;
  cp->pid = piProcInfo.dwProcessId;
  cp->hFrom = hChildStdoutRdDup;
  cp->hTo = hChildStdinWrDup;

  *pr = (void *) cp;

  /* Klaus Friedel says that this Sleep solves a problem under Windows
     2000 where engines sometimes don't see the initial command(s)
     from WinBoard and hang.  I don't understand how that can happen,
     but the Sleep is harmless, so I've put it in.  Others have also
     reported what may be the same problem, so hopefully this will fix
     it for them too.  */
  Sleep(500);

  return NO_ERROR;
}


void
DestroyChildProcess(ProcRef pr, int/*boolean*/ signal)
{
  ChildProc *cp; int result;

  cp = (ChildProc *) pr;
  if (cp == NULL) return;

  switch (cp->kind) {
  case CPReal:
    /* TerminateProcess is considered harmful, so... */
    CloseHandle(cp->hTo); /* Closing this will give the child an EOF and hopefully kill it */
    if (cp->hFrom) CloseHandle(cp->hFrom);  /* if NULL, InputThread will close it */
    /* The following doesn't work because the chess program
       doesn't "have the same console" as WinBoard.  Maybe
       we could arrange for this even though neither WinBoard
       nor the chess program uses a console for stdio? */
    /*!!if (signal) GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, cp->pid);*/

    /* [AS] Special termination modes for misbehaving programs... */
    if( signal & 8 ) { 
        result = TerminateProcess( cp->hProcess, 0 );

        if ( appData.debugMode) {
            fprintf( debugFP, "Terminating process %lu, result=%d\n", cp->pid, result );
        }
    }
    else if( signal & 4 ) {
        DWORD dw = WaitForSingleObject( cp->hProcess, appData.delayAfterQuit*1000 + 50 ); // Wait 3 seconds at most

        if( dw != WAIT_OBJECT_0 ) {
            result = TerminateProcess( cp->hProcess, 0 );

            if ( appData.debugMode) {
                fprintf( debugFP, "Process %lu still alive after timeout, killing... result=%d\n", cp->pid, result );
            }

        }
    }

    CloseHandle(cp->hProcess);
    break;

  case CPComm:
    if (cp->hFrom) CloseHandle(cp->hFrom);
    break;

  case CPSock:
    closesocket(cp->sock);
    WSACleanup();
    break;

  case CPRcmd:
    if (signal) send(cp->sock2, "\017", 1, 0);  /* 017 = 15 = SIGTERM */
    closesocket(cp->sock);
    closesocket(cp->sock2);
    WSACleanup();
    break;
  }
  free(cp);
}

void
InterruptChildProcess(ProcRef pr)
{
  ChildProc *cp;

  cp = (ChildProc *) pr;
  if (cp == NULL) return;
  switch (cp->kind) {
  case CPReal:
    /* The following doesn't work because the chess program
       doesn't "have the same console" as WinBoard.  Maybe
       we could arrange for this even though neither WinBoard
       nor the chess program uses a console for stdio */
    /*!!GenerateConsoleCtrlEvent(CTRL_C_EVENT, cp->pid);*/
    break;

  case CPComm:
  case CPSock:
    /* Can't interrupt */
    break;

  case CPRcmd:
    send(cp->sock2, "\002", 1, 0);  /* 2 = SIGINT */
    break;
  }
}


int
OpenTelnet(char *host, char *port, ProcRef *pr)
{
  char cmdLine[MSG_SIZ];

  if (port[0] == NULLCHAR) {
    snprintf(cmdLine, MSG_SIZ, "%s %s", appData.telnetProgram, host);
  } else {
    snprintf(cmdLine, MSG_SIZ, "%s %s %s", appData.telnetProgram, host, port);
  }
  return StartChildProcess(cmdLine, "", pr);
}


/* Code to open TCP sockets */

int
OpenTCP(char *host, char *port, ProcRef *pr)
{
  ChildProc *cp;
  int err;
  SOCKET s;

  struct sockaddr_in sa, mysa;
  struct hostent FAR *hp;
  unsigned short uport;
  WORD wVersionRequested;
  WSADATA wsaData;

  /* Initialize socket DLL */
  wVersionRequested = MAKEWORD(1, 1);
  err = WSAStartup(wVersionRequested, &wsaData);
  if (err != 0) return err;

  /* Make socket */
  if ((s = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
    err = WSAGetLastError();
    WSACleanup();
    return err;
  }

  /* Bind local address using (mostly) don't-care values.
   */
  memset((char *) &mysa, 0, sizeof(struct sockaddr_in));
  mysa.sin_family = AF_INET;
  mysa.sin_addr.s_addr = INADDR_ANY;
  uport = (unsigned short) 0;
  mysa.sin_port = htons(uport);
  if (bind(s, (struct sockaddr *) &mysa, sizeof(struct sockaddr_in))
      == SOCKET_ERROR) {
    err = WSAGetLastError();
    WSACleanup();
    return err;
  }

  /* Resolve remote host name */
  memset((char *) &sa, 0, sizeof(struct sockaddr_in));
  if (!(hp = gethostbyname(host))) {
    unsigned int b0, b1, b2, b3;

    err = WSAGetLastError();

    if (sscanf(host, "%u.%u.%u.%u", &b0, &b1, &b2, &b3) == 4) {
      hp = (struct hostent *) calloc(1, sizeof(struct hostent));
      hp->h_addrtype = AF_INET;
      hp->h_length = 4;
      hp->h_addr_list = (char **) calloc(2, sizeof(char *));
      hp->h_addr_list[0] = (char *) malloc(4);
      hp->h_addr_list[0][0] = (char) b0;
      hp->h_addr_list[0][1] = (char) b1;
      hp->h_addr_list[0][2] = (char) b2;
      hp->h_addr_list[0][3] = (char) b3;
    } else {
      WSACleanup();
      return err;
    }
  }
  sa.sin_family = hp->h_addrtype;
  uport = (unsigned short) atoi(port);
  sa.sin_port = htons(uport);
  memcpy((char *) &sa.sin_addr, hp->h_addr, hp->h_length);

  /* Make connection */
  if (connect(s, (struct sockaddr *) &sa,
	      sizeof(struct sockaddr_in)) == SOCKET_ERROR) {
    err = WSAGetLastError();
    WSACleanup();
    return err;
  }

  /* Prepare return value */
  cp = (ChildProc *) calloc(1, sizeof(ChildProc));
  cp->kind = CPSock;
  cp->sock = s;
  *pr = (ProcRef *) cp;

  return NO_ERROR;
}

int
OpenCommPort(char *name, ProcRef *pr)
{
  HANDLE h;
  COMMTIMEOUTS ct;
  ChildProc *cp;
  char fullname[MSG_SIZ];

  if (*name != '\\')
    snprintf(fullname, MSG_SIZ, "\\\\.\\%s", name);
  else
    safeStrCpy(fullname, name, sizeof(fullname)/sizeof(fullname[0]) );

  h = CreateFile(name, GENERIC_READ | GENERIC_WRITE,
		 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
  if (h == (HANDLE) -1) {
    return GetLastError();
  }
  hCommPort = h;

  if (!SetCommState(h, (LPDCB) &dcb)) return GetLastError();

  /* Accumulate characters until a 100ms pause, then parse */
  ct.ReadIntervalTimeout = 100;
  ct.ReadTotalTimeoutMultiplier = 0;
  ct.ReadTotalTimeoutConstant = 0;
  ct.WriteTotalTimeoutMultiplier = 0;
  ct.WriteTotalTimeoutConstant = 0;
  if (!SetCommTimeouts(h, (LPCOMMTIMEOUTS) &ct)) return GetLastError();

  /* Prepare return value */
  cp = (ChildProc *) calloc(1, sizeof(ChildProc));
  cp->kind = CPComm;
  cp->hFrom = h;
  cp->hTo = h;
  *pr = (ProcRef *) cp;

  return NO_ERROR;
}

int
OpenLoopback(ProcRef *pr)
{
  DisplayFatalError(_("Not implemented"), 0, 1);
  return NO_ERROR;
}


int
OpenRcmd(char* host, char* user, char* cmd, ProcRef* pr)
{
  ChildProc *cp;
  int err;
  SOCKET s, s2, s3;
  struct sockaddr_in sa, mysa;
  struct hostent FAR *hp;
  unsigned short uport;
  WORD wVersionRequested;
  WSADATA wsaData;
  int fromPort;
  char stderrPortStr[MSG_SIZ];

  /* Initialize socket DLL */
  wVersionRequested = MAKEWORD(1, 1);
  err = WSAStartup(wVersionRequested, &wsaData);
  if (err != 0) return err;

  /* Resolve remote host name */
  memset((char *) &sa, 0, sizeof(struct sockaddr_in));
  if (!(hp = gethostbyname(host))) {
    unsigned int b0, b1, b2, b3;

    err = WSAGetLastError();

    if (sscanf(host, "%u.%u.%u.%u", &b0, &b1, &b2, &b3) == 4) {
      hp = (struct hostent *) calloc(1, sizeof(struct hostent));
      hp->h_addrtype = AF_INET;
      hp->h_length = 4;
      hp->h_addr_list = (char **) calloc(2, sizeof(char *));
      hp->h_addr_list[0] = (char *) malloc(4);
      hp->h_addr_list[0][0] = (char) b0;
      hp->h_addr_list[0][1] = (char) b1;
      hp->h_addr_list[0][2] = (char) b2;
      hp->h_addr_list[0][3] = (char) b3;
    } else {
      WSACleanup();
      return err;
    }
  }
  sa.sin_family = hp->h_addrtype;
  uport = (unsigned short) 514;
  sa.sin_port = htons(uport);
  memcpy((char *) &sa.sin_addr, hp->h_addr, hp->h_length);

  /* Bind local socket to unused "privileged" port address
   */
  s = INVALID_SOCKET;
  memset((char *) &mysa, 0, sizeof(struct sockaddr_in));
  mysa.sin_family = AF_INET;
  mysa.sin_addr.s_addr = INADDR_ANY;
  for (fromPort = 1023;; fromPort--) {
    if (fromPort < 0) {
      WSACleanup();
      return WSAEADDRINUSE;
    }
    if (s == INVALID_SOCKET) {
      if ((s = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
	err = WSAGetLastError();
	WSACleanup();
	return err;
      }
    }
    uport = (unsigned short) fromPort;
    mysa.sin_port = htons(uport);
    if (bind(s, (struct sockaddr *) &mysa, sizeof(struct sockaddr_in))
	== SOCKET_ERROR) {
      err = WSAGetLastError();
      if (err == WSAEADDRINUSE) continue;
      WSACleanup();
      return err;
    }
    if (connect(s, (struct sockaddr *) &sa,
      sizeof(struct sockaddr_in)) == SOCKET_ERROR) {
      err = WSAGetLastError();
      if (err == WSAEADDRINUSE) {
	closesocket(s);
        s = -1;
	continue;
      }
      WSACleanup();
      return err;
    }
    break;
  }

  /* Bind stderr local socket to unused "privileged" port address
   */
  s2 = INVALID_SOCKET;
  memset((char *) &mysa, 0, sizeof(struct sockaddr_in));
  mysa.sin_family = AF_INET;
  mysa.sin_addr.s_addr = INADDR_ANY;
  for (fromPort = 1023;; fromPort--) {
    if (fromPort == prevStderrPort) continue; // don't reuse port
    if (fromPort < 0) {
      (void) closesocket(s);
      WSACleanup();
      return WSAEADDRINUSE;
    }
    if (s2 == INVALID_SOCKET) {
      if ((s2 = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
	err = WSAGetLastError();
	closesocket(s);
	WSACleanup();
	return err;
      }
    }
    uport = (unsigned short) fromPort;
    mysa.sin_port = htons(uport);
    if (bind(s2, (struct sockaddr *) &mysa, sizeof(struct sockaddr_in))
	== SOCKET_ERROR) {
      err = WSAGetLastError();
      if (err == WSAEADDRINUSE) continue;
      (void) closesocket(s);
      WSACleanup();
      return err;
    }
    if (listen(s2, 1) == SOCKET_ERROR) {
      err = WSAGetLastError();
      if (err == WSAEADDRINUSE) {
	closesocket(s2);
	s2 = INVALID_SOCKET;
	continue;
      }
      (void) closesocket(s);
      (void) closesocket(s2);
      WSACleanup();
      return err;
    }
    break;
  }
  prevStderrPort = fromPort; // remember port used
  snprintf(stderrPortStr, MSG_SIZ, "%d", fromPort);

  if (send(s, stderrPortStr, strlen(stderrPortStr) + 1, 0) == SOCKET_ERROR) {
    err = WSAGetLastError();
    (void) closesocket(s);
    (void) closesocket(s2);
    WSACleanup();
    return err;
  }

  if (send(s, UserName(), strlen(UserName()) + 1, 0) == SOCKET_ERROR) {
    err = WSAGetLastError();
    (void) closesocket(s);
    (void) closesocket(s2);
    WSACleanup();
    return err;
  }
  if (*user == NULLCHAR) user = UserName();
  if (send(s, user, strlen(user) + 1, 0) == SOCKET_ERROR) {
    err = WSAGetLastError();
    (void) closesocket(s);
    (void) closesocket(s2);
    WSACleanup();
    return err;
  }
  if (send(s, cmd, strlen(cmd) + 1, 0) == SOCKET_ERROR) {
    err = WSAGetLastError();
    (void) closesocket(s);
    (void) closesocket(s2);
    WSACleanup();
    return err;
  }

  if ((s3 = accept(s2, NULL, NULL)) == INVALID_SOCKET) {
    err = WSAGetLastError();
    (void) closesocket(s);
    (void) closesocket(s2);
    WSACleanup();
    return err;
  }
  (void) closesocket(s2);  /* Stop listening */

  /* Prepare return value */
  cp = (ChildProc *) calloc(1, sizeof(ChildProc));
  cp->kind = CPRcmd;
  cp->sock = s;
  cp->sock2 = s3;
  *pr = (ProcRef *) cp;

  return NO_ERROR;
}


InputSourceRef
AddInputSource(ProcRef pr, int lineByLine,
	       InputCallback func, VOIDSTAR closure)
{
  InputSource *is, *is2 = NULL;
  ChildProc *cp = (ChildProc *) pr;

  is = (InputSource *) calloc(1, sizeof(InputSource));
  is->lineByLine = lineByLine;
  is->func = func;
  is->closure = closure;
  is->second = NULL;
  is->next = is->buf;
  if (pr == NoProc) {
    is->kind = CPReal;
    consoleInputSource = is;
  } else {
    is->kind = cp->kind;
    /* 
        [AS] Try to avoid a race condition if the thread is given control too early:
        we create all threads suspended so that the is->hThread variable can be
        safely assigned, then let the threads start with ResumeThread.
    */
    switch (cp->kind) {
    case CPReal:
      is->hFile = cp->hFrom;
      cp->hFrom = NULL; /* now owned by InputThread */
      is->hThread =
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) NonOvlInputThread,
		     (LPVOID) is, CREATE_SUSPENDED, &is->id);
      break;

    case CPComm:
      is->hFile = cp->hFrom;
      cp->hFrom = NULL; /* now owned by InputThread */
      is->hThread =
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) InputThread,
		     (LPVOID) is, CREATE_SUSPENDED, &is->id);
      break;

    case CPSock:
      is->sock = cp->sock;
      is->hThread =
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) SocketInputThread,
		     (LPVOID) is, CREATE_SUSPENDED, &is->id);
      break;

    case CPRcmd:
      is2 = (InputSource *) calloc(1, sizeof(InputSource));
      *is2 = *is;
      is->sock = cp->sock;
      is->second = is2;
      is2->sock = cp->sock2;
      is2->second = is2;
      is->hThread =
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) SocketInputThread,
		     (LPVOID) is, CREATE_SUSPENDED, &is->id);
      is2->hThread =
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) SocketInputThread,
		     (LPVOID) is2, CREATE_SUSPENDED, &is2->id);
      break;
    }

    if( is->hThread != NULL ) {
        ResumeThread( is->hThread );
    }

    if( is2 != NULL && is2->hThread != NULL ) {
        ResumeThread( is2->hThread );
    }
  }

  return (InputSourceRef) is;
}

void
RemoveInputSource(InputSourceRef isr)
{
  InputSource *is;

  is = (InputSource *) isr;
  is->hThread = NULL;  /* tell thread to stop */
  CloseHandle(is->hThread);
  if (is->second != NULL) {
    is->second->hThread = NULL;
    CloseHandle(is->second->hThread);
  }
}

int no_wrap(char *message, int count)
{
    ConsoleOutput(message, count, FALSE);
    return count;
}

int
OutputToProcess(ProcRef pr, char *message, int count, int *outError)
{
  DWORD dOutCount;
  int outCount = SOCKET_ERROR;
  ChildProc *cp = (ChildProc *) pr;
  static OVERLAPPED ovl;

  static int line = 0;

  if (pr == NoProc)
  {
    if (appData.noJoin || !appData.useInternalWrap)
      return no_wrap(message, count);
    else
    {
      int width = get_term_width();
      int len = wrap(NULL, message, count, width, &line);
      char *msg = malloc(len);
      int dbgchk;

      if (!msg)
        return no_wrap(message, count);
      else
      {
        dbgchk = wrap(msg, message, count, width, &line);
        if (dbgchk != len && appData.debugMode)
            fprintf(debugFP, "wrap(): dbgchk(%d) != len(%d)\n", dbgchk, len);
        ConsoleOutput(msg, len, FALSE);
        free(msg);
        return len;
      }
    }
  }

  if (ovl.hEvent == NULL) {
    ovl.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
  }
  ovl.Internal = ovl.InternalHigh = ovl.Offset = ovl.OffsetHigh = 0;

  switch (cp->kind) {
  case CPSock:
  case CPRcmd:
    outCount = send(cp->sock, message, count, 0);
    if (outCount == SOCKET_ERROR) {
      *outError = WSAGetLastError();
    } else {
      *outError = NO_ERROR;
    }
    break;

  case CPReal:
    if (WriteFile(((ChildProc *)pr)->hTo, message, count,
		  &dOutCount, NULL)) {
      *outError = NO_ERROR;
      outCount = (int) dOutCount;
    } else {
      *outError = GetLastError();
    }
    break;

  case CPComm:
    *outError = DoWriteFile(((ChildProc *)pr)->hTo, message, count,
			    &dOutCount, &ovl);
    if (*outError == NO_ERROR) {
      outCount = (int) dOutCount;
    }
    break;
  }
  return outCount;
}

void
DoSleep(int n)
{
    if(n != 0) Sleep(n);
}

int
OutputToProcessDelayed(ProcRef pr, char *message, int count, int *outError,
		       long msdelay)
{
  /* Ignore delay, not implemented for WinBoard */
  return OutputToProcess(pr, message, count, outError);
}


void
CmailSigHandlerCallBack(InputSourceRef isr, VOIDSTAR closure,
			char *buf, int count, int error)
{
  DisplayFatalError(_("Not implemented"), 0, 1);
}

/* see wgamelist.c for Game List functions */
/* see wedittags.c for Edit Tags functions */


int
ICSInitScript()
{
  FILE *f;
  char buf[MSG_SIZ];
  char *dummy;

  if (SearchPath(installDir, appData.icsLogon, NULL, MSG_SIZ, buf, &dummy)) {
    f = fopen(buf, "r");
    if (f != NULL) {
      ProcessICSInitScript(f);
      fclose(f);
      return TRUE;
    }
  }
  return FALSE;
}


VOID
StartAnalysisClock()
{
  if (analysisTimerEvent) return;
  analysisTimerEvent = SetTimer(hwndMain, (UINT) ANALYSIS_TIMER_ID,
		                        (UINT) 2000, NULL);
}

VOID
SetHighlights(int fromX, int fromY, int toX, int toY)
{
  highlightInfo.sq[0].x = fromX;
  highlightInfo.sq[0].y = fromY;
  highlightInfo.sq[1].x = toX;
  highlightInfo.sq[1].y = toY;
}

VOID
ClearHighlights()
{
  highlightInfo.sq[0].x = highlightInfo.sq[0].y = 
    highlightInfo.sq[1].x = highlightInfo.sq[1].y = -1;
}

VOID
SetPremoveHighlights(int fromX, int fromY, int toX, int toY)
{
  premoveHighlightInfo.sq[0].x = fromX;
  premoveHighlightInfo.sq[0].y = fromY;
  premoveHighlightInfo.sq[1].x = toX;
  premoveHighlightInfo.sq[1].y = toY;
}

VOID
ClearPremoveHighlights()
{
  premoveHighlightInfo.sq[0].x = premoveHighlightInfo.sq[0].y = 
    premoveHighlightInfo.sq[1].x = premoveHighlightInfo.sq[1].y = -1;
}

VOID
ShutDownFrontEnd()
{
  if (saveSettingsOnExit) SaveSettings(settingsFileName);
  DeleteClipboardTempFiles();
}

void
BoardToTop()
{
    if (IsIconic(hwndMain))
      ShowWindow(hwndMain, SW_RESTORE);

    SetActiveWindow(hwndMain);
}

/*
 * Prototypes for animation support routines
 */
static void ScreenSquare(int column, int row, POINT * pt);
static void Tween( POINT * start, POINT * mid, POINT * finish, int factor,
     POINT frames[], int * nFrames);


#define kFactor 4

void
AnimateAtomicCapture(Board board, int fromX, int fromY, int toX, int toY)
{	// [HGM] atomic: animate blast wave
	int i;

	explodeInfo.fromX = fromX;
	explodeInfo.fromY = fromY;
	explodeInfo.toX = toX;
	explodeInfo.toY = toY;
	for(i=1; i<4*kFactor; i++) {
	    explodeInfo.radius = (i*180)/(4*kFactor-1);
	    DrawPosition(FALSE, board);
	    Sleep(appData.animSpeed);
	}
	explodeInfo.radius = 0;
	DrawPosition(TRUE, board);
}

void
AnimateMove(board, fromX, fromY, toX, toY)
     Board board;
     int fromX;
     int fromY;
     int toX;
     int toY;
{
  ChessSquare piece;
  int x = toX, y = toY;
  POINT start, finish, mid;
  POINT frames[kFactor * 2 + 1];
  int nFrames, n;

  if(killX >= 0 && IS_LION(board[fromY][fromX])) Roar();

  if (!appData.animate) return;
  if (doingSizing) return;
  if (fromY < 0 || fromX < 0) return;
  piece = board[fromY][fromX];
  if (piece >= EmptySquare) return;

  if(killX >= 0) toX = killX, toY = killY; // [HGM] lion: first to kill square

again:

  ScreenSquare(fromX, fromY, &start);
  ScreenSquare(toX, toY, &finish);

  /* All moves except knight jumps move in straight line */
  if (!(abs(fromX-toX) == 1 && abs(fromY-toY) == 2 || abs(fromX-toX) == 2 && abs(fromY-toY) == 1)) {
    mid.x = start.x + (finish.x - start.x) / 2;
    mid.y = start.y + (finish.y - start.y) / 2;
  } else {
    /* Knight: make straight movement then diagonal */
    if (abs(toY - fromY) < abs(toX - fromX)) {
       mid.x = start.x + (finish.x - start.x) / 2;
       mid.y = start.y;
     } else {
       mid.x = start.x;
       mid.y = start.y + (finish.y - start.y) / 2;
     }
  }
  
  /* Don't use as many frames for very short moves */
  if (abs(toY - fromY) + abs(toX - fromX) <= 2)
    Tween(&start, &mid, &finish, kFactor - 1, frames, &nFrames);
  else
    Tween(&start, &mid, &finish, kFactor, frames, &nFrames);

  animInfo.from.x = fromX;
  animInfo.from.y = fromY;
  animInfo.to.x = toX;
  animInfo.to.y = toY;
  animInfo.lastpos = start;
  animInfo.piece = piece;
  for (n = 0; n < nFrames; n++) {
    animInfo.pos = frames[n];
    DrawPosition(FALSE, NULL);
    animInfo.lastpos = animInfo.pos;
    Sleep(appData.animSpeed);
  }
  animInfo.pos = finish;
  DrawPosition(FALSE, NULL);

  if(toX != x || toY != y) { fromX = toX; fromY = toY; toX = x; toY = y; goto again; } // second leg

  animInfo.piece = EmptySquare;
  Explode(board, fromX, fromY, toX, toY);
}

/*      Convert board position to corner of screen rect and color       */

static void
ScreenSquare(column, row, pt)
     int column; int row; POINT * pt;
{
  if (flipView) {
    pt->x = lineGap + ((BOARD_WIDTH-1)-column) * (squareSize + lineGap) + border;
    pt->y = lineGap + row * (squareSize + lineGap) + border;
  } else {
    pt->x = lineGap + column * (squareSize + lineGap) + border;
    pt->y = lineGap + ((BOARD_HEIGHT-1)-row) * (squareSize + lineGap) + border;
  }
}

/*      Generate a series of frame coords from start->mid->finish.
        The movement rate doubles until the half way point is
        reached, then halves back down to the final destination,
        which gives a nice slow in/out effect. The algorithmn
        may seem to generate too many intermediates for short
        moves, but remember that the purpose is to attract the
        viewers attention to the piece about to be moved and
        then to where it ends up. Too few frames would be less
        noticeable.                                             */

static void
Tween(start, mid, finish, factor, frames, nFrames)
     POINT * start; POINT * mid;
     POINT * finish; int factor;
     POINT frames[]; int * nFrames;
{
  int n, fraction = 1, count = 0;

  /* Slow in, stepping 1/16th, then 1/8th, ... */
  for (n = 0; n < factor; n++)
    fraction *= 2;
  for (n = 0; n < factor; n++) {
    frames[count].x = start->x + (mid->x - start->x) / fraction;
    frames[count].y = start->y + (mid->y - start->y) / fraction;
    count ++;
    fraction = fraction / 2;
  }
  
  /* Midpoint */
  frames[count] = *mid;
  count ++;
  
  /* Slow out, stepping 1/2, then 1/4, ... */
  fraction = 2;
  for (n = 0; n < factor; n++) {
    frames[count].x = finish->x - (finish->x - mid->x) / fraction;
    frames[count].y = finish->y - (finish->y - mid->y) / fraction;
    count ++;
    fraction = fraction * 2;
  }
  *nFrames = count;
}

void
SettingsPopUp(ChessProgramState *cps)
{     // [HGM] wrapper needed because handles must not be passed through back-end
      EngineOptionsPopup(savedHwnd, cps);
}

int flock(int fid, int code)
{
    HANDLE hFile = (HANDLE) _get_osfhandle(fid);
    OVERLAPPED ov;
    ov.hEvent = NULL;
    ov.Offset = 0;
    ov.OffsetHigh = 0;
    switch(code) {
      case 1: LockFileEx(hFile, LOCKFILE_EXCLUSIVE_LOCK, 0, 1024, 0, &ov); break;   // LOCK_SH

      case 2: LockFileEx(hFile, LOCKFILE_EXCLUSIVE_LOCK, 0, 1024, 0, &ov); break;   // LOCK_EX
      case 3: UnlockFileEx(hFile, 0, 1024, 0, &ov); break; // LOCK_UN
      default: return -1;
    }
    return 0;
}

char *
Col2Text (int n)
{
    static int i=0;
    static char col[8][20];
    COLORREF color = *(COLORREF *) colorVariable[n];
    i = i+1 & 7;
    snprintf(col[i], 20, "#%02lx%02lx%02lx", color&0xff, (color>>8)&0xff, (color>>16)&0xff);
    return col[i];
}

void
ActivateTheme (int new)
{   // Redo initialization of features depending on options that can occur in themes
   InitTextures();
   if(new) InitDrawingColors();
   fontBitmapSquareSize = 0; // request creation of new font pieces
   InitDrawingSizes(boardSize, 0);
   InvalidateRect(hwndMain, NULL, TRUE);
}
