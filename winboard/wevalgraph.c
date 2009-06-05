/*
 * Evaluation graph
 *
 * Author: Alessandro Scotti (Dec 2005)
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
#include "winboard.h"
#include "frontend.h"
#include "backend.h"

#include "wsnap.h"

VOID EvalGraphSet( int first, int last, int current, ChessProgramStats_Move * pvInfo );
VOID EvalGraphPopUp();
VOID EvalGraphPopDown();
BOOL EvalGraphIsUp();

#define WM_REFRESH_GRAPH    (WM_USER + 1)

/* Imports from backend.c */
char * SavePart(char *str);

/* Imports from winboard.c */
extern HWND evalGraphDialog;
extern BOOLEAN evalGraphDialogUp;

extern HINSTANCE hInst;
extern HWND hwndMain;

extern WindowPlacement wpEvalGraph;

/* Module globals */
static ChessProgramStats_Move * currPvInfo;
static int currFirst = 0;
static int currLast = 0;
static int currCurrent = -1;

static COLORREF crWhite = RGB( 0xFF, 0xFF, 0xB0 );
static COLORREF crBlack = RGB( 0xAD, 0x5D, 0x3D );

static HDC hdcPB = NULL;
static HBITMAP hbmPB = NULL;
static int nWidthPB = 0;
static int nHeightPB = 0;
static HPEN hpenDotted = NULL;
static HPEN hpenBlueDotted = NULL;
static HPEN hpenBold[2] = { NULL, NULL };
static HBRUSH hbrHist[2] = { NULL, NULL };

static int MarginX = 18;
static int MarginW = 4;
static int MarginH = 4;

#define MIN_HIST_WIDTH  4
#define MAX_HIST_WIDTH  10

static int GetPvScore( int index )
{
    int score = currPvInfo[ index ].score;

    if( index & 1 ) score = -score; /* Flip score for black */

    return score;
}

static VOID DrawLine( int x1, int y1, int x2, int y2 )
{
    MoveToEx( hdcPB, x1, y1, NULL );

    LineTo( hdcPB, x2, y2 );
}

static VOID DrawLineEx( int x1, int y1, int x2, int y2 )
{
    POINT stPT;

    MoveToEx( hdcPB, x1, y1, &stPT );

    LineTo( hdcPB, x2, y2 );

    MoveToEx( hdcPB, stPT.x, stPT.y, NULL );
}

static HBRUSH CreateBrush( UINT style, COLORREF color )
{
    LOGBRUSH stLB;

    stLB.lbStyle = style;
    stLB.lbColor = color;
    stLB.lbHatch = 0;

    return CreateBrushIndirect( &stLB );
}

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

static VOID DrawAxisSegmentHoriz( int value, BOOL drawValue )
{
    int y = GetValueY( value*100 );

    SelectObject( hdcPB, GetStockObject(BLACK_PEN) );
    DrawLine( MarginX, y, MarginX + MarginW, y );
    SelectObject( hdcPB, hpenDotted );
    DrawLine( MarginX + MarginW, y, nWidthPB - MarginW, y );

    if( drawValue ) {
        SIZE stSize;
        char buf[MSG_SIZ], *b = buf;
        int cbBuf;

        if( value > 0 ) *b++ = '+';
	sprintf(b, "%d", value);

        cbBuf = strlen( buf );
        GetTextExtentPoint32( hdcPB, buf, cbBuf, &stSize );
        TextOut( hdcPB, MarginX - stSize.cx - 2, y - stSize.cy / 2, buf, cbBuf );
    }
}

static VOID DrawAxis()
{
    int cy = nHeightPB / 2;
    
    SelectObject( hdcPB, GetStockObject(NULL_BRUSH) );

    SetBkMode( hdcPB, TRANSPARENT );

    DrawAxisSegmentHoriz( +5, TRUE );
    DrawAxisSegmentHoriz( +3, FALSE );
    DrawAxisSegmentHoriz( +1, FALSE );
    DrawAxisSegmentHoriz(  0, TRUE );
    DrawAxisSegmentHoriz( -1, FALSE );
    DrawAxisSegmentHoriz( -3, FALSE );
    DrawAxisSegmentHoriz( -5, TRUE );

    SelectObject( hdcPB, GetStockObject(BLACK_PEN) );

    DrawLine( MarginX + MarginW, cy, nWidthPB - MarginW, cy );
    DrawLine( MarginX + MarginW, MarginH, MarginX + MarginW, nHeightPB - MarginH );
}

static VOID DrawHistogram( int x, int y, int width, int value, int side )
{
    RECT rc;

    if( value > -25 && value < +25 ) return;

    rc.left = x;
    rc.right = rc.left + width + 1;

    if( value > 0 ) {
        rc.top = GetValueY( value );
        rc.bottom = y+1;
    }
    else {
        rc.top = y;
        rc.bottom = GetValueY( value ) + 1;
    }


    if( width == MIN_HIST_WIDTH ) {
        rc.right--;
        FillRect( hdcPB, &rc, hbrHist[side] );
    }
    else {
        SelectObject( hdcPB, hbrHist[side] );
        Rectangle( hdcPB, rc.left, rc.top, rc.right, rc.bottom );
    }
}

static VOID DrawSeparator( int index, int x )
{
    if( index > 0 ) {
        if( index == currCurrent ) {
            HPEN hp = SelectObject( hdcPB, hpenBlueDotted );
            DrawLineEx( x, MarginH, x, nHeightPB - MarginH );
            SelectObject( hdcPB, hp );
        }
        else if( (index % 20) == 0 ) {
            HPEN hp = SelectObject( hdcPB, hpenDotted );
            DrawLineEx( x, MarginH, x, nHeightPB - MarginH );
            SelectObject( hdcPB, hp );
        }
    }
}

/* Actually draw histogram as a diagram, cause there's too much data */
static VOID DrawHistogramAsDiagram( int cy, int paint_width, int hist_count )
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

        SelectObject( hdcPB, hpenBold[side] );

        MoveToEx( hdcPB, (int) x, cy, NULL );

        index += 2;

        while( index < currLast ) {
            x += step;

            DrawSeparator( index, (int) x );

            /* Extend line up to current point */
            if( currPvInfo[index].depth > 0 ) {
                LineTo( hdcPB, (int) x, GetValueY( GetPvScore(index) ) );
            }

            index += 2;
        }
    }
}

static VOID DrawHistogramFull( int cy, int hist_width, int hist_count )
{
    int i;

    SelectObject( hdcPB, GetStockObject(BLACK_PEN) );

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

static BOOL InitVisualization( VisualizationData * vd )
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

static VOID DrawHistograms()
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

static int GetMoveIndexFromPoint( int x, int y )
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

static VOID DrawBackground()
{
    HBRUSH hbr;
    RECT rc;

    hbr = CreateBrush( BS_SOLID, GetSysColor( COLOR_3DFACE ) );

    rc.left = 0;
    rc.top = 0;
    rc.right = nWidthPB;
    rc.bottom = nHeightPB;

    FillRect( hdcPB, &rc, hbr );

    DeleteObject( hbr );
}

static VOID PaintEvalGraph( HWND hWnd, HDC hDC )
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
        if( hpenDotted == NULL ) {
            hpenDotted = CreatePen( PS_DOT, 0, RGB(0xA0,0xA0,0xA0) );
            hpenBlueDotted = CreatePen( PS_DOT, 0, RGB(0x00,0x00,0xFF) );
            hpenBold[0] = CreatePen( PS_SOLID, 2, crWhite );
            hpenBold[1] = CreatePen( PS_SOLID, 2, crBlack );
            hbrHist[0] = CreateBrush( BS_SOLID, crWhite );
            hbrHist[1] = CreateBrush( BS_SOLID, crBlack );
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

    /* Draw */
    DrawBackground();
    DrawAxis();
    DrawHistograms();

    /* Copy bitmap into destination DC */
    BitBlt( hDC, 0, 0, width, height, hdcPB, 0, 0, SRCCOPY );
}

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
        PaintEvalGraph( hDlg, hDC );
        EndPaint( hDlg, &stPS );
        break;

    case WM_REFRESH_GRAPH:
        hDC = GetDC( hDlg );
        PaintEvalGraph( hDlg, hDC );
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

VOID EvalGraphPopDown()
{
  CheckMenuItem(GetMenu(hwndMain), IDM_ShowEvalGraph, MF_UNCHECKED);

  if( evalGraphDialog ) {
      ShowWindow(evalGraphDialog, SW_HIDE);
  }

  evalGraphDialogUp = FALSE;
}

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

BOOL EvalGraphIsUp()
{
    return evalGraphDialogUp;
}
