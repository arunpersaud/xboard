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

#include "config.h"

#include <windows.h>   /* required for all Windows applications */
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/stat.h>

#include "common.h"
#include "frontend.h"
#include "backend.h"
#include "winboard.h"
#include "wclipbrd.h"

#define _(s) T_(s)
#define N_(s) s

/* Imports from winboard.c */
extern HWND hwndMain;
Boolean ParseFEN(Board b, int *stm, char *FEN);

/* File globals */
static char *copyTemp;
static char *pasteTemp;

VOID 
CopyFENToClipboard()
{
  char *fen = NULL;

  if(gameMode == EditPosition) EditPositionDone(TRUE); // [HGM] mak sure castling rights are set consistently
  fen = PositionToFEN(currentMove, NULL);
  if (!fen) {
    DisplayError(_("Unable to convert position to FEN."), 0);
    return;
  }
  if (!CopyTextToClipboard(fen))
      DisplayError(_("Unable to copy FEN to clipboard."), 0);
  free(fen);
}

/* [AS] */
HGLOBAL ExportGameListAsText();

VOID CopyGameListToClipboard()
{
    HGLOBAL hMem = ExportGameListAsText();
    
    if( hMem != NULL ) {
        /* Assign memory block to clipboard */
        BOOL ok = OpenClipboard( hwndMain );

        if( ok ) {
            ok = EmptyClipboard();

            if( ok ) {
                if( hMem != SetClipboardData( CF_TEXT, hMem ) ) {
                    ok = FALSE;
                }
            }

            CloseClipboard();

            if( ! ok ) {
                GlobalFree( hMem );
            }
        }

        if( ! ok ) {
            DisplayError( "Cannot copy list to clipboard.", 0 );
        }
    }
}

VOID
CopyGameToClipboard()
{
  /* A rather cheesy hack here. Write the game to a file, then read from the
   * file into the clipboard.
   */
  char *buf = NULL;
  FILE *f;
  unsigned long size;
  size_t len;
  struct stat st;

  if (!copyTemp) {
    copyTemp = tempnam(NULL, "wbcp");
  }
  if (!copyTemp) {
      DisplayError(_("Cannot create temporary file name."),0);
      return;
  }
  f = fopen(copyTemp, "w");
  if (!f) {
    DisplayError(_("Cannot open temporary file."), 0);
    return;
  }
  if (!SaveGame(f,0,"")) { 			/* call into backend */
    DisplayError(_("Cannot write to temporary file."), 0);
    goto copy_game_to_clipboard_cleanup;
  }
  f = fopen(copyTemp, "rb");
  if (!f) {
    DisplayError(_("Cannot reopen temporary file."), 0);
    goto copy_game_to_clipboard_cleanup;
  }
  if (fstat(fileno(f), &st) < 0) {
    DisplayError(_(_("Cannot determine size of file.")), 0);
    goto copy_game_to_clipboard_cleanup;
  }
  size = st.st_size;
  if (size == -1) {
    DisplayError(_(_("Cannot determine size of file.")), 0);
    goto copy_game_to_clipboard_cleanup;
  }
  rewind(f);
  buf = (char*)malloc(size+1);
  if (!buf) {
    DisplayError(_("Cannot allocate clipboard buffer."), 0);
    goto copy_game_to_clipboard_cleanup;
  }
  len = fread(buf, sizeof(char), size, f);
  if (len == -1) {
    DisplayError(_("Cannot read from temporary file."), 0);
    goto copy_game_to_clipboard_cleanup;
  }
  if ((unsigned long)size != (unsigned long)len) { /* sigh */ 
      DisplayError(_("Error reading from temporary file."), 0);
      goto copy_game_to_clipboard_cleanup;
  }
  buf[size] = 0;
  if (!CopyTextToClipboard(buf)) {
      DisplayError(_("Cannot copy text to clipboard"), 0);
  }

copy_game_to_clipboard_cleanup:
  if (buf) free(buf);
  if (f) fclose(f);
}


int 
CopyTextToClipboard(char *text)
{
  /* some (most?) of the error checking may be overkill, 
   * but hey, this is Windows 
   */
  HGLOBAL hGlobalMem;
  LPVOID lpGlobalMem;
  BOOL locked;
  UINT lockCount;
  DWORD err;

  hGlobalMem = GlobalAlloc(GHND, (DWORD)lstrlen(text)+1);
  if (hGlobalMem == NULL) {
    DisplayError(_("Unable to allocate memory for clipboard."), 0);
    return FALSE;
  }
  lpGlobalMem = GlobalLock(hGlobalMem);
  if (lpGlobalMem == NULL) {
    DisplayError(_(_("Unable to lock clipboard memory.")), 0);
    GlobalFree(hGlobalMem);
    return FALSE;
  }
  safeStrCpy(lpGlobalMem, text, 1<<20);
  if (appData.debugMode) {
    lockCount = GlobalFlags(hGlobalMem) & GMEM_LOCKCOUNT;
    fprintf(debugFP, "CopyTextToClipboard(): lock count %d\n", lockCount);
  }
  SetLastError(NO_ERROR);
  locked = GlobalUnlock(hGlobalMem);
  err = GetLastError();
  if (appData.debugMode) {
    lockCount = GlobalFlags(hGlobalMem) & GMEM_LOCKCOUNT;
    fprintf(debugFP, "CopyTextToClipboard(): lock count %d\n", lockCount);
  }
  if (!locked) {
    locked = !((err == NO_ERROR) || (err == ERROR_NOT_LOCKED));
    if (appData.debugMode) {
      fprintf(debugFP, 
	      "CopyTextToClipboard(): err %d locked %d\n", (int)err, locked);
    }
  }
  if (locked) {
    DisplayError(_("Cannot unlock clipboard memory."), 0);
    GlobalFree(hGlobalMem);
    return FALSE;
  }
  if (!OpenClipboard(hwndMain)) {
    DisplayError(_("Cannot open clipboard."), 0);
    GlobalFree(hGlobalMem);
    return FALSE;
  }
  if (!EmptyClipboard()) {
    DisplayError(_("Cannot empty clipboard."), 0);
    return FALSE;
  }
  if (hGlobalMem != SetClipboardData(CF_TEXT, hGlobalMem)) {
    DisplayError(_("Cannot copy text to clipboard."), 0);
    CloseClipboard();
    GlobalFree(hGlobalMem);
    return FALSE;
  }
  if (!CloseClipboard())
    DisplayError(_("Cannot close clipboard."), 0);
  
  return TRUE;
}

/* [AS] Reworked paste functions so they can work with strings too */

VOID PasteFENFromString( char * fen )
{
  if (appData.debugMode) {
    fprintf(debugFP, "PasteFenFromString(): fen '%s'\n", fen);
  }
  EditPositionPasteFEN(fen); /* call into backend */
  free(fen);
}


VOID
PasteFENFromClipboard()
{
  char *fen = NULL;
  if (!PasteTextFromClipboard(&fen)) {
      DisplayError(_("Unable to paste FEN from clipboard."), 0);
      return;
  }
  PasteFENFromString( fen );
}

VOID PasteGameFromString( char * buf )
{
  FILE *f;
  size_t len;
  if (!pasteTemp) {
    pasteTemp = tempnam(NULL, "wbpt");
  }
  f = fopen(pasteTemp, "w");
  if (!f) {
    DisplayError(_("Unable to create temporary file."), 0);
    free(buf); /* [AS] */
    return;
  }
  len = fwrite(buf, sizeof(char), strlen(buf), f);
  fclose(f);
  if (len != strlen(buf)) {
    DisplayError(_("Error writing to temporary file."), 0);
    free(buf); /* [AS] */
    return;
  }
  LoadGameFromFile(pasteTemp, 0, "Clipboard", TRUE);
  free( buf ); /* [AS] */
}


VOID
PasteGameFromClipboard()
{
  /* Write the clipboard to a temp file, then let LoadGameFromFile()
   * do all the work.  */
  char *buf;
  if (!PasteTextFromClipboard(&buf)) {
    return;
  }
  PasteGameFromString( buf );
}

/* [AS] Try to detect whether the clipboard contains FEN or PGN data */
VOID PasteGameOrFENFromClipboard()
{
  char *buf;
//  char *tmp;
  Board dummyBoard; int dummy; // [HGM] paste any

  if (!PasteTextFromClipboard(&buf)) {
    return;
  }

  // [HGM] paste any: make still smarter, to allow pasting of games without tags, recognize FEN in stead
  if(!ParseFEN(dummyBoard, &dummy, buf) ) {
      PasteGameFromString( buf );
  }
  else {
      PasteFENFromString( buf );
  }
}

int 
PasteTextFromClipboard(char **text)
{
  /* some (most?) of the error checking may be overkill, 
   * but hey, this is Windows 
   */
  HANDLE hClipMem;
  LPVOID lpClipMem;
  BOOL locked = FALSE;
  DWORD err;
  UINT lockCount;

  if (!OpenClipboard(hwndMain)) {
    DisplayError(_("Unable to open clipboard."), 0);
    return FALSE;
  }
  hClipMem = GetClipboardData(CF_TEXT);
  if (hClipMem == NULL) {
    CloseClipboard();
    DisplayError(_("No text in clipboard."), 0);
    return FALSE;
  }
  lpClipMem = GlobalLock(hClipMem);
  if (lpClipMem == NULL) {
    CloseClipboard();
    DisplayError(_(_("Unable to lock clipboard memory.")), 0);
    return FALSE;
  }
  *text = (char *) malloc(GlobalSize(hClipMem)+1);
  if (!*text) {
    DisplayError(_("Unable to allocate memory for text string."), 0);
    CloseClipboard();
    return FALSE;
  }
  safeStrCpy(*text, lpClipMem, 1<<20 );
  if (appData.debugMode) {
    lockCount = GlobalFlags(hClipMem) & GMEM_LOCKCOUNT;
    fprintf(debugFP, "PasteTextFromClipboard(): lock count %d\n", lockCount);
  }
  SetLastError(NO_ERROR);
  /*suggested by Wilkin Ng*/
  lockCount = GlobalFlags(hClipMem) & GMEM_LOCKCOUNT;
  if (lockCount) {
    locked = GlobalUnlock(hClipMem);
  }
  err = GetLastError();
  if (appData.debugMode) {
    lockCount = GlobalFlags(hClipMem) & GMEM_LOCKCOUNT;
    fprintf(debugFP, "PasteTextFromClipboard(): lock count %d\n", lockCount);
  }
  if (!locked) {
    locked = !((err == NO_ERROR) || (err == ERROR_NOT_LOCKED));
    if (appData.debugMode) {
      fprintf(debugFP, 
	      "PasteTextFromClipboard(): err %d locked %d\n", (int)err, locked);
    }
  }
  if (locked) 
    DisplayError(_("Unable to unlock clipboard memory."), 0);
  
  if (!CloseClipboard())
    DisplayError(_("Unable to close clipboard."), 0);
  
  return TRUE;
}

VOID
DeleteClipboardTempFiles()
{
  if (copyTemp) remove(copyTemp);
  if (pasteTemp) remove(pasteTemp);
}
