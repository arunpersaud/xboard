/*
 * moves.h - Move generation and checking
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

extern ChessSquare PromoPiece P((ChessMove moveType));
extern ChessMove PromoCharToMoveType P((int whiteOnMove, int promoChar));
extern char PieceToChar P((ChessSquare p));
extern ChessSquare CharToPiece P((int c));
extern int PieceToNumber P((ChessSquare p));

extern void CopyBoard P((Board to, Board from));
extern int CompareBoards P((Board board1, Board board2));
extern char pieceToChar[(int)EmptySquare+1];
extern char pieceNickName[(int)EmptySquare];
extern char *pieceDesc[(int)EmptySquare];
extern Board initialPosition;
extern Boolean pieceDefs;

typedef void (*MoveCallback) P((Board board, int flags, ChessMove kind,
				int rf, int ff, int rt, int ft,
				VOIDSTAR closure));

/* Values for flags arguments */
#define F_WHITE_ON_MOVE 1
#define F_WHITE_KCASTLE_OK 2
#define F_WHITE_QCASTLE_OK 4
#define F_BLACK_KCASTLE_OK 8
#define F_BLACK_QCASTLE_OK 16
#define F_ALL_CASTLE_OK (F_WHITE_KCASTLE_OK | F_WHITE_QCASTLE_OK | \
			 F_BLACK_KCASTLE_OK | F_BLACK_QCASTLE_OK)
#define F_IGNORE_CHECK 32
#define F_KRIEGSPIEL_CAPTURE 64 /* pawns can try to capture invisible pieces */
#define F_ATOMIC_CAPTURE 128    /* capturing piece explodes, destroying itself
				   and all non-pawns on adjacent squares;
				   destroying your own king is illegal */
#define F_FRC_TYPE_CASTLING 256 /* generate castlings as captures of own Rook */
#define F_MANDATORY_CAPTURE 0x200
#define F_NULL_MOVE         0x400

/* Special epfile values. [HGM] positive values are non-reversible moves! */
#define EP_NONE (-6)           /* [HGM] Tricky! order matters:            */
#define EP_UNKNOWN (-1)        /*       >= EP_UNKNOWN spoils rep-draw     */
#define EP_CAPTURE (-2)        /*       <= EP_NONE is reversible move     */
#define EP_PAWN_MOVE (-3)
#define EP_IRON_LION (-4)
#define EP_ROYAL_LION (-5)
#define EP_REP_DRAW   (-15)
#define EP_RULE_DRAW  (-14)
#define EP_INSUF_DRAW  (-13)
#define EP_DRAWS (-10)
#define EP_WINS (-9)
#define EP_BEROLIN_A 16        /* [HGM] berolina: add to file if pawn to be taken of a-side of e.p.file */
#define EP_CHECKMATE 100       /* [HGM] verify: record mates in epStatus for easy claim verification    */
#define EP_STALEMATE -16

/* Call callback once for each pseudo-legal move in the given
   position, except castling moves.  A move is pseudo-legal if it is
   legal, or if it would be legal except that it leaves the king in
   check.  In the arguments, epfile is EP_NONE if the previous move
   was not a double pawn push, or the file 0..7 if it was, or
   EP_UNKNOWN if we don't know and want to allow all e.p. captures.
   Promotion moves generated are to Queen only.
*/
extern void GenPseudoLegal P((Board board, int flags,
			      MoveCallback callback, VOIDSTAR closure, ChessSquare filter));

/* Like GenPseudoLegal, but include castling moves and (unless
   F_IGNORE_CHECK is set in the flags) omit moves that would leave the
   king in check.  The CASTLE_OK flags are true if castling is not yet
   ruled out by a move of the king or rook.  Return TRUE if the player
   on move is currently in check and F_IGNORE_CHECK is not set.
*/
extern int GenLegal P((Board board, int flags,
			MoveCallback callback, VOIDSTAR closure, ChessSquare filter));

/* If the player on move were to move from (rf, ff) to (rt, ft), would
   he leave himself in check?  Or if rf == -1, is the player on move
   in check now?  enPassant must be TRUE if the indicated move is an
   e.p. capture.  The possibility of castling out of a check along the
   back rank is not accounted for (i.e., we still return nonzero), as
   this is illegal anyway.  Return value is the number of times the
   king is in check. */
extern int CheckTest P((Board board, int flags,
			int rf, int ff, int rt, int ft, int enPassant));

/* Is a move from (rf, ff) to (rt, ft) legal for the player whom the
   flags say is on move?  Other arguments as in GenPseudoLegal.
   Returns the type of move made, taking promoChar into account. */
extern ChessMove LegalityTest P((Board board, int flags,
				 int rf, int ff, int rt, int ft,
				 int promoChar));

#define MT_NONE 0
#define MT_CHECK 1
#define MT_CHECKMATE 2
#define MT_STALEMATE 3
#define MT_STAINMATE 4 /* [HGM] xq: for games where being stalemated counts as a loss    */
#define MT_STEALMATE 5 /* [HGM] losers: for games where being stalemated counts as a win */
#define MT_TRICKMATE 6 /* [HGM] losers: for games where being checkmated counts as a win */
#define MT_BARE      7 /* [HGM] shatranj: for games where having bare king loses         */
#define MT_DRAW      8 /* [HGM] shatranj: other draws                                    */
#define MT_NOKING    9 /* [HGM] atomic: for games lost through king capture              */

/* Return MT_NONE, MT_CHECK, MT_CHECKMATE, or MT_STALEMATE */
extern int MateTest P((Board board, int flags));

typedef struct {
    /* Input data */
    ChessSquare pieceIn;        /* EmptySquare if unknown */
    int rfIn, ffIn, rtIn, ftIn; /* -1 if unknown */
    int promoCharIn;            /* NULLCHAR if unknown */
    /* Output data for matched move */
    ChessMove kind;
    ChessSquare piece;
    int rf, ff, rt, ft;
    int promoChar; /* 'q' if a promotion and promoCharIn was NULLCHAR */
    int count;     /* Number of possibilities found */
    int captures;  /* [HGM] oneclick: number of matching captures */
} DisambiguateClosure;

/* Disambiguate a partially-known move */
void Disambiguate P((Board board, int flags, DisambiguateClosure *closure));


/* Convert coordinates to normal algebraic notation.
   promoChar must be NULLCHAR or '.' if not a promotion.
*/
ChessMove CoordsToAlgebraic P((Board board, int flags,
			       int rf, int ff, int rt, int ft,
			       int promoChar, char out[MOVE_LEN]));

extern int quickFlag, killX, killY, legNr;
