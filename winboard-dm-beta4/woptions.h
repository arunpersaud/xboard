/*
 * woptions.h -- Options dialog box routines for WinBoard
 * $Id: 
 *
 * Copyright 2000 Free Software Foundation, Inc.
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

#include <windows.h>

VOID GeneralOptionsPopup(HWND hwnd);
VOID BoardOptionsPopup(HWND hwnd);
VOID IcsOptionsPopup(HWND hwnd);
VOID FontsOptionsPopup(HWND hwnd);
VOID SoundOptionsPopup(HWND hwnd);
VOID CommPortOptionsPopup(HWND hwnd);
VOID LoadOptionsPopup(HWND hwnd);
VOID SaveOptionsPopup(HWND hwnd);
VOID TimeControlOptionsPopup(HWND hwnd);
VOID icsZippyDrawPopUp(HWND hwnd);
