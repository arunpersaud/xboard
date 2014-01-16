/*
 * Smart "snapping" for window moving and sizing
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

#include "wsnap.h"

/* Imports from winboard.c */
extern HINSTANCE hInst;

extern HWND hwndMain;
extern HWND moveHistoryDialog;
extern HWND evalGraphDialog;
extern HWND engineOutputDialog;
extern HWND gameListDialog;

static BOOL SnappingEnabled = TRUE;

static void AddSnapPoint( int * grid, int * grid_len, int value )
{
    int len = *grid_len;

    if( len < MAX_SNAP_POINTS ) {
        int i;

        for( i=0; i<len; i++ ) {
            if( grid[i] == value ) {
                return;
            }
        }

        grid[ len++ ] = value;

        *grid_len = len;
    }
}

static void AddSnapRectangle( SnapData * sd, RECT * rc )
{
    AddSnapPoint( sd->x_grid, &sd->x_grid_len, rc->left );
    AddSnapPoint( sd->x_grid, &sd->x_grid_len, rc->right );

    AddSnapPoint( sd->y_grid, &sd->y_grid_len, rc->top );
    AddSnapPoint( sd->y_grid, &sd->y_grid_len, rc->bottom );
}

static RECT activeRect, mainRect;
static int side, loc; // code for edge we were dragging, and its latest coordinate

static void AddSnapWindow( HWND hWndCaller, SnapData * sd, HWND hWndSnapWindow )
{
    if( hWndSnapWindow != NULL && IsWindowVisible(hWndSnapWindow) ) {
        RECT rc;

        GetWindowRect( hWndSnapWindow, &rc );
	if(hWndSnapWindow == hwndMain) mainRect = rc;

	if(hWndCaller != hWndSnapWindow) {
            AddSnapRectangle( sd, &rc );
	} else {
	    activeRect = rc; // [HGM] glue: remember original geometry of dragged window
	}
    }
}

static BOOL AdjustToSnapPoint( int * grid, int grid_len, int value, int * snap_size, int * delta )
{
    BOOL result = FALSE;
    int i;

    for( i=0; i<grid_len; i++ ) {
        int distance = value - grid[i];

        if( distance < 0 ) distance = -distance;

        if( distance < *snap_size ) {
            result = TRUE;
            *snap_size = distance;
            *delta = grid[i] - value;
        }
    }

    return result;
}

LRESULT OnEnterSizeMove( SnapData * snapData, HWND hWnd, WPARAM wParam, LPARAM lParam )
{
    RECT rc;

    snapData->x_grid_len = 0;
    snapData->y_grid_len = 0;
    side = 0;

    /* Add desktop area */
    if( SystemParametersInfo( SPI_GETWORKAREA, 0, &rc, 0 ) ) {
        AddSnapRectangle( snapData, &rc );
    }

    if( hWnd != hwndMain ) {
        /* Add other windows */
        AddSnapWindow( hWnd, snapData, hwndMain );
        AddSnapWindow( hWnd, snapData, moveHistoryDialog );
        AddSnapWindow( hWnd, snapData, evalGraphDialog );
        AddSnapWindow( hWnd, snapData, engineOutputDialog );
        AddSnapWindow( hWnd, snapData, gameListDialog );
    }

    return 0;
}

LRESULT OnMoving( SnapData * snapData, HWND hWnd, WPARAM wParam, LPARAM lParam )
{
    LPRECT lprc = (LPRECT) lParam;
    int delta_x = 0;
    int delta_y = 0;
    int snap_size_x = SNAP_DISTANCE;
    int snap_size_y = SNAP_DISTANCE;

    if( ! SnappingEnabled ) {
        return FALSE;
    }

    AdjustToSnapPoint( snapData->x_grid, snapData->x_grid_len, lprc->left, &snap_size_x, &delta_x );
    AdjustToSnapPoint( snapData->x_grid, snapData->x_grid_len, lprc->right, &snap_size_x, &delta_x );

    AdjustToSnapPoint( snapData->y_grid, snapData->y_grid_len, lprc->top, &snap_size_y, &delta_y );
    AdjustToSnapPoint( snapData->y_grid, snapData->y_grid_len, lprc->bottom, &snap_size_y, &delta_y );

    OffsetRect( lprc, delta_x, delta_y );

    return TRUE;
}

LRESULT OnSizing( SnapData * snapData, HWND hWnd, WPARAM wParam, LPARAM lParam )
{
    LPRECT lprc = (LPRECT) lParam;
    int delta_x = 0;
    int delta_y = 0;
    int snap_size_x = SNAP_DISTANCE;
    int snap_size_y = SNAP_DISTANCE;

    if( ! SnappingEnabled ) {
        return FALSE;
    }

    switch( wParam ) {
    case WMSZ_BOTTOM:
        AdjustToSnapPoint( snapData->y_grid, snapData->y_grid_len, lprc->bottom, &snap_size_y, &delta_y );
        lprc->bottom += delta_y; side = 4; loc = lprc->bottom;
        break;
    case WMSZ_BOTTOMLEFT:
        AdjustToSnapPoint( snapData->y_grid, snapData->y_grid_len, lprc->bottom, &snap_size_y, &delta_y );
        lprc->bottom += delta_y;
        AdjustToSnapPoint( snapData->x_grid, snapData->x_grid_len, lprc->left, &snap_size_x, &delta_x );
        lprc->left += delta_x;
        break;
    case WMSZ_BOTTOMRIGHT:
        AdjustToSnapPoint( snapData->y_grid, snapData->y_grid_len, lprc->bottom, &snap_size_y, &delta_y );
        lprc->bottom += delta_y;
        AdjustToSnapPoint( snapData->x_grid, snapData->x_grid_len, lprc->right, &snap_size_x, &delta_x );
        lprc->right += delta_x;
        break;
    case WMSZ_LEFT:
        AdjustToSnapPoint( snapData->x_grid, snapData->x_grid_len, lprc->left, &snap_size_x, &delta_x );
        lprc->left += delta_x; side = 1; loc = lprc->left;
        break;
    case WMSZ_RIGHT:
        AdjustToSnapPoint( snapData->x_grid, snapData->x_grid_len, lprc->right, &snap_size_x, &delta_x );
        lprc->right += delta_x; side = 2; loc = lprc->right;
        break;
    case WMSZ_TOP:
        AdjustToSnapPoint( snapData->y_grid, snapData->y_grid_len, lprc->top, &snap_size_y, &delta_y );
        lprc->top += delta_y; side = 3; loc = lprc->top;
        break;
    case WMSZ_TOPLEFT:
        AdjustToSnapPoint( snapData->y_grid, snapData->y_grid_len, lprc->top, &snap_size_y, &delta_y );
        lprc->top += delta_y;
        AdjustToSnapPoint( snapData->x_grid, snapData->x_grid_len, lprc->left, &snap_size_x, &delta_x );
        lprc->left += delta_x;
        break;
    case WMSZ_TOPRIGHT:
        AdjustToSnapPoint( snapData->y_grid, snapData->y_grid_len, lprc->top, &snap_size_y, &delta_y );
        lprc->top += delta_y;
        AdjustToSnapPoint( snapData->x_grid, snapData->x_grid_len, lprc->right, &snap_size_x, &delta_x );
        lprc->right += delta_x;
        break;
    default:
        return FALSE;
    }

    return TRUE;
}

static int Adjust( LONG *data, int new, int old , int vertical)
{
    // protect edges that also touch main window
    if(!vertical && (old == mainRect.left || old == mainRect.right))  return 0;
    if( vertical && (old == mainRect.top  || old == mainRect.bottom)) return 0;
    // if the coordinate was the same as the old, now make it the same as the new edge position
    if(*data == old) { *data = new; return 1; }
    return 0;
}

static void KeepTouching( int side, int new, int old, HWND hWnd )
{   // if the mentioned window was touching on the moved edge, move its touching edge too
    if( IsWindowVisible(hWnd) ) {
        RECT rc;
	int i = 0;

        GetWindowRect( hWnd, &rc );

	switch(side) { // figure out which edge we might need to drag along (if any)
	  case 1: i = Adjust(&rc.right,  new, old, 0); break;
	  case 2: i = Adjust(&rc.left,   new, old, 0); break;
	  case 3: i = Adjust(&rc.bottom, new, old, 1); break;
	  case 4: i = Adjust(&rc.top,    new, old, 1); break;
	}

	if(i) { // the correct edge was touching, and is adjusted
	    SetWindowPos(hWnd, HWND_TOP, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOZORDER );
	}
    }
}

LRESULT OnExitSizeMove( SnapData * snapData, HWND hWnd, WPARAM wParam, LPARAM lParam )
{
    if(side && hWnd != hwndMain) { // [HGM] glue: we have been sizing, by dragging an edge
	int *grid = (side > 2 ? snapData->y_grid : snapData->x_grid);
	int i, pos = -1, len = (side > 2 ? snapData->y_grid_len : snapData->x_grid_len);

	switch(side) {
	  case 1: pos = activeRect.left; break;
	  case 2: pos = activeRect.right; break;
	  case 3: pos = activeRect.top; break;
	  case 4: pos = activeRect.bottom; break;
	}

	for(i=0; i<len; i++) {
	    if(grid[i] == pos) break; // the dragged side originally touched another auxiliary window
	}

	if(i < len) { // we were touching another sticky window: figure out how, and adapt it if needed
		KeepTouching(side, loc, pos, moveHistoryDialog);
		KeepTouching(side, loc, pos, evalGraphDialog);
		KeepTouching(side, loc, pos, engineOutputDialog);
		KeepTouching(side, loc, pos, gameListDialog);
	}
    }
    return 0;
}
