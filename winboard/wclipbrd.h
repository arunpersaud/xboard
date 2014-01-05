/*
 * wclipbrd.c -- Clipboard routines for WinBoard
 *
 * Copyright 2000, 2009, 2010, 2011, 2012, 2013, 2014 Free Software Foundation, Inc.
 *
 * Enhancements Copyright 2005 Alessandro Scotti
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

VOID CopyFENToClipboard();
VOID CopyGameToClipboard();
VOID CopyGameListToClipboard();
int CopyTextToClipboard(char *text);

VOID PasteFENFromClipboard();
VOID PasteGameFromClipboard();
int PasteTextFromClipboard(char **text);

VOID DeleteClipboardTempFiles();

VOID PasteGameOrFENFromClipboard(); /* [AS] */
