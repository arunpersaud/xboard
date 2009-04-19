/*
 * WinBoard.c -- Windows NT front end to XBoard
 * $Id: winboard.c,v 2.3 2003/11/25 05:25:20 mann Exp $
 *
 * Copyright 1991 by Digital Equipment Corporation, Maynard, Massachusetts.
 * Enhancements Copyright 1992-2001 Free Software Foundation, Inc.
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
 * The following terms apply to the enhanced version of XBoard distributed
 * by the Free Software Foundation:
 * ------------------------------------------------------------------------
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * ------------------------------------------------------------------------
 */

#include "config.h"

#include <windows.h>
#include <winuser.h>
#include <winsock.h>

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

#if __GNUC__
#include <errno.h>
#include <string.h>
#endif

#include "common.h"
#include "winboard.h"
#include "frontend.h"
#include "backend.h"
#include "moves.h"
#include "wclipbrd.h"
#include "wgamelist.h"
#include "wedittags.h"
#include "woptions.h"
#include "wsockerr.h"
#include "defaults.h"

#include "wsnap.h"

void InitEngineUCI( const char * iniDir, ChessProgramState * cps );

  int myrandom(void);
  void mysrandom(unsigned int seed);

extern int whiteFlag, blackFlag;
Boolean flipClock = FALSE;

void DisplayHoldingsCount(HDC hdc, int x, int y, int align, int copyNumber);

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
} DragInfo;

static DragInfo dragInfo = { {-1,-1}, {-1,-1}, {-1,-1}, {-1,-1} };

typedef struct {
  POINT sq[2];	  /* board coordinates of from, to squares */
} HighlightInfo;

static HighlightInfo highlightInfo        = { {{-1, -1}, {-1, -1}} };
static HighlightInfo premoveHighlightInfo = { {{-1, -1}, {-1, -1}} };

/* Window class names */
char szAppName[] = "WinBoard";
char szConsoleName[] = "WBConsole";

/* Title bar text */
char szTitle[] = "WinBoard";
char szConsoleTitle[] = "ICS Interaction";

char *programName;
char *settingsFileName;
BOOLEAN saveSettingsOnExit;
char installDir[MSG_SIZ];

BoardSize boardSize;
BOOLEAN chessProgram;
static int boardX, boardY, consoleX, consoleY, consoleW, consoleH;
static int squareSize, lineGap, minorSize;
static int winWidth, winHeight;
static RECT messageRect, whiteRect, blackRect;
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

#define ARG_MAX 128*1024 /* [AS] For Roger Brown's very long list! */

#define PALETTESIZE 256

HINSTANCE hInst;          /* current instance */
HWND hwndMain = NULL;        /* root window*/
HWND hwndConsole = NULL;
BOOLEAN alwaysOnTop = FALSE;
RECT boardRect;
COLORREF lightSquareColor, darkSquareColor, whitePieceColor, 
  blackPieceColor, highlightSquareColor, premoveHighlightColor;
HPALETTE hPal;
ColorClass currentColorClass;

HWND hCommPort = NULL;    /* currently open comm port */
static HWND hwndPause;    /* pause button */
static HBITMAP pieceBitmap[3][(int) BlackPawn]; /* [HGM] nr of bitmaps referred to bP in stead of wK */
static HBRUSH lightSquareBrush, darkSquareBrush,
  blackSquareBrush, /* [HGM] for band between board and holdings */
  whitePieceBrush, blackPieceBrush, iconBkgndBrush, outlineBrush;
static POINT gridEndpoints[(BOARD_SIZE + 1) * 4];
static DWORD gridVertexCounts[(BOARD_SIZE + 1) * 2];
static HPEN gridPen = NULL;
static HPEN highlightPen = NULL;
static HPEN premovePen = NULL;
static NPLOGPALETTE pLogPal;
static BOOL paletteChanged = FALSE;
static HICON iconWhite, iconBlack, iconCurrent;
static int doingSizing = FALSE;
static int lastSizing = 0;
static int prevStderrPort;

/* [AS] Support for background textures */
#define BACK_TEXTURE_MODE_DISABLED      0
#define BACK_TEXTURE_MODE_PLAIN         1
#define BACK_TEXTURE_MODE_FULL_RANDOM   2

static HBITMAP liteBackTexture = NULL;
static HBITMAP darkBackTexture = NULL;
static int liteBackTextureMode = BACK_TEXTURE_MODE_PLAIN;
static int darkBackTextureMode = BACK_TEXTURE_MODE_PLAIN;
static int backTextureSquareSize = 0;
static struct { int x; int y; int mode; } backTextureSquareInfo[BOARD_SIZE][BOARD_SIZE];

#if __GNUC__ && !defined(_winmajor)
#define oldDialog 0 /* cygwin doesn't define _winmajor; mingw does */
#else
#define oldDialog (_winmajor < 4)
#endif

char *defaultTextAttribs[] = 
{
  COLOR_SHOUT, COLOR_SSHOUT, COLOR_CHANNEL1, COLOR_CHANNEL, COLOR_KIBITZ,
  COLOR_TELL, COLOR_CHALLENGE, COLOR_REQUEST, COLOR_SEEK, COLOR_NORMAL,
  COLOR_NONE
};

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

#define MF(x) {x, {0, }, {0, }, 0}
MyFont fontRec[NUM_SIZES][NUM_FONTS] =
{
  { MF(CLOCK_FONT_TINY), MF(MESSAGE_FONT_TINY), MF(COORD_FONT_TINY), MF(CONSOLE_FONT_TINY), MF(COMMENT_FONT_TINY), MF(EDITTAGS_FONT_TINY), MF(MOVEHISTORY_FONT_ALL) },
  { MF(CLOCK_FONT_TEENY), MF(MESSAGE_FONT_TEENY), MF(COORD_FONT_TEENY), MF(CONSOLE_FONT_TEENY), MF(COMMENT_FONT_TEENY), MF(EDITTAGS_FONT_TEENY), MF(MOVEHISTORY_FONT_ALL) },
  { MF(CLOCK_FONT_DINKY), MF(MESSAGE_FONT_DINKY), MF(COORD_FONT_DINKY), MF(CONSOLE_FONT_DINKY), MF(COMMENT_FONT_DINKY), MF(EDITTAGS_FONT_DINKY), MF(MOVEHISTORY_FONT_ALL) },
  { MF(CLOCK_FONT_PETITE), MF(MESSAGE_FONT_PETITE), MF(COORD_FONT_PETITE), MF(CONSOLE_FONT_PETITE), MF(COMMENT_FONT_PETITE), MF(EDITTAGS_FONT_PETITE), MF(MOVEHISTORY_FONT_ALL) },
  { MF(CLOCK_FONT_SLIM), MF(MESSAGE_FONT_SLIM), MF(COORD_FONT_SLIM), MF(CONSOLE_FONT_SLIM), MF(COMMENT_FONT_SLIM), MF(EDITTAGS_FONT_SLIM), MF(MOVEHISTORY_FONT_ALL) },
  { MF(CLOCK_FONT_SMALL), MF(MESSAGE_FONT_SMALL), MF(COORD_FONT_SMALL), MF(CONSOLE_FONT_SMALL), MF(COMMENT_FONT_SMALL), MF(EDITTAGS_FONT_SMALL), MF(MOVEHISTORY_FONT_ALL) },
  { MF(CLOCK_FONT_MEDIOCRE), MF(MESSAGE_FONT_MEDIOCRE), MF(COORD_FONT_MEDIOCRE), MF(CONSOLE_FONT_MEDIOCRE), MF(COMMENT_FONT_MEDIOCRE), MF(EDITTAGS_FONT_MEDIOCRE), MF(MOVEHISTORY_FONT_ALL) },
  { MF(CLOCK_FONT_MIDDLING), MF(MESSAGE_FONT_MIDDLING), MF(COORD_FONT_MIDDLING), MF(CONSOLE_FONT_MIDDLING), MF(COMMENT_FONT_MIDDLING), MF(EDITTAGS_FONT_MIDDLING), MF(MOVEHISTORY_FONT_ALL) },
  { MF(CLOCK_FONT_AVERAGE), MF(MESSAGE_FONT_AVERAGE), MF(COORD_FONT_AVERAGE), MF(CONSOLE_FONT_AVERAGE), MF(COMMENT_FONT_AVERAGE), MF(EDITTAGS_FONT_AVERAGE), MF(MOVEHISTORY_FONT_ALL) },
  { MF(CLOCK_FONT_MODERATE), MF(MESSAGE_FONT_MODERATE), MF(COORD_FONT_MODERATE), MF(CONSOLE_FONT_MODERATE), MF(COMMENT_FONT_MODERATE), MF(EDITTAGS_FONT_MODERATE), MF(MOVEHISTORY_FONT_ALL) },
  { MF(CLOCK_FONT_MEDIUM), MF(MESSAGE_FONT_MEDIUM), MF(COORD_FONT_MEDIUM), MF(CONSOLE_FONT_MEDIUM), MF(COMMENT_FONT_MEDIUM), MF(EDITTAGS_FONT_MEDIUM), MF(MOVEHISTORY_FONT_ALL) },
  { MF(CLOCK_FONT_BULKY), MF(MESSAGE_FONT_BULKY), MF(COORD_FONT_BULKY), MF(CONSOLE_FONT_BULKY), MF(COMMENT_FONT_BULKY), MF(EDITTAGS_FONT_BULKY), MF(MOVEHISTORY_FONT_ALL) },
  { MF(CLOCK_FONT_LARGE), MF(MESSAGE_FONT_LARGE), MF(COORD_FONT_LARGE), MF(CONSOLE_FONT_LARGE), MF(COMMENT_FONT_LARGE), MF(EDITTAGS_FONT_LARGE), MF(MOVEHISTORY_FONT_ALL) },
  { MF(CLOCK_FONT_BIG), MF(MESSAGE_FONT_BIG), MF(COORD_FONT_BIG), MF(CONSOLE_FONT_BIG), MF(COMMENT_FONT_BIG), MF(EDITTAGS_FONT_BIG), MF(MOVEHISTORY_FONT_ALL) },
  { MF(CLOCK_FONT_HUGE), MF(MESSAGE_FONT_HUGE), MF(COORD_FONT_HUGE), MF(CONSOLE_FONT_HUGE), MF(COMMENT_FONT_HUGE), MF(EDITTAGS_FONT_HUGE), MF(MOVEHISTORY_FONT_ALL) },
  { MF(CLOCK_FONT_GIANT), MF(MESSAGE_FONT_GIANT), MF(COORD_FONT_GIANT), MF(CONSOLE_FONT_GIANT), MF(COMMENT_FONT_GIANT), MF(EDITTAGS_FONT_GIANT), MF(MOVEHISTORY_FONT_ALL) },
  { MF(CLOCK_FONT_COLOSSAL), MF(MESSAGE_FONT_COLOSSAL), MF(COORD_FONT_COLOSSAL), MF(CONSOLE_FONT_COLOSSAL), MF(COMMENT_FONT_COLOSSAL), MF(EDITTAGS_FONT_COLOSSAL), MF(MOVEHISTORY_FONT_ALL) },
  { MF(CLOCK_FONT_TITANIC), MF(MESSAGE_FONT_TITANIC), MF(COORD_FONT_TITANIC), MF(CONSOLE_FONT_TITANIC), MF(COMMENT_FONT_TITANIC), MF(EDITTAGS_FONT_TITANIC), MF(MOVEHISTORY_FONT_ALL) },
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
#define MENU_BAR_ITEMS 6
char *menuBarText[2][MENU_BAR_ITEMS+1] = {
  { "&File", "&Mode", "&Action", "&Step", "&Options", "&Help", NULL },
  { "&F", "&M", "&A", "&S", "&O", "&H", NULL },
};


MySound sounds[(int)NSoundClasses];
MyTextAttribs textAttribs[(int)NColorClasses];

MyColorizeAttribs colorizeAttribs[] = {
  { (COLORREF)0, 0, "Shout Text" },
  { (COLORREF)0, 0, "SShout/CShout" },
  { (COLORREF)0, 0, "Channel 1 Text" },
  { (COLORREF)0, 0, "Channel Text" },
  { (COLORREF)0, 0, "Kibitz Text" },
  { (COLORREF)0, 0, "Tell Text" },
  { (COLORREF)0, 0, "Challenge Text" },
  { (COLORREF)0, 0, "Request Text" },
  { (COLORREF)0, 0, "Seek Text" },
  { (COLORREF)0, 0, "Normal Text" },
  { (COLORREF)0, 0, "None" }
};



static char *commentTitle;
static char *commentText;
static int commentIndex;
static Boolean editComment = FALSE;
HWND commentDialog = NULL;
BOOLEAN commentDialogUp = FALSE;
static int commentX, commentY, commentH, commentW;

static char *analysisTitle;
static char *analysisText;
HWND analysisDialog = NULL;
BOOLEAN analysisDialogUp = FALSE;
static int analysisX, analysisY, analysisH, analysisW;

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
VOID PopUpMoveDialog(char firstchar);
VOID UpdateSampleText(HWND hDlg, int id, MyColorizeAttribs *mca);

/* [AS] */
int NewGameFRC();
int GameListOptions();

HWND moveHistoryDialog = NULL;
BOOLEAN moveHistoryDialogUp = FALSE;

WindowPlacement wpMoveHistory;

HWND evalGraphDialog = NULL;
BOOLEAN evalGraphDialogUp = FALSE;

WindowPlacement wpEvalGraph;

HWND engineOutputDialog = NULL;
BOOLEAN engineOutputDialogUp = FALSE;

WindowPlacement wpEngineOutput;

VOID MoveHistoryPopUp();
VOID MoveHistoryPopDown();
VOID MoveHistorySet( char movelist[][2*MOVE_LEN], int first, int last, int current, ChessProgramStats_Move * pvInfo );
BOOL MoveHistoryIsUp();

VOID EvalGraphSet( int first, int last, int current, ChessProgramStats_Move * pvInfo );
VOID EvalGraphPopUp();
VOID EvalGraphPopDown();
BOOL EvalGraphIsUp();

VOID EngineOutputPopUp();
VOID EngineOutputPopDown();
BOOL EngineOutputIsUp();
VOID EngineOutputUpdate( FrontEndProgramStats * stats );

VOID GothicPopUp(char *title, char up);
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

/*---------------------------------------------------------------------------*\
 *
 * WinMain
 *
\*---------------------------------------------------------------------------*/

int APIENTRY
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
  MSG msg;
  HANDLE hAccelMain, hAccelNoAlt, hAccelNoICS;

  debugFP = stderr;

  LoadLibrary("RICHED32.DLL");
  consoleCF.cbSize = sizeof(CHARFORMAT);

  if (!InitApplication(hInstance)) {
    return (FALSE);
  }
  if (!InitInstance(hInstance, nCmdShow, lpCmdLine)) {
    return (FALSE);
  }

  hAccelMain = LoadAccelerators (hInstance, szAppName);
  hAccelNoAlt = LoadAccelerators (hInstance, "NO_ALT");
  hAccelNoICS = LoadAccelerators( hInstance, "NO_ICS"); /* [AS] No Ctrl-V on ICS!!! */

  /* Acquire and dispatch messages until a WM_QUIT message is received. */

  while (GetMessage(&msg, /* message structure */
		    NULL, /* handle of window receiving the message */
		    0,    /* lowest message to examine */
		    0))   /* highest message to examine */
    {
      if (!(commentDialog && IsDialogMessage(commentDialog, &msg)) &&
          !(moveHistoryDialog && IsDialogMessage(moveHistoryDialog, &msg)) &&
          !(evalGraphDialog && IsDialogMessage(evalGraphDialog, &msg)) &&
          !(engineOutputDialog && IsDialogMessage(engineOutputDialog, &msg)) &&
	  !(editTagsDialog && IsDialogMessage(editTagsDialog, &msg)) &&
	  !(gameListDialog && IsDialogMessage(gameListDialog, &msg)) &&
	  !(errorDialog && IsDialogMessage(errorDialog, &msg)) &&
	  !(!frozen && TranslateAccelerator(hwndMain, hAccelMain, &msg)) &&
          !(!hwndConsole && TranslateAccelerator(hwndMain, hAccelNoICS, &msg)) &&
	  !(!hwndConsole && TranslateAccelerator(hwndMain, hAccelNoAlt, &msg))) {
	TranslateMessage(&msg);	/* Translates virtual key codes */
	DispatchMessage(&msg);	/* Dispatches message to window */
      }
    }


  return (msg.wParam);	/* Returns the value from PostQuitMessage */
}

/*---------------------------------------------------------------------------*\
 *
 * Initialization functions
 *
\*---------------------------------------------------------------------------*/

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

void
EnsureOnScreen(int *x, int *y)
{
  int gap = GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CYCAPTION);
  /* Be sure window at (x,y) is not off screen (or even mostly off screen) */
  if (*x > screenWidth - 32) *x = 0;
  if (*y > screenHeight - 32) *y = 0;
  if (*x < 10) *x = 10;
  if (*y < gap) *y = gap;
}

BOOL
InitInstance(HINSTANCE hInstance, int nCmdShow, LPSTR lpCmdLine)
{
  HWND hwnd; /* Main window handle. */
  int ibs;
  WINDOWPLACEMENT wp;
  char *filepart;

  hInst = hInstance;	/* Store instance handle in our global variable */

  if (SearchPath(NULL, "WinBoard.exe", NULL, MSG_SIZ, installDir, &filepart)) {
    *filepart = NULLCHAR;
  } else {
    GetCurrentDirectory(MSG_SIZ, installDir);
  }
  gameInfo.boardWidth = gameInfo.boardHeight = 8; // [HGM] won't have open window otherwise
  InitAppData(lpCmdLine);      /* Get run-time parameters */
  if (appData.debugMode) {
    debugFP = fopen(appData.nameOfDebugFile, "w");
    setbuf(debugFP, NULL);
  }

  InitBackEnd1();

  InitEngineUCI( installDir, &first );
  InitEngineUCI( installDir, &second );

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

  iconWhite = LoadIcon(hInstance, "icon_white");
  iconBlack = LoadIcon(hInstance, "icon_black");
  iconCurrent = iconWhite;
  InitDrawingColors();
  screenHeight = GetSystemMetrics(SM_CYSCREEN);
  screenWidth = GetSystemMetrics(SM_CXSCREEN);
  for (ibs = (int) NUM_SIZES - 1; ibs >= 0; ibs--) {
    /* Compute window size for each board size, and use the largest
       size that fits on this screen as the default. */
    InitDrawingSizes((BoardSize)ibs, 0);
    if (boardSize == (BoardSize)-1 &&
        winHeight <= screenHeight
           - GetSystemMetrics(SM_CYFRAME) - GetSystemMetrics(SM_CYCAPTION) - 10
        && winWidth <= screenWidth) {
      boardSize = (BoardSize)ibs;
    }
  }
  InitDrawingSizes(boardSize, 0);
  InitMenuChecks();
  buttonCount = GetSystemMetrics(SM_CMOUSEBUTTONS);

  /* [AS] Load textures if specified */
  ZeroMemory( &backTextureSquareInfo, sizeof(backTextureSquareInfo) );
  
  if( appData.liteBackTextureFile && appData.liteBackTextureFile[0] != NULLCHAR && appData.liteBackTextureFile[0] != '*' ) {
      liteBackTexture = LoadImage( 0, appData.liteBackTextureFile, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE );
      liteBackTextureMode = appData.liteBackTextureMode;

      if (liteBackTexture == NULL && appData.debugMode) {
          fprintf( debugFP, "Unable to load lite texture bitmap '%s'\n", appData.liteBackTextureFile );
      }
  }
  
  if( appData.darkBackTextureFile && appData.darkBackTextureFile[0] != NULLCHAR && appData.darkBackTextureFile[0] != '*' ) {
      darkBackTexture = LoadImage( 0, appData.darkBackTextureFile, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE );
      darkBackTextureMode = appData.darkBackTextureMode;

      if (darkBackTexture == NULL && appData.debugMode) {
          fprintf( debugFP, "Unable to load dark texture bitmap '%s'\n", appData.darkBackTextureFile );
      }
  }

  mysrandom( (unsigned) time(NULL) );

  /* Make a console window if needed */
  if (appData.icsActive) {
    ConsoleCreate();
  }

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

  InitBackEnd2();

  /* Make the window visible; update its client area; and return "success" */
  EnsureOnScreen(&boardX, &boardY);
  wp.length = sizeof(WINDOWPLACEMENT);
  wp.flags = 0;
  wp.showCmd = nCmdShow;
  wp.ptMaxPosition.x = wp.ptMaxPosition.y = 0;
  wp.rcNormalPosition.left = boardX;
  wp.rcNormalPosition.right = boardX + winWidth;
  wp.rcNormalPosition.top = boardY;
  wp.rcNormalPosition.bottom = boardY + winHeight;
  SetWindowPlacement(hwndMain, &wp);

  SetWindowPos(hwndMain, alwaysOnTop ? HWND_TOPMOST : HWND_NOTOPMOST,
               0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);

  /* [AS] Disable the FRC stuff if not playing the proper variant */
  if( gameInfo.variant != VariantFischeRandom ) {
      EnableMenuItem( GetMenu(hwndMain), IDM_NewGameFRC, MF_GRAYED );
  }

  if (hwndConsole) {
#if AOT_CONSOLE
    SetWindowPos(hwndConsole, alwaysOnTop ? HWND_TOPMOST : HWND_NOTOPMOST,
                 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
#endif
    ShowWindow(hwndConsole, nCmdShow);
  }
  UpdateWindow(hwnd);

  return TRUE;

}


typedef enum {
  ArgString, ArgInt, ArgFloat, ArgBoolean, ArgTrue, ArgFalse, ArgNone, 
  ArgColor, ArgAttribs, ArgFilename, ArgBoardSize, ArgFont, ArgCommSettings,
  ArgSettingsFilename
} ArgType;

typedef struct {
  char *argName;
  ArgType argType;
  /***
  union {
    String *pString;       // ArgString
    int *pInt;             // ArgInt
    float *pFloat;         // ArgFloat
    Boolean *pBoolean;     // ArgBoolean
    COLORREF *pColor;      // ArgColor
    ColorClass cc;         // ArgAttribs
    String *pFilename;     // ArgFilename
    BoardSize *pBoardSize; // ArgBoardSize
    int whichFont;         // ArgFont
    DCB *pDCB;             // ArgCommSettings
    String *pFilename;     // ArgSettingsFilename
  } argLoc;
  ***/
  LPVOID argLoc;
  BOOL save;
} ArgDescriptor;

int junk;
ArgDescriptor argDescriptors[] = {
  /* positional arguments */
  { "loadGameFile", ArgFilename, (LPVOID) &appData.loadGameFile, FALSE },
  { "", ArgNone, NULL },
  /* keyword arguments */
  { "whitePieceColor", ArgColor, (LPVOID) &whitePieceColor, TRUE },
  { "wpc", ArgColor, (LPVOID) &whitePieceColor, FALSE },
  { "blackPieceColor", ArgColor, (LPVOID) &blackPieceColor, TRUE },
  { "bpc", ArgColor, (LPVOID) &blackPieceColor, FALSE },
  { "lightSquareColor", ArgColor, (LPVOID) &lightSquareColor, TRUE },
  { "lsc", ArgColor, (LPVOID) &lightSquareColor, FALSE },
  { "darkSquareColor", ArgColor, (LPVOID) &darkSquareColor, TRUE },
  { "dsc", ArgColor, (LPVOID) &darkSquareColor, FALSE },
  { "highlightSquareColor", ArgColor, (LPVOID) &highlightSquareColor, TRUE },
  { "hsc", ArgColor, (LPVOID) &highlightSquareColor, FALSE },
  { "premoveHighlightColor", ArgColor, (LPVOID) &premoveHighlightColor, TRUE },
  { "phc", ArgColor, (LPVOID) &premoveHighlightColor, FALSE },
  { "movesPerSession", ArgInt, (LPVOID) &appData.movesPerSession, TRUE },
  { "mps", ArgInt, (LPVOID) &appData.movesPerSession, FALSE },
  { "initString", ArgString, (LPVOID) &appData.initString, FALSE },
  { "firstInitString", ArgString, (LPVOID) &appData.initString, FALSE },
  { "secondInitString", ArgString, (LPVOID) &appData.secondInitString, FALSE },
  { "firstComputerString", ArgString, (LPVOID) &appData.firstComputerString,
    FALSE },
  { "secondComputerString", ArgString, (LPVOID) &appData.secondComputerString,
    FALSE },
  { "firstChessProgram", ArgFilename, (LPVOID) &appData.firstChessProgram,
    FALSE },
  { "fcp", ArgFilename, (LPVOID) &appData.firstChessProgram, FALSE },
  { "secondChessProgram", ArgFilename, (LPVOID) &appData.secondChessProgram,
    FALSE },
  { "scp", ArgFilename, (LPVOID) &appData.secondChessProgram, FALSE },
  { "firstPlaysBlack", ArgBoolean, (LPVOID) &appData.firstPlaysBlack, FALSE },
  { "fb", ArgTrue, (LPVOID) &appData.firstPlaysBlack, FALSE },
  { "xfb", ArgFalse, (LPVOID) &appData.firstPlaysBlack, FALSE },
  { "-fb", ArgFalse, (LPVOID) &appData.firstPlaysBlack, FALSE },
  { "noChessProgram", ArgBoolean, (LPVOID) &appData.noChessProgram, FALSE },
  { "ncp", ArgTrue, (LPVOID) &appData.noChessProgram, FALSE },
  { "xncp", ArgFalse, (LPVOID) &appData.noChessProgram, FALSE },
  { "-ncp", ArgFalse, (LPVOID) &appData.noChessProgram, FALSE },
  { "firstHost", ArgString, (LPVOID) &appData.firstHost, FALSE },
  { "fh", ArgString, (LPVOID) &appData.firstHost, FALSE },
  { "secondHost", ArgString, (LPVOID) &appData.secondHost, FALSE },
  { "sh", ArgString, (LPVOID) &appData.secondHost, FALSE },
  { "firstDirectory", ArgFilename, (LPVOID) &appData.firstDirectory, FALSE },
  { "fd", ArgFilename, (LPVOID) &appData.firstDirectory, FALSE },
  { "secondDirectory", ArgFilename, (LPVOID) &appData.secondDirectory, FALSE },
  { "sd", ArgFilename, (LPVOID) &appData.secondDirectory, FALSE },
  /*!!bitmapDirectory?*/
  { "remoteShell", ArgFilename, (LPVOID) &appData.remoteShell, FALSE },
  { "rsh", ArgFilename, (LPVOID) &appData.remoteShell, FALSE },
  { "remoteUser", ArgString, (LPVOID) &appData.remoteUser, FALSE },
  { "ruser", ArgString, (LPVOID) &appData.remoteUser, FALSE },
  { "timeDelay", ArgFloat, (LPVOID) &appData.timeDelay, TRUE },
  { "td", ArgFloat, (LPVOID) &appData.timeDelay, FALSE },
  { "timeControl", ArgString, (LPVOID) &appData.timeControl, TRUE },
  { "tc", ArgString, (LPVOID) &appData.timeControl, FALSE },
  { "timeIncrement", ArgInt, (LPVOID) &appData.timeIncrement, TRUE },
  { "inc", ArgInt, (LPVOID) &appData.timeIncrement, FALSE },
  { "internetChessServerMode", ArgBoolean, (LPVOID) &appData.icsActive, FALSE },
  { "ics", ArgTrue, (LPVOID) &appData.icsActive, FALSE },
  { "xics", ArgFalse, (LPVOID) &appData.icsActive, FALSE },
  { "-ics", ArgFalse, (LPVOID) &appData.icsActive, FALSE },
  { "internetChessServerHost", ArgString, (LPVOID) &appData.icsHost, FALSE },
  { "icshost", ArgString, (LPVOID) &appData.icsHost, FALSE },
  { "internetChessServerPort", ArgString, (LPVOID) &appData.icsPort, FALSE },
  { "icsport", ArgString, (LPVOID) &appData.icsPort, FALSE },
  { "internetChessServerCommPort", ArgString, (LPVOID) &appData.icsCommPort, FALSE },
  { "icscomm", ArgString, (LPVOID) &appData.icsCommPort, FALSE },
  { "internetChessServerComPort", ArgString, (LPVOID) &appData.icsCommPort, FALSE },
  { "icscom", ArgString, (LPVOID) &appData.icsCommPort, FALSE },
  { "internetChessServerLogonScript", ArgFilename, (LPVOID) &appData.icsLogon, FALSE },
  { "icslogon", ArgFilename, (LPVOID) &appData.icsLogon, FALSE },
  { "useTelnet", ArgBoolean, (LPVOID) &appData.useTelnet, FALSE },
  { "telnet", ArgTrue, (LPVOID) &appData.useTelnet, FALSE },
  { "xtelnet", ArgFalse, (LPVOID) &appData.useTelnet, FALSE },
  { "-telnet", ArgFalse, (LPVOID) &appData.useTelnet, FALSE },
  { "telnetProgram", ArgFilename, (LPVOID) &appData.telnetProgram, FALSE },
  { "icshelper", ArgFilename, (LPVOID) &appData.icsHelper, FALSE },
  { "gateway", ArgString, (LPVOID) &appData.gateway, FALSE },
  { "loadGameFile", ArgFilename, (LPVOID) &appData.loadGameFile, FALSE },
  { "lgf", ArgFilename, (LPVOID) &appData.loadGameFile, FALSE },
  { "loadGameIndex", ArgInt, (LPVOID) &appData.loadGameIndex, FALSE },
  { "lgi", ArgInt, (LPVOID) &appData.loadGameIndex, FALSE },
  { "saveGameFile", ArgFilename, (LPVOID) &appData.saveGameFile, TRUE },
  { "sgf", ArgFilename, (LPVOID) &appData.saveGameFile, FALSE },
  { "autoSaveGames", ArgBoolean, (LPVOID) &appData.autoSaveGames, TRUE },
  { "autosave", ArgTrue, (LPVOID) &appData.autoSaveGames, FALSE },
  { "xautosave", ArgFalse, (LPVOID) &appData.autoSaveGames, FALSE },
  { "-autosave", ArgFalse, (LPVOID) &appData.autoSaveGames, FALSE },
  { "loadPositionFile", ArgFilename, (LPVOID) &appData.loadPositionFile, FALSE },
  { "lpf", ArgFilename, (LPVOID) &appData.loadPositionFile, FALSE },
  { "loadPositionIndex", ArgInt, (LPVOID) &appData.loadPositionIndex, FALSE },
  { "lpi", ArgInt, (LPVOID) &appData.loadPositionIndex, FALSE },
  { "savePositionFile", ArgFilename, (LPVOID) &appData.savePositionFile, FALSE },
  { "spf", ArgFilename, (LPVOID) &appData.savePositionFile, FALSE },
  { "matchMode", ArgBoolean, (LPVOID) &appData.matchMode, FALSE },
  { "mm", ArgTrue, (LPVOID) &appData.matchMode, FALSE },
  { "xmm", ArgFalse, (LPVOID) &appData.matchMode, FALSE },
  { "-mm", ArgFalse, (LPVOID) &appData.matchMode, FALSE },
  { "matchGames", ArgInt, (LPVOID) &appData.matchGames, FALSE },
  { "mg", ArgInt, (LPVOID) &appData.matchGames, FALSE },
  { "monoMode", ArgBoolean, (LPVOID) &appData.monoMode, TRUE },
  { "mono", ArgTrue, (LPVOID) &appData.monoMode, FALSE },
  { "xmono", ArgFalse, (LPVOID) &appData.monoMode, FALSE },
  { "-mono", ArgFalse, (LPVOID) &appData.monoMode, FALSE },
  { "debugMode", ArgBoolean, (LPVOID) &appData.debugMode, FALSE },
  { "debug", ArgTrue, (LPVOID) &appData.debugMode, FALSE },
  { "xdebug", ArgFalse, (LPVOID) &appData.debugMode, FALSE },
  { "-debug", ArgFalse, (LPVOID) &appData.debugMode, FALSE },
  { "clockMode", ArgBoolean, (LPVOID) &appData.clockMode, FALSE },
  { "clock", ArgTrue, (LPVOID) &appData.clockMode, FALSE },
  { "xclock", ArgFalse, (LPVOID) &appData.clockMode, FALSE },
  { "-clock", ArgFalse, (LPVOID) &appData.clockMode, FALSE },
  { "searchTime", ArgString, (LPVOID) &appData.searchTime, FALSE },
  { "st", ArgString, (LPVOID) &appData.searchTime, FALSE },
  { "searchDepth", ArgInt, (LPVOID) &appData.searchDepth, FALSE },
  { "depth", ArgInt, (LPVOID) &appData.searchDepth, FALSE },
  { "showCoords", ArgBoolean, (LPVOID) &appData.showCoords, TRUE },
  { "coords", ArgTrue, (LPVOID) &appData.showCoords, FALSE },
  { "xcoords", ArgFalse, (LPVOID) &appData.showCoords, FALSE },
  { "-coords", ArgFalse, (LPVOID) &appData.showCoords, FALSE },
  { "showThinking", ArgBoolean, (LPVOID) &appData.showThinking, TRUE },
  { "thinking", ArgTrue, (LPVOID) &appData.showThinking, FALSE },
  { "xthinking", ArgFalse, (LPVOID) &appData.showThinking, FALSE },
  { "-thinking", ArgFalse, (LPVOID) &appData.showThinking, FALSE },
  { "ponderNextMove", ArgBoolean, (LPVOID) &appData.ponderNextMove, TRUE },
  { "ponder", ArgTrue, (LPVOID) &appData.ponderNextMove, FALSE },
  { "xponder", ArgFalse, (LPVOID) &appData.ponderNextMove, FALSE },
  { "-ponder", ArgFalse, (LPVOID) &appData.ponderNextMove, FALSE },
  { "periodicUpdates", ArgBoolean, (LPVOID) &appData.periodicUpdates, TRUE },
  { "periodic", ArgTrue, (LPVOID) &appData.periodicUpdates, FALSE },
  { "xperiodic", ArgFalse, (LPVOID) &appData.periodicUpdates, FALSE },
  { "-periodic", ArgFalse, (LPVOID) &appData.periodicUpdates, FALSE },
  { "popupExitMessage", ArgBoolean, (LPVOID) &appData.popupExitMessage, TRUE },
  { "exit", ArgTrue, (LPVOID) &appData.popupExitMessage, FALSE },
  { "xexit", ArgFalse, (LPVOID) &appData.popupExitMessage, FALSE },
  { "-exit", ArgFalse, (LPVOID) &appData.popupExitMessage, FALSE },
  { "popupMoveErrors", ArgBoolean, (LPVOID) &appData.popupMoveErrors, TRUE },
  { "popup", ArgTrue, (LPVOID) &appData.popupMoveErrors, FALSE },
  { "xpopup", ArgFalse, (LPVOID) &appData.popupMoveErrors, FALSE },
  { "-popup", ArgFalse, (LPVOID) &appData.popupMoveErrors, FALSE },
  { "popUpErrors", ArgBoolean, (LPVOID) &appData.popupMoveErrors, 
    FALSE }, /* only so that old WinBoard.ini files from betas can be read */
  { "clockFont", ArgFont, (LPVOID) CLOCK_FONT, TRUE },
  { "messageFont", ArgFont, (LPVOID) MESSAGE_FONT, TRUE },
  { "coordFont", ArgFont, (LPVOID) COORD_FONT, TRUE },
  { "tagsFont", ArgFont, (LPVOID) EDITTAGS_FONT, TRUE },
  { "commentFont", ArgFont, (LPVOID) COMMENT_FONT, TRUE },
  { "icsFont", ArgFont, (LPVOID) CONSOLE_FONT, TRUE },
  { "moveHistoryFont", ArgFont, (LPVOID) MOVEHISTORY_FONT, TRUE }, /* [AS] */
  { "boardSize", ArgBoardSize, (LPVOID) &boardSize,
    TRUE }, /* must come after all fonts */
  { "size", ArgBoardSize, (LPVOID) &boardSize, FALSE },
  { "ringBellAfterMoves", ArgBoolean, (LPVOID) &appData.ringBellAfterMoves,
    FALSE }, /* historical; kept only so old winboard.ini files will parse */
  { "alwaysOnTop", ArgBoolean, (LPVOID) &alwaysOnTop, TRUE },
  { "top", ArgTrue, (LPVOID) &alwaysOnTop, FALSE },
  { "xtop", ArgFalse, (LPVOID) &alwaysOnTop, FALSE },
  { "-top", ArgFalse, (LPVOID) &alwaysOnTop, FALSE },
  { "autoCallFlag", ArgBoolean, (LPVOID) &appData.autoCallFlag, TRUE },
  { "autoflag", ArgTrue, (LPVOID) &appData.autoCallFlag, FALSE },
  { "xautoflag", ArgFalse, (LPVOID) &appData.autoCallFlag, FALSE },
  { "-autoflag", ArgFalse, (LPVOID) &appData.autoCallFlag, FALSE },
  { "autoComment", ArgBoolean, (LPVOID) &appData.autoComment, TRUE },
  { "autocomm", ArgTrue, (LPVOID) &appData.autoComment, FALSE },
  { "xautocomm", ArgFalse, (LPVOID) &appData.autoComment, FALSE },
  { "-autocomm", ArgFalse, (LPVOID) &appData.autoComment, FALSE },
  { "autoObserve", ArgBoolean, (LPVOID) &appData.autoObserve, TRUE },
  { "autobs", ArgTrue, (LPVOID) &appData.autoObserve, FALSE },
  { "xautobs", ArgFalse, (LPVOID) &appData.autoObserve, FALSE },
  { "-autobs", ArgFalse, (LPVOID) &appData.autoObserve, FALSE },
  { "flipView", ArgBoolean, (LPVOID) &appData.flipView, FALSE },
  { "flip", ArgTrue, (LPVOID) &appData.flipView, FALSE },
  { "xflip", ArgFalse, (LPVOID) &appData.flipView, FALSE },
  { "-flip", ArgFalse, (LPVOID) &appData.flipView, FALSE },
  { "autoFlipView", ArgBoolean, (LPVOID) &appData.autoFlipView, TRUE },
  { "autoflip", ArgTrue, (LPVOID) &appData.autoFlipView, FALSE },
  { "xautoflip", ArgFalse, (LPVOID) &appData.autoFlipView, FALSE },
  { "-autoflip", ArgFalse, (LPVOID) &appData.autoFlipView, FALSE },
  { "autoRaiseBoard", ArgBoolean, (LPVOID) &appData.autoRaiseBoard, TRUE },
  { "autoraise", ArgTrue, (LPVOID) &appData.autoRaiseBoard, FALSE },
  { "xautoraise", ArgFalse, (LPVOID) &appData.autoRaiseBoard, FALSE },
  { "-autoraise", ArgFalse, (LPVOID) &appData.autoRaiseBoard, FALSE },
#if 0
  { "cmailGameName", ArgString, (LPVOID) &appData.cmailGameName, FALSE },
  { "cmail", ArgString, (LPVOID) &appData.cmailGameName, FALSE },
#endif
  { "alwaysPromoteToQueen", ArgBoolean, (LPVOID) &appData.alwaysPromoteToQueen, TRUE },
  { "queen", ArgTrue, (LPVOID) &appData.alwaysPromoteToQueen, FALSE },
  { "xqueen", ArgFalse, (LPVOID) &appData.alwaysPromoteToQueen, FALSE },
  { "-queen", ArgFalse, (LPVOID) &appData.alwaysPromoteToQueen, FALSE },
  { "oldSaveStyle", ArgBoolean, (LPVOID) &appData.oldSaveStyle, TRUE },
  { "oldsave", ArgTrue, (LPVOID) &appData.oldSaveStyle, FALSE },
  { "xoldsave", ArgFalse, (LPVOID) &appData.oldSaveStyle, FALSE },
  { "-oldsave", ArgFalse, (LPVOID) &appData.oldSaveStyle, FALSE },
  { "quietPlay", ArgBoolean, (LPVOID) &appData.quietPlay, TRUE },
  { "quiet", ArgTrue, (LPVOID) &appData.quietPlay, FALSE },
  { "xquiet", ArgFalse, (LPVOID) &appData.quietPlay, FALSE },
  { "-quiet", ArgFalse, (LPVOID) &appData.quietPlay, FALSE },
  { "getMoveList", ArgBoolean, (LPVOID) &appData.getMoveList, TRUE },
  { "moves", ArgTrue, (LPVOID) &appData.getMoveList, FALSE },
  { "xmoves", ArgFalse, (LPVOID) &appData.getMoveList, FALSE },
  { "-moves", ArgFalse, (LPVOID) &appData.getMoveList, FALSE },
  { "testLegality", ArgBoolean, (LPVOID) &appData.testLegality, TRUE },
  { "legal", ArgTrue, (LPVOID) &appData.testLegality, FALSE },
  { "xlegal", ArgFalse, (LPVOID) &appData.testLegality, FALSE },
  { "-legal", ArgFalse, (LPVOID) &appData.testLegality, FALSE },
  { "premove", ArgBoolean, (LPVOID) &appData.premove, TRUE },
  { "pre", ArgTrue, (LPVOID) &appData.premove, FALSE },
  { "xpre", ArgFalse, (LPVOID) &appData.premove, FALSE },
  { "-pre", ArgFalse, (LPVOID) &appData.premove, FALSE },
  { "premoveWhite", ArgBoolean, (LPVOID) &appData.premoveWhite, TRUE },
  { "prewhite", ArgTrue, (LPVOID) &appData.premoveWhite, FALSE },
  { "xprewhite", ArgFalse, (LPVOID) &appData.premoveWhite, FALSE },
  { "-prewhite", ArgFalse, (LPVOID) &appData.premoveWhite, FALSE },
  { "premoveWhiteText", ArgString, (LPVOID) &appData.premoveWhiteText, TRUE },
  { "premoveBlack", ArgBoolean, (LPVOID) &appData.premoveBlack, TRUE },
  { "preblack", ArgTrue, (LPVOID) &appData.premoveBlack, FALSE },
  { "xpreblack", ArgFalse, (LPVOID) &appData.premoveBlack, FALSE },
  { "-preblack", ArgFalse, (LPVOID) &appData.premoveBlack, FALSE },
  { "premoveBlackText", ArgString, (LPVOID) &appData.premoveBlackText, TRUE },
  { "icsAlarm", ArgBoolean, (LPVOID) &appData.icsAlarm, TRUE},
  { "alarm", ArgTrue, (LPVOID) &appData.icsAlarm, FALSE},
  { "xalarm", ArgFalse, (LPVOID) &appData.icsAlarm, FALSE},
  { "-alarm", ArgFalse, (LPVOID) &appData.icsAlarm, FALSE},
  { "icsAlarmTime", ArgInt, (LPVOID) &appData.icsAlarmTime, TRUE},
  { "localLineEditing", ArgBoolean, (LPVOID) &appData.localLineEditing, FALSE},
  { "localLineEditing", ArgBoolean, (LPVOID) &appData.localLineEditing, FALSE},
  { "edit", ArgTrue, (LPVOID) &appData.localLineEditing, FALSE },
  { "xedit", ArgFalse, (LPVOID) &appData.localLineEditing, FALSE },
  { "-edit", ArgFalse, (LPVOID) &appData.localLineEditing, FALSE },
  { "animateMoving", ArgBoolean, (LPVOID) &appData.animate, TRUE },
  { "animate", ArgTrue, (LPVOID) &appData.animate, FALSE },
  { "xanimate", ArgFalse, (LPVOID) &appData.animate, FALSE },
  { "-animate", ArgFalse, (LPVOID) &appData.animate, FALSE },
  { "animateSpeed", ArgInt, (LPVOID) &appData.animSpeed, TRUE },
  { "animateDragging", ArgBoolean, (LPVOID) &appData.animateDragging, TRUE },
  { "drag", ArgTrue, (LPVOID) &appData.animateDragging, FALSE },
  { "xdrag", ArgFalse, (LPVOID) &appData.animateDragging, FALSE },
  { "-drag", ArgFalse, (LPVOID) &appData.animateDragging, FALSE },
  { "blindfold", ArgBoolean, (LPVOID) &appData.blindfold, TRUE },
  { "blind", ArgTrue, (LPVOID) &appData.blindfold, FALSE },
  { "xblind", ArgFalse, (LPVOID) &appData.blindfold, FALSE },
  { "-blind", ArgFalse, (LPVOID) &appData.blindfold, FALSE },
  { "highlightLastMove", ArgBoolean,
    (LPVOID) &appData.highlightLastMove, TRUE },
  { "highlight", ArgTrue, (LPVOID) &appData.highlightLastMove, FALSE },
  { "xhighlight", ArgFalse, (LPVOID) &appData.highlightLastMove, FALSE },
  { "-highlight", ArgFalse, (LPVOID) &appData.highlightLastMove, FALSE },
  { "highlightDragging", ArgBoolean,
    (LPVOID) &appData.highlightDragging, TRUE },
  { "highdrag", ArgTrue, (LPVOID) &appData.highlightDragging, FALSE },
  { "xhighdrag", ArgFalse, (LPVOID) &appData.highlightDragging, FALSE },
  { "-highdrag", ArgFalse, (LPVOID) &appData.highlightDragging, FALSE },
  { "colorizeMessages", ArgBoolean, (LPVOID) &appData.colorize, TRUE },
  { "colorize", ArgTrue, (LPVOID) &appData.colorize, FALSE },
  { "xcolorize", ArgFalse, (LPVOID) &appData.colorize, FALSE },
  { "-colorize", ArgFalse, (LPVOID) &appData.colorize, FALSE },
  { "colorShout", ArgAttribs, (LPVOID) ColorShout, TRUE },
  { "colorSShout", ArgAttribs, (LPVOID) ColorSShout, TRUE },
  { "colorChannel1", ArgAttribs, (LPVOID) ColorChannel1, TRUE },
  { "colorChannel", ArgAttribs, (LPVOID) ColorChannel, TRUE },
  { "colorKibitz", ArgAttribs, (LPVOID) ColorKibitz, TRUE },
  { "colorTell", ArgAttribs, (LPVOID) ColorTell, TRUE },
  { "colorChallenge", ArgAttribs, (LPVOID) ColorChallenge, TRUE },
  { "colorRequest", ArgAttribs, (LPVOID) ColorRequest, TRUE },
  { "colorSeek", ArgAttribs, (LPVOID) ColorSeek, TRUE },
  { "colorNormal", ArgAttribs, (LPVOID) ColorNormal, TRUE },
  { "colorBackground", ArgColor, (LPVOID) &consoleBackgroundColor, TRUE },
  { "soundShout", ArgFilename,
    (LPVOID) &textAttribs[ColorShout].sound.name, TRUE },
  { "soundSShout", ArgFilename,
    (LPVOID) &textAttribs[ColorSShout].sound.name, TRUE },
  { "soundChannel1", ArgFilename,
    (LPVOID) &textAttribs[ColorChannel1].sound.name, TRUE },
  { "soundChannel", ArgFilename,
    (LPVOID) &textAttribs[ColorChannel].sound.name, TRUE },
  { "soundKibitz", ArgFilename,
    (LPVOID) &textAttribs[ColorKibitz].sound.name, TRUE },
  { "soundTell", ArgFilename,
    (LPVOID) &textAttribs[ColorTell].sound.name, TRUE },
  { "soundChallenge", ArgFilename,
    (LPVOID) &textAttribs[ColorChallenge].sound.name, TRUE },
  { "soundRequest", ArgFilename,
    (LPVOID) &textAttribs[ColorRequest].sound.name, TRUE },
  { "soundSeek", ArgFilename,
    (LPVOID) &textAttribs[ColorSeek].sound.name, TRUE },
  { "soundMove", ArgFilename, (LPVOID) &sounds[(int)SoundMove].name, TRUE },
  { "soundBell", ArgFilename, (LPVOID) &sounds[(int)SoundBell].name, TRUE },
  { "soundIcsWin", ArgFilename, (LPVOID) &sounds[(int)SoundIcsWin].name,TRUE },
  { "soundIcsLoss", ArgFilename, 
    (LPVOID) &sounds[(int)SoundIcsLoss].name, TRUE },
  { "soundIcsDraw", ArgFilename, 
    (LPVOID) &sounds[(int)SoundIcsDraw].name, TRUE },
  { "soundIcsUnfinished", ArgFilename, 
    (LPVOID) &sounds[(int)SoundIcsUnfinished].name, TRUE},
  { "soundIcsAlarm", ArgFilename, 
    (LPVOID) &sounds[(int)SoundAlarm].name, TRUE },
  { "reuseFirst", ArgBoolean, (LPVOID) &appData.reuseFirst, FALSE },
  { "reuse", ArgTrue, (LPVOID) &appData.reuseFirst, FALSE },
  { "xreuse", ArgFalse, (LPVOID) &appData.reuseFirst, FALSE },
  { "-reuse", ArgFalse, (LPVOID) &appData.reuseFirst, FALSE },
  { "reuseChessPrograms", ArgBoolean,
    (LPVOID) &appData.reuseFirst, FALSE }, /* backward compat only */
  { "reuseSecond", ArgBoolean, (LPVOID) &appData.reuseSecond, FALSE },
  { "reuse2", ArgTrue, (LPVOID) &appData.reuseSecond, FALSE },
  { "xreuse2", ArgFalse, (LPVOID) &appData.reuseSecond, FALSE },
  { "-reuse2", ArgFalse, (LPVOID) &appData.reuseSecond, FALSE },
  { "comPortSettings", ArgCommSettings, (LPVOID) &dcb, TRUE },
  { "x", ArgInt, (LPVOID) &boardX, TRUE },
  { "y", ArgInt, (LPVOID) &boardY, TRUE },
  { "icsX", ArgInt, (LPVOID) &consoleX, TRUE },
  { "icsY", ArgInt, (LPVOID) &consoleY, TRUE },
  { "icsW", ArgInt, (LPVOID) &consoleW, TRUE },
  { "icsH", ArgInt, (LPVOID) &consoleH, TRUE },
  { "analysisX", ArgInt, (LPVOID) &analysisX, TRUE },
  { "analysisY", ArgInt, (LPVOID) &analysisY, TRUE },
  { "analysisW", ArgInt, (LPVOID) &analysisW, TRUE },
  { "analysisH", ArgInt, (LPVOID) &analysisH, TRUE },
  { "commentX", ArgInt, (LPVOID) &commentX, TRUE },
  { "commentY", ArgInt, (LPVOID) &commentY, TRUE },
  { "commentW", ArgInt, (LPVOID) &commentW, TRUE },
  { "commentH", ArgInt, (LPVOID) &commentH, TRUE },
  { "tagsX", ArgInt, (LPVOID) &editTagsX, TRUE },
  { "tagsY", ArgInt, (LPVOID) &editTagsY, TRUE },
  { "tagsW", ArgInt, (LPVOID) &editTagsW, TRUE },
  { "tagsH", ArgInt, (LPVOID) &editTagsH, TRUE },
  { "gameListX", ArgInt, (LPVOID) &gameListX, TRUE },
  { "gameListY", ArgInt, (LPVOID) &gameListY, TRUE },
  { "gameListW", ArgInt, (LPVOID) &gameListW, TRUE },
  { "gameListH", ArgInt, (LPVOID) &gameListH, TRUE },
  { "settingsFile", ArgSettingsFilename, (LPVOID) &settingsFileName, FALSE },
  { "ini", ArgSettingsFilename, (LPVOID) &settingsFileName, FALSE },
  { "saveSettingsOnExit", ArgBoolean, (LPVOID) &saveSettingsOnExit, TRUE },
  { "chessProgram", ArgBoolean, (LPVOID) &chessProgram, FALSE },
  { "cp", ArgTrue, (LPVOID) &chessProgram, FALSE },
  { "xcp", ArgFalse, (LPVOID) &chessProgram, FALSE },
  { "-cp", ArgFalse, (LPVOID) &chessProgram, FALSE },
  { "icsMenu", ArgString, (LPVOID) &icsTextMenuString, TRUE },
  { "icsNames", ArgString, (LPVOID) &icsNames, TRUE },
  { "firstChessProgramNames", ArgString, (LPVOID) &firstChessProgramNames,
    TRUE },
  { "secondChessProgramNames", ArgString, (LPVOID) &secondChessProgramNames,
    TRUE },
  { "initialMode", ArgString, (LPVOID) &appData.initialMode, FALSE },
  { "mode", ArgString, (LPVOID) &appData.initialMode, FALSE },
  { "variant", ArgString, (LPVOID) &appData.variant, FALSE },
  { "firstProtocolVersion", ArgInt, (LPVOID) &appData.firstProtocolVersion, FALSE },
  { "secondProtocolVersion", ArgInt, (LPVOID) &appData.secondProtocolVersion,FALSE },
  { "showButtonBar", ArgBoolean, (LPVOID) &appData.showButtonBar, TRUE },
  { "buttons", ArgTrue, (LPVOID) &appData.showButtonBar, FALSE },
  { "xbuttons", ArgFalse, (LPVOID) &appData.showButtonBar, FALSE },
  { "-buttons", ArgFalse, (LPVOID) &appData.showButtonBar, FALSE },
  /* [AS] New features */
  { "firstScoreAbs", ArgBoolean, (LPVOID) &appData.firstScoreIsAbsolute, FALSE },
  { "secondScoreAbs", ArgBoolean, (LPVOID) &appData.secondScoreIsAbsolute, FALSE },
  { "pgnExtendedInfo", ArgBoolean, (LPVOID) &appData.saveExtendedInfoInPGN, TRUE },
  { "hideThinkingFromHuman", ArgBoolean, (LPVOID) &appData.hideThinkingFromHuman, TRUE },
  { "liteBackTextureFile", ArgString, (LPVOID) &appData.liteBackTextureFile, TRUE },
  { "darkBackTextureFile", ArgString, (LPVOID) &appData.darkBackTextureFile, TRUE },
  { "liteBackTextureMode", ArgInt, (LPVOID) &appData.liteBackTextureMode, TRUE },
  { "darkBackTextureMode", ArgInt, (LPVOID) &appData.darkBackTextureMode, TRUE },
  { "renderPiecesWithFont", ArgString, (LPVOID) &appData.renderPiecesWithFont, TRUE },
  { "fontPieceToCharTable", ArgString, (LPVOID) &appData.fontToPieceTable, TRUE },
  { "fontPieceBackColorWhite", ArgColor, (LPVOID) &appData.fontBackColorWhite, TRUE },
  { "fontPieceForeColorWhite", ArgColor, (LPVOID) &appData.fontForeColorWhite, TRUE },
  { "fontPieceBackColorBlack", ArgColor, (LPVOID) &appData.fontBackColorBlack, TRUE },
  { "fontPieceForeColorBlack", ArgColor, (LPVOID) &appData.fontForeColorBlack, TRUE },
  { "fontPieceSize", ArgInt, (LPVOID) &appData.fontPieceSize, TRUE },
  { "overrideLineGap", ArgInt, (LPVOID) &appData.overrideLineGap, TRUE },
  { "adjudicateLossThreshold", ArgInt, (LPVOID) &appData.adjudicateLossThreshold, TRUE },
  { "delayBeforeQuit", ArgInt, (LPVOID) &appData.delayBeforeQuit, TRUE },
  { "delayAfterQuit", ArgInt, (LPVOID) &appData.delayAfterQuit, TRUE },
  { "nameOfDebugFile", ArgFilename, (LPVOID) &appData.nameOfDebugFile, FALSE },
  { "debugfile", ArgFilename, (LPVOID) &appData.nameOfDebugFile, FALSE },
  { "pgnEventHeader", ArgString, (LPVOID) &appData.pgnEventHeader, TRUE },
  { "defaultFrcPosition", ArgInt, (LPVOID) &appData.defaultFrcPosition, TRUE },
  { "gameListTags", ArgString, (LPVOID) &appData.gameListTags, TRUE },
  { "saveOutOfBookInfo", ArgBoolean, (LPVOID) &appData.saveOutOfBookInfo, TRUE },
  { "showEvalInMoveHistory", ArgBoolean, (LPVOID) &appData.showEvalInMoveHistory, TRUE },
  { "evalHistColorWhite", ArgColor, (LPVOID) &appData.evalHistColorWhite, TRUE },
  { "evalHistColorBlack", ArgColor, (LPVOID) &appData.evalHistColorBlack, TRUE },
  { "highlightMoveWithArrow", ArgBoolean, (LPVOID) &appData.highlightMoveWithArrow, TRUE },
  { "highlightArrowColor", ArgColor, (LPVOID) &appData.highlightArrowColor, TRUE },
  { "stickyWindows", ArgBoolean, (LPVOID) &appData.useStickyWindows, TRUE },
  { "adjudicateDrawMoves", ArgInt, (LPVOID) &appData.adjudicateDrawMoves, TRUE },
  { "autoDisplayComment", ArgBoolean, (LPVOID) &appData.autoDisplayComment, TRUE },
  { "autoDisplayTags", ArgBoolean, (LPVOID) &appData.autoDisplayTags, TRUE },
  { "firstIsUCI", ArgBoolean, (LPVOID) &appData.firstIsUCI, FALSE },
  { "fUCI", ArgTrue, (LPVOID) &appData.firstIsUCI, FALSE },
  { "secondIsUCI", ArgBoolean, (LPVOID) &appData.secondIsUCI, FALSE },
  { "sUCI", ArgTrue, (LPVOID) &appData.secondIsUCI, FALSE },
  { "firstHasOwnBookUCI", ArgBoolean, (LPVOID) &appData.firstHasOwnBookUCI, FALSE },
  { "fNoOwnBookUCI", ArgFalse, (LPVOID) &appData.firstHasOwnBookUCI, FALSE },
  { "secondHasOwnBookUCI", ArgBoolean, (LPVOID) &appData.secondHasOwnBookUCI, FALSE },
  { "sNoOwnBookUCI", ArgFalse, (LPVOID) &appData.secondHasOwnBookUCI, FALSE },
  { "polyglotDir", ArgFilename, (LPVOID) &appData.polyglotDir, TRUE },
  { "usePolyglotBook", ArgBoolean, (LPVOID) &appData.usePolyglotBook, TRUE },
  { "polyglotBook", ArgFilename, (LPVOID) &appData.polyglotBook, TRUE },
  { "defaultHashSize", ArgInt, (LPVOID) &appData.defaultHashSize, TRUE }, 
  { "defaultCacheSizeEGTB", ArgInt, (LPVOID) &appData.defaultCacheSizeEGTB, TRUE },
  { "defaultPathEGTB", ArgFilename, (LPVOID) &appData.defaultPathEGTB, TRUE },

  /* [AS] Layout stuff */
  { "moveHistoryUp", ArgBoolean, (LPVOID) &wpMoveHistory.visible, TRUE },
  { "moveHistoryX", ArgInt, (LPVOID) &wpMoveHistory.x, TRUE },
  { "moveHistoryY", ArgInt, (LPVOID) &wpMoveHistory.y, TRUE },
  { "moveHistoryW", ArgInt, (LPVOID) &wpMoveHistory.width, TRUE },
  { "moveHistoryH", ArgInt, (LPVOID) &wpMoveHistory.height, TRUE },

  { "evalGraphUp", ArgBoolean, (LPVOID) &wpEvalGraph.visible, TRUE },
  { "evalGraphX", ArgInt, (LPVOID) &wpEvalGraph.x, TRUE },
  { "evalGraphY", ArgInt, (LPVOID) &wpEvalGraph.y, TRUE },
  { "evalGraphW", ArgInt, (LPVOID) &wpEvalGraph.width, TRUE },
  { "evalGraphH", ArgInt, (LPVOID) &wpEvalGraph.height, TRUE },

  { "engineOutputUp", ArgBoolean, (LPVOID) &wpEngineOutput.visible, TRUE },
  { "engineOutputX", ArgInt, (LPVOID) &wpEngineOutput.x, TRUE },
  { "engineOutputY", ArgInt, (LPVOID) &wpEngineOutput.y, TRUE },
  { "engineOutputW", ArgInt, (LPVOID) &wpEngineOutput.width, TRUE },
  { "engineOutputH", ArgInt, (LPVOID) &wpEngineOutput.height, TRUE },

  /* [HGM] board-size, adjudication and misc. options */
  { "boardWidth", ArgInt, (LPVOID) &appData.NrFiles, TRUE },
  { "boardHeight", ArgInt, (LPVOID) &appData.NrRanks, TRUE },
  { "holdingsSize", ArgInt, (LPVOID) &appData.holdingsSize, TRUE },
  { "matchPause", ArgInt, (LPVOID) &appData.matchPause, TRUE },
  { "pieceToCharTable", ArgString, (LPVOID) &appData.pieceToCharTable, FALSE },
  { "flipBlack", ArgBoolean, (LPVOID) &appData.allWhite, TRUE },
  { "allWhite", ArgBoolean, (LPVOID) &appData.allWhite, TRUE },
  { "alphaRank", ArgBoolean, (LPVOID) &appData.alphaRank, FALSE },
  { "testClaims", ArgBoolean, (LPVOID) &appData.testClaims, TRUE },
  { "checkMates", ArgBoolean, (LPVOID) &appData.checkMates, TRUE },
  { "materialDraws", ArgBoolean, (LPVOID) &appData.materialDraws, TRUE },
  { "trivialDraws", ArgBoolean, (LPVOID) &appData.trivialDraws, TRUE },
  { "ruleMoves", ArgInt, (LPVOID) &appData.ruleMoves, TRUE },
  { "repeatsToDraw", ArgInt, (LPVOID) &appData.drawRepeats, TRUE },

#ifdef ZIPPY
  { "zippyTalk", ArgBoolean, (LPVOID) &appData.zippyTalk, FALSE },
  { "zt", ArgTrue, (LPVOID) &appData.zippyTalk, FALSE },
  { "xzt", ArgFalse, (LPVOID) &appData.zippyTalk, FALSE },
  { "-zt", ArgFalse, (LPVOID) &appData.zippyTalk, FALSE },
  { "zippyPlay", ArgBoolean, (LPVOID) &appData.zippyPlay, FALSE },
  { "zp", ArgTrue, (LPVOID) &appData.zippyPlay, FALSE },
  { "xzp", ArgFalse, (LPVOID) &appData.zippyPlay, FALSE },
  { "-zp", ArgFalse, (LPVOID) &appData.zippyPlay, FALSE },
  { "zippyLines", ArgFilename, (LPVOID) &appData.zippyLines, FALSE },
  { "zippyPinhead", ArgString, (LPVOID) &appData.zippyPinhead, FALSE },
  { "zippyPassword", ArgString, (LPVOID) &appData.zippyPassword, FALSE },
  { "zippyPassword2", ArgString, (LPVOID) &appData.zippyPassword2, FALSE },
  { "zippyWrongPassword", ArgString, (LPVOID) &appData.zippyWrongPassword,
    FALSE },
  { "zippyAcceptOnly", ArgString, (LPVOID) &appData.zippyAcceptOnly, FALSE },
  { "zippyUseI", ArgBoolean, (LPVOID) &appData.zippyUseI, FALSE },
  { "zui", ArgTrue, (LPVOID) &appData.zippyUseI, FALSE },
  { "xzui", ArgFalse, (LPVOID) &appData.zippyUseI, FALSE },
  { "-zui", ArgFalse, (LPVOID) &appData.zippyUseI, FALSE },
  { "zippyBughouse", ArgInt, (LPVOID) &appData.zippyBughouse, FALSE },
  { "zippyNoplayCrafty", ArgBoolean, (LPVOID) &appData.zippyNoplayCrafty,
    FALSE },
  { "znc", ArgTrue, (LPVOID) &appData.zippyNoplayCrafty, FALSE },
  { "xznc", ArgFalse, (LPVOID) &appData.zippyNoplayCrafty, FALSE },
  { "-znc", ArgFalse, (LPVOID) &appData.zippyNoplayCrafty, FALSE },
  { "zippyGameEnd", ArgString, (LPVOID) &appData.zippyGameEnd, FALSE },
  { "zippyGameStart", ArgString, (LPVOID) &appData.zippyGameStart, FALSE },
  { "zippyAdjourn", ArgBoolean, (LPVOID) &appData.zippyAdjourn, FALSE },
  { "zadj", ArgTrue, (LPVOID) &appData.zippyAdjourn, FALSE },
  { "xzadj", ArgFalse, (LPVOID) &appData.zippyAdjourn, FALSE },
  { "-zadj", ArgFalse, (LPVOID) &appData.zippyAdjourn, FALSE },
  { "zippyAbort", ArgBoolean, (LPVOID) &appData.zippyAbort, FALSE },
  { "zab", ArgTrue, (LPVOID) &appData.zippyAbort, FALSE },
  { "xzab", ArgFalse, (LPVOID) &appData.zippyAbort, FALSE },
  { "-zab", ArgFalse, (LPVOID) &appData.zippyAbort, FALSE },
  { "zippyVariants", ArgString, (LPVOID) &appData.zippyVariants, FALSE },
  { "zippyMaxGames", ArgInt, (LPVOID)&appData.zippyMaxGames, FALSE },
  { "zippyReplayTimeout", ArgInt, (LPVOID)&appData.zippyReplayTimeout, FALSE },
  /* Kludge to allow winboard.ini files from buggy 4.0.4 to be read: */
  { "zippyReplyTimeout", ArgInt, (LPVOID)&junk, FALSE },
#endif
  { NULL, ArgNone, NULL, FALSE }
};


/* Kludge for indirection files on command line */
char* lastIndirectionFilename;
ArgDescriptor argDescriptorIndirection =
{ "", ArgSettingsFilename, (LPVOID) NULL, FALSE };


VOID
ExitArgError(char *msg, char *badArg)
{
  char buf[MSG_SIZ];

  sprintf(buf, "%s %s", msg, badArg);
  DisplayFatalError(buf, 0, 2);
  exit(2);
}

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
      ExitArgError("Font name too long:", name);
    memcpy(mfp->faceName, p, q - p);
    mfp->faceName[q - p] = NULLCHAR;
    p = q + 1;
  } else {
    q = mfp->faceName;
    while (*p && !isdigit(*p)) {
      *q++ = *p++;
      if (q - mfp->faceName >= sizeof(mfp->faceName))
	ExitArgError("Font name too long:", name);
    }
    while (q > mfp->faceName && q[-1] == ' ') q--;
    *q = NULLCHAR;
  }
  if (!*p) ExitArgError("Font point size missing:", name);
  mfp->pointSize = (float) atof(p);
  mfp->bold = (strchr(p, 'b') != NULL);
  mfp->italic = (strchr(p, 'i') != NULL);
  mfp->underline = (strchr(p, 'u') != NULL);
  mfp->strikeout = (strchr(p, 's') != NULL);
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
    sprintf(buf, "Can't parse color name %s", name);
    DisplayError(buf, 0);
    return RGB(0, 0, 0);
  }
  return PALETTERGB(red, green, blue);
}


void ParseAttribs(COLORREF *color, int *effects, char* argValue)
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


BoardSize
ParseBoardSize(char *name)
{
  BoardSize bs = SizeTiny;
  while (sizeInfo[bs].name != NULL) {
    if (StrCaseCmp(name, sizeInfo[bs].name) == 0) return bs;
    bs++;
  }
  ExitArgError("Unrecognized board size value", name);
  return bs; /* not reached */
}


char
StringGet(void *getClosure)
{
  char **p = (char **) getClosure;
  return *((*p)++);
}

char
FileGet(void *getClosure)
{
  int c;
  FILE* f = (FILE*) getClosure;

  c = getc(f);
  if (c == EOF)
    return NULLCHAR;
  else
    return (char) c;
}

/* Parse settings file named "name". If file found, return the
   full name in fullname and return TRUE; else return FALSE */
BOOLEAN
ParseSettingsFile(char *name, char fullname[MSG_SIZ])
{
  char *dummy;
  FILE *f;

  if (SearchPath(installDir, name, NULL, MSG_SIZ, fullname, &dummy)) {
    f = fopen(fullname, "r");
    if (f != NULL) {
      ParseArgs(FileGet, f);
      fclose(f);
      return TRUE;
    }
  }
  return FALSE;
}

VOID
ParseArgs(GetFunc get, void *cl)
{
  char argName[ARG_MAX];
  char argValue[ARG_MAX];
  ArgDescriptor *ad;
  char start;
  char *q;
  int i, octval;
  char ch;
  int posarg = 0;

  ch = get(cl);
  for (;;) {
    while (ch == ' ' || ch == '\n' || ch == '\t') ch = get(cl);
    if (ch == NULLCHAR) break;
    if (ch == ';') {
      /* Comment to end of line */
      ch = get(cl);
      while (ch != '\n' && ch != NULLCHAR) ch = get(cl);
      continue;
    } else if (ch == '/' || ch == '-') {
      /* Switch */
      q = argName;
      while (ch != ' ' && ch != '=' && ch != ':' && ch != NULLCHAR &&
	     ch != '\n' && ch != '\t') {
	*q++ = ch;
	ch = get(cl);
      }
      *q = NULLCHAR;

      for (ad = argDescriptors; ad->argName != NULL; ad++)
	if (strcmp(ad->argName, argName + 1) == 0) break;

      if (ad->argName == NULL)
	ExitArgError("Unrecognized argument", argName);

    } else if (ch == '@') {
      /* Indirection file */
      ad = &argDescriptorIndirection;
      ch = get(cl);
    } else {
      /* Positional argument */
      ad = &argDescriptors[posarg++];
      strcpy(argName, ad->argName);
    }

    if (ad->argType == ArgTrue) {
      *(Boolean *) ad->argLoc = TRUE;
      continue;
    }
    if (ad->argType == ArgFalse) {
      *(Boolean *) ad->argLoc = FALSE;
      continue;
    }

    while (ch == ' ' || ch == '=' || ch == ':' || ch == '\t') ch = get(cl);
    if (ch == NULLCHAR || ch == '\n') {
      ExitArgError("No value provided for argument", argName);
    }
    q = argValue;
    if (ch == '{') {
      // Quoting with { }.  No characters have to (or can) be escaped.
      // Thus the string cannot contain a '}' character.
      start = ch;
      ch = get(cl);
      while (start) {
	switch (ch) {
	case NULLCHAR:
	  start = NULLCHAR;
	  break;
	  
	case '}':
	  ch = get(cl);
	  start = NULLCHAR;
	  break;

	default:
	  *q++ = ch;
	  ch = get(cl);
	  break;
	}
      }	  
    } else if (ch == '\'' || ch == '"') {
      // Quoting with ' ' or " ", with \ as escape character.
      // Inconvenient for long strings that may contain Windows filenames.
      start = ch;
      ch = get(cl);
      while (start) {
	switch (ch) {
	case NULLCHAR:
	  start = NULLCHAR;
	  break;

	default:
        not_special:
	  *q++ = ch;
	  ch = get(cl);
	  break;

	case '\'':
	case '\"':
	  if (ch == start) {
	    ch = get(cl);
	    start = NULLCHAR;
	    break;
	  } else {
	    goto not_special;
	  }

	case '\\':
          if (ad->argType == ArgFilename
	      || ad->argType == ArgSettingsFilename) {
	      goto not_special;
	  }
	  ch = get(cl);
	  switch (ch) {
	  case NULLCHAR:
	    ExitArgError("Incomplete \\ escape in value for", argName);
	    break;
	  case 'n':
	    *q++ = '\n';
	    ch = get(cl);
	    break;
	  case 'r':
	    *q++ = '\r';
	    ch = get(cl);
	    break;
	  case 't':
	    *q++ = '\t';
	    ch = get(cl);
	    break;
	  case 'b':
	    *q++ = '\b';
	    ch = get(cl);
	    break;
	  case 'f':
	    *q++ = '\f';
	    ch = get(cl);
	    break;
	  default:
	    octval = 0;
	    for (i = 0; i < 3; i++) {
	      if (ch >= '0' && ch <= '7') {
		octval = octval*8 + (ch - '0');
		ch = get(cl);
	      } else {
		break;
	      }
	    }
	    if (i > 0) {
	      *q++ = (char) octval;
	    } else {
	      *q++ = ch;
	      ch = get(cl);
	    }
	    break;
	  }
	  break;
	}
      }
    } else {
      while (ch != ' ' && ch != NULLCHAR && ch != '\t' && ch != '\n') {
	*q++ = ch;
	ch = get(cl);
      }
    }
    *q = NULLCHAR;

    switch (ad->argType) {
    case ArgInt:
      *(int *) ad->argLoc = atoi(argValue);
      break;

    case ArgFloat:
      *(float *) ad->argLoc = (float) atof(argValue);
      break;

    case ArgString:
    case ArgFilename:
      *(char **) ad->argLoc = strdup(argValue);
      break;

    case ArgSettingsFilename:
      {
	char fullname[MSG_SIZ];
	if (ParseSettingsFile(argValue, fullname)) {
	  if (ad->argLoc != NULL) {
	    *(char **) ad->argLoc = strdup(fullname);
	  }
	} else {
	  if (ad->argLoc != NULL) {
	  } else {
	    ExitArgError("Failed to open indirection file", argValue);
	  }
	}
      }
      break;

    case ArgBoolean:
      switch (argValue[0]) {
      case 't':
      case 'T':
	*(Boolean *) ad->argLoc = TRUE;
	break;
      case 'f':
      case 'F':
	*(Boolean *) ad->argLoc = FALSE;
	break;
      default:
	ExitArgError("Unrecognized boolean argument value", argValue);
	break;
      }
      break;

    case ArgColor:
      *(COLORREF *)ad->argLoc = ParseColorName(argValue);
      break;

    case ArgAttribs: {
      ColorClass cc = (ColorClass)ad->argLoc;
      ParseAttribs(&textAttribs[cc].color, &textAttribs[cc].effects, argValue);
      }
      break;
      
    case ArgBoardSize:
      *(BoardSize *)ad->argLoc = ParseBoardSize(argValue);
      break;

    case ArgFont:
      ParseFontName(argValue, &font[boardSize][(int)ad->argLoc]->mfp);
      break;

    case ArgCommSettings:
      ParseCommSettings(argValue, &dcb);
      break;

    case ArgNone:
      ExitArgError("Unrecognized argument", argValue);
      break;
    }
  }
}

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
  lf->lfCharSet = DEFAULT_CHARSET;
  lf->lfOutPrecision = OUT_DEFAULT_PRECIS;
  lf->lfClipPrecision = CLIP_DEFAULT_PRECIS;
  lf->lfQuality = DEFAULT_QUALITY;
  lf->lfPitchAndFamily = DEFAULT_PITCH|FF_DONTCARE;
  strcpy(lf->lfFaceName, mfp->faceName);
}

VOID
CreateFontInMF(MyFont *mf)
{
  LFfromMFP(&mf->lf, &mf->mfp);
  if (mf->hf) DeleteObject(mf->hf);
  mf->hf = CreateFontIndirect(&mf->lf);
}

VOID
SetDefaultTextAttribs()
{
  ColorClass cc;
  for (cc = (ColorClass)0; cc < NColorClasses; cc++) {
    ParseAttribs(&textAttribs[cc].color, 
		 &textAttribs[cc].effects, 
	         defaultTextAttribs[cc]);
  }
}

VOID
SetDefaultSounds()
{
  ColorClass cc;
  SoundClass sc;
  for (cc = (ColorClass)0; cc < NColorClasses; cc++) {
    textAttribs[cc].sound.name = strdup("");
    textAttribs[cc].sound.data = NULL;
  }
  for (sc = (SoundClass)0; sc < NSoundClasses; sc++) {
    sounds[sc].name = strdup("");
    sounds[sc].data = NULL;
  }
  sounds[(int)SoundBell].name = strdup(SOUND_BELL);
}

VOID
LoadAllSounds()
{
  ColorClass cc;
  SoundClass sc;
  for (cc = (ColorClass)0; cc < NColorClasses; cc++) {
    MyLoadSound(&textAttribs[cc].sound);
  }
  for (sc = (SoundClass)0; sc < NSoundClasses; sc++) {
    MyLoadSound(&sounds[sc]);
  }
}

VOID
InitAppData(LPSTR lpCmdLine)
{
  int i, j;
  char buf[ARG_MAX], currDir[MSG_SIZ];
  char *dummy, *p;

  programName = szAppName;

  /* Initialize to defaults */
  lightSquareColor = ParseColorName(LIGHT_SQUARE_COLOR);
  darkSquareColor = ParseColorName(DARK_SQUARE_COLOR);
  whitePieceColor = ParseColorName(WHITE_PIECE_COLOR);
  blackPieceColor = ParseColorName(BLACK_PIECE_COLOR);
  highlightSquareColor = ParseColorName(HIGHLIGHT_SQUARE_COLOR);
  premoveHighlightColor = ParseColorName(PREMOVE_HIGHLIGHT_COLOR);
  consoleBackgroundColor = ParseColorName(COLOR_BKGD);
  SetDefaultTextAttribs();
  SetDefaultSounds();
  appData.movesPerSession = MOVES_PER_SESSION;
  appData.initString = INIT_STRING;
  appData.secondInitString = INIT_STRING;
  appData.firstComputerString = COMPUTER_STRING;
  appData.secondComputerString = COMPUTER_STRING;
  appData.firstChessProgram = FIRST_CHESS_PROGRAM;
  appData.secondChessProgram = SECOND_CHESS_PROGRAM;
  appData.firstPlaysBlack = FALSE;
  appData.noChessProgram = FALSE;
  chessProgram = FALSE;
  appData.firstHost = FIRST_HOST;
  appData.secondHost = SECOND_HOST;
  appData.firstDirectory = FIRST_DIRECTORY;
  appData.secondDirectory = SECOND_DIRECTORY;
  appData.bitmapDirectory = "";
  appData.remoteShell = REMOTE_SHELL;
  appData.remoteUser = "";
  appData.timeDelay = TIME_DELAY;
  appData.timeControl = TIME_CONTROL;
  appData.timeIncrement = TIME_INCREMENT;
  appData.icsActive = FALSE;
  appData.icsHost = "";
  appData.icsPort = ICS_PORT;
  appData.icsCommPort = ICS_COMM_PORT;
  appData.icsLogon = ICS_LOGON;
  appData.icsHelper = "";
  appData.useTelnet = FALSE;
  appData.telnetProgram = TELNET_PROGRAM;
  appData.gateway = "";
  appData.loadGameFile = "";
  appData.loadGameIndex = 0;
  appData.saveGameFile = "";
  appData.autoSaveGames = FALSE;
  appData.loadPositionFile = "";
  appData.loadPositionIndex = 1;
  appData.savePositionFile = "";
  appData.matchMode = FALSE;
  appData.matchGames = 0;
  appData.monoMode = FALSE;
  appData.debugMode = FALSE;
  appData.clockMode = TRUE;
  boardSize = (BoardSize) -1; /* determine by screen size */
  appData.Iconic = FALSE; /*unused*/
  appData.searchTime = "";
  appData.searchDepth = 0;
  appData.showCoords = FALSE;
  appData.ringBellAfterMoves = TRUE; /*obsolete in WinBoard*/
  appData.autoCallFlag = FALSE;
  appData.flipView = FALSE;
  appData.autoFlipView = TRUE;
  appData.cmailGameName = "";
  appData.alwaysPromoteToQueen = FALSE;
  appData.oldSaveStyle = FALSE;
  appData.quietPlay = FALSE;
  appData.showThinking = FALSE;
  appData.ponderNextMove = TRUE;
  appData.periodicUpdates = TRUE;
  appData.popupExitMessage = TRUE;
  appData.popupMoveErrors = FALSE;
  appData.autoObserve = FALSE;
  appData.autoComment = FALSE;
  appData.animate = TRUE;
  appData.animSpeed = 10;
  appData.animateDragging = TRUE;
  appData.highlightLastMove = TRUE;
  appData.getMoveList = TRUE;
  appData.testLegality = TRUE;
  appData.premove = TRUE;
  appData.premoveWhite = FALSE;
  appData.premoveWhiteText = "";
  appData.premoveBlack = FALSE;
  appData.premoveBlackText = "";
  appData.icsAlarm = TRUE;
  appData.icsAlarmTime = 5000;
  appData.autoRaiseBoard = TRUE;
  appData.localLineEditing = TRUE;
  appData.colorize = TRUE;
  appData.reuseFirst = TRUE;
  appData.reuseSecond = TRUE;
  appData.blindfold = FALSE;
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
  dcb.wReserved = 0;
  dcb.ByteSize = 7;
  dcb.Parity = SPACEPARITY;
  dcb.StopBits = ONESTOPBIT;
  settingsFileName = SETTINGS_FILE;
  saveSettingsOnExit = TRUE;
  boardX = CW_USEDEFAULT;
  boardY = CW_USEDEFAULT;
  consoleX = CW_USEDEFAULT; 
  consoleY = CW_USEDEFAULT; 
  consoleW = CW_USEDEFAULT;
  consoleH = CW_USEDEFAULT;
  analysisX = CW_USEDEFAULT; 
  analysisY = CW_USEDEFAULT; 
  analysisW = CW_USEDEFAULT;
  analysisH = CW_USEDEFAULT;
  commentX = CW_USEDEFAULT; 
  commentY = CW_USEDEFAULT; 
  commentW = CW_USEDEFAULT;
  commentH = CW_USEDEFAULT;
  editTagsX = CW_USEDEFAULT; 
  editTagsY = CW_USEDEFAULT; 
  editTagsW = CW_USEDEFAULT;
  editTagsH = CW_USEDEFAULT;
  gameListX = CW_USEDEFAULT; 
  gameListY = CW_USEDEFAULT; 
  gameListW = CW_USEDEFAULT;
  gameListH = CW_USEDEFAULT;
  icsTextMenuString = ICS_TEXT_MENU_DEFAULT;
  icsNames = ICS_NAMES;
  firstChessProgramNames = FCP_NAMES;
  secondChessProgramNames = SCP_NAMES;
  appData.initialMode = "";
  appData.variant = "normal";
  appData.firstProtocolVersion = PROTOVER;
  appData.secondProtocolVersion = PROTOVER;
  appData.showButtonBar = TRUE;

   /* [AS] New properties (see comments in header file) */
  appData.firstScoreIsAbsolute = FALSE;
  appData.secondScoreIsAbsolute = FALSE;
  appData.saveExtendedInfoInPGN = FALSE;
  appData.hideThinkingFromHuman = FALSE;
  appData.liteBackTextureFile = "";
  appData.liteBackTextureMode = BACK_TEXTURE_MODE_PLAIN;
  appData.darkBackTextureFile = "";
  appData.darkBackTextureMode = BACK_TEXTURE_MODE_PLAIN;
  appData.renderPiecesWithFont = "";
  appData.fontToPieceTable = "";
  appData.fontBackColorWhite = 0;
  appData.fontForeColorWhite = 0;
  appData.fontBackColorBlack = 0;
  appData.fontForeColorBlack = 0;
  appData.fontPieceSize = 80;
  appData.overrideLineGap = 1;
  appData.adjudicateLossThreshold = 0;
  appData.delayBeforeQuit = 0;
  appData.delayAfterQuit = 0;
  appData.nameOfDebugFile = "winboard.debug";
  appData.pgnEventHeader = "Computer Chess Game";
  appData.defaultFrcPosition = -1;
  appData.gameListTags = GLT_DEFAULT_TAGS;
  appData.saveOutOfBookInfo = TRUE;
  appData.showEvalInMoveHistory = TRUE;
  appData.evalHistColorWhite = ParseColorName( "#FFFFB0" );
  appData.evalHistColorBlack = ParseColorName( "#AD5D3D" );
  appData.highlightMoveWithArrow = FALSE;
  appData.highlightArrowColor = ParseColorName( "#FFFF80" );
  appData.useStickyWindows = TRUE;
  appData.adjudicateDrawMoves = 0;
  appData.autoDisplayComment = TRUE;
  appData.autoDisplayTags = TRUE;
  appData.firstIsUCI = FALSE;
  appData.secondIsUCI = FALSE;
  appData.firstHasOwnBookUCI = TRUE;
  appData.secondHasOwnBookUCI = TRUE;
  appData.polyglotDir = "";
  appData.usePolyglotBook = FALSE;
  appData.polyglotBook = "";
  appData.defaultHashSize = 64;
  appData.defaultCacheSizeEGTB = 4;
  appData.defaultPathEGTB = "c:\\egtb";

  InitWindowPlacement( &wpMoveHistory );
  InitWindowPlacement( &wpEvalGraph );
  InitWindowPlacement( &wpEngineOutput );

  /* [HGM] User-selectable board size, adjudication control, miscellaneous */
  appData.NrFiles      = -1;
  appData.NrRanks      = -1;
  appData.holdingsSize = -1;
  appData.testClaims   = FALSE;
  appData.checkMates   = FALSE;
  appData.materialDraws= FALSE;
  appData.trivialDraws = FALSE;
  appData.ruleMoves    = 51;
  appData.drawRepeats  = 6;
  appData.matchPause   = 10000;
  appData.alphaRank    = FALSE;
  appData.allWhite     = FALSE;
  appData.upsideDown   = FALSE;

#ifdef ZIPPY
  appData.zippyTalk = ZIPPY_TALK;
  appData.zippyPlay = ZIPPY_PLAY;
  appData.zippyLines = ZIPPY_LINES;
  appData.zippyPinhead = ZIPPY_PINHEAD;
  appData.zippyPassword = ZIPPY_PASSWORD;
  appData.zippyPassword2 = ZIPPY_PASSWORD2;
  appData.zippyWrongPassword = ZIPPY_WRONG_PASSWORD;
  appData.zippyAcceptOnly = ZIPPY_ACCEPT_ONLY;
  appData.zippyUseI = ZIPPY_USE_I;
  appData.zippyBughouse = ZIPPY_BUGHOUSE;
  appData.zippyNoplayCrafty = ZIPPY_NOPLAY_CRAFTY;
  appData.zippyGameEnd = ZIPPY_GAME_END;
  appData.zippyGameStart = ZIPPY_GAME_START;
  appData.zippyAdjourn = ZIPPY_ADJOURN;
  appData.zippyAbort = ZIPPY_ABORT;
  appData.zippyVariants = ZIPPY_VARIANTS;
  appData.zippyMaxGames = ZIPPY_MAX_GAMES;
  appData.zippyReplayTimeout = ZIPPY_REPLAY_TIMEOUT;
#endif

  /* Point font array elements to structures and
     parse default font names */
  for (i=0; i<NUM_FONTS; i++) {
    for (j=0; j<NUM_SIZES; j++) {
      font[j][i] = &fontRec[j][i];
      ParseFontName(font[j][i]->def, &font[j][i]->mfp);
    }
  }
  
  /* Parse default settings file if any */
  if (ParseSettingsFile(settingsFileName, buf)) {
    settingsFileName = strdup(buf);
  }

  /* Parse command line */
  ParseArgs(StringGet, &lpCmdLine);

  /* [HGM] make sure board size is acceptable */
  if(appData.NrFiles > BOARD_SIZE ||
     appData.NrRanks > BOARD_SIZE   )
      DisplayFatalError("Recompile with BOARD_SIZE > 12, to support this size", 0, 2);

  /* Propagate options that affect others */
  if (appData.matchMode || appData.matchGames) chessProgram = TRUE;
  if (appData.icsActive || appData.noChessProgram) {
     chessProgram = FALSE;  /* not local chess program mode */
  }

  /* Open startup dialog if needed */
  if ((!appData.noChessProgram && !chessProgram && !appData.icsActive) ||
      (appData.icsActive && *appData.icsHost == NULLCHAR) ||
      (chessProgram && (*appData.firstChessProgram == NULLCHAR ||
                        *appData.secondChessProgram == NULLCHAR))) {
    FARPROC lpProc;
    
    lpProc = MakeProcInstance((FARPROC)StartupDialog, hInst);
    DialogBox(hInst, MAKEINTRESOURCE(DLG_Startup), NULL, (DLGPROC)lpProc);
    FreeProcInstance(lpProc);
  }

  /* Make sure save files land in the right (?) directory */
  if (GetFullPathName(appData.saveGameFile, MSG_SIZ, buf, &dummy)) {
    appData.saveGameFile = strdup(buf);
  }
  if (GetFullPathName(appData.savePositionFile, MSG_SIZ, buf, &dummy)) {
    appData.savePositionFile = strdup(buf);
  }

  /* Finish initialization for fonts and sounds */
  for (i=0; i<NUM_FONTS; i++) {
    for (j=0; j<NUM_SIZES; j++) {
      CreateFontInMF(font[j][i]);
    }
  }
  /* xboard, and older WinBoards, controlled the move sound with the
     appData.ringBellAfterMoves option.  In the current WinBoard, we
     always turn the option on (so that the backend will call us),
     then let the user turn the sound off by setting it to silence if
     desired.  To accommodate old winboard.ini files saved by old
     versions of WinBoard, we also turn off the sound if the option
     was initially set to false. */
  if (!appData.ringBellAfterMoves) {
    sounds[(int)SoundMove].name = strdup("");
    appData.ringBellAfterMoves = TRUE;
  }
  GetCurrentDirectory(MSG_SIZ, currDir);
  SetCurrentDirectory(installDir);
  LoadAllSounds();
  SetCurrentDirectory(currDir);

  p = icsTextMenuString;
  if (p[0] == '@') {
    FILE* f = fopen(p + 1, "r");
    if (f == NULL) {
      DisplayFatalError(p + 1, errno, 2);
      return;
    }
    i = fread(buf, 1, sizeof(buf)-1, f);
    fclose(f);
    buf[i] = NULLCHAR;
    p = buf;
  }
  ParseIcsTextMenu(strdup(p));
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
}


VOID
SaveSettings(char* name)
{
  FILE *f;
  ArgDescriptor *ad;
  WINDOWPLACEMENT wp;
  char dir[MSG_SIZ];

  if (!hwndMain) return;

  GetCurrentDirectory(MSG_SIZ, dir);
  SetCurrentDirectory(installDir);
  f = fopen(name, "w");
  SetCurrentDirectory(dir);
  if (f == NULL) {
    DisplayError(name, errno);
    return;
  }
  fprintf(f, ";\n");
  fprintf(f, "; %s %s.%s Save Settings file\n", PRODUCT, VERSION, PATCHLEVEL);
  fprintf(f, ";\n");
  fprintf(f, "; You can edit the values of options that are already set in this file,\n");
  fprintf(f, "; but if you add other options, the next Save Settings will not save them.\n");
  fprintf(f, "; Use a shortcut, an @indirection file, or a .bat file instead.\n");
  fprintf(f, ";\n");

  wp.length = sizeof(WINDOWPLACEMENT);
  GetWindowPlacement(hwndMain, &wp);
  boardX = wp.rcNormalPosition.left;
  boardY = wp.rcNormalPosition.top;

  if (hwndConsole) {
    GetWindowPlacement(hwndConsole, &wp);
    consoleX = wp.rcNormalPosition.left;
    consoleY = wp.rcNormalPosition.top;
    consoleW = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
    consoleH = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
  }

  if (analysisDialog) {
    GetWindowPlacement(analysisDialog, &wp);
    analysisX = wp.rcNormalPosition.left;
    analysisY = wp.rcNormalPosition.top;
    analysisW = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
    analysisH = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
  }

  if (commentDialog) {
    GetWindowPlacement(commentDialog, &wp);
    commentX = wp.rcNormalPosition.left;
    commentY = wp.rcNormalPosition.top;
    commentW = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
    commentH = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
  }

  if (editTagsDialog) {
    GetWindowPlacement(editTagsDialog, &wp);
    editTagsX = wp.rcNormalPosition.left;
    editTagsY = wp.rcNormalPosition.top;
    editTagsW = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
    editTagsH = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
  }

  if (gameListDialog) {
    GetWindowPlacement(gameListDialog, &wp);
    gameListX = wp.rcNormalPosition.left;
    gameListY = wp.rcNormalPosition.top;
    gameListW = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
    gameListH = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
  }

  /* [AS] Move history */
  wpMoveHistory.visible = MoveHistoryIsUp();
  
  if( moveHistoryDialog ) {
    GetWindowPlacement(moveHistoryDialog, &wp);
    wpMoveHistory.x = wp.rcNormalPosition.left;
    wpMoveHistory.y = wp.rcNormalPosition.top;
    wpMoveHistory.width = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
    wpMoveHistory.height = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
  }

  /* [AS] Eval graph */
  wpEvalGraph.visible = EvalGraphIsUp();

  if( evalGraphDialog ) {
    GetWindowPlacement(evalGraphDialog, &wp);
    wpEvalGraph.x = wp.rcNormalPosition.left;
    wpEvalGraph.y = wp.rcNormalPosition.top;
    wpEvalGraph.width = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
    wpEvalGraph.height = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
  }

  /* [AS] Engine output */
  wpEngineOutput.visible = EngineOutputIsUp();

  if( engineOutputDialog ) {
    GetWindowPlacement(engineOutputDialog, &wp);
    wpEngineOutput.x = wp.rcNormalPosition.left;
    wpEngineOutput.y = wp.rcNormalPosition.top;
    wpEngineOutput.width = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
    wpEngineOutput.height = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
  }

  for (ad = argDescriptors; ad->argName != NULL; ad++) {
    if (!ad->save) continue;
    switch (ad->argType) {
    case ArgString:
      {
	char *p = *(char **)ad->argLoc;
	if ((strchr(p, '\\') || strchr(p, '\n')) && !strchr(p, '}')) {
	  /* Quote multiline values or \-containing values
	     with { } if possible */
	  fprintf(f, "/%s={%s}\n", ad->argName, p);
	} else {
	  /* Else quote with " " */
	  fprintf(f, "/%s=\"", ad->argName);
	  while (*p) {
	    if (*p == '\n') fprintf(f, "\n");
	    else if (*p == '\r') fprintf(f, "\\r");
	    else if (*p == '\t') fprintf(f, "\\t");
	    else if (*p == '\b') fprintf(f, "\\b");
	    else if (*p == '\f') fprintf(f, "\\f");
	    else if (*p < ' ') fprintf(f, "\\%03o", *p);
	    else if (*p == '\"') fprintf(f, "\\\"");
	    else if (*p == '\\') fprintf(f, "\\\\");
	    else putc(*p, f);
	    p++;
	  }
	  fprintf(f, "\"\n");
	}
      }
      break;
    case ArgInt:
      fprintf(f, "/%s=%d\n", ad->argName, *(int *)ad->argLoc);
      break;
    case ArgFloat:
      fprintf(f, "/%s=%g\n", ad->argName, *(float *)ad->argLoc);
      break;
    case ArgBoolean:
      fprintf(f, "/%s=%s\n", ad->argName, 
	(*(Boolean *)ad->argLoc) ? "true" : "false");
      break;
    case ArgTrue:
      if (*(Boolean *)ad->argLoc) fprintf(f, "/%s\n", ad->argName);
      break;
    case ArgFalse:
      if (!*(Boolean *)ad->argLoc) fprintf(f, "/%s\n", ad->argName);
      break;
    case ArgColor:
      {
	COLORREF color = *(COLORREF *)ad->argLoc;
	fprintf(f, "/%s=#%02x%02x%02x\n", ad->argName, 
	  color&0xff, (color>>8)&0xff, (color>>16)&0xff);
      }
      break;
    case ArgAttribs:
      {
	MyTextAttribs* ta = &textAttribs[(ColorClass)ad->argLoc];
	fprintf(f, "/%s=\"%s%s%s%s%s#%02x%02x%02x\"\n", ad->argName,
          (ta->effects & CFE_BOLD) ? "b" : "",
          (ta->effects & CFE_ITALIC) ? "i" : "",
          (ta->effects & CFE_UNDERLINE) ? "u" : "",
          (ta->effects & CFE_STRIKEOUT) ? "s" : "",
          (ta->effects) ? " " : "",
	  ta->color&0xff, (ta->color >> 8)&0xff, (ta->color >> 16)&0xff);
      }
      break;
    case ArgFilename:
      if (strchr(*(char **)ad->argLoc, '\"')) {
	fprintf(f, "/%s='%s'\n", ad->argName, *(char **)ad->argLoc);
      } else {
	fprintf(f, "/%s=\"%s\"\n", ad->argName, *(char **)ad->argLoc);
      }
      break;
    case ArgBoardSize:
      fprintf(f, "/%s=%s\n", ad->argName,
	      sizeInfo[*(BoardSize *)ad->argLoc].name);
      break;
    case ArgFont:
      {
        int bs;
	for (bs=0; bs<NUM_SIZES; bs++) {
	  MyFontParams *mfp = &font[bs][(int) ad->argLoc]->mfp;
          fprintf(f, "/size=%s ", sizeInfo[bs].name);
	  fprintf(f, "/%s=\"%s:%g%s%s%s%s%s\"\n",
	    ad->argName, mfp->faceName, mfp->pointSize,
            mfp->bold || mfp->italic || mfp->underline || mfp->strikeout ? " " : "",
	    mfp->bold ? "b" : "",
	    mfp->italic ? "i" : "",
	    mfp->underline ? "u" : "",
	    mfp->strikeout ? "s" : "");
	}
      }
      break;
    case ArgCommSettings:
      PrintCommSettings(f, ad->argName, (DCB *)ad->argLoc);
    }
  }
  fclose(f);
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
    PM_WA = (int) WhiteCardinal, 
    PM_WC = (int) WhiteMarshall, 
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
    PM_BA = (int) BlackCardinal, 
    PM_BC = (int) BlackMarshall, 
    PM_BG = (int) BlackGrasshopper, 
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
    int shapeIndex = index < 6 ? index+6 : index;
    
    if( index < 6 && appData.fontBackColorWhite != appData.fontForeColorWhite ) {
        backColor = appData.fontBackColorWhite;
        foreColor = appData.fontForeColorWhite;
    }
    else if( index >= 6 && appData.fontBackColorBlack != appData.fontForeColorBlack ) {
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
    TextOut( hdc, pt.x, pt.y, &pieceToFontChar[index], 1 );

    SelectObject( hdc, GetStockObject(WHITE_BRUSH) );
    /* Step 3: the area outside the piece is filled with white */
    FloodFill( hdc, 0, 0, chroma );
    SelectObject( hdc, GetStockObject(BLACK_BRUSH) );
    /* 
        Step 4: this is the tricky part, the area inside the piece is filled with black,
        but if the start point is not inside the piece we're lost!
        There should be a better way to do this... if we could create a region or path
        from the fill operation we would be fine for example.
    */
    FloodFill( hdc, squareSize / 2, squareSize / 2, RGB(0xFF,0xFF,0xFF) );

    SetTextColor( hdc, 0 );
    /* 
        Step 5: some fonts have "disconnected" areas that are skipped by the fill:
        draw the piece again in black for safety.
    */
    TextOut( hdc, pt.x, pt.y, &pieceToFontChar[index], 1 );

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
    TextOut( hdc, pt.x, pt.y, &pieceToFontChar[index], 1 );

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
#ifdef FAIRY
    case BlackCardinal:
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
    case WhiteCardinal:
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
#endif
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

    if( appData.renderPiecesWithFont == NULL || appData.renderPiecesWithFont[0] == NULLCHAR || appData.renderPiecesWithFont[0] == '*' ) {
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
            for( i=0; i<12; i++ ) {
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
                     SetCharTable(pieceToFontChar, "PNBRQFWEMOUHACGSKpnbrqfwemouhacgsk");
                }
                else if( strstr(lf.lfFaceName,"GC2004D") != NULL ) {
                    /* Good Companion (Some characters get warped as literal :-( */
                    char s[] = "1cmWG0ñueOS¯®oYI23wgQU";
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

            CreatePieceMaskFromFont( hdc_window, hdc, PM_WP );
            CreatePieceMaskFromFont( hdc_window, hdc, PM_WN );
            CreatePieceMaskFromFont( hdc_window, hdc, PM_WB );
            CreatePieceMaskFromFont( hdc_window, hdc, PM_WR );
            CreatePieceMaskFromFont( hdc_window, hdc, PM_WQ );
            CreatePieceMaskFromFont( hdc_window, hdc, PM_WK );
            CreatePieceMaskFromFont( hdc_window, hdc, PM_BP );
            CreatePieceMaskFromFont( hdc_window, hdc, PM_BN );
            CreatePieceMaskFromFont( hdc_window, hdc, PM_BB );
            CreatePieceMaskFromFont( hdc_window, hdc, PM_BR );
            CreatePieceMaskFromFont( hdc_window, hdc, PM_BQ );
            CreatePieceMaskFromFont( hdc_window, hdc, PM_BK );
#ifdef FAIRY
            CreatePieceMaskFromFont( hdc_window, hdc, PM_WA );
            CreatePieceMaskFromFont( hdc_window, hdc, PM_WC );
            CreatePieceMaskFromFont( hdc_window, hdc, PM_WF );
            CreatePieceMaskFromFont( hdc_window, hdc, PM_WH );
            CreatePieceMaskFromFont( hdc_window, hdc, PM_WE );
            CreatePieceMaskFromFont( hdc_window, hdc, PM_WW );
            CreatePieceMaskFromFont( hdc_window, hdc, PM_WU );
            CreatePieceMaskFromFont( hdc_window, hdc, PM_WO );
            CreatePieceMaskFromFont( hdc_window, hdc, PM_WG );
            CreatePieceMaskFromFont( hdc_window, hdc, PM_WM );
            CreatePieceMaskFromFont( hdc_window, hdc, PM_BA );
            CreatePieceMaskFromFont( hdc_window, hdc, PM_BC );
            CreatePieceMaskFromFont( hdc_window, hdc, PM_BF );
            CreatePieceMaskFromFont( hdc_window, hdc, PM_BH );
            CreatePieceMaskFromFont( hdc_window, hdc, PM_BE );
            CreatePieceMaskFromFont( hdc_window, hdc, PM_BW );
            CreatePieceMaskFromFont( hdc_window, hdc, PM_BU );
            CreatePieceMaskFromFont( hdc_window, hdc, PM_BO );
            CreatePieceMaskFromFont( hdc_window, hdc, PM_BG );
            CreatePieceMaskFromFont( hdc_window, hdc, PM_BM );
#endif

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
  char name[128];

  sprintf(name, "%s%d%s", piece, squareSize, suffix);
  if (gameInfo.event &&
      strcmp(gameInfo.event, "Easter Egg Hunt") == 0 &&
      strcmp(name, "k80s") == 0) {
    strcpy(name, "tim");
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
    DisplayFatalError("Too many colors", 0, 1);
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
  while (newSize > 0 &&
	 (newSizeX < sizeInfo[newSize].cliWidth ||
	  newSizeY < sizeInfo[newSize].cliHeight)) {
    newSize--;
  } 
  boardSize = newSize;
  InitDrawingSizes(boardSize, flags);
  recurse--;
}



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
  RECT crect, wrect;
  int offby;
  LOGBRUSH logbrush;

  /* [HGM] call with -1 uses old size (for if nr of files, ranks changes) */
  if(boardSize == (BoardSize)(-2) ) boardSize = oldBoardSize;

  tinyLayout = sizeInfo[boardSize].tinyLayout;
  smallLayout = sizeInfo[boardSize].smallLayout;
  squareSize = sizeInfo[boardSize].squareSize;
  lineGap = sizeInfo[boardSize].lineGap;
  minorSize = 0; /* [HGM] Kludge to see if demagnified pieces need to be shifted  */

  if( appData.overrideLineGap >= 0 && appData.overrideLineGap <= 5 ) {
      lineGap = appData.overrideLineGap;
  }

  if (tinyLayout != oldTinyLayout) {
    long style = GetWindowLong(hwndMain, GWL_STYLE);
    if (tinyLayout) {
      style &= ~WS_SYSMENU;
      InsertMenu(hmenu, IDM_Exit, MF_BYCOMMAND, IDM_Minimize,
		 "&Minimize\tCtrl+F4");
    } else {
      style |= WS_SYSMENU;
      RemoveMenu(hmenu, IDM_Minimize, MF_BYCOMMAND);
    }
    SetWindowLong(hwndMain, GWL_STYLE, style);

    for (i=0; menuBarText[tinyLayout][i]; i++) {
      ModifyMenu(hmenu, i, MF_STRING|MF_BYPOSITION|MF_POPUP, 
	(UINT)GetSubMenu(hmenu, i), menuBarText[tinyLayout][i]);
    }
    DrawMenuBar(hwndMain);
  }

  boardWidth  = BoardWidth(boardSize, BOARD_WIDTH);
  boardHeight = BoardWidth(boardSize, BOARD_HEIGHT);

  /* Get text area sizes */
  hdc = GetDC(hwndMain);
  if (appData.clockMode) {
    sprintf(buf, "White: %s", TimeString(23*60*60*1000L));
  } else {
    sprintf(buf, "White");
  }
  oldFont = SelectObject(hdc, font[boardSize][CLOCK_FONT]->hf);
  GetTextExtentPoint(hdc, buf, strlen(buf), &clockSize);
  SelectObject(hdc, font[boardSize][MESSAGE_FONT]->hf);
  str = "We only care about the height here";
  GetTextExtentPoint(hdc, str, strlen(str), &messageSize);
  SelectObject(hdc, oldFont);
  ReleaseDC(hwndMain, hdc);

  /* Compute where everything goes */
  whiteRect.left = OUTER_MARGIN;
  whiteRect.right = whiteRect.left + boardWidth/2 - INNER_MARGIN/2;
  whiteRect.top = OUTER_MARGIN;
  whiteRect.bottom = whiteRect.top + clockSize.cy;

  blackRect.left = whiteRect.right + INNER_MARGIN;
  blackRect.right = blackRect.left + boardWidth/2 - 1;
  blackRect.top = whiteRect.top;
  blackRect.bottom = whiteRect.bottom;

  messageRect.left = whiteRect.left + MESSAGE_LINE_LEFTMARGIN;
  if (appData.showButtonBar) {
    messageRect.right = blackRect.right
      - N_BUTTONS*BUTTON_WIDTH - MESSAGE_LINE_LEFTMARGIN;
  } else {
    messageRect.right = blackRect.right;
  }
  messageRect.top = whiteRect.bottom + INNER_MARGIN;
  messageRect.bottom = messageRect.top + messageSize.cy;

  boardRect.left = whiteRect.left;
  boardRect.right = boardRect.left + boardWidth;
  boardRect.top = messageRect.bottom + INNER_MARGIN;
  boardRect.bottom = boardRect.top + boardHeight;

  sizeInfo[boardSize].cliWidth = boardRect.right + OUTER_MARGIN;
  sizeInfo[boardSize].cliHeight = boardRect.bottom + OUTER_MARGIN;
  winWidth = 2 * GetSystemMetrics(SM_CXFRAME) + boardRect.right + OUTER_MARGIN;
  winHeight = 2 * GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CYMENU) +
    GetSystemMetrics(SM_CYCAPTION) + boardRect.bottom + OUTER_MARGIN;
  GetWindowRect(hwndMain, &wrect);
  SetWindowPos(hwndMain, NULL, 0, 0, winWidth, winHeight,
	       SWP_NOCOPYBITS|SWP_NOZORDER|SWP_NOMOVE);
  /* compensate if menu bar wrapped */
  GetClientRect(hwndMain, &crect);
  offby = boardRect.bottom + OUTER_MARGIN - crect.bottom;
  winHeight += offby;
  switch (flags) {
  case WMSZ_TOPLEFT:
    SetWindowPos(hwndMain, NULL, 
                 wrect.right - winWidth, wrect.bottom - winHeight, 
                 winWidth, winHeight, SWP_NOCOPYBITS|SWP_NOZORDER);
    break;

  case WMSZ_TOPRIGHT:
  case WMSZ_TOP:
    SetWindowPos(hwndMain, NULL, 
                 wrect.left, wrect.bottom - winHeight, 
                 winWidth, winHeight, SWP_NOCOPYBITS|SWP_NOZORDER);
    break;

  case WMSZ_BOTTOMLEFT:
  case WMSZ_LEFT:
    SetWindowPos(hwndMain, NULL, 
                 wrect.right - winWidth, wrect.top, 
                 winWidth, winHeight, SWP_NOCOPYBITS|SWP_NOZORDER);
    break;

  case WMSZ_BOTTOMRIGHT:
  case WMSZ_BOTTOM:
  case WMSZ_RIGHT:
  default:
    SetWindowPos(hwndMain, NULL, 0, 0, winWidth, winHeight,
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
		     (HINSTANCE) GetWindowLong(hwndMain, GWL_HINSTANCE), NULL);
      if (tinyLayout) {
	SendMessage(buttonDesc[i].hwnd, WM_SETFONT, 
		    (WPARAM)font[boardSize][MESSAGE_FONT]->hf,
		    MAKELPARAM(FALSE, 0));
      }
      if (buttonDesc[i].id == IDM_Pause)
	hwndPause = buttonDesc[i].hwnd;
      buttonDesc[i].wndproc = (WNDPROC)
	SetWindowLong(buttonDesc[i].hwnd, GWL_WNDPROC, (LONG) ButtonProc);
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
      gridEndpoints[i*2].x = boardRect.left + lineGap / 2;
      gridEndpoints[i*2].y = gridEndpoints[i*2 + 1].y =
	boardRect.top + lineGap / 2 + (i * (squareSize + lineGap));
      gridEndpoints[i*2 + 1].x = boardRect.left + lineGap / 2 +
        BOARD_WIDTH * (squareSize + lineGap);
	lineGap / 2 + (i * (squareSize + lineGap));
      gridVertexCounts[i*2] = gridVertexCounts[i*2 + 1] = 2;
    }
    for (i = 0; i < BOARD_WIDTH + 1; i++) {
      gridEndpoints[i*2 + BOARD_HEIGHT*2 + 2].y = boardRect.top + lineGap / 2;
      gridEndpoints[i*2 + BOARD_HEIGHT*2 + 2].x =
        gridEndpoints[i*2 + 1 + BOARD_HEIGHT*2 + 2].x = boardRect.left +
	lineGap / 2 + (i * (squareSize + lineGap));
      gridEndpoints[i*2 + 1 + BOARD_HEIGHT*2 + 2].y =
        boardRect.top + BOARD_HEIGHT * (squareSize + lineGap);
      gridVertexCounts[i*2] = gridVertexCounts[i*2 + 1] = 2;
    }
  }

#ifdef GOTHIC
  /* [HGM] Gothic licensing requirement */
  GothicPopUp( GOTHIC, gameInfo.variant == VariantGothic );
#endif

/*  if (boardSize == oldBoardSize) return; [HGM] variant might have changed */
  oldBoardSize = boardSize;
  oldTinyLayout = tinyLayout;

  /* Load piece bitmaps for this board size */
  for (i=0; i<=2; i++) {
    for (piece = WhitePawn;
         (int) piece < (int) BlackPawn;
	 piece = (ChessSquare) ((int) piece + 1)) {
      if (pieceBitmap[i][piece] != NULL)
	DeleteObject(pieceBitmap[i][piece]);
    }
  }

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
  if( !strcmp(appData.variant, "shogi") && (squareSize==72 || squareSize==49)) {
  pieceBitmap[0][WhiteQueen] = DoLoadBitmap(hInst, "l", squareSize, "s");
  pieceBitmap[1][WhiteQueen] = DoLoadBitmap(hInst, "l", squareSize, "o");
  pieceBitmap[2][WhiteQueen] = DoLoadBitmap(hInst, "l", squareSize, "w");
  } else {
  pieceBitmap[0][WhiteQueen] = DoLoadBitmap(hInst, "q", squareSize, "s");
  pieceBitmap[1][WhiteQueen] = DoLoadBitmap(hInst, "q", squareSize, "o");
  pieceBitmap[2][WhiteQueen] = DoLoadBitmap(hInst, "q", squareSize, "w");
  }
  if(squareSize==72 || squareSize==49) { /* experiment with some home-made bitmaps */
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
  pieceBitmap[0][WhiteCannon] = DoLoadBitmap(hInst, "o", squareSize, "s");
  pieceBitmap[1][WhiteCannon] = DoLoadBitmap(hInst, "o", squareSize, "o");
  pieceBitmap[2][WhiteCannon] = DoLoadBitmap(hInst, "o", squareSize, "w");
  pieceBitmap[0][WhiteCardinal] = DoLoadBitmap(hInst, "a", squareSize, "s");
  pieceBitmap[1][WhiteCardinal] = DoLoadBitmap(hInst, "a", squareSize, "o");
  pieceBitmap[2][WhiteCardinal] = DoLoadBitmap(hInst, "a", squareSize, "w");
  if(gameInfo.variant == VariantShogi) { /* promoted Gold represemtations */
  pieceBitmap[0][WhiteUnicorn] = DoLoadBitmap(hInst, "wp", squareSize, "s");
  pieceBitmap[1][WhiteUnicorn] = DoLoadBitmap(hInst, "wp", squareSize, "o");
  pieceBitmap[2][WhiteUnicorn] = DoLoadBitmap(hInst, "w", squareSize, "w");
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
  pieceBitmap[0][WhiteUnicorn] = DoLoadBitmap(hInst, "u", squareSize, "s");
  pieceBitmap[1][WhiteUnicorn] = DoLoadBitmap(hInst, "u", squareSize, "o");
  pieceBitmap[2][WhiteUnicorn] = DoLoadBitmap(hInst, "u", squareSize, "w");
  pieceBitmap[0][WhiteNightrider] = DoLoadBitmap(hInst, "h", squareSize, "s");
  pieceBitmap[1][WhiteNightrider] = DoLoadBitmap(hInst, "h", squareSize, "o");
  pieceBitmap[2][WhiteNightrider] = DoLoadBitmap(hInst, "h", squareSize, "w");
  pieceBitmap[0][WhiteGrasshopper] = DoLoadBitmap(hInst, "g", squareSize, "s");
  pieceBitmap[1][WhiteGrasshopper] = DoLoadBitmap(hInst, "g", squareSize, "o");
  pieceBitmap[2][WhiteGrasshopper] = DoLoadBitmap(hInst, "g", squareSize, "w");
  pieceBitmap[0][WhiteSilver] = DoLoadBitmap(hInst, "l", squareSize, "s");
  pieceBitmap[1][WhiteSilver] = DoLoadBitmap(hInst, "l", squareSize, "o");
  pieceBitmap[2][WhiteSilver] = DoLoadBitmap(hInst, "l", squareSize, "w");
  }
  if(gameInfo.variant != VariantCrazyhouse && gameInfo.variant != VariantShogi) {
  pieceBitmap[0][WhiteMarshall] = DoLoadBitmap(hInst, "c", squareSize, "s");
  pieceBitmap[1][WhiteMarshall] = DoLoadBitmap(hInst, "c", squareSize, "o");
  pieceBitmap[2][WhiteMarshall] = DoLoadBitmap(hInst, "c", squareSize, "w");
  } else {
  pieceBitmap[0][WhiteMarshall] = DoLoadBitmap(hInst, "dk", squareSize, "s");
  pieceBitmap[1][WhiteMarshall] = DoLoadBitmap(hInst, "dk", squareSize, "o");
  pieceBitmap[2][WhiteMarshall] = DoLoadBitmap(hInst, "dk", squareSize, "w");
  }
  } else { /* other size, no special bitmaps available. Use smaller symbols */
      if((int)boardSize < 2) minorSize = sizeInfo[0].squareSize;
      else  minorSize = sizeInfo[(int)boardSize - 2].squareSize;
  pieceBitmap[0][WhiteNightrider] = DoLoadBitmap(hInst, "n", minorSize, "s");
  pieceBitmap[1][WhiteNightrider] = DoLoadBitmap(hInst, "n", minorSize, "o");
  pieceBitmap[2][WhiteNightrider] = DoLoadBitmap(hInst, "n", minorSize, "w");
  pieceBitmap[0][WhiteCardinal] = DoLoadBitmap(hInst, "b", minorSize, "s");
  pieceBitmap[1][WhiteCardinal] = DoLoadBitmap(hInst, "b", minorSize, "o");
  pieceBitmap[2][WhiteCardinal] = DoLoadBitmap(hInst, "b", minorSize, "w");
  pieceBitmap[0][WhiteMarshall] = DoLoadBitmap(hInst, "r", minorSize, "s");
  pieceBitmap[1][WhiteMarshall] = DoLoadBitmap(hInst, "r", minorSize, "o");
  pieceBitmap[2][WhiteMarshall] = DoLoadBitmap(hInst, "r", minorSize, "w");
  pieceBitmap[0][WhiteGrasshopper] = DoLoadBitmap(hInst, "q", minorSize, "s");
  pieceBitmap[1][WhiteGrasshopper] = DoLoadBitmap(hInst, "q", minorSize, "o");
  pieceBitmap[2][WhiteGrasshopper] = DoLoadBitmap(hInst, "q", minorSize, "w");
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
    *x = boardRect.left + lineGap + ((BOARD_WIDTH-1)-column) * (squareSize + lineGap);
    *y = boardRect.top + lineGap + row * (squareSize + lineGap);
  } else {
    *x = boardRect.left + lineGap + column * (squareSize + lineGap);
    *y = boardRect.top + lineGap + ((BOARD_HEIGHT-1)-row) * (squareSize + lineGap);
  }
}

VOID
DrawCoordsOnDC(HDC hdc)
{
  static char files[24] = {'0', '1','2','3','4','5','6','7','8','9','0','1','1','0','9','8','7','6','5','4','3','2','1','0'};
  static char ranks[24] = {'l', 'k','j','i','h','g','f','e','d','c','b','a','a','b','c','d','e','f','g','h','i','j','k','l'};
  char str[2] = { NULLCHAR, NULLCHAR };
  int oldMode, oldAlign, x, y, start, i;
  HFONT oldFont;
  HBRUSH oldBrush;

  if (!appData.showCoords)
    return;

  start = flipView ? 1-(ONE!='1') : 23+(ONE!='1')-BOARD_HEIGHT;

  oldBrush = SelectObject(hdc, GetStockObject(BLACK_BRUSH));
  oldMode = SetBkMode(hdc, (appData.monoMode ? OPAQUE : TRANSPARENT));
  oldAlign = GetTextAlign(hdc);
  oldFont = SelectObject(hdc, font[boardSize][COORD_FONT]->hf);

  y = boardRect.top + lineGap;
  x = boardRect.left + lineGap;

  SetTextAlign(hdc, TA_LEFT|TA_TOP);
  for (i = 0; i < BOARD_HEIGHT; i++) {
    str[0] = files[start + i];
    ExtTextOut(hdc, x + 2, y + 1, 0, NULL, str, 1, NULL);
    y += squareSize + lineGap;
  }

  start = flipView ? 12-BOARD_WIDTH : 12;

  SetTextAlign(hdc, TA_RIGHT|TA_BOTTOM);
  for (i = 0; i < BOARD_WIDTH; i++) {
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
      lineGap/2 + ((BOARD_WIDTH-1)-x) * (squareSize + lineGap);
    y1 = boardRect.top +
      lineGap/2 + y * (squareSize + lineGap);
  } else {
    x1 = boardRect.left +
      lineGap/2 + x * (squareSize + lineGap);
    y1 = boardRect.top +
      lineGap/2 + ((BOARD_HEIGHT-1)-y) * (squareSize + lineGap);
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
DrawHighlightsOnDC(HDC hdc)
{
  int i;
  for (i=0; i<2; i++) {
    if (highlightInfo.sq[i].x >= 0 && highlightInfo.sq[i].y >= 0) 
      DrawHighlightOnDC(hdc, TRUE,
			highlightInfo.sq[i].x, highlightInfo.sq[i].y,
			HIGHLIGHT_PEN);
  }
  for (i=0; i<2; i++) {
    if (premoveHighlightInfo.sq[i].x >= 0 && 
	premoveHighlightInfo.sq[i].y >= 0) {
	DrawHighlightOnDC(hdc, TRUE,
	   		  premoveHighlightInfo.sq[i].x, 
			  premoveHighlightInfo.sq[i].y,
			  PREMOVE_PEN);
    }
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

  if (appData.blindfold) return;

  /* [AS] Use font-based pieces if needed */
  if( fontBitmapSquareSize >= 0 && squareSize > 32 ) {
    /* Create piece bitmaps, or do nothing if piece set is up to date */
    CreatePiecesFromFont();

    if( fontBitmapSquareSize == squareSize ) {
        int index = TranslatePieceToFontPiece( piece );

        SelectObject( tmphdc, hPieceMask[ index ] );

        BitBlt( hdc,
            x, y,
            squareSize, squareSize,
            tmphdc,
            0, 0,
            SRCAND );

        SelectObject( tmphdc, hPieceFace[ index ] );

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
    if(minorSize &&
        (piece >= (int)WhiteNightrider && piece <= WhiteGrasshopper ||
         piece >= (int)BlackNightrider && piece <= BlackGrasshopper)  ) {
      /* [HGM] no bitmap available for promoted pieces in Crazyhouse        */
      /* Bitmaps of smaller size are substituted, but we have to align them */
      x += (squareSize - minorSize)>>1;
      y += squareSize - minorSize - 2;
    }
    if (color || appData.allWhite ) {
      oldBitmap = SelectObject(tmphdc, PieceBitmap(piece, WHITE_PIECE));
      if( color )
              oldBrush = SelectObject(hdc, whitePieceBrush);
      else    oldBrush = SelectObject(hdc, blackPieceBrush);
      BitBlt(hdc, x, y, squareSize, squareSize, tmphdc, 0, 0, 0x00B8074A);
#if 0
      /* Use black piece color for outline of white pieces */
      /* Not sure this looks really good (though xboard does it).
	 Maybe better to have another selectable color, default black */
      SelectObject(hdc, blackPieceBrush); /* could have own brush */
      SelectObject(tmphdc, PieceBitmap(piece, OUTLINE_PIECE));
      BitBlt(hdc, x, y, squareSize, squareSize, tmphdc, 0, 0, 0x00B8074A);
#else
      /* Use black for outline of white pieces */
      SelectObject(tmphdc, PieceBitmap(piece, OUTLINE_PIECE));
      BitBlt(hdc, x, y, squareSize, squareSize, tmphdc, 0, 0, SRCAND);
#endif
    } else {
#if 0
      /* Use white piece color for details of black pieces */
      /* Requires filled-in solid bitmaps (BLACK_PIECE class); the
	 WHITE_PIECE ones aren't always the right shape. */
      /* Not sure this looks really good (though xboard does it).
	 Maybe better to have another selectable color, default medium gray? */
      oldBitmap = SelectObject(tmphdc, PieceBitmap(piece, BLACK_PIECE));
      oldBrush = SelectObject(hdc, whitePieceBrush); /* could have own brush */
      BitBlt(hdc, x, y, squareSize, squareSize, tmphdc, 0, 0, 0x00B8074A);
      SelectObject(tmphdc, PieceBitmap(piece, SOLID_PIECE));
      SelectObject(hdc, blackPieceBrush);
      BitBlt(hdc, x, y, squareSize, squareSize, tmphdc, 0, 0, 0x00B8074A);
#else
      /* Use square color for details of black pieces */
      oldBitmap = SelectObject(tmphdc, PieceBitmap(piece, SOLID_PIECE));
      oldBrush = SelectObject(hdc, blackPieceBrush);
      BitBlt(hdc, x, y, squareSize, squareSize, tmphdc, 0, 0, 0x00B8074A);
#endif
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
                    backTextureSquareInfo[row][col].x = col * (lite_w - squareSize) / BOARD_WIDTH;
                    backTextureSquareInfo[row][col].y = row * (lite_h - squareSize) / BOARD_HEIGHT;
                    backTextureSquareInfo[row][col].mode = GetBackTextureMode(liteBackTextureMode);
                }
            }
            else {
                /* Dark square */
                if( dark_w >= squareSize && dark_h >= squareSize ) {
                    backTextureSquareInfo[row][col].x = col * (dark_w - squareSize) / BOARD_WIDTH;
                    backTextureSquareInfo[row][col].y = row * (dark_h - squareSize) / BOARD_HEIGHT;
                    backTextureSquareInfo[row][col].mode = GetBackTextureMode(darkBackTextureMode);
                }
            }
        }
    }
}

/* [AS] Arrow highlighting support */

static int A_WIDTH = 5; /* Width of arrow body */

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

        arrow[0].x = s_x + A_WIDTH;
        arrow[0].y = s_y;

        arrow[1].x = s_x + A_WIDTH;
        arrow[1].y = d_y - h;

        arrow[2].x = s_x + A_WIDTH*A_WIDTH_FACTOR;
        arrow[2].y = d_y - h;

        arrow[3].x = d_x;
        arrow[3].y = d_y;

        arrow[4].x = s_x - A_WIDTH*A_WIDTH_FACTOR;
        arrow[4].y = d_y - h;

        arrow[5].x = s_x - A_WIDTH;
        arrow[5].y = d_y - h;

        arrow[6].x = s_x - A_WIDTH;
        arrow[6].y = s_y;
    }
    else if( d_y == s_y ) {
        int w = (d_x > s_x) ? +A_WIDTH*A_HEIGHT_FACTOR : -A_WIDTH*A_HEIGHT_FACTOR;

        arrow[0].x = s_x;
        arrow[0].y = s_y + A_WIDTH;

        arrow[1].x = d_x - w;
        arrow[1].y = s_y + A_WIDTH;

        arrow[2].x = d_x - w;
        arrow[2].y = s_y + A_WIDTH*A_WIDTH_FACTOR;

        arrow[3].x = d_x;
        arrow[3].y = d_y;

        arrow[4].x = d_x - w;
        arrow[4].y = s_y - A_WIDTH*A_WIDTH_FACTOR;

        arrow[5].x = d_x - w;
        arrow[5].y = s_y - A_WIDTH;

        arrow[6].x = s_x;
        arrow[6].y = s_y - A_WIDTH;
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

        arrow[1].x = Round(x + j);
        arrow[1].y = Round(y - j*dx);

        if( d_x > s_x ) {
            x = (double) d_x - k;
            y = (double) d_y - k*dy;
        }
        else {
            x = (double) d_x + k;
            y = (double) d_y + k*dy;
        }

        arrow[2].x = Round(x + j);
        arrow[2].y = Round(y - j*dx);

        arrow[3].x = Round(x + j*A_WIDTH_FACTOR);
        arrow[3].y = Round(y - j*A_WIDTH_FACTOR*dx);

        arrow[4].x = d_x;
        arrow[4].y = d_y;

        arrow[5].x = Round(x - j*A_WIDTH_FACTOR);
        arrow[5].y = Round(y + j*A_WIDTH_FACTOR*dx);

        arrow[6].x = Round(x - j);
        arrow[6].y = Round(y + j*dx);
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
        d_y += squareSize / 4;
    }
    else if( d_y < s_y ) {
        d_y += 3 * squareSize / 4;
    }
    else {
        d_y += squareSize / 2;
    }

    if( d_x > s_x ) {
        d_x += squareSize / 4;
    }
    else if( d_x < s_x ) {
        d_x += 3 * squareSize / 4;
    }
    else {
        d_x += squareSize / 2;
    }

    s_x += squareSize / 2;
    s_y += squareSize / 2;

    /* Adjust width */
    A_WIDTH = squareSize / 14;

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
        if( (appData.highlightLastMove || appData.highlightDragging) && IsDrawArrowEnabled() && HasHighlightInfo() ) {
            result = TRUE;
        }
    }

    return result;
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
      if( backTextureSquareSize != squareSize ) {
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
      if(!strcmp(appData.variant, "xiangqi") ) {
          square_color = !InPalace(row, column);
          if(BOARD_HEIGHT&1) { if(row==BOARD_HEIGHT/2) square_color ^= 1; }
          else if(row < BOARD_HEIGHT/2) square_color ^= 1;
      }
      piece_color = (int) piece < (int) BlackPawn;


#ifdef FAIRY
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
            DisplayHoldingsCount(hdc, x, y, 0, (int) board[row][column]);
      else if( column == BOARD_RGHT) /* right align */
            DisplayHoldingsCount(hdc, x, y, 1, (int) board[row][column]);
      else
#endif
      if (appData.monoMode) {
        if (piece == EmptySquare) {
          BitBlt(hdc, x, y, squareSize, squareSize, 0, 0, 0,
		 square_color ? WHITENESS : BLACKNESS);
        } else {
          DrawPieceOnDC(hdc, piece, piece_color, square_color, x, y, tmphdc);
        }
      } 
      else if( backTextureSquareInfo[row][column].mode > 0 ) {
          /* [AS] Draw the square using a texture bitmap */
          HBITMAP hbm = SelectObject( texture_hdc, square_color ? liteBackTexture : darkBackTexture );

          DrawTile( x, y, 
              squareSize, squareSize, 
              hdc, 
              texture_hdc,
              backTextureSquareInfo[row][column].mode,
              backTextureSquareInfo[row][column].x,
              backTextureSquareInfo[row][column].y );

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

#define MAX_CLIPS 200   /* more than enough */

VOID
HDCDrawPosition(HDC hdc, BOOLEAN repaint, Board board)
{
  static Board lastReq, lastDrawn;
  static HighlightInfo lastDrawnHighlight, lastDrawnPremove;
  static int lastDrawnFlipView = 0;
  static int lastReqValid = 0, lastDrawnValid = 0;
  int releaseDC, x, y, x2, y2, row, column, num_clips = 0, i;
  HDC tmphdc;
  HDC hdcmem;
  HBITMAP bufferBitmap;
  HBITMAP oldBitmap;
  RECT Rect;
  HRGN clips[MAX_CLIPS];
  ChessSquare dragged_piece = EmptySquare;

  /* I'm undecided on this - this function figures out whether a full
   * repaint is necessary on its own, so there's no real reason to have the
   * caller tell it that.  I think this can safely be set to FALSE - but
   * if we trust the callers not to request full repaints unnessesarily, then
   * we could skip some clipping work.  In other words, only request a full
   * redraw when the majority of pieces have changed positions (ie. flip, 
   * gamestart and similar)  --Hawk
   */
  Boolean fullrepaint = repaint;

  if( DrawPositionNeedsFullRepaint() ) {
      fullrepaint = TRUE;
  }

#if 0
  if( fullrepaint ) {
      static int repaint_count = 0;
      char buf[128];

      repaint_count++;
      sprintf( buf, "FULL repaint: %d\n", repaint_count );
      OutputDebugString( buf );
  }
#endif

  if (board == NULL) {
    if (!lastReqValid) {
      return;
    }
    board = lastReq;
  } else {
    CopyBoard(lastReq, board);
    lastReqValid = 1;
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

#if 0
  fprintf(debugFP, "*******************************\n"
                   "repaint = %s\n"
                   "dragInfo.from (%d,%d)\n"
                   "dragInfo.start (%d,%d)\n"
                   "dragInfo.pos (%d,%d)\n"
                   "dragInfo.lastpos (%d,%d)\n", 
                    repaint ? "TRUE" : "FALSE",
                    dragInfo.from.x, dragInfo.from.y, 
                    dragInfo.start.x, dragInfo.start.y,
                    dragInfo.pos.x, dragInfo.pos.y,
                    dragInfo.lastpos.x, dragInfo.lastpos.y);
  fprintf(debugFP, "prev:  ");
  for (row = 0; row < BOARD_HEIGHT; row++) {
    for (column = 0; column < BOARD_WIDTH; column++) {
      fprintf(debugFP, "%d ", lastDrawn[row][column]);
    }
  }
  fprintf(debugFP, "\n");
  fprintf(debugFP, "board: ");
  for (row = 0; row < BOARD_HEIGHT; row++) {
    for (column = 0; column < BOARD_WIDTH; column++) {
      fprintf(debugFP, "%d ", board[row][column]);
    }
  }
  fprintf(debugFP, "\n");
  fflush(debugFP);
#endif

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
        board[dragInfo.from.y][dragInfo.from.x] = EmptySquare;
  }

  /* Figure out which squares need updating by comparing the 
   * newest board with the last drawn board and checking if
   * flipping has changed.
   */
  if (!fullrepaint && lastDrawnValid && lastDrawnFlipView == flipView) {
    for (row = 0; row < BOARD_HEIGHT; row++) { /* [HGM] true size, not 8 */
      for (column = 0; column < BOARD_WIDTH; column++) {
	if (lastDrawn[row][column] != board[row][column]) {
	  SquareToPos(row, column, &x, &y);
	  clips[num_clips++] =
	    CreateRectRgn(x, y, x + squareSize, y + squareSize);
	}
      }
    }
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
	 we have to remember to erase it. */
      lastDrawn[animInfo.to.y][animInfo.to.x] = animInfo.piece;
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
  DrawGridOnDC(hdcmem);
  DrawHighlightsOnDC(hdcmem);
  DrawBoardOnDC(hdcmem, board, tmphdc);

  if( appData.highlightMoveWithArrow ) {
    DrawArrowHighlight(hdcmem);
  }

  DrawCoordsOnDC(hdcmem);

  CopyBoard(lastDrawn, board); /* [HGM] Moved to here from end of routine, */
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
    DrawPieceOnDC(hdcmem, dragged_piece,
		  ((int) dragged_piece < (int) BlackPawn), 
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
  BitBlt(hdc, boardRect.left, boardRect.top,
	 boardRect.right - boardRect.left,
	 boardRect.bottom - boardRect.top,
	 tmphdc, boardRect.left, boardRect.top, SRCCOPY);
  SelectObject(tmphdc, oldBitmap);

  /* Massive cleanup */
  for (x = 0; x < num_clips; x++)
    DeleteObject(clips[x]);

  DeleteDC(tmphdc);
  DeleteObject(bufferBitmap);

  if (releaseDC) 
    ReleaseDC(hwndMain, hdc);
  
  if (lastDrawnFlipView != flipView) {
    if (flipView)
      CheckMenuItem(GetMenu(hwndMain),IDM_FlipView, MF_BYCOMMAND|MF_CHECKED);
    else
      CheckMenuItem(GetMenu(hwndMain),IDM_FlipView, MF_BYCOMMAND|MF_UNCHECKED);
  }

/*  CopyBoard(lastDrawn, board);*/
  lastDrawnHighlight = highlightInfo;
  lastDrawnPremove   = premoveHighlightInfo;
  lastDrawnFlipView = flipView;
  lastDrawnValid = 1;
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

  if(hdc = BeginPaint(hwnd, &ps)) {
    if (IsIconic(hwnd)) {
      DrawIcon(hdc, 2, 2, iconCurrent);
    } else {
      if (!appData.monoMode) {
	SelectPalette(hdc, hPal, FALSE);
	RealizePalette(hdc);
      }
      HDCDrawPosition(hdc, 1, NULL);
      oldFont =
	SelectObject(hdc, font[boardSize][MESSAGE_FONT]->hf);
      ExtTextOut(hdc, messageRect.left, messageRect.top,
		 ETO_CLIPPED|ETO_OPAQUE,
		 &messageRect, messageText, strlen(messageText), NULL);
      SelectObject(hdc, oldFont);
      DisplayBothClocks();
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
int
EventToSquare(int x)
{
  if (x <= 0)
    return -2;
  if (x < lineGap)
    return -1;
  x -= lineGap;
  if ((x % (squareSize + lineGap)) >= squareSize)
    return -1;
  x /= (squareSize + lineGap);
  if (x >= BOARD_SIZE)
    return -2;
  return x;
}

typedef struct {
  char piece;
  int command;
  char* name;
} DropEnable;

DropEnable dropEnables[] = {
  { 'P', DP_Pawn, "Pawn" },
  { 'N', DP_Knight, "Knight" },
  { 'B', DP_Bishop, "Bishop" },
  { 'R', DP_Rook, "Rook" },
  { 'Q', DP_Queen, "Queen" },
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
    sprintf(item, "%s  %d", dropEnables[i].name, count);
    enable = count > 0 || !appData.testLegality
      /*!!temp:*/ || (gameInfo.variant == VariantCrazyhouse
		      && !appData.icsActive);
    ModifyMenu(hmenu, dropEnables[i].command,
	       MF_BYCOMMAND | (enable ? MF_ENABLED : MF_GRAYED) | MF_STRING,
	       dropEnables[i].command, item);
  }
}

static int fromX = -1, fromY = -1, toX, toY;

/* Event handler for mouse messages */
VOID
MouseEvent(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  int x, y;
  POINT pt;
  static int recursive = 0;
  HMENU hmenu;
  BOOLEAN needsRedraw = FALSE;
  BOOLEAN saveAnimate;
  BOOLEAN forceFullRepaint = IsFullRepaintPreferrable(); /* [AS] */
  static BOOLEAN sameAgain = FALSE;
  ChessMove moveType;

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
  x = EventToSquare(pt.x - boardRect.left);
  y = EventToSquare(pt.y - boardRect.top);
  if (!flipView && y >= 0) {
    y = BOARD_HEIGHT - 1 - y;
  }
  if (flipView && x >= 0) {
    x = BOARD_WIDTH - 1 - x;
  }

  switch (message) {
  case WM_LBUTTONDOWN:
    ErrorPopDown();
    sameAgain = FALSE;
    if (y == -2) {
      /* Downclick vertically off board; check if on clock */
      if (PtInRect((LPRECT) &whiteRect, pt)) {
        if (gameMode == EditPosition) {
	  SetWhiteToPlayEvent();
	} else if (gameMode == IcsPlayingBlack ||
		   gameMode == MachinePlaysWhite) {
	  CallFlagEvent();
        } else if (gameMode == EditGame) {
          AdjustClock(flipClock, -1);
        }
      } else if (PtInRect((LPRECT) &blackRect, pt)) {
	if (gameMode == EditPosition) {
	  SetBlackToPlayEvent();
	} else if (gameMode == IcsPlayingWhite ||
		   gameMode == MachinePlaysBlack) {
	  CallFlagEvent();
        } else if (gameMode == EditGame) {
          AdjustClock(!flipClock, -1);
	}
      }
      if (!appData.highlightLastMove) {
        ClearHighlights();
	DrawPosition(forceFullRepaint || FALSE, NULL);
      }
      fromX = fromY = -1;
      dragInfo.start.x = dragInfo.start.y = -1;
      dragInfo.from = dragInfo.start;
      break;
    } else if (x < 0 || y < 0
      /* [HGM] block clicks between board and holdings */
              || x == BOARD_LEFT-1 || x == BOARD_RGHT
              || x == BOARD_LEFT-2 && y < BOARD_HEIGHT-gameInfo.holdingsSize
              || x == BOARD_RGHT+1 && y >= gameInfo.holdingsSize
	/* EditPosition, empty square, or different color piece;
	   click-click move is possible */
                               ) {
      break;
    } else if (fromX == x && fromY == y) {
      /* Downclick on same square again */
      ClearHighlights();
      DrawPosition(forceFullRepaint || FALSE, NULL);
      sameAgain = TRUE;  
    } else if (fromX != -1 &&
               x != BOARD_LEFT-2 && x != BOARD_RGHT+1 
                                                                        ) {
      /* Downclick on different square. */
      /* [HGM] if on holdings file, should count as new first click ! */
      { /* [HGM] <sameColor> now always do UserMoveTest(), and check colors there */
	toX = x;
	toY = y;
        /* [HGM] <popupFix> UserMoveEvent requires two calls now,
           to make sure move is legal before showing promotion popup */
        moveType = UserMoveTest(fromX, fromY, toX, toY, NULLCHAR);
        if(moveType != ImpossibleMove) {
          /* [HGM] We use PromotionToKnight in Shogi to indicate frorced promotion */
          if (moveType == WhitePromotionKnight || moveType == BlackPromotionKnight ||
             (moveType == WhitePromotionQueen || moveType == BlackPromotionQueen) &&
              appData.alwaysPromoteToQueen) {
                  FinishMove(moveType, fromX, fromY, toX, toY, 'q');
                  if (!appData.highlightLastMove) {
                      ClearHighlights();
                      DrawPosition(forceFullRepaint || FALSE, NULL);
                  }
          } else
          if (moveType == WhitePromotionQueen || moveType == BlackPromotionQueen ) {
                  SetHighlights(fromX, fromY, toX, toY);
                  DrawPosition(forceFullRepaint || FALSE, NULL);
                  /* [HGM] <popupFix> Popup calls FinishMove now.
                     If promotion to Q is legal, all are legal! */
                  PromotionPopup(hwnd);
          } else {       /* not a promotion */
             if (appData.animate || appData.highlightLastMove) {
                 SetHighlights(fromX, fromY, toX, toY);
             } else {
                 ClearHighlights();
             }
             FinishMove(moveType, fromX, fromY, toX, toY, NULLCHAR);
             if (appData.animate && !appData.highlightLastMove) {
                  ClearHighlights();
                  DrawPosition(forceFullRepaint || FALSE, NULL);
             }
          }
        }
	if (gotPremove) SetPremoveHighlights(fromX, fromY, toX, toY);
	fromX = fromY = -1;
	break;
      }
      ClearHighlights();
      DrawPosition(forceFullRepaint || FALSE, NULL);
    }
    /* First downclick, or restart on a square with same color piece */
    if (!frozen && OKToStartUserMove(x, y)) {
      fromX = x;
      fromY = y;
      dragInfo.lastpos = pt;
      dragInfo.from.x = fromX;
      dragInfo.from.y = fromY;
      dragInfo.start = dragInfo.from;
      SetCapture(hwndMain);
    } else {
      fromX = fromY = -1;
      dragInfo.start.x = dragInfo.start.y = -1;
      dragInfo.from = dragInfo.start;
      DrawPosition(forceFullRepaint || FALSE, NULL); /* [AS] */
    }
    break;

  case WM_LBUTTONUP:
    ReleaseCapture();
    if (fromX == -1) break;
    if (x == fromX && y == fromY) {
      /* Upclick on same square */
      if (sameAgain) {
	/* Clicked same square twice: abort click-click move */
	fromX = fromY = -1;
	gotPremove = 0;
	ClearPremoveHighlights();
      } else {
	/* First square clicked: start click-click move */
	SetHighlights(fromX, fromY, -1, -1);
      }
      dragInfo.from.x = dragInfo.from.y = -1;
      DrawPosition(forceFullRepaint || FALSE, NULL);
    } else if (dragInfo.from.x < 0 || dragInfo.from.y < 0) {
      /* Errant click; ignore */
      break;
    } else {
      /* Finish drag move. */
    if (appData.debugMode) {
        fprintf(debugFP, "release\n");
    }
      dragInfo.from.x = dragInfo.from.y = -1;
      toX = x;
      toY = y;
      saveAnimate = appData.animate; /* sorry, Hawk :) */
      appData.animate = appData.animate && !appData.animateDragging;
      moveType = UserMoveTest(fromX, fromY, toX, toY, NULLCHAR);
      if(moveType != ImpossibleMove) {
          /* [HGM] use move type to determine if move is promotion.
             Knight is Shogi kludge for mandatory promotion, Queen means choice */
          if (moveType == WhitePromotionKnight || moveType == BlackPromotionKnight ||
             (moveType == WhitePromotionQueen || moveType == BlackPromotionQueen) &&
              appData.alwaysPromoteToQueen) 
               FinishMove(moveType, fromX, fromY, toX, toY, 'q');
          else 
          if (moveType == WhitePromotionQueen || moveType == BlackPromotionQueen ) {
               DrawPosition(forceFullRepaint || FALSE, NULL);
               PromotionPopup(hwnd); /* [HGM] Popup now calls FinishMove */
        } else FinishMove(moveType, fromX, fromY, toX, toY, NULLCHAR);
      }
      if (gotPremove) SetPremoveHighlights(fromX, fromY, toX, toY);
      appData.animate = saveAnimate;
      fromX = fromY = -1;
      if (appData.highlightDragging && !appData.highlightLastMove) {
	ClearHighlights();
      }
      if (appData.animate || appData.animateDragging ||
	  appData.highlightDragging || gotPremove) {
	DrawPosition(forceFullRepaint || FALSE, NULL);
      }
    }
    dragInfo.start.x = dragInfo.start.y = -1; 
    dragInfo.pos = dragInfo.lastpos = dragInfo.start;
    break;

  case WM_MOUSEMOVE:
    if ((appData.animateDragging || appData.highlightDragging)
	&& (wParam & MK_LBUTTON)
	&& dragInfo.from.x >= 0) 
    {
      BOOL full_repaint = FALSE;

      sameAgain = FALSE; /* [HGM] if we drag something around, do keep square selected */
      if (appData.animateDragging) {
	dragInfo.pos = pt;
      }
      if (appData.highlightDragging) {
	SetHighlights(fromX, fromY, x, y);
        if( IsDrawArrowEnabled() && (x < 0 || x >= BOARD_WIDTH || y < 0 || y >= BOARD_HEIGHT) ) {
            full_repaint = TRUE;
        }
      }
      
      DrawPosition( full_repaint, NULL);
      
      dragInfo.lastpos = dragInfo.pos;
    }
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
          if (gameMode == EditGame) AdjustClock(flipClock, 1);
      } else if (PtInRect((LPRECT) &blackRect, pt)) {
          if (gameMode == EditGame) AdjustClock(!flipClock, 1);
      }
    }
    DrawPosition(TRUE, NULL);

    switch (gameMode) {
    case EditPosition:
    case IcsExamining:
      if (x < 0 || y < 0) break;
      fromX = x;
      fromY = y;
      if (message == WM_MBUTTONDOWN) {
	buttonCount = 3;  /* even if system didn't think so */
	if (wParam & MK_SHIFT) 
	  MenuPopup(hwnd, pt, LoadMenu(hInst, "BlackPieceMenu"), -1);
	else
	  MenuPopup(hwnd, pt, LoadMenu(hInst, "WhitePieceMenu"), -1);
      } else { /* message == WM_RBUTTONDOWN */
#if 0
	if (buttonCount == 3) {
	  if (wParam & MK_SHIFT) 
	    MenuPopup(hwnd, pt, LoadMenu(hInst, "WhitePieceMenu"), -1);
	  else
	    MenuPopup(hwnd, pt, LoadMenu(hInst, "BlackPieceMenu"), -1);
	} else {
	  MenuPopup(hwnd, pt, LoadMenu(hInst, "PieceMenu"), -1);
	}
#else
	/* Just have one menu, on the right button.  Windows users don't
	   think to try the middle one, and sometimes other software steals
	   it, or it doesn't really exist. */
        if(gameInfo.variant != VariantShogi)
            MenuPopup(hwnd, pt, LoadMenu(hInst, "PieceMenu"), -1);
        else
            MenuPopup(hwnd, pt, LoadMenu(hInst, "ShogiPieceMenu"), -1);
#endif
      }
      break;
    case IcsPlayingWhite:
    case IcsPlayingBlack:
    case EditGame:
    case MachinePlaysWhite:
    case MachinePlaysBlack:
      if (appData.testLegality &&
	  gameInfo.variant != VariantBughouse &&
	  gameInfo.variant != VariantCrazyhouse) break;
      if (x < 0 || y < 0) break;
      fromX = x;
      fromY = y;
      hmenu = LoadMenu(hInst, "DropPieceMenu");
      SetupDropMenu(hmenu);
      MenuPopup(hwnd, pt, hmenu, -1);
      break;
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
  int id = GetWindowLong(hwnd, GWL_ID);
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
    case '\t':
      if (appData.icsActive) {
	if (GetKeyState(VK_SHIFT) < 0) {
	  /* shifted */
	  HWND h = GetDlgItem(hwndConsole, OPT_ConsoleInput);
	  if (IsIconic(hwndConsole)) ShowWindow(hwndConsole, SW_RESTORE);
	  SetFocus(h);
	} else {
	  /* unshifted */
	  HWND h = GetDlgItem(hwndConsole, OPT_ConsoleText);
	  if (IsIconic(hwndConsole)) ShowWindow(hwndConsole, SW_RESTORE);
	  SetFocus(h);
	}
	return TRUE;
      }
      break;
    default:
      if (appData.icsActive) {
        HWND h = GetDlgItem(hwndConsole, OPT_ConsoleInput);
	if (IsIconic(hwndConsole)) ShowWindow(hwndConsole, SW_RESTORE);
	SetFocus(h);
	SendMessage(h, WM_CHAR, wParam, lParam);
	return TRUE;
      } else if (isalpha((char)wParam) || isdigit((char)wParam)){
	PopUpMoveDialog((char)wParam);
      }
      break;
    }
    break;
  }
  return CallWindowProc(buttonDesc[i].wndproc, hwnd, message, wParam, lParam);
}

/* Process messages for Promotion dialog box */
LRESULT CALLBACK
Promotion(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  char promoChar;

  switch (message) {
  case WM_INITDIALOG: /* message: initialize dialog box */
    /* Center the dialog over the application window */
    CenterWindow(hDlg, GetWindow(hDlg, GW_OWNER));
    ShowWindow(GetDlgItem(hDlg, PB_King), 
      (!appData.testLegality || gameInfo.variant == VariantSuicide ||
       gameInfo.variant == VariantGiveaway) ?
	       SW_SHOW : SW_HIDE);
    /* [HGM] Only allow C & A promotions if these pieces are defined */
    ShowWindow(GetDlgItem(hDlg, PB_Archbishop),
       (PieceToChar(WhiteCardinal) != '.' ||
        PieceToChar(BlackCardinal) != '.'   ) ?
	       SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hDlg, PB_Chancellor), 
       (PieceToChar(WhiteMarshall) != '.' ||
        PieceToChar(BlackMarshall) != '.'   ) ?
	       SW_SHOW : SW_HIDE);
    /* [HGM] Hide B & R button in Shogi, use Q as promote, N as defer */
    ShowWindow(GetDlgItem(hDlg, PB_Rook),
       gameInfo.variant != VariantShogi ?
	       SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hDlg, PB_Bishop), 
       gameInfo.variant != VariantShogi ?
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
      promoChar = PieceToChar(BlackKing);
      break;
    case PB_Queen:
      promoChar = gameInfo.variant == VariantShogi ? '+' : PieceToChar(BlackQueen);
      break;
    case PB_Rook:
      promoChar = PieceToChar(BlackRook);
      break;
    case PB_Bishop:
      promoChar = PieceToChar(BlackBishop);
      break;
    case PB_Chancellor:
      promoChar = PieceToChar(BlackMarshall);
      break;
    case PB_Archbishop:
      promoChar = PieceToChar(BlackCardinal);
      break;
    case PB_Knight:
      promoChar = gameInfo.variant == VariantShogi ? '=' : PieceToChar(BlackKnight);
      break;
    default:
      return FALSE;
    }
    EndDialog(hDlg, TRUE); /* Exit the dialog */
    /* [HGM] <popupFix> Call FinishMove rather than UserMoveEvent, as we
       only show the popup when we are already sure the move is valid or
       legal. We pass a faulty move type, but the kludge is that FinishMove
       will figure out it is a promotion from the promoChar. */
    FinishMove(NormalMove, fromX, fromY, toX, toY, promoChar);
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

/* Toggle ShowThinking */
VOID
ToggleShowThinking()
{
  ShowThinkingEvent(!appData.showThinking);
}

VOID
LoadGameDialog(HWND hwnd, char* title)
{
  UINT number = 0;
  FILE *f;
  char fileTitle[MSG_SIZ];
  f = OpenFileDialog(hwnd, FALSE, "",
 	             appData.oldSaveStyle ? "gam" : "pgn",
		     GAME_FILT,
		     title, &number, fileTitle, NULL);
  if (f != NULL) {
    cmailMsgLoaded = FALSE;
    if (number == 0) {
      int error = GameListBuild(f);
      if (error) {
        DisplayError("Cannot build game list", error);
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
  strcpy(cfmt.szFaceName, font[boardSize][CONSOLE_FONT]->mfp.faceName);
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
  int wmId, wmEvent;
  char *defName;
  FILE *f;
  UINT number;
  char fileTitle[MSG_SIZ];
  static SnapData sd;

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
    MouseEvent(hwnd, message, wParam, lParam);
    break;

  case WM_CHAR:
    
    if (appData.icsActive) {
      if (wParam == '\t') {
	if (GetKeyState(VK_SHIFT) < 0) {
	  /* shifted */
	  HWND h = GetDlgItem(hwndConsole, OPT_ConsoleInput);
	  if (IsIconic(hwndConsole)) ShowWindow(hwndConsole, SW_RESTORE);
	  SetFocus(h);
	} else {
	  /* unshifted */
	  HWND h = GetDlgItem(hwndConsole, OPT_ConsoleText);
	  if (IsIconic(hwndConsole)) ShowWindow(hwndConsole, SW_RESTORE);
	  SetFocus(h);
	}
      } else {
	HWND h = GetDlgItem(hwndConsole, OPT_ConsoleInput);
	if (IsIconic(hwndConsole)) ShowWindow(hwndConsole, SW_RESTORE);
	SetFocus(h);
	SendMessage(h, message, wParam, lParam);
      }
    } else if (isalpha((char)wParam) || isdigit((char)wParam)) {
      PopUpMoveDialog((char)wParam);
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
#if 0
        UpdateColors(hdc);
#else
        InvalidateRect(hwnd, &boardRect, FALSE);/*faster!*/
#endif
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
    wmEvent = HIWORD(wParam);

    switch (wmId) {
    case IDM_NewGame:
      ResetGameEvent();
      AnalysisPopDown();
      break;

    case IDM_NewGameFRC:
      if( NewGameFRC() == 0 ) {
        ResetGameEvent();
        AnalysisPopDown();
      }
      break;

    case IDM_NewVariant:
      NewVariantPopup(hwnd);
      break;

    case IDM_LoadGame:
      LoadGameDialog(hwnd, "Load Game from File");
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
      f = OpenFileDialog(hwnd, FALSE, "",
			 appData.oldSaveStyle ? "pos" : "fen",
			 POSITION_FILT,
			 "Load Position from File", &number, fileTitle, NULL);
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
      f = OpenFileDialog(hwnd, TRUE, defName,
			 appData.oldSaveStyle ? "gam" : "pgn",
			 GAME_FILT,
			 "Save Game to File", NULL, fileTitle, NULL);
      if (f != NULL) {
	SaveGame(f, 0, "");
      }
      break;

    case IDM_SavePosition:
      defName = DefaultFileName(appData.oldSaveStyle ? "pos" : "fen");
      f = OpenFileDialog(hwnd, TRUE, defName,
			 appData.oldSaveStyle ? "pos" : "fen",
			 POSITION_FILT,
			 "Save Position to File", NULL, fileTitle, NULL);
      if (f != NULL) {
	SavePosition(f, 0, "");
      }
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
      break;

    case IDM_AnalysisMode:
      if (!first.analysisSupport) {
        char buf[MSG_SIZ];
        sprintf(buf, "%s does not support analysis", first.tidy);
        DisplayError(buf, 0);
      } else {
	if (!appData.showThinking) ToggleShowThinking();
	AnalyzeModeEvent();
      }
      break;

    case IDM_AnalyzeFile:
      if (!first.analysisSupport) {
        char buf[MSG_SIZ];
        sprintf(buf, "%s does not support analysis", first.tidy);
        DisplayError(buf, 0);
      } else {
	if (!appData.showThinking) ToggleShowThinking();
	AnalyzeFileEvent();
	LoadGameDialog(hwnd, "Analyze Game from File");
	AnalysisPeriodicEvent(1);
      }
      break;

    case IDM_IcsClient:
      IcsClientEvent();
      break;

    case IDM_EditGame:
      EditGameEvent();
      break;

    case IDM_EditPosition:
      EditPositionEvent();
      break;

    case IDM_Training:
      TrainingEvent();
      break;

    case IDM_ShowGameList:
      ShowGameListProc();
      break;

    case IDM_EditTags:
      EditTagsProc();
      break;

    case IDM_EditComment:
      if (commentDialogUp && editComment) {
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

    case IDM_TypeInMove:
      PopUpMoveDialog('\000');
      break;

    case IDM_Backward:
      BackwardEvent();
      SetFocus(hwndMain);
      break;

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

    case IDM_Revert:
      RevertEvent();
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
      break;

    case IDM_GeneralOptions:
      GeneralOptionsPopup(hwnd);
      DrawPosition(TRUE, NULL);
      break;

    case IDM_BoardOptions:
      BoardOptionsPopup(hwnd);
      break;

    case IDM_EnginePlayOptions:
      EnginePlayOptionsPopup(hwnd);
      break;

    case IDM_OptionsUCI:
      UciOptionsPopup(hwnd);
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
      if (!WinHelp (hwnd, "winboard.hlp", HELP_KEY,(DWORD)(LPSTR)"CONTENTS")) {
	MessageBox (GetFocus(),
		    "Unable to activate help",
		    szAppName, MB_SYSTEMMODAL|MB_OK|MB_ICONHAND);
      }
      break;

    case IDM_HELPSEARCH:
      if (!WinHelp(hwnd, "winboard.hlp", HELP_PARTIALKEY, (DWORD)(LPSTR)"")) {
	MessageBox (GetFocus(),
		    "Unable to activate help",
		    szAppName, MB_SYSTEMMODAL|MB_OK|MB_ICONHAND);
      }
      break;

    case IDM_HELPHELP:
      if(!WinHelp(hwnd, (LPSTR)NULL, HELP_HELPONHELP, 0)) {
	MessageBox (GetFocus(),
		    "Unable to activate help",
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
      AskQuestionEvent("Direct Command",
		       "Send to chess program:", "", "1");
      break;
    case IDM_DirectCommand2:
      AskQuestionEvent("Direct Command",
		       "Send to second chess program:", "", "2");
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
      EditPositionMenuEvent(WhiteCardinal, fromX, fromY);
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
      EditPositionMenuEvent(BlackCardinal, fromX, fromY);
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

    default:
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
      if ((gameMode == AnalyzeMode || gameMode == AnalyzeFile) && 
	  appData.periodicUpdates) {
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

        if( ((lpwp->flags & SWP_NOMOVE) == 0) && ((lpwp->flags & SWP_NOSIZE) != 0) ) {
            /* Window is moving */
            RECT rcMain;

            GetWindowRect( hwnd, &rcMain );
            
            ReattachAfterMove( &rcMain, lpwp->x, lpwp->y, moveHistoryDialog, &wpMoveHistory );
            ReattachAfterMove( &rcMain, lpwp->x, lpwp->y, evalGraphDialog, &wpEvalGraph );
            ReattachAfterMove( &rcMain, lpwp->x, lpwp->y, engineOutputDialog, &wpEngineOutput );
        }
    }
    break;

  /* [AS] Snapping */
  case WM_ENTERSIZEMOVE:
    if (hwnd == hwndMain) {
      doingSizing = TRUE;
      lastSizing = 0;
    }
    return OnEnterSizeMove( &sd, hwnd, wParam, lParam );
    break;

  case WM_SIZING:
    if (hwnd == hwndMain) {
      lastSizing = wParam;
    }
    break;

  case WM_MOVING:
      return OnMoving( &sd, hwnd, wParam, lParam );

  case WM_EXITSIZEMOVE:
    if (hwnd == hwndMain) {
      RECT client;
      doingSizing = FALSE;
      InvalidateRect(hwnd, &boardRect, FALSE);
      GetClientRect(hwnd, &client);
      ResizeBoard(client.right, client.bottom, lastSizing);
      lastSizing = 0;
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

  if (ms->data) free(ms->data);
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
    if (fread(ms->data, st.st_size, 1, f) < 1) break;
    fclose(f);
    ok = TRUE;
    break;
  }
  if (!ok) {
    char buf[MSG_SIZ];
    sprintf(buf, "Error loading sound %s", ms->name);
    DisplayError(buf, GetLastError());
  }
  return ok;
}

BOOLEAN
MyPlaySound(MySound *ms)
{
  BOOLEAN ok = FALSE;
  switch (ms->name[0]) {
  case NULLCHAR:
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
#if 0
  if (!ok) {
    char buf[MSG_SIZ];
    sprintf(buf, "Error playing sound %s", ms->name);
    DisplayError(buf, GetLastError());
  }
#endif
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
OpenFileDialog(HWND hwnd, BOOL write, char *defName, char *defExt,
	       char *nameFilt, char *dlgTitle, UINT *number,
	       char fileTitle[MSG_SIZ], char fileName[MSG_SIZ])
{
  OPENFILENAME openFileName;
  char buf1[MSG_SIZ];
  FILE *f;

  if (fileName == NULL) fileName = buf1;
  if (defName == NULL) {
    strcpy(fileName, "*.");
    strcat(fileName, defExt);
  } else {
    strcpy(fileName, defName);
  }
  if (fileTitle) strcpy(fileTitle, "");
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
    | (write ? 0 : OFN_FILEMUSTEXIST) 
    | (number ? OFN_ENABLETEMPLATE | OFN_ENABLEHOOK: 0)
    | (oldDialog ? 0 : OFN_EXPLORER);
  openFileName.nFileOffset       = 0;
  openFileName.nFileExtension    = 0;
  openFileName.lpstrDefExt       = defExt;
  openFileName.lCustData         = (LONG) number;
  openFileName.lpfnHook          = oldDialog ?
    (LPOFNHOOKPROC) OldOpenFileHook : (LPOFNHOOKPROC) OpenFileHook;
  openFileName.lpTemplateName    = (LPSTR)(oldDialog ? 1536 : DLG_IndexNumber);

  if (write ? GetSaveFileName(&openFileName) : 
              GetOpenFileName(&openFileName)) {
    /* open the file */
    f = fopen(openFileName.lpstrFile, write ? "a" : "rb");
    if (f == NULL) {
      MessageBox(hwnd, "File open failed", NULL,
		 MB_OK|MB_ICONEXCLAMATION);
      return NULL;
    }
  } else {
    int err = CommDlgExtendedError();
    if (err != 0) DisplayError("Internal error in file dialog box", err);
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
    SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM) *cd);
    cd++;
  }
}

void
InitComboStringsFromOption(HANDLE hwndCombo, char *str)
{
  char buf1[ARG_MAX];
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
    appData.zippyPlay && IsDlgButtonChecked(hDlg, OPT_ChessServer));
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
  sprintf(buf, "%s%s%s", q, nthcp, q);
  if (*nthdir != NULLCHAR) {
    q = QuoteForFilename(nthdir);
    sprintf(buf + strlen(buf), " /%s=%s%s%s", nthd, q, nthdir, q);
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
    /* Initialize the dialog items */
    InitEngineBox(hDlg, GetDlgItem(hDlg, OPT_ChessEngineName),
	          appData.firstChessProgram, "fd", appData.firstDirectory,
		  firstChessProgramNames);
    InitEngineBox(hDlg, GetDlgItem(hDlg, OPT_SecondChessEngineName),
	          appData.secondChessProgram, "sd", appData.secondDirectory,
		  secondChessProgramNames);
    hwndCombo = GetDlgItem(hDlg, OPT_ChessServerName);
    InitComboStringsFromOption(hwndCombo, icsNames);    
    sprintf(buf, "%s /icsport=%s", appData.icsHost, appData.icsPort);
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
        strcpy(buf, "/fcp=");
	GetDlgItemText(hDlg, OPT_ChessEngineName, buf + strlen(buf), sizeof(buf) - strlen(buf));
        p = buf;
	ParseArgs(StringGet, &p);
        strcpy(buf, "/scp=");
	GetDlgItemText(hDlg, OPT_SecondChessEngineName, buf + strlen(buf), sizeof(buf) - strlen(buf));
        p = buf;
	ParseArgs(StringGet, &p);
	appData.noChessProgram = FALSE;
	appData.icsActive = FALSE;
      } else if (IsDlgButtonChecked(hDlg, OPT_ChessServer)) {
        strcpy(buf, "/ics /icshost=");
	GetDlgItemText(hDlg, OPT_ChessServerName, buf + strlen(buf), sizeof(buf) - strlen(buf));
        p = buf;
	ParseArgs(StringGet, &p);
	if (appData.zippyPlay) {
	  strcpy(buf, "/fcp=");
  	  GetDlgItemText(hDlg, OPT_ChessEngineName, buf + strlen(buf), sizeof(buf) - strlen(buf));
	  p = buf;
	  ParseArgs(StringGet, &p);
	}
      } else if (IsDlgButtonChecked(hDlg, OPT_View)) {
	appData.noChessProgram = TRUE;
	appData.icsActive = FALSE;
      } else {
	MessageBox(hDlg, "Choose an option, or cancel to exit",
		   "Option Error", MB_OK|MB_ICONEXCLAMATION);
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
		    "Unable to activate help",
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
  int len, newSizeX, newSizeY, flags;
  static int sizeX, sizeY;
  char *str;
  RECT rect;
  MINMAXINFO *mmi;

  switch (message) {
  case WM_INITDIALOG: /* message: initialize dialog box */
    /* Initialize the dialog items */
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
      flags = SWP_NOZORDER;
      GetClientRect(hDlg, &rect);
      sizeX = rect.right;
      sizeY = rect.bottom;
      if (commentX != CW_USEDEFAULT && commentY != CW_USEDEFAULT &&
	  commentW != CW_USEDEFAULT && commentH != CW_USEDEFAULT) {
	WINDOWPLACEMENT wp;
	EnsureOnScreen(&commentX, &commentY);
	wp.length = sizeof(WINDOWPLACEMENT);
	wp.flags = 0;
	wp.showCmd = SW_SHOW;
	wp.ptMaxPosition.x = wp.ptMaxPosition.y = 0;
	wp.rcNormalPosition.left = commentX;
	wp.rcNormalPosition.right = commentX + commentW;
	wp.rcNormalPosition.top = commentY;
	wp.rcNormalPosition.bottom = commentY + commentH;
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

  CheckMenuItem(GetMenu(hwndMain), IDM_EditComment, edit ? MF_CHECKED : MF_UNCHECKED);

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
    if (!commentDialogUp) ShowWindow(commentDialog, SW_SHOW);
  } else {
    lpProc = MakeProcInstance((FARPROC)CommentDialog, hInst);
    CreateDialog(hInst, MAKEINTRESOURCE(DLG_EditComment),
		 hwndMain, (DLGPROC)lpProc);
    FreeProcInstance(lpProc);
  }
  commentDialogUp = TRUE;
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
  ChessMove moveType;
  int fromX, fromY, toX, toY;
  char promoChar;

  switch (message) {
  case WM_INITDIALOG:
    move[0] = (char) lParam;
    move[1] = NULLCHAR;
    CenterWindowEx(hDlg, GetWindow(hDlg, GW_OWNER), 1 );
    hInput = GetDlgItem(hDlg, OPT_Move);
    SetWindowText(hInput, move);
    SetFocus(hInput);
    SendMessage(hInput, EM_SETSEL, (WPARAM)9999, (LPARAM)9999);
    return FALSE;

  case WM_COMMAND:
    switch (LOWORD(wParam)) {
    case IDOK:
      if (gameMode != EditGame && currentMove != forwardMostMove && 
	gameMode != Training) {
	DisplayMoveError("Displayed move is not current");
      } else {
	GetDlgItemText(hDlg, OPT_Move, move, sizeof(move));
	if (ParseOneMove(move, gameMode == EditPosition ? blackPlaysFirst : currentMove, 
	  &moveType, &fromX, &fromY, &toX, &toY, &promoChar)) {
	  if (gameMode != Training)
	      forwardMostMove = currentMove;
	  UserMoveEvent(fromX, fromY, toX, toY, promoChar);	
	} else {
	  DisplayMoveError("Could not parse move");
	}
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
PopUpMoveDialog(char firstchar)
{
    FARPROC lpProc;
    
    if ((gameMode == BeginningOfGame && !appData.icsActive) || 
        gameMode == MachinePlaysWhite || gameMode == MachinePlaysBlack ||
	gameMode == AnalyzeMode || gameMode == EditGame || 
	gameMode == EditPosition || gameMode == IcsExamining ||
	gameMode == IcsPlayingWhite || gameMode == IcsPlayingBlack ||
	gameMode == Training) {
      lpProc = MakeProcInstance((FARPROC)TypeInMoveDialog, hInst);
      DialogBoxParam(hInst, MAKEINTRESOURCE(DLG_TypeInMove),
	hwndMain, (DLGPROC)lpProc, (LPARAM)firstchar);
      FreeProcInstance(lpProc);
    }
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
}

LRESULT CALLBACK
ErrorDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  HANDLE hwndText;
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

    errorDialog = hDlg;
    SetWindowText(hDlg, errorTitle);
    hwndText = GetDlgItem(hDlg, OPT_ErrorText);
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
  HANDLE hwndText;
  RECT rChild;
  int height = GetSystemMetrics(SM_CYCAPTION)+GetSystemMetrics(SM_CYFRAME);

  switch (message) {
  case WM_INITDIALOG:
    GetWindowRect(hDlg, &rChild);

    SetWindowPos(hDlg, NULL, boardX, boardY-height, winWidth, height,
                                                             SWP_NOZORDER);

    /* 
        [AS] It seems that the above code wants to move the dialog up in the "caption
        area" of the main window, but it uses the dialog height as an hard-coded constant,
        and it doesn't work when you resize the dialog.
        For now, just give it a default position.
    */
    gothicDialog = hDlg;
    SetWindowText(hDlg, errorTitle);
    hwndText = GetDlgItem(hDlg, OPT_ErrorText);
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
GothicPopUp(char *title, char up)
{
  FARPROC lpProc;
  char *p, *q;
  BOOLEAN modal = hwndMain == NULL;

  strncpy(errorTitle, title, sizeof(errorTitle));
  errorTitle[sizeof(errorTitle) - 1] = '\0';

  if(up && gothicDialog == NULL) {
    lpProc = MakeProcInstance((FARPROC)GothicDialog, hInst);
    CreateDialog(hInst, MAKEINTRESOURCE(DLG_Error),
		 hwndMain, (DLGPROC)lpProc);
    FreeProcInstance(lpProc);
  } else if(!up && gothicDialog != NULL) {
    DestroyWindow(gothicDialog);
    gothicDialog = NULL;
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

typedef struct {
  char *item;
  char *command;
  BOOLEAN getname;
  BOOLEAN immediate;
} IcsTextMenuEntry;
#define ICS_TEXT_MENU_SIZE (IDM_CommandXLast - IDM_CommandX + 1)
IcsTextMenuEntry icsTextMenuEntry[ICS_TEXT_MENU_SIZE];

void
ParseIcsTextMenu(char *icsTextMenuString)
{
  int flags = 0;
  IcsTextMenuEntry *e = icsTextMenuEntry;
  char *p = icsTextMenuString;
  while (e->item != NULL && e < icsTextMenuEntry + ICS_TEXT_MENU_SIZE) {
    free(e->item);
    e->item = NULL;
    if (e->command != NULL) {
      free(e->command);
      e->command = NULL;
    }
    e++;
  }
  e = icsTextMenuEntry;
  while (*p && e < icsTextMenuEntry + ICS_TEXT_MENU_SIZE) {
    if (*p == ';' || *p == '\n') {
      e->item = strdup("-");
      e->command = NULL;
      p++;
    } else if (*p == '-') {
      e->item = strdup("-");
      e->command = NULL;
      p++;
      if (*p) p++;
    } else {
      char *q, *r, *s, *t;
      char c;
      q = strchr(p, ',');
      if (q == NULL) break;
      *q = NULLCHAR;
      r = strchr(q + 1, ',');
      if (r == NULL) break;
      *r = NULLCHAR;
      s = strchr(r + 1, ',');
      if (s == NULL) break;
      *s = NULLCHAR;
      c = ';';
      t = strchr(s + 1, c);
      if (t == NULL) {
	c = '\n';
	t = strchr(s + 1, c);
      }
      if (t != NULL) *t = NULLCHAR;
      e->item = strdup(p);
      e->command = strdup(q + 1);
      e->getname = *(r + 1) != '0';
      e->immediate = *(s + 1) != '0';
      *q = ',';
      *r = ',';
      *s = ',';
      if (t == NULL) break;
      *t = c;
      p = t + 1;
    }
    e++;
  } 
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
    } else {
      if (e->item[0] == '|') {
	AppendMenu(h, MF_STRING|MF_MENUBARBREAK,
		   IDM_CommandX + i, &e->item[1]);
      } else {
	AppendMenu(h, MF_STRING, IDM_CommandX + i, e->item);
      }
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
    sprintf(buf, "%s %s", command, name);
    SetWindowText(hInput, buf);
    SendMessage(hInput, WM_CHAR, '\r', 0);
  } else {
    sprintf(buf, "%s %s ", command, name); /* trailing space */
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
      SetFocus(hInput);
      SendMessage(hInput, message, wParam, lParam);
    }
    return 0;
  case WM_PASTE:
    hInput = GetDlgItem(hwndConsole, OPT_ConsoleInput);
    SetFocus(hInput);
    return SendMessage(hInput, message, wParam, lParam);
  case WM_MBUTTONDOWN:
    return SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDM_QuickPaste, 0), 0);
  case WM_RBUTTONDOWN:
    if (!(GetKeyState(VK_SHIFT) & ~1)) {
      /* Move selection here if it was empty */
      POINT pt;
      pt.x = LOWORD(lParam);
      pt.y = HIWORD(lParam);
      SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM)&sel);
      if (sel.cpMin == sel.cpMax) {
        sel.cpMin = SendMessage(hwnd, EM_CHARFROMPOS, 0, (LPARAM)&pt); /*doc is wrong*/
	sel.cpMax = sel.cpMin;
	SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM)&sel);
      }
      SendMessage(hwnd, EM_HIDESELECTION, FALSE, FALSE);
    }
    return 0;
  case WM_RBUTTONUP:
    if (GetKeyState(VK_SHIFT) & ~1) {
      SendDlgItemMessage(hwndConsole, OPT_ConsoleText, 
        WM_COMMAND, MAKEWPARAM(IDM_QuickPaste, 0), 0);
    } else {
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
      pt.x = LOWORD(lParam);
      pt.y = HIWORD(lParam);
      MenuPopup(hwnd, pt, hmenu, -1);
    }
    return 0;
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
  static HWND hText, hInput, hFocus;
  InputSource *is = consoleInputSource;
  RECT rect;
  static int sizeX, sizeY;
  int newSizeX, newSizeY;
  MINMAXINFO *mmi;

  switch (message) {
  case WM_INITDIALOG: /* message: initialize dialog box */
    hwndConsole = hDlg;
    hText = GetDlgItem(hDlg, OPT_ConsoleText);
    hInput = GetDlgItem(hDlg, OPT_ConsoleInput);
    SetFocus(hInput);
    consoleTextWindowProc = (WNDPROC)
      SetWindowLong(hText, GWL_WNDPROC, (LONG) ConsoleTextSubclass);
    SendMessage(hText, EM_SETBKGNDCOLOR, FALSE, consoleBackgroundColor);
    consoleInputWindowProc = (WNDPROC)
      SetWindowLong(hInput, GWL_WNDPROC, (LONG) ConsoleInputSubclass);
    SendMessage(hInput, EM_SETBKGNDCOLOR, FALSE, consoleBackgroundColor);
    Colorize(ColorNormal, TRUE);
    SendMessage(hInput, EM_SETCHARFORMAT, SCF_ALL, (LPARAM) &consoleCF);
    ChangedConsoleFont();
    GetClientRect(hDlg, &rect);
    sizeX = rect.right;
    sizeY = rect.bottom;
    if (consoleX != CW_USEDEFAULT && consoleY != CW_USEDEFAULT &&
	consoleW != CW_USEDEFAULT && consoleH != CW_USEDEFAULT) {
      WINDOWPLACEMENT wp;
      EnsureOnScreen(&consoleX, &consoleY);
      wp.length = sizeof(WINDOWPLACEMENT);
      wp.flags = 0;
      wp.showCmd = SW_SHOW;
      wp.ptMaxPosition.x = wp.ptMaxPosition.y = 0;
      wp.rcNormalPosition.left = consoleX;
      wp.rcNormalPosition.right = consoleX + consoleW;
      wp.rcNormalPosition.top = consoleY;
      wp.rcNormalPosition.bottom = consoleY + consoleH;
      SetWindowPlacement(hDlg, &wp);
    }
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

  if(copyNumber > 1) sprintf(buf, "%d", copyNumber); else buf[0] = 0;

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

  if (appData.clockMode) {
    if (tinyLayout)
      sprintf(buf, "%c %s %s %s", color[0], TimeString(timeRemaining), flagFell);
    else
      sprintf(buf, "%s: %s %s", color, TimeString(timeRemaining), flagFell);
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

  ExtTextOut(hdc, rect->left + MESSAGE_LINE_LEFTMARGIN,
	     rect->top, ETO_CLIPPED|ETO_OPAQUE,
	     rect, str, strlen(str), NULL);

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
                fprintf( debugFP, "Input line exceeded buffer size (source id=%u)\n", is->id );
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
    fprintf( debugFP, "Input thread terminated (id=%u, error=%d, count=%d)\n", is->id, is->error, is->count );
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
  { IDM_Revert, MF_BYCOMMAND|MF_GRAYED },
  { -1, -1 }
};

Enables icsEnables[] = {
  { IDM_MailMove, MF_BYCOMMAND|MF_GRAYED },
  { IDM_ReloadCMailMsg, MF_BYCOMMAND|MF_GRAYED },
  { IDM_MachineWhite, MF_BYCOMMAND|MF_GRAYED },
  { IDM_MachineBlack, MF_BYCOMMAND|MF_GRAYED },
  { IDM_TwoMachines, MF_BYCOMMAND|MF_GRAYED },
  { IDM_AnalysisMode, MF_BYCOMMAND|MF_GRAYED },
  { IDM_AnalyzeFile, MF_BYCOMMAND|MF_GRAYED },
  { IDM_TimeControl, MF_BYCOMMAND|MF_GRAYED },
  { IDM_MoveNow, MF_BYCOMMAND|MF_GRAYED },
  { IDM_Hint, MF_BYCOMMAND|MF_GRAYED },
  { IDM_Book, MF_BYCOMMAND|MF_GRAYED },
  { IDM_IcsOptions, MF_BYCOMMAND|MF_ENABLED },
  { -1, -1 }
};

#ifdef ZIPPY
Enables zippyEnables[] = {
  { IDM_MoveNow, MF_BYCOMMAND|MF_ENABLED },
  { IDM_Hint, MF_BYCOMMAND|MF_ENABLED },
  { IDM_Book, MF_BYCOMMAND|MF_ENABLED },
  { -1, -1 }
};
#endif

Enables ncpEnables[] = {
  { IDM_MailMove, MF_BYCOMMAND|MF_GRAYED },
  { IDM_ReloadCMailMsg, MF_BYCOMMAND|MF_GRAYED },
  { IDM_MachineWhite, MF_BYCOMMAND|MF_GRAYED },
  { IDM_MachineBlack, MF_BYCOMMAND|MF_GRAYED },
  { IDM_TwoMachines, MF_BYCOMMAND|MF_GRAYED },
  { IDM_AnalysisMode, MF_BYCOMMAND|MF_GRAYED },
  { IDM_AnalyzeFile, MF_BYCOMMAND|MF_GRAYED },
  { IDM_IcsClient, MF_BYCOMMAND|MF_GRAYED },
  { ACTION_POS, MF_BYPOSITION|MF_GRAYED },
  { IDM_Revert, MF_BYCOMMAND|MF_GRAYED },
  { IDM_MoveNow, MF_BYCOMMAND|MF_GRAYED },
  { IDM_RetractMove, MF_BYCOMMAND|MF_GRAYED },
  { IDM_TimeControl, MF_BYCOMMAND|MF_GRAYED },
  { IDM_Hint, MF_BYCOMMAND|MF_GRAYED },
  { IDM_Book, MF_BYCOMMAND|MF_GRAYED },
  { -1, -1 }
};

Enables trainingOnEnables[] = {
  { IDM_EditComment, MF_BYCOMMAND|MF_GRAYED },
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
  if (prevChecked != 0)
    (void) CheckMenuItem(GetMenu(hwndMain),
			 prevChecked, MF_BYCOMMAND|MF_UNCHECKED);
  if (nowChecked != 0)
    (void) CheckMenuItem(GetMenu(hwndMain),
			 nowChecked, MF_BYCOMMAND|MF_CHECKED);

  if (nowChecked == IDM_LoadGame || nowChecked == IDM_Training) {
    (void) EnableMenuItem(GetMenu(hwndMain), IDM_Training, 
			  MF_BYCOMMAND|MF_ENABLED);
  } else {
    (void) EnableMenuItem(GetMenu(hwndMain), 
			  IDM_Training, MF_BYCOMMAND|MF_GRAYED);
  }

  prevChecked = nowChecked;
}

VOID
SetICSMode()
{
  HMENU hmenu = GetMenu(hwndMain);
  SetMenuEnables(hmenu, icsEnables);
  EnableMenuItem(GetSubMenu(hmenu, OPTIONS_POS), ICS_POS,
    MF_BYPOSITION|MF_ENABLED);
#ifdef ZIPPY
  if (appData.zippyPlay) {
    SetMenuEnables(hmenu, zippyEnables);
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
  EnableMenuItem(GetSubMenu(hmenu, OPTIONS_POS), SOUNDS_POS,
    MF_BYPOSITION|MF_GRAYED);
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
    (void)EnableMenuItem(hMenu, IDM_TwoMachines, flags);
  }
}


VOID
DisplayTitle(char *str)
{
  char title[MSG_SIZ], *host;
  if (str[0] != NULLCHAR) {
    strcpy(title, str);
  } else if (appData.icsActive) {
    if (appData.icsCommPort[0] != NULLCHAR)
      host = "ICS";
    else 
      host = appData.icsHost;
    sprintf(title, "%s: %s", szTitle, host);
  } else if (appData.noChessProgram) {
    strcpy(title, szTitle);
  } else {
    strcpy(title, szTitle);
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

  if (IsIconic(hwndMain)) return;
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
    strcpy(buf, str);
  } else {
    len = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
			NULL, error, LANG_NEUTRAL,
			(LPSTR) buf2, MSG_SIZ, NULL);
    if (len > 0) {
      sprintf(buf, "%s:\n%s", str, buf2);
    } else {
      ErrorMap *em = errmap;
      while (em->err != 0 && em->err != error) em++;
      if (em->err != 0) {
	sprintf(buf, "%s:\n%s", str, em->msg);
      } else {
	sprintf(buf, "%s:\nError code %d", str, error);
      }
    }
  }
  
  ErrorPopUp("Error", buf);
}


VOID
DisplayMoveError(char *str)
{
  fromX = fromY = -1;
  ClearHighlights();
  DrawPosition(FALSE, NULL);
  if (appData.popupMoveErrors) {
    ErrorPopUp("Error", str);
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
  char *label = exitStatus ? "Fatal Error" : "Exiting";

  if (error != 0) {
    len = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
			NULL, error, LANG_NEUTRAL,
			(LPSTR) buf2, MSG_SIZ, NULL);
    if (len > 0) {
      sprintf(buf, "%s:\n%s", str, buf2);
    } else {
      ErrorMap *em = errmap;
      while (em->err != 0 && em->err != error) em++;
      if (em->err != 0) {
	sprintf(buf, "%s:\n%s", str, em->msg);
      } else {
	sprintf(buf, "%s:\nError code %d", str, error);
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
  (void) MessageBox(hwndMain, str, "Information", MB_OK|MB_ICONINFORMATION);
}


VOID
DisplayNote(char *str)
{
  ErrorPopUp("Note", str);
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
    SetWindowText(hDlg, qp->title);
    SetDlgItemText(hDlg, OPT_QuestionText, qp->question);
    SetFocus(GetDlgItem(hDlg, OPT_QuestionInput));
    return FALSE;

  case WM_COMMAND:
    switch (LOWORD(wParam)) {
    case IDOK:
      strcpy(reply, qp->replyPrefix);
      if (*reply) strcat(reply, " ");
      len = strlen(reply);
      GetDlgItemText(hDlg, OPT_QuestionInput, reply + len, sizeof(reply) - len);
      strcat(reply, "\n");
      OutputToProcess(qp->pr, reply, strlen(reply), &err);
      EndDialog(hDlg, TRUE);
      if (err) DisplayFatalError("Error writing to chess program", err, 1);
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
            sprintf( buf, "%d", myrandom() % 960 );
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

/* [AS] Game list options */
typedef struct {
    char id;
    char * name;
} GLT_Item;

static GLT_Item GLT_ItemInfo[] = {
    { GLT_EVENT,      "Event" },
    { GLT_SITE,       "Site" },
    { GLT_DATE,       "Date" },
    { GLT_ROUND,      "Round" },
    { GLT_PLAYERS,    "Players" },
    { GLT_RESULT,     "Result" },
    { GLT_WHITE_ELO,  "White Rating" },
    { GLT_BLACK_ELO,  "Black Rating" },
    { GLT_TIME_CONTROL,"Time Control" },
    { GLT_VARIANT,    "Variant" },
    { GLT_OUT_OF_BOOK,PGN_OUT_OF_BOOK },
    { 0, 0 }
};

const char * GLT_FindItem( char id )
{
    const char * result = 0;

    GLT_Item * list = GLT_ItemInfo;

    while( list->id != 0 ) {
        if( list->id == id ) {
            result = list->name;
            break;
        }

        list++;
    }

    return result;
}

void GLT_AddToList( HWND hDlg, int iDlgItem, char id, int index )
{
    const char * name = GLT_FindItem( id );

    if( name != 0 ) {
        if( index >= 0 ) {
            SendDlgItemMessage( hDlg, iDlgItem, LB_INSERTSTRING, index, (LPARAM) name );
        }
        else {
            SendDlgItemMessage( hDlg, iDlgItem, LB_ADDSTRING, 0, (LPARAM) name );
        }
    }
}

void GLT_TagsToList( HWND hDlg, char * tags )
{
    char * pc = tags;

    SendDlgItemMessage( hDlg, IDC_GameListTags, LB_RESETCONTENT, 0, 0 );

    while( *pc ) {
        GLT_AddToList( hDlg, IDC_GameListTags, *pc, -1 );
        pc++;
    }

    SendDlgItemMessage( hDlg, IDC_GameListTags, LB_ADDSTRING, 0, (LPARAM) "\t --- Hidden tags ---" );

    pc = GLT_ALL_TAGS;

    while( *pc ) {
        if( strchr( tags, *pc ) == 0 ) {
            GLT_AddToList( hDlg, IDC_GameListTags, *pc, -1 );
        }
        pc++;
    }

    SendDlgItemMessage( hDlg, IDC_GameListTags, LB_SETCURSEL, 0, 0 );
}

char GLT_ListItemToTag( HWND hDlg, int index )
{
    char result = '\0';
    char name[128];

    GLT_Item * list = GLT_ItemInfo;

    if( SendDlgItemMessage( hDlg, IDC_GameListTags, LB_GETTEXT, index, (LPARAM) name ) != LB_ERR ) {
        while( list->id != 0 ) {
            if( strcmp( list->name, name ) == 0 ) {
                result = list->id;
                break;
            }

            list++;
        }
    }

    return result;
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
    static char glt[64];
    static char * lpUserGLT;

    switch( message )
    {
    case WM_INITDIALOG:
        lpUserGLT = (char *) lParam;
        
        strcpy( glt, lpUserGLT );

        CenterWindow(hDlg, GetWindow(hDlg, GW_OWNER));

        /* Initialize list */
        GLT_TagsToList( hDlg, glt );

        SetFocus( GetDlgItem(hDlg, IDC_GameListTags) );

        break;

    case WM_COMMAND:
        switch( LOWORD(wParam) ) {
        case IDOK:
            {
                char * pc = lpUserGLT;
                int idx = 0;
                int cnt = (int) SendDlgItemMessage( hDlg, IDC_GameListTags, LB_GETCOUNT, 0, 0 );
                char id;

                do {
                    id = GLT_ListItemToTag( hDlg, idx );

                    *pc++ = id;
                    idx++;
                } while( id != '\0' );
            }
            EndDialog( hDlg, 0 );
            return TRUE;
        case IDCANCEL:
            EndDialog( hDlg, 1 );
            return TRUE;

        case IDC_GLT_Default:
            strcpy( glt, GLT_DEFAULT_TAGS );
            GLT_TagsToList( hDlg, glt );
            return TRUE;

        case IDC_GLT_Restore:
            strcpy( glt, lpUserGLT );
            GLT_TagsToList( hDlg, glt );
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
    char glt[64];
    int result;
    FARPROC lpProc = MakeProcInstance( (FARPROC) GameListOptions_Proc, hInst );

    strcpy( glt, appData.gameListTags );

    result = DialogBoxParam( hInst, MAKEINTRESOURCE(DLG_GameListOptions), hwndMain, (DLGPROC)lpProc, (LPARAM)glt );

    if( result == 0 ) {
        /* [AS] Memory leak here! */
        appData.gameListTags = strdup( glt ); 
    }

    return result;
}


VOID
DisplayIcsInteractionTitle(char *str)
{
  char consoleTitle[MSG_SIZ];

  sprintf(consoleTitle, "%s: %s", szConsoleTitle, str);
  SetWindowText(hwndConsole, consoleTitle);
}

void
DrawPosition(int fullRedraw, Board board)
{
  HDCDrawPosition(NULL, (BOOLEAN) fullRedraw, board); 
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
}


VOID
CommentPopUp(char *title, char *str)
{
  HWND hwnd = GetActiveWindow();
  EitherCommentPopUp(0, title, str, FALSE);
  SetActiveWindow(hwnd);
}

VOID
CommentPopDown(void)
{
  CheckMenuItem(GetMenu(hwndMain), IDM_EditComment, MF_UNCHECKED);
  if (commentDialog) {
    ShowWindow(commentDialog, SW_HIDE);
  }
  commentDialogUp = FALSE;
}

VOID
EditCommentPopUp(int index, char *title, char *str)
{
  EitherCommentPopUp(index, title, str, TRUE);
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

  if (!GetUserName(buf, &bufsiz)) {
    /*DisplayError("Error getting user name", GetLastError());*/
    strcpy(buf, "User");
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
    strcpy(buf, "Unknown");
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
  hdc = GetDC(hwndMain);
  char *flag = whiteFlag && gameMode == TwoMachinesPlay ? "(!)" : "";

  if (!IsIconic(hwndMain)) {
    DisplayAClock(hdc, timeRemaining, highlight, flipClock ? &blackRect : &whiteRect, "White", flag);
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

  hdc = GetDC(hwndMain);
  if (!IsIconic(hwndMain)) {
    DisplayAClock(hdc, timeRemaining, highlight, flipClock ? &whiteRect : &blackRect, "Black", flag);
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
  f = OpenFileDialog(hwndMain, TRUE, defName,
		     appData.oldSaveStyle ? "gam" : "pgn",
		     GAME_FILT, 
		     "Save Game to File", NULL, fileTitle, NULL);
  if (f != NULL) {
    SaveGame(f, 0, "");
    fclose(f);
  }
}


void
ScheduleDelayedEvent(DelayedEventCallback cb, long millisec)
{
  if (delayedTimerEvent != 0) {
    if (appData.debugMode) {
      fprintf(debugFP, "ScheduleDelayedEvent: event already scheduled\n");
    }
    KillTimer(hwndMain, delayedTimerEvent);
    delayedTimerEvent = 0;
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
  ChildProc *cp;

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
    if( signal == 9 ) {
        if ( appData.debugMode) {
            fprintf( debugFP, "Terminating process %u\n", cp->pid );
        }

        TerminateProcess( cp->hProcess, 0 );
    }
    else if( signal == 10 ) {
        DWORD dw = WaitForSingleObject( cp->hProcess, 3*1000 ); // Wait 3 seconds at most

        if( dw != WAIT_OBJECT_0 ) {
            if ( appData.debugMode) {
                fprintf( debugFP, "Process %u still alive after timeout, killing...\n", cp->pid );
            }

            TerminateProcess( cp->hProcess, 0 );
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
    sprintf(cmdLine, "%s %s", appData.telnetProgram, host);
  } else {
    sprintf(cmdLine, "%s %s %s", appData.telnetProgram, host, port);
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
    sprintf(fullname, "\\\\.\\%s", name);
  else
    strcpy(fullname, name);

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
  DisplayFatalError("Not implemented", 0, 1);
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
  sprintf(stderrPortStr, "%d", fromPort);

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


int
OutputToProcess(ProcRef pr, char *message, int count, int *outError)
{
  DWORD dOutCount;
  int outCount = SOCKET_ERROR;
  ChildProc *cp = (ChildProc *) pr;
  static OVERLAPPED ovl;

  if (pr == NoProc) {
    ConsoleOutput(message, count, FALSE);
    return count;
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
  DisplayFatalError("Not implemented", 0, 1);
}

/* see wgamelist.c for Game List functions */
/* see wedittags.c for Edit Tags functions */


VOID
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
    }
  }
}


VOID
StartAnalysisClock()
{
  if (analysisTimerEvent) return;
  analysisTimerEvent = SetTimer(hwndMain, (UINT) ANALYSIS_TIMER_ID,
		                        (UINT) 2000, NULL);
}

LRESULT CALLBACK
AnalysisDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  static HANDLE hwndText;
  RECT rect;
  static int sizeX, sizeY;
  int newSizeX, newSizeY, flags;
  MINMAXINFO *mmi;

  switch (message) {
  case WM_INITDIALOG: /* message: initialize dialog box */
    /* Initialize the dialog items */
    hwndText = GetDlgItem(hDlg, OPT_AnalysisText);
    SetWindowText(hDlg, analysisTitle);
    SetDlgItemText(hDlg, OPT_AnalysisText, analysisText);
    /* Size and position the dialog */
    if (!analysisDialog) {
      analysisDialog = hDlg;
      flags = SWP_NOZORDER;
      GetClientRect(hDlg, &rect);
      sizeX = rect.right;
      sizeY = rect.bottom;
      if (analysisX != CW_USEDEFAULT && analysisY != CW_USEDEFAULT &&
	  analysisW != CW_USEDEFAULT && analysisH != CW_USEDEFAULT) {
	WINDOWPLACEMENT wp;
	EnsureOnScreen(&analysisX, &analysisY);
	wp.length = sizeof(WINDOWPLACEMENT);
	wp.flags = 0;
	wp.showCmd = SW_SHOW;
	wp.ptMaxPosition.x = wp.ptMaxPosition.y = 0;
	wp.rcNormalPosition.left = analysisX;
	wp.rcNormalPosition.right = analysisX + analysisW;
	wp.rcNormalPosition.top = analysisY;
	wp.rcNormalPosition.bottom = analysisY + analysisH;
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
    return FALSE;

  case WM_COMMAND: /* message: received a command */
    switch (LOWORD(wParam)) {
    case IDCANCEL:
      EditGameEvent();
      return TRUE;
    default:
      break;
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
AnalysisPopUp(char* title, char* str)
{
  FARPROC lpProc;
  char *p, *q;

  /* [AS] */
  EngineOutputPopUp();
  return;

  if (str == NULL) str = "";
  p = (char *) malloc(2 * strlen(str) + 2);
  q = p;
  while (*str) {
    if (*str == '\n') *q++ = '\r';
    *q++ = *str++;
  }
  *q = NULLCHAR;
  if (analysisText != NULL) free(analysisText);
  analysisText = p;

  if (analysisDialog) {
    SetWindowText(analysisDialog, title);
    SetDlgItemText(analysisDialog, OPT_AnalysisText, analysisText);
    ShowWindow(analysisDialog, SW_SHOW);
  } else {
    analysisTitle = title;
    lpProc = MakeProcInstance((FARPROC)AnalysisDialog, hInst);
    CreateDialog(hInst, MAKEINTRESOURCE(DLG_Analysis),
		 hwndMain, (DLGPROC)lpProc);
    FreeProcInstance(lpProc);
  }
  analysisDialogUp = TRUE;  
}

VOID
AnalysisPopDown()
{
  if (analysisDialog) {
    ShowWindow(analysisDialog, SW_HIDE);
  }
  analysisDialogUp = FALSE;  
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
AnimateMove(board, fromX, fromY, toX, toY)
     Board board;
     int fromX;
     int fromY;
     int toX;
     int toY;
{
  ChessSquare piece;
  POINT start, finish, mid;
  POINT frames[kFactor * 2 + 1];
  int nFrames, n;

  if (!appData.animate) return;
  if (doingSizing) return;
  if (fromY < 0 || fromX < 0) return;
  piece = board[fromY][fromX];
  if (piece >= EmptySquare) return;

  ScreenSquare(fromX, fromY, &start);
  ScreenSquare(toX, toY, &finish);

  /* All pieces except knights move in straight line */
  if (piece != WhiteKnight && piece != BlackKnight) {
    mid.x = start.x + (finish.x - start.x) / 2;
    mid.y = start.y + (finish.y - start.y) / 2;
  } else {
    /* Knight: make diagonal movement then straight */
    if (abs(toY - fromY) < abs(toX - fromX)) {
       mid.x = start.x + (finish.x - start.x) / 2;
       mid.y = finish.y;
     } else {
       mid.x = finish.x;
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
  animInfo.piece = EmptySquare;
}

/*      Convert board position to corner of screen rect and color       */

static void
ScreenSquare(column, row, pt)
     int column; int row; POINT * pt;
{
  if (flipView) {
    pt->x = lineGap + ((BOARD_WIDTH-1)-column) * (squareSize + lineGap);
    pt->y = lineGap + row * (squareSize + lineGap);
  } else {
    pt->x = lineGap + column * (squareSize + lineGap);
    pt->y = lineGap + ((BOARD_HEIGHT-1)-row) * (squareSize + lineGap);
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
HistorySet( char movelist[][2*MOVE_LEN], int first, int last, int current )
{
#if 0
    char buf[256];

    sprintf( buf, "HistorySet: first=%d, last=%d, current=%d (%s)\n",
        first, last, current, current >= 0 ? movelist[current] : "n/a" );

    OutputDebugString( buf );
#endif

    MoveHistorySet( movelist, first, last, current, pvInfoList );

    EvalGraphSet( first, last, current, pvInfoList );
}

void SetProgramStats( FrontEndProgramStats * stats )
{
#if 0
    char buf[1024];

    sprintf( buf, "SetStats for %d: depth=%d, nodes=%lu, score=%5.2f, time=%5.2f, pv=%s\n",
        stats->which, stats->depth, stats->nodes, stats->score / 100.0, stats->time / 100.0, stats->pv == 0 ? "n/a" : stats->pv );

    OutputDebugString( buf );
#endif

    EngineOutputUpdate( stats );
}
