/*
 * Move history for WinBoard
 *
 * Author: Alessandro Scotti
 *
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

#include <windows.h> /* required for all Windows applications */
#include <richedit.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <commdlg.h>
#include <dlgs.h>

#include "common.h"
#include "winboard.h"
#include "frontend.h"
#include "backend.h"

#include "wsnap.h"

VOID MoveHistorySet( char movelist[][2*MOVE_LEN], int first, int last, int current, ChessProgramStats_Move * pvInfo );
VOID MoveHistoryPopUp();
VOID MoveHistoryPopDown();
BOOL MoveHistoryIsUp();

/* Imports from backend.c */
char * SavePart(char *str);

/* Imports from winboard.c */
extern HWND moveHistoryDialog;
extern BOOLEAN moveHistoryDialogUp;

extern HINSTANCE hInst;
extern HWND hwndMain;

extern WindowPlacement wpMoveHistory;

/* Module globals */
typedef char MoveHistoryString[ MOVE_LEN*2 ];

static int lastFirst = 0;
static int lastLast = 0;
static int lastCurrent = -1;

static MoveHistoryString * currMovelist;
static ChessProgramStats_Move * currPvInfo;
static int currFirst = 0;
static int currLast = 0;
static int currCurrent = -1;

typedef struct {
    int memoOffset;
    int memoLength;
} HistoryMove;

static HistoryMove histMoves[ MAX_MOVES ];

#define WM_REFRESH_HISTORY  (WM_USER+4657)

#define DEFAULT_COLOR       0xFFFFFFFF

#define H_MARGIN            2
#define V_MARGIN            2

/* Note: in the following code a "Memo" is a Rich Edit control (it's Delphi lingo) */

static VOID HighlightMove( int index, BOOL highlight )
{
    if( index >= 0 && index < MAX_MOVES ) {
        CHARFORMAT cf;
        HWND hMemo = GetDlgItem( moveHistoryDialog, IDC_MoveHistory );

        SendMessage( hMemo,
            EM_SETSEL,
            histMoves[index].memoOffset,
            histMoves[index].memoOffset + histMoves[index].memoLength );


        /* Set style */
        ZeroMemory( &cf, sizeof(cf) );

        cf.cbSize = sizeof(cf);
        cf.dwMask = CFM_BOLD | CFM_COLOR;

        if( highlight ) {
            cf.dwEffects |= CFE_BOLD;
            cf.crTextColor = RGB( 0x00, 0x00, 0xFF );
        }
        else {
            cf.dwEffects |= CFE_AUTOCOLOR;
        }

        SendMessage( hMemo, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM) &cf );
    }
}

static BOOL OnlyCurrentPositionChanged()
{
    BOOL result = FALSE;

    if( lastFirst >= 0 &&
        lastLast >= lastFirst &&
        lastCurrent >= lastFirst &&
        currFirst == lastFirst &&
        currLast == lastLast &&
        currCurrent >= 0 &&
        TRUE )
    {
        result = TRUE;
    }

    return result;
}

static BOOL OneMoveAppended()
{
    BOOL result = FALSE;

    if( lastCurrent >= 0 && lastCurrent >= lastFirst && lastLast >= lastFirst &&
        currCurrent >= 0 && currCurrent >= currFirst && currLast >= currFirst &&
        lastFirst == currFirst &&
        lastLast == (currLast-1) &&
        lastCurrent == (currCurrent-1) &&
        currCurrent == (currLast-1) &&
        TRUE )
    {
        result = TRUE;
    }

    return result;
}

static VOID ClearMemo()
{
    SendDlgItemMessage( moveHistoryDialog, IDC_MoveHistory, WM_SETTEXT, 0, (LPARAM) "" );
}

static int AppendToMemo( char * text, DWORD flags, DWORD color )
{
    CHARFORMAT cf;

    HWND hMemo = GetDlgItem( moveHistoryDialog, IDC_MoveHistory );

    /* Select end of text */
    int cbTextLen = (int) SendMessage( hMemo, WM_GETTEXTLENGTH, 0, 0 );

    SendMessage( hMemo, EM_SETSEL, cbTextLen, cbTextLen );

    /* Set style */
    ZeroMemory( &cf, sizeof(cf) );

    cf.cbSize = sizeof(cf);
    cf.dwMask = CFM_BOLD | CFM_ITALIC | CFM_COLOR | CFM_UNDERLINE;
    cf.dwEffects = flags;

    if( color != DEFAULT_COLOR ) {
        cf.crTextColor = color;
    }
    else {
        cf.dwEffects |= CFE_AUTOCOLOR;
    }

    SendMessage( hMemo, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM) &cf );

    /* Append text */
    SendMessage( hMemo, EM_REPLACESEL, (WPARAM) FALSE, (LPARAM) text );

    /* Return offset of appended text */
    return cbTextLen;
}

static VOID AppendMoveToMemo( int index )
{
    char buf[64];
    DWORD flags = 0;
    DWORD color = DEFAULT_COLOR;

    if( index < 0 || index >= MAX_MOVES ) {
        return;
    }

    buf[0] = '\0';

    /* Move number */
    if( (index % 2) == 0 ) {
        sprintf( buf, "%d.%s ", (index / 2)+1, index & 1 ? ".." : "" );
        AppendToMemo( buf, CFE_BOLD, DEFAULT_COLOR );
    }

    /* Move text */
    strcpy( buf, SavePart( currMovelist[index] ) );
    strcat( buf, " " );

    histMoves[index].memoOffset = AppendToMemo( buf, flags, color );
    histMoves[index].memoLength = strlen(buf)-1;

    /* PV info (if any) */
    if( appData.showEvalInMoveHistory && currPvInfo[index].depth > 0 ) {
        sprintf( buf, "%{%s%.2f/%d} ",
            currPvInfo[index].score >= 0 ? "+" : "",
            currPvInfo[index].score / 100.0,
            currPvInfo[index].depth );

        AppendToMemo( buf, flags,
            color == DEFAULT_COLOR ? GetSysColor(COLOR_GRAYTEXT) : color );
    }
}

static void RefreshMemoContent()
{
    int i;

    ClearMemo();

    for( i=currFirst; i<currLast; i++ ) {
        AppendMoveToMemo( i );
    }
}

static void MemoContentUpdated()
{
    int caretPos;

    HighlightMove( lastCurrent, FALSE );
    HighlightMove( currCurrent, TRUE );

    lastFirst = currFirst;
    lastLast = currLast;
    lastCurrent = currCurrent;

    /* Deselect any text, move caret to end of memo */
    if( currCurrent >= 0 ) {
        caretPos = histMoves[currCurrent].memoOffset + histMoves[currCurrent].memoLength;
    }
    else {
        caretPos = (int) SendDlgItemMessage( moveHistoryDialog, IDC_MoveHistory, WM_GETTEXTLENGTH, 0, 0 );
    }

    SendDlgItemMessage( moveHistoryDialog, IDC_MoveHistory, EM_SETSEL, caretPos, caretPos );

    SendDlgItemMessage( moveHistoryDialog, IDC_MoveHistory, EM_SCROLLCARET, 0, 0 );
}

int FindMoveByCharIndex( int char_index )
{
    int index;

    for( index=currFirst; index<currLast; index++ ) {
        if( char_index >= histMoves[index].memoOffset &&
            char_index <  (histMoves[index].memoOffset + histMoves[index].memoLength) )
        {
            return index;
        }
    }

    return -1;
}

LRESULT CALLBACK HistoryDialogProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
    static SnapData sd;

    switch (message) {
    case WM_INITDIALOG:
        if( moveHistoryDialog == NULL ) {
            moveHistoryDialog = hDlg;

            /* Enable word wrapping and notifications */
            SendDlgItemMessage( moveHistoryDialog, IDC_MoveHistory, EM_SETTARGETDEVICE, 0, 0 );

            SendDlgItemMessage( moveHistoryDialog, IDC_MoveHistory, EM_SETEVENTMASK, 0, ENM_MOUSEEVENTS );

            /* Restore window placement */
            RestoreWindowPlacement( hDlg, &wpMoveHistory );
        }

        /* Update memo */
        RefreshMemoContent();

        MemoContentUpdated();

        return FALSE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
          EndDialog(hDlg, TRUE);
          return TRUE;

        case IDCANCEL:
          EndDialog(hDlg, FALSE);
          return TRUE;

        default:
          break;
        }

        break;

    case WM_NOTIFY:
        if( wParam == IDC_MoveHistory ) {
            MSGFILTER * lpMF = (MSGFILTER *) lParam;

            if( lpMF->msg == WM_LBUTTONDBLCLK && (lpMF->wParam & (MK_CONTROL | MK_SHIFT)) == 0 ) {
                POINTL pt;
                LRESULT index;

                pt.x = LOWORD( lpMF->lParam );
                pt.y = HIWORD( lpMF->lParam );

                index = SendDlgItemMessage( hDlg, IDC_MoveHistory, EM_CHARFROMPOS, 0, (LPARAM) &pt );

                index = FindMoveByCharIndex( index );

                if( index >= 0 ) {
                    ToNrEvent( index + 1 );
                }

                /* Zap the message for good: apparently, returning non-zero is not enough */
                lpMF->msg = WM_USER;

                return TRUE;
            }
        }
        break;

    case WM_REFRESH_HISTORY:
        /* Update the GUI */
        if( OnlyCurrentPositionChanged() ) {
            /* Only "cursor" changed, no need to update memo content */
        }
        else if( OneMoveAppended() ) {
            AppendMoveToMemo( currCurrent );
        }
        else {
            RefreshMemoContent();
        }

        MemoContentUpdated();

        break;

    case WM_SIZE:
        SetWindowPos( GetDlgItem( moveHistoryDialog, IDC_MoveHistory ),
            HWND_TOP,
            H_MARGIN, V_MARGIN,
            LOWORD(lParam) - 2*H_MARGIN,
            HIWORD(lParam) - 2*V_MARGIN,
            SWP_NOZORDER );
        break;

    case WM_GETMINMAXINFO:
        {
            MINMAXINFO * mmi = (MINMAXINFO *) lParam;

            mmi->ptMinTrackSize.x = 100;
            mmi->ptMinTrackSize.y = 100;
        }
        break;

    case WM_CLOSE:
        MoveHistoryPopDown();
        break;

    case WM_ENTERSIZEMOVE:
        return OnEnterSizeMove( &sd, hDlg, wParam, lParam );

    case WM_SIZING:
        return OnSizing( &sd, hDlg, wParam, lParam );

    case WM_MOVING:
        return OnMoving( &sd, hDlg, wParam, lParam );

    case WM_EXITSIZEMOVE:
        return OnExitSizeMove( &sd, hDlg, wParam, lParam );
    }

    return FALSE;
}

VOID MoveHistoryPopUp()
{
  FARPROC lpProc;

  CheckMenuItem(GetMenu(hwndMain), IDM_ShowMoveHistory, MF_CHECKED);

  if( moveHistoryDialog ) {
    SendMessage( moveHistoryDialog, WM_INITDIALOG, 0, 0 );

    if( ! moveHistoryDialogUp ) {
        ShowWindow(moveHistoryDialog, SW_SHOW);
    }
  }
  else {
    lpProc = MakeProcInstance( (FARPROC) HistoryDialogProc, hInst );

    /* Note to self: dialog must have the WS_VISIBLE style set, otherwise it's not shown! */
    CreateDialog( hInst, MAKEINTRESOURCE(DLG_MoveHistory), hwndMain, (DLGPROC)lpProc );

    FreeProcInstance(lpProc);
  }

  moveHistoryDialogUp = TRUE;
}

VOID MoveHistoryPopDown()
{
  CheckMenuItem(GetMenu(hwndMain), IDM_ShowMoveHistory, MF_UNCHECKED);

  if( moveHistoryDialog ) {
      ShowWindow(moveHistoryDialog, SW_HIDE);
  }

  moveHistoryDialogUp = FALSE;
}

VOID MoveHistorySet( char movelist[][2*MOVE_LEN], int first, int last, int current, ChessProgramStats_Move * pvInfo )
{
    /* [AS] Danger! For now we rely on the movelist parameter being a static variable! */

    currMovelist = movelist;
    currFirst = first;
    currLast = last;
    currCurrent = current;
    currPvInfo = pvInfo;

    if( moveHistoryDialog ) {
        SendMessage( moveHistoryDialog, WM_REFRESH_HISTORY, 0, 0 );
    }
}

BOOL MoveHistoryIsUp()
{
    return moveHistoryDialogUp;
}
