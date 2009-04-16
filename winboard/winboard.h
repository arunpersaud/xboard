/*
 * WinBoard.h -- Definitions for Windows NT front end to XBoard
 *
 * Copyright 1991 by Digital Equipment Corporation, Maynard, Massachusetts.
 * Enhancements Copyright 1992-97 Free Software Foundation, Inc.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 * ------------------------------------------------------------------------
 */
#include "resource.h"
#include <dlgs.h>

/* Types */
typedef struct {
  char faceName[LF_FACESIZE];
  float pointSize;
  BYTE bold, italic, underline, strikeout;
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
VOID ICSInitScript(VOID);
BOOL CenterWindow(HWND hwndChild, HWND hwndParent);
VOID ResizeEditPlusButtons(HWND hDlg, HWND hText, int sizeX, int sizeY, int newSizeX, int newSizeY);
VOID PromotionPopup(HWND hwnd);
FILE *OpenFileDialog(HWND hWnd, BOOL write, char *defName, char *defExt, 
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
VOID EnsureOnScreen(int *x, int *y);
typedef char GetFunc(void *getClosure);
VOID ParseArgs(GetFunc get, void *cl);
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
VOID ExitArgError(char *msg, char *badArg);

/* Constants */

#define CLOCK_FONT 0
#define MESSAGE_FONT 1
#define COORD_FONT 2
#define CONSOLE_FONT 3
#define COMMENT_FONT 4
#define EDITTAGS_FONT 5
#define NUM_FONTS 6

/* Positions of some menu items.  Origin is zero and separator lines count. */
/* It's gross that these are needed. */
#define ACTION_POS 2	 /* Posn of "Action" on menu bar */
#define OPTIONS_POS 4	 /* Posn of "Options" on menu bar */
#define ICS_POS 4 	 /* Posn of "ICS " on Options menu */
#define SOUNDS_POS 6     /* Posn of "Sounds" on Options menu */
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

/* [AS] Layout management */
typedef struct {
    Boolean visible;
    int x;
    int y;
    int width;
    int height;
} WindowPlacement;

VOID InitWindowPlacement( WindowPlacement * wp );

VOID RestoreWindowPlacement( HWND hWnd, WindowPlacement * wp );

VOID ReattachAfterMove( LPRECT lprcOldPos, int new_x, int new_y, HWND hWndChild, WindowPlacement * pwpChild );
