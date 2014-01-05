/*
 * draw.c -- drawing routines for XBoard
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

#include "config.h"

#include <stdio.h>
#include <math.h>
#include <cairo/cairo.h>
#include <cairo/cairo-xlib.h>
#include <librsvg/rsvg.h>
#include <librsvg/rsvg-cairo.h>

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

#include "common.h"

#include "backend.h"
#include "board.h"
#include "menus.h"
#include "dialogs.h"
#include "evalgraph.h"
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
Option *currBoard;
cairo_surface_t *csBoardWindow;
static cairo_surface_t *pngPieceImages[2][(int)BlackPawn+4];   // png 256 x 256 images
static cairo_surface_t *pngPieceBitmaps[2][(int)BlackPawn];    // scaled pieces as used
static cairo_surface_t *pngPieceBitmaps2[2][(int)BlackPawn+4]; // scaled pieces in store
static RsvgHandle *svgPieces[2][(int)BlackPawn+4]; // vector pieces in store
static cairo_surface_t *pngBoardBitmap[2];
int useTexture, textureW[2], textureH[2];

#define pieceToSolid(piece) &pieceBitmap[SOLID][(piece) % (int)BlackPawn]
#define pieceToOutline(piece) &pieceBitmap[OUTLINE][(piece) % (int)BlackPawn]

#define White(piece) ((int)(piece) < (int)BlackPawn)

char *crWhite = "#FFFFB0";
char *crBlack = "#AD5D3D";

struct {
  int x1, x2, y1, y2;
} gridSegments[BOARD_RANKS + BOARD_FILES + 2];

void
SwitchWindow (int main)
{
    currBoard = (main ? &mainOptions[W_BOARD] : &dualOptions[3]);
    csBoardWindow = DRAWABLE(currBoard);
}

void
SelectPieces(VariantClass v)
{
    int i;
    for(i=0; i<2; i++) {
	int p;
	for(p=0; p<=(int)WhiteKing; p++)
	   pngPieceBitmaps[i][p] = pngPieceBitmaps2[i][p]; // defaults
	if(v == VariantShogi) {
	   pngPieceBitmaps[i][(int)WhiteCannon] = pngPieceBitmaps2[i][(int)WhiteKing+1];
	   pngPieceBitmaps[i][(int)WhiteNightrider] = pngPieceBitmaps2[i][(int)WhiteKing+2];
	   pngPieceBitmaps[i][(int)WhiteSilver] = pngPieceBitmaps2[i][(int)WhiteKing+3];
	   pngPieceBitmaps[i][(int)WhiteGrasshopper] = pngPieceBitmaps2[i][(int)WhiteKing+4];
	   pngPieceBitmaps[i][(int)WhiteQueen] = pngPieceBitmaps2[i][(int)WhiteLance];
	}
#ifdef GOTHIC
	if(v == VariantGothic) {
	   pngPieceBitmaps[i][(int)WhiteMarshall] = pngPieceBitmaps2[i][(int)WhiteSilver];
	}
#endif
	if(v == VariantSChess) {
	   pngPieceBitmaps[i][(int)WhiteAngel]    = pngPieceBitmaps2[i][(int)WhiteFalcon];
	   pngPieceBitmaps[i][(int)WhiteMarshall] = pngPieceBitmaps2[i][(int)WhiteAlfil];
	}
    }
}

#define BoardSize int
void
InitDrawingSizes (BoardSize boardSize, int flags)
{   // [HGM] resize is functional now, but for board format changes only (nr of ranks, files)
    int boardWidth, boardHeight;
    static int oldWidth, oldHeight;
    static VariantClass oldVariant;
    static int oldTwoBoards = 0;

    if(!mainOptions[W_BOARD].handle) return;

    if(oldTwoBoards && !twoBoards) PopDown(DummyDlg);
    oldTwoBoards = twoBoards;

    if(appData.overrideLineGap >= 0) lineGap = appData.overrideLineGap;
    boardWidth = lineGap + BOARD_WIDTH * (squareSize + lineGap);
    boardHeight = lineGap + BOARD_HEIGHT * (squareSize + lineGap);

  if(boardWidth != oldWidth || boardHeight != oldHeight) { // do resizing stuff only if size actually changed

    oldWidth = boardWidth; oldHeight = boardHeight;
    CreateGrid();

    /*
     * Inhibit shell resizing.
     */
    ResizeBoardWindow(boardWidth, boardHeight, 0);

    DelayedDrag();
  }

    // [HGM] pieces: tailor piece bitmaps to needs of specific variant
    // (only for xpm)

  if(gameInfo.variant != oldVariant) { // and only if variant changed

    SelectPieces(gameInfo.variant);

    oldVariant = gameInfo.variant;
  }
  CreateAnimVars();
}

void
ExposeRedraw (Option *graph, int x, int y, int w, int h)
{   // copy a selected part of the buffer bitmap to the display
    cairo_t *cr = cairo_create((cairo_surface_t *) graph->textValue);
    cairo_set_source_surface(cr, (cairo_surface_t *) graph->choice, 0, 0);
    cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
    cairo_rectangle(cr, x, y, w, h);
    cairo_fill(cr);
    cairo_destroy(cr);
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

RsvgHandle *
LoadSVG (char *dir, int color, int piece)
{
    char buf[MSG_SIZ];
  RsvgHandle *svg=svgPieces[color][piece];
  RsvgDimensionData svg_dimensions;
  GError *svgerror=NULL;
  cairo_surface_t *img;
  cairo_t *cr;

    snprintf(buf, MSG_SIZ, "%s/%s%s.svg", dir, color ? "Black" : "White", pngPieceNames[piece]);

    if(svg || *dir && (svg = rsvg_handle_new_from_file(buf, &svgerror))) {

      rsvg_handle_get_dimensions(svg, &svg_dimensions);
      img = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, squareSize,  squareSize);

      cr = cairo_create(img);
      cairo_scale(cr, squareSize/(double) svg_dimensions.width, squareSize/(double) svg_dimensions.height);
      rsvg_handle_render_cairo(svg, cr);
      if(cairo_surface_status(img) == CAIRO_STATUS_SUCCESS) {
        if(pngPieceImages[color][piece]) cairo_surface_destroy(pngPieceImages[color][piece]);
        pngPieceImages[color][piece] = img;
      }
      cairo_destroy(cr);

      return svg;
    }
    if(svgerror)
	g_error_free(svgerror);
    return NULL;
}

static void
ScaleOnePiece (int color, int piece)
{
  float w, h;
  char buf[MSG_SIZ];
  cairo_surface_t *img, *cs;
  cairo_t *cr;

  g_type_init ();

  svgPieces[color][piece] = LoadSVG("", color, piece); // this fills pngPieceImages if we had cached svg with bitmap of wanted size

  if(!pngPieceImages[color][piece]) { // we don't have cached bitmap (implying we did not have cached svg)
    if(*appData.pieceDirectory) { // user specified piece directory
      snprintf(buf, MSG_SIZ, "%s/%s%s.png", appData.pieceDirectory, color ? "Black" : "White", pngPieceNames[piece]);
      img = cairo_image_surface_create_from_png (buf); // try if there are png pieces there
      if(cairo_surface_status(img) != CAIRO_STATUS_SUCCESS) { // there were not
	svgPieces[color][piece] = LoadSVG(appData.pieceDirectory, color, piece); // so try if he has svg there
      } else pngPieceImages[color][piece] = img;
    }
  }

  if(!pngPieceImages[color][piece]) { // we still did not manage to acquire a piece bitmap
    static int warned = 0;
    if(!(svgPieces[color][piece] = LoadSVG(SVGDIR, color, piece)) && !warned) { // try to fall back on installed svg
      char *msg = _("No default pieces installed\nSelect your own -pieceImageDirectory");
      printf("%s\n", msg); // give up
      DisplayError(msg, 0);
      warned = 1; // prevent error message being repeated for each piece type
    }
  }

  img = pngPieceImages[color][piece];

  // create new bitmap to hold scaled piece image (and remove any old)
  if(pngPieceBitmaps2[color][piece]) cairo_surface_destroy (pngPieceBitmaps2[color][piece]);
  pngPieceBitmaps2[color][piece] = cs = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, squareSize, squareSize);

  if(!img) return;

  // scaled copying of the raw png image
  cr = cairo_create(cs);
  w = cairo_image_surface_get_width (img);
  h = cairo_image_surface_get_height (img);
  cairo_scale(cr, squareSize/w, squareSize/h);
  cairo_set_source_surface (cr, img, 0, 0);
  cairo_paint (cr);
  cairo_destroy (cr);

  if(!appData.trueColors || !*appData.pieceDirectory) { // operate on bitmap to color it (king-size hack...)
    int stride = cairo_image_surface_get_stride(cs)/4;
    int *buf = (int *) cairo_image_surface_get_data(cs);
    int i, j, p;
    sscanf(color ? appData.blackPieceColor+1 : appData.whitePieceColor+1, "%x", &p); // replacement color
    cairo_surface_flush(cs);
    for(i=0; i<squareSize; i++) for(j=0; j<squareSize; j++) {
	int r, a;
	float f;
	unsigned int c = buf[i*stride + j];
	a = c >> 24; r = c >> 16 & 255;     // alpha and red, where red is the 'white' weight, since white is #FFFFCC in the source images
        f = (color ? a - r : r)/255.;       // fraction of black or white in the mix that has to be replaced
	buf[i*stride + j] = c & 0xFF000000; // alpha channel is kept at same opacity
	buf[i*stride + j] += ((int)(f*(p&0xFF0000)) & 0xFF0000) + ((int)(f*(p&0xFF00)) & 0xFF00) + (int)(f*(p&0xFF)); // add desired fraction of new color
	if(color) buf[i*stride + j] += r | r << 8 | r << 16; // details on black pieces get their weight added in pure white
	if(appData.monoMode) {
	    if(a < 64) buf[i*stride + j] = 0; // if not opaque enough, totally transparent
	    else if(2*r < a) buf[i*stride + j] = 0xFF000000; // if not light enough, totally black
            else buf[i*stride + j] = 0xFFFFFFFF; // otherwise white
	}
    }
    cairo_surface_mark_dirty(cs);
  }
}

void
CreatePNGPieces ()
{
  int p;

  for(p=0; pngPieceNames[p]; p++) {
    ScaleOnePiece(0, p);
    ScaleOnePiece(1, p);
  }
  SelectPieces(gameInfo.variant);
}

void
CreateAnyPieces ()
{   // [HGM] taken out of main
    CreatePNGPieces();
    CreatePNGBoard(appData.liteBackTextureFile, 1);
    CreatePNGBoard(appData.darkBackTextureFile, 0);
}

void
InitDrawingParams (int reloadPieces)
{
    int i, p;
    if(reloadPieces)
    for(i=0; i<2; i++) for(p=0; p<BlackPawn+4; p++) {
	if(pngPieceImages[i][p]) cairo_surface_destroy(pngPieceImages[i][p]);
	pngPieceImages[i][p] = NULL;
	if(svgPieces[i][p]) rsvg_handle_close(svgPieces[i][p], NULL);
	svgPieces[i][p] = NULL;
    }
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
    GraphExpose(currBoard, x-1, yTo-1, xTo-x+2, y-yTo+2);
}

void DrawSeekBackground( int left, int top, int right, int bottom )
{
    cairo_t *cr = cairo_create (csBoardWindow);

    cairo_rectangle (cr, left, top, right-left, bottom-top);

    cairo_set_source_rgba(cr, 0.8, 0.8, 0.4,1.0);
    cairo_fill(cr);

    /* free memory */
    cairo_destroy (cr);
    GraphExpose(currBoard, left, top, right-left, bottom-top);
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
    GraphExpose(currBoard, x-5, y-10, 60, 15);
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
    GraphExpose(currBoard, x-squareSize/8, y-squareSize/8, 2*(squareSize/8), 2*(squareSize/8));
}

void
InitDrawingHandle (Option *opt)
{
    csBoardWindow = DRAWABLE(opt);
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
DrawGrid()
{
  /* draws a grid starting around Nx, Ny squares starting at x,y */
  int i;
  float odd = (lineGap & 1)/2.;
  cairo_t *cr;

  /* get a cairo_t */
  cr = cairo_create (csBoardWindow);

  cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);
  SetPen(cr, lineGap, "#000000", 0);

  /* lines in X */
  for (i = 0; i < BOARD_WIDTH + BOARD_HEIGHT + 2; i++)
    {
      int h = (gridSegments[i].y1 == gridSegments[i].y2); // horizontal
      cairo_move_to (cr, gridSegments[i].x1 + !h*odd, gridSegments[i].y1 + h*odd);
      cairo_line_to (cr, gridSegments[i].x2 + !h*odd, gridSegments[i].y2 + h*odd);
      cairo_stroke (cr);
    }

  /* free memory */
  cairo_destroy (cr);

  return;
}

void
DrawBorder (int x, int y, int type, int odd)
{
    cairo_t *cr;
    char *col;

    switch(type) {
	case 0: col = "#000000"; break;
	case 1: col = appData.highlightSquareColor; break;
	case 2: col = appData.premoveHighlightColor; break;
	default: col = "#808080"; break; // cannot happen
    }
    cr = cairo_create(csBoardWindow);
    cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
    cairo_rectangle(cr, x+odd/2., y+odd/2., squareSize+lineGap, squareSize+lineGap);
    SetPen(cr, lineGap, col, 0);
    cairo_stroke(cr);
    cairo_destroy(cr);
    GraphExpose(currBoard, x - lineGap/2, y - lineGap/2, squareSize+2*lineGap+odd, squareSize+2*lineGap+odd);
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
DrawLogo (Option *opt, void *logo)
{
    cairo_surface_t *img;
    cairo_t *cr;
    int w, h;

    if(!logo || !opt) return;
    img = cairo_image_surface_create_from_png (logo);
    w = cairo_image_surface_get_width (img);
    h = cairo_image_surface_get_height (img);
    cr = cairo_create(DRAWABLE(opt));
//    cairo_scale(cr, (float)appData.logoSize/w, appData.logoSize/(2.*h));
    cairo_scale(cr, (float)opt->max/w, (float)opt->value/h);
    cairo_set_source_surface (cr, img, 0, 0);
    cairo_paint (cr);
    cairo_destroy (cr);
    cairo_surface_destroy (img);
    GraphExpose(opt, 0, 0, appData.logoSize, appData.logoSize/2);
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
	char *col = NULL;
	switch (color) {
	  case 0: col = appData.darkSquareColor; break;
	  case 1: col = appData.lightSquareColor; break;
	  case 2: col = "#000000"; break;
	  default: col = "#808080"; break; // cannot happen
	}
	SetPen(cr, 2.0, col, 0);
	cairo_rectangle (cr, fac*x, fac*y, squareSize, squareSize);
	cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);
	cairo_fill (cr);
	cairo_destroy (cr);
    }
}

static void
pngDrawPiece (cairo_surface_t *dest, ChessSquare piece, int square_color, int x, int y)
{
    int kind;
    cairo_t *cr;

    if ((int)piece < (int) BlackPawn) {
	kind = 0;
    } else {
	kind = 1;
	piece -= BlackPawn;
    }
    if(appData.upsideDown && flipView) kind = 1 - kind; // swap white and black pieces
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
  DoDrawDot(csBoardWindow, marker, x, y, r);
  GraphExpose(currBoard, x-r, y-r, 2*r, 2*r);
}

static void
DrawText (char *string, int x, int y, int align)
{
	int xx = x, yy = y;
	cairo_text_extents_t te;
	cairo_t *cr;

	cr = cairo_create (csBoardWindow);
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

void
DrawOneSquare (int x, int y, ChessSquare piece, int square_color, int marker, char *tString, char *bString, int align)
{   // basic front-end board-draw function: takes care of everything that can be in square:
    // piece, background, coordinate/count, marker dot

    if (piece == EmptySquare) {
	BlankSquare(csBoardWindow, x, y, square_color, piece, 1);
    } else {
	pngDrawPiece(csBoardWindow, piece, square_color, x, y);
    }

    if(align) { // square carries inscription (coord or piece count)
	if(align > 1) DrawText(tString, x, y, align);       // top (rank or count)
	if(bString && *bString) DrawText(bString, x, y, 1); // bottom (always lower right file ID)
    }

    if(marker) { // print fat marker dot, if requested
	DoDrawDot(csBoardWindow, marker, x + squareSize/4, y+squareSize/4, squareSize/2);
    }
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
	cairo_t *cr;
	c_animBufs[anr+4] = csBoardWindow;
	cr = cairo_create (c_animBufs[anr+destBuf]);
	cairo_set_source_surface (cr, c_animBufs[anr+srcBuf], destX - srcX, destY - srcY);
	cairo_rectangle (cr, destX, destY, width, height);
	cairo_fill (cr);
	cairo_destroy (cr);
	if(c_animBufs[anr+destBuf] == csBoardWindow) // suspect that GTK needs this!
	    GraphExpose(currBoard, destX, destY, width, height);
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
//    if(!dual) DoDrawPolygon(csBoardBackup, arrow, nr);
}

//-------------------- Eval Graph drawing routines (formerly in xevalgraph.h) --------------------

static void
ChoosePen(cairo_t *cr, int i)
{
  switch(i) {
    case PEN_BLACK:
      SetPen(cr, 1.0, "#000000", 0);
      break;
    case PEN_DOTTED:
      SetPen(cr, 1.0, "#A0A0A0", 1);
      break;
    case PEN_BLUEDOTTED:
      SetPen(cr, 1.0, "#0000FF", 1);
      break;
    case PEN_BOLDWHITE:
      SetPen(cr, 3.0, crWhite, 0);
      break;
    case PEN_BOLDBLACK:
      SetPen(cr, 3.0, crBlack, 0);
      break;
    case PEN_BACKGD:
      SetPen(cr, 3.0, "#E0E0F0", 0);
      break;
  }
}

// [HGM] front-end, added as wrapper to avoid use of LineTo and MoveToEx in other routines (so they can be back-end)
void
DrawSegment (int x, int y, int *lastX, int *lastY, int penType)
{
  static int curX, curY;

  if(penType != PEN_NONE) {
    cairo_t *cr = cairo_create(DRAWABLE(disp));
    cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);
    cairo_move_to (cr, curX, curY);
    cairo_line_to (cr, x,y);
    ChoosePen(cr, penType);
    cairo_stroke (cr);
    cairo_destroy (cr);
  }

  if(lastX != NULL) { *lastX = curX; *lastY = curY; }
  curX = x; curY = y;
}

// front-end wrapper for drawing functions to do rectangles
void
DrawRectangle (int left, int top, int right, int bottom, int side, int style)
{
  cairo_t *cr;

  cr = cairo_create (DRAWABLE(disp));
  cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);
  cairo_rectangle (cr, left, top, right-left, bottom-top);
  switch(side)
    {
    case 0: ChoosePen(cr, PEN_BOLDWHITE); break;
    case 1: ChoosePen(cr, PEN_BOLDBLACK); break;
    case 2: ChoosePen(cr, PEN_BACKGD); break;
    }
  cairo_fill (cr);

  if(style != FILLED)
    {
      cairo_rectangle (cr, left, top, right-left-1, bottom-top-1);
      ChoosePen(cr, PEN_BLACK);
      cairo_stroke (cr);
    }

  cairo_destroy(cr);
}

// front-end wrapper for putting text in graph
void
DrawEvalText (char *buf, int cbBuf, int y)
{
    // the magic constants 8 and 5 should really be derived from the font size somehow
  cairo_text_extents_t extents;
  cairo_t *cr = cairo_create(DRAWABLE(disp));

  /* GTK-TODO this has to go into the font-selection */
  cairo_select_font_face (cr, "Sans",
			  CAIRO_FONT_SLANT_NORMAL,
			  CAIRO_FONT_WEIGHT_NORMAL);
  cairo_set_font_size (cr, 12.0);


  cairo_text_extents (cr, buf, &extents);

  cairo_move_to (cr, MarginX - 2 - 8*cbBuf, y+5);
  cairo_text_path (cr, buf);
  cairo_set_source_rgb (cr, 0.0, 0.0, 0);
  cairo_fill_preserve (cr);
  cairo_set_source_rgb (cr, 0, 1.0, 0);
  cairo_set_line_width (cr, 0.1);
  cairo_stroke (cr);

  /* free memory */
  cairo_destroy (cr);
}
