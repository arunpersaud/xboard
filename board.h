/*
 * board.h -- header for XBoard: variables shared by xboard.c and board.c
 *
 * Copyright 1991 by Digital Equipment Corporation, Maynard,
 * Massachusetts.
 *
 * Enhancements Copyright 1992-2001, 2002, 2003, 2004, 2005, 2006,
 * 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014 Free Software Foundation, Inc.
 *
 * The following terms apply to Digital Equipment Corporation's copyright
 * interest in XBoard:
 * ------------------------------------------------------------------------
 * All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of Digital not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 * ------------------------------------------------------------------------
 *
 * The following terms apply to the enhanced version of XBoard
 * distributed by the Free Software Foundation:
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


/* This magic number is the number of intermediate frames used
   in each half of the animation. For short moves it's reduced
   by 1. The total number of frames will be factor * 2 + 1.  */
#define kFactor	   4

/* Variables for doing smooth animation. This whole thing
   would be much easier if the board was double-buffered,
   but that would require a fairly major rewrite.	*/

#define DISP 4

typedef enum { Game=0, Player, NrOfAnims } AnimNr;

typedef struct {
	short int x, y;
    } Pnt;

typedef struct {
	Pnt	startSquare, prevFrame, mouseDelta;
	int	startColor;
	int	dragPiece;
	Boolean	dragActive;
        int     startBoardX, startBoardY;
    } AnimState;

extern AnimState anims[];

void DrawPolygon P((Pnt arrow[], int nr));
void DrawOneSquare P((int x, int y, ChessSquare piece, int square_color, int marker, char *tString, char *bString, int align));
void DrawDot P((int marker, int x, int y, int r));
void DrawGrid P((void));
int SquareColor P((int row, int column));
void ScreenSquare P((int column, int row, Pnt *pt, int *color));
void BoardSquare P((int x, int y, int *column, int *row));
void FrameDelay P((int time));
void InsertPiece P((AnimNr anr, ChessSquare piece));
void DrawBlank P((AnimNr anr, int x, int y, int startColor));
void CopyRectangle P((AnimNr anr, int srcBuf, int destBuf, int srcX, int srcY, int width, int height, int destX, int destY));
void SetDragPiece P((AnimNr anr, ChessSquare piece));
void DragPieceMove P((int x, int y));
void DrawBorder P((int x, int y, int type, int odd));
void FlashDelay P((int flash_delay));
void SwitchWindow P((int main));

extern int damage[2][BOARD_RANKS][BOARD_FILES];
extern Option *currBoard;
