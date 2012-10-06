/*
 * draw.c -- drawing routines for XBoard
 *
 * Copyright 1991 by Digital Equipment Corporation, Maynard,
 * Massachusetts.
 *
 * Enhancements Copyright 1992-2001, 2002, 2003, 2004, 2005, 2006,
 * 2007, 2008, 2009, 2010, 2011, 2012 Free Software Foundation, Inc.
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

#include "config.h"

#include <stdio.h>
#include <math.h>
#include <cairo/cairo.h>
#include <cairo/cairo-xlib.h>

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

#if ENABLE_NLS
#include <locale.h>
#endif


// [HGM] bitmaps: put before incuding the bitmaps / pixmaps, to know how many piece types there are.
#include "common.h"

#if HAVE_LIBXPM
#include "pixmaps/pixmaps.h"
#else
#include "bitmaps/bitmaps.h"
#endif

#include "frontend.h"
#include "backend.h"
#include "xevalgraph.h"
#include "board.h"
#include "menus.h"
#include "dialogs.h"
#include "gettext.h"
#include "draw.h"


#ifdef __EMX__
#ifndef HAVE_USLEEP
#define HAVE_USLEEP
#endif
#define usleep(t)   _sleep2(((t)+500)/1000)
#endif

#ifdef ENABLE_NLS
# define  _(s) gettext (s)
# define N_(s) gettext_noop (s)
#else
# define  _(s) (s)
# define N_(s)  s
#endif

#define SOLID 0
#define OUTLINE 1
Boolean cairoAnimate;
static cairo_surface_t *csBoardWindow, *csBoardBackup, *csDualBoard;
static cairo_surface_t *pngPieceBitmaps[2][(int)BlackPawn];    // scaled pieces as used
static cairo_surface_t *pngPieceBitmaps2[2][(int)BlackPawn+4]; // scaled pieces in store
static cairo_surface_t *pngBoardBitmap[2];
int useTexture, textureW[2], textureH[2];

#define pieceToSolid(piece) &pieceBitmap[SOLID][(piece) % (int)BlackPawn]
#define pieceToOutline(piece) &pieceBitmap[OUTLINE][(piece) % (int)BlackPawn]

#define White(piece) ((int)(piece) < (int)BlackPawn)

struct {
  int x1, x2, y1, y2;
} gridSegments[BOARD_RANKS + BOARD_FILES + 2];

static int dual = 0;

void
SwitchWindow ()
{
    cairo_surface_t *cstmp = csBoardWindow;
    csBoardWindow = csDualBoard;
    dual = !dual;
    if(!csDualBoard) {
	csBoardWindow = GetOutputSurface(&dualOptions[3], 0, 0);
	dual = 1;
    }
    csDualBoard = cstmp;
}

void
NewSurfaces ()
{
    // delete surfaces after size becomes invalid, so they will be recreated
    if(csBoardWindow) cairo_surface_destroy(csBoardWindow);
    if(csBoardBackup) cairo_surface_destroy(csBoardBackup);
    if(csDualBoard) cairo_surface_destroy(csDualBoard);
    csBoardWindow = csBoardBackup = csDualBoard = NULL;
}

#define BoardSize int
void
InitDrawingSizes (BoardSize boardSize, int flags)
{   // [HGM] resize is functional now, but for board format changes only (nr of ranks, files)
    int boardWidth, boardHeight;
    int i;
    static int oldWidth, oldHeight;
    static VariantClass oldVariant;
    static int oldMono = -1, oldTwoBoards = 0;
    extern Widget formWidget;

    if(!formWidget) return;

    if(oldTwoBoards && !twoBoards) PopDown(DummyDlg);
    oldTwoBoards = twoBoards;

    if(appData.overrideLineGap >= 0) lineGap = appData.overrideLineGap;
    boardWidth = lineGap + BOARD_WIDTH * (squareSize + lineGap);
    boardHeight = lineGap + BOARD_HEIGHT * (squareSize + lineGap);

  if(boardWidth != oldWidth || boardHeight != oldHeight) { // do resizing stuff only if size actually changed

    oldWidth = boardWidth; oldHeight = boardHeight;
    CreateGrid();
    NewSurfaces();

    /*
     * Inhibit shell resizing.
     */
    ResizeBoardWindow(boardWidth, boardHeight, 0);

    DelayedDrag();
  }

    // [HGM] pieces: tailor piece bitmaps to needs of specific variant
    // (only for xpm)

  if(gameInfo.variant != oldVariant) { // and only if variant changed

    for(i=0; i<2; i++) {
	int p;
	for(p=0; p<=(int)WhiteKing; p++)
	   pngPieceBitmaps[i][p] = pngPieceBitmaps2[i][p]; // defaults
	if(gameInfo.variant == VariantShogi) {
	   pngPieceBitmaps[i][(int)WhiteCannon] = pngPieceBitmaps2[i][(int)WhiteKing+1];
	   pngPieceBitmaps[i][(int)WhiteNightrider] = pngPieceBitmaps2[i][(int)WhiteKing+2];
	   pngPieceBitmaps[i][(int)WhiteSilver] = pngPieceBitmaps2[i][(int)WhiteKing+3];
	   pngPieceBitmaps[i][(int)WhiteGrasshopper] = pngPieceBitmaps2[i][(int)WhiteKing+4];
	   pngPieceBitmaps[i][(int)WhiteQueen] = pngPieceBitmaps2[i][(int)WhiteLance];
	}
#ifdef GOTHIC
	if(gameInfo.variant == VariantGothic) {
	   pngPieceBitmaps[i][(int)WhiteMarshall] = pngPieceBitmaps2[i][(int)WhiteSilver];
	}
#endif
	if(gameInfo.variant == VariantSChess) {
	   pngPieceBitmaps[i][(int)WhiteAngel]    = pngPieceBitmaps2[i][(int)WhiteFalcon];
	   pngPieceBitmaps[i][(int)WhiteMarshall] = pngPieceBitmaps2[i][(int)WhiteAlfil];
	}
    }
    oldMono = -10; // kludge to force recreation of animation masks
    oldVariant = gameInfo.variant;
  }
  CreateAnimVars();
  oldMono = appData.monoMode;
}

static void
CreatePNGBoard (char *s, int kind)
{
    if(!appData.useBitmaps || s == NULL || *s == 0 || *s == '*') { useTexture &= ~(kind+1); return; }
    if(strstr(s, ".png")) {
	cairo_surface_t *img = cairo_image_surface_create_from_png (s);
	if(img) {
	    useTexture |= kind + 1; pngBoardBitmap[kind] = img;
	    textureW[kind] = cairo_image_surface_get_width (img);
	    textureH[kind] = cairo_image_surface_get_height (img);
	}
    }
}

char *pngPieceNames[] = // must be in same order as internal piece encoding
{ "Pawn", "Knight", "Bishop", "Rook", "Queen", "Advisor", "Elephant", "Archbishop", "Marshall", "Gold", "Commoner", 
  "Canon", "Nightrider", "CrownedBishop", "CrownedRook", "Princess", "Chancellor", "Hawk", "Lance", "Cobra", "Unicorn", "King", 
  "GoldKnight", "GoldLance", "GoldPawn", "GoldSilver", NULL
};

cairo_surface_t *
ConvertPixmap (int color, int piece)
{
  int i, j, stride, f, colcode[10], w, b;
  char ch[10];
  cairo_surface_t *res;
  XpmPieces *p = builtInXpms + 10;
  char **pixels = p->xpm[piece % BlackPawn][2*color];
  int *buf;
  sscanf(pixels[0], "%*d %*d %d", &f);
  sscanf(appData.whitePieceColor+1, "%x", &w);
  sscanf(appData.blackPieceColor+1, "%x", &b);
  res = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, p->size, p->size);
  stride = cairo_image_surface_get_stride(res);
  buf = (int *) cairo_image_surface_get_data(res);
  for(i=0; i<f; i++) {
    ch[i] = pixels[i+1][0];
    colcode[i] = 0;
    if(strstr(pixels[i+1], "black")) colcode[i] = 0xFF000000 + (color ? b : 0);
    if(strstr(pixels[i+1], "white")) colcode[i] = 0xFF000000 + w;
  }
  for(i=0; i<p->size; i++) {
    for(j=0; j<p->size; j++) {
      char c = pixels[i+f+1][j];
      int k;
      for(k=0; ch[k] != c && k < f; k++);
      buf[i*p->size + j] = colcode[k];
    }
  }
  cairo_surface_mark_dirty(res);
  return res;
}

static void
ScaleOnePiece (char *name, int color, int piece)
{
  float w, h;
  char buf[MSG_SIZ];
  cairo_surface_t *img, *cs;
  cairo_t *cr;
  static cairo_surface_t *pngPieceImages[2][(int)BlackPawn+4];   // png 256 x 256 images

  if((img = pngPieceImages[color][piece]) == NULL) { // if PNG file for this piece was not yet read, read it now and store it
    if(!*appData.pngDirectory) img = ConvertPixmap(color, piece); else {
      snprintf(buf, MSG_SIZ, "%s/%s%s.png", appData.pngDirectory, color ? "Black" : "White", pngPieceNames[piece]);
      img = cairo_image_surface_create_from_png (buf);
      if(cairo_surface_status(img) != CAIRO_STATUS_SUCCESS) img = ConvertPixmap(color, piece);
    }
  }
  pngPieceImages[color][piece] = img;
  // create new bitmap to hold scaled piece image (and remove any old)
  if(pngPieceBitmaps2[color][piece]) cairo_surface_destroy (pngPieceBitmaps2[color][piece]);
  pngPieceBitmaps2[color][piece] = cs = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, squareSize, squareSize);
  if(piece <= WhiteKing) pngPieceBitmaps[color][piece] = cs;
  // scaled copying of the raw png image
  cr = cairo_create(cs);
  w = cairo_image_surface_get_width (img);
  h = cairo_image_surface_get_height (img);
  cairo_scale(cr, squareSize/w, squareSize/h);
  cairo_set_source_surface (cr, img, 0, 0);
  cairo_paint (cr);
  cairo_destroy (cr);
}

void
CreatePNGPieces ()
{
  int p;

  for(p=0; pngPieceNames[p]; p++) {
    ScaleOnePiece(pngPieceNames[p], 0, p);
    ScaleOnePiece(pngPieceNames[p], 1, p);
  }
}

void
CreateAnyPieces ()
{   // [HGM] taken out of main
    CreatePNGPieces();
    CreatePNGBoard(appData.liteBackTextureFile, 1);
    CreatePNGBoard(appData.darkBackTextureFile, 0);
}

void
InitDrawingParams ()
{
    MakeColors();
    CreateAnyPieces();
}

// [HGM] seekgraph: some low-level drawing routines (by JC, mostly)

float
Color (char *col, int n)
{
  int c;
  sscanf(col, "#%x", &c);
  c = c >> 4*n & 255;
  return c/255.;
}

void
SetPen (cairo_t *cr, float w, char *col, int dash)
{
  static const double dotted[] = {4.0, 4.0};
  static int len  = sizeof(dotted) / sizeof(dotted[0]);
  cairo_set_line_width (cr, w);
  cairo_set_source_rgba (cr, Color(col, 4), Color(col, 2), Color(col, 0), 1.0);
  if(dash) cairo_set_dash (cr, dotted, len, 0.0);
}

void DrawSeekAxis( int x, int y, int xTo, int yTo )
{
    cairo_t *cr;

    /* get a cairo_t */
    cr = cairo_create (csBoardWindow);

    cairo_move_to (cr, x, y);
    cairo_line_to(cr, xTo, yTo );

    SetPen(cr, 2, "#000000", 0);
    cairo_stroke(cr);

    /* free memory */
    cairo_destroy (cr);
}

void DrawSeekBackground( int left, int top, int right, int bottom )
{
    cairo_t *cr = cairo_create (csBoardWindow);

    cairo_rectangle (cr, left, top, right-left, bottom-top);

    cairo_set_source_rgba(cr, 0.8, 0.8, 0.4,1.0);
    cairo_fill(cr);

    /* free memory */
    cairo_destroy (cr);
}

void DrawSeekText(char *buf, int x, int y)
{
    cairo_t *cr = cairo_create (csBoardWindow);

    cairo_select_font_face (cr, "Sans",
			    CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);

    cairo_set_font_size (cr, 12.0);

    cairo_move_to (cr, x, y+4);
    cairo_set_source_rgba(cr, 0, 0, 0,1.0);
    cairo_show_text( cr, buf);

    /* free memory */
    cairo_destroy (cr);
}

void DrawSeekDot(int x, int y, int colorNr)
{
    cairo_t *cr = cairo_create (csBoardWindow);
    int square = colorNr & 0x80;
    colorNr &= 0x7F;

    if(square)
	cairo_rectangle (cr, x-squareSize/9, y-squareSize/9, 2*(squareSize/9), 2*(squareSize/9));
    else
	cairo_arc(cr, x, y, squareSize/9, 0.0, 2*M_PI);

    SetPen(cr, 2, "#000000", 0);
    cairo_stroke_preserve(cr);
    switch (colorNr) {
      case 0: cairo_set_source_rgba(cr, 1.0, 0, 0,1.0);	break;
      case 1: cairo_set_source_rgba (cr, 0.0, 0.7, 0.2, 1.0); break;
      default: cairo_set_source_rgba (cr, 1.0, 1.0, 0.0, 1.0); break;
    }
    cairo_fill(cr);

    /* free memory */
    cairo_destroy (cr);
}

void
DrawSeekOpen ()
{
    int boardWidth = lineGap + BOARD_WIDTH * (squareSize + lineGap);
    int boardHeight = lineGap + BOARD_HEIGHT * (squareSize + lineGap);
    if(!csBoardWindow) {
	csBoardWindow = GetOutputSurface(&mainOptions[W_BOARD], 0, 0);
	csBoardBackup = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, boardWidth, boardHeight);
    }
}

void
DrawSeekClose ()
{
}

void
CreateGrid ()
{
    int i, j;

    if (lineGap == 0) return;

    /* [HR] Split this into 2 loops for non-square boards. */

    for (i = 0; i < BOARD_HEIGHT + 1; i++) {
        gridSegments[i].x1 = 0;
        gridSegments[i].x2 =
          lineGap + BOARD_WIDTH * (squareSize + lineGap);
        gridSegments[i].y1 = gridSegments[i].y2
          = lineGap / 2 + (i * (squareSize + lineGap));
    }

    for (j = 0; j < BOARD_WIDTH + 1; j++) {
        gridSegments[j + i].y1 = 0;
        gridSegments[j + i].y2 =
          lineGap + BOARD_HEIGHT * (squareSize + lineGap);
        gridSegments[j + i].x1 = gridSegments[j + i].x2
          = lineGap / 2 + (j * (squareSize + lineGap));
    }
}

void
DoDrawGrid(cairo_surface_t *cs)
{
  /* draws a grid starting around Nx, Ny squares starting at x,y */
  int i;
  cairo_t *cr;

  DrawSeekOpen();
  /* get a cairo_t */
  cr = cairo_create (cs);

  cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);
  SetPen(cr, lineGap, "#000000", 0);

  /* lines in X */
  for (i = 0; i < BOARD_WIDTH + BOARD_HEIGHT + 2; i++)
    {
      cairo_move_to (cr, gridSegments[i].x1, gridSegments[i].y1);
      cairo_line_to (cr, gridSegments[i].x2, gridSegments[i].y2);
      cairo_stroke (cr);
    }

  /* free memory */
  cairo_destroy (cr);

  return;
}

void
DrawGrid()
{
  DoDrawGrid(csBoardWindow);
  if(!dual) DoDrawGrid(csBoardBackup);
}

void
DoDrawBorder (cairo_surface_t *cs, int x, int y, int type)
{
    cairo_t *cr;
    DrawSeekOpen();
    char *col;

    switch(type) {
	case 0: col = "#000000"; break;
	case 1: col = appData.highlightSquareColor; break;
	case 2: col = appData.premoveHighlightColor; break;
    }
    cr = cairo_create(cs);
    cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
    cairo_rectangle(cr, x, y, squareSize+lineGap, squareSize+lineGap);
    SetPen(cr, lineGap, col, 0);
    cairo_stroke(cr);
}

void
DrawBorder (int x, int y, int type)
{
  DoDrawBorder(csBoardWindow, x, y, type);
  if(!dual) DoDrawBorder(csBoardBackup, x, y, type);
}

static int
CutOutSquare (int x, int y, int *x0, int *y0, int  kind)
{
    int W = BOARD_WIDTH, H = BOARD_HEIGHT;
    int nx = x/(squareSize + lineGap), ny = y/(squareSize + lineGap);
    *x0 = 0; *y0 = 0;
    if(textureW[kind] < squareSize || textureH[kind] < squareSize) return 0;
    if(textureW[kind] < W*squareSize)
	*x0 = (textureW[kind] - squareSize) * nx/(W-1);
    else
	*x0 = textureW[kind]*nx / W + (textureW[kind] - W*squareSize) / (2*W);
    if(textureH[kind] < H*squareSize)
	*y0 = (textureH[kind] - squareSize) * ny/(H-1);
    else
	*y0 = textureH[kind]*ny / H + (textureH[kind] - H*squareSize) / (2*H);
    return 1;
}

void
DrawLogo (void *handle, void *logo)
{
    cairo_surface_t *img, *cs;
    cairo_t *cr;
    int w, h;

    if(!logo || !handle) return;
    cs = GetOutputSurface(handle, appData.logoSize, appData.logoSize/2);
    img = cairo_image_surface_create_from_png (logo);
    w = cairo_image_surface_get_width (img);
    h = cairo_image_surface_get_height (img);
    cr = cairo_create(cs);
    cairo_scale(cr, (float)appData.logoSize/w, appData.logoSize/(2.*h));
    cairo_set_source_surface (cr, img, 0, 0);
    cairo_paint (cr);
    cairo_destroy (cr);
    cairo_surface_destroy (img);
    cairo_surface_destroy (cs);
}

static void
BlankSquare (cairo_surface_t *dest, int x, int y, int color, ChessSquare piece, int fac)
{   // [HGM] extra param 'fac' for forcing destination to (0,0) for copying to animation buffer
    int x0, y0;
    cairo_t *cr;

    cr = cairo_create (dest);

    if ((useTexture & color+1) && CutOutSquare(x, y, &x0, &y0, color)) {
	    cairo_set_source_surface (cr, pngBoardBitmap[color], x*fac - x0, y*fac - y0);
	    cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);
	    cairo_rectangle (cr, x*fac, y*fac, squareSize, squareSize);
	    cairo_fill (cr);
	    cairo_destroy (cr);
    } else { // evenly colored squares
	char *col;
	switch (color) {
	  case 0: col = appData.darkSquareColor; break;
	  case 1: col = appData.lightSquareColor; break;
	  case 2: col = "#000000"; break;
	}
	SetPen(cr, 2.0, col, 0);
	cairo_rectangle (cr, x, y, squareSize, squareSize);
	cairo_fill (cr);
	cairo_destroy (cr);
    }
}

static void
pngDrawPiece (cairo_surface_t *dest, ChessSquare piece, int square_color, int x, int y)
{
    int kind, p = piece;
    cairo_t *cr;

    if ((int)piece < (int) BlackPawn) {
	kind = 0;
    } else {
	kind = 1;
	piece -= BlackPawn;
    }
    if(appData.upsideDown && flipView) { p += p < BlackPawn ? BlackPawn : -BlackPawn; }// swap white and black pieces
    BlankSquare(dest, x, y, square_color, piece, 1); // erase previous contents with background
    cr = cairo_create (dest);
    cairo_set_source_surface (cr, pngPieceBitmaps[kind][piece], x, y);
    cairo_paint(cr);
    cairo_destroy (cr);
}

void
DoDrawDot (cairo_surface_t *cs, int marker, int x, int y, int r)
{
	cairo_t *cr;

	cr = cairo_create(cs);
	cairo_arc(cr, x+r/2, y+r/2, r/2, 0.0, 2*M_PI);
	if(appData.monoMode) {
	    SetPen(cr, 2, marker == 2 ? "#000000" : "#FFFFFF", 0);
	    cairo_stroke_preserve(cr);
	    SetPen(cr, 2, marker == 2 ? "#FFFFFF" : "#000000", 0);
	} else {
	    SetPen(cr, 2, marker == 2 ? "#FF0000" : "#FFFF00", 0);
	}
	cairo_fill(cr);

	cairo_destroy(cr);
}

void
DrawDot (int marker, int x, int y, int r)
{ // used for atomic captures; no need to draw on backup
  DrawSeekOpen();
  DoDrawDot(csBoardWindow, marker, x, y, r);
}

static void
DoDrawOneSquare (cairo_surface_t *dest, int x, int y, ChessSquare piece, int square_color, int marker, char *string, int align)
{   // basic front-end board-draw function: takes care of everything that can be in square:
    // piece, background, coordinate/count, marker dot
    cairo_t *cr;

    if (piece == EmptySquare) {
	BlankSquare(dest, x, y, square_color, piece, 1);
    } else {
	pngDrawPiece(dest, piece, square_color, x, y);
    }

    if(align) { // square carries inscription (coord or piece count)
	int xx = x, yy = y;
	cairo_text_extents_t te;

	cr = cairo_create (dest);
	cairo_select_font_face (cr, "Sans",
		    CAIRO_FONT_SLANT_NORMAL,
		    CAIRO_FONT_WEIGHT_BOLD);

	cairo_set_font_size (cr, squareSize/4);
	// calculate where it goes
	cairo_text_extents (cr, string, &te);

	if (align == 1) {
	    xx += squareSize - te.width - te.x_bearing - 1;
	    yy += squareSize - te.height - te.y_bearing - 1;
	} else if (align == 2) {
	    xx += te.x_bearing + 1, yy += -te.y_bearing + 1;
	} else if (align == 3) {
	    xx += squareSize - te.width -te.x_bearing - 1;
	    yy += -te.y_bearing + 3;
	} else if (align == 4) {
	    xx += te.x_bearing + 1, yy += -te.y_bearing + 3;
	}

	cairo_move_to (cr, xx-1, yy);
	if(align < 3) cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
	else          cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
	cairo_show_text (cr, string);
	cairo_destroy (cr);
    }

    if(marker) { // print fat marker dot, if requested
	DoDrawDot(dest, marker, x + squareSize/4, y+squareSize/4, squareSize/2);
    }
}

void
DrawOneSquare (int x, int y, ChessSquare piece, int square_color, int marker, char *string, int align)
{
  DrawSeekOpen();
  DoDrawOneSquare (csBoardWindow, x, y, piece, square_color, marker, string, align);
  if(!dual)
  DoDrawOneSquare (csBoardBackup, x, y, piece, square_color, marker, string, align);
}

/****	Animation code by Hugh Fisher, DCS, ANU. ****/

/*	Masks for XPM pieces. Black and white pieces can have
	different shapes, but in the interest of retaining my
	sanity pieces must have the same outline on both light
	and dark squares, and all pieces must use the same
	background square colors/images.		*/

static cairo_surface_t *c_animBufs[3*NrOfAnims]; // newBuf, saveBuf

static void
InitAnimState (AnimNr anr)
{
    DrawSeekOpen(); // set cs to board widget
    if(c_animBufs[anr]) cairo_surface_destroy (c_animBufs[anr]);
    if(c_animBufs[anr+2]) cairo_surface_destroy (c_animBufs[anr+2]);
    c_animBufs[anr+4] = csBoardWindow;
    c_animBufs[anr+2] = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, squareSize, squareSize);
    c_animBufs[anr] = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, squareSize, squareSize);
}

void
CreateAnimVars ()
{
  InitAnimState(Game);
  InitAnimState(Player);
}

static void
CairoOverlayPiece (ChessSquare piece, cairo_surface_t *dest)
{
  static cairo_t *pieceSource;
  pieceSource = cairo_create (dest);
  cairo_set_source_surface (pieceSource, pngPieceBitmaps[!White(piece)][piece % BlackPawn], 0, 0);
  if(doubleClick) cairo_paint_with_alpha (pieceSource, 0.6);
  else cairo_paint(pieceSource);
  cairo_destroy (pieceSource);
}

void
InsertPiece (AnimNr anr, ChessSquare piece)
{
    CairoOverlayPiece(piece, c_animBufs[anr]);
}

void
DrawBlank (AnimNr anr, int x, int y, int startColor)
{
    BlankSquare(c_animBufs[anr+2], x, y, startColor, EmptySquare, 0);
}

void CopyRectangle (AnimNr anr, int srcBuf, int destBuf,
		 int srcX, int srcY, int width, int height, int destX, int destY)
{
	cairo_t *cr;// = cairo_create (c_animBufs[anr+destBuf]);
	cr = cairo_create (c_animBufs[anr+destBuf]);
	if(c_animBufs[anr+srcBuf] == csBoardWindow)
	cairo_set_source_surface (cr, csBoardBackup, destX - srcX, destY - srcY);
	else
	cairo_set_source_surface (cr, c_animBufs[anr+srcBuf], destX - srcX, destY - srcY);
	cairo_rectangle (cr, destX, destY, width, height);
	cairo_fill (cr);
	cairo_destroy (cr);
	if(c_animBufs[anr+destBuf] == csBoardWindow) {
	cr = cairo_create (csBoardBackup); // also draw to backup
	cairo_set_source_surface (cr, c_animBufs[anr+srcBuf], destX - srcX, destY - srcY);
	cairo_rectangle (cr, destX, destY, width, height);
	cairo_fill (cr);
	cairo_destroy (cr);
	}
}

void
SetDragPiece (AnimNr anr, ChessSquare piece)
{
}

/* [AS] Arrow highlighting support */

void
DoDrawPolygon (cairo_surface_t *cs, Pnt arrow[], int nr)
{
    cairo_t *cr;
    int i;
    cr = cairo_create (cs);
    cairo_move_to (cr, arrow[nr-1].x, arrow[nr-1].y);
    for (i=0;i<nr;i++) {
        cairo_line_to(cr, arrow[i].x, arrow[i].y);
    }
    if(appData.monoMode) { // should we always outline arrow?
        cairo_line_to(cr, arrow[0].x, arrow[0].y);
        SetPen(cr, 2, "#000000", 0);
        cairo_stroke_preserve(cr);
    }
    SetPen(cr, 2, appData.highlightSquareColor, 0);
    cairo_fill(cr);

    /* free memory */
    cairo_destroy (cr);
}

void
DrawPolygon (Pnt arrow[], int nr)
{
    DoDrawPolygon(csBoardWindow, arrow, nr);
    if(!dual) DoDrawPolygon(csBoardBackup, arrow, nr);
}


