/*
 * WinBoard.h -- Definitions for Windows NT front end to XBoard
 *
 * Copyright 1991 by Digital Equipment Corporation, Maynard,
 * Massachusetts.
 *
 * Enhancements Copyright 1992-2001, 2002, 2003, 2004, 2005, 2006,
 * 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014 Free Software Foundation, Inc.
 *
 * Enhancements Copyright 2005 Alessandro Scotti
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

#include "resource.h"
#include <dlgs.h>

/* Types */
typedef struct {
  char faceName[LF_FACESIZE];
  float pointSize;
  BYTE bold, italic, underline, strikeout;
  BYTE charset;
} MyFontParams;

typedef struct {
  char *def;
  MyFontParams mfp;
  LOGFONT lf;
  HFONT hf;
} MyFont;

typedef enum { 
  SizeTiny, SizeTeeny, SizeDinky, SizePetite, SizeSlim, SizeSmall,
  SizeMediocre, SizeMiddling, SizeAverage, SizeModerate, SizeMedium,
  SizeBulky, SizeLarge, SizeBig, SizeHuge, SizeGiant, SizeColossal,
  SizeTitanic, NUM_SIZES 
} BoardSize;

typedef struct {
    COLORREF color;
    int effects;
    char *name;
} MyColorizeAttribs;

typedef struct {
  char* name;
  void* data;
  int flag; // [HGM] needed to indicate if data was malloc'ed or not
} MySound;

typedef struct {
    COLORREF color;
    int effects;
    MySound sound;
} MyTextAttribs;

/* Functions */

BOOL InitApplication(HINSTANCE);
BOOL InitInstance(HINSTANCE, int, LPSTR);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK About(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK BoardSizeDlg(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK ButtonProc(HWND, UINT, WPARAM, LPARAM);
VOID InitAppData(LPSTR);
VOID InitDrawingColors(VOID);
VOID InitDrawingSizes(BoardSize boardSize, int flags);
VOID InitMenuChecks(VOID);
int  ICSInitScript(VOID);
BOOL CenterWindow(HWND hwndChild, HWND hwndParent);
VOID ResizeEditPlusButtons(HWND hDlg, HWND hText, int sizeX, int sizeY, int newSizeX, int newSizeY);
VOID PromotionPopup(HWND hwnd);
FILE *OpenFileDialog(HWND hWnd, char *write, char *defName, char *defExt, 
		     char *nameFilt, char *dlgTitle, UINT *number,
		     char fileTitle[MSG_SIZ], char fileName[MSG_SIZ]);
VOID InputEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
DWORD InputThread(LPVOID arg);
DWORD NonOvlInputThread(LPVOID arg);
DWORD SocketInputThread(LPVOID arg);
BOOL ChangeColor(HWND hwnd, COLORREF *which);
VOID ChangeBoardSize(BoardSize newSize);
BOOL APIENTRY MyCreateFont(HWND hwnd, MyFont *font);
VOID ErrorPopDown(VOID);
VOID EnsureOnScreen(int *x, int *y, int minX, int minY);
HBITMAP 
DoLoadBitmap(HINSTANCE hinst, char *piece, int squareSize, char *suffix);
COLORREF ParseColorName(char *name);
void ParseAttribs(COLORREF *color, int *effects, char* argValue);
VOID CreateFontInMF(MyFont *mf);
VOID ChangedConsoleFont();
VOID ParseFontName(char *name, MyFontParams *mfp);
void InitComboStrings(HANDLE hwndCombo, char **cd);
BOOLEAN MyLoadSound(MySound *ms);
BOOLEAN MyPlaySound(MySound *ms);
VOID ExitArgError(char *msg, char *badArg, Boolean quit);
void SaveSettings(char* name);
BOOL BrowseForFolder( const char * title, char * path );
VOID TourneyPopup();
VOID LoadEnginePopUp();
VOID LoadOptionsPopup(HWND hDlg);
VOID InitTextures();
void ThemeOptionsPopup(HWND hwnd);

/* Constants */

#define CLOCK_FONT 0
#define MESSAGE_FONT 1
#define COORD_FONT 2
#define CONSOLE_FONT 3
#define COMMENT_FONT 4
#define EDITTAGS_FONT 5
#define MOVEHISTORY_FONT 6
#define GAMELIST_FONT 7
#define NUM_FONTS 8

/* Positions of some menu items.  Origin is zero and separator lines count. */
/* It's gross that these are needed. */
#define ACTION_POS 4	 /* Posn of "Action" on menu bar */
#define OPTIONS_POS 6	 /* Posn of "Options" on menu bar */
/* end grossness */

extern MyFont *font[NUM_SIZES][NUM_FONTS];

#define WM_USER_Input                 (WM_USER + 4242)
#define WM_USER_Mouseleave            (WM_USER + 4243)
#define WM_USER_GetConsoleBackground  (WM_USER + 4244)

#define CLOCK_TIMER_ID        51
#define LOAD_GAME_TIMER_ID    52
#define ANALYSIS_TIMER_ID     53
#define MOUSE_TIMER_ID        54
#define DELAYED_TIMER_ID      55

#define SOLID_PIECE           0
#define OUTLINE_PIECE         1
#define WHITE_PIECE           2

#define COPY_TMP "wbcopy.tmp"
#define PASTE_TMP "wbpaste.tmp"

/* variables */
extern HINSTANCE hInst;
extern HWND hwndMain;
extern BoardSize boardSize;

// [HGM] Some stuff to allo a platform-independent reference to windows
// This should be moved to frontend.h in due time

typedef enum {
  W_Main, W_Console, W_Comment, W_Tags, W_GameList, 
  W_MoveHist, W_EngineOut, NUM_WINDOWS
} WindowID;

extern WindowPlacement placementTab[NUM_WINDOWS];
extern HWND hwndTab[NUM_WINDOWS]; // this remains pure front-end.

void Translate( HWND hDlg, int id);
VOID InitWindowPlacement( WindowPlacement * wp );
VOID RestoreWindowPlacement( HWND hWnd, WindowPlacement * wp );
VOID ReattachAfterMove( LPRECT lprcOldPos, int new_x, int new_y, HWND hWndChild, WindowPlacement * pwpChild );
VOID ReattachAfterSize( LPRECT lprcOldPos, int new_w, int new_h, HWND hWndChild, WindowPlacement * pwpChild );
BOOL GetActualPlacement( HWND hWnd, WindowPlacement * wp );

VOID MoveHistoryPopUp();
VOID MoveHistoryPopDown();
extern HWND moveHistoryDialog;

VOID EvalGraphPopUp();
VOID EvalGraphPopDown();
extern HWND evalGraphDialog;

extern HWND engineOutputDialog;

struct GameListStats
{
    int white_wins;
    int black_wins;
    int drawn;
    int unfinished;
};

int GameListToListBox( HWND hDlg, BOOL boReset, char * pszFilter, struct GameListStats * stats, BOOL byPos, BOOL narrow );
VOID ShowGameListProc(void);
extern HWND gameListDialog;

VOID EditTagsProc(void);
extern HWND editTagsDialog;
extern int screenWidth, screenHeight;
extern RECT screenGeometry; // Top-left coordiate of the screen can be different from (0,0)

