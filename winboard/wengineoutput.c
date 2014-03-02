/*
 * wengineoutput.c - split-off front-end of Engine output (PV) by HGM
 *
 * Author: Alessandro Scotti (Dec 2005)
 *
 * Copyright 2005 Alessandro Scotti
 *
 * Enhancements Copyright 2009, 2010, 2011, 2012, 2013, 2014 Free Software Foundation, Inc.
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

#include <windows.h> /* required for all Windows applications */
#include <richedit.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <commdlg.h>
#include <dlgs.h>

#include "common.h"
#include "frontend.h"
#include "backend.h"
#include "winboard.h"

#include "wsnap.h"
#include "engineoutput.h"

/* Module variables */
int  windowMode = 1;
static BOOLEAN engineOutputDialogUp = FALSE;
HICON icons[8]; // [HGM] this front-end array translates back-end icon indicator to handle
HWND outputField[2][7]; // [HGM] front-end array to translate output field to window handle

// front end
static HICON LoadIconEx( int id )
{
    return LoadImage( hInst, MAKEINTRESOURCE(id), IMAGE_ICON, ICON_SIZE, ICON_SIZE, 0 );
}

// [HGM] the platform-dependent way of indicating where output should go is now all
// concentrated here, where a table of platform-dependent handles are initialized.
// This cleanses most other routines of front-end stuff, so they can go into the back end.
static void InitializeEngineOutput()
{
	// [HGM] made this into a table, rather than separate global variables
        icons[nColorBlack]   = LoadIconEx( IDI_BLACK_14 );
        icons[nColorWhite]   = LoadIconEx( IDI_WHITE_14 );
        icons[nColorUnknown] = LoadIconEx( IDI_UNKNOWN_14 );
        icons[nClear]        = LoadIconEx( IDI_TRANS_14 );
        icons[nPondering]    = LoadIconEx( IDI_PONDER_14 );
        icons[nThinking]     = LoadIconEx( IDI_CLOCK_14 );
        icons[nAnalyzing]    = LoadIconEx( IDI_ANALYZE2_14 );

	// [HGM] also make a table of handles to output controls
	// Note that engineOutputDialog must be defined first!
        outputField[0][nColorIcon] = GetDlgItem( engineOutputDialog, IDC_Color1 );
        outputField[0][nLabel]     = GetDlgItem( engineOutputDialog, IDC_EngineLabel1 );
        outputField[0][nStateIcon] = GetDlgItem( engineOutputDialog, IDC_StateIcon1 );
        outputField[0][nStateData] = GetDlgItem( engineOutputDialog, IDC_StateData1 );
        outputField[0][nLabelNPS]  = GetDlgItem( engineOutputDialog, IDC_Engine1_NPS );
        outputField[0][nMemo]      = GetDlgItem( engineOutputDialog, IDC_EngineMemo1 );

        outputField[1][nColorIcon] = GetDlgItem( engineOutputDialog, IDC_Color2 );
        outputField[1][nLabel]     = GetDlgItem( engineOutputDialog, IDC_EngineLabel2 );
        outputField[1][nStateIcon] = GetDlgItem( engineOutputDialog, IDC_StateIcon2 );
        outputField[1][nStateData] = GetDlgItem( engineOutputDialog, IDC_StateData2 );
        outputField[1][nLabelNPS]  = GetDlgItem( engineOutputDialog, IDC_Engine2_NPS );
        outputField[1][nMemo]      = GetDlgItem( engineOutputDialog, IDC_EngineMemo2 );
}

// front end
static void SetControlPos( HWND hDlg, int id, int x, int y, int width, int height )
{
    HWND hControl = GetDlgItem( hDlg, id );

    SetWindowPos( hControl, HWND_TOP, x, y, width, height, SWP_NOZORDER );
}

#define HIDDEN_X    20000
#define HIDDEN_Y    20000

// front end
static void HideControl( HWND hDlg, int id )
{
    HWND hControl = GetDlgItem( hDlg, id );
    RECT rc;

    GetWindowRect( hControl, &rc );

    /* 
        Avoid hiding an already hidden control, because that causes many
        unnecessary WM_ERASEBKGND messages!
    */
    if( rc.left != HIDDEN_X || rc.top != HIDDEN_Y ) {
        SetControlPos( hDlg, id, 20000, 20000, 100, 100 );
    }
}

// front end, although we might make GetWindowRect front end instead
static int GetControlWidth( HWND hDlg, int id )
{
    RECT rc;

    GetWindowRect( GetDlgItem( hDlg, id ), &rc );

    return rc.right - rc.left;
}

// front end?
static int GetControlHeight( HWND hDlg, int id )
{
    RECT rc;

    GetWindowRect( GetDlgItem( hDlg, id ), &rc );

    return rc.bottom - rc.top;
}

static int GetHeaderHeight()
{
    int result = GetControlHeight( engineOutputDialog, IDC_EngineLabel1 );

    if( result < ICON_SIZE ) result = ICON_SIZE;

    return result;
}

// The size calculations should be backend? If setControlPos is a platform-dependent way of doing things,
// a platform-independent wrapper for it should be supplied.
static void PositionControlSet( HWND hDlg, int x, int y, int clientWidth, int memoHeight, int idColor, int idEngineLabel, int idNPS, int idMemo, int idStateIcon, int idStateData )
{
    int label_x = x + ICON_SIZE + H_MARGIN;
    int label_h = GetControlHeight( hDlg, IDC_EngineLabel1 );
    int label_y = y + ICON_SIZE - label_h;
    int nps_w = GetControlWidth( hDlg, IDC_Engine1_NPS );
    int nps_x = clientWidth - H_MARGIN - nps_w;
    int state_data_w = GetControlWidth( hDlg, IDC_StateData1 );
    int state_data_x = nps_x - H_MARGIN - state_data_w;
    int state_icon_x = state_data_x - ICON_SIZE - 2;
    int max_w = clientWidth - 2*H_MARGIN;
    int memo_y = y + ICON_SIZE + LABEL_V_DISTANCE;

    SetControlPos( hDlg, idColor, x, y, ICON_SIZE, ICON_SIZE );
    SetControlPos( hDlg, idEngineLabel, label_x, label_y, state_icon_x - label_x, label_h );
    SetControlPos( hDlg, idStateIcon, state_icon_x, y, ICON_SIZE, ICON_SIZE );
    SetControlPos( hDlg, idStateData, state_data_x, label_y, state_data_w, label_h );
    SetControlPos( hDlg, idNPS, nps_x, label_y, nps_w, label_h );
    SetControlPos( hDlg, idMemo, x, memo_y, max_w, memoHeight );
}

// Also here some of the size calculations should go to the back end, and their actual application to a front-end routine
void ResizeWindowControls( int mode )
{
    HWND hDlg = engineOutputDialog; // [HGM] used to be parameter, but routine is called from back-end
    RECT rc;
    int headerHeight = GetHeaderHeight();
//    int labelHeight = GetControlHeight( hDlg, IDC_EngineLabel1 );
//    int labelOffset = H_MARGIN + ICON_SIZE + H_MARGIN;
//    int labelDeltaY = ICON_SIZE - labelHeight;
    int clientWidth;
    int clientHeight;
    int maxControlWidth;
    int npsWidth;

    /* Initialize variables */
    GetClientRect( hDlg, &rc );

    clientWidth = rc.right - rc.left;
    clientHeight = rc.bottom - rc.top;

    maxControlWidth = clientWidth - 2*H_MARGIN;

    npsWidth = GetControlWidth( hDlg, IDC_Engine1_NPS );

    /* Resize controls */
    if( mode == 0 ) {
        /* One engine */
        PositionControlSet( hDlg, H_MARGIN, V_MARGIN, 
            clientWidth, 
            clientHeight - V_MARGIN - LABEL_V_DISTANCE - headerHeight- V_MARGIN,
            IDC_Color1, IDC_EngineLabel1, IDC_Engine1_NPS, IDC_EngineMemo1, IDC_StateIcon1, IDC_StateData1 );

        /* Hide controls for the second engine */
        HideControl( hDlg, IDC_Color2 );
        HideControl( hDlg, IDC_EngineLabel2 );
        HideControl( hDlg, IDC_StateIcon2 );
        HideControl( hDlg, IDC_StateData2 );
        HideControl( hDlg, IDC_Engine2_NPS );
        HideControl( hDlg, IDC_EngineMemo2 );
        SendDlgItemMessage( hDlg, IDC_EngineMemo2, WM_SETTEXT, 0, (LPARAM) "" );
        /* TODO: we should also hide/disable them!!! what about tab stops?!?! */
    }
    else {
        /* Two engines */
        int memo_h = (clientHeight - headerHeight*2 - V_MARGIN*2 - LABEL_V_DISTANCE*2 - SPLITTER_SIZE) / 2;
        int header1_y = V_MARGIN;
        int header2_y = V_MARGIN + headerHeight + LABEL_V_DISTANCE + memo_h + SPLITTER_SIZE;

        PositionControlSet( hDlg, H_MARGIN, header1_y, clientWidth, memo_h,
            IDC_Color1, IDC_EngineLabel1, IDC_Engine1_NPS, IDC_EngineMemo1, IDC_StateIcon1, IDC_StateData1 );

        PositionControlSet( hDlg, H_MARGIN, header2_y, clientWidth, memo_h,
            IDC_Color2, IDC_EngineLabel2, IDC_Engine2_NPS, IDC_EngineMemo2, IDC_StateIcon2, IDC_StateData2 );
    }

    InvalidateRect( GetDlgItem(hDlg,IDC_EngineMemo1), NULL, FALSE );
    InvalidateRect( GetDlgItem(hDlg,IDC_EngineMemo2), NULL, FALSE );
}

static int currentPV;
int highTextStart[2], highTextEnd[2];
extern RECT boardRect;

VOID
GetMemoLine(HWND hDlg, int x, int y)
{	// [HGM] pv: translate click to PV line, and load it for display
	char buf[10000];
	int index, start, end, memo;
	POINT pt;

	pt.x = x; pt.y = y;
	memo = currentPV ? IDC_EngineMemo2 : IDC_EngineMemo1;
	index = SendDlgItemMessage( hDlg, memo, EM_CHARFROMPOS, 0, (LPARAM) &pt );
	GetDlgItemText(hDlg, memo, buf, sizeof(buf));
	if(LoadMultiPV(x, y, buf, index, &start, &end, currentPV)) {
	    SetCapture(hDlg);
	    SendMessage( outputField[currentPV][nMemo], EM_SETSEL, (WPARAM)start, (LPARAM)end );
	    highTextStart[currentPV] = start; highTextEnd[currentPV] = end;
	    SetFocus(outputField[currentPV][nMemo]);
	}
}

// front end. Actual printing of PV lines into the output field
void InsertIntoMemo( int which, char * text, int where )
{
    SendMessage( outputField[which][nMemo], EM_SETSEL, where, where ); // [HGM] multivar: choose insertion point

    SendMessage( outputField[which][nMemo], EM_REPLACESEL, (WPARAM) FALSE, (LPARAM) text );
    if(where < highTextStart[which]) { // [HGM] multiPVdisplay: move highlighting
	int len = strlen(text);
	highTextStart[which] += len; highTextEnd[which] += len;
	SendMessage( outputField[which][nMemo], EM_SETSEL, highTextStart[which], highTextEnd[which] );
    }
}

// front end. Associates an icon with an output field ("control" in Windows jargon).
// [HGM] let it find out the output field from the 'which' number by itself
void SetIcon( int which, int field, int nIcon )
{

    if( nIcon != 0 ) {
        SendMessage( outputField[which][field], STM_SETICON, (WPARAM) icons[nIcon], 0 );
    }
}

// front end wrapper for SetWindowText, taking control number in stead of handle
void DoSetWindowText(int which, int field, char *s_label)
{
    SetWindowText( outputField[which][field], s_label );
}

void SetEngineOutputTitle(char *title)
{
    SetWindowText( engineOutputDialog, title );
}

// This seems pure front end
LRESULT CALLBACK EngineOutputProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
    static SnapData sd;

    switch (message) {
    case WM_INITDIALOG:
        if( engineOutputDialog == NULL ) {
            engineOutputDialog = hDlg;

            Translate(hDlg, DLG_EngineOutput);
            RestoreWindowPlacement( hDlg, &wpEngineOutput ); /* Restore window placement */

            ResizeWindowControls( windowMode );

            SendDlgItemMessage( hDlg, IDC_EngineMemo1, EM_SETEVENTMASK, 0, ENM_MOUSEEVENTS );
            SendDlgItemMessage( hDlg, IDC_EngineMemo2, EM_SETEVENTMASK, 0, ENM_MOUSEEVENTS );

	    /* Set font */
	    SendDlgItemMessage( engineOutputDialog, IDC_EngineMemo1, WM_SETFONT, (WPARAM)font[boardSize][MOVEHISTORY_FONT]->hf, MAKELPARAM(TRUE, 0 ));
	    SendDlgItemMessage( engineOutputDialog, IDC_EngineMemo2, WM_SETFONT, (WPARAM)font[boardSize][MOVEHISTORY_FONT]->hf, MAKELPARAM(TRUE, 0 ));

            SetEngineState( 0, STATE_IDLE, "" );
            SetEngineState( 1, STATE_IDLE, "" );
        }

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

    case WM_MOUSEMOVE:
        MovePV(LOWORD(lParam) - boardRect.left, HIWORD(lParam) - boardRect.top, boardRect.bottom - boardRect.top);
        break;

    case WM_RBUTTONUP:
        ReleaseCapture();
        SendMessage( outputField[currentPV][nMemo], EM_SETSEL, 0, 0 );
        highTextStart[currentPV] = highTextEnd[currentPV] = 0;
        UnLoadPV();
        break;

    case WM_NOTIFY:
        if( wParam == IDC_EngineMemo1 || wParam == IDC_EngineMemo2 ) {
            MSGFILTER * lpMF = (MSGFILTER *) lParam;
            if( lpMF->msg == WM_RBUTTONDOWN && (lpMF->wParam & (MK_CONTROL)) == 0 ) {
		shiftKey = (lpMF->wParam & MK_SHIFT) != 0; // [HGM] remember last shift status
                currentPV = (wParam == IDC_EngineMemo2);
                GetMemoLine(hDlg, LOWORD(lpMF->lParam), HIWORD(lpMF->lParam));
            }
        }
        break;

    case WM_GETMINMAXINFO:
        {
            MINMAXINFO * mmi = (MINMAXINFO *) lParam;
        
            mmi->ptMinTrackSize.x = 100;
            mmi->ptMinTrackSize.y = 160;
        }
        break;

    case WM_CLOSE:
        EngineOutputPopDown();
        break;

    case WM_SIZE:
        ResizeWindowControls( windowMode );
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

// front end
void EngineOutputPopUp()
{
  FARPROC lpProc;
  static int  needInit = TRUE;
  
  CheckMenuItem(GetMenu(hwndMain), IDM_ShowEngineOutput, MF_CHECKED);

  if( engineOutputDialog ) {
    SendMessage( engineOutputDialog, WM_INITDIALOG, 0, 0 );

    if( ! engineOutputDialogUp ) {
        ShowWindow(engineOutputDialog, SW_SHOW);
    }
  }
  else {
    lpProc = MakeProcInstance( (FARPROC) EngineOutputProc, hInst );

    /* Note to self: dialog must have the WS_VISIBLE style set, otherwise it's not shown! */
    CreateDialog( hInst, MAKEINTRESOURCE(DLG_EngineOutput), hwndMain, (DLGPROC)lpProc );

    FreeProcInstance(lpProc);
  }

  // [HGM] displaced to after creation of dialog, to allow initialization of output fields
  if( needInit ) {
      InitializeEngineOutput();
      needInit = FALSE;
  }

  engineOutputDialogUp = TRUE;
}

// front end
void EngineOutputPopDown()
{
  CheckMenuItem(GetMenu(hwndMain), IDM_ShowEngineOutput, MF_UNCHECKED);

  if( engineOutputDialog ) {
      ShowWindow(engineOutputDialog, SW_HIDE);
  }

  engineOutputDialogUp = FALSE;
}

// front end. [HGM] Takes handle of output control from table, so only number is passed
void DoClearMemo(int which)
{
        SendMessage( outputField[which][nMemo], WM_SETTEXT, 0, (LPARAM) "" );
}

// front end (because only other front-end wants to know)
int EngineOutputIsUp()
{
    return engineOutputDialogUp;
}

// front end, to give back-end access to engineOutputDialog
int EngineOutputDialogExists()
{
    return engineOutputDialog != NULL;
}
