/*
 * Layout management
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

#include "config.h"

#include <windows.h> /* required for all Windows applications */
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <commdlg.h>
#include <dlgs.h>

#include "common.h"
#include "frontend.h"
#include "winboard.h"

VOID RestoreWindowPlacement( HWND hWnd, WindowPlacement * wp )
{
    if( wp->x != CW_USEDEFAULT || 
        wp->y != CW_USEDEFAULT ||
        wp->width != CW_USEDEFAULT || 
        wp->height != CW_USEDEFAULT )
    {
	WINDOWPLACEMENT stWP;

        ZeroMemory( &stWP, sizeof(stWP) );

	EnsureOnScreen( &wp->x, &wp->y, 0, 0);

	stWP.length = sizeof(stWP);
	stWP.flags = 0;
	stWP.showCmd = SW_SHOW;
	stWP.ptMaxPosition.x = 0;
        stWP.ptMaxPosition.y = 0;
	stWP.rcNormalPosition.left = wp->x;
	stWP.rcNormalPosition.right = wp->x + wp->width;
	stWP.rcNormalPosition.top = wp->y;
	stWP.rcNormalPosition.bottom = wp->y + wp->height;

	SetWindowPlacement(hWnd, &stWP);
    }
}

VOID InitWindowPlacement( WindowPlacement * wp )
{
    wp->visible = TRUE;
    wp->x = CW_USEDEFAULT;
    wp->y = CW_USEDEFAULT;
    wp->width = CW_USEDEFAULT;
    wp->height = CW_USEDEFAULT;
}

static BOOL IsAttachDistance( int a, int b )
{
    BOOL result = FALSE;

    if( a == b ) {
        result = TRUE;
    }

    return result;
}

static BOOL IsDefaultPlacement( WindowPlacement * wp )
{
    BOOL result = FALSE;

    if( wp->x == CW_USEDEFAULT || wp->y == CW_USEDEFAULT || wp->width == CW_USEDEFAULT || wp->height == CW_USEDEFAULT ) {
        result = TRUE;
    }

    return result;
}

BOOL GetActualPlacement( HWND hWnd, WindowPlacement * wp )
{
    BOOL result = FALSE;

    if( hWnd != NULL ) {
        WINDOWPLACEMENT stWP;

        ZeroMemory( &stWP, sizeof(stWP) );

        stWP.length = sizeof(stWP);

        GetWindowPlacement( hWnd, &stWP );

        wp->x = stWP.rcNormalPosition.left;
        wp->y = stWP.rcNormalPosition.top;
        wp->width = stWP.rcNormalPosition.right - stWP.rcNormalPosition.left;
        wp->height = stWP.rcNormalPosition.bottom - stWP.rcNormalPosition.top;

        result = TRUE;
    }

    return result;
}

static BOOL IsAttachedByWindowPlacement( LPRECT lprcMain, WindowPlacement * wp )
{
    BOOL result = FALSE;

    if( ! IsDefaultPlacement(wp) ) {
        if( IsAttachDistance( lprcMain->right, wp->x ) ||
            IsAttachDistance( lprcMain->bottom, wp->y ) ||
            IsAttachDistance( lprcMain->left, (wp->x + wp->width) ) ||
            IsAttachDistance( lprcMain->top, (wp->y + wp->height) ) )
        {
            result = TRUE;
        }
    }

    return result;
}

VOID ReattachAfterMove( LPRECT lprcOldPos, int new_x, int new_y, HWND hWndChild, WindowPlacement * pwpChild )
{
    if( ! IsDefaultPlacement( pwpChild ) ) {
        GetActualPlacement( hWndChild, pwpChild );

        if( IsAttachedByWindowPlacement( lprcOldPos, pwpChild ) ) {
            /* Get position delta */
            int delta_x = pwpChild->x - lprcOldPos->left;
            int delta_y = pwpChild->y - lprcOldPos->top;

            /* Adjust placement */
            pwpChild->x = new_x + delta_x;
            pwpChild->y = new_y + delta_y;

            /* Move window */
            if( hWndChild != NULL ) {
                SetWindowPos( hWndChild, HWND_TOP,
                    pwpChild->x, pwpChild->y,
                    0, 0,
                    SWP_NOZORDER | SWP_NOSIZE );
            }
        }
    }
}

extern FILE *debugFP;
VOID ReattachAfterSize( LPRECT lprcOldPos, int new_w, int new_h, HWND hWndChild, WindowPlacement * pwpChild )
{
    if( ! IsDefaultPlacement( pwpChild ) ) {
        GetActualPlacement( hWndChild, pwpChild );

        if( IsAttachedByWindowPlacement( lprcOldPos, pwpChild ) ) {
            /* Get delta of lower right corner */
            int delta_x = new_w - (lprcOldPos->right  - lprcOldPos->left);
            int delta_y = new_h - (lprcOldPos->bottom - lprcOldPos->top);

            /* Adjust size & placement */
            if(pwpChild->x + pwpChild->width  >= lprcOldPos->right  )
		pwpChild->width += delta_x;
            if(pwpChild->y + pwpChild->height >= lprcOldPos->bottom )
		pwpChild->height += delta_y;
            if(pwpChild->x >= lprcOldPos->right)  pwpChild->width  -= delta_x, pwpChild->x += delta_x;
            if(pwpChild->y >= lprcOldPos->bottom) pwpChild->height -= delta_y, pwpChild->y += delta_y;
            /* Move window */
            if( hWndChild != NULL ) {
                SetWindowPos( hWndChild, HWND_TOP,
                    pwpChild->x, pwpChild->y,
                    pwpChild->width, pwpChild->height,
                    SWP_NOZORDER );
            }
        }
    }
}
