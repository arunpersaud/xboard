/*
 * wsnap.h -- Smart "snapping" for window moving and sizing
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
#ifndef WSNAP_H_
#define WSNAP_H_

#include <windows.h>

#define MAX_SNAP_POINTS     12

#define SNAP_DISTANCE       4

typedef struct {
    int x_grid[ MAX_SNAP_POINTS ];
    int x_grid_len;
    int y_grid[ MAX_SNAP_POINTS ];
    int y_grid_len;
} SnapData;

LRESULT OnEnterSizeMove( SnapData * snapData, HWND hWnd, WPARAM wParam, LPARAM lParam );
LRESULT OnMoving( SnapData * snapData, HWND hWnd, WPARAM wParam, LPARAM lParam );
LRESULT OnSizing( SnapData * snapData, HWND hWnd, WPARAM wParam, LPARAM lParam );
LRESULT OnExitSizeMove( SnapData * snapData, HWND hWnd, WPARAM wParam, LPARAM lParam );

#endif // WSNAP_H_
