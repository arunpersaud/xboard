/*
 * board.c -- platform-independent drawing code for XBoard
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

#define HIGHDRAG 1

#include "config.h"

#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <math.h>

#if STDC_HEADERS
# include <stdlib.h>
# include <string.h>
#else /* not STDC_HEADERS */
extern char *getenv();
# if HAVE_STRING_H
#  include <string.h>
# else /* not HAVE_STRING_H */
#  include <strings.h>
# endif /* not HAVE_STRING_H */
#endif /* not STDC_HEADERS */

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#if HAVE_UNISTD_H
# include <unistd.h>
#endif

#if HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif

#include "common.h"
#include "frontend.h"
#include "backend.h"
#include "xboard2.h"
#include "moves.h"
#include "board.h"
#include "draw.h"


#ifdef __EMX__
#ifndef HAVE_USLEEP
#define HAVE_USLEEP
#endif
#define usleep(t)   _sleep2(((t)+500)/1000)
#endif


int squareSize, lineGap;

int damage[2][BOARD_RANKS][BOARD_FILES];

/* There can be two pieces being animated at once: a player
   can begin dragging a piece before the remote opponent has moved. */

AnimState anims[NrOfAnims];

static void DrawSquare P((int row, int column, ChessSquare piece, int do_flash));
static Boolean IsDrawArrowEnabled P((void));
static void DrawArrowHighlight P((int fromX, int fromY, int toX,int toY));
static void ArrowDamage P((int s_col, int s_row, int d_col, int d_row));

static void
drawHighlight (int file, int rank, int type)
{
    int x, y;

    if (lineGap == 0) return;

    if (flipView) {
	x = lineGap/2 + ((BOARD_WIDTH-1)-file) *
	  (squareSize + lineGap);
	y = lineGap/2 + rank * (squareSize + lineGap);
    } else {
	x = lineGap/2 + file * (squareSize + lineGap);
	y = lineGap/2 + ((BOARD_HEIGHT-1)-rank) *
	  (squareSize + lineGap);
    }

    DrawBorder(x,y, type, lineGap & 1); // pass whether lineGap is odd
}

int hi1X = -1, hi1Y = -1, hi2X = -1, hi2Y = -1;
int pm1X = -1, pm1Y = -1, pm2X = -1, pm2Y = -1;

void
SetHighlights (int fromX, int fromY, int toX, int toY)
{
    int arrow = hi2X >= 0 && hi1Y >= 0 && IsDrawArrowEnabled();

    if (hi1X != fromX || hi1Y != fromY) {
	if (hi1X >= 0 && hi1Y >= 0) {
	    drawHighlight(hi1X, hi1Y, 0);
	}
    } // [HGM] first erase both, then draw new!

    if (hi2X != toX || hi2Y != toY) {
	if (hi2X >= 0 && hi2Y >= 0) {
	    drawHighlight(hi2X, hi2Y, 0);
	}
    }

    if(arrow) // there currently is an arrow displayed
	ArrowDamage(hi1X, hi1Y, hi2X, hi2Y); // mark which squares it damaged

    if (hi1X != fromX || hi1Y != fromY) {
	if (fromX >= 0 && fromY >= 0) {
	    drawHighlight(fromX, fromY, 1);
	}
    }
    if (hi2X != toX || hi2Y != toY) {
	if (toX >= 0 && toY >= 0) {
	    drawHighlight(toX, toY, 1);
	}
    }

    hi1X = fromX;
    hi1Y = fromY;
    hi2X = toX;
    hi2Y = toY;

    if(arrow || toX >= 0 && fromY >= 0 && IsDrawArrowEnabled())
	DrawPosition(FALSE, NULL); // repair any arrow damage, or draw a new one
}

void
ClearHighlights ()
{
    SetHighlights(-1, -1, -1, -1);
}


void
SetPremoveHighlights (int fromX, int fromY, int toX, int toY)
{
    if (pm1X != fromX || pm1Y != fromY) {
	if (pm1X >= 0 && pm1Y >= 0) {
	    drawHighlight(pm1X, pm1Y, 0);
	}
	if (fromX >= 0 && fromY >= 0) {
	    drawHighlight(fromX, fromY, 2);
	}
    }
    if (pm2X != toX || pm2Y != toY) {
	if (pm2X >= 0 && pm2Y >= 0) {
	    drawHighlight(pm2X, pm2Y, 0);
	}
	if (toX >= 0 && toY >= 0) {
	    drawHighlight(toX, toY, 2);
	}
    }
    pm1X = fromX;
    pm1Y = fromY;
    pm2X = toX;
    pm2Y = toY;
}

void
ClearPremoveHighlights ()
{
  SetPremoveHighlights(-1, -1, -1, -1);
}

/*
 * If the user selects on a border boundary, return -1; if off the board,
 *   return -2.  Otherwise map the event coordinate to the square.
 */
int
EventToSquare (int x, int limit)
{
    if (x <= 0)
      return -2;
    if (x < lineGap)
      return -1;
    x -= lineGap;
    if ((x % (squareSize + lineGap)) >= squareSize)
      return -1;
    x /= (squareSize + lineGap);
    if (x >= limit)
      return -2;
    return x;
}

/* [HR] determine square color depending on chess variant. */
int
SquareColor (int row, int column)
{
    int square_color;

    if (gameInfo.variant == VariantXiangqi) {
        if (column >= 3 && column <= 5 && row >= 0 && row <= 2) {
            square_color = 1;
        } else if (column >= 3 && column <= 5 && row >= 7 && row <= 9) {
            square_color = 0;
        } else if (row <= 4) {
            square_color = 0;
        } else {
            square_color = 1;
        }
    } else {
        square_color = ((column + row) % 2) == 1;
    }

    /* [hgm] holdings: next line makes all holdings squares light */
    if(column < BOARD_LEFT || column >= BOARD_RGHT) square_color = 1;

    if ( // [HGM] holdings: blank out area between board and holdings
                 column == BOARD_LEFT-1
	     ||  column == BOARD_RGHT
             || (column == BOARD_LEFT-2 && row < BOARD_HEIGHT-gameInfo.holdingsSize)
	     || (column == BOARD_RGHT+1 && row >= gameInfo.holdingsSize) )
	square_color = 2; // black

    return square_color;
}

/*	Convert board position to corner of screen rect and color	*/

void
ScreenSquare (int column, int row, Pnt *pt, int *color)
{
  if (flipView) {
    pt->x = lineGap + ((BOARD_WIDTH-1)-column) * (squareSize + lineGap);
    pt->y = lineGap + row * (squareSize + lineGap);
  } else {
    pt->x = lineGap + column * (squareSize + lineGap);
    pt->y = lineGap + ((BOARD_HEIGHT-1)-row) * (squareSize + lineGap);
  }
  *color = SquareColor(row, column);
}

/*	Convert window coords to square			*/

void
BoardSquare (int x, int y, int *column, int *row)
{
  *column = EventToSquare(x, BOARD_WIDTH);
  if (flipView && *column >= 0)
    *column = BOARD_WIDTH - 1 - *column;
  *row = EventToSquare(y, BOARD_HEIGHT);
  if (!flipView && *row >= 0)
    *row = BOARD_HEIGHT - 1 - *row;
}

/*	Generate a series of frame coords from start->mid->finish.
	The movement rate doubles until the half way point is
	reached, then halves back down to the final destination,
	which gives a nice slow in/out effect. The algorithmn
	may seem to generate too many intermediates for short
	moves, but remember that the purpose is to attract the
	viewers attention to the piece about to be moved and
	then to where it ends up. Too few frames would be less
	noticeable.						*/

static void
Tween (Pnt *start, Pnt *mid, Pnt *finish, int factor, Pnt frames[], int *nFrames)
{
  int fraction, n, count;

  count = 0;

  /* Slow in, stepping 1/16th, then 1/8th, ... */
  fraction = 1;
  for (n = 0; n < factor; n++)
    fraction *= 2;
  for (n = 0; n < factor; n++) {
    frames[count].x = start->x + (mid->x - start->x) / fraction;
    frames[count].y = start->y + (mid->y - start->y) / fraction;
    count ++;
    fraction = fraction / 2;
  }

  /* Midpoint */
  frames[count] = *mid;
  count ++;

  /* Slow out, stepping 1/2, then 1/4, ... */
  fraction = 2;
  for (n = 0; n < factor; n++) {
    frames[count].x = finish->x - (finish->x - mid->x) / fraction;
    frames[count].y = finish->y - (finish->y - mid->y) / fraction;
    count ++;
    fraction = fraction * 2;
  }
  *nFrames = count;
}

/****	Animation code by Hugh Fisher, DCS, ANU.

	Known problem: if a window overlapping the board is
	moved away while a piece is being animated underneath,
	the newly exposed area won't be updated properly.
	I can live with this.

	Known problem: if you look carefully at the animation
	of pieces in mono mode, they are being drawn as solid
	shapes without interior detail while moving. Fixing
	this would be a major complication for minimal return.
****/

/*   Utilities	*/

#undef Max  /* just in case */
#undef Min
#define Max(a, b) ((a) > (b) ? (a) : (b))
#define Min(a, b) ((a) < (b) ? (a) : (b))

typedef struct {
  short int x, y, width, height;
} MyRectangle;

void
DoSleep (int n)
{
    FrameDelay(n);
}

static void
SetRect (MyRectangle *rect, int x, int y, int width, int height)
{
  rect->x = x;
  rect->y = y;
  rect->width  = width;
  rect->height = height;
}

/*	Test if two frames overlap. If they do, return
	intersection rect within old and location of
	that rect within new. */

static Boolean
Intersect ( Pnt *old, Pnt *new, int size, MyRectangle *area, Pnt *pt)
{
  if (old->x > new->x + size || new->x > old->x + size ||
      old->y > new->y + size || new->y > old->y + size) {
    return False;
  } else {
    SetRect(area, Max(new->x - old->x, 0), Max(new->y - old->y, 0),
            size - abs(old->x - new->x), size - abs(old->y - new->y));
    pt->x = Max(old->x - new->x, 0);
    pt->y = Max(old->y - new->y, 0);
    return True;
  }
}

/*	For two overlapping frames, return the rect(s)
	in the old that do not intersect with the new.   */

static void
CalcUpdateRects (Pnt *old, Pnt *new, int size, MyRectangle update[], int *nUpdates)
{
  int	     count;

  /* If old = new (shouldn't happen) then nothing to draw */
  if (old->x == new->x && old->y == new->y) {
    *nUpdates = 0;
    return;
  }
  /* Work out what bits overlap. Since we know the rects
     are the same size we don't need a full intersect calc. */
  count = 0;
  /* Top or bottom edge? */
  if (new->y > old->y) {
    SetRect(&(update[count]), old->x, old->y, size, new->y - old->y);
    count ++;
  } else if (old->y > new->y) {
    SetRect(&(update[count]), old->x, old->y + size - (old->y - new->y),
			      size, old->y - new->y);
    count ++;
  }
  /* Left or right edge - don't overlap any update calculated above. */
  if (new->x > old->x) {
    SetRect(&(update[count]), old->x, Max(new->y, old->y),
			      new->x - old->x, size - abs(new->y - old->y));
    count ++;
  } else if (old->x > new->x) {
    SetRect(&(update[count]), new->x + size, Max(new->y, old->y),
			      old->x - new->x, size - abs(new->y - old->y));
    count ++;
  }
  /* Done */
  *nUpdates = count;
}

/* Animate the movement of a single piece */

static void
BeginAnimation (AnimNr anr, ChessSquare piece, ChessSquare bgPiece, int startColor, Pnt *start)
{
  AnimState *anim = &anims[anr];

  if(appData.upsideDown && flipView) piece += piece < BlackPawn ? BlackPawn : -BlackPawn;
  /* The old buffer is initialised with the start square (empty) */
  if(bgPiece == EmptySquare) {
    DrawBlank(anr, start->x, start->y, startColor);
  } else {
       /* Kludge alert: When gating we want the introduced
          piece to appear on the from square. To generate an
          image of it, we draw it on the board, copy the image,
          and draw the original piece again. */
       if(piece != bgPiece) DrawSquare(anim->startBoardY, anim->startBoardX, bgPiece, 0);
       CopyRectangle(anr, DISP, 2,
                 start->x, start->y, squareSize, squareSize,
                 0, 0); // [HGM] zh: unstack in stead of grab
       if(piece != bgPiece) DrawSquare(anim->startBoardY, anim->startBoardX, piece, 0);
  }
  anim->prevFrame = *start;

  SetDragPiece(anr, piece);
}

static void
AnimationFrame (AnimNr anr, Pnt *frame, ChessSquare piece)
{
  MyRectangle updates[4];
  MyRectangle overlap;
  Pnt     pt;
  AnimState *anim = &anims[anr];
  int     count, i, x, y, w, h;

  /* Save what we are about to draw into the new buffer */
  CopyRectangle(anr, DISP, 0,
	    x = frame->x, y = frame->y, w = squareSize, h = squareSize,
	    0, 0);

  /* Erase bits of the previous frame */
  if (Intersect(&anim->prevFrame, frame, squareSize, &overlap, &pt)) {
    /* Where the new frame overlapped the previous,
       the contents in newBuf are wrong. */
    CopyRectangle(anr, 2, 0,
	      overlap.x, overlap.y,
	      overlap.width, overlap.height,
	      pt.x, pt.y);
    /* Repaint the areas in the old that don't overlap new */
    CalcUpdateRects(&anim->prevFrame, frame, squareSize, updates, &count);
    for (i = 0; i < count; i++)
      CopyRectangle(anr, 2, DISP,
		updates[i].x - anim->prevFrame.x,
		updates[i].y - anim->prevFrame.y,
		updates[i].width, updates[i].height,
		updates[i].x, updates[i].y);
    /* [HGM] correct expose rectangle to encompass both overlapping squares */
    if(x > anim->prevFrame.x) w += x - anim->prevFrame.x, x = anim->prevFrame.x;
    else  w += anim->prevFrame.x - x;
    if(y > anim->prevFrame.y) h += y - anim->prevFrame.y, y = anim->prevFrame.y;
    else  h += anim->prevFrame.y - y;
  } else {
    /* Easy when no overlap */
    CopyRectangle(anr, 2, DISP,
		  0, 0, squareSize, squareSize,
		  anim->prevFrame.x, anim->prevFrame.y);
    GraphExpose(currBoard, anim->prevFrame.x, anim->prevFrame.y, squareSize, squareSize);
  }

  /* Save this frame for next time round */
  CopyRectangle(anr, 0, 2,
		0, 0, squareSize, squareSize,
		0, 0);
  anim->prevFrame = *frame;

  /* Draw piece over original screen contents, not current,
     and copy entire rect. Wipes out overlapping piece images. */
  InsertPiece(anr, piece);
  CopyRectangle(anr, 0, DISP,
		0, 0, squareSize, squareSize,
		frame->x, frame->y);
  GraphExpose(currBoard, x, y, w, h);
}

static void
EndAnimation (AnimNr anr, Pnt *finish)
{
  MyRectangle updates[4];
  MyRectangle overlap;
  Pnt     pt;
  int	     count, i;
  AnimState *anim = &anims[anr];

  /* The main code will redraw the final square, so we
     only need to erase the bits that don't overlap.	*/
  if (Intersect(&anim->prevFrame, finish, squareSize, &overlap, &pt)) {
    CalcUpdateRects(&anim->prevFrame, finish, squareSize, updates, &count);
    for (i = 0; i < count; i++)
      CopyRectangle(anr, 2, DISP,
		updates[i].x - anim->prevFrame.x,
		updates[i].y - anim->prevFrame.y,
		updates[i].width, updates[i].height,
		updates[i].x, updates[i].y);
  } else {
    CopyRectangle(anr, 2, DISP,
		0, 0, squareSize, squareSize,
		anim->prevFrame.x, anim->prevFrame.y);
  }
}

static void
FrameSequence (AnimNr anr, ChessSquare piece, int startColor, Pnt *start, Pnt *finish, Pnt frames[], int nFrames)
{
  int n;

  BeginAnimation(anr, piece, EmptySquare, startColor, start);
  for (n = 0; n < nFrames; n++) {
    AnimationFrame(anr, &(frames[n]), piece);
    FrameDelay(appData.animSpeed);
  }
  EndAnimation(anr, finish);
}

void
AnimateAtomicCapture (Board board, int fromX, int fromY, int toX, int toY)
{
    int i, x, y;
    ChessSquare piece = board[fromY][toY];
    board[fromY][toY] = EmptySquare;
    DrawPosition(FALSE, board);
    if (flipView) {
	x = lineGap + ((BOARD_WIDTH-1)-toX) * (squareSize + lineGap);
	y = lineGap + toY * (squareSize + lineGap);
    } else {
	x = lineGap + toX * (squareSize + lineGap);
	y = lineGap + ((BOARD_HEIGHT-1)-toY) * (squareSize + lineGap);
    }
    for(i=1; i<4*kFactor; i++) {
	int r = squareSize * 9 * i/(20*kFactor - 5);
	DrawDot(1, x + squareSize/2 - r, y+squareSize/2 - r, 2*r);
	FrameDelay(appData.animSpeed);
    }
    board[fromY][toY] = piece;
    DrawGrid();
}

/* Main control logic for deciding what to animate and how */

void
AnimateMove (Board board, int fromX, int fromY, int toX, int toY)
{
  ChessSquare piece;
  int hop, x = toX, y = toY;
  Pnt      start, finish, mid;
  Pnt      frames[kFactor * 2 + 1];
  int	      nFrames, startColor, endColor;

  if(killX >= 0 && IS_LION(board[fromY][fromX])) Roar();

  /* Are we animating? */
  if (!appData.animate || appData.blindfold)
    return;

  if(board[toY][toX] == WhiteRook && board[fromY][fromX] == WhiteKing ||
     board[toY][toX] == BlackRook && board[fromY][fromX] == BlackKing ||
     board[toY][toX] == WhiteKing && board[fromY][fromX] == WhiteRook || // [HGM] seirawan
     board[toY][toX] == BlackKing && board[fromY][fromX] == BlackRook)
	return; // [HGM] FRC: no animtion of FRC castlings, as to-square is not true to-square

  if (fromY < 0 || fromX < 0 || toX < 0 || toY < 0) return;
  piece = board[fromY][fromX];
  if (piece >= EmptySquare) return;

  if(killX >= 0) toX = killX, toY = killY; // [HGM] lion: first to kill square

again:

#if DONT_HOP
  hop = FALSE;
#else
  hop = abs(fromX-toX) == 1 && abs(fromY-toY) == 2 || abs(fromX-toX) == 2 && abs(fromY-toY) == 1;
#endif

  ScreenSquare(fromX, fromY, &start, &startColor);
  ScreenSquare(toX, toY, &finish, &endColor);

  if (hop) {
    /* Knight: make straight movement then diagonal */
    if (abs(toY - fromY) < abs(toX - fromX)) {
       mid.x = start.x + (finish.x - start.x) / 2;
       mid.y = start.y;
     } else {
       mid.x = start.x;
       mid.y = start.y + (finish.y - start.y) / 2;
     }
  } else {
    mid.x = start.x + (finish.x - start.x) / 2;
    mid.y = start.y + (finish.y - start.y) / 2;
  }

  /* Don't use as many frames for very short moves */
  if (abs(toY - fromY) + abs(toX - fromX) <= 2)
    Tween(&start, &mid, &finish, kFactor - 1, frames, &nFrames);
  else
    Tween(&start, &mid, &finish, kFactor, frames, &nFrames);
  FrameSequence(Game, piece, startColor, &start, &finish, frames, nFrames);
  if(Explode(board, fromX, fromY, toX, toY)) { // mark as damaged
    int i,j;
    for(i=0; i<BOARD_WIDTH; i++) for(j=0; j<BOARD_HEIGHT; j++)
      if((i-toX)*(i-toX) + (j-toY)*(j-toY) < 6) damage[0][j][i] |=  1 + ((i-toX ^ j-toY) & 1);
  }

  /* Be sure end square is redrawn */
  damage[0][toY][toX] |= True;

  if(toX != x || toY != y) { fromX = toX; fromY = toY; toX = x; toY = y; goto again; } // second leg
}

void
ChangeDragPiece (ChessSquare piece)
{
  anims[Player].dragPiece = piece;
  SetDragPiece(Player, piece);
}

void
DragPieceMove (int x, int y)
{
    Pnt corner;

    /* Are we animating? */
    if (!appData.animateDragging || appData.blindfold)
      return;

    /* Sanity check */
    if (! anims[Player].dragActive)
      return;
    /* Move piece, maintaining same relative position
       of mouse within square	 */
    corner.x = x - anims[Player].mouseDelta.x;
    corner.y = y - anims[Player].mouseDelta.y;
    AnimationFrame(Player, &corner, anims[Player].dragPiece);
#if HIGHDRAG*0
    if (appData.highlightDragging) {
	int boardX, boardY;
	BoardSquare(x, y, &boardX, &boardY);
	SetHighlights(fromX, fromY, boardX, boardY);
    }
#endif
}

void
DragPieceEnd (int x, int y)
{
    int boardX, boardY, color;
    Pnt corner;

    /* Are we animating? */
    if (!appData.animateDragging || appData.blindfold)
      return;

    /* Sanity check */
    if (! anims[Player].dragActive)
      return;
    /* Last frame in sequence is square piece is
       placed on, which may not match mouse exactly. */
    BoardSquare(x, y, &boardX, &boardY);
    ScreenSquare(boardX, boardY, &corner, &color);
    EndAnimation(Player, &corner);

    /* Be sure end square is redrawn */
    damage[0][boardY][boardX] = True;

    /* This prevents weird things happening with fast successive
       clicks which on my Sun at least can cause motion events
       without corresponding press/release. */
    anims[Player].dragActive = False;
}

void
DragPieceBegin (int x, int y, Boolean instantly)
{
    int	 boardX, boardY, color;
    Pnt corner;

    /* Are we animating? */
    if (!appData.animateDragging || appData.blindfold)
      return;

    /* Figure out which square we start in and the
       mouse position relative to top left corner. */
    BoardSquare(x, y, &boardX, &boardY);
    anims[Player].startBoardX = boardX;
    anims[Player].startBoardY = boardY;
    ScreenSquare(boardX, boardY, &corner, &color);
    anims[Player].startSquare  = corner;
    anims[Player].startColor   = color;
    /* As soon as we start dragging, the piece will jump slightly to
       be centered over the mouse pointer. */
    anims[Player].mouseDelta.x = squareSize/2;
    anims[Player].mouseDelta.y = squareSize/2;
    /* Initialise animation */
    anims[Player].dragPiece = PieceForSquare(boardX, boardY);
    /* Sanity check */
    if (anims[Player].dragPiece >= 0 && anims[Player].dragPiece < EmptySquare) {
	ChessSquare bgPiece = EmptySquare;
	anims[Player].dragActive = True;
        if(boardX == BOARD_RGHT+1 && PieceForSquare(boardX-1, boardY) > 1 ||
           boardX == BOARD_LEFT-2 && PieceForSquare(boardX+1, boardY) > 1)
	    bgPiece = anims[Player].dragPiece;
        if(gatingPiece != EmptySquare) bgPiece = gatingPiece;
	BeginAnimation(Player, anims[Player].dragPiece, bgPiece, color, &corner);
	/* Mark this square as needing to be redrawn. Note that
	   we don't remove the piece though, since logically (ie
	   as seen by opponent) the move hasn't been made yet. */
	damage[0][boardY][boardX] = True;
    } else {
	anims[Player].dragActive = False;
    }
}

/* Handle expose event while piece being dragged */

static void
DrawDragPiece ()
{
  if (!anims[Player].dragActive || appData.blindfold)
    return;

  /* What we're doing: logically, the move hasn't been made yet,
     so the piece is still in it's original square. But visually
     it's being dragged around the board. So we erase the square
     that the piece is on and draw it at the last known drag point. */
  DrawOneSquare(anims[Player].startSquare.x, anims[Player].startSquare.y,
		EmptySquare, anims[Player].startColor, 0, NULL, NULL, 0);
  AnimationFrame(Player, &anims[Player].prevFrame, anims[Player].dragPiece);
  damage[0][anims[Player].startBoardY][anims[Player].startBoardX] = TRUE;
}

static void
DrawSquare (int row, int column, ChessSquare piece, int do_flash)
{
    int square_color, x, y, align=0;
    int i;
    char tString[3], bString[2];
    int flash_delay;

    /* Calculate delay in milliseconds (2-delays per complete flash) */
    flash_delay = 500 / appData.flashRate;

    if (flipView) {
	x = lineGap + ((BOARD_WIDTH-1)-column) *
	  (squareSize + lineGap);
	y = lineGap + row * (squareSize + lineGap);
    } else {
	x = lineGap + column * (squareSize + lineGap);
	y = lineGap + ((BOARD_HEIGHT-1)-row) *
	  (squareSize + lineGap);
    }

    square_color = SquareColor(row, column);

    bString[1] = bString[0] = NULLCHAR;
    if (appData.showCoords && row == (flipView ? BOARD_HEIGHT-1 : 0)
		&& column >= BOARD_LEFT && column < BOARD_RGHT) {
	bString[0] = 'a' + column - BOARD_LEFT;
	align = 1; // coord in lower-right corner
    }
    if (appData.showCoords && column == (flipView ? BOARD_RGHT-1 : BOARD_LEFT)) {
	snprintf(tString, 3, "%d", ONE - '0' + row);
	align = 2; // coord in upper-left corner
    }
    if (column == (flipView ? BOARD_LEFT-1 : BOARD_RGHT) && piece > 1 ) {
	snprintf(tString, 3, "%d", piece);
	align = 3; // holdings count in upper-right corner
    }
    if (column == (flipView ? BOARD_RGHT : BOARD_LEFT-1) && piece > 1) {
	snprintf(tString, 3, "%d", piece);
	align = 4; // holdings count in upper-left corner
    }
    if(piece == DarkSquare) square_color = 2;
    if(square_color == 2 || appData.blindfold) piece = EmptySquare;

    if (do_flash && piece != EmptySquare && appData.flashCount > 0) {
	for (i=0; i<appData.flashCount; ++i) {
	    DrawOneSquare(x, y, piece, square_color, 0, tString, bString, 0);
	    GraphExpose(currBoard, x, y, squareSize, squareSize);
	    FlashDelay(flash_delay);
	    DrawOneSquare(x, y, EmptySquare, square_color, 0, tString, bString, 0);
	    GraphExpose(currBoard, x, y, squareSize, squareSize);
	    FlashDelay(flash_delay);
	}
    }
    DrawOneSquare(x, y, piece, square_color, partnerUp ? 0 : marker[row][column], tString, bString, align);
}

/* Returns 1 if there are "too many" differences between b1 and b2
   (i.e. more than 1 move was made) */
static int
too_many_diffs (Board b1, Board b2)
{
    int i, j;
    int c = 0;

    for (i=0; i<BOARD_HEIGHT; ++i) {
	for (j=0; j<BOARD_WIDTH; ++j) {
	    if (b1[i][j] != b2[i][j]) {
		if (++c > 4)	/* Castling causes 4 diffs */
		  return 1;
	    }
	}
    }
    return 0;
}

/* Matrix describing castling maneuvers */
/* Row, ColRookFrom, ColKingFrom, ColRookTo, ColKingTo */
static int castling_matrix[4][5] = {
    { 0, 0, 4, 3, 2 },		/* 0-0-0, white */
    { 0, 7, 4, 5, 6 },		/* 0-0,   white */
    { 7, 0, 4, 3, 2 },		/* 0-0-0, black */
    { 7, 7, 4, 5, 6 }		/* 0-0,   black */
};

/* Checks whether castling occurred. If it did, *rrow and *rcol
   are set to the destination (row,col) of the rook that moved.

   Returns 1 if castling occurred, 0 if not.

   Note: Only handles a max of 1 castling move, so be sure
   to call too_many_diffs() first.
   */
static int
check_castle_draw (Board newb, Board oldb, int *rrow, int *rcol)
{
    int i, *r, j;
    int match;

    /* For each type of castling... */
    for (i=0; i<4; ++i) {
	r = castling_matrix[i];

	/* Check the 4 squares involved in the castling move */
	match = 0;
	for (j=1; j<=4; ++j) {
	    if (newb[r[0]][r[j]] == oldb[r[0]][r[j]]) {
		match = 1;
		break;
	    }
	}

	if (!match) {
	    /* All 4 changed, so it must be a castling move */
	    *rrow = r[0];
	    *rcol = r[3];
	    return 1;
	}
    }
    return 0;
}

void
DrawPosition (int repaint, Board board)
{
    int i, j, do_flash, exposeAll = False;
    static int lastFlipView = 0;
    static int lastBoardValid[2] = {0, 0};
    static Board lastBoard[2];
    static char lastMarker[BOARD_RANKS][BOARD_FILES];
    int rrow, rcol;
    int nr = twoBoards*partnerUp;

    if(DrawSeekGraph()) return; // [HGM] seekgraph: suppress any drawing if seek graph up

    if (board == NULL) {
	if (!lastBoardValid[nr]) return;
	board = lastBoard[nr];
    }
    if (!lastBoardValid[nr] || (nr == 0 && lastFlipView != flipView)) {
	MarkMenuItem("View.Flip View", flipView);
    }

    if(nr) { SlavePopUp(); SwitchWindow(0); } // [HGM] popup board if not yet popped up, and switch drawing to it.

    /*
     * It would be simpler to clear the window with XClearWindow()
     * but this causes a very distracting flicker.
     */

    if (!repaint && lastBoardValid[nr] && (nr == 1 || lastFlipView == flipView)) {

//	if ( lineGap && IsDrawArrowEnabled())
//	    DrawGrid();

	/* If too much changes (begin observing new game, etc.), don't
	   do flashing */
	do_flash = too_many_diffs(board, lastBoard[nr]) ? 0 : 1;

	/* Special check for castling so we don't flash both the king
	   and the rook (just flash the king). */
	if (do_flash) {
	    if (check_castle_draw(board, lastBoard[nr], &rrow, &rcol)) {
		/* Mark rook for drawing with NO flashing. */
		damage[nr][rrow][rcol] |= 1;
	    }
	}

	/* First pass -- Draw (newly) empty squares and repair damage.
	   This prevents you from having a piece show up twice while it
	   is flashing on its new square */
	for (i = 0; i < BOARD_HEIGHT; i++)
	  for (j = 0; j < BOARD_WIDTH; j++)
	    if (((board[i][j] != lastBoard[nr][i][j] || !nr && marker[i][j] != lastMarker[i][j]) && board[i][j] == EmptySquare)
		|| damage[nr][i][j]) {
		DrawSquare(i, j, board[i][j], 0);
		if(damage[nr][i][j] & 2) {
		    drawHighlight(j, i, 0);   // repair arrow damage
		    if(lineGap) damage[nr][i][j] = False; // this flushed the square as well
		} else damage[nr][i][j] = 1;  // mark for expose
	    }

	/* Second pass -- Draw piece(s) in new position and flash them */
	for (i = 0; i < BOARD_HEIGHT; i++)
	  for (j = 0; j < BOARD_WIDTH; j++)
	    if (board[i][j] != lastBoard[nr][i][j] || !nr && marker[i][j] != lastMarker[i][j]) {
		DrawSquare(i, j, board[i][j], do_flash);
		damage[nr][i][j] = 1; // mark for expose
	    }
    } else {
	if (lineGap > 0)
	  DrawGrid();

	for (i = 0; i < BOARD_HEIGHT; i++)
	  for (j = 0; j < BOARD_WIDTH; j++) {
	      DrawSquare(i, j, board[i][j], 0);
	      damage[nr][i][j] = False;
	  }

	exposeAll = True;
    }

    CopyBoard(lastBoard[nr], board);
    lastBoardValid[nr] = 1;
  if(nr == 0) { // [HGM] dual: no highlights on second board yet
    lastFlipView = flipView;
    for (i = 0; i < BOARD_HEIGHT; i++)
	for (j = 0; j < BOARD_WIDTH; j++)
	    lastMarker[i][j] = marker[i][j];

    /* Draw highlights */
    if (pm1X >= 0 && pm1Y >= 0) {
      drawHighlight(pm1X, pm1Y, 2);
      if(lineGap) damage[nr][pm1Y][pm1X] = False;
    }
    if (pm2X >= 0 && pm2Y >= 0) {
      drawHighlight(pm2X, pm2Y, 2);
      if(lineGap) damage[nr][pm2Y][pm2X] = False;
    }
    if (hi1X >= 0 && hi1Y >= 0) {
      drawHighlight(hi1X, hi1Y, 1);
      if(lineGap) damage[nr][hi1Y][hi1X] = False;
    }
    if (hi2X >= 0 && hi2Y >= 0) {
      drawHighlight(hi2X, hi2Y, 1);
      if(lineGap) damage[nr][hi2Y][hi2X] = False;
    }
    DrawArrowHighlight(hi1X, hi1Y, hi2X, hi2Y);
  }
  else DrawArrowHighlight (board[EP_STATUS-3], board[EP_STATUS-4], board[EP_STATUS-1], board[EP_STATUS-2]);

    /* If piece being dragged around board, must redraw that too */
    DrawDragPiece();

    if(exposeAll)
	GraphExpose(currBoard, 0, 0, BOARD_WIDTH*(squareSize + lineGap) + lineGap, BOARD_HEIGHT*(squareSize + lineGap) + lineGap);
    else {
	for (i = 0; i < BOARD_HEIGHT; i++)
	    for (j = 0; j < BOARD_WIDTH; j++)
		if(damage[nr][i][j]) {
		    int x, y;
		    if (flipView) {
			x = lineGap + ((BOARD_WIDTH-1)-j) *
			  (squareSize + lineGap);
			y = lineGap + i * (squareSize + lineGap);
		    } else {
			x = lineGap + j * (squareSize + lineGap);
			y = lineGap + ((BOARD_HEIGHT-1)-i) *
			  (squareSize + lineGap);
		    }
		    if(damage[nr][i][j] & 2) // damage by old or new arrow
			GraphExpose(currBoard, x - lineGap, y - lineGap, squareSize + 2*lineGap, squareSize  + 2*lineGap);
		    else
			GraphExpose(currBoard, x, y, squareSize, squareSize);
		    damage[nr][i][j] &= 2; // remember damage by newly drawn error in '2' bit, to schedule it for erasure next draw
		}
    }

    FlashDelay(0); // this flushes drawing queue;
    if(nr) SwitchWindow(1);
}

/* [AS] Arrow highlighting support */

static double A_WIDTH = 5; /* Width of arrow body */

#define A_HEIGHT_FACTOR 6   /* Length of arrow "point", relative to body width */
#define A_WIDTH_FACTOR  3   /* Width of arrow "point", relative to body width */

static double
Sqr (double x)
{
    return x*x;
}

static int
Round (double x)
{
    return (int) (x + 0.5);
}

void
SquareToPos (int rank, int file, int *x, int *y)
{
    if (flipView) {
	*x = lineGap + ((BOARD_WIDTH-1)-file) * (squareSize + lineGap);
	*y = lineGap + rank * (squareSize + lineGap);
    } else {
	*x = lineGap + file * (squareSize + lineGap);
	*y = lineGap + ((BOARD_HEIGHT-1)-rank) * (squareSize + lineGap);
    }
}

/* Draw an arrow between two points using current settings */
static void
DrawArrowBetweenPoints (int s_x, int s_y, int d_x, int d_y)
{
    Pnt arrow[8];
    double dx, dy, j, k, x, y;

    if( d_x == s_x ) {
        int h = (d_y > s_y) ? +A_WIDTH*A_HEIGHT_FACTOR : -A_WIDTH*A_HEIGHT_FACTOR;

        arrow[0].x = s_x + A_WIDTH + 0.5;
        arrow[0].y = s_y;

        arrow[1].x = s_x + A_WIDTH + 0.5;
        arrow[1].y = d_y - h;

        arrow[2].x = arrow[1].x + A_WIDTH*(A_WIDTH_FACTOR-1) + 0.5;
        arrow[2].y = d_y - h;

        arrow[3].x = d_x;
        arrow[3].y = d_y;

        arrow[5].x = arrow[1].x - 2*A_WIDTH + 0.5;
        arrow[5].y = d_y - h;

        arrow[4].x = arrow[5].x - A_WIDTH*(A_WIDTH_FACTOR-1) + 0.5;
        arrow[4].y = d_y - h;

        arrow[6].x = arrow[1].x - 2*A_WIDTH + 0.5;
        arrow[6].y = s_y;
    }
    else if( d_y == s_y ) {
        int w = (d_x > s_x) ? +A_WIDTH*A_HEIGHT_FACTOR : -A_WIDTH*A_HEIGHT_FACTOR;

        arrow[0].x = s_x;
        arrow[0].y = s_y + A_WIDTH + 0.5;

        arrow[1].x = d_x - w;
        arrow[1].y = s_y + A_WIDTH + 0.5;

        arrow[2].x = d_x - w;
        arrow[2].y = arrow[1].y + A_WIDTH*(A_WIDTH_FACTOR-1) + 0.5;

        arrow[3].x = d_x;
        arrow[3].y = d_y;

        arrow[5].x = d_x - w;
        arrow[5].y = arrow[1].y - 2*A_WIDTH + 0.5;

        arrow[4].x = d_x - w;
        arrow[4].y = arrow[5].y - A_WIDTH*(A_WIDTH_FACTOR-1) + 0.5;

        arrow[6].x = s_x;
        arrow[6].y = arrow[1].y - 2*A_WIDTH + 0.5;
    }
    else {
        /* [AS] Needed a lot of paper for this! :-) */
        dy = (double) (d_y - s_y) / (double) (d_x - s_x);
        dx = (double) (s_x - d_x) / (double) (s_y - d_y);

        j = sqrt( Sqr(A_WIDTH) / (1.0 + Sqr(dx)) );

        k = sqrt( Sqr(A_WIDTH*A_HEIGHT_FACTOR) / (1.0 + Sqr(dy)) );

        x = s_x;
        y = s_y;

        arrow[0].x = Round(x - j);
        arrow[0].y = Round(y + j*dx);

        arrow[1].x = Round(arrow[0].x + 2*j);   // [HGM] prevent width to be affected by rounding twice
        arrow[1].y = Round(arrow[0].y - 2*j*dx);

        if( d_x > s_x ) {
            x = (double) d_x - k;
            y = (double) d_y - k*dy;
        }
        else {
            x = (double) d_x + k;
            y = (double) d_y + k*dy;
        }

        x = Round(x); y = Round(y); // [HGM] make sure width of shaft is rounded the same way on both ends

        arrow[6].x = Round(x - j);
        arrow[6].y = Round(y + j*dx);

        arrow[2].x = Round(arrow[6].x + 2*j);
        arrow[2].y = Round(arrow[6].y - 2*j*dx);

        arrow[3].x = Round(arrow[2].x + j*(A_WIDTH_FACTOR-1));
        arrow[3].y = Round(arrow[2].y - j*(A_WIDTH_FACTOR-1)*dx);

        arrow[4].x = d_x;
        arrow[4].y = d_y;

        arrow[5].x = Round(arrow[6].x - j*(A_WIDTH_FACTOR-1));
        arrow[5].y = Round(arrow[6].y + j*(A_WIDTH_FACTOR-1)*dx);
    }

    DrawPolygon(arrow, 7);
//    Polygon( hdc, arrow, 7 );
}

static void
ArrowDamage (int s_col, int s_row, int d_col, int d_row)
{
    int hor, vert, i, n = partnerUp * twoBoards;
    hor = 64*s_col + 32; vert = 64*s_row + 32;
    for(i=0; i<= 64; i++) {
            damage[n][vert+6>>6][hor+6>>6] |= 2;
            damage[n][vert-6>>6][hor+6>>6] |= 2;
            damage[n][vert+6>>6][hor-6>>6] |= 2;
            damage[n][vert-6>>6][hor-6>>6] |= 2;
            hor += d_col - s_col; vert += d_row - s_row;
    }
}

/* [AS] Draw an arrow between two squares */
static void
DrawArrowBetweenSquares (int s_col, int s_row, int d_col, int d_row)
{
    int s_x, s_y, d_x, d_y;

    if( s_col == d_col && s_row == d_row ) {
        return;
    }

    /* Get source and destination points */
    SquareToPos( s_row, s_col, &s_x, &s_y);
    SquareToPos( d_row, d_col, &d_x, &d_y);

    if( d_y > s_y ) {
        d_y += squareSize / 2 - squareSize / 4; // [HGM] round towards same centers on all sides!
    }
    else if( d_y < s_y ) {
        d_y += squareSize / 2 + squareSize / 4;
    }
    else {
        d_y += squareSize / 2;
    }

    if( d_x > s_x ) {
        d_x += squareSize / 2 - squareSize / 4;
    }
    else if( d_x < s_x ) {
        d_x += squareSize / 2 + squareSize / 4;
    }
    else {
        d_x += squareSize / 2;
    }

    s_x += squareSize / 2;
    s_y += squareSize / 2;

    /* Adjust width */
    A_WIDTH = squareSize / 14.; //[HGM] make float

    DrawArrowBetweenPoints( s_x, s_y, d_x, d_y );
    ArrowDamage(s_col, s_row, d_col, d_row);
}

static Boolean
IsDrawArrowEnabled ()
{
    return (appData.highlightMoveWithArrow || twoBoards && partnerUp) && squareSize >= 32;
}

static void
DrawArrowHighlight (int fromX, int fromY, int toX,int toY)
{
    if( IsDrawArrowEnabled() && fromX >= 0 && fromY >= 0 && toX >= 0 && toY >= 0)
        DrawArrowBetweenSquares(fromX, fromY, toX, toY);
}
