/*
 * Move history for WinBoard
 *
 * Author: Alessandro Scotti (Dec 2005)
 * front-end code split off by HGM
 *
 * Copyright 2005 Alessandro Scotti
 *
 * Enhancements Copyright 2009, 2010, 2014 Free Software Foundation, Inc.
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
 * along with this program. If not, see http://www.gnu.org/licenses/.
 *
 * ------------------------------------------------------------------------
 ** See the file ChangeLog for a revision history.  */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <windows.h> /* required for all Windows applications */
#include <richedit.h>
#include <commdlg.h>
#include <dlgs.h>

#include "common.h"
#include "frontend.h"
#include "backend.h"
#include "winboard.h"
#include "wsnap.h"

// templates for calls into back-end
void RefreshMemoContent P((void));
void MemoContentUpdated P((void));
void FindMoveByCharIndex P(( int char_index ));

#define DEFAULT_COLOR       0xFFFFFFFF

#define H_MARGIN            2
#define V_MARGIN            2

static BOOLEAN moveHistoryDialogUp = FALSE;

// ------------- low-level front-end actions called by MoveHistory back-end -----------------

// low-level front-end, after calculating from & to is left to caller
// it task is to highlight the indicated characters. (In WinBoard it makes them bold and blue.)
void HighlightMove( int from, int to, Boolean highlight )
{
        CHARFORMAT cf;
        HWND hMemo = GetDlgItem( moveHistoryDialog, IDC_MoveHistory );

        SendMessage( hMemo, EM_SETSEL, from, to);


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

// low-level front-end, but replace Windows data types to make it callable from back-end
// its task is to clear the contents of the move-history text edit
void ClearHistoryMemo()
{
    SendDlgItemMessage( moveHistoryDialog, IDC_MoveHistory, WM_SETTEXT, 0, (LPARAM) "" );
}

// low-level front-end, made callable from back-end by passing flags and color numbers
// its task is to append the given text to the text edit
// the bold argument says 0 = normal, 1 = bold typeface
// the colorNr argument says 0 = font-default, 1 = gray
int AppendToHistoryMemo( char * text, int bold, int colorNr )
{
    CHARFORMAT cf;
    DWORD flags = bold ? CFE_BOLD :0;
    DWORD color = colorNr ? GetSysColor(COLOR_GRAYTEXT) : DEFAULT_COLOR;

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

// low-level front-end; wrapper for the code to scroll the mentioned character in view (-1 = end)
void ScrollToCurrent(int caretPos)
{
    if(caretPos < 0)
        caretPos = (int) SendDlgItemMessage( moveHistoryDialog, IDC_MoveHistory, WM_GETTEXTLENGTH, 0, 0 );
    SendDlgItemMessage( moveHistoryDialog, IDC_MoveHistory, EM_SETSEL, caretPos, caretPos );

    SendDlgItemMessage( moveHistoryDialog, IDC_MoveHistory, EM_SCROLLCARET, 0, 0 );
}


// ------------------------------ call backs --------------------------

// front-end. Universal call-back for any event. Recognized vents are dialog creation, OK and cancel button-press
// (dead code, as these buttons do not exist?), mouse clicks on the text edit, and moving / sizing
LRESULT CALLBACK HistoryDialogProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
    static SnapData sd;

    switch (message) {
    case WM_INITDIALOG:
        if( moveHistoryDialog == NULL ) {
            moveHistoryDialog = hDlg;
            Translate(hDlg, DLG_MoveHistory);

            /* Enable word wrapping and notifications */
            SendDlgItemMessage( moveHistoryDialog, IDC_MoveHistory, EM_SETTARGETDEVICE, 0, 0 );

            SendDlgItemMessage( moveHistoryDialog, IDC_MoveHistory, EM_SETEVENTMASK, 0, ENM_MOUSEEVENTS );

            /* Set font */
	    SendDlgItemMessage( moveHistoryDialog, IDC_MoveHistory, WM_SETFONT, (WPARAM)font[boardSize][MOVEHISTORY_FONT]->hf, MAKELPARAM(TRUE, 0 ));

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

                FindMoveByCharIndex( index ); // [HGM] also does the actual moving to it, now

                /* Zap the message for good: apparently, returning non-zero is not enough */
                lpMF->msg = WM_USER;

                return TRUE;
            }
        }
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

// ------------ standard entry points into MoveHistory code -----------

// front-end
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

// Note that in WIndows creating the dialog causes its call-back to perform
// RefreshMemoContent() and MemoContentUpdated() immediately after it is realized.
// To port this to X we might have to do that from here.
}

// front-end
VOID MoveHistoryPopDown()
{
  CheckMenuItem(GetMenu(hwndMain), IDM_ShowMoveHistory, MF_UNCHECKED);

  if( moveHistoryDialog ) {
      ShowWindow(moveHistoryDialog, SW_HIDE);
  }

  moveHistoryDialogUp = FALSE;
}

// front-end
Boolean MoveHistoryIsUp()
{
    return moveHistoryDialogUp;
}

// front-end
Boolean MoveHistoryDialogExists()
{
    return moveHistoryDialog != NULL;
}
