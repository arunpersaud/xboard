/*
 * wclipbrd.c -- Clipboard routines for WinBoard
 * $Id: wclipbrd.h,v 2.1 2003/10/27 19:21:02 mann Exp $
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

VOID CopyFENToClipboard();
VOID CopyGameToClipboard();
VOID CopyGameListToClipboard();
int CopyTextToClipboard(char *text);

VOID PasteFENFromClipboard();
VOID PasteGameFromClipboard();
int PasteTextFromClipboard(char **text);

VOID DeleteClipboardTempFiles();

VOID PasteGameOrFENFromClipboard(); /* [AS] */
