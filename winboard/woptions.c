/*
 * woptions.c -- Options dialog box routines for WinBoard
 *
 * Copyright 2000, 2009, 2010, 2011, 2012, 2013, 2014 Free Software Foundation, Inc.
 *
 * Enhancements Copyright 2005 Alessandro Scotti
 *
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

#include <windows.h>   /* required for all Windows applications */
#include <stdio.h>
#include <stdlib.h>
#include <shlobj.h>    /* [AS] Requires NT 4.0 or Win95 */
#include <ctype.h>

#include "common.h"
#include "frontend.h"
#include "winboard.h"
#include "backend.h"
#include "woptions.h"
#include "defaults.h"
#include <richedit.h>

#if __GNUC__
#include <errno.h>
#include <string.h>
#endif

#define _(s) T_(s)
#define N_(s) s

/* Imports from winboard.c */

extern MyFont *font[NUM_SIZES][NUM_FONTS];
extern HINSTANCE hInst;          /* current instance */
extern HWND hwndMain;            /* root window*/
extern BOOLEAN alwaysOnTop;
extern RECT boardRect;
extern COLORREF lightSquareColor, darkSquareColor, whitePieceColor,
  blackPieceColor, highlightSquareColor, premoveHighlightColor;
extern HPALETTE hPal;
extern BoardSize boardSize;
extern COLORREF consoleBackgroundColor;
extern MyColorizeAttribs colorizeAttribs[]; /* do I need the size? */
extern MyTextAttribs textAttribs[];
extern MySound sounds[];
extern ColorClass currentColorClass;
extern HWND hwndConsole;
extern char *defaultTextAttribs[];
extern HWND commentDialog;
extern HWND moveHistoryDialog;
extern HWND engineOutputDialog;
extern char installDir[];
extern HWND hCommPort;    /* currently open comm port */
extern DCB dcb;
extern BOOLEAN chessProgram;
extern int startedFromPositionFile; /* [HGM] loadPos */
extern int searchTime;

/* types */

typedef struct {
  char *label;
  unsigned value;
} ComboData;

typedef struct {
  char *label;
  char *name;
} SoundComboData;

/* module prototypes */

LRESULT CALLBACK GeneralOptions(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK BoardOptions(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK NewVariant(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK IcsOptions(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK FontOptions(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK CommPortOptions(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK LoadOptions(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK SaveOptions(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK TimeControl(HWND, UINT, WPARAM, LPARAM);
VOID ChangeBoardSize(BoardSize newSize);
VOID PaintSampleSquare(
    HWND     hwnd,
    int      ctrlid,
    COLORREF squareColor,
    COLORREF pieceColor,
    COLORREF squareOutlineColor,
    COLORREF pieceDetailColor,
    BOOL     isWhitePiece,
    BOOL     isMono,
    HBITMAP  pieces[3]
    );
VOID PaintColorBlock(HWND hwnd, int ctrlid, COLORREF color);
VOID SetBoardOptionEnables(HWND hDlg);
BoardSize BoardOptionsWhichRadio(HWND hDlg);
BOOL APIENTRY MyCreateFont(HWND hwnd, MyFont *font);
VOID UpdateSampleText(HWND hDlg, int id, MyColorizeAttribs *mca);
LRESULT CALLBACK ColorizeTextDialog(HWND , UINT, WPARAM, LPARAM);
VOID ColorizeTextPopup(HWND hwnd, ColorClass cc);
VOID SetIcsOptionEnables(HWND hDlg);
VOID SetSampleFontText(HWND hwnd, int id, const MyFont *mf);
VOID CopyFont(MyFont *dest, const MyFont *src);
void InitSoundComboData(SoundComboData *scd);
void ResetSoundComboData(SoundComboData *scd);
void InitSoundCombo(HWND hwndCombo, SoundComboData *scd);
int SoundDialogWhichRadio(HWND hDlg);
VOID SoundDialogSetEnables(HWND hDlg, int radio);
char * SoundDialogGetName(HWND hDlg, int radio);
void DisplaySelectedSound(HWND hDlg, HWND hCombo, const char *name);
VOID ParseCommSettings(char *arg, DCB *dcb);
VOID PrintCommSettings(FILE *f, char *name, DCB *dcb);
void InitCombo(HANDLE hwndCombo, ComboData *cd);
void SelectComboValue(HANDLE hwndCombo, ComboData *cd, unsigned value);
VOID SetLoadOptionEnables(HWND hDlg);
VOID SetSaveOptionEnables(HWND hDlg);
VOID SetTimeControlEnables(HWND hDlg);

char *
InterpretFileName(char *buf, char *homeDir)
{ // [HGM] file name relative to homeDir. (Taken out of SafeOptionsDialog, because it is generally useful)
  char *result = NULL;
  if ((isalpha(buf[0]) && buf[1] == ':') ||
    (buf[0] == '\\' && buf[1] == '\\')) {
    return strdup(buf);
  } else {
    char buf2[MSG_SIZ], buf3[MSG_SIZ];
    char *dummy;
    GetCurrentDirectory(MSG_SIZ, buf3);
    SetCurrentDirectory(homeDir);
    if (GetFullPathName(buf, MSG_SIZ, buf2, &dummy)) {
      result = strdup(buf2);
    } else {
      result = strdup(buf);
    }
    SetCurrentDirectory(buf3);
  }
  return result;
}

/*---------------------------------------------------------------------------*\
 *
 * General Options Dialog functions
 *
\*---------------------------------------------------------------------------*/


LRESULT CALLBACK
GeneralOptionsDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  static Boolean oldShowCoords;
  static Boolean oldBlindfold;
  static Boolean oldShowButtonBar;
  static Boolean oldAutoLogo;

  switch (message) {
  case WM_INITDIALOG: /* message: initialize dialog box */
    oldShowCoords = appData.showCoords;
    oldBlindfold  = appData.blindfold;
    oldShowButtonBar = appData.showButtonBar;
    oldAutoLogo  = appData.autoLogo;

    /* Center the dialog over the application window */
    CenterWindow (hDlg, GetWindow (hDlg, GW_OWNER));
    Translate(hDlg, DLG_GeneralOptions);

    /* Initialize the dialog items */
#define CHECK_BOX(x,y) CheckDlgButton(hDlg, (x), (BOOL)(y))

    CHECK_BOX(OPT_AlwaysOnTop, alwaysOnTop);
    CHECK_BOX(OPT_AlwaysQueen, appData.alwaysPromoteToQueen);
    CHECK_BOX(OPT_AnimateDragging, appData.animateDragging);
    CHECK_BOX(OPT_AnimateMoving, appData.animate);
    CHECK_BOX(OPT_AutoFlag, appData.autoCallFlag);
    CHECK_BOX(OPT_AutoFlipView, appData.autoFlipView);
    CHECK_BOX(OPT_AutoRaiseBoard, appData.autoRaiseBoard);
    CHECK_BOX(OPT_Blindfold, appData.blindfold);
    CHECK_BOX(OPT_HighlightDragging, appData.highlightDragging);
    CHECK_BOX(OPT_HighlightLastMove, appData.highlightLastMove);
    CHECK_BOX(OPT_PeriodicUpdates, appData.periodicUpdates);
    CHECK_BOX(OPT_PonderNextMove, appData.ponderNextMove);
    CHECK_BOX(OPT_PopupExitMessage, appData.popupExitMessage);
    CHECK_BOX(OPT_PopupMoveErrors, appData.popupMoveErrors);
    CHECK_BOX(OPT_ShowButtonBar, appData.showButtonBar);
    CHECK_BOX(OPT_ShowCoordinates, appData.showCoords);
    CHECK_BOX(OPT_ShowThinking, appData.showThinking);
    CHECK_BOX(OPT_TestLegality, appData.testLegality);
    CHECK_BOX(OPT_HideThinkFromHuman, appData.hideThinkingFromHuman);
    CHECK_BOX(OPT_SaveExtPGN, appData.saveExtendedInfoInPGN);
    CHECK_BOX(OPT_ExtraInfoInMoveHistory, appData.showEvalInMoveHistory);
    CHECK_BOX(OPT_HighlightMoveArrow, appData.highlightMoveWithArrow);
    CHECK_BOX(OPT_AutoLogo, appData.autoLogo); // [HGM] logo
    CHECK_BOX(OPT_SmartMove, appData.oneClick); // [HGM] one-click

#undef CHECK_BOX

    EnableWindow(GetDlgItem(hDlg, OPT_AutoFlag),
		 appData.icsActive || !appData.noChessProgram);
    EnableWindow(GetDlgItem(hDlg, OPT_AutoFlipView),
		 appData.icsActive || !appData.noChessProgram);
    EnableWindow(GetDlgItem(hDlg, OPT_PonderNextMove),
		 !appData.noChessProgram);
    EnableWindow(GetDlgItem(hDlg, OPT_PeriodicUpdates),
		 !appData.noChessProgram && !appData.icsActive);
    EnableWindow(GetDlgItem(hDlg, OPT_ShowThinking),
		 !appData.noChessProgram);
    return TRUE;


  case WM_COMMAND: /* message: received a command */
    switch (LOWORD(wParam)) {
    case IDOK:
      /* Read changed options from the dialog box */

#define IS_CHECKED(x) (Boolean)IsDlgButtonChecked(hDlg, (x))

      alwaysOnTop                  = IS_CHECKED(OPT_AlwaysOnTop);
      appData.alwaysPromoteToQueen = IS_CHECKED(OPT_AlwaysQueen);
      appData.animateDragging      = IS_CHECKED(OPT_AnimateDragging);
      appData.animate              = IS_CHECKED(OPT_AnimateMoving);
      appData.autoCallFlag         = IS_CHECKED(OPT_AutoFlag);
      appData.autoFlipView         = IS_CHECKED(OPT_AutoFlipView);
      appData.autoRaiseBoard       = IS_CHECKED(OPT_AutoRaiseBoard);
      appData.blindfold            = IS_CHECKED(OPT_Blindfold);
      appData.highlightDragging    = IS_CHECKED(OPT_HighlightDragging);
      appData.highlightLastMove    = IS_CHECKED(OPT_HighlightLastMove);
      PeriodicUpdatesEvent(          IS_CHECKED(OPT_PeriodicUpdates));
      PonderNextMoveEvent(           IS_CHECKED(OPT_PonderNextMove));
      appData.popupExitMessage     = IS_CHECKED(OPT_PopupExitMessage);
      appData.popupMoveErrors      = IS_CHECKED(OPT_PopupMoveErrors);
      appData.showButtonBar        = IS_CHECKED(OPT_ShowButtonBar);
      appData.showCoords           = IS_CHECKED(OPT_ShowCoordinates);
      // [HGM] thinking: next three moved up
      appData.saveExtendedInfoInPGN= IS_CHECKED(OPT_SaveExtPGN);
      appData.hideThinkingFromHuman= IS_CHECKED(OPT_HideThinkFromHuman);
      appData.showEvalInMoveHistory= IS_CHECKED(OPT_ExtraInfoInMoveHistory);
      appData.showThinking         = IS_CHECKED(OPT_ShowThinking);
      ShowThinkingEvent(); // [HGM] thinking: tests four options
      appData.testLegality         = IS_CHECKED(OPT_TestLegality);
      appData.highlightMoveWithArrow=IS_CHECKED(OPT_HighlightMoveArrow);
      appData.autoLogo             =IS_CHECKED(OPT_AutoLogo); // [HGM] logo
      appData.oneClick             =IS_CHECKED(OPT_SmartMove); // [HGM] one-click

#undef IS_CHECKED

      SetWindowPos(hwndMain, alwaysOnTop ? HWND_TOPMOST : HWND_NOTOPMOST,
		   0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
#if AOT_CONSOLE
      if (hwndConsole) {
	SetWindowPos(hwndConsole, alwaysOnTop ? HWND_TOPMOST : HWND_NOTOPMOST,
		     0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
      }
#endif
      if (!appData.highlightLastMove) {
	ClearHighlights();
	DrawPosition(FALSE, NULL);
      }
      /*
       * for some reason the redraw seems smoother when we invalidate
       * the board rect after the call to EndDialog()
       */
      EndDialog(hDlg, TRUE);

      if (oldAutoLogo != appData.autoLogo) { // [HGM] logo: remove any logos when we switch autologo off
	if(oldAutoLogo) first.programLogo = second.programLogo = NULL;
	InitDrawingSizes(boardSize, 0);
      } else if (oldShowButtonBar != appData.showButtonBar) {
	InitDrawingSizes(boardSize, 0);
      } else if ((oldShowCoords != appData.showCoords) ||
		 (oldBlindfold != appData.blindfold)) {
	InvalidateRect(hwndMain, &boardRect, FALSE);
      }

      return TRUE;

    case IDCANCEL:
      EndDialog(hDlg, FALSE);
      return TRUE;

    }
    break;
  }
  return FALSE;
}

VOID
GeneralOptionsPopup(HWND hwnd)
{
  FARPROC lpProc;

  lpProc = MakeProcInstance((FARPROC)GeneralOptionsDialog, hInst);
  DialogBox(hInst, MAKEINTRESOURCE(DLG_GeneralOptions), hwnd,
	    (DLGPROC) lpProc);
  FreeProcInstance(lpProc);
}
/*---------------------------------------------------------------------------*\
 *
 * Board Options Dialog functions
 *
\*---------------------------------------------------------------------------*/

const int SAMPLE_SQ_SIZE = 54;

VOID
ChangeBoardSize(BoardSize newSize)
{
  if (newSize != boardSize) {
    boardSize = newSize;
    InitDrawingSizes(boardSize, 0);
  }
}

VOID
PaintSampleSquare(
    HWND     hwnd,
    int      ctrlid,
    COLORREF squareColor,
    COLORREF pieceColor,
    COLORREF squareOutlineColor,
    COLORREF pieceDetailColor,
    BOOL     isWhitePiece,
    BOOL     isMono,
    HBITMAP  pieces[3]
    )
{
  HBRUSH  brushSquare;
  HBRUSH  brushSquareOutline;
  HBRUSH  brushPiece;
  HBRUSH  brushPieceDetail;
  HBRUSH  oldBrushPiece = NULL;
  HBRUSH  oldBrushSquare;
  HBITMAP oldBitmapMem;
  HBITMAP oldBitmapTemp;
  HBITMAP bufferBitmap;
  RECT    rect;
  HDC     hdcScreen, hdcMem, hdcTemp;
  HPEN    pen, oldPen;
  HWND    hCtrl = GetDlgItem(hwnd, ctrlid);
  int     x, y;

  const int SOLID   = 0;
  const int WHITE   = 1;
  const int OUTLINE = 2;
  const int BORDER  = 4;

  InvalidateRect(hCtrl, NULL, TRUE);
  UpdateWindow(hCtrl);
  GetClientRect(hCtrl, &rect);
  x = rect.left + (BORDER / 2);
  y = rect.top  + (BORDER / 2);
  hdcScreen = GetDC(hCtrl);
  hdcMem  = CreateCompatibleDC(hdcScreen);
  hdcTemp = CreateCompatibleDC(hdcScreen);

  bufferBitmap = CreateCompatibleBitmap(hdcScreen, rect.right-rect.left+1,
					rect.bottom-rect.top+1);
  oldBitmapMem = SelectObject(hdcMem, bufferBitmap);
  if (!isMono) {
    SelectPalette(hdcMem, hPal, FALSE);
  }
  brushSquare         = CreateSolidBrush(squareColor);
  brushSquareOutline  = CreateSolidBrush(squareOutlineColor);
  brushPiece          = CreateSolidBrush(pieceColor);
  brushPieceDetail    = CreateSolidBrush(pieceDetailColor);

  /*
   * first draw the rectangle
   */
  pen      = CreatePen(PS_SOLID, BORDER, squareOutlineColor);
  oldPen   = (HPEN)  SelectObject(hdcMem, pen);
  oldBrushSquare = (HBRUSH)SelectObject(hdcMem, brushSquare);
  Rectangle(hdcMem, rect.left, rect.top, rect.right, rect.bottom);

  /*
   * now draw the piece
   */
  if (isMono) {
    oldBitmapTemp = SelectObject(hdcTemp, pieces[OUTLINE]);
    BitBlt(hdcMem, x, y, SAMPLE_SQ_SIZE, SAMPLE_SQ_SIZE, hdcTemp, 0, 0,
	   isWhitePiece ? SRCCOPY : NOTSRCCOPY);
    SelectObject(hdcTemp, oldBitmapTemp);
  } else {
    if (isWhitePiece) {
      oldBitmapTemp = SelectObject(hdcTemp, pieces[WHITE]);
      oldBrushPiece = SelectObject(hdcMem, brushPiece);
      BitBlt(hdcMem, x, y, SAMPLE_SQ_SIZE, SAMPLE_SQ_SIZE,
	     hdcTemp, 0, 0, 0x00B8074A);
      /* Use black for outline of white pieces */
      SelectObject(hdcTemp, pieces[OUTLINE]);
      BitBlt(hdcMem, x, y, SAMPLE_SQ_SIZE, SAMPLE_SQ_SIZE,
	     hdcTemp, 0, 0, SRCAND);
    } else {
      /* Use square color for details of black pieces */
      oldBitmapTemp = SelectObject(hdcTemp, pieces[SOLID]);
      oldBrushPiece = SelectObject(hdcMem, brushPiece);
      BitBlt(hdcMem, x, y, SAMPLE_SQ_SIZE, SAMPLE_SQ_SIZE,
	     hdcTemp, 0, 0, 0x00B8074A);
    }
    SelectObject(hdcMem, oldBrushPiece);
    SelectObject(hdcTemp, oldBitmapTemp);
  }
  /*
   * copy the memory dc to the screen
   */
  SelectObject(hdcMem, bufferBitmap);
  BitBlt(hdcScreen, rect.left, rect.top,
	 rect.right - rect.left,
	 rect.bottom - rect.top,
	 hdcMem, rect.left, rect.top, SRCCOPY);
  SelectObject(hdcMem, oldBitmapMem);
  /*
   * clean up
   */
  SelectObject(hdcMem, oldBrushPiece);
  SelectObject(hdcMem, oldPen);
  DeleteObject(brushPiece);
  DeleteObject(brushPieceDetail);
  DeleteObject(brushSquare);
  DeleteObject(brushSquareOutline);
  DeleteObject(pen);
  DeleteDC(hdcTemp);
  DeleteDC(hdcMem);
  ReleaseDC(hCtrl, hdcScreen);
}


VOID
PaintColorBlock(HWND hwnd, int ctrlid, COLORREF color)
{
  HDC    hdc;
  HBRUSH brush, oldBrush;
  RECT   rect;
  HWND   hCtrl = GetDlgItem(hwnd, ctrlid);

  hdc = GetDC(hCtrl);
  InvalidateRect(hCtrl, NULL, TRUE);
  UpdateWindow(hCtrl);
  GetClientRect(hCtrl, &rect);
  brush = CreateSolidBrush(color);
  oldBrush = (HBRUSH)SelectObject(hdc, brush);
  Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);
  SelectObject(hdc, oldBrush);
  DeleteObject(brush);
  ReleaseDC(hCtrl, hdc);
}


VOID
SetBoardOptionEnables(HWND hDlg)
{
  if (IsDlgButtonChecked(hDlg, OPT_Monochrome)) {
    ShowWindow(GetDlgItem(hDlg, OPT_LightSquareColor), SW_HIDE);
    ShowWindow(GetDlgItem(hDlg, OPT_DarkSquareColor), SW_HIDE);
    ShowWindow(GetDlgItem(hDlg, OPT_WhitePieceColor), SW_HIDE);
    ShowWindow(GetDlgItem(hDlg, OPT_BlackPieceColor), SW_HIDE);

    EnableWindow(GetDlgItem(hDlg, OPT_ChooseLightSquareColor), FALSE);
    EnableWindow(GetDlgItem(hDlg, OPT_ChooseDarkSquareColor), FALSE);
    EnableWindow(GetDlgItem(hDlg, OPT_ChooseWhitePieceColor), FALSE);
    EnableWindow(GetDlgItem(hDlg, OPT_ChooseBlackPieceColor), FALSE);
  } else {
    ShowWindow(GetDlgItem(hDlg, OPT_LightSquareColor), SW_SHOW);
    ShowWindow(GetDlgItem(hDlg, OPT_DarkSquareColor), SW_SHOW);
    ShowWindow(GetDlgItem(hDlg, OPT_WhitePieceColor), SW_SHOW);
    ShowWindow(GetDlgItem(hDlg, OPT_BlackPieceColor), SW_SHOW);

    EnableWindow(GetDlgItem(hDlg, OPT_ChooseLightSquareColor), TRUE);
    EnableWindow(GetDlgItem(hDlg, OPT_ChooseDarkSquareColor), TRUE);
    EnableWindow(GetDlgItem(hDlg, OPT_ChooseWhitePieceColor), TRUE);
    EnableWindow(GetDlgItem(hDlg, OPT_ChooseBlackPieceColor), TRUE);
  }
}

BoardSize
BoardOptionsWhichRadio(HWND hDlg)
{
  return (IsDlgButtonChecked(hDlg, OPT_SizeTiny) ? SizeTiny :
         (IsDlgButtonChecked(hDlg, OPT_SizeTeeny) ? SizeTeeny :
         (IsDlgButtonChecked(hDlg, OPT_SizeDinky) ? SizeDinky :
         (IsDlgButtonChecked(hDlg, OPT_SizePetite) ? SizePetite :
         (IsDlgButtonChecked(hDlg, OPT_SizeSlim) ? SizeSlim :
         (IsDlgButtonChecked(hDlg, OPT_SizeSmall) ? SizeSmall :
         (IsDlgButtonChecked(hDlg, OPT_SizeMediocre) ? SizeMediocre :
         (IsDlgButtonChecked(hDlg, OPT_SizeMiddling) ? SizeMiddling :
         (IsDlgButtonChecked(hDlg, OPT_SizeAverage) ? SizeAverage :
         (IsDlgButtonChecked(hDlg, OPT_SizeModerate) ? SizeModerate :
         (IsDlgButtonChecked(hDlg, OPT_SizeMedium) ? SizeMedium :
         (IsDlgButtonChecked(hDlg, OPT_SizeBulky) ? SizeBulky :
         (IsDlgButtonChecked(hDlg, OPT_SizeLarge) ? SizeLarge :
         (IsDlgButtonChecked(hDlg, OPT_SizeBig) ? SizeBig :
         (IsDlgButtonChecked(hDlg, OPT_SizeHuge) ? SizeHuge :
         (IsDlgButtonChecked(hDlg, OPT_SizeGiant) ? SizeGiant :
         (IsDlgButtonChecked(hDlg, OPT_SizeColossal) ? SizeColossal :
          SizeTitanic )))))))))))))))));
}

LRESULT CALLBACK
BoardOptionsDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  static Boolean  mono, white, flip, fonts, bitmaps, grid;
  static BoardSize size;
  static COLORREF lsc, dsc, wpc, bpc, hsc, phc;
  static HBITMAP pieces[3];

  switch (message) {
  case WM_INITDIALOG: /* message: initialize dialog box */
    /* Center the dialog over the application window */
    CenterWindow (hDlg, GetWindow (hDlg, GW_OWNER));
    Translate(hDlg, DLG_BoardOptions);
    /* Initialize the dialog items */
    switch (boardSize) {
    case SizeTiny:
      CheckDlgButton(hDlg, OPT_SizeTiny, TRUE);
      break;
    case SizeTeeny:
      CheckDlgButton(hDlg, OPT_SizeTeeny, TRUE);
      break;
    case SizeDinky:
      CheckDlgButton(hDlg, OPT_SizeDinky, TRUE);
      break;
    case SizePetite:
      CheckDlgButton(hDlg, OPT_SizePetite, TRUE);
      break;
    case SizeSlim:
      CheckDlgButton(hDlg, OPT_SizeSlim, TRUE);
      break;
    case SizeSmall:
      CheckDlgButton(hDlg, OPT_SizeSmall, TRUE);
      break;
    case SizeMediocre:
      CheckDlgButton(hDlg, OPT_SizeMediocre, TRUE);
      break;
    case SizeMiddling:
      CheckDlgButton(hDlg, OPT_SizeMiddling, TRUE);
      break;
    case SizeAverage:
      CheckDlgButton(hDlg, OPT_SizeAverage, TRUE);
      break;
    case SizeModerate:
      CheckDlgButton(hDlg, OPT_SizeModerate, TRUE);
      break;
    case SizeMedium:
      CheckDlgButton(hDlg, OPT_SizeMedium, TRUE);
      break;
    case SizeBulky:
      CheckDlgButton(hDlg, OPT_SizeBulky, TRUE);
      break;
    case SizeLarge:
      CheckDlgButton(hDlg, OPT_SizeLarge, TRUE);
      break;
    case SizeBig:
      CheckDlgButton(hDlg, OPT_SizeBig, TRUE);
      break;
    case SizeHuge:
      CheckDlgButton(hDlg, OPT_SizeHuge, TRUE);
      break;
    case SizeGiant:
      CheckDlgButton(hDlg, OPT_SizeGiant, TRUE);
      break;
    case SizeColossal:
      CheckDlgButton(hDlg, OPT_SizeColossal, TRUE);
      break;
    case SizeTitanic:
      CheckDlgButton(hDlg, OPT_SizeTitanic, TRUE);
    default: ; // should not happen, but suppresses warning on pedantic compilers
    }

    if (appData.monoMode)
      CheckDlgButton(hDlg, OPT_Monochrome, TRUE);

    if (appData.allWhite)
      CheckDlgButton(hDlg, OPT_AllWhite, TRUE);

    if (appData.upsideDown)
      CheckDlgButton(hDlg, OPT_UpsideDown, TRUE);

    if (appData.useBitmaps)
      CheckDlgButton(hDlg, OPT_Bitmaps, TRUE);

    if (appData.useFont)
      CheckDlgButton(hDlg, OPT_PieceFont, TRUE);

    if (appData.overrideLineGap >= 0)
      CheckDlgButton(hDlg, OPT_Grid, TRUE);

    pieces[0] = DoLoadBitmap(hInst, "n", SAMPLE_SQ_SIZE, "s");
    pieces[1] = DoLoadBitmap(hInst, "n", SAMPLE_SQ_SIZE, "w");
    pieces[2] = DoLoadBitmap(hInst, "n", SAMPLE_SQ_SIZE, "o");

    lsc = lightSquareColor;
    dsc = darkSquareColor;
    fonts = appData.useFont;
    wpc = fonts ? appData.fontBackColorWhite : whitePieceColor;
    bpc = fonts ? appData.fontForeColorBlack : blackPieceColor;
    hsc = highlightSquareColor;
    phc = premoveHighlightColor;
    mono = appData.monoMode;
    white= appData.allWhite;
    flip = appData.upsideDown;
    size = boardSize;
    bitmaps = appData.useBitmaps;
    grid = appData.overrideLineGap >= 0;

    SetBoardOptionEnables(hDlg);
    return TRUE;

  case WM_PAINT:
    PaintColorBlock(hDlg, OPT_LightSquareColor, lsc);
    PaintColorBlock(hDlg, OPT_DarkSquareColor,  dsc);
    PaintColorBlock(hDlg, OPT_WhitePieceColor,  wpc);
    PaintColorBlock(hDlg, OPT_BlackPieceColor,  bpc);
    PaintColorBlock(hDlg, OPT_HighlightSquareColor, hsc);
    PaintColorBlock(hDlg, OPT_PremoveHighlightColor, phc);
    PaintSampleSquare(hDlg, OPT_SampleLightSquare, lsc, wpc, hsc, bpc,
	TRUE, mono, pieces);
    PaintSampleSquare(hDlg, OPT_SampleDarkSquare, dsc, bpc, phc, wpc,
	FALSE, mono, pieces);

    return FALSE;

  case WM_COMMAND: /* message: received a command */
    switch (LOWORD(wParam)) {
    case IDOK:
      /*
       * if we call EndDialog() after the call to ChangeBoardSize(),
       * then ChangeBoardSize() does not take effect, although the new
       * boardSize is saved. Go figure...
       */
      EndDialog(hDlg, TRUE);

      size = BoardOptionsWhichRadio(hDlg);

      /*
       * did any settings change?
       */
      if (size != boardSize) {
	ChangeBoardSize(size);
      }

      if (bitmaps && !appData.useBitmaps) InitTextures();

      if ((mono != appData.monoMode) ||
	  (lsc  != lightSquareColor) ||
	  (dsc  != darkSquareColor) ||
	  (wpc  != fonts ? appData.fontBackColorWhite : whitePieceColor) ||
	  (bpc  != fonts ? appData.fontForeColorBlack : blackPieceColor) ||
	  (hsc  != highlightSquareColor) ||
          (flip != appData.upsideDown) ||
          (white != appData.allWhite) ||
          (fonts != appData.useFont) ||
          (bitmaps != appData.useBitmaps) ||
          (grid != appData.overrideLineGap >= 0) ||
	  (phc  != premoveHighlightColor)) {

	  lightSquareColor = lsc;
	  darkSquareColor = dsc;
	  if(fonts) {
	    appData.fontBackColorWhite = wpc;
	    appData.fontForeColorBlack = bpc;
	  } else {
	    whitePieceColor = wpc;
	    blackPieceColor = bpc;
	  }
	  highlightSquareColor = hsc;
	  premoveHighlightColor = phc;
	  appData.monoMode = mono;
          appData.allWhite = white;
          appData.upsideDown = flip;
          appData.useFont = fonts;
          appData.useBitmaps = bitmaps;
          if(grid != appData.overrideLineGap >= 0) appData.overrideLineGap = grid - 1;

	  InitDrawingColors();
	  InitDrawingSizes(boardSize, 0);
	  InvalidateRect(hwndMain, &boardRect, FALSE);
      }
      DeleteObject(pieces[0]);
      DeleteObject(pieces[1]);
      DeleteObject(pieces[2]);
      return TRUE;

    case IDCANCEL:
      DeleteObject(pieces[0]);
      DeleteObject(pieces[1]);
      DeleteObject(pieces[2]);
      EndDialog(hDlg, FALSE);
      return TRUE;

    case OPT_ChooseLightSquareColor:
      if (ChangeColor(hDlg, &lsc))
	PaintColorBlock(hDlg, OPT_LightSquareColor, lsc);
	PaintSampleSquare(hDlg, OPT_SampleLightSquare, lsc, wpc, hsc, bpc,
	    TRUE, mono, pieces);
      break;

    case OPT_ChooseDarkSquareColor:
      if (ChangeColor(hDlg, &dsc))
	PaintColorBlock(hDlg, OPT_DarkSquareColor, dsc);
	PaintSampleSquare(hDlg, OPT_SampleDarkSquare, dsc, bpc, phc, wpc,
	    FALSE, mono, pieces);
      break;

    case OPT_ChooseWhitePieceColor:
      if (ChangeColor(hDlg, &wpc))
	PaintColorBlock(hDlg, OPT_WhitePieceColor, wpc);
	PaintSampleSquare(hDlg, OPT_SampleLightSquare, lsc, wpc, hsc, bpc,
	    TRUE, mono, pieces);
      break;

    case OPT_ChooseBlackPieceColor:
      if (ChangeColor(hDlg, &bpc))
	PaintColorBlock(hDlg, OPT_BlackPieceColor, bpc);
	PaintSampleSquare(hDlg, OPT_SampleDarkSquare, dsc, bpc, phc, wpc,
	    FALSE, mono, pieces);
      break;

    case OPT_ChooseHighlightSquareColor:
      if (ChangeColor(hDlg, &hsc))
	PaintColorBlock(hDlg, OPT_HighlightSquareColor, hsc);
	PaintSampleSquare(hDlg, OPT_SampleLightSquare, lsc, wpc, hsc, bpc,
	    TRUE, mono, pieces);
      break;

    case OPT_ChoosePremoveHighlightColor:
      if (ChangeColor(hDlg, &phc))
	PaintColorBlock(hDlg, OPT_PremoveHighlightColor, phc);
	PaintSampleSquare(hDlg, OPT_SampleDarkSquare, dsc, bpc, phc, wpc,
	    FALSE, mono, pieces);
      break;

    case OPT_DefaultBoardColors:
      lsc = ParseColorName(LIGHT_SQUARE_COLOR);
      dsc = ParseColorName(DARK_SQUARE_COLOR);
      wpc = ParseColorName(WHITE_PIECE_COLOR);
      bpc = ParseColorName(BLACK_PIECE_COLOR);
      hsc = ParseColorName(HIGHLIGHT_SQUARE_COLOR);
      phc = ParseColorName(PREMOVE_HIGHLIGHT_COLOR);
      mono = FALSE;
      white= FALSE;
      flip = FALSE;
      CheckDlgButton(hDlg, OPT_Monochrome, FALSE);
      CheckDlgButton(hDlg, OPT_AllWhite,   FALSE);
      CheckDlgButton(hDlg, OPT_UpsideDown, FALSE);
      PaintColorBlock(hDlg, OPT_LightSquareColor, lsc);
      PaintColorBlock(hDlg, OPT_DarkSquareColor,  dsc);
      PaintColorBlock(hDlg, OPT_WhitePieceColor,  wpc);
      PaintColorBlock(hDlg, OPT_BlackPieceColor,  bpc);
      PaintColorBlock(hDlg, OPT_HighlightSquareColor, hsc);
      PaintColorBlock(hDlg, OPT_PremoveHighlightColor, phc);
      SetBoardOptionEnables(hDlg);
      PaintSampleSquare(hDlg, OPT_SampleLightSquare, lsc, wpc, hsc, bpc,
	  TRUE, mono, pieces);
      PaintSampleSquare(hDlg, OPT_SampleDarkSquare, dsc, bpc, phc, wpc,
	  FALSE, mono, pieces);
      break;

    case OPT_Monochrome:
      mono = !mono;
      SetBoardOptionEnables(hDlg);
      break;

    case OPT_AllWhite:
      white = !white;
      break;

    case OPT_UpsideDown:
      flip = !flip;
      break;

    case OPT_Bitmaps:
      bitmaps = !bitmaps;
      break;

    case OPT_PieceFont:
      fonts = !fonts;
      break;

    case OPT_Grid:
      grid = !grid;
      break;
    }
    break;
  }
  return FALSE;
}


VOID
BoardOptionsPopup(HWND hwnd)
{
  FARPROC lpProc = MakeProcInstance((FARPROC)BoardOptionsDialog, hInst);
  DialogBox(hInst, MAKEINTRESOURCE(DLG_BoardOptions), hwnd,
	  (DLGPROC) lpProc);
  FreeProcInstance(lpProc);
}

int radioButton[] = {
    OPT_VariantNormal,
    -1, // Loadable
    OPT_VariantWildcastle,
    OPT_VariantNocastle,
    OPT_VariantFRC,
    OPT_VariantBughouse,
    OPT_VariantCrazyhouse,
    OPT_VariantLosers,
    OPT_VariantSuicide,
    OPT_VariantGiveaway,
    OPT_VariantTwoKings,
    -1, //Kriegspiel
    OPT_VariantAtomic,
    OPT_Variant3Check,
    OPT_VariantShatranj,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    OPT_VariantShogi,
    OPT_VariantXiangqi,
    OPT_VariantCourier,
    OPT_VariantGothic,
    OPT_VariantCapablanca,
    OPT_VariantKnightmate,
    OPT_VariantFairy,        
    OPT_VariantCylinder,
    OPT_VariantFalcon,
    OPT_VariantCRC,
    OPT_VariantBerolina,
    OPT_VariantJanus,
    OPT_VariantSuper,
    OPT_VariantGreat,
    -1, // Twilight,
    OPT_VariantMakruk,
    OPT_VariantSChess,
    OPT_VariantGrand,
    OPT_VariantSpartan, // Spartan
    -2 // sentinel
};

VariantClass
VariantWhichRadio(HWND hDlg)
{
  int i=0, j;
  while((j = radioButton[i++]) != -2) {
	if(j == -1) continue; // no menu button
	if(IsDlgButtonChecked(hDlg, j) &&
	   (appData.noChessProgram || strstr(first.variants, VariantName(i-1)))) return (VariantClass) i-1;
  }
  return gameInfo.variant; // If no button checked, keep old
}

void
VariantShowRadio(HWND hDlg)
{
  int i=0, j;
  CheckDlgButton(hDlg, radioButton[gameInfo.variant], TRUE);
  while((j = radioButton[i++]) != -2) {
	if(j == -1) continue; // no menu button
	EnableWindow(GetDlgItem(hDlg, j), appData.noChessProgram || strstr(first.variants, VariantName(i-1)));
  }
}

LRESULT CALLBACK
NewVariantDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  static VariantClass v;
  static int n1_ok, n2_ok, n3_ok;

  switch (message) {
  case WM_INITDIALOG: /* message: initialize dialog box */
    /* Center the dialog over the application window */
    CenterWindow (hDlg, GetWindow (hDlg, GW_OWNER));
    Translate(hDlg, DLG_NewVariant);

    /* Initialize the dialog items */
    VariantShowRadio(hDlg);

    SetDlgItemInt( hDlg, IDC_Files, -1, TRUE );
    SendDlgItemMessage( hDlg, IDC_Files, EM_SETSEL, 0, -1 );

    SetDlgItemInt( hDlg, IDC_Ranks, -1, TRUE );
    SendDlgItemMessage( hDlg, IDC_Ranks, EM_SETSEL, 0, -1 );

    SetDlgItemInt( hDlg, IDC_Holdings, -1, TRUE );
    SendDlgItemMessage( hDlg, IDC_Ranks, EM_SETSEL, 0, -1 );

    n1_ok = n2_ok = n3_ok = FALSE;

    return TRUE;

  case WM_COMMAND: /* message: received a command */
    switch (LOWORD(wParam)) {
    case IDOK:
      /*
       * if we call EndDialog() after the call to ChangeBoardSize(),
       * then ChangeBoardSize() does not take effect, although the new
       * boardSize is saved. Go figure...
       */
      EndDialog(hDlg, TRUE);

      v = VariantWhichRadio(hDlg);
      if(!appData.noChessProgram) {
	char *name = VariantName(v), buf[MSG_SIZ];
	if (first.protocolVersion > 1 && StrStr(first.variants, name) == NULL) {
	  /* [HGM] in protocol 2 we check if variant is suported by engine */
	  snprintf(buf, MSG_SIZ, _("Variant %s not supported by %s"), name, first.tidy);
	  DisplayError(buf, 0);
	  return TRUE; /* treat as _("Cancel") if first engine does not support it */
	} else
	if (second.initDone && second.protocolVersion > 1 && StrStr(second.variants, name) == NULL) {
	  snprintf(buf, MSG_SIZ, _("Warning: second engine (%s) does not support this!"), second.tidy);
	  DisplayError(buf, 0);   /* use of second engine is optional; only warn user */
	}
      }

      gameInfo.variant = v;
      appData.variant = VariantName(v);

      appData.NrFiles = (int) GetDlgItemInt(hDlg, IDC_Files, NULL, FALSE );
      appData.NrRanks = (int) GetDlgItemInt(hDlg, IDC_Ranks, NULL, FALSE );
      appData.holdingsSize = (int) GetDlgItemInt(hDlg, IDC_Holdings, NULL, FALSE );

      if(!n1_ok) appData.NrFiles = -1;
      if(!n2_ok) appData.NrRanks = -1;
      if(!n3_ok) appData.holdingsSize = -1;

      shuffleOpenings = FALSE; /* [HGM] shuffle: possible shuffle reset when we switch */
      startedFromPositionFile = FALSE; /* [HGM] loadPos: no longer valid in new variant */
      appData.pieceToCharTable = NULL;
      Reset(TRUE, TRUE);

      return TRUE;

    case IDCANCEL:
      EndDialog(hDlg, FALSE);
      return TRUE;

    case IDC_Ranks:
    case IDC_Files:
    case IDC_Holdings:
        if( HIWORD(wParam) == EN_CHANGE ) {

            GetDlgItemInt(hDlg, IDC_Files, &n1_ok, FALSE );
            GetDlgItemInt(hDlg, IDC_Ranks, &n2_ok, FALSE );
            GetDlgItemInt(hDlg, IDC_Holdings, &n3_ok, FALSE );

            /*EnableWindow( GetDlgItem(hDlg, IDOK), n1_ok && n2_ok && n3_ok ? TRUE : FALSE );*/
        }
        return TRUE;
    }
    break;
  }
  return FALSE;
}


VOID
NewVariantPopup(HWND hwnd)
{
  FARPROC lpProc = MakeProcInstance((FARPROC)NewVariantDialog, hInst);
  DialogBox(hInst, MAKEINTRESOURCE(DLG_NewVariant), hwnd,
	  (DLGPROC) lpProc);
  FreeProcInstance(lpProc);
}

/*---------------------------------------------------------------------------*\
 *
 * ICS Options Dialog functions
 *
\*---------------------------------------------------------------------------*/

BOOL APIENTRY
MyCreateFont(HWND hwnd, MyFont *font)
{
  CHOOSEFONT cf;
  HFONT hf;

  /* Initialize members of the CHOOSEFONT structure. */
  cf.lStructSize = sizeof(CHOOSEFONT);
  cf.hwndOwner = hwnd;
  cf.hDC = (HDC)NULL;
  cf.lpLogFont = &font->lf;
  cf.iPointSize = 0;
  cf.Flags = CF_SCREENFONTS|/*CF_ANSIONLY|*/CF_INITTOLOGFONTSTRUCT;
  cf.rgbColors = RGB(0,0,0);
  cf.lCustData = 0L;
  cf.lpfnHook = (LPCFHOOKPROC)NULL;
  cf.lpTemplateName = (LPSTR)NULL;
  cf.hInstance = (HINSTANCE) NULL;
  cf.lpszStyle = (LPSTR)NULL;
  cf.nFontType = SCREEN_FONTTYPE;
  cf.nSizeMin = 0;
  cf.nSizeMax = 0;

  /* Display the CHOOSEFONT common-dialog box. */
  if (!ChooseFont(&cf)) {
    return FALSE;
  }

  /* Create a logical font based on the user's   */
  /* selection and return a handle identifying   */
  /* that font. */
  hf = CreateFontIndirect(cf.lpLogFont);
  if (hf == NULL) {
    return FALSE;
  }

  font->hf = hf;
  font->mfp.pointSize = (float) (cf.iPointSize / 10.0);
  font->mfp.bold = (font->lf.lfWeight >= FW_BOLD);
  font->mfp.italic = font->lf.lfItalic;
  font->mfp.underline = font->lf.lfUnderline;
  font->mfp.strikeout = font->lf.lfStrikeOut;
  font->mfp.charset = font->lf.lfCharSet;
  safeStrCpy(font->mfp.faceName, font->lf.lfFaceName, sizeof(font->mfp.faceName)/sizeof(font->mfp.faceName[0]) );
  return TRUE;
}


VOID
UpdateSampleText(HWND hDlg, int id, MyColorizeAttribs *mca)
{
  CHARFORMAT cf;
  cf.cbSize = sizeof(CHARFORMAT);
  cf.dwMask =
    CFM_COLOR|CFM_CHARSET|CFM_BOLD|CFM_ITALIC|CFM_UNDERLINE|CFM_STRIKEOUT|CFM_FACE|CFM_SIZE;
  cf.crTextColor = mca->color;
  cf.dwEffects = mca->effects;
  safeStrCpy(cf.szFaceName, font[boardSize][CONSOLE_FONT]->mfp.faceName, sizeof(cf.szFaceName)/sizeof(cf.szFaceName[0]) );
  /*
   * The 20.0 below is in fact documented. yHeight is expressed in twips.
   * A twip is 1/20 of a font's point size. See documentation of CHARFORMAT.
   * --msw
   */
  cf.yHeight = (int)(font[boardSize][CONSOLE_FONT]->mfp.pointSize * 20.0 + 0.5);
  cf.bCharSet = DEFAULT_CHARSET; /* should be ignored anyway */
  cf.bPitchAndFamily = DEFAULT_PITCH|FF_DONTCARE;
  SendDlgItemMessage(hDlg, id, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
}

LRESULT CALLBACK
ColorizeTextDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  static MyColorizeAttribs mca;
  static ColorClass cc;
  COLORREF background = (COLORREF)0;

  switch (message) {
  case WM_INITDIALOG:
    cc = (ColorClass)lParam;
    mca = colorizeAttribs[cc];
    /* Center the dialog over the application window */
    CenterWindow (hDlg, GetWindow (hDlg, GW_OWNER));
    Translate(hDlg, DLG_Colorize);
    /* Initialize the dialog items */
    CheckDlgButton(hDlg, OPT_Bold, (mca.effects & CFE_BOLD) != 0);
    CheckDlgButton(hDlg, OPT_Italic, (mca.effects & CFE_ITALIC) != 0);
    CheckDlgButton(hDlg, OPT_Underline, (mca.effects & CFE_UNDERLINE) != 0);
    CheckDlgButton(hDlg, OPT_Strikeout, (mca.effects & CFE_STRIKEOUT) != 0);

    /* get the current background color from the parent window */
    SendMessage(GetWindow(hDlg, GW_OWNER),WM_COMMAND,
        	(WPARAM)WM_USER_GetConsoleBackground,
	        (LPARAM)&background);

    /* set the background color */
    SendDlgItemMessage(hDlg, OPT_Sample, EM_SETBKGNDCOLOR, FALSE, background);

    SetDlgItemText(hDlg, OPT_Sample, T_(mca.name));
    UpdateSampleText(hDlg, OPT_Sample, &mca);
    return TRUE;

  case WM_COMMAND: /* message: received a command */
    switch (LOWORD(wParam)) {
    case IDOK:
      /* Read changed options from the dialog box */
      colorizeAttribs[cc] = mca;
      textAttribs[cc].color = mca.color;
      textAttribs[cc].effects = mca.effects;
      Colorize(currentColorClass, TRUE);
      if (cc == ColorNormal) {
	CHARFORMAT cf;
	cf.cbSize = sizeof(CHARFORMAT);
	cf.dwMask = CFM_COLOR;
	cf.crTextColor = mca.color;
	SendDlgItemMessage(hwndConsole, OPT_ConsoleInput,
	  EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
      }
      EndDialog(hDlg, TRUE);
      return TRUE;

    case IDCANCEL:
      EndDialog(hDlg, FALSE);
      return TRUE;

    case OPT_ChooseColor:
      ChangeColor(hDlg, &mca.color);
      UpdateSampleText(hDlg, OPT_Sample, &mca);
      return TRUE;

    default:
      mca.effects =
	(IsDlgButtonChecked(hDlg, OPT_Bold) ? CFE_BOLD : 0) |
	(IsDlgButtonChecked(hDlg, OPT_Italic) ? CFE_ITALIC : 0) |
	(IsDlgButtonChecked(hDlg, OPT_Underline) ? CFE_UNDERLINE : 0) |
	(IsDlgButtonChecked(hDlg, OPT_Strikeout) ? CFE_STRIKEOUT : 0);
      UpdateSampleText(hDlg, OPT_Sample, &mca);
      break;
    }
    break;
  }
  return FALSE;
}

VOID
ColorizeTextPopup(HWND hwnd, ColorClass cc)
{
  FARPROC lpProc;

  lpProc = MakeProcInstance((FARPROC)ColorizeTextDialog, hInst);
  DialogBoxParam(hInst, MAKEINTRESOURCE(DLG_Colorize),
    hwnd, (DLGPROC)lpProc, (LPARAM) cc);
  FreeProcInstance(lpProc);
}

VOID
SetIcsOptionEnables(HWND hDlg)
{
#define ENABLE_DLG_ITEM(x,y) EnableWindow(GetDlgItem(hDlg,(x)), (y))

  UINT state = IsDlgButtonChecked(hDlg, OPT_Premove);
  ENABLE_DLG_ITEM(OPT_PremoveWhite, state);
  ENABLE_DLG_ITEM(OPT_PremoveWhiteText, state);
  ENABLE_DLG_ITEM(OPT_PremoveBlack, state);
  ENABLE_DLG_ITEM(OPT_PremoveBlackText, state);

  ENABLE_DLG_ITEM(OPT_IcsAlarmTime, IsDlgButtonChecked(hDlg, OPT_IcsAlarm));

#undef ENABLE_DLG_ITEM
}


LRESULT CALLBACK
IcsOptionsDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  char buf[MSG_SIZ];
  int number;
  int i;
  static COLORREF cbc;
  static MyColorizeAttribs *mca;
  COLORREF *colorref;

  switch (message) {
  case WM_INITDIALOG: /* message: initialize dialog box */

    mca = colorizeAttribs;

    for (i=0; i < NColorClasses - 1; i++) {
      mca[i].color   = textAttribs[i].color;
      mca[i].effects = textAttribs[i].effects;
    }
    cbc = consoleBackgroundColor;

    /* Center the dialog over the application window */
    CenterWindow (hDlg, GetWindow (hDlg, GW_OWNER));
    Translate(hDlg, DLG_IcsOptions);

    /* Initialize the dialog items */
#define CHECK_BOX(x,y) CheckDlgButton(hDlg, (x), (BOOL)(y))

    CHECK_BOX(OPT_AutoKibitz, appData.autoKibitz);
    CHECK_BOX(OPT_AutoComment, appData.autoComment);
    CHECK_BOX(OPT_AutoObserve, appData.autoObserve);
    CHECK_BOX(OPT_AutoCreate, appData.autoCreateLogon);
    CHECK_BOX(OPT_GetMoveList, appData.getMoveList);
    CHECK_BOX(OPT_LocalLineEditing, appData.localLineEditing);
    CHECK_BOX(OPT_QuietPlay, appData.quietPlay);
    CHECK_BOX(OPT_SeekGraph, appData.seekGraph);
    CHECK_BOX(OPT_AutoRefresh, appData.autoRefresh);
    CHECK_BOX(OPT_BgObserve, appData.bgObserve);
    CHECK_BOX(OPT_DualBoard, appData.dualBoard);
    CHECK_BOX(OPT_SmartMove, appData.oneClick);
    CHECK_BOX(OPT_Premove, appData.premove);
    CHECK_BOX(OPT_PremoveWhite, appData.premoveWhite);
    CHECK_BOX(OPT_PremoveBlack, appData.premoveBlack);
    CHECK_BOX(OPT_IcsAlarm, appData.icsAlarm);
    CHECK_BOX(OPT_DontColorize, !appData.colorize);

#undef CHECK_BOX

    snprintf(buf, MSG_SIZ, "%d", appData.icsAlarmTime / 1000);
    SetDlgItemText(hDlg, OPT_IcsAlarmTime, buf);
    SetDlgItemText(hDlg, OPT_PremoveWhiteText, appData.premoveWhiteText);
    SetDlgItemText(hDlg, OPT_PremoveBlackText, appData.premoveBlackText);
    SetDlgItemText(hDlg, OPT_StartupChatBoxes, appData.chatBoxes);

    SendDlgItemMessage(hDlg, OPT_SampleShout,     EM_SETBKGNDCOLOR, 0, cbc);
    SendDlgItemMessage(hDlg, OPT_SampleSShout,    EM_SETBKGNDCOLOR, 0, cbc);
    SendDlgItemMessage(hDlg, OPT_SampleChannel1,  EM_SETBKGNDCOLOR, 0, cbc);
    SendDlgItemMessage(hDlg, OPT_SampleChannel,   EM_SETBKGNDCOLOR, 0, cbc);
    SendDlgItemMessage(hDlg, OPT_SampleKibitz,    EM_SETBKGNDCOLOR, 0, cbc);
    SendDlgItemMessage(hDlg, OPT_SampleTell,      EM_SETBKGNDCOLOR, 0, cbc);
    SendDlgItemMessage(hDlg, OPT_SampleChallenge, EM_SETBKGNDCOLOR, 0, cbc);
    SendDlgItemMessage(hDlg, OPT_SampleRequest,   EM_SETBKGNDCOLOR, 0, cbc);
    SendDlgItemMessage(hDlg, OPT_SampleSeek,      EM_SETBKGNDCOLOR, 0, cbc);
    SendDlgItemMessage(hDlg, OPT_SampleNormal,    EM_SETBKGNDCOLOR, 0, cbc);

    SetDlgItemText(hDlg, OPT_SampleShout,     T_(mca[ColorShout].name));
    SetDlgItemText(hDlg, OPT_SampleSShout,    T_(mca[ColorSShout].name));
    SetDlgItemText(hDlg, OPT_SampleChannel1,  T_(mca[ColorChannel1].name));
    SetDlgItemText(hDlg, OPT_SampleChannel,   T_(mca[ColorChannel].name));
    SetDlgItemText(hDlg, OPT_SampleKibitz,    T_(mca[ColorKibitz].name));
    SetDlgItemText(hDlg, OPT_SampleTell,      T_(mca[ColorTell].name));
    SetDlgItemText(hDlg, OPT_SampleChallenge, T_(mca[ColorChallenge].name));
    SetDlgItemText(hDlg, OPT_SampleRequest,   T_(mca[ColorRequest].name));
    SetDlgItemText(hDlg, OPT_SampleSeek,      T_(mca[ColorSeek].name));
    SetDlgItemText(hDlg, OPT_SampleNormal,    T_(mca[ColorNormal].name));

    UpdateSampleText(hDlg, OPT_SampleShout,     &mca[ColorShout]);
    UpdateSampleText(hDlg, OPT_SampleSShout,    &mca[ColorSShout]);
    UpdateSampleText(hDlg, OPT_SampleChannel1,  &mca[ColorChannel1]);
    UpdateSampleText(hDlg, OPT_SampleChannel,   &mca[ColorChannel]);
    UpdateSampleText(hDlg, OPT_SampleKibitz,    &mca[ColorKibitz]);
    UpdateSampleText(hDlg, OPT_SampleTell,      &mca[ColorTell]);
    UpdateSampleText(hDlg, OPT_SampleChallenge, &mca[ColorChallenge]);
    UpdateSampleText(hDlg, OPT_SampleRequest,   &mca[ColorRequest]);
    UpdateSampleText(hDlg, OPT_SampleSeek,      &mca[ColorSeek]);
    UpdateSampleText(hDlg, OPT_SampleNormal,    &mca[ColorNormal]);

    SetIcsOptionEnables(hDlg);
    return TRUE;

  case WM_COMMAND: /* message: received a command */
    switch (LOWORD(wParam)) {

    case WM_USER_GetConsoleBackground:
      /* the ColorizeTextDialog needs the current background color */
      colorref = (COLORREF *)lParam;
      *colorref = cbc;
      return FALSE;

    case IDOK:
      /* Read changed options from the dialog box */
      GetDlgItemText(hDlg, OPT_IcsAlarmTime, buf, MSG_SIZ);
      if (sscanf(buf, "%d", &number) != 1 || (number < 0)){
	  MessageBox(hDlg, _("Invalid ICS Alarm Time"),
		     _("Option Error"), MB_OK|MB_ICONEXCLAMATION);
	  return FALSE;
      }

#define IS_CHECKED(x) (Boolean)IsDlgButtonChecked(hDlg, (x))

      appData.icsAlarm         = IS_CHECKED(OPT_IcsAlarm);
      appData.premove          = IS_CHECKED(OPT_Premove);
      appData.premoveWhite     = IS_CHECKED(OPT_PremoveWhite);
      appData.premoveBlack     = IS_CHECKED(OPT_PremoveBlack);
      appData.autoKibitz       = IS_CHECKED(OPT_AutoKibitz);
      appData.autoComment      = IS_CHECKED(OPT_AutoComment);
      appData.autoObserve      = IS_CHECKED(OPT_AutoObserve);
      appData.autoCreateLogon  = IS_CHECKED(OPT_AutoCreate);
      appData.getMoveList      = IS_CHECKED(OPT_GetMoveList);
      appData.localLineEditing = IS_CHECKED(OPT_LocalLineEditing);
      appData.quietPlay        = IS_CHECKED(OPT_QuietPlay);
      appData.seekGraph        = IS_CHECKED(OPT_SeekGraph);
      appData.autoRefresh      = IS_CHECKED(OPT_AutoRefresh);
      appData.bgObserve        = IS_CHECKED(OPT_BgObserve);
      appData.dualBoard        = IS_CHECKED(OPT_DualBoard);
      appData.oneClick         = IS_CHECKED(OPT_SmartMove);

#undef IS_CHECKED

      appData.icsAlarmTime = number * 1000;
      GetDlgItemText(hDlg, OPT_PremoveWhiteText, appData.premoveWhiteText, 5);
      GetDlgItemText(hDlg, OPT_PremoveBlackText, appData.premoveBlackText, 5);
      GetDlgItemText(hDlg, OPT_StartupChatBoxes, buf, sizeof(buf));
      buf[sizeof(buf)-1] = NULLCHAR; appData.chatBoxes = StrSave(buf); // memory leak

      if (appData.localLineEditing) {
	DontEcho();
	EchoOn();
      } else {
	DoEcho();
	EchoOff();
      }

      appData.colorize =
	(Boolean)!IsDlgButtonChecked(hDlg, OPT_DontColorize);

    ChangedConsoleFont();

    if (!appData.colorize) {
	CHARFORMAT cf;
	COLORREF background = ParseColorName(COLOR_BKGD);
	/*
	SetDefaultTextAttribs();
        Colorize(currentColorClass);
	*/
	cf.cbSize = sizeof(CHARFORMAT);
	cf.dwMask = CFM_COLOR;
	cf.crTextColor = ParseColorName(COLOR_NORMAL);

	SendDlgItemMessage(hwndConsole, OPT_ConsoleInput,
	  EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
        SendDlgItemMessage(hwndConsole, OPT_ConsoleText,
  	  EM_SETBKGNDCOLOR, FALSE, background);
	SendDlgItemMessage(hwndConsole, OPT_ConsoleInput,
	  EM_SETBKGNDCOLOR, FALSE, background);
      }

      if (cbc != consoleBackgroundColor) {
	consoleBackgroundColor = cbc;
	if (appData.colorize) {
	  SendDlgItemMessage(hwndConsole, OPT_ConsoleText,
	    EM_SETBKGNDCOLOR, FALSE, consoleBackgroundColor);
	  SendDlgItemMessage(hwndConsole, OPT_ConsoleInput,
	    EM_SETBKGNDCOLOR, FALSE, consoleBackgroundColor);
	}
      }

      for (i=0; i < NColorClasses - 1; i++) {
	textAttribs[i].color   = mca[i].color;
	textAttribs[i].effects = mca[i].effects;
      }

      EndDialog(hDlg, TRUE);
      return TRUE;

    case IDCANCEL:
      EndDialog(hDlg, FALSE);
      return TRUE;

    case OPT_ChooseShoutColor:
      ColorizeTextPopup(hDlg, ColorShout);
      UpdateSampleText(hDlg, OPT_SampleShout, &mca[ColorShout]);
      break;

    case OPT_ChooseSShoutColor:
      ColorizeTextPopup(hDlg, ColorSShout);
      UpdateSampleText(hDlg, OPT_SampleSShout, &mca[ColorSShout]);
      break;

    case OPT_ChooseChannel1Color:
      ColorizeTextPopup(hDlg, ColorChannel1);
      UpdateSampleText(hDlg, OPT_SampleChannel1,
		       &colorizeAttribs[ColorChannel1]);
      break;

    case OPT_ChooseChannelColor:
      ColorizeTextPopup(hDlg, ColorChannel);
      UpdateSampleText(hDlg, OPT_SampleChannel, &mca[ColorChannel]);
      break;

    case OPT_ChooseKibitzColor:
      ColorizeTextPopup(hDlg, ColorKibitz);
      UpdateSampleText(hDlg, OPT_SampleKibitz, &mca[ColorKibitz]);
      break;

    case OPT_ChooseTellColor:
      ColorizeTextPopup(hDlg, ColorTell);
      UpdateSampleText(hDlg, OPT_SampleTell, &mca[ColorTell]);
      break;

    case OPT_ChooseChallengeColor:
      ColorizeTextPopup(hDlg, ColorChallenge);
      UpdateSampleText(hDlg, OPT_SampleChallenge, &mca[ColorChallenge]);
      break;

    case OPT_ChooseRequestColor:
      ColorizeTextPopup(hDlg, ColorRequest);
      UpdateSampleText(hDlg, OPT_SampleRequest, &mca[ColorRequest]);
      break;

    case OPT_ChooseSeekColor:
      ColorizeTextPopup(hDlg, ColorSeek);
      UpdateSampleText(hDlg, OPT_SampleSeek, &mca[ColorSeek]);
      break;

    case OPT_ChooseNormalColor:
      ColorizeTextPopup(hDlg, ColorNormal);
      UpdateSampleText(hDlg, OPT_SampleNormal, &mca[ColorNormal]);
      break;

    case OPT_ChooseBackgroundColor:
      if (ChangeColor(hDlg, &cbc)) {
	SendDlgItemMessage(hDlg, OPT_SampleShout,     EM_SETBKGNDCOLOR, 0, cbc);
	SendDlgItemMessage(hDlg, OPT_SampleSShout,    EM_SETBKGNDCOLOR, 0, cbc);
	SendDlgItemMessage(hDlg, OPT_SampleChannel1,  EM_SETBKGNDCOLOR, 0, cbc);
	SendDlgItemMessage(hDlg, OPT_SampleChannel,   EM_SETBKGNDCOLOR, 0, cbc);
	SendDlgItemMessage(hDlg, OPT_SampleKibitz,    EM_SETBKGNDCOLOR, 0, cbc);
	SendDlgItemMessage(hDlg, OPT_SampleTell,      EM_SETBKGNDCOLOR, 0, cbc);
	SendDlgItemMessage(hDlg, OPT_SampleChallenge, EM_SETBKGNDCOLOR, 0, cbc);
	SendDlgItemMessage(hDlg, OPT_SampleRequest,   EM_SETBKGNDCOLOR, 0, cbc);
	SendDlgItemMessage(hDlg, OPT_SampleSeek,      EM_SETBKGNDCOLOR, 0, cbc);
	SendDlgItemMessage(hDlg, OPT_SampleNormal,    EM_SETBKGNDCOLOR, 0, cbc);
      }
      break;

    case OPT_DefaultColors:
      for (i=0; i < NColorClasses - 1; i++)
	ParseAttribs(&mca[i].color,
		     &mca[i].effects,
		     defaultTextAttribs[i]);

      cbc = ParseColorName(COLOR_BKGD);
      SendDlgItemMessage(hDlg, OPT_SampleShout,     EM_SETBKGNDCOLOR, 0, cbc);
      SendDlgItemMessage(hDlg, OPT_SampleSShout,    EM_SETBKGNDCOLOR, 0, cbc);
      SendDlgItemMessage(hDlg, OPT_SampleChannel1,  EM_SETBKGNDCOLOR, 0, cbc);
      SendDlgItemMessage(hDlg, OPT_SampleChannel,   EM_SETBKGNDCOLOR, 0, cbc);
      SendDlgItemMessage(hDlg, OPT_SampleKibitz,    EM_SETBKGNDCOLOR, 0, cbc);
      SendDlgItemMessage(hDlg, OPT_SampleTell,      EM_SETBKGNDCOLOR, 0, cbc);
      SendDlgItemMessage(hDlg, OPT_SampleChallenge, EM_SETBKGNDCOLOR, 0, cbc);
      SendDlgItemMessage(hDlg, OPT_SampleRequest,   EM_SETBKGNDCOLOR, 0, cbc);
      SendDlgItemMessage(hDlg, OPT_SampleSeek,      EM_SETBKGNDCOLOR, 0, cbc);
      SendDlgItemMessage(hDlg, OPT_SampleNormal,    EM_SETBKGNDCOLOR, 0, cbc);

      UpdateSampleText(hDlg, OPT_SampleShout,     &mca[ColorShout]);
      UpdateSampleText(hDlg, OPT_SampleSShout,    &mca[ColorSShout]);
      UpdateSampleText(hDlg, OPT_SampleChannel1,  &mca[ColorChannel1]);
      UpdateSampleText(hDlg, OPT_SampleChannel,   &mca[ColorChannel]);
      UpdateSampleText(hDlg, OPT_SampleKibitz,    &mca[ColorKibitz]);
      UpdateSampleText(hDlg, OPT_SampleTell,      &mca[ColorTell]);
      UpdateSampleText(hDlg, OPT_SampleChallenge, &mca[ColorChallenge]);
      UpdateSampleText(hDlg, OPT_SampleRequest,   &mca[ColorRequest]);
      UpdateSampleText(hDlg, OPT_SampleSeek,      &mca[ColorSeek]);
      UpdateSampleText(hDlg, OPT_SampleNormal,    &mca[ColorNormal]);
      break;

    default:
      SetIcsOptionEnables(hDlg);
      break;
    }
    break;
  }
  return FALSE;
}

VOID
IcsOptionsPopup(HWND hwnd)
{
  FARPROC lpProc = MakeProcInstance((FARPROC)IcsOptionsDialog, hInst);
  DialogBox(hInst, MAKEINTRESOURCE(DLG_IcsOptions), hwnd,
	    (DLGPROC) lpProc);
  FreeProcInstance(lpProc);
}

/*---------------------------------------------------------------------------*\
 *
 * Fonts Dialog functions
 *
\*---------------------------------------------------------------------------*/

char *string; // sorry

VOID
SetSampleFontText(HWND hwnd, int id, const MyFont *mf)
{
  char buf[MSG_SIZ];
  HWND hControl;
  HDC hdc;
  CHARFORMAT cf;
  SIZE size;
  RECT rectClient, rectFormat;
  HFONT oldFont;
  POINT center;
  int len;

  len = snprintf(buf, MSG_SIZ, "%.0f pt. %s%s%s\n",
		 mf->mfp.pointSize, mf->mfp.faceName,
		 mf->mfp.bold ? " bold" : "",
		 mf->mfp.italic ? " italic" : "");
 if(id != OPT_SamplePieceFont)
  SetDlgItemText(hwnd, id, buf);
 else SetDlgItemText(hwnd, id, string);

  hControl = GetDlgItem(hwnd, id);
  hdc = GetDC(hControl);
  SetMapMode(hdc, MM_TEXT);	/* 1 pixel == 1 logical unit */
  oldFont = SelectObject(hdc, mf->hf);

  /* get number of logical units necessary to display font name */
  GetTextExtentPoint32(hdc, buf, len, &size);

  /* calculate formatting rectangle in the rich edit control.
   * May be larger or smaller than the actual control.
   */
  GetClientRect(hControl, &rectClient);
  center.x = (rectClient.left + rectClient.right) / 2;
  center.y = (rectClient.top  + rectClient.bottom) / 2;
  rectFormat.top    = center.y - (size.cy / 2) - 1;
  rectFormat.bottom = center.y + (size.cy / 2) + 1;
  rectFormat.left   = center.x - (size.cx / 2) - 1;
  rectFormat.right  = center.x + (size.cx / 2) + 1;

  cf.cbSize = sizeof(CHARFORMAT);
  cf.dwMask = CFM_FACE|CFM_SIZE|CFM_CHARSET|CFM_BOLD|CFM_ITALIC;
  cf.dwEffects = 0;
  if (mf->lf.lfWeight == FW_BOLD) cf.dwEffects |= CFE_BOLD;
  if (mf->lf.lfItalic) cf.dwEffects |= CFE_ITALIC;
  safeStrCpy(cf.szFaceName, mf->mfp.faceName, sizeof(cf.szFaceName)/sizeof(cf.szFaceName[0]) );
  /*
   * yHeight is expressed in twips.  A twip is 1/20 of a font's point
   * size. See documentation of CHARFORMAT.  --msw
   */
  cf.yHeight = (int)(mf->mfp.pointSize * 20.0 + 0.5);
  cf.bCharSet = mf->lf.lfCharSet;
  cf.bPitchAndFamily = mf->lf.lfPitchAndFamily;

  /* format the text in the rich edit control */
  SendMessage(hControl, EM_SETCHARFORMAT, SCF_ALL, (LPARAM) &cf);
 if(id != OPT_SamplePieceFont)
  SendMessage(hControl, EM_SETRECT, (WPARAM)0, (LPARAM) &rectFormat);

  /* clean up */
  SelectObject(hdc, oldFont);
  ReleaseDC(hControl, hdc);
}

VOID
CopyFont(MyFont *dest, const MyFont *src)
{
  dest->mfp.pointSize = src->mfp.pointSize;
  dest->mfp.bold      = src->mfp.bold;
  dest->mfp.italic    = src->mfp.italic;
  dest->mfp.underline = src->mfp.underline;
  dest->mfp.strikeout = src->mfp.strikeout;
  dest->mfp.charset   = src->mfp.charset;
  safeStrCpy(dest->mfp.faceName, src->mfp.faceName, sizeof(dest->mfp.faceName)/sizeof(dest->mfp.faceName[0]) );
  CreateFontInMF(dest);
}


LRESULT CALLBACK
FontOptionsDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  static MyFont workFont[NUM_FONTS+1];
  static BOOL firstPaint;
  static char pieceText[] = "ABCDEFGHIJKLMNOPQRSTUVWXZabcdefghijklmnopqrstuvwxyz";
  int i;
  RECT rect;

  switch (message) {
  case WM_INITDIALOG:

    /* copy the current font settings into a working copy */
    for (i=0; i < NUM_FONTS; i++)
      CopyFont(&workFont[i], font[boardSize][i]);
    strncpy(workFont[NUM_FONTS].mfp.faceName, appData.renderPiecesWithFont, sizeof(workFont[NUM_FONTS].mfp.faceName));
    workFont[NUM_FONTS].mfp.pointSize = 16.;
    workFont[NUM_FONTS].mfp.charset = DEFAULT_CHARSET;

    Translate(hDlg, DLG_Fonts);
    if (!appData.icsActive)
      EnableWindow(GetDlgItem(hDlg, OPT_ChooseConsoleFont), FALSE);

    firstPaint = TRUE;	/* see rant below */

    /* If I don't call SetFocus(), the dialog won't respond to the keyboard
     * when first drawn. Why is this the only dialog that behaves this way? Is
     * is the WM_PAINT stuff below?? Sigh...
     */
    SetFocus(GetDlgItem(hDlg, IDOK));
    break;

  case WM_PAINT:
    /* This should not be necessary. However, if SetSampleFontText() is called
     * in response to WM_INITDIALOG, the strings are not properly centered in
     * the controls when the dialog first appears. I can't figure out why, so
     * this is the workaround.  --msw
     */
    if (firstPaint) {
      SetSampleFontText(hDlg, OPT_SampleClockFont, &workFont[CLOCK_FONT]);
      SetSampleFontText(hDlg, OPT_SampleMessageFont, &workFont[MESSAGE_FONT]);
      SetSampleFontText(hDlg, OPT_SampleCoordFont, &workFont[COORD_FONT]);
      SetSampleFontText(hDlg, OPT_SampleTagFont, &workFont[EDITTAGS_FONT]);
      SetSampleFontText(hDlg, OPT_SampleCommentsFont, &workFont[COMMENT_FONT]);
      SetSampleFontText(hDlg, OPT_SampleConsoleFont, &workFont[CONSOLE_FONT]);
      SetSampleFontText(hDlg, OPT_SampleMoveHistoryFont, &workFont[MOVEHISTORY_FONT]);
      SetSampleFontText(hDlg, OPT_SampleGameListFont, &workFont[GAMELIST_FONT]);
      string = appData.fontToPieceTable;
      SetSampleFontText(hDlg, OPT_SamplePieceFont, &workFont[NUM_FONTS]);
      firstPaint = FALSE;
    }
    break;

  case WM_COMMAND: /* message: received a command */
    switch (LOWORD(wParam)) {

    case IDOK:
      /* again, it seems to avoid redraw problems if we call EndDialog first */
      EndDialog(hDlg, FALSE);

      /* copy modified settings back to the fonts array */
      for (i=0; i < NUM_FONTS; i++)
	CopyFont(font[boardSize][i], &workFont[i]);

      { // Make new piece-to-char table
	char buf[MSG_SIZ];
	GetDlgItemText(hDlg, OPT_SamplePieceFont, buf, MSG_SIZ);
	ASSIGN(appData.fontToPieceTable, buf);
      }
      ASSIGN(appData.renderPiecesWithFont, workFont[NUM_FONTS].mfp.faceName); // piece font

      /* a sad necessity due to the original design of having a separate
       * console font, tags font, and comment font for each board size.  IMHO
       * these fonts should not be dependent on the current board size.  I'm
       * running out of time, so I am doing this hack rather than redesign the
       * data structure. Besides, I think if I redesigned the data structure, I
       * might break backwards compatibility with old winboard.ini files.
       * --msw
       */
      for (i=0; i < NUM_SIZES; i++) {
	CopyFont(font[i][EDITTAGS_FONT], &workFont[EDITTAGS_FONT]);
	CopyFont(font[i][CONSOLE_FONT],  &workFont[CONSOLE_FONT]);
	CopyFont(font[i][COMMENT_FONT],  &workFont[COMMENT_FONT]);
	CopyFont(font[i][MOVEHISTORY_FONT],  &workFont[MOVEHISTORY_FONT]);
	CopyFont(font[i][GAMELIST_FONT],  &workFont[GAMELIST_FONT]);
      }
      /* end sad necessity */

      InitDrawingSizes(boardSize, 0);
      InvalidateRect(hwndMain, NULL, TRUE);

      if (commentDialog) {
	SendDlgItemMessage(commentDialog, OPT_CommentText,
	  WM_SETFONT, (WPARAM)font[boardSize][COMMENT_FONT]->hf,
	  MAKELPARAM(TRUE, 0));
	GetClientRect(GetDlgItem(commentDialog, OPT_CommentText), &rect);
	InvalidateRect(commentDialog, &rect, TRUE);
      }

      if (editTagsDialog) {
	SendDlgItemMessage(editTagsDialog, OPT_TagsText,
  	  WM_SETFONT, (WPARAM)font[boardSize][EDITTAGS_FONT]->hf,
	  MAKELPARAM(TRUE, 0));
	GetClientRect(GetDlgItem(editTagsDialog, OPT_TagsText), &rect);
	InvalidateRect(editTagsDialog, &rect, TRUE);
      }

      if( moveHistoryDialog != NULL ) {
	SendDlgItemMessage(moveHistoryDialog, IDC_MoveHistory,
  	  WM_SETFONT, (WPARAM)font[boardSize][MOVEHISTORY_FONT]->hf,
	  MAKELPARAM(TRUE, 0));
        SendMessage( moveHistoryDialog, WM_INITDIALOG, 0, 0 );
//	InvalidateRect(editTagsDialog, NULL, TRUE); // [HGM] this ws improperly cloned?
      }

      if( engineOutputDialog != NULL ) {
	SendDlgItemMessage(engineOutputDialog, IDC_EngineMemo1,
  	  WM_SETFONT, (WPARAM)font[boardSize][MOVEHISTORY_FONT]->hf,
	  MAKELPARAM(TRUE, 0));
	SendDlgItemMessage(engineOutputDialog, IDC_EngineMemo2,
  	  WM_SETFONT, (WPARAM)font[boardSize][MOVEHISTORY_FONT]->hf,
	  MAKELPARAM(TRUE, 0));
      }

      if (hwndConsole) {
	ChangedConsoleFont();
      }

      for (i=0; i<NUM_FONTS; i++)
	DeleteObject(&workFont[i].hf);

      return TRUE;

    case IDCANCEL:
      for (i=0; i<NUM_FONTS; i++)
	DeleteObject(&workFont[i].hf);
      EndDialog(hDlg, FALSE);
      return TRUE;

    case OPT_ChooseClockFont:
      MyCreateFont(hDlg, &workFont[CLOCK_FONT]);
      SetSampleFontText(hDlg, OPT_SampleClockFont, &workFont[CLOCK_FONT]);
      break;

    case OPT_ChooseMessageFont:
      MyCreateFont(hDlg, &workFont[MESSAGE_FONT]);
      SetSampleFontText(hDlg, OPT_SampleMessageFont, &workFont[MESSAGE_FONT]);
      break;

    case OPT_ChooseCoordFont:
      MyCreateFont(hDlg, &workFont[COORD_FONT]);
      SetSampleFontText(hDlg, OPT_SampleCoordFont, &workFont[COORD_FONT]);
      break;

    case OPT_ChooseTagFont:
      MyCreateFont(hDlg, &workFont[EDITTAGS_FONT]);
      SetSampleFontText(hDlg, OPT_SampleTagFont, &workFont[EDITTAGS_FONT]);
      break;

    case OPT_ChooseCommentsFont:
      MyCreateFont(hDlg, &workFont[COMMENT_FONT]);
      SetSampleFontText(hDlg, OPT_SampleCommentsFont, &workFont[COMMENT_FONT]);
      break;

    case OPT_ChooseConsoleFont:
      MyCreateFont(hDlg, &workFont[CONSOLE_FONT]);
      SetSampleFontText(hDlg, OPT_SampleConsoleFont, &workFont[CONSOLE_FONT]);
      break;

    case OPT_ChooseMoveHistoryFont:
      MyCreateFont(hDlg, &workFont[MOVEHISTORY_FONT]);
      SetSampleFontText(hDlg, OPT_SampleMoveHistoryFont, &workFont[MOVEHISTORY_FONT]);
      break;

    case OPT_ChooseGameListFont:
      MyCreateFont(hDlg, &workFont[GAMELIST_FONT]);
      SetSampleFontText(hDlg, OPT_SampleGameListFont, &workFont[GAMELIST_FONT]);
      break;

    case OPT_ChoosePieceFont:
      MyCreateFont(hDlg, &workFont[NUM_FONTS]);
      string = pieceText;
      SetSampleFontText(hDlg, OPT_SamplePieceFont, &workFont[NUM_FONTS]);
      break;

    case OPT_DefaultFonts:
      for (i=0; i<NUM_FONTS; i++) {
	DeleteObject(&workFont[i].hf);
	ParseFontName(font[boardSize][i]->def, &workFont[i].mfp);
	CreateFontInMF(&workFont[i]);
      }
      SetSampleFontText(hDlg, OPT_SampleClockFont, &workFont[CLOCK_FONT]);
      SetSampleFontText(hDlg, OPT_SampleMessageFont, &workFont[MESSAGE_FONT]);
      SetSampleFontText(hDlg, OPT_SampleCoordFont, &workFont[COORD_FONT]);
      SetSampleFontText(hDlg, OPT_SampleTagFont, &workFont[EDITTAGS_FONT]);
      SetSampleFontText(hDlg, OPT_SampleCommentsFont, &workFont[COMMENT_FONT]);
      SetSampleFontText(hDlg, OPT_SampleConsoleFont, &workFont[CONSOLE_FONT]);
      SetSampleFontText(hDlg, OPT_SampleMoveHistoryFont, &workFont[MOVEHISTORY_FONT]);
      SetSampleFontText(hDlg, OPT_SampleGameListFont, &workFont[GAMELIST_FONT]);
      break;
    }
  }
  return FALSE;
}

VOID
FontsOptionsPopup(HWND hwnd)
{
  FARPROC lpProc = MakeProcInstance((FARPROC)FontOptionsDialog, hInst);
  DialogBox(hInst, MAKEINTRESOURCE(DLG_Fonts), hwnd,
	  (DLGPROC) lpProc);
  FreeProcInstance(lpProc);
}

/*---------------------------------------------------------------------------*\
 *
 * Sounds Dialog functions
 *
\*---------------------------------------------------------------------------*/


SoundComboData soundComboData[] = {
  {N_("Move"), NULL},
  {N_("Bell"), NULL},
  {N_("ICS Alarm"), NULL},
  {N_("ICS Win"), NULL},
  {N_("ICS Loss"), NULL},
  {N_("ICS Draw"), NULL},
  {N_("ICS Unfinished"), NULL},
  {N_("Shout"), NULL},
  {N_("SShout/CShout"), NULL},
  {N_("Channel 1"), NULL},
  {N_("Channel"), NULL},
  {N_("Kibitz"), NULL},
  {N_("Tell"), NULL},
  {N_("Challenge"), NULL},
  {N_("Request"), NULL},
  {N_("Seek"), NULL},
  {NULL, NULL},
};


void
InitSoundComboData(SoundComboData *scd)
{
  SoundClass sc;
  ColorClass cc;
  int index;

  /* copy current sound settings to combo array */

  for ( sc = (SoundClass)0; sc < NSoundClasses; sc++) {
    scd[sc].name = strdup(sounds[sc].name);
  }
  for ( cc = (ColorClass)0; cc < NColorClasses - 2; cc++) {
    index = (int)cc + (int)NSoundClasses;
    scd[index].name = strdup(textAttribs[cc].sound.name);
  }
}


void
ResetSoundComboData(SoundComboData *scd)
{
  while (scd->label) {
    if (scd->name != NULL) {
      free (scd->name);
      scd->name = NULL;
    }
    scd++;
  }
}

void
InitSoundCombo(HWND hwndCombo, SoundComboData *scd)
{
  char buf[255];
  DWORD err;
  DWORD cnt = 0;
  SendMessage(hwndCombo, CB_RESETCONTENT, 0, 0);

  /* send the labels to the combo box */
  while (scd->label) {
    err = SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM) T_(scd->label));
    if (err != cnt++) {
      snprintf(buf, MSG_SIZ,  "InitSoundCombo(): err '%d', cnt '%d'\n",
	  (int)err, (int)cnt);
      MessageBox(NULL, buf, NULL, MB_OK);
    }
    scd++;
  }
  SendMessage(hwndCombo, CB_SETCURSEL, (WPARAM) 0, (LPARAM) 0);
}

int
SoundDialogWhichRadio(HWND hDlg)
{
  if (IsDlgButtonChecked(hDlg, OPT_NoSound)) return OPT_NoSound;
  if (IsDlgButtonChecked(hDlg, OPT_DefaultBeep)) return OPT_DefaultBeep;
  if (IsDlgButtonChecked(hDlg, OPT_BuiltInSound)) return OPT_BuiltInSound;
  if (IsDlgButtonChecked(hDlg, OPT_WavFile)) return OPT_WavFile;
  return -1;
}

VOID
SoundDialogSetEnables(HWND hDlg, int radio)
{
  EnableWindow(GetDlgItem(hDlg, OPT_BuiltInSoundName),
	       radio == OPT_BuiltInSound);
  EnableWindow(GetDlgItem(hDlg, OPT_WavFileName), radio == OPT_WavFile);
  EnableWindow(GetDlgItem(hDlg, OPT_BrowseSound), radio == OPT_WavFile);
}

char *
SoundDialogGetName(HWND hDlg, int radio)
{
  static char buf[MSG_SIZ], buf2[MSG_SIZ], buf3[MSG_SIZ];
  char *dummy, *ret;
  switch (radio) {
  case OPT_NoSound:
  default:
    return "";
  case OPT_DefaultBeep:
    return "$";
  case OPT_BuiltInSound:
    buf[0] = '!';
    GetDlgItemText(hDlg, OPT_BuiltInSoundName, buf + 1, sizeof(buf) - 1);
    return buf;
  case OPT_WavFile:
    GetDlgItemText(hDlg, OPT_WavFileName, buf, sizeof(buf));
    GetCurrentDirectory(MSG_SIZ, buf3);
    SetCurrentDirectory(installDir);
    if (GetFullPathName(buf, MSG_SIZ, buf2, &dummy)) {
      ret = buf2;
    } else {
      ret = buf;
    }
    SetCurrentDirectory(buf3);
    return ret;
  }
}

void
DisplaySelectedSound(HWND hDlg, HWND hCombo, const char *name)
{
  int radio;
  /*
   * I think it's best to clear the combo and edit boxes. It looks stupid
   * to have a value from another sound event sitting there grayed out.
   */
  SetDlgItemText(hDlg, OPT_WavFileName, "");
  SendMessage(hCombo, CB_SETCURSEL, (WPARAM) -1, (LPARAM) 0);

  if (appData.debugMode)
      fprintf(debugFP, "DisplaySelectedSound(,,'%s'):\n", name);
  switch (name[0]) {
  case NULLCHAR:
    radio = OPT_NoSound;
    break;
  case '$':
    if (name[1] == NULLCHAR) {
      radio = OPT_DefaultBeep;
    } else {
      radio = OPT_WavFile;
      SetDlgItemText(hDlg, OPT_WavFileName, name);
    }
    break;
  case '!':
    if (name[1] == NULLCHAR) {
      radio = OPT_NoSound;
    } else {
      radio = OPT_BuiltInSound;
      if (SendMessage(hCombo, CB_SELECTSTRING, (WPARAM) -1,
		      (LPARAM) (name + 1)) == CB_ERR) {
	SendMessage(hCombo, CB_SETCURSEL, (WPARAM) -1, (LPARAM) 0);
	SendMessage(hCombo, WM_SETTEXT, (WPARAM) 0, (LPARAM) (name + 1));
      }
    }
    break;
  default:
    radio = OPT_WavFile;
    SetDlgItemText(hDlg, OPT_WavFileName, name);
    break;
  }
  SoundDialogSetEnables(hDlg, radio);
  CheckRadioButton(hDlg, OPT_NoSound, OPT_WavFile, radio);
}


char *builtInSoundNames[] = BUILT_IN_SOUND_NAMES;

LRESULT CALLBACK
SoundOptionsDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  static HWND hSoundCombo;
  static DWORD index;
  static HWND hBISN;
  int radio;
  MySound tmp;
  FILE *f;
  char buf[MSG_SIZ];
  char *newName;
  SoundClass sc;
  ColorClass cc;
  SoundComboData *scd;
  int oldMute;

  switch (message) {
  case WM_INITDIALOG:
    /* Center the dialog over the application window */
    CenterWindow (hDlg, GetWindow (hDlg, GW_OWNER));
    Translate(hDlg, DLG_Sound);

    /* Initialize the built-in sounds combo */
    hBISN = GetDlgItem(hDlg, OPT_BuiltInSoundName);
     InitComboStrings(hBISN, builtInSoundNames);

    /* Initialize the  sound events combo */
    index = 0;
    InitSoundComboData(soundComboData);
    hSoundCombo = GetDlgItem(hDlg, CBO_Sounds);
    InitSoundCombo(hSoundCombo, soundComboData);

    /* update the dialog */
    DisplaySelectedSound(hDlg, hBISN, soundComboData[index].name);
    return TRUE;

  case WM_COMMAND: /* message: received a command */

    if (((HWND)lParam == hSoundCombo) &&
	(HIWORD(wParam) == CBN_SELCHANGE)) {
      /*
       * the user has selected a new sound event. We must store the name for
       * the previously selected event, then retrieve the name for the
       * newly selected event and update the dialog.
       */
      radio = SoundDialogWhichRadio(hDlg);
      newName = strdup(SoundDialogGetName(hDlg, radio));

      if (strcmp(newName, soundComboData[index].name) != 0) {
	free(soundComboData[index].name);
	soundComboData[index].name = newName;
      } else {
	free(newName);
	newName = NULL;
      }
      /* now get the settings for the newly selected event */
      index = SendMessage(hSoundCombo, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
      DisplaySelectedSound(hDlg, hBISN, soundComboData[index].name);

      return TRUE;
    }
    switch (LOWORD(wParam)) {
    case IDOK:
      /*
       * save the name for the currently selected sound event
       */
      radio = SoundDialogWhichRadio(hDlg);
      newName = strdup(SoundDialogGetName(hDlg, radio));

      if (strcmp(soundComboData[index].name, newName) != 0) {
	free(soundComboData[index].name);
	soundComboData[index].name = newName;
      } else {
	free(newName);
	newName = NULL;
      }

      /* save all the sound names that changed and load the sounds */

      for ( sc = (SoundClass)0; sc < NSoundClasses; sc++) {
	if (strcmp(soundComboData[sc].name, sounds[sc].name) != 0) {
	  free(sounds[sc].name);
	  sounds[sc].name = strdup(soundComboData[sc].name);
	  MyLoadSound(&sounds[sc]);
	}
      }
      for ( cc = (ColorClass)0; cc < NColorClasses - 2; cc++) {
	index = (int)cc + (int)NSoundClasses;
	if (strcmp(soundComboData[index].name,
		   textAttribs[cc].sound.name) != 0) {
	  free(textAttribs[cc].sound.name);
	  textAttribs[cc].sound.name = strdup(soundComboData[index].name);
	  MyLoadSound(&textAttribs[cc].sound);
	}
      }

	mute = FALSE; // [HGM] mute: switch sounds automatically on if we select one
      CheckMenuItem(GetMenu(hwndMain),IDM_MuteSounds,MF_BYCOMMAND|MF_UNCHECKED);
      ResetSoundComboData(soundComboData);
      EndDialog(hDlg, TRUE);
      return TRUE;

    case IDCANCEL:
      ResetSoundComboData(soundComboData);
      EndDialog(hDlg, FALSE);
      return TRUE;

    case OPT_DefaultSounds:
      /* can't use SetDefaultSounds() because we need to be able to "undo" if
       * user selects "Cancel" later on. So we do it the hard way here.
       */
      scd = &soundComboData[0];
      while (scd->label != NULL) {
	if (scd->name != NULL) free(scd->name);
	scd->name = strdup("");
	scd++;
      }
      free(soundComboData[(int)SoundBell].name);
      soundComboData[(int)SoundBell].name = strdup(SOUND_BELL);
      DisplaySelectedSound(hDlg, hBISN, soundComboData[index].name);
      break;

    case OPT_PlaySound:
      radio = SoundDialogWhichRadio(hDlg);
      tmp.name = strdup(SoundDialogGetName(hDlg, radio));
      tmp.data = NULL;
      MyLoadSound(&tmp);
	oldMute = mute; mute = FALSE; // [HGM] mute: always sound when user presses play, ignorig mute setting
      MyPlaySound(&tmp);
	mute = oldMute;
      if (tmp.data  != NULL) FreeResource(tmp.data); // technically obsolete fn, but tmp.data is NOT malloc'd mem
      if (tmp.name != NULL) free(tmp.name);
      return TRUE;

    case OPT_BrowseSound:
      f = OpenFileDialog(hDlg, "rb", NULL, "wav", SOUND_FILT,
	_("Browse for Sound File"), NULL, NULL, buf);
      if (f != NULL) {
	fclose(f);
	SetDlgItemText(hDlg, OPT_WavFileName, buf);
      }
      return TRUE;

    default:
      radio = SoundDialogWhichRadio(hDlg);
      SoundDialogSetEnables(hDlg, radio);
      break;
    }
    break;
  }
  return FALSE;
}


VOID SoundOptionsPopup(HWND hwnd)
{
  FARPROC lpProc;

  lpProc = MakeProcInstance((FARPROC)SoundOptionsDialog, hInst);
  DialogBox(hInst, MAKEINTRESOURCE(DLG_Sound), hwnd, (DLGPROC)lpProc);
  FreeProcInstance(lpProc);
}


/*---------------------------------------------------------------------------*\
 *
 * Comm Port dialog functions
 *
\*---------------------------------------------------------------------------*/


#define FLOW_NONE   0
#define FLOW_XOFF   1
#define FLOW_CTS    2
#define FLOW_DSR    3

#define PORT_NONE

ComboData cdPort[]     = { {"None", PORT_NONE}, {"COM1", 1}, {"COM2", 2},
			   {"COM3", 3}, {"COM4", 4}, {NULL, 0} };
ComboData cdDataRate[] = { {"110", 110}, {"300", 300}, {"600", 600}, {"1200", 1200},
			   {"2400", 2400}, {"4800", 4800}, {"9600", 9600}, {"19200", 19200},
			   {"38400", 38400}, {NULL, 0} };
ComboData cdDataBits[] = { {"5", 5}, {"6", 6}, {"7", 7}, {"8", 8}, {NULL, 0} };
ComboData cdParity[]   = { {"None", NOPARITY}, {"Odd", ODDPARITY}, {"Even", EVENPARITY},
			   {"Mark", MARKPARITY}, {"Space", SPACEPARITY}, {NULL, 0} };
ComboData cdStopBits[] = { {"1", ONESTOPBIT}, {"1.5", ONE5STOPBITS},
			   {"2", TWOSTOPBITS}, {NULL, 0} };
ComboData cdFlow[]     = { {"None", FLOW_NONE}, {"Xoff/Xon", FLOW_XOFF}, {"CTS", FLOW_CTS},
			   {"DSR", FLOW_DSR}, {NULL, 0} };


VOID
ParseCommSettings(char *arg, DCB *dcb)
{
  int dataRate, count;
  char bits[MSG_SIZ], parity[MSG_SIZ], stopBits[MSG_SIZ], flow[MSG_SIZ];
  ComboData *cd;
  count = sscanf(arg, "%d%*[, ]%[^, ]%*[, ]%[^, ]%*[, ]%[^, ]%*[, ]%[^, ]",
    &dataRate, bits, parity, stopBits, flow);
  if (count != 5) goto cant_parse;
  dcb->BaudRate = dataRate;
  cd = cdDataBits;
  while (cd->label != NULL) {
    if (StrCaseCmp(cd->label, bits) == 0) {
      dcb->ByteSize = cd->value;
      break;
    }
    cd++;
  }
  if (cd->label == NULL) goto cant_parse;
  cd = cdParity;
  while (cd->label != NULL) {
    if (StrCaseCmp(cd->label, parity) == 0) {
      dcb->Parity = cd->value;
      break;
    }
    cd++;
  }
  if (cd->label == NULL) goto cant_parse;
  cd = cdStopBits;
  while (cd->label != NULL) {
    if (StrCaseCmp(cd->label, stopBits) == 0) {
      dcb->StopBits = cd->value;
      break;
    }
    cd++;
  }
  cd = cdFlow;
  if (cd->label == NULL) goto cant_parse;
  while (cd->label != NULL) {
    if (StrCaseCmp(cd->label, flow) == 0) {
      switch (cd->value) {
      case FLOW_NONE:
  	dcb->fOutX = FALSE;
	dcb->fOutxCtsFlow = FALSE;
	dcb->fOutxDsrFlow = FALSE;
	break;
      case FLOW_CTS:
	dcb->fOutX = FALSE;
	dcb->fOutxCtsFlow = TRUE;
	dcb->fOutxDsrFlow = FALSE;
	break;
      case FLOW_DSR:
	dcb->fOutX = FALSE;
	dcb->fOutxCtsFlow = FALSE;
	dcb->fOutxDsrFlow = TRUE;
	break;
      case FLOW_XOFF:
	dcb->fOutX = TRUE;
	dcb->fOutxCtsFlow = FALSE;
	dcb->fOutxDsrFlow = FALSE;
	break;
      }
      break;
    }
    cd++;
  }
  if (cd->label == NULL) goto cant_parse;
  return;
cant_parse:
    ExitArgError(_("Can't parse com port settings"), arg, TRUE);
}


VOID PrintCommSettings(FILE *f, char *name, DCB *dcb)
{
  char *flow = "??", *parity = "??", *stopBits = "??";
  ComboData *cd;

  cd = cdParity;
  while (cd->label != NULL) {
    if (dcb->Parity == cd->value) {
      parity = cd->label;
      break;
    }
    cd++;
  }
  cd = cdStopBits;
  while (cd->label != NULL) {
    if (dcb->StopBits == cd->value) {
      stopBits = cd->label;
      break;
    }
    cd++;
  }
  if (dcb->fOutX) {
    flow = cdFlow[FLOW_XOFF].label;
  } else if (dcb->fOutxCtsFlow) {
    flow = cdFlow[FLOW_CTS].label;
  } else if (dcb->fOutxDsrFlow) {
    flow = cdFlow[FLOW_DSR].label;
  } else {
    flow = cdFlow[FLOW_NONE].label;
  }
  fprintf(f, "/%s=%d,%d,%s,%s,%s\n", name,
    (int)dcb->BaudRate, dcb->ByteSize, parity, stopBits, flow);
}


void
InitCombo(HANDLE hwndCombo, ComboData *cd)
{
  SendMessage(hwndCombo, CB_RESETCONTENT, 0, 0);

  while (cd->label != NULL) {
    SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM) cd->label);
    cd++;
  }
}

void
SelectComboValue(HANDLE hwndCombo, ComboData *cd, unsigned value)
{
  int i;

  i = 0;
  while (cd->label != NULL) {
    if (cd->value == value) {
      SendMessage(hwndCombo, CB_SETCURSEL, (WPARAM) i, (LPARAM) 0);
      return;
    }
    cd++;
    i++;
  }
}

LRESULT CALLBACK
CommPortOptionsDialog(HWND hDlg, UINT message, WPARAM wParam,	LPARAM lParam)
{
  char buf[MSG_SIZ];
  HANDLE hwndCombo;
  char *p;
  LRESULT index;
  unsigned value;
  int err;

  switch (message) {
  case WM_INITDIALOG: /* message: initialize dialog box */
    /* Center the dialog over the application window */
    CenterWindow (hDlg, GetWindow(hDlg, GW_OWNER));
    Translate(hDlg, DLG_CommPort);
    /* Initialize the dialog items */
    /* !! There should probably be some synchronization
       in accessing hCommPort and dcb.  Or does modal nature
       of this dialog box do it for us?
       */
    hwndCombo = GetDlgItem(hDlg, OPT_Port);
    InitCombo(hwndCombo, cdPort);
    p = strrchr(appData.icsCommPort, '\\');
    if (p++ == NULL) p = appData.icsCommPort;
    if ((*p == '\0') ||
	(SendMessage(hwndCombo, CB_SELECTSTRING, (WPARAM) -1, (LPARAM) p) == CB_ERR)) {
      SendMessage(hwndCombo, CB_SELECTSTRING, (WPARAM) -1, (LPARAM) "None");
    }
    EnableWindow(hwndCombo, hCommPort == NULL); /*!! don't allow change for now*/

    hwndCombo = GetDlgItem(hDlg, OPT_DataRate);
    InitCombo(hwndCombo, cdDataRate);
    snprintf(buf, MSG_SIZ, "%u", (int)dcb.BaudRate);
    if (SendMessage(hwndCombo, CB_SELECTSTRING, (WPARAM) -1, (LPARAM) buf) == CB_ERR) {
      SendMessage(hwndCombo, CB_SETCURSEL, (WPARAM) -1, (LPARAM) 0);
      SendMessage(hwndCombo, WM_SETTEXT, (WPARAM) 0, (LPARAM) buf);
    }

    hwndCombo = GetDlgItem(hDlg, OPT_Bits);
    InitCombo(hwndCombo, cdDataBits);
    SelectComboValue(hwndCombo, cdDataBits, dcb.ByteSize);

    hwndCombo = GetDlgItem(hDlg, OPT_Parity);
    InitCombo(hwndCombo, cdParity);
    SelectComboValue(hwndCombo, cdParity, dcb.Parity);

    hwndCombo = GetDlgItem(hDlg, OPT_StopBits);
    InitCombo(hwndCombo, cdStopBits);
    SelectComboValue(hwndCombo, cdStopBits, dcb.StopBits);

    hwndCombo = GetDlgItem(hDlg, OPT_Flow);
    InitCombo(hwndCombo, cdFlow);
    if (dcb.fOutX) {
      SelectComboValue(hwndCombo, cdFlow, FLOW_XOFF);
    } else if (dcb.fOutxCtsFlow) {
      SelectComboValue(hwndCombo, cdFlow, FLOW_CTS);
    } else if (dcb.fOutxDsrFlow) {
      SelectComboValue(hwndCombo, cdFlow, FLOW_DSR);
    } else {
      SelectComboValue(hwndCombo, cdFlow, FLOW_NONE);
    }
    return TRUE;

  case WM_COMMAND: /* message: received a command */
    switch (LOWORD(wParam)) {
    case IDOK:
      /* Read changed options from the dialog box */
#ifdef NOTDEF
      /* !! Currently we can't change comm ports in midstream */
      hwndCombo = GetDlgItem(hDlg, OPT_Port);
      index = SendMessage(hwndCombo, CB_GETCURSEL, (WPARAM) 0, (LPARAM) 0);
      if (index == PORT_NONE) {
	appData.icsCommPort = "";
	if (hCommPort != NULL) {
	  CloseHandle(hCommPort);
	  hCommPort = NULL;
	}
	EndDialog(hDlg, TRUE);
	return TRUE;
      }
      SendMessage(hwndCombo, WM_GETTEXT, (WPARAM) MSG_SIZ, (LPARAM) buf);
      appData.icsCommPort = strdup(buf);
      if (hCommPort != NULL) {
	CloseHandle(hCommPort);
	hCommPort = NULL;
      }
      /* now what?? can't really do this; have to fix up the ChildProc
	 and InputSource records for the comm port that we gave to the
	 back end. */
#endif /*NOTDEF*/

      hwndCombo = GetDlgItem(hDlg, OPT_DataRate);
      SendMessage(hwndCombo, WM_GETTEXT, (WPARAM) MSG_SIZ, (LPARAM) buf);
      if (sscanf(buf, "%u", &value) != 1) {
	MessageBox(hDlg, _("Invalid data rate"),
		   _("Option Error"), MB_OK|MB_ICONEXCLAMATION);
	return TRUE;
      }
      dcb.BaudRate = value;

      hwndCombo = GetDlgItem(hDlg, OPT_Bits);
      index = SendMessage(hwndCombo, CB_GETCURSEL, (WPARAM) 0, (LPARAM) 0);
      dcb.ByteSize = cdDataBits[index].value;

      hwndCombo = GetDlgItem(hDlg, OPT_Parity);
      index = SendMessage(hwndCombo, CB_GETCURSEL, (WPARAM) 0, (LPARAM) 0);
      dcb.Parity = cdParity[index].value;

      hwndCombo = GetDlgItem(hDlg, OPT_StopBits);
      index = SendMessage(hwndCombo, CB_GETCURSEL, (WPARAM) 0, (LPARAM) 0);
      dcb.StopBits = cdStopBits[index].value;

      hwndCombo = GetDlgItem(hDlg, OPT_Flow);
      index = SendMessage(hwndCombo, CB_GETCURSEL, (WPARAM) 0, (LPARAM) 0);
      switch (cdFlow[index].value) {
      case FLOW_NONE:
	dcb.fOutX = FALSE;
	dcb.fOutxCtsFlow = FALSE;
	dcb.fOutxDsrFlow = FALSE;
	break;
      case FLOW_CTS:
	dcb.fOutX = FALSE;
	dcb.fOutxCtsFlow = TRUE;
	dcb.fOutxDsrFlow = FALSE;
	break;
      case FLOW_DSR:
	dcb.fOutX = FALSE;
	dcb.fOutxCtsFlow = FALSE;
	dcb.fOutxDsrFlow = TRUE;
	break;
      case FLOW_XOFF:
	dcb.fOutX = TRUE;
	dcb.fOutxCtsFlow = FALSE;
	dcb.fOutxDsrFlow = FALSE;
	break;
      }
      if (!SetCommState(hCommPort, (LPDCB) &dcb)) {
	err = GetLastError();
	switch(MessageBox(hDlg,
	                 "Failed to set comm port state;\r\ninvalid options?",
			 _("Option Error"), MB_ABORTRETRYIGNORE|MB_ICONQUESTION)) {
	case IDABORT:
	  DisplayFatalError(_("Failed to set comm port state"), err, 1);
	  exit(1);  /*is it ok to do this from here?*/

	case IDRETRY:
	  return TRUE;

	case IDIGNORE:
	  EndDialog(hDlg, TRUE);
	  return TRUE;
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
CommPortOptionsPopup(HWND hwnd)
{
  FARPROC lpProc = MakeProcInstance((FARPROC)CommPortOptionsDialog, hInst);
  DialogBox(hInst, MAKEINTRESOURCE(DLG_CommPort), hwnd, (DLGPROC) lpProc);
  FreeProcInstance(lpProc);
}

/*---------------------------------------------------------------------------*\
 *
 * Load Options dialog functions
 *
\*---------------------------------------------------------------------------*/

int
LoadOptionsWhichRadio(HWND hDlg)
{
  return (IsDlgButtonChecked(hDlg, OPT_Exact) ? 1 :
         (IsDlgButtonChecked(hDlg, OPT_Subset) ? 2 :
         (IsDlgButtonChecked(hDlg, OPT_Struct) ? 3 :
         (IsDlgButtonChecked(hDlg, OPT_Material) ? 4 :
         (IsDlgButtonChecked(hDlg, OPT_Range) ? 5 :
         (IsDlgButtonChecked(hDlg, OPT_Difference) ? 6 : -1))))));
}

VOID
SetLoadOptionEnables(HWND hDlg)
{
  UINT state;

  state = IsDlgButtonChecked(hDlg, OPT_Autostep);
  EnableWindow(GetDlgItem(hDlg, OPT_ASTimeDelay), state);
  EnableWindow(GetDlgItem(hDlg, OPT_AStext1), state);
}

LRESULT CALLBACK
LoadOptions(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  char buf[MSG_SIZ];
  float fnumber;
  int ok;

  switch (message) {
  case WM_INITDIALOG: /* message: initialize dialog box */
    /* Center the dialog over the application window */
    CenterWindow (hDlg, GetWindow (hDlg, GW_OWNER));
    Translate(hDlg, DLG_LoadOptions);
    /* Initialize the dialog items */
    if (appData.timeDelay >= 0.0) {
      CheckDlgButton(hDlg, OPT_Autostep, TRUE);
      snprintf(buf, MSG_SIZ, "%.2g", appData.timeDelay);
      SetDlgItemText(hDlg, OPT_ASTimeDelay, buf);
    } else {
      CheckDlgButton(hDlg, OPT_Autostep, FALSE);
    }
    SetLoadOptionEnables(hDlg);
    SetDlgItemInt(hDlg, OPT_elo1, appData.eloThreshold1, FALSE);
    SetDlgItemInt(hDlg, OPT_elo2, appData.eloThreshold2, FALSE);
    SetDlgItemInt(hDlg, OPT_date, appData.dateThreshold, FALSE);
    SetDlgItemInt(hDlg, OPT_Stretch, appData.stretch, FALSE);
    CheckDlgButton(hDlg, OPT_Reversed, appData.ignoreColors);
    CheckDlgButton(hDlg, OPT_Mirror, appData.findMirror);
    switch (appData.searchMode) {
    case 1:
      CheckDlgButton(hDlg, OPT_Exact, TRUE);
      break;
    case 2:
      CheckDlgButton(hDlg, OPT_Subset, TRUE);
      break;
    case 3:
      CheckDlgButton(hDlg, OPT_Struct, TRUE);
      break;
    case 4:
      CheckDlgButton(hDlg, OPT_Material, TRUE);
      break;
    case 5:
      CheckDlgButton(hDlg, OPT_Range, TRUE);
      break;
    case 6:
      CheckDlgButton(hDlg, OPT_Difference, TRUE);
      break;
    }
    return TRUE;

  case WM_COMMAND: /* message: received a command */
    switch (LOWORD(wParam)) {
    case IDOK:
      /* Read changed options from the dialog box */
      if (IsDlgButtonChecked(hDlg, OPT_Autostep)) {
	GetDlgItemText(hDlg, OPT_ASTimeDelay, buf, MSG_SIZ);
	if (sscanf(buf, "%f", &fnumber) != 1) {
	  MessageBox(hDlg, _("Invalid load game step rate"),
		     _("Option Error"), MB_OK|MB_ICONEXCLAMATION);
	  return FALSE;
	}
	appData.timeDelay = fnumber;
      } else {
	appData.timeDelay = (float) -1.0;
      }
      appData.eloThreshold1 = GetDlgItemInt(hDlg, OPT_elo1, &ok, FALSE);
      appData.eloThreshold2 = GetDlgItemInt(hDlg, OPT_elo2, &ok, FALSE);
      appData.dateThreshold = GetDlgItemInt(hDlg, OPT_date, &ok, FALSE);
      appData.stretch = GetDlgItemInt(hDlg, OPT_Stretch, &ok, FALSE);
      appData.searchMode = LoadOptionsWhichRadio(hDlg);
      appData.ignoreColors = IsDlgButtonChecked(hDlg, OPT_Reversed);
      appData.findMirror   = IsDlgButtonChecked(hDlg, OPT_Mirror);
      EndDialog(hDlg, TRUE);
      return TRUE;

    case IDCANCEL:
      EndDialog(hDlg, FALSE);
      return TRUE;

    default:
      SetLoadOptionEnables(hDlg);
      break;
    }
    break;
  }
  return FALSE;
}


VOID
LoadOptionsPopup(HWND hwnd)
{
  FARPROC lpProc = MakeProcInstance((FARPROC)LoadOptions, hInst);
  DialogBox(hInst, MAKEINTRESOURCE(DLG_LoadOptions), hwnd, (DLGPROC) lpProc);
  FreeProcInstance(lpProc);
}

/*---------------------------------------------------------------------------*\
 *
 * Save Options dialog functions
 *
\*---------------------------------------------------------------------------*/

VOID
SetSaveOptionEnables(HWND hDlg)
{
  UINT state;

  state = IsDlgButtonChecked(hDlg, OPT_Autosave);
  EnableWindow(GetDlgItem(hDlg, OPT_AVPrompt), state);
  EnableWindow(GetDlgItem(hDlg, OPT_AVToFile), state);
  if (state && !IsDlgButtonChecked(hDlg, OPT_AVPrompt) &&
      !IsDlgButtonChecked(hDlg, OPT_AVToFile)) {
    CheckRadioButton(hDlg, OPT_AVPrompt, OPT_AVToFile, OPT_AVPrompt);
  }

  state = state && IsDlgButtonChecked(hDlg, OPT_AVToFile);
  EnableWindow(GetDlgItem(hDlg, OPT_AVFilename), state);
  EnableWindow(GetDlgItem(hDlg, OPT_AVBrowse), state);
}

LRESULT CALLBACK
SaveOptionsDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  char buf[MSG_SIZ];
  FILE *f;

  switch (message) {
  case WM_INITDIALOG: /* message: initialize dialog box */
    /* Center the dialog over the application window */
    CenterWindow (hDlg, GetWindow (hDlg, GW_OWNER));
    Translate(hDlg, DLG_SaveOptions);
    /* Initialize the dialog items */
    if (*appData.saveGameFile != NULLCHAR) {
      CheckDlgButton(hDlg, OPT_Autosave, (UINT) TRUE);
      CheckRadioButton(hDlg, OPT_AVPrompt, OPT_AVToFile, OPT_AVToFile);
      SetDlgItemText(hDlg, OPT_AVFilename, appData.saveGameFile);
    } else if (appData.autoSaveGames) {
      CheckDlgButton(hDlg, OPT_Autosave, (UINT) TRUE);
      CheckRadioButton(hDlg, OPT_AVPrompt, OPT_AVToFile, OPT_AVPrompt);
    } else {
      CheckDlgButton(hDlg, OPT_Autosave, (UINT) FALSE);
    }
    if (appData.oldSaveStyle) {
      CheckRadioButton(hDlg, OPT_PGN, OPT_Old, OPT_Old);
    } else {
      CheckRadioButton(hDlg, OPT_PGN, OPT_Old, OPT_PGN);
    }
    CheckDlgButton( hDlg, OPT_OutOfBookInfo, appData.saveOutOfBookInfo );
    SetSaveOptionEnables(hDlg);
    return TRUE;

  case WM_COMMAND: /* message: received a command */
    switch (LOWORD(wParam)) {
    case IDOK:
      /* Read changed options from the dialog box */
      if (IsDlgButtonChecked(hDlg, OPT_Autosave)) {
	appData.autoSaveGames = TRUE;
	if (IsDlgButtonChecked(hDlg, OPT_AVPrompt)) {
	  ASSIGN(appData.saveGameFile, ""); // [HGM] make sure value is ALWAYS in allocated memory
	} else /*if (IsDlgButtonChecked(hDlg, OPT_AVToFile))*/ {
	  GetDlgItemText(hDlg, OPT_AVFilename, buf, MSG_SIZ);
	  if (*buf == NULLCHAR) {
	    MessageBox(hDlg, _("Invalid save game file name"),
		       _("Option Error"), MB_OK|MB_ICONEXCLAMATION);
	    return FALSE;
	  }
	  FREE(appData.saveGameFile);
	  appData.saveGameFile = InterpretFileName(buf, homeDir);
	}
      } else {
	appData.autoSaveGames = FALSE;
	ASSIGN(appData.saveGameFile, "");
      }
      appData.oldSaveStyle = IsDlgButtonChecked(hDlg, OPT_Old);
      appData.saveOutOfBookInfo = IsDlgButtonChecked( hDlg, OPT_OutOfBookInfo );
      EndDialog(hDlg, TRUE);
      return TRUE;

    case IDCANCEL:
      EndDialog(hDlg, FALSE);
      return TRUE;

    case OPT_AVBrowse:
      f = OpenFileDialog(hDlg, "a", NULL,
	                 appData.oldSaveStyle ? "gam" : "pgn",
   	                 GAME_FILT, _("Browse for Auto Save File"),
			 NULL, NULL, buf);
      if (f != NULL) {
	fclose(f);
	SetDlgItemText(hDlg, OPT_AVFilename, buf);
      }
      break;

    default:
      SetSaveOptionEnables(hDlg);
      break;
    }
    break;
  }
  return FALSE;
}

VOID
SaveOptionsPopup(HWND hwnd)
{
  FARPROC lpProc = MakeProcInstance((FARPROC)SaveOptionsDialog, hInst);
  DialogBox(hInst, MAKEINTRESOURCE(DLG_SaveOptions), hwnd, (DLGPROC) lpProc);
  FreeProcInstance(lpProc);
}

/*---------------------------------------------------------------------------*\
 *
 * Time Control Options dialog functions
 *
\*---------------------------------------------------------------------------*/

VOID
SetTimeControlEnables(HWND hDlg)
{
  UINT state;

  state = IsDlgButtonChecked(hDlg, OPT_TCUseMoves)
	+ 2*IsDlgButtonChecked(hDlg, OPT_TCUseFixed);
  EnableWindow(GetDlgItem(hDlg, OPT_TCTime), state == 1);
  EnableWindow(GetDlgItem(hDlg, OPT_TCMoves), state == 1);
  EnableWindow(GetDlgItem(hDlg, OPT_TCtext1), state == 1);
  EnableWindow(GetDlgItem(hDlg, OPT_TCtext2), state == 1);
  EnableWindow(GetDlgItem(hDlg, OPT_TCTime2), !state);
  EnableWindow(GetDlgItem(hDlg, OPT_TCInc), !state);
  EnableWindow(GetDlgItem(hDlg, OPT_TCitext1), !state);
  EnableWindow(GetDlgItem(hDlg, OPT_TCitext2), !state);
  EnableWindow(GetDlgItem(hDlg, OPT_TCitext3), !state);
  EnableWindow(GetDlgItem(hDlg, OPT_TCFixed), state == 2);
  EnableWindow(GetDlgItem(hDlg, OPT_TCftext), state == 2);
}


LRESULT CALLBACK
TimeControl(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  char buf[MSG_SIZ], *tc;
  int mps, odds1, odds2, st;
  float increment;
  BOOL ok, ok2;

  switch (message) {
  case WM_INITDIALOG: /* message: initialize dialog box */
    /* Center the dialog over the application window */
    CenterWindow (hDlg, GetWindow (hDlg, GW_OWNER));
    Translate(hDlg, DLG_TimeControl);
    /* Initialize the dialog items */
    if (/*appData.clockMode &&*/ !appData.icsActive) { // [HGM] even if !clockMode, we could want to set it in tournament dialog
      if (searchTime) {
	CheckRadioButton(hDlg, OPT_TCUseMoves, OPT_TCUseFixed,
			 OPT_TCUseFixed);
	SetDlgItemInt(hDlg, OPT_TCFixed, searchTime, FALSE);
      } else
      if (appData.timeIncrement == -1) {
	CheckRadioButton(hDlg, OPT_TCUseMoves, OPT_TCUseFixed,
			 OPT_TCUseMoves);
	SetDlgItemText(hDlg, OPT_TCTime, appData.timeControl);
	SetDlgItemInt(hDlg, OPT_TCMoves, appData.movesPerSession,
		      FALSE);
	SetDlgItemText(hDlg, OPT_TCTime2, "");
	SetDlgItemText(hDlg, OPT_TCInc, "");
      } else {
	int i = appData.timeIncrement;
	if(i == appData.timeIncrement) snprintf(buf, MSG_SIZ, "%d", i);
	else snprintf(buf, MSG_SIZ, "%4.2f", appData.timeIncrement);
	CheckRadioButton(hDlg, OPT_TCUseMoves, OPT_TCUseFixed,
			 OPT_TCUseInc);
	SetDlgItemText(hDlg, OPT_TCTime, "");
	SetDlgItemText(hDlg, OPT_TCMoves, "");
	SetDlgItemText(hDlg, OPT_TCTime2, appData.timeControl);
	SetDlgItemText(hDlg, OPT_TCInc, buf);
      }
      SetDlgItemInt(hDlg, OPT_TCOdds1, 1, FALSE);
      SetDlgItemInt(hDlg, OPT_TCOdds2, 1, FALSE);
      SetTimeControlEnables(hDlg);
    }
    return TRUE;

  case WM_COMMAND: /* message: received a command */
    switch (LOWORD(wParam)) {
    case IDOK:
      mps = appData.movesPerSession;
      increment = appData.timeIncrement;
      tc = appData.timeControl;
      st = 0;
      /* Read changed options from the dialog box */
      if (IsDlgButtonChecked(hDlg, OPT_TCUseFixed)) {
	st = GetDlgItemInt(hDlg, OPT_TCFixed, &ok, FALSE);
	if (!ok || st <= 0) {
	  MessageBox(hDlg, _("Invalid max time per move"),
		     _("Option Error"), MB_OK|MB_ICONEXCLAMATION);
	  return FALSE;
	}
      } else
      if (IsDlgButtonChecked(hDlg, OPT_TCUseMoves)) {
	increment = -1;
	mps = GetDlgItemInt(hDlg, OPT_TCMoves, &ok, FALSE);
	if (!ok || mps <= 0) {
	  MessageBox(hDlg, _("Invalid moves per time control"),
		     _("Option Error"), MB_OK|MB_ICONEXCLAMATION);
	  return FALSE;
	}
	GetDlgItemText(hDlg, OPT_TCTime, buf, MSG_SIZ);
	if (!ParseTimeControl(buf, increment, mps)) {
	  MessageBox(hDlg, _("Invalid minutes per time control"),
		     _("Option Error"), MB_OK|MB_ICONEXCLAMATION);
	  return FALSE;
	}
      tc = buf;
      } else {
	GetDlgItemText(hDlg, OPT_TCInc, buf, MSG_SIZ);
	ok = (sscanf(buf, "%f%c", &increment, buf) == 1);
	if (!ok || increment < 0) {
	  MessageBox(hDlg, _("Invalid increment"),
		     _("Option Error"), MB_OK|MB_ICONEXCLAMATION);
	  return FALSE;
	}
	GetDlgItemText(hDlg, OPT_TCTime2, buf, MSG_SIZ);
	if (!ParseTimeControl(buf, increment, mps)) {
	  MessageBox(hDlg, _("Invalid initial time"),
		     _("Option Error"), MB_OK|MB_ICONEXCLAMATION);
	  return FALSE;
	}
      tc = buf;
      }
      odds1 = GetDlgItemInt(hDlg, OPT_TCOdds1, &ok, FALSE);
      odds2 = GetDlgItemInt(hDlg, OPT_TCOdds2, &ok2, FALSE);
      if (!ok || !ok2 || odds1 <= 0 || odds2 <= 0) {
	  MessageBox(hDlg, _("Invalid time-odds factor"),
		     _("Option Error"), MB_OK|MB_ICONEXCLAMATION);
	  return FALSE;
      }
      searchTime = st;
      appData.timeControl = strdup(tc);
      appData.movesPerSession = mps;
      appData.timeIncrement = increment;
      appData.firstTimeOdds  = first.timeOdds  = odds1;
      appData.secondTimeOdds = second.timeOdds = odds2;
      Reset(TRUE, TRUE);
      EndDialog(hDlg, TRUE);
      return TRUE;

    case IDCANCEL:
      EndDialog(hDlg, FALSE);
      return TRUE;

    default:
      SetTimeControlEnables(hDlg);
      break;
    }
    break;
  }
  return FALSE;
}

VOID
TimeControlOptionsPopup(HWND hwnd)
{
  if (gameMode != BeginningOfGame) {
    DisplayError(_("Changing time control during a game is not implemented"), 0);
  } else {
    FARPROC lpProc = MakeProcInstance((FARPROC)TimeControl, hInst);
    DialogBox(hInst, MAKEINTRESOURCE(DLG_TimeControl), hwnd, (DLGPROC) lpProc);
    FreeProcInstance(lpProc);
  }
}

/*---------------------------------------------------------------------------*\
 *
 * Engine Options Dialog functions
 *
\*---------------------------------------------------------------------------*/
#define CHECK_BOX(x,y) CheckDlgButton(hDlg, (x), (BOOL)(y))
#define IS_CHECKED(x) (Boolean)IsDlgButtonChecked(hDlg, (x))

#define INT_ABS( n )    ((n) >= 0 ? (n) : -(n))

LRESULT CALLBACK EnginePlayOptionsDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message) {
  case WM_INITDIALOG: /* message: initialize dialog box */

    /* Center the dialog over the application window */
    CenterWindow (hDlg, GetWindow (hDlg, GW_OWNER));
    Translate(hDlg, DLG_EnginePlayOptions);

    /* Initialize the dialog items */
    CHECK_BOX(IDC_EpPeriodicUpdates, appData.periodicUpdates);
    CHECK_BOX(IDC_EpPonder, appData.ponderNextMove);
    CHECK_BOX(IDC_EpShowThinking, appData.showThinking);
    CHECK_BOX(IDC_EpHideThinkingHuman, appData.hideThinkingFromHuman);

    CHECK_BOX(IDC_TestClaims, appData.testClaims);
    CHECK_BOX(IDC_DetectMates, appData.checkMates);
    CHECK_BOX(IDC_MaterialDraws, appData.materialDraws);
    CHECK_BOX(IDC_TrivialDraws, appData.trivialDraws);

    CHECK_BOX(IDC_ScoreAbs1, appData.firstScoreIsAbsolute);
    CHECK_BOX(IDC_ScoreAbs2, appData.secondScoreIsAbsolute);

    SetDlgItemInt( hDlg, IDC_EpDrawMoveCount, appData.adjudicateDrawMoves, TRUE );
    SendDlgItemMessage( hDlg, IDC_EpDrawMoveCount, EM_SETSEL, 0, -1 );

    SetDlgItemInt( hDlg, IDC_EpAdjudicationThreshold, INT_ABS(appData.adjudicateLossThreshold), TRUE );
    SendDlgItemMessage( hDlg, IDC_EpAdjudicationThreshold, EM_SETSEL, 0, -1 );

    SetDlgItemInt( hDlg, IDC_RuleMoves, appData.ruleMoves, TRUE );
    SendDlgItemMessage( hDlg, IDC_RuleMoves, EM_SETSEL, 0, -1 );

    SetDlgItemInt( hDlg, IDC_DrawRepeats, INT_ABS(appData.drawRepeats), TRUE );
    SendDlgItemMessage( hDlg, IDC_DrawRepeats, EM_SETSEL, 0, -1 );

    return TRUE;

  case WM_COMMAND: /* message: received a command */
    switch (LOWORD(wParam)) {
    case IDOK:
      /* Read changed options from the dialog box */
      PeriodicUpdatesEvent(          IS_CHECKED(IDC_EpPeriodicUpdates));
      PonderNextMoveEvent(           IS_CHECKED(IDC_EpPonder));
      appData.hideThinkingFromHuman= IS_CHECKED(IDC_EpHideThinkingHuman); // [HGM] thinking: moved up
      appData.showThinking   = IS_CHECKED(IDC_EpShowThinking);
      ShowThinkingEvent(); // [HGM] thinking: tests all options that need thinking output
      appData.testClaims    = IS_CHECKED(IDC_TestClaims);
      appData.checkMates    = IS_CHECKED(IDC_DetectMates);
      appData.materialDraws = IS_CHECKED(IDC_MaterialDraws);
      appData.trivialDraws  = IS_CHECKED(IDC_TrivialDraws);

      appData.adjudicateDrawMoves = GetDlgItemInt(hDlg, IDC_EpDrawMoveCount, NULL, FALSE );
      appData.adjudicateLossThreshold = - (int) GetDlgItemInt(hDlg, IDC_EpAdjudicationThreshold, NULL, FALSE );
      appData.ruleMoves = GetDlgItemInt(hDlg, IDC_RuleMoves, NULL, FALSE );
      appData.drawRepeats = (int) GetDlgItemInt(hDlg, IDC_DrawRepeats, NULL, FALSE );

      first.scoreIsAbsolute  = appData.firstScoreIsAbsolute  = IS_CHECKED(IDC_ScoreAbs1);
      second.scoreIsAbsolute = appData.secondScoreIsAbsolute = IS_CHECKED(IDC_ScoreAbs2);

      EndDialog(hDlg, TRUE);
      return TRUE;

    case IDCANCEL:
      EndDialog(hDlg, FALSE);
      return TRUE;

    case IDC_EpDrawMoveCount:
    case IDC_EpAdjudicationThreshold:
    case IDC_DrawRepeats:
    case IDC_RuleMoves:
        if( HIWORD(wParam) == EN_CHANGE ) {
            int n1_ok;
            int n2_ok;
            int n3_ok;
            int n4_ok;

            GetDlgItemInt(hDlg, IDC_EpDrawMoveCount, &n1_ok, FALSE );
            GetDlgItemInt(hDlg, IDC_EpAdjudicationThreshold, &n2_ok, FALSE );
            GetDlgItemInt(hDlg, IDC_RuleMoves, &n3_ok, FALSE );
            GetDlgItemInt(hDlg, IDC_DrawRepeats, &n4_ok, FALSE );

            EnableWindow( GetDlgItem(hDlg, IDOK), n1_ok && n2_ok && n3_ok && n4_ok ? TRUE : FALSE );
        }
        return TRUE;
    }
    break;
  }
  return FALSE;
}

VOID EnginePlayOptionsPopup(HWND hwnd)
{
  FARPROC lpProc;

  lpProc = MakeProcInstance((FARPROC)EnginePlayOptionsDialog, hInst);
  DialogBox(hInst, MAKEINTRESOURCE(DLG_EnginePlayOptions), hwnd, (DLGPROC) lpProc);
  FreeProcInstance(lpProc);
}

/*---------------------------------------------------------------------------*\
 *
 * UCI Options Dialog functions
 *
\*---------------------------------------------------------------------------*/
INT CALLBACK BrowseCallbackProc(HWND hwnd, 
                                UINT uMsg,
                                LPARAM lp, 
                                LPARAM pData) 
{
    switch(uMsg) 
    {
      case BFFM_INITIALIZED: 
        SendMessage(hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)pData);
        break;
    }
    return 0;
}

BOOL BrowseForFolder( const char * title, char * path )
{
    BOOL result = FALSE;
    BROWSEINFO bi;
    LPITEMIDLIST pidl;

    ZeroMemory( &bi, sizeof(bi) );

    bi.lpszTitle = title == 0 ? _("Choose Folder") : title;
    bi.ulFlags = BIF_RETURNONLYFSDIRS;
    bi.lpfn = BrowseCallbackProc;
    bi.lParam = (LPARAM) path;

    pidl = SHBrowseForFolder( &bi );

    if( pidl != 0 ) {
        IMalloc * imalloc = 0;

        if( SHGetPathFromIDList( pidl, path ) ) {
            result = TRUE;
        }

        if( SUCCEEDED( SHGetMalloc ( &imalloc ) ) ) {
            imalloc->lpVtbl->Free(imalloc,pidl);
            imalloc->lpVtbl->Release(imalloc);
        }
    }

    return result;
}

LRESULT CALLBACK UciOptionsDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  char buf[MAX_PATH];
  int oldCores;

  switch (message) {
  case WM_INITDIALOG: /* message: initialize dialog box */

    /* Center the dialog over the application window */
    CenterWindow (hDlg, GetWindow (hDlg, GW_OWNER));
    Translate(hDlg, DLG_OptionsUCI);

    /* Initialize the dialog items */
    SetDlgItemText( hDlg, IDC_PolyglotDir, appData.polyglotDir );
    SetDlgItemInt( hDlg, IDC_HashSize, appData.defaultHashSize, TRUE );
    SetDlgItemText( hDlg, IDC_PathToEGTB, appData.defaultPathEGTB );
    SetDlgItemInt( hDlg, IDC_SizeOfEGTB, appData.defaultCacheSizeEGTB, TRUE );
    CheckDlgButton( hDlg, IDC_UseBook, (BOOL) appData.usePolyglotBook );
    SetDlgItemText( hDlg, IDC_BookFile, appData.polyglotBook );
    // [HGM] smp: input field for nr of cores:
    SetDlgItemInt( hDlg, IDC_Cores, appData.smpCores, TRUE );
    // [HGM] book: tick boxes for own book use
    CheckDlgButton( hDlg, IDC_OwnBook1, (BOOL) appData.firstHasOwnBookUCI );
    CheckDlgButton( hDlg, IDC_OwnBook2, (BOOL) appData.secondHasOwnBookUCI );
    SetDlgItemInt( hDlg, IDC_BookDep, appData.bookDepth, TRUE );
    SetDlgItemInt( hDlg, IDC_BookStr, appData.bookStrength, TRUE );
    SetDlgItemInt( hDlg, IDC_Games, appData.defaultMatchGames, TRUE );

    SendDlgItemMessage( hDlg, IDC_PolyglotDir, EM_SETSEL, 0, -1 );

    return TRUE;

  case WM_COMMAND: /* message: received a command */
    switch (LOWORD(wParam)) {
    case IDOK:
      GetDlgItemText( hDlg, IDC_PolyglotDir, buf, sizeof(buf) );
      appData.polyglotDir = strdup(buf);
      appData.defaultHashSize = GetDlgItemInt(hDlg, IDC_HashSize, NULL, FALSE );
      appData.defaultCacheSizeEGTB = GetDlgItemInt(hDlg, IDC_SizeOfEGTB, NULL, FALSE );
      GetDlgItemText( hDlg, IDC_PathToEGTB, buf, sizeof(buf) );
      appData.defaultPathEGTB = strdup(buf);
      GetDlgItemText( hDlg, IDC_BookFile, buf, sizeof(buf) );
      appData.polyglotBook = strdup(buf);
      appData.usePolyglotBook = (Boolean) IsDlgButtonChecked( hDlg, IDC_UseBook );
      // [HGM] smp: get nr of cores:
      oldCores = appData.smpCores;
      appData.smpCores = GetDlgItemInt(hDlg, IDC_Cores, NULL, FALSE );
      if(appData.smpCores != oldCores) NewSettingEvent(FALSE, &(first.maxCores), "cores", appData.smpCores);
      // [HGM] book: read tick boxes for own book use
      appData.firstHasOwnBookUCI  = (Boolean) IsDlgButtonChecked( hDlg, IDC_OwnBook1 );
      appData.secondHasOwnBookUCI = (Boolean) IsDlgButtonChecked( hDlg, IDC_OwnBook2 );
      appData.bookDepth = GetDlgItemInt(hDlg, IDC_BookDep, NULL, FALSE );
      appData.bookStrength = GetDlgItemInt(hDlg, IDC_BookStr, NULL, FALSE );
      appData.defaultMatchGames = GetDlgItemInt(hDlg, IDC_Games, NULL, FALSE );

      if(gameMode == BeginningOfGame) Reset(TRUE, TRUE);
      EndDialog(hDlg, TRUE);
      return TRUE;

    case IDCANCEL:
      EndDialog(hDlg, FALSE);
      return TRUE;

    case IDC_BrowseForBook:
      {
          char filter[] = {
              'A','l','l',' ','F','i','l','e','s', 0,
              '*','.','*', 0,
              'B','I','N',' ','F','i','l','e','s', 0,
              '*','.','b','i','n', 0,
              0 };

          OPENFILENAME ofn;

          safeStrCpy( buf, "" , sizeof( buf)/sizeof( buf[0]) );

          ZeroMemory( &ofn, sizeof(ofn) );

          ofn.lStructSize = sizeof(ofn);
          ofn.hwndOwner = hDlg;
          ofn.hInstance = hInst;
          ofn.lpstrFilter = filter;
          ofn.lpstrFile = buf;
          ofn.nMaxFile = sizeof(buf);
          ofn.lpstrTitle = _("Choose Book");
          ofn.Flags = OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_HIDEREADONLY;

          if( GetOpenFileName( &ofn ) ) {
              SetDlgItemText( hDlg, IDC_BookFile, buf );
          }
      }
      return TRUE;

    case IDC_BrowseForPolyglotDir:
      if( BrowseForFolder( _("Choose Polyglot Directory"), buf ) ) {
        SetDlgItemText( hDlg, IDC_PolyglotDir, buf );

        strcat( buf, "\\polyglot.exe" );

        if( GetFileAttributes(buf) == 0xFFFFFFFF ) {
            MessageBox( hDlg, _("Polyglot was not found in the specified folder!"), "Warning", MB_OK | MB_ICONWARNING );
        }
      }
      return TRUE;

    case IDC_BrowseForEGTB:
      if( BrowseForFolder( _("Choose EGTB Directory:"), buf ) ) {
        SetDlgItemText( hDlg, IDC_PathToEGTB, buf );
      }
      return TRUE;

    case IDC_HashSize:
    case IDC_SizeOfEGTB:
        if( HIWORD(wParam) == EN_CHANGE ) {
            int n1_ok;
            int n2_ok;

            GetDlgItemInt(hDlg, IDC_HashSize, &n1_ok, FALSE );
            GetDlgItemInt(hDlg, IDC_SizeOfEGTB, &n2_ok, FALSE );

            EnableWindow( GetDlgItem(hDlg, IDOK), n1_ok && n2_ok ? TRUE : FALSE );
        }
        return TRUE;
    }
    break;
  }
  return FALSE;
}

VOID UciOptionsPopup(HWND hwnd)
{
  FARPROC lpProc;

  lpProc = MakeProcInstance((FARPROC)UciOptionsDialog, hInst);
  DialogBox(hInst, MAKEINTRESOURCE(DLG_OptionsUCI), hwnd, (DLGPROC) lpProc);
  FreeProcInstance(lpProc);
}
