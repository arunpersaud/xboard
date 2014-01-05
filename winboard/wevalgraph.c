/*
 * wevalgraph.c - Evaluation graph front-end part
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

// code refactored by HGM to obtain front-end / back-end separation

#include "config.h"

#include <windows.h>
#include <commdlg.h>
#include <dlgs.h>
#include <stdio.h>

#include "common.h"
#include "frontend.h"
#include "backend.h"
#include "winboard.h"
#include "evalgraph.h"
#include "wsnap.h"

#define WM_REFRESH_GRAPH    (WM_USER + 1)

/* Module globals */
static BOOLEAN evalGraphDialogUp;

static COLORREF crWhite = RGB( 0xFF, 0xFF, 0xB0 );
static COLORREF crBlack = RGB( 0xAD, 0x5D, 0x3D );

static HDC hdcPB = NULL;
static HBITMAP hbmPB = NULL;
static HPEN pens[6]; // [HGM] put all pens in one array
static HBRUSH hbrHist[3] = { NULL, NULL, NULL };

Boolean EvalGraphIsUp()
{
    return evalGraphDialogUp;
}

// [HGM] front-end, added as wrapper to avoid use of LineTo and MoveToEx in other routines (so they can be back-end) 
void DrawSegment( int x, int y, int *lastX, int *lastY, int penType )
{
    POINT stPt;
    if(penType == PEN_NONE) MoveToEx( hdcPB, x, y, &stPt ); else {
	HPEN hp = SelectObject( hdcPB, pens[penType] );
	LineTo( hdcPB, x, y );
	SelectObject( hdcPB, hp );
    }
    if(lastX != NULL) { *lastX = stPt.x; *lastY = stPt.y; }
}

// front-end wrapper for drawing functions to do rectangles
void DrawRectangle( int left, int top, int right, int bottom, int side, int style )
{
    HPEN hp = SelectObject( hdcPB, pens[PEN_BLACK] );
    RECT rc;

    rc.top = top; rc.left = left; rc.bottom = bottom; rc.right = right;
    if(style == FILLED)
        FillRect( hdcPB, &rc, hbrHist[side] );
    else {
        SelectObject( hdcPB, hbrHist[side] );
        Rectangle( hdcPB, left, top, right, bottom );
    }
    SelectObject( hdcPB, hp );
}

// front-end wrapper for putting text in graph
void DrawEvalText(char *buf, int cbBuf, int y)
{
        SIZE stSize;
	SetBkMode( hdcPB, TRANSPARENT );
        GetTextExtentPoint32( hdcPB, buf, cbBuf, &stSize );
        TextOut( hdcPB, MarginX - stSize.cx - 2, y - stSize.cy / 2, buf, cbBuf );
}

// front-end
static HBRUSH CreateBrush( UINT style, COLORREF color )
{
    LOGBRUSH stLB;

    stLB.lbStyle = style;
    stLB.lbColor = color;
    stLB.lbHatch = 0;

    return CreateBrushIndirect( &stLB );
}

// front-end. Create pens, device context and buffer bitmap for global use, copy result to display
// The back-end part n the middle has been taken out and moed to PainEvalGraph()
static VOID DisplayEvalGraph( HWND hWnd, HDC hDC )
{
    RECT rcClient;
    int width;
    int height;

    /* Get client area */
    GetClientRect( hWnd, &rcClient );

    width = rcClient.right - rcClient.left;
    height = rcClient.bottom - rcClient.top;

    /* Create or recreate paint box if needed */
    if( hbmPB == NULL || width != nWidthPB || height != nHeightPB ) {
        if( pens[PEN_DOTTED] == NULL ) {
	    pens[PEN_BLACK]      = GetStockObject(BLACK_PEN);
            pens[PEN_DOTTED]     = CreatePen( PS_DOT, 0, RGB(0xA0,0xA0,0xA0) );
            pens[PEN_BLUEDOTTED] = CreatePen( PS_DOT, 0, RGB(0x00,0x00,0xFF) );
            pens[PEN_BOLDWHITE]  = CreatePen( PS_SOLID, 2, crWhite );
            pens[PEN_BOLDBLACK]  = CreatePen( PS_SOLID, 2, crBlack );
            hbrHist[0] = CreateBrush( BS_SOLID, crWhite );
            hbrHist[1] = CreateBrush( BS_SOLID, crBlack );
            hbrHist[2] = CreateBrush( BS_SOLID, GetSysColor( COLOR_3DFACE ) ); // background
        }

        if( hdcPB != NULL ) {
            DeleteDC( hdcPB );
            hdcPB = NULL;
        }

        if( hbmPB != NULL ) {
            DeleteObject( hbmPB );
            hbmPB = NULL;
        }

        hdcPB = CreateCompatibleDC( hDC );

        nWidthPB = width;
        nHeightPB = height;
        hbmPB = CreateCompatibleBitmap( hDC, nWidthPB, nHeightPB );

        SelectObject( hdcPB, hbmPB );
    }

    // back-end painting; calls back front-end primitives for lines, rectangles and text
    PaintEvalGraph();
    SetWindowText(hWnd, MakeEvalTitle(T_("Evaluation Graph")));

    /* Copy bitmap into destination DC */
    BitBlt( hDC, 0, 0, nWidthPB, nHeightPB, hdcPB, 0, 0, SRCCOPY );
}

// Note: Once the eval graph is opened, this window-proc lives forever; een closing the
// eval-graph window merely hides it. On opening we re-initialize it, though, so it could
// as well hae been destroyed. While it is open it processes the REFRESH_GRAPH commands.
LRESULT CALLBACK EvalGraphProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
    static SnapData sd;

    PAINTSTRUCT stPS;
    HDC hDC;

    switch (message) {
    case WM_INITDIALOG:
        Translate(hDlg, DLG_EvalGraph);
        if( evalGraphDialog == NULL ) {
            evalGraphDialog = hDlg;

            RestoreWindowPlacement( hDlg, &wpEvalGraph ); /* Restore window placement */
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

    case WM_ERASEBKGND:
        return TRUE;

    case WM_PAINT:
        hDC = BeginPaint( hDlg, &stPS );
        DisplayEvalGraph( hDlg, hDC );
        EndPaint( hDlg, &stPS );
        break;

    case WM_REFRESH_GRAPH:
        hDC = GetDC( hDlg );
        DisplayEvalGraph( hDlg, hDC );
        ReleaseDC( hDlg, hDC );
        break;

    case WM_LBUTTONDOWN:
        if( wParam == 0 || wParam == MK_LBUTTON ) {
            int index = GetMoveIndexFromPoint( LOWORD(lParam), HIWORD(lParam) );

            if( index >= 0 && index < currLast ) {
                ToNrEvent( index + 1 );
            }
        }
        return TRUE;

    case WM_SIZE:
        InvalidateRect( hDlg, NULL, FALSE );
        break;

    case WM_GETMINMAXINFO:
        {
            MINMAXINFO * mmi = (MINMAXINFO *) lParam;
        
            mmi->ptMinTrackSize.x = 100;
            mmi->ptMinTrackSize.y = 100;
        }
        break;

    /* Support for captionless window */
    case WM_CLOSE:
        EvalGraphPopDown();
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

// creates the eval graph, or unhides it.
VOID EvalGraphPopUp()
{
  FARPROC lpProc;
  
  CheckMenuItem(GetMenu(hwndMain), IDM_ShowEvalGraph, MF_CHECKED);

  if( evalGraphDialog ) {
    SendMessage( evalGraphDialog, WM_INITDIALOG, 0, 0 );

    if( ! evalGraphDialogUp ) {
        ShowWindow(evalGraphDialog, SW_SHOW);
    }
  }
  else {
    crWhite = appData.evalHistColorWhite;
    crBlack = appData.evalHistColorBlack;

    lpProc = MakeProcInstance( (FARPROC) EvalGraphProc, hInst );

    /* Note to self: dialog must have the WS_VISIBLE style set, otherwise it's not shown! */
    CreateDialog( hInst, MAKEINTRESOURCE(DLG_EvalGraph), hwndMain, (DLGPROC)lpProc );

    FreeProcInstance(lpProc);
  }

  evalGraphDialogUp = TRUE;
}

// Note that this hides the window. It could as well have destroyed it.
VOID EvalGraphPopDown()
{
  CheckMenuItem(GetMenu(hwndMain), IDM_ShowEvalGraph, MF_UNCHECKED);

  if( evalGraphDialog ) {
      ShowWindow(evalGraphDialog, SW_HIDE);
  }

  evalGraphDialogUp = FALSE;
}

// This function is the interface to the back-end. It is currently called through the front-end,
// though, where it shares the HistorySet() wrapper with MoveHistorySet(). Once all front-ends
// support the eval graph, it would be more logical to call it directly from the back-end.
VOID EvalGraphSet( int first, int last, int current, ChessProgramStats_Move * pvInfo )
{
    /* [AS] Danger! For now we rely on the pvInfo parameter being a static variable! */

    currFirst = first;
    currLast = last;
    currCurrent = current;
    currPvInfo = pvInfo;

    if( evalGraphDialog ) {
        SendMessage( evalGraphDialog, WM_REFRESH_GRAPH, 0, 0 );
    }
}
