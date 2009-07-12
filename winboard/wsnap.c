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

static void AddSnapWindow( HWND hWndCaller, SnapData * sd, HWND hWndSnapWindow )
{
    if( hWndSnapWindow != NULL && hWndCaller != hWndSnapWindow && IsWindowVisible(hWndSnapWindow) ) {
        RECT rc;

        GetWindowRect( hWndSnapWindow, &rc );

        AddSnapRectangle( sd, &rc );
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
        lprc->bottom += delta_y;
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
        lprc->left += delta_x;
        break;
    case WMSZ_RIGHT:
        AdjustToSnapPoint( snapData->x_grid, snapData->x_grid_len, lprc->right, &snap_size_x, &delta_x );
        lprc->right += delta_x;
        break;
    case WMSZ_TOP:
        AdjustToSnapPoint( snapData->y_grid, snapData->y_grid_len, lprc->top, &snap_size_y, &delta_y );
        lprc->top += delta_y;
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

LRESULT OnExitSizeMove( SnapData * snapData, HWND hWnd, WPARAM wParam, LPARAM lParam )
{
    return 0;
}
