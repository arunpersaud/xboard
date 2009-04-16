/*
 * Engine output (PV)
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

VOID EngineOutputPopUp();
VOID EngineOutputPopDown();
BOOL EngineOutputIsUp();

/* Imports from backend.c */
char * SavePart(char *str);

/* Imports from winboard.c */
extern HWND engineOutputDialog;
extern BOOLEAN engineOutputDialogUp;

extern HINSTANCE hInst;
extern HWND hwndMain;

extern WindowPlacement wpEngineOutput;

/* Module variables */
#define H_MARGIN            2
#define V_MARGIN            2
#define LABEL_V_DISTANCE    1   /* Distance between label and memo */
#define SPLITTER_SIZE       4   /* Distance between first memo and second label */

static int  windowMode = 1;

static int  lastDepth[2] = { -1, -1 };

static VOID SetControlPos( HWND hDlg, int id, int x, int y, int width, int height )
{
    HWND hControl = GetDlgItem( hDlg, id );

    SetWindowPos( hControl, HWND_TOP, x, y, width, height, SWP_NOZORDER );
}

static VOID HideControl( HWND hDlg, int id )
{
    /* TODO: we should also hide/disable it!!! what about tab stops?!?! */
    SetControlPos( hDlg, id, 20000, 20000, 100, 100 );
}

static int GetControlWidth( HWND hDlg, int id )
{
    RECT rc;

    GetWindowRect( GetDlgItem( hDlg, IDC_EngineLabel1 ), &rc );

    return rc.right - rc.left;
}

static VOID ResizeWindowControls( HWND hDlg, int mode )
{
    RECT rc;
    int labelHeight;
    int clientWidth;
    int clientHeight;
    int maxControlWidth;
    int npsWidth;

    /* Initialize variables */
    GetWindowRect( GetDlgItem( hDlg, IDC_EngineLabel1 ), &rc );

    labelHeight = rc.bottom - rc.top;

    GetClientRect( hDlg, &rc );

    clientWidth = rc.right - rc.left;
    clientHeight = rc.bottom - rc.top;

    maxControlWidth = clientWidth - 2*H_MARGIN;

    npsWidth = GetControlWidth( hDlg, IDC_Engine1_NPS );

    /* Resize controls */
    if( mode == 0 ) {
        /* One engine */
        int memo_y = V_MARGIN + labelHeight + LABEL_V_DISTANCE;
        int memo_h = clientHeight - memo_y - V_MARGIN;

        SetControlPos( hDlg, IDC_EngineLabel1, H_MARGIN, V_MARGIN, maxControlWidth / 2, labelHeight );
        SetControlPos( hDlg, IDC_EngineMemo1, H_MARGIN, memo_y, maxControlWidth, memo_h );

        /* Hide controls for the second engine */
        HideControl( hDlg, IDC_EngineLabel2 );
        HideControl( hDlg, IDC_Engine2_NPS );
        HideControl( hDlg, IDC_EngineMemo2 );
        /* TODO: we should also hide/disable them!!! what about tab stops?!?! */
    }
    else {
        /* Two engines */
        int memo1_y = V_MARGIN + labelHeight + LABEL_V_DISTANCE;
        int memo_h = (clientHeight - memo1_y - V_MARGIN - labelHeight - LABEL_V_DISTANCE - SPLITTER_SIZE) / 2;
        int label2_y = memo1_y + memo_h + SPLITTER_SIZE;
        int memo2_y = label2_y + labelHeight + LABEL_V_DISTANCE;
        int nps_x = clientWidth - H_MARGIN - npsWidth;

        SetControlPos( hDlg, IDC_EngineLabel1, H_MARGIN, V_MARGIN, maxControlWidth / 2, labelHeight );
        SetControlPos( hDlg, IDC_Engine1_NPS, nps_x, V_MARGIN, npsWidth, labelHeight );
        SetControlPos( hDlg, IDC_EngineMemo1, H_MARGIN, memo1_y, maxControlWidth, memo_h );

        SetControlPos( hDlg, IDC_EngineLabel2, H_MARGIN, label2_y, maxControlWidth / 2, labelHeight );
        SetControlPos( hDlg, IDC_Engine2_NPS, nps_x, label2_y, npsWidth, labelHeight );
        SetControlPos( hDlg, IDC_EngineMemo2, H_MARGIN, memo2_y, maxControlWidth, memo_h );
    }

    InvalidateRect( GetDlgItem(hDlg,IDC_EngineMemo1), NULL, FALSE );
    InvalidateRect( GetDlgItem(hDlg,IDC_EngineMemo2), NULL, FALSE );
}

static VOID SetDisplayMode( int mode )
{
    if( windowMode != mode ) {
        windowMode = mode;

        ResizeWindowControls( engineOutputDialog, mode );
    }
}

static VOID VerifyDisplayMode()
{
    int mode;

    /* Get proper mode for current game */
    switch( gameMode ) {
    case AnalyzeMode:
    case AnalyzeFile:
    case MachinePlaysWhite:
    case MachinePlaysBlack:
        mode = 0;
        break;
    case TwoMachinesPlay:
        mode = 1;
        break;
    default:
        /* Do not change */
        return;
    }

    SetDisplayMode( mode );
}

static VOID InsertIntoMemo( HWND hMemo, char * text )
{
    SendMessage( hMemo, EM_SETSEL, 0, 0 );

    SendMessage( hMemo, EM_REPLACESEL, (WPARAM) FALSE, (LPARAM) text );
}

#define MAX_NAME_LENGTH 32

static VOID UpdateControls( HWND hLabel, HWND hLabelNPS, HWND hMemo, char * name, int depth, unsigned long nodes, int score, int time, char * pv )
{
    char s_label[MAX_NAME_LENGTH + 64];

    /* Label */
    if( name == 0 || *name == '\0' ) {
        name = "?";
    }

    strncpy( s_label, name, MAX_NAME_LENGTH );
    s_label[ MAX_NAME_LENGTH-1 ] = '\0';

    SetWindowText( hLabel, s_label );

    s_label[0] = '\0';

    if( time > 0 && nodes > 0 ) {
        unsigned long nps_100 = nodes / time;

        if( nps_100 < 100000 ) {
            sprintf( s_label, "NPS: %lu", nps_100 * 100 );
        }
        else {
            sprintf( s_label, "NPS: %.1fk", nps_100 / 10.0 );
        }
    }

    SetWindowText( hLabelNPS, s_label );

    /* Memo */
    if( pv != 0 && *pv != '\0' ) {
        char s_nodes[24];
        char s_score[16];
        char s_time[24];
        char buf[256];
        int buflen;
        int time_secs = time / 100;
        int time_cent = time % 100;

        /* Nodes */
        if( nodes < 1000000 ) {
            sprintf( s_nodes, "%lu", nodes );
        }
        else {
            sprintf( s_nodes, "%.1fM", nodes / 1000000.0 );
        }

        /* Score */
        if( score > 0 ) {
            sprintf( s_score, "+%.2f", score / 100.0 );
        }
        else {
            sprintf( s_score, "%.2f", score / 100.0 );
        }

        /* Time */
        sprintf( s_time, "%d:%02d.%02d", time_secs / 60, time_secs % 60, time_cent );

        /* Put all together... */
        sprintf( buf, "%3d\t%s\t%s\t%s\t", depth, s_score, s_nodes, s_time );

        /* Add PV */
        buflen = strlen(buf);

        strncpy( buf + buflen, pv, sizeof(buf) - buflen );

        buf[ sizeof(buf) - 3 ] = '\0';

        strcat( buf + buflen, "\r\n" );

        /* Update memo */
        InsertIntoMemo( hMemo, buf );
    }
}

LRESULT CALLBACK EngineOutputProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
    static SnapData sd;

    switch (message) {
    case WM_INITDIALOG:
        if( engineOutputDialog == NULL ) {
            engineOutputDialog = hDlg;

            RestoreWindowPlacement( hDlg, &wpEngineOutput ); /* Restore window placement */

            ResizeWindowControls( hDlg, windowMode );
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
        ResizeWindowControls( hDlg, windowMode );
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

VOID EngineOutputPopUp()
{
  FARPROC lpProc;

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

  engineOutputDialogUp = TRUE;
}

VOID EngineOutputPopDown()
{
  CheckMenuItem(GetMenu(hwndMain), IDM_ShowEngineOutput, MF_UNCHECKED);

  if( engineOutputDialog ) {
      ShowWindow(engineOutputDialog, SW_HIDE);
  }

  engineOutputDialogUp = FALSE;
}

BOOL EngineOutputIsUp()
{
    return engineOutputDialogUp;
}

VOID EngineOutputUpdate( int which, int depth, unsigned long nodes, int score, int time, char * pv )
{
    HWND hLabel;
    HWND hLabelNPS;
    HWND hMemo;
    char * name;

    if( which < 0 || which > 1 || depth < 0 || time < 0 || pv == 0 || *pv == '\0' ) {
        return;
    }

    if( engineOutputDialog == NULL ) {
        return;
    }

    VerifyDisplayMode();

    /* Get target control */
    if( which == 0 ) {
        hLabel = GetDlgItem( engineOutputDialog, IDC_EngineLabel1 );
        hLabelNPS = GetDlgItem( engineOutputDialog, IDC_Engine1_NPS );
        hMemo  = GetDlgItem( engineOutputDialog, IDC_EngineMemo1 );
        name = first.tidy;
    }
    else {
        hLabel = GetDlgItem( engineOutputDialog, IDC_EngineLabel2 );
        hLabelNPS = GetDlgItem( engineOutputDialog, IDC_Engine2_NPS );
        hMemo  = GetDlgItem( engineOutputDialog, IDC_EngineMemo2 );
        name = second.tidy;
    }

    /* Clear memo if needed */
    if( lastDepth[which] > depth || (lastDepth[which] == depth && depth <= 1) ) {
        SendMessage( hMemo, WM_SETTEXT, 0, (LPARAM) "" );
    }

    /* Update */
    lastDepth[which] = depth;

    if( pv[0] == ' ' ) {
        if( strncmp( pv, " no PV", 6 ) == 0 ) { /* Hack on hack! :-O */
            pv = "";
        }
    }

    UpdateControls( hLabel, hLabelNPS, hMemo, name, depth, nodes, score, time, pv );
}
