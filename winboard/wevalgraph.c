/*
 * Evaluation graph
 *
 * Author: Alessandro Scotti (Dec 2005)
 *
 * Copyright 2005 Alessandro Scotti
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

#include <windows.h> /* required for all Windows applications */
//include <richedit.h>
#include <stdio.h>
//include <stdlib.h>
//include <malloc.h>

#include "common.h"
#include "frontend.h"
#include "backend.h"

/* Imports from winboard.c */
extern BOOLEAN evalGraphDialogUp; // should be back-end variable, and defined here

/* Module globals */ // used to communicate between back-end and front-end part
static ChessProgramStats_Move * currPvInfo;
static int currFirst = 0;
static int currLast = 0;
static int currCurrent = -1;

static int nWidthPB = 0;
static int nHeightPB = 0;

static int MarginX = 18;
static int MarginW = 4;
static int MarginH = 4;

#define MIN_HIST_WIDTH  4
#define MAX_HIST_WIDTH  10

#define PEN_NONE	0
#define PEN_BLACK	1
#define PEN_DOTTED	2
#define PEN_BLUEDOTTED	3
#define PEN_BOLD	4 /* or 5 for black */

#define FILLED 1
#define OPEN   0

// calls from back-end part into front-end
static void DrawSegment( int x, int y, int *lastX, int *lastY, int penType );
void DrawRectangle( int left, int top, int right, int bottom, int side, int style );
void DrawEvalText(char *buf, int cbBuf, int y);


// back-end
static void DrawLine( int x1, int y1, int x2, int y2, int penType )
{
    DrawSegment( x1, y1, NULL, NULL, PEN_NONE );
    DrawSegment( x2, y2, NULL, NULL, penType );
}

// back-end
static void DrawLineEx( int x1, int y1, int x2, int y2, int penType )
{
    int savX, savY;
    DrawSegment( x1, y1, &savX, &savY, PEN_NONE );
    DrawSegment( x2, y2, NULL, NULL, penType );
    DrawSegment( savX, savY, NULL, NULL, PEN_NONE );
}

// back-end
static int GetPvScore( int index )
{
    int score = currPvInfo[ index ].score;

    if( index & 1 ) score = -score; /* Flip score for black */

    return score;
}

// back-end
/*
    For a centipawn value, this function returns the height of the corresponding
    histogram, centered on the reference axis.

    Note: height can be negative!
*/
static int GetValueY( int value )
{
    if( value < -700 ) value = -700;
    if( value > +700 ) value = +700;

    return (nHeightPB / 2) - (int)(value * (nHeightPB - 2*MarginH) / 1400.0);
}

// the brush selection is made part of the DrawLine, by passing a style argument
// the wrapper for doing the text output makes this back-end
static void DrawAxisSegmentHoriz( int value, BOOL drawValue )
{
    int y = GetValueY( value*100 );

    if( drawValue ) {
        char buf[MSG_SIZ], *b = buf;

        if( value > 0 ) *b++ = '+';
	sprintf(b, "%d", value);

	DrawEvalText(buf, strlen(buf), y);
    }
    // [HGM] counts on DrawEvalText to have select transparent background for dotted line!
    DrawLine( MarginX, y, MarginX + MarginW, y, PEN_BLACK ); // Y-axis tick marks
    DrawLine( MarginX + MarginW, y, nWidthPB - MarginW, y, PEN_DOTTED ); // hor grid
}

// The DrawLines again must select their own brush.
// the initial brush selection is useless? BkMode needed for dotted line and text
static void DrawAxis()
{
    int cy = nHeightPB / 2;
    
//    SelectObject( hdcPB, GetStockObject(NULL_BRUSH) );

//    SetBkMode( hdcPB, TRANSPARENT );

    DrawAxisSegmentHoriz( +5, TRUE );
    DrawAxisSegmentHoriz( +3, FALSE );
    DrawAxisSegmentHoriz( +1, FALSE );
    DrawAxisSegmentHoriz(  0, TRUE );
    DrawAxisSegmentHoriz( -1, FALSE );
    DrawAxisSegmentHoriz( -3, FALSE );
    DrawAxisSegmentHoriz( -5, TRUE );

    DrawLine( MarginX + MarginW, cy, nWidthPB - MarginW, cy, PEN_BLACK ); // x-axis
    DrawLine( MarginX + MarginW, MarginH, MarginX + MarginW, nHeightPB - MarginH, PEN_BLACK ); // y-axis
}

// back-end
static void DrawHistogram( int x, int y, int width, int value, int side )
{
    int left, top, right, bottom;

    if( value > -25 && value < +25 ) return;

    left = x;
    right = left + width + 1;

    if( value > 0 ) {
        top = GetValueY( value );
        bottom = y+1;
    }
    else {
        top = y;
        bottom = GetValueY( value ) + 1;
    }


    if( width == MIN_HIST_WIDTH ) {
        right--;
        DrawRectangle( left, top, right, bottom, side, FILLED );
    }
    else {
        DrawRectangle( left, top, right, bottom, side, OPEN );
    }
}

// back-end
static void DrawSeparator( int index, int x )
{
    if( index > 0 ) {
        if( index == currCurrent ) {
            DrawLineEx( x, MarginH, x, nHeightPB - MarginH, PEN_BLUEDOTTED );
        }
        else if( (index % 20) == 0 ) {
            DrawLineEx( x, MarginH, x, nHeightPB - MarginH, PEN_DOTTED );
        }
    }
}

// made back-end by replacing MoveToEx and LineTo by DrawSegment
/* Actually draw histogram as a diagram, cause there's too much data */
static void DrawHistogramAsDiagram( int cy, int paint_width, int hist_count )
{
    double step;
    int i;

    /* Rescale the graph every few moves (as opposed to every move) */
    hist_count -= hist_count % 8;
    hist_count += 8;
    hist_count /= 2;

    step = (double) paint_width / (hist_count + 1);

    for( i=0; i<2; i++ ) {
        int index = currFirst;
        int side = (currCurrent + i + 1) & 1; /* Draw current side last */
        double x = MarginX + MarginW;

        if( (index & 1) != side ) {
            x += step / 2;
            index++;
        }

        DrawSegment( (int) x, cy, NULL, NULL, PEN_NONE );

        index += 2;

        while( index < currLast ) {
            x += step;

            DrawSeparator( index, (int) x );

            /* Extend line up to current point */
            if( currPvInfo[index].depth > 0 ) {
                DrawSegment((int) x, GetValueY( GetPvScore(index) ), NULL, NULL, PEN_BOLD + side );
            }

            index += 2;
        }
    }
}

// back-end, delete pen selection
static void DrawHistogramFull( int cy, int hist_width, int hist_count )
{
    int i;

//    SelectObject( hdcPB, GetStockObject(BLACK_PEN) );

    for( i=0; i<hist_count; i++ ) {
        int index = currFirst + i;
        int x = MarginX + MarginW + index * hist_width;

        /* Draw a separator every 10 moves */
        DrawSeparator( index, x );

        /* Draw histogram */
        if( currPvInfo[i].depth > 0 ) {
            DrawHistogram( x, cy, hist_width, GetPvScore(index), index & 1 );
        }
    }
}

typedef struct {
    int cy;
    int hist_width;
    int hist_count;
    int paint_width;
} VisualizationData;

// back-end
static Boolean InitVisualization( VisualizationData * vd )
{
    BOOL result = FALSE;

    vd->cy = nHeightPB / 2;
    vd->hist_width = MIN_HIST_WIDTH;
    vd->hist_count = currLast - currFirst;
    vd->paint_width = nWidthPB - MarginX - 2*MarginW;

    if( vd->hist_count > 0 ) {
        result = TRUE;

        /* Compute width */
        vd->hist_width = vd->paint_width / vd->hist_count;

        if( vd->hist_width > MAX_HIST_WIDTH ) vd->hist_width = MAX_HIST_WIDTH;

        vd->hist_width -= vd->hist_width % 2;
    }

    return result;
}

// back-end
static void DrawHistograms()
{
    VisualizationData vd;

    if( InitVisualization( &vd ) ) {
        if( vd.hist_width < MIN_HIST_WIDTH ) {
            DrawHistogramAsDiagram( vd.cy, vd.paint_width, vd.hist_count );
        }
        else {
            DrawHistogramFull( vd.cy, vd.hist_width, vd.hist_count );
        }
    }
}

// back-end
int GetMoveIndexFromPoint( int x, int y )
{
    int result = -1;
    int start_x = MarginX + MarginW;
    VisualizationData vd;

    if( x >= start_x && InitVisualization( &vd ) ) {
        /* Almost an hack here... we duplicate some of the paint logic */
        if( vd.hist_width < MIN_HIST_WIDTH ) {
            double step;

            vd.hist_count -= vd.hist_count % 8;
            vd.hist_count += 8;
            vd.hist_count /= 2;

            step = (double) vd.paint_width / (vd.hist_count + 1);
            step /= 2;

            result = (int) (0.5 + (double) (x - start_x) / step);
        }
        else {
            result = (x - start_x) / vd.hist_width;
        }
    }

    if( result >= currLast ) {
        result = -1;
    }

    return result;
}

// init and display part split of so they can be moved to front end
void PaintEvalGraph( void )
{
    /* Draw */
    DrawRectangle(0, 0, nWidthPB, nHeightPB, 2, FILLED);
    DrawAxis();
    DrawHistograms();
}

Boolean EvalGraphIsUp()
{
    return evalGraphDialogUp;
}

// ------------------------------------------ front-end starts here ----------------------------------------------

#include <commdlg.h>
#include <dlgs.h>

#include "winboard.h"
#include "wsnap.h"

#define WM_REFRESH_GRAPH    (WM_USER + 1)

void EvalGraphSet( int first, int last, int current, ChessProgramStats_Move * pvInfo );
void EvalGraphPopUp();
void EvalGraphPopDown();
Boolean EvalGraphIsUp();

// calls of front-end part into back-end part
extern int GetMoveIndexFromPoint( int x, int y );
extern void PaintEvalGraph( void );

/* Imports from winboard.c */
extern HWND evalGraphDialog;
extern BOOLEAN evalGraphDialogUp; // should be back-end, really

extern HINSTANCE hInst;
extern HWND hwndMain;

extern WindowPlacement wpEvalGraph;

static COLORREF crWhite = RGB( 0xFF, 0xFF, 0xB0 );
static COLORREF crBlack = RGB( 0xAD, 0x5D, 0x3D );

static HDC hdcPB = NULL;
static HBITMAP hbmPB = NULL;
static HPEN pens[6]; // [HGM] put all pens in one array
static HBRUSH hbrHist[3] = { NULL, NULL, NULL };

// [HGM] front-end, added as wrapper to avoid use of LineTo and MoveToEx in other routines (so they can be back-end) 
static void DrawSegment( int x, int y, int *lastX, int *lastY, int penType )
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
	    pens[PEN_BLACK]     = GetStockObject(BLACK_PEN);
            pens[PEN_DOTTED]    = CreatePen( PS_DOT, 0, RGB(0xA0,0xA0,0xA0) );
            pens[PEN_BLUEDOTTED] = CreatePen( PS_DOT, 0, RGB(0x00,0x00,0xFF) );
            pens[PEN_BOLD]      = CreatePen( PS_SOLID, 2, crWhite );
            pens[PEN_BOLD+1]    = CreatePen( PS_SOLID, 2, crBlack );
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

    case WM_LBUTTONDBLCLK:
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
#if 0
    case WM_NCLBUTTONDBLCLK:
        if( wParam == HTCAPTION ) {
            int index;
            POINT mouse_xy;
            POINTS pts = MAKEPOINTS(lParam);

            mouse_xy.x = pts.x;
            mouse_xy.y = pts.y;
            ScreenToClient( hDlg, &mouse_xy );

            index = GetMoveIndexFromPoint( mouse_xy.x, mouse_xy.y );

            if( index >= 0 && index < currLast ) {
                ToNrEvent( index + 1 );
            }
        }
        break;

    case WM_NCHITTEST:
        {
            LRESULT res = DefWindowProc( hDlg, message, wParam, lParam );

            if( res == HTCLIENT ) res = HTCAPTION;

            SetWindowLong( hDlg, DWL_MSGRESULT, res );

            return TRUE;
        }
        break;
#endif

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

