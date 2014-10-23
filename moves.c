/*
 * moves.c - Move generation and checking
 *
 * Copyright 1991 by Digital Equipment Corporation, Maynard,
 * Massachusetts.
 *
 * Enhancements Copyright 1992-2001, 2002, 2003, 2004, 2005, 2006,
 * 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014 Free Software Foundation, Inc.
 *
 * Enhancements Copyright 2005 Alessandro Scotti
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
#include <ctype.h>
#include <stdlib.h>
#if HAVE_STRING_H
# include <string.h>
#else /* not HAVE_STRING_H */
# include <strings.h>
#endif /* not HAVE_STRING_H */
#include "common.h"
#include "backend.h"
#include "moves.h"
#include "parser.h"

int WhitePiece P((ChessSquare));
int BlackPiece P((ChessSquare));
int SameColor P((ChessSquare, ChessSquare));
int PosFlags(int index);

extern signed char initialRights[BOARD_FILES]; /* [HGM] all rights enabled, set in InitPosition */
int quickFlag;
char *pieceDesc[EmptySquare];
char *defaultDesc[EmptySquare] = {
 "fmWfceFifmnD", "N", "B", "R", "Q",
 "F", "A", "BN", "RN", "W", "K",
 "mRcpR", "N0", "BW", "RF", "gQ",
 "", "", "QN", "", "N", "",
 "", "", "", "", "",
 "", "", "", "", "", "",
 "", "", "", "", "",
 "", "", "", "", "", "K"
};

int
WhitePiece (ChessSquare piece)
{
    return (int) piece >= (int) WhitePawn && (int) piece < (int) BlackPawn;
}

int
BlackPiece (ChessSquare piece)
{
    return (int) piece >= (int) BlackPawn && (int) piece < (int) EmptySquare;
}

#if 0
int
SameColor (ChessSquare piece1, ChessSquare piece2)
{
    return ((int) piece1 >= (int) WhitePawn &&   /* [HGM] can be > King ! */
            (int) piece1 <  (int) BlackPawn &&
	    (int) piece2 >= (int) WhitePawn &&
            (int) piece2 <  (int) BlackPawn)
      ||   ((int) piece1 >= (int) BlackPawn &&
            (int) piece1 <  (int) EmptySquare &&
	    (int) piece2 >= (int) BlackPawn &&
            (int) piece2 <  (int) EmptySquare);
}
#else
#define SameColor(piece1, piece2) (piece1 < EmptySquare && piece2 < EmptySquare && (piece1 < BlackPawn) == (piece2 < BlackPawn) || piece1 == DarkSquare || piece2 == DarkSquare)
#endif

char pieceToChar[] = {
                        'P', 'N', 'B', 'R', 'Q', 'F', 'E', 'A', 'C', 'W', 'M',
                        'O', 'H', 'I', 'J', 'G', 'D', 'V', 'L', 'S', 'U', 'K',
                        'p', 'n', 'b', 'r', 'q', 'f', 'e', 'a', 'c', 'w', 'm',
                        'o', 'h', 'i', 'j', 'g', 'd', 'v', 'l', 's', 'u', 'k',
                        'x' };
char pieceNickName[EmptySquare];

char
PieceToChar (ChessSquare p)
{
    if((int)p < 0 || (int)p >= (int)EmptySquare) return('x'); /* [HGM] for safety */
    return pieceToChar[(int) p];
}

int
PieceToNumber (ChessSquare p)  /* [HGM] holdings: count piece type, ignoring non-participating piece types */
{
    int i=0;
    ChessSquare start = (int)p >= (int)BlackPawn ? BlackPawn : WhitePawn;

    while(start++ != p) if(pieceToChar[start-1] != '.' && pieceToChar[start-1] != '+') i++;
    return i;
}

ChessSquare
CharToPiece (int c)
{
     int i;
     if(c == '.') return EmptySquare;
     for(i=0; i< (int) EmptySquare; i++)
          if(pieceNickName[i] == c) return (ChessSquare) i;
     for(i=0; i< (int) EmptySquare; i++)
          if(pieceToChar[i] == c) return (ChessSquare) i;
     return EmptySquare;
}

void
CopyBoard (Board to, Board from)
{
    int i, j;

    for (i = 0; i < BOARD_HEIGHT; i++)
      for (j = 0; j < BOARD_WIDTH; j++)
	to[i][j] = from[i][j];
    for (j = 0; j < BOARD_FILES; j++) // [HGM] gamestate: copy castling rights and ep status
	to[VIRGIN][j] = from[VIRGIN][j],
	to[CASTLING][j] = from[CASTLING][j];
    to[HOLDINGS_SET] = 0; // flag used in ICS play
}

int
CompareBoards (Board board1, Board board2)
{
    int i, j;

    for (i = 0; i < BOARD_HEIGHT; i++)
      for (j = 0; j < BOARD_WIDTH; j++) {
	  if (board1[i][j] != board2[i][j])
	    return FALSE;
    }
    return TRUE;
}

char defaultName[] = "PNBRQ......................................K"  // white
                     "pnbrq......................................k"; // black
char shogiName[]   = "PNBRLS...G.++++++..........................K"  // white
                     "pnbrls...g.++++++..........................k"; // black
char xqName[]      = "PH.R.AE..K.C................................"  // white
                     "ph.r.ae..k.c................................"; // black

char *
CollectPieceDescriptors ()
{   // make a line of piece descriptions for use in the PGN Piece tag:
    // dump all engine defined pieces, and pieces with non-standard names,
    // but suppress black pieces that are the same as their white counterpart
    ChessSquare p;
    static char buf[MSG_SIZ];
    char *m, c, d, *pieceName = defaultName;
    int len;
    *buf = NULLCHAR;
    if(!pieceDefs) return "";
    if(gameInfo.variant == VariantChu) return ""; // for now don't do this for Chu Shogi
    if(gameInfo.variant == VariantShogi) pieceName = shogiName;
    if(gameInfo.variant == VariantXiangqi) pieceName = xqName;
    for(p=WhitePawn; p<EmptySquare; p++) {
	if((c = pieceToChar[p]) == '.' || c == '~') continue;  // does not participate
	m = pieceDesc[p]; d = (c == '+' ? pieceToChar[DEMOTED p] : c);
	if(p >= BlackPawn && pieceToChar[BLACK_TO_WHITE p] == toupper(c)
             && (c != '+' || pieceToChar[DEMOTED BLACK_TO_WHITE p] == d)) { // black member of normal pair
	    char *wm = pieceDesc[BLACK_TO_WHITE p];
	    if(!m && !wm || m && wm && !strcmp(wm, m)) continue;            // moves as a white piece
	} else                                                              // white or unpaired black
	if((p < BlackPawn || CharToPiece(toupper(d)) != EmptySquare) &&     // white or lone black
	   !pieceDesc[p] /*&& pieceName[p] == c*/) continue; // orthodox piece known by its usual name
// TODO: listing pieces because of unusual name can only be done if we have accurate Betza of all defaults
	if(!m) m = defaultDesc[p];
	len = strlen(buf);
	snprintf(buf+len, MSG_SIZ-len, "%s%s%c:%s", len ? ";" : "", c == '+' ? "+" : "", d, m);
    }
    return buf;
}

// [HGM] gen: configurable move generation from Betza notation sent by engine.
// Some notes about two-leg moves: GenPseudoLegal() works in two modes, depending on whether a 'kill-
// square has been set: without one is generates all moves, and a global int legNr flags in bits 0 and 1
// if the move has 1 or 2 legs. Only the marking of squares makes use of this info, by only marking
// target squares of leg 1 (rejecting null move). A dummy move with MoveType 'FirstLeg' to the relay square
// is generated, so a cyan marker can be put there, and other functions can ignore such a move. When the
// user selects this square, it becomes the kill-square. Once a kill-square is set, only 2-leg moves are
// generated that use that square as relay, plus 1-leg moves, so the 1-leg move that goes to the kill-square
// can be marked during 2nd-leg entry to terminate the move there. For judging the pseudo-legality of the
// 2nd leg, the from-square has to be considered empty, although the moving piece is still on it.

Boolean pieceDefs;

//  alphabet      "abcdefghijklmnopqrstuvwxyz"
char symmetry[] = "FBNW.FFW.NKN.NW.QR....W..N";
char xStep[]    = "2110.130.102.10.00....0..2";
char yStep[]    = "2132.133.313.20.11....1..3";
char dirType[]  = "01000104000200000260050000";
char upgrade[]  = "AFCD.BGH.JQL.NO.KW....R..Z";
char rotate[]   = "DRCA.WHG.JKL.NO.QB....F..Z";

//  alphabet   "a b    c d e f    g h    i j k l    m n o p q r    s    t u v    w x y z "
int dirs1[] = { 0,0x3C,0,0,0,0xC3,0,0,   0,0,0,0xF0,0,0,0,0,0,0x0F,0   ,0,0,0   ,0,0,0,0 };
int dirs2[] = { 0,0x18,0,0,0,0x81,0,0xFF,0,0,0,0x60,0,0,0,0,0,0x06,0x66,0,0,0x99,0,0,0,0 };
int dirs3[] = { 0,0x38,0,0,0,0x83,0,0xFF,0,0,0,0xE0,0,0,0,0,0,0x0E,0xEE,0,0,0xBB,0,0,0,0 };
int dirs4[] = { 0,0x10,0,0,0,0x01,0,0xFF,0,0,0,0x40,0,0,0,0,0,0x04,0x44,0,0,0x11,0,0,0,0 };

int rot[][4] = { // rotation matrices for each direction
  { 1, 0, 0, 1 },
  { 0, 1, 1, 0 },
  { 0, 1,-1, 0 },
  { 1, 0, 0,-1 },
  {-1, 0, 0,-1 },
  { 0,-1,-1, 0 },
  { 0,-1, 1, 0 },
  {-1, 0, 0, 1 }
};

void
OK (Board board, int flags, ChessMove kind, int rf, int ff, int rt, int ft, VOIDSTAR cl)
{
    (*(int*)cl)++;
}

void
MovesFromString (Board board, int flags, int f, int r, int tx, int ty, int angle, char *desc, MoveCallback cb, VOIDSTAR cl)
{
    char buf[80], *p = desc, *atom = NULL;
    int mine, his, dir, bit, occup, i, promoRank = -1;
    ChessMove promo= NormalMove; ChessSquare pc = board[r][f];
    if(flags & F_WHITE_ON_MOVE) his = 2, mine = 1; else his = 1, mine = 2;
    if(pc == WhitePawn || pc == WhiteLance) promo = WhitePromotion, promoRank = BOARD_HEIGHT-1; else
    if(pc == BlackPawn || pc == BlackLance) promo = BlackPromotion, promoRank = 0;
    while(*p) {                  // more moves to go
	int expo = 1, dx, dy, x, y, mode, dirSet, ds2=0, retry=0, initial=0, jump=1, skip = 0, all = 0;
	char *cont = NULL;
	if(*p == 'i') initial = 1, desc = ++p;
	while(islower(*p)) p++;  // skip prefixes
	if(!isupper(*p)) return; // syntax error: no atom
	dx = xStep[*p-'A'] - '0';// step vector of atom
	dy = yStep[*p-'A'] - '0';
	dirSet = 0;              // build direction set based on atom symmetry
	switch(symmetry[*p-'A']) {
	  case 'B': expo = 0;    // bishop, slide
	  case 'F': all = 0xAA;  // diagonal atom (degenerate 4-fold)
		    if(tx >= 0) goto king;        // continuation legs specified in K/Q system!
		    while(islower(*desc) && (i = dirType[*desc-'a']) != '0') {
			int b = dirs1[*desc-'a']; // use wide version
			if( islower(desc[1]) &&
				 ((i | dirType[desc[1]-'a']) & 3) == 3) {   // combinable (perpendicular dim)
			    b = dirs1[*desc-'a'] & dirs1[desc[1]-'a'];      // intersect wide & perp wide
			    desc += 2;
			} else desc++;
			dirSet |= b;
		    }
		    dirSet &= 0xAA; if(!dirSet) dirSet = 0xAA;
		    break;
	  case 'R': expo = 0;    // rook, slide
	  case 'W': all = 0x55;  // orthogonal atom (non-deg 4-fold)
		    if(tx >= 0) goto king;        // continuation legs specified in K/Q system!
		    while(islower(*desc) && (dirType[*desc-'a'] & ~4) != '0') dirSet |= dirs2[*desc++-'a'];
		    dirSet &= 0x55; if(!dirSet) dirSet = 0x55;
		    dirSet = (dirSet << angle | dirSet >> 8-angle) & 255;   // re-orient direction system
		    break;
	  case 'N': all = 0xFF;  // oblique atom (degenerate 8-fold)
		    if(tx >= 0) goto king;        // continuation legs specified in K/Q system!
		    if(*desc == 'h') {            // chiral direction sets 'hr' and 'hl'
			dirSet = (desc[1] == 'r' ? 0x55 :  0xAA); desc += 2;
 		    } else
		    while(islower(*desc) && (i = dirType[*desc-'a']) != '0') {
			int b = dirs2[*desc-'a']; // when alone, use narrow version
			if(desc[1] == 'h') b = dirs1[*desc-'a'], desc += 2; // dirs1 is wide version
			else if(*desc == desc[1] || islower(desc[1]) && i < '4'
				&& ((i | dirType[desc[1]-'a']) & 3) == 3) { // combinable (perpendicular dim or same)
			    b = dirs1[*desc-'a'] & dirs2[desc[1]-'a'];      // intersect wide & perp narrow
			    desc += 2;
			} else desc++;
			dirSet |= b;
		    }
		    if(!dirSet) dirSet = 0xFF;
		    break;
	  case 'Q': expo = 0;    // queen, slide
	  case 'K': all = 0xFF;  // non-deg (pseudo) 8-fold
	  king:
		    while(islower(*desc) && (i = dirType[*desc-'a']) != '0') {
			int b = dirs4[*desc-'a'];    // when alone, use narrow version
			if(desc[1] == *desc) desc++; // doubling forces alone
			else if(islower(desc[1]) && i < '4'
				&& ((i | dirType[desc[1]-'a']) & 3) == 3) { // combinable (perpendicular dim or same)
			    b = dirs3[*desc-'a'] & dirs3[desc[1]-'a'];      // intersect wide & perp wide
			    desc += 2;
			} else desc++;
			dirSet |= b;
		    }
		    if(!dirSet) dirSet = (tx < 0 ? 0xFF                     // default is all directions, but in continuation leg
					  : all == 0xFF ? 0xEF : 0x45);     // omits backward, and for 4-fold atoms also diags
		    dirSet = (dirSet << angle | dirSet >> 8-angle) & 255;   // re-orient direction system
		    ds2 = dirSet & 0xAA;          // extract diagonal directions
		    if(dirSet &= 0x55)            // start with orthogonal moves, if present
		         retry = 1, dx = 0;       // and schedule the diagonal moves for later
		    else dx = dy, dirSet = ds2;   // if no orthogonal directions, do diagonal immediately
		    break;       // should not have direction indicators
	  default:  return;      // syntax error: invalid atom
	}
	if(mine == 2 && tx < 0) dirSet = dirSet >> 4 | dirSet << 4 & 255;   // invert black moves
	mode = 0;                // build mode mask
	if(*desc == 'm') mode |= 4, desc++;           // move to empty
	if(*desc == 'c') mode |= his, desc++;         // capture foe
	if(*desc == 'd') mode |= mine, desc++;        // destroy (capture friend)
	if(*desc == 'e') mode |= 8, desc++;           // e.p. capture last mover
	if(*desc == 't') mode |= 16, desc++;          // exclude enemies as hop platform ('test')
	if(*desc == 'p') mode |= 32, desc++;          // hop over occupied
	if(*desc == 'g') mode |= 64, desc++;          // hop and toggle range
	if(*desc == 'o') mode |= 128, desc++;         // wrap around cylinder board
	if(*desc == 'y') mode |= 512, desc++;         // toggle range on empty square
	if(*desc == 'n') jump = 0, desc++;            // non-jumping
	while(*desc == 'j') jump++, desc++;           // must jump (on B,R,Q: skip first square)
	if(*desc == 'a') cont = ++desc;               // move again after doing what preceded it
	if(isdigit(*++p)) expo = atoi(p++);           // read exponent
	if(expo > 9) p++;                             // allow double-digit
	desc = p;                                     // this is start of next move
	if(initial && (board[r][f] != initialPosition[r][f] ||
		       r == 0              && board[TOUCHED_W] & 1<<f ||
		       r == BOARD_HEIGHT-1 && board[TOUCHED_B] & 1<<f   ) ) continue;
	if(expo > 1 && dx == 0 && dy == 0) {          // castling indicated by O + number
	    mode |= 1024; dy = 1;
	}
        if(!cont) {
	    if(!(mode & 15)) mode |= his + 4;         // no mode spec, use default = mc
	} else {
	    strncpy(buf, cont, 80); cont = buf;       // copy next leg(s), so we can modify
	    atom = buf; while(islower(*atom)) atom++; // skip to atom
	    if(mode & 32) mode ^= 256 + 32;           // in non-final legs 'p' means 'pass through'
	    if(mode & 64 + 512) {
		mode |= 256;                          // and 'g' too, but converts leaper <-> slider
		if(mode & 512) mode ^= 0x304;         // and 'y' is m-like 'g'
		*atom = upgrade[*atom-'A'];           // replace atom, BRQ <-> FWK
		atom[1] = atom[2] = '\0';             // make sure any old range is stripped off
		if(expo == 1) atom[1] = '0';          // turn other leapers into riders 
	    }
	    if(!(mode & 0x30F)) mode |= 4;            // and default of this leg = m
	}
	if(dy == 1) skip = jump - 1, jump = 1;        // on W & F atoms 'j' = skip first square
        do {
	  for(dir=0, bit=1; dir<8; dir++, bit += bit) { // loop over directions
	    int i = expo, j = skip, hop = mode, vx, vy, loop = 0;
	    if(!(bit & dirSet)) continue;             // does not move in this direction
	    if(dy != 1) j = 0;                        // 
	    vx = dx*rot[dir][0] + dy*rot[dir][1];     // rotate step vector
	    vy = dx*rot[dir][2] + dy*rot[dir][3];
	    if(tx < 0) x = f, y = r;                  // start square
	    else      x = tx, y = ty;                 // from previous to-square if continuation
	    do {                                      // traverse ray
		x += vx; y += vy;                     // step to next square
		if(y < 0 || y >= BOARD_HEIGHT) break; // vertically off-board: always done
		if(x <  BOARD_LEFT) { if(mode & 128) x += BOARD_RGHT - BOARD_LEFT, loop++; else break; }
		if(x >= BOARD_RGHT) { if(mode & 128) x -= BOARD_RGHT - BOARD_LEFT, loop++; else break; }
		if(j) { j--; continue; }              // skip irrespective of occupation
		if(!jump    && board[y - vy + vy/2][x - vx + vx/2] != EmptySquare) break; // blocked
		if(jump > 1 && board[y - vy + vy/2][x - vx + vx/2] == EmptySquare) break; // no hop
		if(x == f && y == r && !loop) occup = 4;     else // start square counts as empty (if not around cylinder!)
		if(board[y][x] < BlackPawn)   occup = 0x101; else
		if(board[y][x] < EmptySquare) occup = 0x102; else
					      occup = 4;
		if(cont) {                            // non-final leg
		  if(mode&16 && his&occup) occup &= 3;// suppress hopping foe in t-mode
		  if(occup & mode) {                  // valid intermediate square, do continuation
		    char origAtom = *atom;
		    if(!(bit & all)) *atom = rotate[*atom - 'A']; // orth-diag interconversion to make direction valid
		    if(occup & mode & 0x104)          // no side effects, merge legs to one move
			MovesFromString(board, flags, f, r, x, y, dir, cont, cb, cl);
		    if(occup & mode & 3 && (killX < 0 || killX == x && killY == y)) {     // destructive first leg
			int cnt = 0;
			MovesFromString(board, flags, f, r, x, y, dir, cont, &OK, &cnt);  // count possible continuations
			if(cnt) {                                                         // and if there are
			    if(killX < 0) cb(board, flags, FirstLeg, r, f, y, x, cl);     // then generate their first leg
			    legNr <<= 1;
			    MovesFromString(board, flags, f, r, x, y, dir, cont, cb, cl);
			    legNr >>= 1;
			}
		    }
		    *atom = origAtom;        // undo any interconversion
		  }
		  if(occup != 4) break;      // occupied squares always terminate the leg
		  continue;
		}
		if(hop & 32+64) { if(occup != 4) { if(hop & 64 && i != 1) i = 2; hop &= 31; } continue; } // hopper
		if(mode & 8 && y == board[EP_RANK] && occup == 4 && board[EP_FILE] == x) { // to e.p. square
		    cb(board, flags, mine == 1 ? WhiteCapturesEnPassant : BlackCapturesEnPassant, r, f, y, x, cl);
		}
		if(mode & 1024) {            // castling
		    i = 2;                   // kludge to elongate move indefinitely
		    if(occup == 4) continue; // skip empty squares
		    if(x == BOARD_LEFT   && board[y][x] == initialPosition[y][x]) // reached initial corner piece
			cb(board, flags, mine == 1 ? WhiteQueenSideCastle : BlackQueenSideCastle, r, f, y, f - expo, cl);
		    if(x == BOARD_RGHT-1 && board[y][x] == initialPosition[y][x])
			cb(board, flags, mine == 1 ? WhiteKingSideCastle : BlackKingSideCastle, r, f, y, f + expo, cl);
		    break;
		}
		if(mode & 16 && (board[y][x] == WhiteKing || board[y][x] == BlackKing)) break; // tame piece, cannot capture royal
		if(occup & mode) cb(board, flags, y == promoRank ? promo : NormalMove, r, f, y, x, cl); // allowed, generate
		if(occup != 4) break; // not valid transit square
	    } while(--i);
	  }
	  dx = dy; dirSet = ds2;      // prepare for diagonal moves of K,Q
	} while(retry-- && ds2);      // and start doing them
	if(tx >= 0) break;            // don't do other atoms in continuation legs
    }
} // next atom

// [HGM] move generation now based on hierarchy of subroutines for rays and combinations of rays

void
SlideForward (Board board, int flags, int rf, int ff, MoveCallback callback, VOIDSTAR closure)
{
  int i, rt, ft = ff;
  for (i = 1;; i++) {
      rt = rf + i;
      if (rt >= BOARD_HEIGHT) break;
      if (SameColor(board[rf][ff], board[rt][ft])) break;
      callback(board, flags, NormalMove, rf, ff, rt, ft, closure);
      if (board[rt][ft] != EmptySquare) break;
  }
}

void
SlideBackward (Board board, int flags, int rf, int ff, MoveCallback callback, VOIDSTAR closure)
{
  int i, rt, ft = ff;
  for (i = 1;; i++) {
      rt = rf - i;
      if (rt < 0) break;
      if (SameColor(board[rf][ff], board[rt][ft])) break;
      callback(board, flags, NormalMove, rf, ff, rt, ft, closure);
      if (board[rt][ft] != EmptySquare) break;
  }
}

void
SlideVertical (Board board, int flags, int rf, int ff, MoveCallback callback, VOIDSTAR closure)
{
  SlideForward(board, flags, rf, ff, callback, closure);
  SlideBackward(board, flags, rf, ff, callback, closure);
}

void
SlideSideways (Board board, int flags, int rf, int ff, MoveCallback callback, VOIDSTAR closure)
{
  int i, s, rt = rf, ft;
  for(s = -1; s <= 1; s+= 2) {
    for (i = 1;; i++) {
      ft = ff + i*s;
      if (ft < BOARD_LEFT || ft >= BOARD_RGHT) break;
      if (SameColor(board[rf][ff], board[rt][ft])) break;
      callback(board, flags, NormalMove, rf, ff, rt, ft, closure);
      if (board[rt][ft] != EmptySquare) break;
    }
  }
}

void
SlideDiagForward (Board board, int flags, int rf, int ff, MoveCallback callback, VOIDSTAR closure)
{
  int i, s, rt, ft;
  for(s = -1; s <= 1; s+= 2) {
    for (i = 1;; i++) {
      rt = rf + i;
      ft = ff + i * s;
      if (rt >= BOARD_HEIGHT || ft < BOARD_LEFT || ft >= BOARD_RGHT) break;
      if (SameColor(board[rf][ff], board[rt][ft])) break;
      callback(board, flags, NormalMove, rf, ff, rt, ft, closure);
      if (board[rt][ft] != EmptySquare) break;
    }
  }
}

void
SlideDiagBackward (Board board, int flags, int rf, int ff, MoveCallback callback, VOIDSTAR closure)
{
  int i, s, rt, ft;
  for(s = -1; s <= 1; s+= 2) {
    for (i = 1;; i++) {
      rt = rf - i;
      ft = ff + i * s;
      if (rt < 0 || ft < BOARD_LEFT || ft >= BOARD_RGHT) break;
      if (SameColor(board[rf][ff], board[rt][ft])) break;
      callback(board, flags, NormalMove, rf, ff, rt, ft, closure);
      if (board[rt][ft] != EmptySquare) break;
    }
  }
}

void
Rook (Board board, int flags, int rf, int ff, MoveCallback callback, VOIDSTAR closure)
{
  SlideVertical(board, flags, rf, ff, callback, closure);
  SlideSideways(board, flags, rf, ff, callback, closure);
}

void
Bishop (Board board, int flags, int rf, int ff, MoveCallback callback, VOIDSTAR closure)
{
  SlideDiagForward(board, flags, rf, ff, callback, closure);
  SlideDiagBackward(board, flags, rf, ff, callback, closure);
}

void
Sting (Board board, int flags, int rf, int ff, int dy, int dx, MoveCallback callback, VOIDSTAR closure)
{ // Lion-like move of Horned Falcon and Souring Eagle
  int ft = ff + dx, rt = rf + dy;
  if (rt < 0 || rt >= BOARD_HEIGHT || ft < BOARD_LEFT || ft >= BOARD_RGHT) return;
  legNr += 2;
  if (!SameColor(board[rf][ff], board[rt][ft]))
    callback(board, flags, board[rt][ft] != EmptySquare ? FirstLeg : NormalMove, rf, ff, rt, ft, closure);
  legNr -= 2;
  ft += dx; rt += dy;
  if (rt < 0 || rt >= BOARD_HEIGHT || ft < BOARD_LEFT || ft >= BOARD_RGHT) return;
  legNr += 2;
  if (!SameColor(board[rf][ff], board[rt][ft]))
    callback(board, flags, NormalMove, rf, ff, rt, ft, closure);
  if (!SameColor(board[rf][ff], board[rf+dy][ff+dx]))
    callback(board, flags, NormalMove, rf, ff, rf, ff, closure);
  legNr -= 2;
}

void
StepForward (Board board, int flags, int rf, int ff, MoveCallback callback, VOIDSTAR closure)
{
  int ft = ff, rt = rf + 1;
  if (rt >= BOARD_HEIGHT) return;
  if (SameColor(board[rf][ff], board[rt][ft])) return;
  callback(board, flags, NormalMove, rf, ff, rt, ft, closure);
}

void
StepBackward (Board board, int flags, int rf, int ff, MoveCallback callback, VOIDSTAR closure)
{
  int ft = ff, rt = rf - 1;
  if (rt < 0) return;
  if (SameColor(board[rf][ff], board[rt][ft])) return;
  callback(board, flags, NormalMove, rf, ff, rt, ft, closure);
}

void
StepSideways (Board board, int flags, int rf, int ff, MoveCallback callback, VOIDSTAR closure)
{
  int ft, rt = rf;
  ft = ff + 1;
  if (!(rt >= BOARD_HEIGHT || ft >= BOARD_RGHT) && !SameColor(board[rf][ff], board[rt][ft]))
      callback(board, flags, NormalMove, rf, ff, rt, ft, closure);
  ft = ff - 1;
  if (!(rt >= BOARD_HEIGHT || ft < BOARD_LEFT) && !SameColor(board[rf][ff], board[rt][ft]))
      callback(board, flags, NormalMove, rf, ff, rt, ft, closure);
}

void
StepDiagForward (Board board, int flags, int rf, int ff, MoveCallback callback, VOIDSTAR closure)
{
  int ft, rt = rf + 1;
  if (rt >= BOARD_HEIGHT) return;
  ft = ff + 1;
  if (!(rt >= BOARD_HEIGHT || ft >= BOARD_RGHT) && !SameColor(board[rf][ff], board[rt][ft]))
      callback(board, flags, NormalMove, rf, ff, rt, ft, closure);
  ft = ff - 1;
  if (!(rt >= BOARD_HEIGHT || ft < BOARD_LEFT) && !SameColor(board[rf][ff], board[rt][ft]))
      callback(board, flags, NormalMove, rf, ff, rt, ft, closure);
}

void
StepDiagBackward (Board board, int flags, int rf, int ff, MoveCallback callback, VOIDSTAR closure)
{
  int ft, rt = rf - 1;
  if(rt < 0) return;
  ft = ff + 1;
  if (!(rt < 0 || ft >= BOARD_RGHT) && !SameColor(board[rf][ff], board[rt][ft]))
      callback(board, flags, NormalMove, rf, ff, rt, ft, closure);
  ft = ff - 1;
  if (!(rt < 0 || ft < BOARD_LEFT) && !SameColor(board[rf][ff], board[rt][ft]))
      callback(board, flags, NormalMove, rf, ff, rt, ft, closure);
}

void
StepVertical (Board board, int flags, int rf, int ff, MoveCallback callback, VOIDSTAR closure)
{
  StepForward(board, flags, rf, ff, callback, closure);
  StepBackward(board, flags, rf, ff, callback, closure);
}

void
Ferz (Board board, int flags, int rf, int ff, MoveCallback callback, VOIDSTAR closure)
{
  StepDiagForward(board, flags, rf, ff, callback, closure);
  StepDiagBackward(board, flags, rf, ff, callback, closure);
}

void
Wazir (Board board, int flags, int rf, int ff, MoveCallback callback, VOIDSTAR closure)
{
  StepVertical(board, flags, rf, ff, callback, closure);
  StepSideways(board, flags, rf, ff, callback, closure);
}

void
Knight (Board board, int flags, int rf, int ff, MoveCallback callback, VOIDSTAR closure)
{
    int i, j, s, rt, ft;
    for (i = -1; i <= 1; i += 2)
	for (j = -1; j <= 1; j += 2)
	    for (s = 1; s <= 2; s++) {
		rt = rf + i*s;
		ft = ff + j*(3-s);
		if (!(rt < 0 || rt >= BOARD_HEIGHT || ft < BOARD_LEFT || ft >= BOARD_RGHT)
		    && ( gameInfo.variant != VariantXiangqi || board[rf+i*(s-1)][ff+j*(2-s)] == EmptySquare)
		    && !SameColor(board[rf][ff], board[rt][ft]))
		    callback(board, flags, NormalMove, rf, ff, rt, ft, closure);
	    }
}

/* Call callback once for each pseudo-legal move in the given
   position, except castling moves. A move is pseudo-legal if it is
   legal, or if it would be legal except that it leaves the king in
   check.  In the arguments, epfile is EP_NONE if the previous move
   was not a double pawn push, or the file 0..7 if it was, or
   EP_UNKNOWN if we don't know and want to allow all e.p. captures.
   Promotion moves generated are to Queen only.
*/
void
GenPseudoLegal (Board board, int flags, MoveCallback callback, VOIDSTAR closure, ChessSquare filter)
// speed: only do moves with this piece type
{
    int rf, ff;
    int i, j, d, s, fs, rs, rt, ft, m;
    int epfile = (signed char)board[EP_STATUS]; // [HGM] gamestate: extract ep status from board
    int promoRank = gameInfo.variant == VariantMakruk || gameInfo.variant == VariantGrand || gameInfo.variant == VariantChuChess ? 3 : 1;

    for (rf = 0; rf < BOARD_HEIGHT; rf++)
      for (ff = BOARD_LEFT; ff < BOARD_RGHT; ff++) {
          ChessSquare piece;

	  if(board[rf][ff] == EmptySquare) continue;
	  if ((flags & F_WHITE_ON_MOVE) != (board[rf][ff] < BlackPawn)) continue; // [HGM] speed: wrong color
          m = 0; piece = board[rf][ff];
          if(PieceToChar(piece) == '~')
                 piece = (ChessSquare) ( DEMOTED piece );
          if(filter != EmptySquare && piece != filter) continue;
          if(pieceDefs && pieceDesc[piece]) { // [HGM] gen: use engine-defined moves
              MovesFromString(board, flags, ff, rf, -1, -1, 0, pieceDesc[piece], callback, closure);
              continue;
          }
          if(IS_SHOGI(gameInfo.variant))
                 piece = (ChessSquare) ( SHOGI piece );

          switch ((int)piece) {
            /* case EmptySquare: [HGM] this is nonsense, and conflicts with Shogi cases */
	    default:
	      /* can't happen ([HGM] except for faries...) */
	      break;

             case WhitePawn:
              if(gameInfo.variant == VariantXiangqi) {
                  /* [HGM] capture and move straight ahead in Xiangqi */
                  if (rf < BOARD_HEIGHT-1 &&
                           !SameColor(board[rf][ff], board[rf + 1][ff]) ) {
                           callback(board, flags, NormalMove,
                                    rf, ff, rf + 1, ff, closure);
                  }
                  /* and move sideways when across the river */
                  for (s = -1; s <= 1; s += 2) {
                      if (rf >= BOARD_HEIGHT>>1 &&
                          ff + s >= BOARD_LEFT && ff + s < BOARD_RGHT &&
                          !WhitePiece(board[rf][ff+s]) ) {
                           callback(board, flags, NormalMove,
                                    rf, ff, rf, ff+s, closure);
                      }
                  }
                  break;
              }
              if (rf < BOARD_HEIGHT-1 && board[rf + 1][ff] == EmptySquare) {
		  callback(board, flags,
			   rf >= BOARD_HEIGHT-1-promoRank ? WhitePromotion : NormalMove,
			   rf, ff, rf + 1, ff, closure);
	      }
	      if (rf <= (BOARD_HEIGHT>>1)-3 && board[rf+1][ff] == EmptySquare && // [HGM] grand: also on 3rd rank on 10-board
                  gameInfo.variant != VariantShatranj && /* [HGM] */
                  gameInfo.variant != VariantCourier  && /* [HGM] */
                  board[rf+2][ff] == EmptySquare ) {
                      callback(board, flags, NormalMove,
                               rf, ff, rf+2, ff, closure);
	      }
	      for (s = -1; s <= 1; s += 2) {
                  if (rf < BOARD_HEIGHT-1 && ff + s >= BOARD_LEFT && ff + s < BOARD_RGHT &&
		      ((flags & F_KRIEGSPIEL_CAPTURE) ||
		       BlackPiece(board[rf + 1][ff + s]))) {
		      callback(board, flags,
			       rf >= BOARD_HEIGHT-1-promoRank ? WhitePromotion : NormalMove,
			       rf, ff, rf + 1, ff + s, closure);
		  }
		  if (rf >= BOARD_HEIGHT+1>>1) {// [HGM] grand: 4th & 5th rank on 10-board
                      if (ff + s >= BOARD_LEFT && ff + s < BOARD_RGHT &&
			  (epfile == ff + s || epfile == EP_UNKNOWN) && rf < BOARD_HEIGHT-3 &&
                          board[rf][ff + s] == BlackPawn &&
                          board[rf+1][ff + s] == EmptySquare) {
			  callback(board, flags, WhiteCapturesEnPassant,
				   rf, ff, rf+1, ff + s, closure);
		      }
		  }
	      }
	      break;

	    case BlackPawn:
              if(gameInfo.variant == VariantXiangqi) {
                  /* [HGM] capture straight ahead in Xiangqi */
                  if (rf > 0 && !SameColor(board[rf][ff], board[rf - 1][ff]) ) {
                           callback(board, flags, NormalMove,
                                    rf, ff, rf - 1, ff, closure);
                  }
                  /* and move sideways when across the river */
                  for (s = -1; s <= 1; s += 2) {
                      if (rf < BOARD_HEIGHT>>1 &&
                          ff + s >= BOARD_LEFT && ff + s < BOARD_RGHT &&
                          !BlackPiece(board[rf][ff+s]) ) {
                           callback(board, flags, NormalMove,
                                    rf, ff, rf, ff+s, closure);
                      }
                  }
                  break;
              }
	      if (rf > 0 && board[rf - 1][ff] == EmptySquare) {
		  callback(board, flags,
			   rf <= promoRank ? BlackPromotion : NormalMove,
			   rf, ff, rf - 1, ff, closure);
	      }
	      if (rf >= (BOARD_HEIGHT+1>>1)+2 && board[rf-1][ff] == EmptySquare && // [HGM] grand
                  gameInfo.variant != VariantShatranj && /* [HGM] */
                  gameInfo.variant != VariantCourier  && /* [HGM] */
		  board[rf-2][ff] == EmptySquare) {
		  callback(board, flags, NormalMove,
			   rf, ff, rf-2, ff, closure);
	      }
	      for (s = -1; s <= 1; s += 2) {
                  if (rf > 0 && ff + s >= BOARD_LEFT && ff + s < BOARD_RGHT &&
		      ((flags & F_KRIEGSPIEL_CAPTURE) ||
		       WhitePiece(board[rf - 1][ff + s]))) {
		      callback(board, flags,
			       rf <= promoRank ? BlackPromotion : NormalMove,
			       rf, ff, rf - 1, ff + s, closure);
		  }
		  if (rf < BOARD_HEIGHT>>1) {
                      if (ff + s >= BOARD_LEFT && ff + s < BOARD_RGHT &&
			  (epfile == ff + s || epfile == EP_UNKNOWN) && rf > 2 &&
			  board[rf][ff + s] == WhitePawn &&
			  board[rf-1][ff + s] == EmptySquare) {
			  callback(board, flags, BlackCapturesEnPassant,
				   rf, ff, rf-1, ff + s, closure);
		      }
		  }
	      }
	      break;

            case WhiteUnicorn:
            case BlackUnicorn:
	    case WhiteKnight:
	    case BlackKnight:
	      for (i = -1; i <= 1; i += 2)
		for (j = -1; j <= 1; j += 2)
		  for (s = 1; s <= 2; s++) {
		      rt = rf + i*s;
		      ft = ff + j*(3-s);
                      if (!(rt < 0 || rt >= BOARD_HEIGHT || ft < BOARD_LEFT || ft >= BOARD_RGHT)
                          && ( gameInfo.variant != VariantXiangqi || board[rf+i*(s-1)][ff+j*(2-s)] == EmptySquare)
                          && !SameColor(board[rf][ff], board[rt][ft]))
		      callback(board, flags, NormalMove,
			       rf, ff, rt, ft, closure);
		  }
	      break;

            case SHOGI WhiteKnight:
	      for (s = -1; s <= 1; s += 2) {
                  if (rf < BOARD_HEIGHT-2 && ff + s >= BOARD_LEFT && ff + s < BOARD_RGHT &&
                      !SameColor(board[rf][ff], board[rf + 2][ff + s])) {
                      callback(board, flags, NormalMove,
                               rf, ff, rf + 2, ff + s, closure);
		  }
              }
	      break;

            case SHOGI BlackKnight:
	      for (s = -1; s <= 1; s += 2) {
                  if (rf > 1 && ff + s >= BOARD_LEFT && ff + s < BOARD_RGHT &&
                      !SameColor(board[rf][ff], board[rf - 2][ff + s])) {
                      callback(board, flags, NormalMove,
                               rf, ff, rf - 2, ff + s, closure);
		  }
	      }
	      break;

            case WhiteCannon:
            case BlackCannon:
              for (d = 0; d <= 1; d++)
                for (s = -1; s <= 1; s += 2) {
                  m = 0;
		  for (i = 1;; i++) {
		      rt = rf + (i * s) * d;
		      ft = ff + (i * s) * (1 - d);
                      if (rt < 0 || rt >= BOARD_HEIGHT || ft < BOARD_LEFT || ft >= BOARD_RGHT) break;
                      if (m == 0 && board[rt][ft] == EmptySquare)
                                 callback(board, flags, NormalMove,
                                          rf, ff, rt, ft, closure);
                      if (m == 1 && board[rt][ft] != EmptySquare &&
                          !SameColor(board[rf][ff], board[rt][ft]) )
                                 callback(board, flags, NormalMove,
                                          rf, ff, rt, ft, closure);
                      if (board[rt][ft] != EmptySquare && m++) break;
                  }
                }
	      break;

            /* Gold General (and all its promoted versions) . First do the */
            /* diagonal forward steps, then proceed as normal Wazir        */
            case SHOGI (PROMOTED WhitePawn):
		if(gameInfo.variant == VariantShogi) goto WhiteGold;
            case SHOGI (PROMOTED BlackPawn):
		if(gameInfo.variant == VariantShogi) goto BlackGold;
		SlideVertical(board, flags, rf, ff, callback, closure);
		break;

            case SHOGI (PROMOTED WhiteKnight):
		if(gameInfo.variant == VariantShogi) goto WhiteGold;
            case SHOGI BlackDrunk:
            case SHOGI BlackAlfil:
		Ferz(board, flags, rf, ff, callback, closure);
		StepSideways(board, flags, rf, ff, callback, closure);
		StepBackward(board, flags, rf, ff, callback, closure);
		break;

            case SHOGI (PROMOTED BlackKnight):
		if(gameInfo.variant == VariantShogi) goto BlackGold;
            case SHOGI WhiteDrunk:
            case SHOGI WhiteAlfil:
		Ferz(board, flags, rf, ff, callback, closure);
		StepSideways(board, flags, rf, ff, callback, closure);
		StepForward(board, flags, rf, ff, callback, closure);
		break;


            case SHOGI WhiteStag:
            case SHOGI BlackStag:
		if(gameInfo.variant == VariantShogi) goto BlackGold;
		SlideVertical(board, flags, rf, ff, callback, closure);
		Ferz(board, flags, rf, ff, callback, closure);
		StepSideways(board, flags, rf, ff, callback, closure);
		break;

            case SHOGI (PROMOTED WhiteQueen):
            case SHOGI WhiteTokin:
            case SHOGI WhiteWazir:
	    WhiteGold:
		StepDiagForward(board, flags, rf, ff, callback, closure);
		Wazir(board, flags, rf, ff, callback, closure);
		break;

            case SHOGI (PROMOTED BlackQueen):
            case SHOGI BlackTokin:
            case SHOGI BlackWazir:
            BlackGold:
		StepDiagBackward(board, flags, rf, ff, callback, closure);
		Wazir(board, flags, rf, ff, callback, closure);
		break;

            case WhiteWazir:
            case BlackWazir:
		Wazir(board, flags, rf, ff, callback, closure);
		break;

            case SHOGI WhiteMarshall:
            case SHOGI BlackMarshall:
		Ferz(board, flags, rf, ff, callback, closure);
		for (d = 0; d <= 1; d++)
		    for (s = -2; s <= 2; s += 4) {
			rt = rf + s * d;
			ft = ff + s * (1 - d);
			if (rt < 0 || rt >= BOARD_HEIGHT || ft < BOARD_LEFT || ft >= BOARD_RGHT) continue;
			if (!SameColor(board[rf][ff], board[rt][ft]) )
			    callback(board, flags, NormalMove, rf, ff, rt, ft, closure);
		    }
		break;

            case SHOGI WhiteAngel:
            case SHOGI BlackAngel:
		Wazir(board, flags, rf, ff, callback, closure);

            case WhiteAlfil:
            case BlackAlfil:
                /* [HGM] support Shatranj pieces */
                for (rs = -1; rs <= 1; rs += 2)
                  for (fs = -1; fs <= 1; fs += 2) {
                      rt = rf + 2 * rs;
                      ft = ff + 2 * fs;
                      if (!(rt < 0 || rt >= BOARD_HEIGHT || ft < BOARD_LEFT || ft >= BOARD_RGHT)
                          && ( gameInfo.variant != VariantXiangqi ||
                               board[rf+rs][ff+fs] == EmptySquare && (2*rf < BOARD_HEIGHT) == (2*rt < BOARD_HEIGHT) )

                          && !SameColor(board[rf][ff], board[rt][ft]))
                               callback(board, flags, NormalMove,
                                        rf, ff, rt, ft, closure);
                      if(gameInfo.variant == VariantShatranj || gameInfo.variant == VariantCourier ||
                         gameInfo.variant == VariantChu      || gameInfo.variant == VariantXiangqi) continue; // classical Alfil
                      rt = rf + rs; // in unknown variant we assume Modern Elephant, which can also do one step
                      ft = ff + fs;
                      if (!(rt < 0 || rt >= BOARD_HEIGHT || ft < BOARD_LEFT || ft >= BOARD_RGHT)
                          && !SameColor(board[rf][ff], board[rt][ft]))
                               callback(board, flags, NormalMove,
                                        rf, ff, rt, ft, closure);
		  }
                if(gameInfo.variant == VariantSpartan)
                   for(fs = -1; fs <= 1; fs += 2) {
                      ft = ff + fs;
                      if (!(ft < BOARD_LEFT || ft >= BOARD_RGHT) && board[rf][ft] == EmptySquare)
                               callback(board, flags, NormalMove, rf, ff, rf, ft, closure);
                   }
                break;

            /* Make Dragon-Horse also do Dababba moves outside Shogi, for better disambiguation in variant Fairy */
	    case WhiteCardinal:
	    case BlackCardinal:
              if(gameInfo.variant == VariantChuChess) goto DragonHorse;
              for (d = 0; d <= 1; d++) // Dababba moves that Rook cannot do
                for (s = -2; s <= 2; s += 4) {
		      rt = rf + s * d;
		      ft = ff + s * (1 - d);
                      if (rt < 0 || rt >= BOARD_HEIGHT || ft < BOARD_LEFT || ft >= BOARD_RGHT) continue;
		      if (SameColor(board[rf][ff], board[rt][ft])) continue;
		      callback(board, flags, NormalMove, rf, ff, rt, ft, closure);
		  }

            /* Shogi Dragon Horse has to continue with Wazir after Bishop */
            case SHOGI WhiteCardinal:
            case SHOGI BlackCardinal:
            case SHOGI WhitePCardinal:
            case SHOGI BlackPCardinal:
            DragonHorse:
		Bishop(board, flags, rf, ff, callback, closure);
		Wazir(board, flags, rf, ff, callback, closure);
		break;

            /* Capablanca Archbishop continues as Knight                  */
            case WhiteAngel:
            case BlackAngel:
		Knight(board, flags, rf, ff, callback, closure);

            /* Shogi Bishops are ordinary Bishops */
            case SHOGI WhiteBishop:
            case SHOGI BlackBishop:
            case SHOGI WhitePBishop:
            case SHOGI BlackPBishop:
	    case WhiteBishop:
	    case BlackBishop:
		Bishop(board, flags, rf, ff, callback, closure);
		break;

            /* Shogi Lance is unlike anything, and asymmetric at that */
            case SHOGI WhiteQueen:
              if(gameInfo.variant == VariantChu) goto doQueen;
              for(i = 1;; i++) {
                      rt = rf + i;
                      ft = ff;
                      if (rt >= BOARD_HEIGHT) break;
		      if (SameColor(board[rf][ff], board[rt][ft])) break;
		      callback(board, flags, NormalMove,
			       rf, ff, rt, ft, closure);
                      if (board[rt][ft] != EmptySquare) break;
              }
              break;

            case SHOGI BlackQueen:
              if(gameInfo.variant == VariantChu) goto doQueen;
              for(i = 1;; i++) {
                      rt = rf - i;
                      ft = ff;
                      if (rt < 0) break;
		      if (SameColor(board[rf][ff], board[rt][ft])) break;
		      callback(board, flags, NormalMove,
			       rf, ff, rt, ft, closure);
                      if (board[rt][ft] != EmptySquare) break;
              }
              break;

            /* Make Dragon-King Dababba & Rook-like outside Shogi, for better disambiguation in variant Fairy */
	    case WhiteDragon:
	    case BlackDragon:
              if(gameInfo.variant == VariantChuChess) goto DragonKing;
              for (d = 0; d <= 1; d++) // Dababba moves that Rook cannot do
                for (s = -2; s <= 2; s += 4) {
		      rt = rf + s * d;
		      ft = ff + s * (1 - d);
                      if (rt < 0 || rt >= BOARD_HEIGHT || ft < BOARD_LEFT || ft >= BOARD_RGHT) continue;
                      if (board[rf+rt>>1][ff+ft>>1] == EmptySquare && gameInfo.variant != VariantSpartan) continue;
		      if (SameColor(board[rf][ff], board[rt][ft])) continue;
		      callback(board, flags, NormalMove, rf, ff, rt, ft, closure);
		  }
              if(gameInfo.variant == VariantSpartan) // in Spartan Chess restrict range to modern Dababba
		Wazir(board, flags, rf, ff, callback, closure);
	      else
		Rook(board, flags, rf, ff, callback, closure);
              break;

            /* Shogi Dragon King has to continue as Ferz after Rook moves */
            case SHOGI WhiteDragon:
            case SHOGI BlackDragon:
            case SHOGI WhitePDragon:
            case SHOGI BlackPDragon:
            DragonKing:
		Rook(board, flags, rf, ff, callback, closure);
		Ferz(board, flags, rf, ff, callback, closure);
		break;
              m++;

            /* Capablanca Chancellor sets flag to continue as Knight      */
            case WhiteMarshall:
            case BlackMarshall:
		Rook(board, flags, rf, ff, callback, closure);
		if(gameInfo.variant == VariantSpartan) // in Spartan Chess Chancellor is used for Dragon King.
		    Ferz(board, flags, rf, ff, callback, closure);
		else
		    Knight(board, flags, rf, ff, callback, closure);
		break;

            /* Shogi Rooks are ordinary Rooks */
            case SHOGI WhiteRook:
            case SHOGI BlackRook:
            case SHOGI WhitePRook:
            case SHOGI BlackPRook:
	    case WhiteRook:
	    case BlackRook:
		Rook(board, flags, rf, ff, callback, closure);
		break;

	    case WhiteQueen:
	    case BlackQueen:
            case SHOGI WhiteMother:
            case SHOGI BlackMother:
	    doQueen:
		Rook(board, flags, rf, ff, callback, closure);
		Bishop(board, flags, rf, ff, callback, closure);
		break;

           case SHOGI WhitePawn:
		StepForward(board, flags, rf, ff, callback, closure);
		break;

            case SHOGI BlackPawn:
		StepBackward(board, flags, rf, ff, callback, closure);
		break;

            case WhiteMan:
                if(gameInfo.variant != VariantMakruk && gameInfo.variant != VariantASEAN) goto commoner;
            case SHOGI WhiteFerz:
		Ferz(board, flags, rf, ff, callback, closure);
		StepForward(board, flags, rf, ff, callback, closure);
		break;

            case BlackMan:
                if(gameInfo.variant != VariantMakruk && gameInfo.variant != VariantASEAN) goto commoner;
            case SHOGI BlackFerz:
		StepBackward(board, flags, rf, ff, callback, closure);

            case WhiteFerz:
            case BlackFerz:
                /* [HGM] support Shatranj pieces */
		Ferz(board, flags, rf, ff, callback, closure);
		break;

	    case WhiteSilver:
	    case BlackSilver:
		Knight(board, flags, rf, ff, callback, closure); // [HGM] superchess: use for Centaur

            commoner:
            case SHOGI WhiteMonarch:
            case SHOGI BlackMonarch:
            case SHOGI WhiteKing:
            case SHOGI BlackKing:
	    case WhiteKing:
	    case BlackKing:
		Ferz(board, flags, rf, ff, callback, closure);
		Wazir(board, flags, rf, ff, callback, closure);
		break;

	    case WhiteNightrider:
	    case BlackNightrider:
	      for (i = -1; i <= 1; i += 2)
		for (j = -1; j <= 1; j += 2)
		  for (s = 1; s <= 2; s++) {  int k;
                    for(k=1;; k++) {
		      rt = rf + k*i*s;
		      ft = ff + k*j*(3-s);
                      if (rt < 0 || rt >= BOARD_HEIGHT || ft < BOARD_LEFT || ft >= BOARD_RGHT) break;
		      if (SameColor(board[rf][ff], board[rt][ft])) break;
		      callback(board, flags, NormalMove,
			       rf, ff, rt, ft, closure);
		      if (board[rt][ft] != EmptySquare) break;
                    }
		  }
	      break;

	    Amazon:
		Bishop(board, flags, rf, ff, callback, closure);
		Rook(board, flags, rf, ff, callback, closure);
		Knight(board, flags, rf, ff, callback, closure);
		break;

	    // Use Lance as Berolina / Spartan Pawn.
	    case WhiteLance:
	      if(gameInfo.variant == VariantSuper) goto Amazon;
	      if (rf < BOARD_HEIGHT-1 && BlackPiece(board[rf + 1][ff]))
		  callback(board, flags,
			   rf >= BOARD_HEIGHT-1-promoRank ? WhitePromotion : NormalMove,
			   rf, ff, rf + 1, ff, closure);
	      for (s = -1; s <= 1; s += 2) {
	          if (rf < BOARD_HEIGHT-1 && ff + s >= BOARD_LEFT && ff + s < BOARD_RGHT && board[rf + 1][ff + s] == EmptySquare)
		      callback(board, flags,
			       rf >= BOARD_HEIGHT-1-promoRank ? WhitePromotion : NormalMove,
			       rf, ff, rf + 1, ff + s, closure);
	          if (rf == 1 && ff + 2*s >= BOARD_LEFT && ff + 2*s < BOARD_RGHT && board[3][ff + 2*s] == EmptySquare )
		      callback(board, flags, NormalMove, rf, ff, 3, ff + 2*s, closure);
	      }
	      break;

	    case BlackLance:
	      if(gameInfo.variant == VariantSuper) goto Amazon;
	      if (rf > 0 && WhitePiece(board[rf - 1][ff]))
		  callback(board, flags,
			   rf <= promoRank ? BlackPromotion : NormalMove,
			   rf, ff, rf - 1, ff, closure);
	      for (s = -1; s <= 1; s += 2) {
	          if (rf > 0 && ff + s >= BOARD_LEFT && ff + s < BOARD_RGHT && board[rf - 1][ff + s] == EmptySquare)
		      callback(board, flags,
			       rf <= promoRank ? BlackPromotion : NormalMove,
			       rf, ff, rf - 1, ff + s, closure);
	          if (rf == BOARD_HEIGHT-2 && ff + 2*s >= BOARD_LEFT && ff + 2*s < BOARD_RGHT && board[rf-2][ff + 2*s] == EmptySquare )
		      callback(board, flags, NormalMove, rf, ff, rf-2, ff + 2*s, closure);
	      }
            break;

            case SHOGI WhiteNothing:
            case SHOGI BlackNothing:
            case SHOGI WhiteLion:
            case SHOGI BlackLion:
            case WhiteLion:
            case BlackLion:
              for(rt = rf - 2; rt <= rf + 2; rt++) for(ft = ff - 2; ft <= ff + 2; ft++) {
                if (rt < 0 || rt >= BOARD_HEIGHT || ft < BOARD_LEFT || ft >= BOARD_RGHT) continue;
                if (!(ff == ft && rf == rt) && SameColor(board[rf][ff], board[rt][ft])) continue;
                i = (killX >= 0 && (rt-killY)*(rt-killY) + (killX-ft)*(killX-ft) < 3); legNr += 2*i;
                callback(board, flags, (rt-rf)*(rt-rf) + (ff-ft)*(ff-ft) < 3 && board[rt][ft] != EmptySquare ? FirstLeg : NormalMove,
                         rf, ff, rt, ft, closure);
                legNr -= 2*i;
              }
              break;

            case SHOGI WhiteFalcon:
            case SHOGI BlackFalcon:
            case SHOGI WhitePDagger:
            case SHOGI BlackPDagger:
		SlideSideways(board, flags, rf, ff, callback, closure);
		StepVertical(board, flags, rf, ff, callback, closure);
		break;

            case SHOGI WhiteCobra:
            case SHOGI BlackCobra:
		StepVertical(board, flags, rf, ff, callback, closure);
		break;

            case SHOGI (PROMOTED WhiteFerz):
		if(gameInfo.variant == VariantShogi) goto WhiteGold;
            case SHOGI (PROMOTED BlackFerz):
		if(gameInfo.variant == VariantShogi) goto BlackGold;
            case SHOGI WhitePSword:
            case SHOGI BlackPSword:
		SlideVertical(board, flags, rf, ff, callback, closure);
		StepSideways(board, flags, rf, ff, callback, closure);
		break;

            case SHOGI WhiteUnicorn:
            case SHOGI BlackUnicorn:
		Ferz(board, flags, rf, ff, callback, closure);
		StepVertical(board, flags, rf, ff, callback, closure);
		break;

            case SHOGI WhiteMan:
		StepDiagForward(board, flags, rf, ff, callback, closure);
		StepVertical(board, flags, rf, ff, callback, closure);
		break;

            case SHOGI BlackMan:
		StepDiagBackward(board, flags, rf, ff, callback, closure);
		StepVertical(board, flags, rf, ff, callback, closure);
		break;

            case SHOGI WhiteHCrown:
            case SHOGI BlackHCrown:
		Bishop(board, flags, rf, ff, callback, closure);
		SlideSideways(board, flags, rf, ff, callback, closure);
		break;

            case SHOGI WhiteCrown:
            case SHOGI BlackCrown:
		Bishop(board, flags, rf, ff, callback, closure);
		SlideVertical(board, flags, rf, ff, callback, closure);
		break;

            case SHOGI WhiteHorned:
		Sting(board, flags, rf, ff, 1, 0, callback, closure);
		callback(board, flags, NormalMove, rf, ff, rf, ff, closure);
		if(killX >= 0) break;
		Bishop(board, flags, rf, ff, callback, closure);
		SlideSideways(board, flags, rf, ff, callback, closure);
		SlideBackward(board, flags, rf, ff, callback, closure);
		break;

            case SHOGI BlackHorned:
		Sting(board, flags, rf, ff, -1, 0, callback, closure);
		callback(board, flags, NormalMove, rf, ff, rf, ff, closure);
		if(killX >= 0) break;
		Bishop(board, flags, rf, ff, callback, closure);
		SlideSideways(board, flags, rf, ff, callback, closure);
		SlideForward(board, flags, rf, ff, callback, closure);
		break;

            case SHOGI WhiteEagle:
		Sting(board, flags, rf, ff, 1,  1, callback, closure);
		Sting(board, flags, rf, ff, 1, -1, callback, closure);
		callback(board, flags, NormalMove, rf, ff, rf, ff, closure);
		if(killX >= 0) break;
		Rook(board, flags, rf, ff, callback, closure);
		SlideDiagBackward(board, flags, rf, ff, callback, closure);
		break;

            case SHOGI BlackEagle:
		Sting(board, flags, rf, ff, -1,  1, callback, closure);
		Sting(board, flags, rf, ff, -1, -1, callback, closure);
		callback(board, flags, NormalMove, rf, ff, rf, ff, closure);
		if(killX >= 0) break;
		Rook(board, flags, rf, ff, callback, closure);
		SlideDiagForward(board, flags, rf, ff, callback, closure);
		break;

            case SHOGI WhiteDolphin:
            case SHOGI BlackHorse:
		SlideDiagBackward(board, flags, rf, ff, callback, closure);
		SlideVertical(board, flags, rf, ff, callback, closure);
		break;

            case SHOGI BlackDolphin:
            case SHOGI WhiteHorse:
		SlideDiagForward(board, flags, rf, ff, callback, closure);
		SlideVertical(board, flags, rf, ff, callback, closure);
		break;

            case SHOGI WhiteLance:
		SlideForward(board, flags, rf, ff, callback, closure);
		break;

            case SHOGI BlackLance:
		SlideBackward(board, flags, rf, ff, callback, closure);
		break;

	    case WhiteFalcon: // [HGM] wild: for wildcards, self-capture symbolizes move to anywhere
	    case BlackFalcon:
	    case WhiteCobra:
	    case BlackCobra:
	      callback(board, flags, NormalMove, rf, ff, rf, ff, closure);
	      break;

	  }
      }
}


typedef struct {
    MoveCallback cb;
    VOIDSTAR cl;
} GenLegalClosure;

int rFilter, fFilter; // [HGM] speed: sorry, but I get a bit tired of this closure madness
Board xqCheckers, nullBoard;

extern void GenLegalCallback P((Board board, int flags, ChessMove kind,
				int rf, int ff, int rt, int ft,
				VOIDSTAR closure));

void
GenLegalCallback (Board board, int flags, ChessMove kind, int rf, int ff, int rt, int ft, VOIDSTAR closure)
{
    register GenLegalClosure *cl = (GenLegalClosure *) closure;

    if(rFilter >= 0 && rFilter != rt || fFilter >= 0 && fFilter != ft) return; // [HGM] speed: ignore moves with wrong to-square

    if (board[EP_STATUS] == EP_IRON_LION && (board[rt][ft] == WhiteLion || board[rt][ft] == BlackLion)) return; //[HGM] lion

    if (!(flags & F_IGNORE_CHECK) ) {
      int check, promo = (gameInfo.variant == VariantSpartan && kind == BlackPromotion);
      if(promo) {
	    int r, f, kings=0;
	    for(r=0; r<BOARD_HEIGHT; r++) for(f=BOARD_LEFT; f<BOARD_RGHT;	f++)
		kings += (board[r][f] == BlackKing);
	    if(kings >= 2)
	      promo = 0;
	    else
		board[rf][ff] = BlackKing; // [HGM] spartan: promote to King before check-test
	}
	check = CheckTest(board, flags, rf, ff, rt, ft,
		  kind == WhiteCapturesEnPassant ||
		  kind == BlackCapturesEnPassant);
	if(promo) board[rf][ff] = BlackLance;
      if(check) return;
    }
    if (flags & F_ATOMIC_CAPTURE) {
      if (board[rt][ft] != EmptySquare ||
	  kind == WhiteCapturesEnPassant || kind == BlackCapturesEnPassant) {
	int r, f;
	ChessSquare king = (flags & F_WHITE_ON_MOVE) ? WhiteKing : BlackKing;
	if (board[rf][ff] == king) return;
	for (r = rt-1; r <= rt+1; r++) {
	  for (f = ft-1; f <= ft+1; f++) {
            if (r >= 0 && r < BOARD_HEIGHT && f >= BOARD_LEFT && f < BOARD_RGHT &&
		board[r][f] == king) return;
	  }
	}
      }
    }
    cl->cb(board, flags, kind, rf, ff, rt, ft, cl->cl);
}


typedef struct {
    int rf, ff, rt, ft;
    ChessMove kind;
    int captures; // [HGM] losers
} LegalityTestClosure;


/* Like GenPseudoLegal, but (1) include castling moves, (2) unless
   F_IGNORE_CHECK is set in the flags, omit moves that would leave the
   king in check, and (3) if F_ATOMIC_CAPTURE is set in the flags, omit
   moves that would destroy your own king.  The CASTLE_OK flags are
   true if castling is not yet ruled out by a move of the king or
   rook.  Return TRUE if the player on move is currently in check and
   F_IGNORE_CHECK is not set.  [HGM] add castlingRights parameter */
int
GenLegal (Board board, int  flags, MoveCallback callback, VOIDSTAR closure, ChessSquare filter)
{
    GenLegalClosure cl;
    int ff, ft, k, left, right, swap;
    int ignoreCheck = (flags & F_IGNORE_CHECK) != 0;
    ChessSquare wKing = WhiteKing, bKing = BlackKing, *castlingRights = board[CASTLING];
    int inCheck = !ignoreCheck && CheckTest(board, flags, -1, -1, -1, -1, FALSE); // kludge alert: this would mark pre-existing checkers if status==1
    char *p;

    cl.cb = callback;
    cl.cl = closure;
    xqCheckers[EP_STATUS] *= 2; // quasi: if previous CheckTest has been marking, we now set flag for suspending same checkers
    if(filter == EmptySquare) rFilter = fFilter = -1; // [HGM] speed: do not filter on square if we do not filter on piece
    GenPseudoLegal(board, flags, GenLegalCallback, (VOIDSTAR) &cl, filter);

    if (inCheck) return TRUE;

    /* Generate castling moves */
    if(gameInfo.variant == VariantKnightmate) { /* [HGM] Knightmate */
        wKing = WhiteUnicorn; bKing = BlackUnicorn;
    }

    p = (flags & F_WHITE_ON_MOVE ? pieceDesc[wKing] : pieceDesc[bKing]);
    if(p && strchr(p, 'O')) return FALSE; // [HGM] gen: castlings were already generated from string

    for (ff = BOARD_WIDTH>>1; ff >= (BOARD_WIDTH-1)>>1; ff-- /*ics wild 1*/) {
	if ((flags & F_WHITE_ON_MOVE) &&
	    (flags & F_WHITE_KCASTLE_OK) &&
            board[0][ff] == wKing &&
            board[0][ff + 1] == EmptySquare &&
            board[0][ff + 2] == EmptySquare &&
            board[0][BOARD_RGHT-3] == EmptySquare &&
            board[0][BOARD_RGHT-2] == EmptySquare &&
            board[0][BOARD_RGHT-1] == WhiteRook &&
            castlingRights[0] != NoRights && /* [HGM] check rights */
            ( castlingRights[2] == ff || castlingRights[6] == ff ) &&
            (ignoreCheck ||
	     (!CheckTest(board, flags, 0, ff, 0, ff + 1, FALSE) &&
              !CheckTest(board, flags, 0, ff, 0, BOARD_RGHT-3, FALSE) &&
              (gameInfo.variant != VariantJanus || !CheckTest(board, flags, 0, ff, 0, BOARD_RGHT-2, FALSE)) &&
	      !CheckTest(board, flags, 0, ff, 0, ff + 2, FALSE)))) {

	    callback(board, flags,
                     ff==BOARD_WIDTH>>1 ? WhiteKingSideCastle : WhiteKingSideCastleWild,
                     0, ff, 0, ff + ((gameInfo.boardWidth+2)>>2) + (gameInfo.variant == VariantJanus), closure);
	}
	if ((flags & F_WHITE_ON_MOVE) &&
	    (flags & F_WHITE_QCASTLE_OK) &&
            board[0][ff] == wKing &&
	    board[0][ff - 1] == EmptySquare &&
	    board[0][ff - 2] == EmptySquare &&
            board[0][BOARD_LEFT+2] == EmptySquare &&
            board[0][BOARD_LEFT+1] == EmptySquare &&
            board[0][BOARD_LEFT+0] == WhiteRook &&
            castlingRights[1] != NoRights && /* [HGM] check rights */
            ( castlingRights[2] == ff || castlingRights[6] == ff ) &&
	    (ignoreCheck ||
	     (!CheckTest(board, flags, 0, ff, 0, ff - 1, FALSE) &&
              !CheckTest(board, flags, 0, ff, 0, BOARD_LEFT+3, FALSE) &&
	      !CheckTest(board, flags, 0, ff, 0, ff - 2, FALSE)))) {

	    callback(board, flags,
		     ff==BOARD_WIDTH>>1 ? WhiteQueenSideCastle : WhiteQueenSideCastleWild,
                     0, ff, 0, ff - ((gameInfo.boardWidth+2)>>2), closure);
	}
	if (!(flags & F_WHITE_ON_MOVE) &&
	    (flags & F_BLACK_KCASTLE_OK) &&
            board[BOARD_HEIGHT-1][ff] == bKing &&
	    board[BOARD_HEIGHT-1][ff + 1] == EmptySquare &&
	    board[BOARD_HEIGHT-1][ff + 2] == EmptySquare &&
            board[BOARD_HEIGHT-1][BOARD_RGHT-3] == EmptySquare &&
            board[BOARD_HEIGHT-1][BOARD_RGHT-2] == EmptySquare &&
            board[BOARD_HEIGHT-1][BOARD_RGHT-1] == BlackRook &&
            castlingRights[3] != NoRights && /* [HGM] check rights */
            ( castlingRights[5] == ff || castlingRights[7] == ff ) &&
	    (ignoreCheck ||
	     (!CheckTest(board, flags, BOARD_HEIGHT-1, ff, BOARD_HEIGHT-1, ff + 1, FALSE) &&
              !CheckTest(board, flags, BOARD_HEIGHT-1, ff, BOARD_HEIGHT-1, BOARD_RGHT-3, FALSE) &&
              (gameInfo.variant != VariantJanus || !CheckTest(board, flags, BOARD_HEIGHT-1, ff, BOARD_HEIGHT-1, BOARD_RGHT-2, FALSE)) &&
	      !CheckTest(board, flags, BOARD_HEIGHT-1, ff, BOARD_HEIGHT-1, ff + 2, FALSE)))) {

	    callback(board, flags,
		     ff==BOARD_WIDTH>>1 ? BlackKingSideCastle : BlackKingSideCastleWild,
                     BOARD_HEIGHT-1, ff, BOARD_HEIGHT-1, ff + ((gameInfo.boardWidth+2)>>2) + (gameInfo.variant == VariantJanus), closure);
	}
	if (!(flags & F_WHITE_ON_MOVE) &&
	    (flags & F_BLACK_QCASTLE_OK) &&
            board[BOARD_HEIGHT-1][ff] == bKing &&
	    board[BOARD_HEIGHT-1][ff - 1] == EmptySquare &&
	    board[BOARD_HEIGHT-1][ff - 2] == EmptySquare &&
            board[BOARD_HEIGHT-1][BOARD_LEFT+2] == EmptySquare &&
            board[BOARD_HEIGHT-1][BOARD_LEFT+1] == EmptySquare &&
            board[BOARD_HEIGHT-1][BOARD_LEFT+0] == BlackRook &&
            castlingRights[4] != NoRights && /* [HGM] check rights */
            ( castlingRights[5] == ff || castlingRights[7] == ff ) &&
	    (ignoreCheck ||
	     (!CheckTest(board, flags, BOARD_HEIGHT-1, ff, BOARD_HEIGHT-1, ff - 1, FALSE) &&
              !CheckTest(board, flags, BOARD_HEIGHT-1, ff, BOARD_HEIGHT-1, BOARD_LEFT+3, FALSE) &&
              !CheckTest(board, flags, BOARD_HEIGHT-1, ff, BOARD_HEIGHT-1, ff - 2, FALSE)))) {

	    callback(board, flags,
		     ff==BOARD_WIDTH>>1 ? BlackQueenSideCastle : BlackQueenSideCastleWild,
                     BOARD_HEIGHT-1, ff, BOARD_HEIGHT-1, ff - ((gameInfo.boardWidth+2)>>2), closure);
	}
    }

  if((swap = gameInfo.variant == VariantSChess) || flags & F_FRC_TYPE_CASTLING) {

    /* generate all potential FRC castling moves (KxR), ignoring flags */
    /* [HGM] test if the Rooks we find have castling rights */
    /* In S-Chess we generate RxK for allowed castlings, for gating at Rook square */


    if ((flags & F_WHITE_ON_MOVE) != 0) {
        ff = castlingRights[2]; /* King file if we have any rights */
        if(ff != NoRights && board[0][ff] == WhiteKing) {
    if (appData.debugMode) {
        fprintf(debugFP, "FRC castling, %d %d %d %d %d %d\n",
                castlingRights[0],castlingRights[1],ff,castlingRights[3],castlingRights[4],castlingRights[5]);
    }
            ft = castlingRights[0]; /* Rook file if we have H-side rights */
            left  = ff+1;
            right = BOARD_RGHT-2;
            if(ff == BOARD_RGHT-2) left = right = ff-1;    /* special case */
            for(k=left; k<=right && ft != NoRights; k++) /* first test if blocked */
                if(k != ft && board[0][k] != EmptySquare) ft = NoRights;
            for(k=left; k<right && ft != NoRights; k++) /* then if not checked */
                if(!ignoreCheck && CheckTest(board, flags, 0, ff, 0, k, FALSE)) ft = NoRights;
            if(ft != NoRights && board[0][ft] == WhiteRook) {
                if(flags & F_FRC_TYPE_CASTLING) callback(board, flags, WhiteHSideCastleFR, 0, ff, 0, ft, closure);
                if(swap)                        callback(board, flags, WhiteHSideCastleFR, 0, ft, 0, ff, closure);
            }

            ft = castlingRights[1]; /* Rook file if we have A-side rights */
            left  = BOARD_LEFT+2;
            right = ff-1;
            if(ff <= BOARD_LEFT+2) { left = ff+1; right = BOARD_LEFT+3; }
            for(k=left; k<=right && ft != NoRights; k++) /* first test if blocked */
                if(k != ft && board[0][k] != EmptySquare) ft = NoRights;
            if(ft == 0 && ff != 1 && board[0][1] != EmptySquare) ft = NoRights; /* Rook can be blocked on b1 */
            if(ff > BOARD_LEFT+2)
            for(k=left+1; k<=right && ft != NoRights; k++) /* then if not checked */
                if(!ignoreCheck && CheckTest(board, flags, 0, ff, 0, k, FALSE)) ft = NoRights;
            if(ft != NoRights && board[0][ft] == WhiteRook) {
                if(flags & F_FRC_TYPE_CASTLING) callback(board, flags, WhiteASideCastleFR, 0, ff, 0, ft, closure);
                if(swap)                        callback(board, flags, WhiteASideCastleFR, 0, ft, 0, ff, closure);
            }
        }
    } else {
        ff = castlingRights[5]; /* King file if we have any rights */
        if(ff != NoRights && board[BOARD_HEIGHT-1][ff] == BlackKing) {
            ft = castlingRights[3]; /* Rook file if we have H-side rights */
            left  = ff+1;
            right = BOARD_RGHT-2;
            if(ff == BOARD_RGHT-2) left = right = ff-1;    /* special case */
            for(k=left; k<=right && ft != NoRights; k++) /* first test if blocked */
                if(k != ft && board[BOARD_HEIGHT-1][k] != EmptySquare) ft = NoRights;
            for(k=left; k<right && ft != NoRights; k++) /* then if not checked */
                if(!ignoreCheck && CheckTest(board, flags, BOARD_HEIGHT-1, ff, BOARD_HEIGHT-1, k, FALSE)) ft = NoRights;
            if(ft != NoRights && board[BOARD_HEIGHT-1][ft] == BlackRook) {
                if(flags & F_FRC_TYPE_CASTLING) callback(board, flags, BlackHSideCastleFR, BOARD_HEIGHT-1, ff, BOARD_HEIGHT-1, ft, closure);
                if(swap)                        callback(board, flags, BlackHSideCastleFR, BOARD_HEIGHT-1, ft, BOARD_HEIGHT-1, ff, closure);
            }

            ft = castlingRights[4]; /* Rook file if we have A-side rights */
            left  = BOARD_LEFT+2;
            right = ff-1;
            if(ff <= BOARD_LEFT+2) { left = ff+1; right = BOARD_LEFT+3; }
            for(k=left; k<=right && ft != NoRights; k++) /* first test if blocked */
                if(k != ft && board[BOARD_HEIGHT-1][k] != EmptySquare) ft = NoRights;
            if(ft == 0 && ff != 1 && board[BOARD_HEIGHT-1][1] != EmptySquare) ft = NoRights; /* Rook can be blocked on b8 */
            if(ff > BOARD_LEFT+2)
            for(k=left+1; k<=right && ft != NoRights; k++) /* then if not checked */
                if(!ignoreCheck && CheckTest(board, flags, BOARD_HEIGHT-1, ff, BOARD_HEIGHT-1, k, FALSE)) ft = NoRights;
            if(ft != NoRights && board[BOARD_HEIGHT-1][ft] == BlackRook) {
                if(flags & F_FRC_TYPE_CASTLING) callback(board, flags, BlackASideCastleFR, BOARD_HEIGHT-1, ff, BOARD_HEIGHT-1, ft, closure);
                if(swap)                        callback(board, flags, BlackASideCastleFR, BOARD_HEIGHT-1, ft, BOARD_HEIGHT-1, ff, closure);
            }
        }
    }

  }

    return FALSE;
}


typedef struct {
    int rking, fking;
    int check;
} CheckTestClosure;


extern void CheckTestCallback P((Board board, int flags, ChessMove kind,
				 int rf, int ff, int rt, int ft,
				 VOIDSTAR closure));


void
CheckTestCallback (Board board, int flags, ChessMove kind, int rf, int ff, int rt, int ft, VOIDSTAR closure)
{
    register CheckTestClosure *cl = (CheckTestClosure *) closure;

    if (rt == cl->rking && ft == cl->fking) {
	if(xqCheckers[EP_STATUS] >= 2 && xqCheckers[rf][ff]) return; // checker is piece with suspended checking power
	cl->check++;
	xqCheckers[rf][ff] = xqCheckers[EP_STATUS] & 1; // remember who is checking (if status == 1)
    }
    if( board[EP_STATUS] == EP_ROYAL_LION && (board[rt][ft] == WhiteLion || board[rt][ft] == BlackLion)
	&& (gameInfo.variant != VariantLion || board[rf][ff] != WhiteKing && board[rf][ff] != BlackKing) )
	cl->check++; // [HGM] lion: forbidden counterstrike against Lion equated to putting yourself in check
}


/* If the player on move were to move from (rf, ff) to (rt, ft), would
   he leave himself in check?  Or if rf == -1, is the player on move
   in check now?  enPassant must be TRUE if the indicated move is an
   e.p. capture.  The possibility of castling out of a check along the
   back rank is not accounted for (i.e., we still return nonzero), as
   this is illegal anyway.  Return value is the number of times the
   king is in check. */
int
CheckTest (Board board, int flags, int rf, int ff, int rt, int ft, int enPassant)
{
    CheckTestClosure cl;
    ChessSquare king = flags & F_WHITE_ON_MOVE ? WhiteKing : BlackKing;
    ChessSquare captured = EmptySquare, ep=0, trampled=0;
    int saveKill = killX;
    /*  Suppress warnings on uninitialized variables    */

    if(gameInfo.variant == VariantXiangqi)
        king = flags & F_WHITE_ON_MOVE ? WhiteWazir : BlackWazir;
    if(gameInfo.variant == VariantKnightmate)
        king = flags & F_WHITE_ON_MOVE ? WhiteUnicorn : BlackUnicorn;
    if(gameInfo.variant == VariantChu || gameInfo.variant == VariantShogi) { // strictly speaking this is not needed, as Chu officially has no check
	int r, f, k = king, royals=0, prince = flags & F_WHITE_ON_MOVE ? WhiteMonarch : BlackMonarch;
	if(gameInfo.variant == VariantShogi) prince -= 11;                   // White/BlackFalcon
	for(r=0; r<BOARD_HEIGHT; r++) for(f=BOARD_LEFT; f<BOARD_RGHT; f++) {
	    if(board[r][f] == k || board[r][f] == prince) {
		if(++royals > 1) return FALSE; // no check if we have two royals (ignores double captureby Lion!)
		king = board[r][f]; // remember hich one we had
	    }
	}
    }

    if (rt >= 0) {
	if (enPassant) {
	    captured = board[rf][ft];
	    board[rf][ft] = EmptySquare;
	} else {
 	    captured = board[rt][ft];
	    if(killX >= 0) { trampled = board[killY][killX]; board[killY][killX] = EmptySquare; killX = -1; }
	}
	if(rf == DROP_RANK) board[rt][ft] = ff; else { // [HGM] drop
	    board[rt][ft] = board[rf][ff];
	    if(rf != rt || ff != ft) board[rf][ff] = EmptySquare;
	}
	ep = board[EP_STATUS];
	if( captured == WhiteLion || captured == BlackLion ) { // [HGM] lion: Chu Lion-capture rules
	    ChessSquare victim = killX < 0 ? EmptySquare : trampled;
	    if( (board[rt][ft] == WhiteLion || board[rt][ft] == BlackLion) &&           // capturer is Lion
		(ff - ft > 1 || ft - ff > 1 || rf - rt > 1 || rt - rf > 1) &&           // captures from a distance
		(victim == EmptySquare || victim == WhitePawn || victim == BlackPawn) ) // no or worthless 'bridge'
		     board[EP_STATUS] = EP_ROYAL_LION; // on distant Lion x Lion victim must not be pseudo-legally protected
	}
    }

    /* For compatibility with ICS wild 9, we scan the board in the
       order a1, a2, a3, ... b1, b2, ..., h8 to find the first king,
       and we test only whether that one is in check. */
    for (cl.fking = BOARD_LEFT+0; cl.fking < BOARD_RGHT; cl.fking++)
	for (cl.rking = 0; cl.rking < BOARD_HEIGHT; cl.rking++) {
          if (board[cl.rking][cl.fking] == king) {
	      cl.check = 0;
              if(gameInfo.variant == VariantXiangqi) {
                  /* [HGM] In Xiangqi opposing Kings means check as well */
                  int i, dir;
                  dir = (king >= BlackPawn) ? -1 : 1;
                  for( i=cl.rking+dir; i>=0 && i<BOARD_HEIGHT &&
                                board[i][cl.fking] == EmptySquare; i+=dir );
                  if(i>=0 && i<BOARD_HEIGHT &&
                      board[i][cl.fking] == (dir>0 ? BlackWazir : WhiteWazir) )
                          cl.check++;
              }
	      GenPseudoLegal(board, flags ^ F_WHITE_ON_MOVE, CheckTestCallback, (VOIDSTAR) &cl, EmptySquare);
	      if(gameInfo.variant != VariantSpartan || cl.check == 0) // in Spartan Chess go on to test if other King is checked too
	         goto undo_move;  /* 2-level break */
	  }
      }

  undo_move:

    if (rt >= 0) {
	if(rf != DROP_RANK) // [HGM] drop
	    board[rf][ff] = board[rt][ft];
	if (enPassant) {
	    board[rf][ft] = captured;
	    board[rt][ft] = EmptySquare;
	} else {
	    if(saveKill >= 0) board[killY][killX] = trampled, killX = saveKill;
	    board[rt][ft] = captured;
	}
	board[EP_STATUS] = ep;
    }

    return cl.fking < BOARD_RGHT ? cl.check : 1000; // [HGM] atomic: return 1000 if we have no king
}

int
HasLion (Board board, int flags)
{
    int lion = F_WHITE_ON_MOVE & flags ? WhiteLion : BlackLion;
    int r, f;
    for(r=0; r<BOARD_HEIGHT; r++) for(f=BOARD_LEFT; f<BOARD_RGHT; f++)
        if(board[r][f] == lion) return 1;
    return 0;
}

ChessMove
LegalDrop (Board board, int flags, ChessSquare piece, int rt, int ft)
{   // [HGM] put drop legality testing in separate routine for clarity
    int n;
if(appData.debugMode) fprintf(debugFP, "LegalDrop: %d @ %d,%d)\n", piece, ft, rt);
    if(board[rt][ft] != EmptySquare) return ImpossibleMove; // must drop to empty square
    n = PieceToNumber(piece);
    if((gameInfo.holdingsWidth == 0 || (flags & F_WHITE_ON_MOVE ? board[n][BOARD_WIDTH-1] : board[BOARD_HEIGHT-1-n][0]) != piece)
	&& gameInfo.variant != VariantBughouse) // in bughouse we don't check for availability, because ICS doesn't always tell us
        return ImpossibleMove; // piece not available
    if(gameInfo.variant == VariantShogi) { // in Shogi lots of drops are forbidden!
        if((piece == WhitePawn || piece == WhiteQueen) && rt == BOARD_HEIGHT-1 ||
           (piece == BlackPawn || piece == BlackQueen) && rt == 0 ||
            piece == WhiteKnight && rt > BOARD_HEIGHT-3 ||
            piece == BlackKnight && rt < 2 ) return IllegalMove; // e.g. where dropped piece has no moves
        if(piece == WhitePawn || piece == BlackPawn) {
            int r, max = 1 + (BOARD_HEIGHT == 7); // two Pawns per file in Tori!
            for(r=1; r<BOARD_HEIGHT-1; r++)
                if(!(max -= (board[r][ft] == piece))) return IllegalMove; // or there already is a Pawn in file
            // should still test if we mate with this Pawn
        }
    } else if(gameInfo.variant == VariantSChess) { // only back-rank drops
        if (rt != (piece < BlackPawn ? 0 : BOARD_HEIGHT-1)) return IllegalMove;
    } else {
        if( (piece == WhitePawn || piece == BlackPawn) &&
            (rt == 0 || rt == BOARD_HEIGHT -1 ) )
            return IllegalMove; /* no pawn drops on 1st/8th */
    }
if(appData.debugMode) fprintf(debugFP, "LegalDrop: %d @ %d,%d)\n", piece, ft, rt);
    if (!(flags & F_IGNORE_CHECK) &&
	CheckTest(board, flags, DROP_RANK, piece, rt, ft, FALSE) ) return IllegalMove;
    return flags & F_WHITE_ON_MOVE ? WhiteDrop : BlackDrop;
}

extern void LegalityTestCallback P((Board board, int flags, ChessMove kind,
				    int rf, int ff, int rt, int ft,
				    VOIDSTAR closure));

void
LegalityTestCallback (Board board, int flags, ChessMove kind, int rf, int ff, int rt, int ft, VOIDSTAR closure)
{
    register LegalityTestClosure *cl = (LegalityTestClosure *) closure;

    if(board[rt][ft] != EmptySquare || kind==WhiteCapturesEnPassant || kind==BlackCapturesEnPassant)
	cl->captures++; // [HGM] losers: count legal captures
    if (rf == cl->rf && ff == cl->ff && rt == cl->rt && ft == cl->ft)
      cl->kind = kind;
}

ChessMove
LegalityTest (Board board, int flags, int rf, int ff, int rt, int ft, int promoChar)
{
    LegalityTestClosure cl; ChessSquare piece, filterPiece;

    if(quickFlag) flags = flags & ~1 | quickFlag & 1; // [HGM] speed: in quick mode quickFlag specifies side-to-move.
    if(rf == DROP_RANK) return LegalDrop(board, flags, ff, rt, ft);
    piece = filterPiece = board[rf][ff];
    if(PieceToChar(piece) == '~') filterPiece = DEMOTED piece;

    /* [HGM] Cobra and Falcon are wildcard pieces; consider all their moves legal */
    /* (perhaps we should disallow moves that obviously leave us in check?)              */
    if((piece == WhiteFalcon || piece == BlackFalcon ||
        piece == WhiteCobra  || piece == BlackCobra) && gameInfo.variant != VariantChu && !pieceDesc[piece])
        return CheckTest(board, flags, rf, ff, rt, ft, FALSE) ? IllegalMove : NormalMove;

    cl.rf = rf;
    cl.ff = ff;
    cl.rt = rFilter = rt; // [HGM] speed: filter on to-square
    cl.ft = fFilter = ft;
    cl.kind = IllegalMove;
    cl.captures = 0; // [HGM] losers: prepare to count legal captures.
    if(flags & F_MANDATORY_CAPTURE) filterPiece = EmptySquare; // [HGM] speed: do not filter in suicide, to find all captures
    GenLegal(board, flags, LegalityTestCallback, (VOIDSTAR) &cl, filterPiece);
    if((flags & F_MANDATORY_CAPTURE) && cl.captures && board[rt][ft] == EmptySquare
		&& cl.kind != WhiteCapturesEnPassant && cl.kind != BlackCapturesEnPassant)
	return(IllegalMove); // [HGM] losers: if there are legal captures, non-capts are illegal

    if(promoChar == 'x') promoChar = NULLCHAR; // [HGM] is this ever the case?
    if(gameInfo.variant == VariantSChess && promoChar && promoChar != '=' && board[rf][ff] != WhitePawn && board[rf][ff] != BlackPawn) {
        if(board[rf][ff] < BlackPawn) { // white
            if(rf != 0) return IllegalMove; // must be on back rank
            if(!(board[VIRGIN][ff] & VIRGIN_W)) return IllegalMove; // non-virgin
            if(board[PieceToNumber(CharToPiece(ToUpper(promoChar)))][BOARD_WIDTH-2] == 0) return ImpossibleMove;// must be in stock
            if(cl.kind == WhiteHSideCastleFR && (ff == BOARD_RGHT-2 || ff == BOARD_RGHT-3)) return ImpossibleMove;
            if(cl.kind == WhiteASideCastleFR && (ff == BOARD_LEFT+2 || ff == BOARD_LEFT+3)) return ImpossibleMove;
        } else {
            if(rf != BOARD_HEIGHT-1) return IllegalMove;
            if(!(board[VIRGIN][ff] & VIRGIN_B)) return IllegalMove; // non-virgin
            if(board[BOARD_HEIGHT-1-PieceToNumber(CharToPiece(ToLower(promoChar)))][1] == 0) return ImpossibleMove;
            if(cl.kind == BlackHSideCastleFR && (ff == BOARD_RGHT-2 || ff == BOARD_RGHT-3)) return ImpossibleMove;
            if(cl.kind == BlackASideCastleFR && (ff == BOARD_LEFT+2 || ff == BOARD_LEFT+3)) return ImpossibleMove;
        }
    } else
    if(gameInfo.variant == VariantChu) {
        if(cl.kind != NormalMove || promoChar == NULLCHAR || promoChar == '=') return cl.kind;
        if(promoChar != '+')
            return CharToPiece(promoChar) == EmptySquare ? ImpossibleMove : IllegalMove;
        if(PieceToChar(CHUPROMOTED board[rf][ff]) != '+') return ImpossibleMove;
        return flags & F_WHITE_ON_MOVE ? WhitePromotion : BlackPromotion;
    } else
    if(gameInfo.variant == VariantShogi) {
        /* [HGM] Shogi promotions. '=' means defer */
        if(rf != DROP_RANK && cl.kind == NormalMove) {
            ChessSquare piece = board[rf][ff];

            if(promoChar == PieceToChar(BlackQueen)) promoChar = NULLCHAR; /* [HGM] Kludge */
            if(promoChar == 'd' && (piece == WhiteRook   || piece == BlackRook)   ||
               promoChar == 'h' && (piece == WhiteBishop || piece == BlackBishop) ||
               promoChar == 'g' && (piece <= WhiteFerz || piece <= BlackFerz && piece >= BlackPawn) )
                  promoChar = '+'; // allowed ICS notations
if(appData.debugMode)fprintf(debugFP,"SHOGI promoChar = %c\n", promoChar ? promoChar : '-');
            if(promoChar != NULLCHAR && promoChar != '+' && promoChar != '=')
                return CharToPiece(promoChar) == EmptySquare ? ImpossibleMove : IllegalMove;
            else if(flags & F_WHITE_ON_MOVE) {
                if( (int) piece < (int) WhiteWazir &&
                     (rf >= BOARD_HEIGHT - BOARD_HEIGHT/3 || rt >= BOARD_HEIGHT - BOARD_HEIGHT/3) ) {
                    if( (piece == WhitePawn || piece == WhiteQueen) && rt > BOARD_HEIGHT-2 ||
                         piece == WhiteKnight && rt > BOARD_HEIGHT-3) /* promotion mandatory */
                       cl.kind = promoChar == '=' ? IllegalMove : WhitePromotion;
                    else /* promotion optional, default is defer */
                       cl.kind = promoChar == '+' ? WhitePromotion : WhiteNonPromotion;
                } else cl.kind = promoChar == '+' ? IllegalMove : NormalMove;
            } else {
                if( (int) piece < (int) BlackWazir && (rf < BOARD_HEIGHT/3 || rt < BOARD_HEIGHT/3) ) {
                    if( (piece == BlackPawn || piece == BlackQueen) && rt < 1 ||
                         piece == BlackKnight && rt < 2 ) /* promotion obligatory */
                       cl.kind = promoChar == '=' ? IllegalMove : BlackPromotion;
                    else /* promotion optional, default is defer */
                       cl.kind = promoChar == '+' ? BlackPromotion : BlackNonPromotion;
                } else cl.kind = promoChar == '+' ? IllegalMove : NormalMove;
            }
        }
    } else
    if (promoChar != NULLCHAR) {
	if(cl.kind == NormalMove && promoChar == '+') { // allow shogi-style promotion is pieceToChar specifies them
            ChessSquare piece = board[rf][ff];
            if(piece < BlackPawn ? piece > WhiteMan : piece > BlackMan) return ImpossibleMove; // already promoted
            // should test if in zone, really
            if(gameInfo.variant == VariantChuChess && (piece == WhiteKnight || piece == BlackKnight) && HasLion(board, flags))
                return IllegalMove;
            if(PieceToChar(PROMOTED piece) == '+') return flags & F_WHITE_ON_MOVE ? WhitePromotion : BlackPromotion;
        } else
	if(promoChar == '=') cl.kind = IllegalMove; else // [HGM] shogi: no deferred promotion outside Shogi
	if (cl.kind == WhitePromotion || cl.kind == BlackPromotion) {
	    ChessSquare piece = CharToPiece(flags & F_WHITE_ON_MOVE ? ToUpper(promoChar) : ToLower(promoChar));
	    if(piece == EmptySquare)
                cl.kind = ImpossibleMove; // non-existing piece
	    if(gameInfo.variant == VariantChuChess && promoChar == 'l' && HasLion(board, flags)) {
                cl.kind = IllegalMove; // no two Lions
	    } else if(gameInfo.variant == VariantSpartan && cl.kind == BlackPromotion ) {
		if(promoChar != PieceToChar(BlackKing)) {
		    if(CheckTest(board, flags, rf, ff, rt, ft, FALSE)) cl.kind = IllegalMove; // [HGM] spartan: only promotion to King was possible
		    if(piece == BlackLance) cl.kind = ImpossibleMove;
		} else { // promotion to King allowed only if we do not have two yet
		    int r, f, kings = 0;
		    for(r=0; r<BOARD_HEIGHT; r++) for(f=BOARD_LEFT; f<BOARD_RGHT; f++) kings += (board[r][f] == BlackKing);
		    if(kings == 2) cl.kind = IllegalMove;
		}
	    } else if(piece == WhitePawn && rt == BOARD_HEIGHT-1 ||
			  piece == BlackPawn && rt == 0) cl.kind = IllegalMove; // cannot stay Pawn on last rank in any variant
	    else if((piece == WhiteUnicorn || piece == BlackUnicorn) && gameInfo.variant == VariantKnightmate)
             cl.kind = IllegalMove; // promotion to Royal Knight not allowed
	    else if((piece == WhiteKing || piece == BlackKing) && gameInfo.variant != VariantSuicide && gameInfo.variant != VariantGiveaway)
             cl.kind = IllegalMove; // promotion to King usually not allowed
	} else {
	    cl.kind = IllegalMove;
	}
    }
    return cl.kind;
}

typedef struct {
    int count;
} MateTestClosure;

extern void MateTestCallback P((Board board, int flags, ChessMove kind,
				int rf, int ff, int rt, int ft,
				VOIDSTAR closure));

void
MateTestCallback (Board board, int flags, ChessMove kind, int rf, int ff, int rt, int ft, VOIDSTAR closure)
{
    register MateTestClosure *cl = (MateTestClosure *) closure;

    cl->count++;
}

/* Return MT_NONE, MT_CHECK, MT_CHECKMATE, or MT_STALEMATE */
int
MateTest (Board board, int flags)
{
    MateTestClosure cl;
    int inCheck, r, f, myPieces=0, hisPieces=0, nrKing=0;
    ChessSquare king = flags & F_WHITE_ON_MOVE ? WhiteKing : BlackKing;

    for(r=0; r<BOARD_HEIGHT; r++) for(f=BOARD_LEFT; f<BOARD_RGHT; f++) {
        // [HGM] losers: Count pieces and kings, to detect other unorthodox winning conditions
	nrKing += (board[r][f] == king);   // stm has king
        if( board[r][f] != EmptySquare ) {
	    if((int)board[r][f] <= (int)king && (int)board[r][f] >= (int)king - (int)WhiteKing + (int)WhitePawn)
		 myPieces++;
	    else hisPieces++;
	}
    }
    switch(gameInfo.variant) { // [HGM] losers: extinction wins
	case VariantShatranj:
		if(hisPieces == 1) return myPieces > 1 ? MT_BARE : MT_DRAW;
	default:
		break;
	case VariantAtomic:
		if(nrKing == 0) return MT_NOKING;
		break;
	case VariantLosers:
		if(myPieces == 1) return MT_BARE;
    }
    cl.count = 0;
    inCheck = GenLegal(board, flags, MateTestCallback, (VOIDSTAR) &cl, EmptySquare);
    // [HGM] 3check: yet to do!
    if (cl.count > 0) {
	return inCheck ? MT_CHECK : MT_NONE;
    } else {
        if(gameInfo.holdingsWidth && gameInfo.variant != VariantSuper && gameInfo.variant != VariantGreat
                                 && gameInfo.variant != VariantSChess && gameInfo.variant != VariantGrand) { // drop game
            int r, f, n, holdings = flags & F_WHITE_ON_MOVE ? BOARD_WIDTH-1 : 0;
            for(r=0; r<BOARD_HEIGHT; r++) for(f=BOARD_LEFT; f<BOARD_RGHT; f++) if(board[r][f] == EmptySquare) // all empty squares
                for(n=0; n<BOARD_HEIGHT; n++) // all pieces in hand
                    if(board[n][holdings] != EmptySquare) {
                        int moveType = LegalDrop(board, flags, board[n][holdings], r, f);
                        if(moveType == WhiteDrop || moveType == BlackDrop) return (inCheck ? MT_CHECK : MT_NONE); // we have legal drop
                    }
        }
	if(gameInfo.variant == VariantSuicide) // [HGM] losers: always stalemate, since no check, but result varies
		return myPieces == hisPieces ? MT_STALEMATE :
					myPieces > hisPieces ? MT_STAINMATE : MT_STEALMATE;
	else if(gameInfo.variant == VariantLosers) return inCheck ? MT_TRICKMATE : MT_STEALMATE;
	else if(gameInfo.variant == VariantGiveaway) return MT_STEALMATE; // no check exists, stalemated = win

        return inCheck ? MT_CHECKMATE
		       : (gameInfo.variant == VariantXiangqi || gameInfo.variant == VariantShatranj || IS_SHOGI(gameInfo.variant)) ?
			  MT_STAINMATE : MT_STALEMATE;
    }
}


extern void DisambiguateCallback P((Board board, int flags, ChessMove kind,
				    int rf, int ff, int rt, int ft,
				    VOIDSTAR closure));

void
DisambiguateCallback (Board board, int flags, ChessMove kind, int rf, int ff, int rt, int ft, VOIDSTAR closure)
{
    register DisambiguateClosure *cl = (DisambiguateClosure *) closure;
    int wildCard = FALSE; ChessSquare piece = board[rf][ff];

    // [HGM] wild: for wild-card pieces rt and rf are dummies
    if(piece == WhiteFalcon || piece == BlackFalcon ||
       piece == WhiteCobra  || piece == BlackCobra)
        wildCard = TRUE;

    if ((cl->pieceIn == EmptySquare || cl->pieceIn == board[rf][ff]
         || PieceToChar(board[rf][ff]) == '~'
              && cl->pieceIn == (ChessSquare)(DEMOTED board[rf][ff])
                                                                      ) &&
	(cl->rfIn == -1 || cl->rfIn == rf) &&
	(cl->ffIn == -1 || cl->ffIn == ff) &&
	(cl->rtIn == -1 || cl->rtIn == rt || wildCard) &&
	(cl->ftIn == -1 || cl->ftIn == ft || wildCard)) {

	if(cl->count && rf == cl->rf && ff == cl->ff) return; // duplicate move

	cl->count++;
	if(cl->count == 1 || board[rt][ft] != EmptySquare) {
	  // [HGM] oneclick: if multiple moves, be sure we remember capture
	  cl->piece = board[rf][ff];
	  cl->rf = rf;
	  cl->ff = ff;
	  cl->rt = wildCard ? cl->rtIn : rt;
	  cl->ft = wildCard ? cl->ftIn : ft;
	  cl->kind = kind;
	}
	cl->captures += (board[rt][ft] != EmptySquare); // [HGM] oneclick: count captures
    }
}

void
Disambiguate (Board board, int flags, DisambiguateClosure *closure)
{
    int illegal = 0; char c = closure->promoCharIn;

    if(quickFlag) flags = flags & ~1 | quickFlag & 1; // [HGM] speed: in quick mode quickFlag specifies side-to-move.
    closure->count = closure->captures = 0;
    closure->rf = closure->ff = closure->rt = closure->ft = 0;
    closure->kind = ImpossibleMove;
    rFilter = closure->rtIn; // [HGM] speed: only consider moves to given to-square
    fFilter = closure->ftIn;
    if(quickFlag) { // [HGM] speed: try without check test first, because if that is not ambiguous, we are happy
        GenLegal(board, flags|F_IGNORE_CHECK, DisambiguateCallback, (VOIDSTAR) closure, closure->pieceIn);
        if(closure->count > 1) { // gamble did not pay off. retry with check test to resolve ambiguity
            closure->count = closure->captures = 0;
            closure->rf = closure->ff = closure->rt = closure->ft = 0;
            closure->kind = ImpossibleMove;
            GenLegal(board, flags, DisambiguateCallback, (VOIDSTAR) closure, closure->pieceIn); // [HGM] speed: only pieces of requested type
        }
    } else
    GenLegal(board, flags, DisambiguateCallback, (VOIDSTAR) closure, closure->pieceIn); // [HGM] speed: only pieces of requested type
    if (closure->count == 0) {
	/* See if it's an illegal move due to check */
        illegal = 1;
        GenLegal(board, flags|F_IGNORE_CHECK, DisambiguateCallback, (VOIDSTAR) closure, closure->pieceIn);
	if (closure->count == 0) {
	    /* No, it's not even that */
	  if(!appData.testLegality && closure->pieceIn != EmptySquare) {
	    int f, r; // if there is only a single piece of the requested type on the board, use that
	    closure->rt = closure->rtIn, closure->ft = closure->ftIn;
	    for(r=0; r<BOARD_HEIGHT; r++) for(f=BOARD_LEFT; f<BOARD_RGHT; f++)
		if(board[r][f] == closure->pieceIn) closure->count++, closure->rf = r, closure->ff = f;
	    if(closure->count > 1) illegal = 0; // ambiguous
	  }
	  if(closure->count == 0) {
	    if (appData.debugMode) { int i, j;
		for(i=BOARD_HEIGHT-1; i>=0; i--) {
		    for(j=0; j<BOARD_WIDTH; j++)
			fprintf(debugFP, "%3d", (int) board[i][j]);
		    fprintf(debugFP, "\n");
		}
	    }
	    return;
	  }
	}
    } else if(pieceDefs && closure->count > 1) { // [HGM] gen: move is ambiguous under engine-defined rules
	DisambiguateClosure spare = *closure;
	pieceDefs = FALSE; spare.count = 0;     // See if the (erroneous) built-in rules would resolve that
        GenLegal(board, flags, DisambiguateCallback, (VOIDSTAR) &spare, closure->pieceIn);
	if(spare.count == 1) *closure = spare;  // It does, so use those in stead (game from file saved before gen patch?)
	pieceDefs = TRUE;
    }

    if (c == 'x') c = NULLCHAR; // get rid of any 'x' (which should never happen?)
    if(gameInfo.variant == VariantSChess && c && c != '=' && closure->piece != WhitePawn && closure->piece != BlackPawn) {
        if(closure->piece < BlackPawn) { // white
            if(closure->rf != 0) closure->kind = IllegalMove; // must be on back rank
            if(!(board[VIRGIN][closure->ff] & VIRGIN_W)) closure->kind = IllegalMove; // non-virgin
            if(board[PieceToNumber(CharToPiece(ToUpper(c)))][BOARD_WIDTH-2] == 0) closure->kind = ImpossibleMove;// must be in stock
            if(closure->kind == WhiteHSideCastleFR && (closure->ff == BOARD_RGHT-2 || closure->ff == BOARD_RGHT-3)) closure->kind = ImpossibleMove;
            if(closure->kind == WhiteASideCastleFR && (closure->ff == BOARD_LEFT+2 || closure->ff == BOARD_LEFT+3)) closure->kind = ImpossibleMove;
        } else {
            if(closure->rf != BOARD_HEIGHT-1) closure->kind = IllegalMove;
            if(!(board[VIRGIN][closure->ff] & VIRGIN_B)) closure->kind = IllegalMove; // non-virgin
            if(board[BOARD_HEIGHT-1-PieceToNumber(CharToPiece(ToLower(c)))][1] == 0) closure->kind = ImpossibleMove;
            if(closure->kind == BlackHSideCastleFR && (closure->ff == BOARD_RGHT-2 || closure->ff == BOARD_RGHT-3)) closure->kind = ImpossibleMove;
            if(closure->kind == BlackASideCastleFR && (closure->ff == BOARD_LEFT+2 || closure->ff == BOARD_LEFT+3)) closure->kind = ImpossibleMove;
        }
    } else
    if(gameInfo.variant == VariantChu) {
        if(c == '+') closure->kind = (flags & F_WHITE_ON_MOVE ? WhitePromotion : BlackPromotion); // for now, accept any
    } else
    if(gameInfo.variant == VariantShogi) {
        /* [HGM] Shogi promotions. On input, '=' means defer, '+' promote. Afterwards, c is set to '+' for promotions, NULL other */
        if(closure->rfIn != DROP_RANK && closure->kind == NormalMove) {
            ChessSquare piece = closure->piece;
            if (c == 'd' && (piece == WhiteRook   || piece == BlackRook)   ||
                c == 'h' && (piece == WhiteBishop || piece == BlackBishop) ||
                c == 'g' && (piece <= WhiteFerz || piece <= BlackFerz && piece >= BlackPawn) )
                   c = '+'; // allowed ICS notations
            if(c != NULLCHAR && c != '+' && c != '=') closure->kind = IllegalMove; // otherwise specifying a piece is illegal
            else if(flags & F_WHITE_ON_MOVE) {
                if( (int) piece < (int) WhiteWazir &&
                     (closure->rf >= BOARD_HEIGHT-(BOARD_HEIGHT/3) || closure->rt >= BOARD_HEIGHT-(BOARD_HEIGHT/3)) ) {
                    if( (piece == WhitePawn || piece == WhiteQueen) && closure->rt > BOARD_HEIGHT-2 ||
                         piece == WhiteKnight && closure->rt > BOARD_HEIGHT-3) /* promotion mandatory */
                       closure->kind = c == '=' ? IllegalMove : WhitePromotion;
                    else /* promotion optional, default is defer */
                       closure->kind = c == '+' ? WhitePromotion : WhiteNonPromotion;
                } else closure->kind = c == '+' ? IllegalMove : NormalMove;
            } else {
                if( (int) piece < (int) BlackWazir && (closure->rf < BOARD_HEIGHT/3 || closure->rt < BOARD_HEIGHT/3) ) {
                    if( (piece == BlackPawn || piece == BlackQueen) && closure->rt < 1 ||
                         piece == BlackKnight && closure->rt < 2 ) /* promotion obligatory */
                       closure->kind = c == '=' ? IllegalMove : BlackPromotion;
                    else /* promotion optional, default is defer */
                       closure->kind = c == '+' ? BlackPromotion : BlackNonPromotion;
                } else closure->kind = c == '+' ? IllegalMove : NormalMove;
            }
        }
        if(closure->kind == WhitePromotion || closure->kind == BlackPromotion) c = '+'; else
        if(closure->kind == WhiteNonPromotion || closure->kind == BlackNonPromotion) c = '=';
    } else
    if (closure->kind == WhitePromotion || closure->kind == BlackPromotion) {
        if(c == NULLCHAR) { // missing promoChar on mandatory promotion; use default for variant
            if(gameInfo.variant == VariantShatranj || gameInfo.variant == VariantCourier ||
               gameInfo.variant == VariantMakruk || gameInfo.variant == VariantASEAN)
                c = PieceToChar(BlackFerz);
            else if(gameInfo.variant == VariantGreat)
                c = PieceToChar(BlackMan);
            else if(gameInfo.variant == VariantGrand)
		    closure->kind = closure->rt != 0 && closure->rt != BOARD_HEIGHT-1 ? NormalMove : AmbiguousMove; // no default in Grand Chess
            else
                c = PieceToChar(BlackQueen);
        } else if(c == '=') closure->kind = IllegalMove; // no deferral outside Shogi
        else if(c == 'l' && gameInfo.variant == VariantChuChess && HasLion(board, flags)) closure->kind = IllegalMove;
    } else if (c == '+') { // '+' outside shogi, check if pieceToCharTable enabled it
        ChessSquare p = closure->piece;
        if(p > WhiteMan && p < BlackPawn || p > BlackMan || PieceToChar(PROMOTED p) != '+')
            closure->kind = ImpossibleMove; // used on non-promotable piece
        else if(gameInfo.variant == VariantChuChess && HasLion(board, flags)) closure->kind = IllegalMove;
    } else if (c != NULLCHAR) closure->kind = IllegalMove;

    closure->promoChar = ToLower(c); // this can be NULLCHAR! Note we keep original promoChar even if illegal.
    if(c != '+' && c != '=' && c != NULLCHAR && CharToPiece(flags & F_WHITE_ON_MOVE ? ToUpper(c) : ToLower(c)) == EmptySquare)
	closure->kind = ImpossibleMove; // but we cannot handle non-existing piece types!
    if (closure->count > 1) {
	closure->kind = AmbiguousMove;
    }
    if (illegal) {
	/* Note: If more than one illegal move matches, but no legal
	   moves, we return IllegalMove, not AmbiguousMove.  Caller
	   can look at closure->count to detect this.
	*/
	closure->kind = IllegalMove;
    }
}


typedef struct {
    /* Input */
    ChessSquare piece;
    int rf, ff, rt, ft;
    /* Output */
    ChessMove kind;
    int rank;
    int file;
    int either;
} CoordsToAlgebraicClosure;

extern void CoordsToAlgebraicCallback P((Board board, int flags,
					 ChessMove kind, int rf, int ff,
					 int rt, int ft, VOIDSTAR closure));

void
CoordsToAlgebraicCallback (Board board, int flags, ChessMove kind, int rf, int ff, int rt, int ft, VOIDSTAR closure)
{
    register CoordsToAlgebraicClosure *cl =
      (CoordsToAlgebraicClosure *) closure;

    if ((rt == cl->rt && ft == cl->ft || rt == rf && ft == ff) && // [HGM] null move matches any toSquare
        (board[rf][ff] == cl->piece
         || PieceToChar(board[rf][ff]) == '~' &&
            (ChessSquare) (DEMOTED board[rf][ff]) == cl->piece)
                                     ) {
	if (rf == cl->rf) {
	    if (ff == cl->ff) {
		cl->kind = kind; /* this is the move we want */
	    } else {
		cl->file++; /* need file to rule out this move */
	    }
	} else {
	    if (ff == cl->ff) {
		cl->rank++; /* need rank to rule out this move */
	    } else {
		cl->either++; /* rank or file will rule out this move */
	    }
	}
    }
}

/* Convert coordinates to normal algebraic notation.
   promoChar must be NULLCHAR or 'x' if not a promotion.
*/
ChessMove
CoordsToAlgebraic (Board board, int flags, int rf, int ff, int rt, int ft, int promoChar, char out[MOVE_LEN])
{
    ChessSquare piece;
    ChessMove kind;
    char *outp = out, c, capture;
    CoordsToAlgebraicClosure cl;

    if (rf == DROP_RANK) {
	if(ff == EmptySquare) { strncpy(outp, "--",3); return NormalMove; } // [HGM] pass
	/* Bughouse piece drop */
	*outp++ = ToUpper(PieceToChar((ChessSquare) ff));
	*outp++ = '@';
        *outp++ = ft + AAA;
        if(rt+ONE <= '9')
           *outp++ = rt + ONE;
        else { *outp++ = (rt+ONE-'0')/10 + '0';*outp++ = (rt+ONE-'0')%10 + '0'; }
	*outp = NULLCHAR;
	return (flags & F_WHITE_ON_MOVE) ? WhiteDrop : BlackDrop;
    }

    if (promoChar == 'x') promoChar = NULLCHAR;
    piece = board[rf][ff];
    if(PieceToChar(piece)=='~') piece = (ChessSquare)(DEMOTED piece);

    switch (piece) {
      case WhitePawn:
      case BlackPawn:
        kind = LegalityTest(board, flags, rf, ff, rt, ft, promoChar);
	if (kind == IllegalMove && !(flags&F_IGNORE_CHECK)) {
	    /* Keep short notation if move is illegal only because it
               leaves the player in check, but still return IllegalMove */
            kind = LegalityTest(board, flags|F_IGNORE_CHECK, rf, ff, rt, ft, promoChar);
	    if (kind == IllegalMove) break;
	    kind = IllegalMove;
	}
	/* Pawn move */
        *outp++ = ff + AAA;
	capture = board[rt][ft] != EmptySquare || kind == WhiteCapturesEnPassant || kind == BlackCapturesEnPassant;
        if (ff == ft && !capture) { /* [HGM] Xiangqi has straight noncapts! */
	    /* Non-capture; use style "e5" */
            if(rt+ONE <= '9')
               *outp++ = rt + ONE;
            else { *outp++ = (rt+ONE-'0')/10 + '0';*outp++ = (rt+ONE-'0')%10 + '0'; }
	} else {
	    /* Capture; use style "exd5" */
            if(capture)
            *outp++ = 'x';  /* [HGM] Xiangqi has sideway noncaptures across river! */
            *outp++ = ft + AAA;
            if(rt+ONE <= '9')
               *outp++ = rt + ONE;
            else { *outp++ = (rt+ONE-'0')/10 + '0';*outp++ = (rt+ONE-'0')%10 + '0'; }
	}
	/* Use promotion suffix style "=Q" */
	*outp = NULLCHAR;
        if (promoChar != NULLCHAR) {
            if(IS_SHOGI(gameInfo.variant)) {
                /* [HGM] ... but not in Shogi! */
                *outp++ = promoChar == '=' ? '=' : '+';
            } else {
                *outp++ = '=';
                *outp++ = ToUpper(promoChar);
            }
            *outp = NULLCHAR;
	}
        return kind;


      case WhiteKing:
      case BlackKing:
        /* Fabien moved code: FRC castling first (if KxR), wild castling second */
	/* Code added by Tord:  FRC castling. */
	if((piece == WhiteKing && board[rt][ft] == WhiteRook) ||
	   (piece == BlackKing && board[rt][ft] == BlackRook)) {
	  if(ft > ff)
	    safeStrCpy(out, "O-O", MOVE_LEN);
	  else
	    safeStrCpy(out, "O-O-O", MOVE_LEN);
	  return LegalityTest(board, flags, rf, ff, rt, ft, promoChar);
	}
	/* End of code added by Tord */
	/* Test for castling or ICS wild castling */
	/* Use style "O-O" (oh-oh) for PGN compatibility */
	else if (rf == rt &&
	    rf == ((piece == WhiteKing) ? 0 : BOARD_HEIGHT-1) &&
            (ft - ff > 1 || ff - ft > 1) &&  // No castling if legal King move (on narrow boards!)
            ((ff == BOARD_WIDTH>>1 && (ft == BOARD_LEFT+2 || ft == BOARD_RGHT-2)) ||
             (ff == (BOARD_WIDTH-1)>>1 && (ft == BOARD_LEFT+1 || ft == BOARD_RGHT-3)))) {
            if(ft==BOARD_LEFT+1 || ft==BOARD_RGHT-2)
	      snprintf(out, MOVE_LEN, "O-O%c%c", promoChar ? '/' : 0, ToUpper(promoChar));
            else
	      snprintf(out, MOVE_LEN, "O-O-O%c%c", promoChar ? '/' : 0, ToUpper(promoChar));

	    /* This notation is always unambiguous, unless there are
	       kings on both the d and e files, with "wild castling"
	       possible for the king on the d file and normal castling
	       possible for the other.  ICS rules for wild 9
	       effectively make castling illegal for either king in
	       this situation.  So I am not going to worry about it;
	       I'll just generate an ambiguous O-O in this case.
	    */
            return LegalityTest(board, flags, rf, ff, rt, ft, promoChar);
	}

	/* else fall through */
      default:
	/* Piece move */
	cl.rf = rf;
	cl.ff = ff;
	cl.rt = rFilter = rt; // [HGM] speed: filter on to-square
	cl.ft = fFilter = ft;
	cl.piece = piece;
	cl.kind = IllegalMove;
	cl.rank = cl.file = cl.either = 0;
        c = PieceToChar(piece) ;
        GenLegal(board, flags, CoordsToAlgebraicCallback, (VOIDSTAR) &cl, c!='~' ? piece : (DEMOTED piece)); // [HGM] speed

	if (cl.kind == IllegalMove && !(flags&F_IGNORE_CHECK)) {
	    /* Generate pretty moves for moving into check, but
	       still return IllegalMove.
	    */
            GenLegal(board, flags|F_IGNORE_CHECK, CoordsToAlgebraicCallback, (VOIDSTAR) &cl, c!='~' ? piece : (DEMOTED piece));
	    if (cl.kind == IllegalMove) break;
	    cl.kind = IllegalMove;
	}

	/* Style is "Nf3" or "Nxf7" if this is unambiguous,
	   else "Ngf3" or "Ngxf7",
	   else "N1f3" or "N5xf7",
	   else "Ng1f3" or "Ng5xf7".
	*/
        if( c == '~' || c == '+') {
           /* [HGM] print nonexistent piece as its demoted version */
           piece = (ChessSquare) (DEMOTED piece - 11*(gameInfo.variant == VariantChu));
        }
        if(c=='+') *outp++ = c;
        *outp++ = ToUpper(PieceToChar(piece));

	if (cl.file || (cl.either && !cl.rank)) {
            *outp++ = ff + AAA;
	}
	if (cl.rank) {
            if(rf+ONE <= '9')
                *outp++ = rf + ONE;
            else { *outp++ = (rf+ONE-'0')/10 + '0';*outp++ = (rf+ONE-'0')%10 + '0'; }
	}

	if(board[rt][ft] != EmptySquare)
	  *outp++ = 'x';

        *outp++ = ft + AAA;
        if(rt+ONE <= '9')
           *outp++ = rt + ONE;
        else { *outp++ = (rt+ONE-'0')/10 + '0';*outp++ = (rt+ONE-'0')%10 + '0'; }
        if (IS_SHOGI(gameInfo.variant)) {
            /* [HGM] in Shogi non-pawns can promote */
            *outp++ = promoChar; // Don't bother to correct move type, return value is never used!
        }
        else if (gameInfo.variant == VariantChuChess && promoChar ||
                 gameInfo.variant != VariantSuper && promoChar &&
                 (piece == WhiteLance || piece == BlackLance) ) { // Lance sometimes represents Pawn
            *outp++ = '=';
            *outp++ = ToUpper(promoChar);
        }
        else if (gameInfo.variant == VariantSChess && promoChar) { // and in S-Chess we have gating
            *outp++ = '/';
            *outp++ = ToUpper(promoChar);
        }
	*outp = NULLCHAR;
        return cl.kind;

      case EmptySquare:
	/* Moving a nonexistent piece */
	break;
    }

    /* Not a legal move, even ignoring check.
       If there was a piece on the from square,
       use style "Ng1g3" or "Ng1xe8";
       if there was a pawn or nothing (!),
       use style "g1g3" or "g1xe8".  Use "x"
       if a piece was on the to square, even
       a piece of the same color.
    */
    outp = out;
    c = 0;
    if (piece != EmptySquare && piece != WhitePawn && piece != BlackPawn) {
	int r, f;
      for(r=0; r<BOARD_HEIGHT; r++) for(f=BOARD_LEFT; f<=BOARD_RGHT; f++)
 		c += (board[r][f] == piece); // count on-board pieces of given type
	*outp++ = ToUpper(PieceToChar(piece));
    }
  if(c != 1) { // [HGM] but if there is only one piece of the mentioned type, no from-square, thank you!
    *outp++ = ff + AAA;
    if(rf+ONE <= '9')
       *outp++ = rf + ONE;
    else { *outp++ = (rf+ONE-'0')/10 + '0';*outp++ = (rf+ONE-'0')%10 + '0'; }
  }
    if (board[rt][ft] != EmptySquare) *outp++ = 'x';
    *outp++ = ft + AAA;
    if(rt+ONE <= '9')
       *outp++ = rt + ONE;
    else { *outp++ = (rt+ONE-'0')/10 + '0';*outp++ = (rt+ONE-'0')%10 + '0'; }
    /* Use promotion suffix style "=Q" */
    if (promoChar != NULLCHAR && promoChar != 'x') {
	*outp++ = '=';
	*outp++ = ToUpper(promoChar);
    }
    *outp = NULLCHAR;

    return IllegalMove;
}

// [HGM] XQ: the following code serves to detect perpetual chasing (Asian rules)

typedef struct {
    /* Input */
    int rf, ff, rt, ft;
    /* Output */
    int recaptures;
} ChaseClosure;

// I guess the following variables logically belong in the closure too, but I was too lazy and used globals

int preyStackPointer, chaseStackPointer;

struct {
unsigned char rf, ff, rt, ft;
} chaseStack[100];

struct {
unsigned char rank, file;
} preyStack[100];




// there are three new callbacks for use with GenLegal: for adding captures, deleting them, and finding a recapture

extern void AtacksCallback P((Board board, int flags, ChessMove kind,
				int rf, int ff, int rt, int ft,
				VOIDSTAR closure));

void
AttacksCallback (Board board, int flags, ChessMove kind, int rf, int ff, int rt, int ft, VOIDSTAR closure)
{   // For adding captures that can lead to chase indictment to the chaseStack
    if(board[rt][ft] == EmptySquare) return;                               // non-capture
    if(board[rt][ft] == WhitePawn && rt <  BOARD_HEIGHT/2) return;         // Pawn before river can be chased
    if(board[rt][ft] == BlackPawn && rt >= BOARD_HEIGHT/2) return;         // Pawn before river can be chased
    if(board[rf][ff] == WhitePawn  || board[rf][ff] == BlackPawn)  return; // Pawns are allowed to chase
    if(board[rf][ff] == WhiteWazir || board[rf][ff] == BlackWazir) return; // King is allowed to chase
    // move cannot be excluded from being a chase trivially (based on attacker and victim); save it on chaseStack
    chaseStack[chaseStackPointer].rf = rf;
    chaseStack[chaseStackPointer].ff = ff;
    chaseStack[chaseStackPointer].rt = rt;
    chaseStack[chaseStackPointer].ft = ft;
    chaseStackPointer++;
}

extern void ExistingAtacksCallback P((Board board, int flags, ChessMove kind,
				int rf, int ff, int rt, int ft,
				VOIDSTAR closure));

void
ExistingAttacksCallback (Board board, int flags, ChessMove kind, int rf, int ff, int rt, int ft, VOIDSTAR closure)
{   // for removing pre-exsting captures from the chaseStack, to be left with newly created ones
    int i;
    register ChaseClosure *cl = (ChaseClosure *) closure; //closure tells us the move played in the repeat loop

    if(board[rt][ft] == EmptySquare) return; // no capture
    if(rf == cl->rf && ff == cl->ff) { // attacks with same piece from new position are not considered new
	rf = cl->rt; ff = cl->ft;      // doctor their fromSquare so they will be recognized in chaseStack
    }
    // search move in chaseStack, and delete it if it occurred there (as we know now it is not a new capture)
    for(i=0; i<chaseStackPointer; i++) {
	if(chaseStack[i].rf == rf && chaseStack[i].ff == ff &&
	   chaseStack[i].rt == rt && chaseStack[i].ft == ft   ) {
	    // move found on chaseStack, delete it by overwriting with move popped from top of chaseStack
	    chaseStack[i] = chaseStack[--chaseStackPointer];
	    break;
	}
    }
}

extern void ProtectedCallback P((Board board, int flags, ChessMove kind,
				int rf, int ff, int rt, int ft,
				VOIDSTAR closure));

void
ProtectedCallback (Board board, int flags, ChessMove kind, int rf, int ff, int rt, int ft, VOIDSTAR closure)
{   // for determining if a piece (given through the closure) is protected
    register ChaseClosure *cl = (ChaseClosure *) closure; // closure tells us where to recapture

    if(rt == cl->rt && ft == cl->ft) cl->recaptures++;    // count legal recaptures to this square
    if(appData.debugMode && board[rt][ft] != EmptySquare)
	fprintf(debugFP, "try %c%c%c%c=%d\n", ff+AAA, rf+ONE,ft+AAA, rt+ONE, cl->recaptures);
}

extern char moveList[MAX_MOVES][MOVE_LEN];

int
PerpetualChase (int first, int last)
{   // this routine detects if the side to move in the 'first' position is perpetually chasing (when not checking)
    int i, j, k, tail;
    ChaseClosure cl;
    ChessSquare captured;

    preyStackPointer = 0;        // clear stack of chased pieces
    for(i=first; i<last; i+=2) { // for all positions with same side to move
        if(appData.debugMode) fprintf(debugFP, "judge position %i\n", i);
	chaseStackPointer = 0;   // clear stack that is going to hold possible chases
	// determine all captures possible after the move, and put them on chaseStack
	GenLegal(boards[i+1], PosFlags(i), AttacksCallback, &cl, EmptySquare);
	if(appData.debugMode) { int n;
	    for(n=0; n<chaseStackPointer; n++)
                fprintf(debugFP, "%c%c%c%c ", chaseStack[n].ff+AAA, chaseStack[n].rf+ONE,
                                              chaseStack[n].ft+AAA, chaseStack[n].rt+ONE);
            fprintf(debugFP, ": all capts\n");
	}
	// determine all captures possible before the move, and delete them from chaseStack
	cl.rf = moveList[i][1]-ONE; // prepare closure to pass move that led from i to i+1
	cl.ff = moveList[i][0]-AAA+BOARD_LEFT;
	cl.rt = moveList[i][3]-ONE;
	cl.ft = moveList[i][2]-AAA+BOARD_LEFT;
	CopyBoard(xqCheckers, nullBoard); xqCheckers[EP_STATUS] = 1; // giant kludge to make GenLegal ignore pre-existing checks
	GenLegal(boards[i],   PosFlags(i), ExistingAttacksCallback, &cl, EmptySquare);
	xqCheckers[EP_STATUS] = 0; // disable the generation of quasi-legal moves again
	if(appData.debugMode) { int n;
	    for(n=0; n<chaseStackPointer; n++)
                fprintf(debugFP, "%c%c%c%c ", chaseStack[n].ff+AAA, chaseStack[n].rf+ONE,
                                              chaseStack[n].ft+AAA, chaseStack[n].rt+ONE);
            fprintf(debugFP, ": new capts after %c%c%c%c\n", cl.ff+AAA, cl.rf+ONE, cl.ft+AAA, cl.rt+ONE);
	}
	// chaseSack now contains all captures made possible by the move
	for(j=0; j<chaseStackPointer; j++) { // run through chaseStack to identify true chases
            int attacker = (int)boards[i+1][chaseStack[j].rf][chaseStack[j].ff];
            int victim   = (int)boards[i+1][chaseStack[j].rt][chaseStack[j].ft];

	    if(attacker >= (int) BlackPawn) attacker = BLACK_TO_WHITE attacker; // convert to white, as piecee type
	    if(victim   >= (int) BlackPawn) victim   = BLACK_TO_WHITE victim;

	    if((attacker == WhiteKnight || attacker == WhiteCannon) && victim == WhiteRook)
		continue; // C or H attack on R is always chase; leave on chaseStack

	    if(attacker == victim) {
                if(LegalityTest(boards[i+1], PosFlags(i+1), chaseStack[j].rt,
                   chaseStack[j].ft, chaseStack[j].rf, chaseStack[j].ff, NULLCHAR) == NormalMove) {
			// we can capture back with equal piece, so this is no chase but a sacrifice
                        chaseStack[j] = chaseStack[--chaseStackPointer]; // delete the capture from the chaseStack
			j--; /* ! */ continue;
		}

	    }

	    // the attack is on a lower piece, or on a pinned or blocked equal one
	    CopyBoard(xqCheckers, nullBoard); xqCheckers[EP_STATUS] = 1;
	    CheckTest(boards[i+1], PosFlags(i+1), -1, -1, -1, -1, FALSE); // if we deliver check with our move, the checkers get marked
            // test if the victim is protected by a true protector. First make the capture.
	    captured = boards[i+1][chaseStack[j].rt][chaseStack[j].ft];
	    boards[i+1][chaseStack[j].rt][chaseStack[j].ft] = boards[i+1][chaseStack[j].rf][chaseStack[j].ff];
	    boards[i+1][chaseStack[j].rf][chaseStack[j].ff] = EmptySquare;
	    // Then test if the opponent can recapture
	    cl.recaptures = 0;         // prepare closure to pass recapture square and count moves to it
	    cl.rt = chaseStack[j].rt;
	    cl.ft = chaseStack[j].ft;
	    if(appData.debugMode) {
            	fprintf(debugFP, "test if we can recapture %c%c\n", cl.ft+AAA, cl.rt+ONE);
	    }
	    xqCheckers[EP_STATUS] = 2; // causes GenLegal to ignore the checks we delivered with the move, in real life evaded before we captured
            GenLegal(boards[i+1], PosFlags(i+1), ProtectedCallback, &cl, EmptySquare); // try all moves
	    xqCheckers[EP_STATUS] = 0; // disable quasi-legal moves again
	    // unmake the capture
	    boards[i+1][chaseStack[j].rf][chaseStack[j].ff] = boards[i+1][chaseStack[j].rt][chaseStack[j].ft];
            boards[i+1][chaseStack[j].rt][chaseStack[j].ft] = captured;
	    // if a recapture was found, piece is protected, and we are not chasing it.
	    if(cl.recaptures) { // attacked piece was defended by true protector, no chase
		chaseStack[j] = chaseStack[--chaseStackPointer]; // so delete from chaseStack
		j--; /* ! */
	    }
	}
	// chaseStack now contains all moves that chased
	if(appData.debugMode) { int n;
	    for(n=0; n<chaseStackPointer; n++)
                fprintf(debugFP, "%c%c%c%c ", chaseStack[n].ff+AAA, chaseStack[n].rf+ONE,
                                              chaseStack[n].ft+AAA, chaseStack[n].rt+ONE);
            fprintf(debugFP, ": chases\n");
	}
        if(i == first) { // copy all people chased by first move of repeat cycle to preyStack
	    for(j=0; j<chaseStackPointer; j++) {
                preyStack[j].rank = chaseStack[j].rt;
                preyStack[j].file = chaseStack[j].ft;
	    }
	    preyStackPointer = chaseStackPointer;
	}
	tail = 0;
        for(j=0; j<chaseStackPointer; j++) {
	    for(k=0; k<preyStackPointer; k++) {
		// search the victim of each chase move on the preyStack (first occurrence)
		if(chaseStack[j].ft == preyStack[k].file && chaseStack[j].rt == preyStack[k].rank ) {
		    if(k < tail) break; // piece was already identified as still being chased
		    preyStack[preyStackPointer] = preyStack[tail]; // move chased piece to bottom part of preyStack
		    preyStack[tail] = preyStack[k];                // by swapping
		    preyStack[k] = preyStack[preyStackPointer];
		    tail++;
		    break;
		}
	    }
	}
        preyStackPointer = tail; // keep bottom part of preyStack, popping pieces unchased on move i.
	if(appData.debugMode) { int n;
            for(n=0; n<preyStackPointer; n++)
                fprintf(debugFP, "%c%c ", preyStack[n].file+AAA, preyStack[n].rank+ONE);
            fprintf(debugFP, "always chased upto ply %d\n", i);
	}
        // now adjust the location of the chased pieces according to opponent move
        for(j=0; j<preyStackPointer; j++) {
            if(preyStack[j].rank == moveList[i+1][1]-ONE &&
               preyStack[j].file == moveList[i+1][0]-AAA+BOARD_LEFT) {
                preyStack[j].rank = moveList[i+1][3]-ONE;
                preyStack[j].file = moveList[i+1][2]-AAA+BOARD_LEFT;
                break;
            }
        }
    }
    return preyStackPointer ? 256*(preyStack[preyStackPointer].file - BOARD_LEFT + AAA) + (preyStack[preyStackPointer].rank + ONE)
				: 0; // if any piece was left on preyStack, it has been perpetually chased,and we return the
}
