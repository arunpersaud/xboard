%a 10000
%o 10000
%e 2000
%k 2500
%p 7000
%n 1000
%{
/*
 * parser.l -- lex parser of algebraic chess moves for XBoard
 * $Id$
 *
 * Copyright 1991 by Digital Equipment Corporation, Maynard, Massachusetts.
 * Enhancements Copyright 1992-95 Free Software Foundation, Inc.
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
 * The following terms apply to the enhanced version of XBoard distributed
 * by the Free Software Foundation:
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 * ------------------------------------------------------------------------
 */

/* This parser handles all forms of promotion.
 * The parser resolves ambiguous moves by searching and check-testing.
 * It also parses comments of the form [anything] or (anything).
 */

#include "config.h"

#define NO_CONSTRAINT	-1
#undef YYLMAX
#define YYLMAX			4096
#define UNPUT_BUF_SIZE		YYLMAX

#ifdef FLEX_SCANNER
/* yytext is probably a char*, but could be a char[].  yy_text is set
   in YY_DECL below, because if yytext is a char*, its value is not
   constant. */
char *yy_text;
#else /*!FLEX_SCANNER*/
/* yytext is definitely a char[], so yy_text can be set here, statically. */
char *yy_text = (char *) yytext;
#endif

#ifdef FLEX_SCANNER
/* This is flex */
#undef YY_INPUT
#define YY_INPUT(buf, result, max_size) my_yy_input(buf, &result, max_size)
#undef YY_DECL
#define YY_DECL                     \
    int _yylex YY_PROTO((void));    \
    int yylex YY_PROTO((void))      \
    {                               \
	int result = _yylex();      \
	yy_text = (char *) yytext;  \
	return(result);             \
    }                               \
    int _yylex YY_PROTO((void))
#else
/* This is lex */
#undef input
#undef output
#undef unput
#endif

/* The includes must be here, below the #undef input */

#include <ctype.h>

#if STDC_HEADERS
# include <stdlib.h>
# include <string.h>
#else /* not STDC_HEADERS */
# if HAVE_STRING_H
#  include <string.h>
# else /* not HAVE_STRING_H */
#  include <strings.h>
# endif /* not HAVE_STRING_H */
#endif /* not STDC_HEADERS */

#if HAVE_UNISTD_H
# include <unistd.h>
#endif

#if defined(_amigados)
# include <errno.h>
# if HAVE_FCNTL_H
#  include <fcntl.h>    /*  isatty() prototype  */
# endif /*  HAVE_FCNTL_H        */
#endif  /*  defined(_amigados)  */

#include "common.h"
#include "backend.h"
#include "frontend.h"
#include "parser.h"
#include "moves.h"

extern int PosFlags P((int));

extern Board	boards[MAX_MOVES];
int		yyboardindex;
int             yyskipmoves = FALSE;
char		currentMoveString[YYLMAX];
#ifndef FLEX_SCANNER
char		unputBuffer[UNPUT_BUF_SIZE];
int		unputCount = 0;
#endif

#ifdef FLEX_SCANNER
void my_yy_input P((char *buf, int *result, int max_size));
#else /*!FLEX_SCANNER*/
static int input P((void));
static void output P((int ch));
static void unput P((int ch));
int yylook P((void));
int yyback P((int *, int));
#endif
#undef yywrap
int yywrap P((void));
extern void CopyBoard P((Board to, Board from));

%}
%%

[RrBbNnQqKkPp][/]?[a-h][1-8][xX:-]?[a-h][1-8](=?\(?[RrBbNnQqKk]\)?)? {
    /*
     * Fully-qualified algebraic move, possibly with promotion
     */
    int skip1 = 0, skip2 = 0;
    ChessSquare piece;
    ChessMove result;
    
    if (yyskipmoves) return (int) AmbiguousMove; /* not disambiguated */

    /* remove the / */
    if (yytext[1] == '/') skip1 = 1;
    
    /* remove the [xX:-] */
    if ((yytext[3+skip1] == 'x') || (yytext[3+skip1] == 'X') ||
	(yytext[3+skip1] == '-') || (yytext[3+skip1] == ':')) skip2 = 1;
    
    currentMoveString[0] = yytext[1+skip1];
    currentMoveString[1] = yytext[2+skip1];
    currentMoveString[2] = yytext[3+skip1+skip2];
    currentMoveString[3] = yytext[4+skip1+skip2];
    currentMoveString[4] = NULLCHAR;
    
    if (yyleng-skip1-skip2 > 5) {
	if (yytext[yyleng-1] == ')') {
	    currentMoveString[4] = ToLower(yytext[yyleng-2]);
	} else {
	    currentMoveString[4] = ToLower(yytext[yyleng-1]);
	}
	currentMoveString[5] = NULLCHAR;
    }

    piece = boards[yyboardindex]
      [currentMoveString[1] - '1'][currentMoveString[0] - 'a'];
    if (ToLower(yytext[0]) != ToLower(PieceToChar(piece)))
      return (int) IllegalMove;

    result = LegalityTest(boards[yyboardindex],
			  PosFlags(yyboardindex), EP_UNKNOWN,
			  currentMoveString[1] - '1',
			  currentMoveString[0] - 'a',
			  currentMoveString[3] - '1',
			  currentMoveString[2] - 'a',
			  currentMoveString[4]);

    if (currentMoveString[4] == NULLCHAR &&
	(result == WhitePromotionQueen || result == BlackPromotionQueen)) {
	currentMoveString[4] = 'q';
	currentMoveString[5] = NULLCHAR;
    }

    return (int) result;
}

[a-h][1-8][xX:-]?[a-h][1-8](=?\(?[RrBbNnQqKk]\)?)?	{
    /*
     * Simple algebraic move, possibly with promotion
     */
    int skip = 0;
    ChessMove result;

    if (yyskipmoves) return (int) AmbiguousMove; /* not disambiguated */

    /* remove the [xX:-] */
    if ((yytext[2] == 'x') || (yytext[2] == 'X') ||
	(yytext[2] == '-') || (yytext[2] == ':')) skip = 1;

    currentMoveString[0] = yytext[0];
    currentMoveString[1] = yytext[1];
    currentMoveString[2] = yytext[2+skip];
    currentMoveString[3] = yytext[3+skip];
    currentMoveString[4] = NULLCHAR;

    if (yyleng-skip > 4) {
	if (yytext[yyleng-1] == ')') {
	    currentMoveString[4] = ToLower(yytext[yyleng-2]);
	} else {
	    currentMoveString[4] = ToLower(yytext[yyleng-1]);
	}
	currentMoveString[5] = NULLCHAR;
    }

    result = LegalityTest(boards[yyboardindex],
			  PosFlags(yyboardindex), EP_UNKNOWN,
			  currentMoveString[1] - '1',
			  currentMoveString[0] - 'a',
			  currentMoveString[3] - '1',
			  currentMoveString[2] - 'a',
			  currentMoveString[4]);

    if (currentMoveString[4] == NULLCHAR &&
	(result == WhitePromotionQueen || result == BlackPromotionQueen)) {
	currentMoveString[4] = 'q';
	currentMoveString[5] = NULLCHAR;
    }

    return (int) result;
}

[a-h][1-8](=?\(?[RrBbNnQqKk]\)?)?	{
    /*
     * Pawn move, possibly with promotion
     */
    DisambiguateClosure cl;
    int skip = 0;

    if (yyskipmoves) return (int) AmbiguousMove; /* not disambiguated */

    /* remove the =() */
    if (yytext[2] == '=') skip++;
    if (yytext[2+skip] == '(') skip++;

    cl.pieceIn = WhiteOnMove(yyboardindex) ? WhitePawn : BlackPawn;
    cl.rfIn = -1;
    cl.ffIn = yytext[0] - 'a';
    cl.rtIn = yytext[1] - '1';
    cl.ftIn = yytext[0] - 'a';
    cl.promoCharIn = yytext[2+skip];
    Disambiguate(boards[yyboardindex],
		 PosFlags(yyboardindex), EP_UNKNOWN, &cl);

    currentMoveString[0] = cl.ff + 'a';
    currentMoveString[1] = cl.rf + '1';
    currentMoveString[2] = cl.ft + 'a';
    currentMoveString[3] = cl.rt + '1';
    currentMoveString[4] = cl.promoChar;
    currentMoveString[5] = NULLCHAR;

    return (int) cl.kind;
}


(ab|bc|cd|de|ef|fg|gh|hg|gf|fe|ed|dc|cb|ba|([a-h][xX:-][a-h]))(=?\(?[RrBbNnQqKk]\)?)?(ep|"e.p.")? {
    /*
     * Pawn capture, possibly with promotion, possibly ambiguous
     */
    DisambiguateClosure cl;
    int skip1 = 0, skip2 = 0;

    if (yyskipmoves) return (int) AmbiguousMove; /* not disambiguated */

    /* remove trailing ep or e.p. (nonstandard PGN) */
    if (yytext[yyleng-1] == 'p') {
      yyleng -= 2;
      yytext[yyleng] = NULLCHAR;
    } else if (yytext[yyleng-1] == '.') {
      yyleng -= 4;
      yytext[yyleng] = NULLCHAR;
    }

    /* remove the [xX:-] and =() */
    if ((yytext[1] == 'x') || (yytext[1] == 'X')
	|| (yytext[1] == ':') || (yytext[1] == '-')) skip1 = 1;
    if (yytext[2+skip1] == '=') skip2++;
    if (yytext[2+skip1+skip2] == '(') skip2++;

    cl.pieceIn = WhiteOnMove(yyboardindex) ? WhitePawn : BlackPawn;
    cl.rfIn = -1;
    cl.ffIn = yytext[0] - 'a';
    cl.rtIn = -1;
    cl.ftIn = yytext[1+skip1] - 'a';
    cl.promoCharIn = yytext[2+skip1+skip2];
    Disambiguate(boards[yyboardindex],
		 PosFlags(yyboardindex), EP_UNKNOWN, &cl);

    currentMoveString[0] = cl.ff + 'a';
    currentMoveString[1] = cl.rf + '1';
    currentMoveString[2] = cl.ft + 'a';
    currentMoveString[3] = cl.rt + '1';
    currentMoveString[4] = cl.promoChar;
    currentMoveString[5] = NULLCHAR;

    return (int) cl.kind;
}

[a-h][xX:]?[a-h][1-8](=?\(?[RrBbNnQqKk]\)?)?(ep|"e.p.")? {
    /*
     * unambiguously abbreviated Pawn capture, possibly with promotion
     */
    int skip = 0;
    ChessMove result;

    if (yyskipmoves) return (int) AmbiguousMove; /* not disambiguated */

    /* remove trailing ep or e.p. (nonstandard PGN) */
    if (yytext[yyleng-1] == 'p') {
      yyleng -= 2;
      yytext[yyleng] = NULLCHAR;
    } else if (yytext[yyleng-1] == '.') {
      yyleng -= 4;
      yytext[yyleng] = NULLCHAR;
    }

    /* remove the [xX:-] */
    if ((yytext[1] == 'x') || (yytext[1] == 'X')
	|| (yytext[1] == ':') || (yytext[1] == '-')) skip = 1;

    currentMoveString[0] = yytext[0];
    currentMoveString[2] = yytext[1+skip];
    currentMoveString[3] = yytext[2+skip];
    if (WhiteOnMove(yyboardindex)) {
	if (yytext[2+skip] == '1') return (int) ImpossibleMove;
	currentMoveString[1] = yytext[2+skip] - 1;
    } else {
	if (yytext[2+skip] == '8') return (int) ImpossibleMove;
	currentMoveString[1] = yytext[2+skip] + 1;
    }
    if (yyleng-skip > 3) {
	if (yytext[yyleng-1] == ')')
	  currentMoveString[4] = ToLower(yytext[yyleng-2]);
	else
	  currentMoveString[4] = ToLower(yytext[yyleng-1]);
	currentMoveString[5] = NULLCHAR;
    } else {
	currentMoveString[4] = NULLCHAR;
    }

    result = LegalityTest(boards[yyboardindex],
			  PosFlags(yyboardindex), EP_UNKNOWN,
			  currentMoveString[1] - '1',
			  currentMoveString[0] - 'a',
			  currentMoveString[3] - '1',
			  currentMoveString[2] - 'a',
			  currentMoveString[4]);

    if (currentMoveString[4] == NULLCHAR &&
	(result == WhitePromotionQueen || result == BlackPromotionQueen)) {
	currentMoveString[4] = 'q';
	currentMoveString[5] = NULLCHAR;
    }

    if (result != IllegalMove) return (int) result;

    /* Special case: improperly written en passant capture */
    if (WhiteOnMove(yyboardindex)) {
	if (currentMoveString[3] == '5') {
	    currentMoveString[1] = '5';
	    currentMoveString[3] = '6';
	} else {
	    return (int) IllegalMove;
	}
    } else {
	if (currentMoveString[3] == '4') {
	    currentMoveString[1] = '4';
	    currentMoveString[3] = '3';
	} else {
	    return (int) IllegalMove;
	}
    }

    result = LegalityTest(boards[yyboardindex],
			  PosFlags(yyboardindex), EP_UNKNOWN,
			  currentMoveString[1] - '1',
			  currentMoveString[0] - 'a',
			  currentMoveString[3] - '1',
			  currentMoveString[2] - 'a',
			  currentMoveString[4]);

    if (result == WhiteCapturesEnPassant || result == BlackCapturesEnPassant)
      return (int) result;
    else
      return (int) IllegalMove;
}

[RrBbNnQqKk][xX:-]?[a-h][1-8]  {
    /*
     * piece move, possibly ambiguous
     */
    DisambiguateClosure cl;
    int skip = 0;

    if (yyskipmoves) return (int) AmbiguousMove; /* not disambiguated */

    /* remove the [xX:-] */
    if ((yytext[1] == 'x') || (yytext[1] == 'X')
	|| (yytext[1] == ':') || (yytext[1] == '-')) skip = 1;

    if (WhiteOnMove(yyboardindex)) {
	cl.pieceIn = CharToPiece(ToUpper(yytext[0]));
    } else {
	cl.pieceIn = CharToPiece(ToLower(yytext[0]));
    }
    cl.rfIn = -1;
    cl.ffIn = -1;
    cl.rtIn = yytext[2+skip] - '1';
    cl.ftIn = yytext[1+skip] - 'a';
    cl.promoCharIn = NULLCHAR;
    Disambiguate(boards[yyboardindex],
		 PosFlags(yyboardindex), EP_UNKNOWN, &cl);

    currentMoveString[0] = cl.ff + 'a';
    currentMoveString[1] = cl.rf + '1';
    currentMoveString[2] = cl.ft + 'a';
    currentMoveString[3] = cl.rt + '1';
    currentMoveString[4] = cl.promoChar;
    currentMoveString[5] = NULLCHAR;

    return (int) cl.kind;
}

[RrBbNnQqKk][a-h1-8][xX:-]?[a-h][1-8]	{
    /*
     * piece move with rank or file disambiguator
     */
    DisambiguateClosure cl;
    int skip = 0;

    if (yyskipmoves) return (int) AmbiguousMove; /* not disambiguated */

    /* remove the [xX:-] */
    if ((yytext[2] == 'x') || (yytext[2] == 'X')
	|| (yytext[2] == ':') || (yytext[2] == '-')) skip = 1;

    if (WhiteOnMove(yyboardindex)) {
	cl.pieceIn = CharToPiece(ToUpper(yytext[0]));
    } else {
	cl.pieceIn = CharToPiece(ToLower(yytext[0]));
    }
    if (isalpha(yytext[1])) {
	cl.rfIn = -1;
	cl.ffIn = yytext[1] - 'a';
    } else {
	cl.rfIn = yytext[1] - '1';
	cl.ffIn = -1;
    }
    cl.rtIn = yytext[3+skip] - '1';
    cl.ftIn = yytext[2+skip] - 'a';
    cl.promoCharIn = NULLCHAR;
    Disambiguate(boards[yyboardindex],
		 PosFlags(yyboardindex), EP_UNKNOWN, &cl);

    currentMoveString[0] = cl.ff + 'a';
    currentMoveString[1] = cl.rf + '1';
    currentMoveString[2] = cl.ft + 'a';
    currentMoveString[3] = cl.rt + '1';
    currentMoveString[4] = cl.promoChar;
    currentMoveString[5] = NULLCHAR;

    return (int) cl.kind;
}

000|0-0-0|ooo|OOO|o-o-o|O-O-O	{
    int rf, ff, rt, ft;

    if (yyskipmoves) return (int) AmbiguousMove; /* not disambiguated */

    if (WhiteOnMove(yyboardindex)) {
	if (boards[yyboardindex][0][3] == WhiteKing) {
	    /* ICS wild castling */
	    strcpy(currentMoveString, "d1f1");
	    rf = 0;
	    ff = 3;
	    rt = 0;
	    ft = 5;
	} else {
	    strcpy(currentMoveString, "e1c1");
	    rf = 0;
	    ff = 4;
	    rt = 0;
	    ft = 2;
	}
    } else{ 
	if (boards[yyboardindex][7][3] == BlackKing) {
	    /* ICS wild castling */
	    strcpy(currentMoveString, "d8f8");
	    rf = 7;
	    ff = 3;
	    rt = 7;
	    ft = 5;
	} else {
	    strcpy(currentMoveString, "e8c8");
	    rf = 7;
	    ff = 4;
	    rt = 7;
	    ft = 2;
	}
    }
    return (int) LegalityTest(boards[yyboardindex],
			      PosFlags(yyboardindex), EP_UNKNOWN,
			      rf, ff, rt, ft, NULLCHAR);
}

00|0-0|oo|OO|o-o|O-O	{
    int rf, ff, rt, ft;

    if (yyskipmoves) return (int) AmbiguousMove; /* not disambiguated */

    if (WhiteOnMove(yyboardindex)) {
	if (boards[yyboardindex][0][3] == WhiteKing) {
	    /* ICS wild castling */
	    strcpy(currentMoveString, "d1b1");
	    rf = 0;
	    ff = 3;
	    rt = 0;
	    ft = 1;
	} else {
	    strcpy(currentMoveString, "e1g1");
	    rf = 0;
	    ff = 4;
	    rt = 0;
	    ft = 6;
	}
    } else {
	if (boards[yyboardindex][7][3] == BlackKing) {
	    /* ICS wild castling */
	    strcpy(currentMoveString, "d8b8");
	    rf = 7;
	    ff = 3;
	    rt = 7;
	    ft = 1;
	} else {
	    strcpy(currentMoveString, "e8g8");
	    rf = 7;
	    ff = 4;
	    rt = 7;
	    ft = 6;
	}
    }
    return (int) LegalityTest(boards[yyboardindex],
			      PosFlags(yyboardindex), EP_UNKNOWN,
			      rf, ff, rt, ft, NULLCHAR);
}

[PpNnBbRrQq]@[a-h][1-8] {
    /* Bughouse piece drop.  No legality checking for now. */
    currentMoveString[1] = '@';
    currentMoveString[2] = yytext[2];
    currentMoveString[3] = yytext[3];
    currentMoveString[4] = NULLCHAR;
    if (WhiteOnMove(yyboardindex)) {
	currentMoveString[0] = ToUpper(yytext[0]);
	return (int) WhiteDrop;
    } else {
	currentMoveString[0] = ToLower(yytext[0]);
	return (int) BlackDrop;
    }
}

[Rr]esign(s|ed)?  {
    if (WhiteOnMove(yyboardindex))
      return (int) BlackWins;
    else
      return (int) WhiteWins;
}

(([Ww](hite)?)|([Bb](lack)?))" "(([Rr]esign)|([Ff]orfeit))(s|ed)?  {
    return (int) (ToUpper(yytext[0]) == 'W' ? BlackWins : WhiteWins);
}

(([Ww](hite)?)|([Bb](lack)?))" "[Dd]isconnect(s|ed)  {
    return (int) GameUnfinished;
}

[Ss]talemate  {
    return (int) GameIsDrawn;
}

"+-+"  {
    return (int) GameIsDrawn;
}

([Cc]heck)?[Mm]ate {
    if (WhiteOnMove(yyboardindex))
      return (int) BlackWins;
    else
      return (int) WhiteWins;
}

"++"  {
    if (WhiteOnMove(yyboardindex))
      return (int) BlackWins;
    else
      return (int) WhiteWins;
}

[Dd]raw(n)?(" "by)?(" "[Rr]epetition)|(" "[Aa]gree(d|ment))  {
    return (int) GameIsDrawn;
}

[Dd]raw(n)?(" (".*")")?  {
    return (int) GameIsDrawn;
}

(([Ww](hite)?)|([Bb](lack)?))" "([Mm]ate(s|ed)?)|([Ww][io]n(s)?.*)  {
    return (int) (ToUpper(yytext[0]) == 'W' ? WhiteWins : BlackWins);
}

(([Ww](hite)?)|([Bb](lack)?))" "([Mm]ate(s|ed)?)|([Ll]os[tes]+.*)  {
    return (int) (ToUpper(yytext[0]) == 'W' ? BlackWins : WhiteWins);
}

("{"[^\}\n]*"} ")?(1-0|"1 - 0"|"1/0"|"1 / 0"|"1:0"|"1 : 0")(" (".*")"|" {".*"}")? { 
    return (int) WhiteWins;
}

("{"[^\}\n]*"} ")?(0-1|"0 - 1"|"0/1"|"0 / 1"|"0:1"|"0 : 1")(" (".*")"|" {".*"}")? { 
    return (int) BlackWins;
}

("{"[^\}\n]*"} ")?("1/2"|"1 / 2")(" "?[-:]" "?("1/2"|"1 / 2"))?(" (".*")"|" {".*"}")? {
    return (int) GameIsDrawn;
}

("{"[^\}\n]*"} ")?"*"(" (".*")"|" {".*"}")? {
    return (int) GameUnfinished;
}

[1-9][0-9]*/"."?[ \t\n]*[a-hNnPpRrBQqKkOo]    {
    /* move numbers */
    if ((yyleng == 1) && (yytext[0] == '1'))
      return (int) MoveNumberOne;
}

\([0-9]+:[0-9][0-9](\.[0-9]+)?\)|\{[0-9]+:[0-9][0-9](\.[0-9]+)?\} {
    /* elapsed time indication, e.g. (0:12) or {10:21.071} */ 
    return (int) ElapsedTime;
}

"[--"[^\]]*"--]" {
    /* position diagram enclosed in [-- --] */
    return (int) PositionDiagram;
}

^"{--------------"\n[^\}]*\n"--------------}"$ {
    /* position diagram enclosed in {-- --} */
    return (int) PositionDiagram;
}

\[[ \t\n]*[A-Za-z0-9][A-Za-z0-9_+#=-]*[ \t\n]*\"[^"]*\"[ \t\n]*\] {
    return (int) PGNTag;
}    

[Gg](nu|NU)" "?[Cc](hess|HESS).*[Gg](ame|AME) {
    return (int) GNUChessGame;
}

^[#;%]" "[^ ]*(" game file"|" position file").*$ {
    return (int) XBoardGame;
}

\$[0-9]+	{				/* numeric annotation glyph */
    return (int) NAG;
}

\{[^\}]*\}	{        			/* anything in {} */
    return (int) Comment; 
}

;.*$ {                                          /* ; to end of line */
    return (int) Comment;
}

\[[^\]]*\]	{        			/* anything in [] */
    return (int) Comment; 
}

\([^()]*(\([^()]*\)[^()]*)+[^()]*\)  { 	  	/* nested () */
    return (int) Comment; 
}

\([^)][^)]+\)   { 				/* >=2 chars in () */
    return (int) Comment; 
}       

^[-a-zA-Z0-9]+:" ".*(\n[ \t]+.*)*  {
        /* Skip mail headers */
}

[a-zA-Z0-9'-]+			{
        /* Skip random words */
}

.|\n				{
        /* Skip everything else */
}

%%


static char *StringToLex;

#ifndef FLEX_SCANNER
static FILE *lexFP;

static int input()
{
    int ret;
    
    if (StringToLex != NULL) {
	ret = *StringToLex;
	if (ret == NULLCHAR)
	  ret = EOF;
	else
	  StringToLex++;
    } else if (unputCount > 0) {
	ret = unputBuffer[--unputCount];
    } else {
	ret = fgetc(lexFP);
    }    

    if (ret == EOF) 
      return 0;
    else
      return ret;
}

/*
 * Return offset of next pattern within current file
 */
int yyoffset()
{
    int offset = ftell(lexFP) - unputCount;

    if (offset < 0) {
	offset = 0;
    }
    return(offset);
}
 
static void output(ch)
     int ch;
{
    fprintf(stderr, "PARSER BUG: unmatched character '%c' (0%o)\n",
	    ch, ch);
}

static void unput(ch)
     int ch;
{
    if (ch == 0) return;
    if (StringToLex != NULL) {
	StringToLex--;
    } else {
	if (unputCount >= UNPUT_BUF_SIZE)
	  fprintf(stderr, "PARSER BUG: unput buffer overflow '%c' (0%o)\n",
		  ch, ch);
	unputBuffer[unputCount++] = ch;
    }
}

/* Get ready to lex from a new file.  Kludge below sticks
   an artificial newline at the front of the file, which the
   above grammar ignores, but which makes ^ at start of pattern
   match at the real start of the file.
*/
void yynewfile(f)
     FILE *f;
{
    lexFP = f;
    StringToLex = NULL;
    unputCount = 0;
    unput('\n'); /* kludge */
}

/* Get ready to lex from a string.  ^ at start of pattern WON'T
   match at the start of the string!
*/
void yynewstr(s)
     char *s;
{
    lexFP = NULL;
    StringToLex = s;
    unputCount = 0;
}
#endif /*!FLEX_SCANNER*/

#ifdef FLEX_SCANNER
void my_yy_input(buf, result, max_size)
     char *buf;
     int *result;
     int max_size;
{
    int count;

    if (StringToLex != NULL) {
	count = 0;
	while (*StringToLex != NULLCHAR) {
	    *buf++ = *StringToLex++;
	    count++;
	}
	*result = count;
	return;
    } else {
	count = fread(buf, 1, max_size, yyin);
	if (count == 0) {
	    *result = YY_NULL;
	} else {
	    *result = count;
	}
	return;
    }    
}

static YY_BUFFER_STATE my_file_buffer = NULL;

/*
    Return offset of next pattern in the current file.
*/
int yyoffset()
{
    int pos = yy_c_buf_p - yy_current_buffer->yy_ch_buf;

    return(ftell(yy_current_buffer->yy_input_file) -
         yy_n_chars + pos);
}


void yynewstr(s)
     char *s;
{
    if (my_file_buffer != NULL)
      yy_delete_buffer(my_file_buffer);
    StringToLex = s;
    my_file_buffer = yy_create_buffer(stdin, YY_BUF_SIZE);
    yy_switch_to_buffer(my_file_buffer);
}

void yynewfile(f)
     FILE *f;
{
    if (my_file_buffer != NULL)
      yy_delete_buffer(my_file_buffer);
    StringToLex = NULL;
    my_file_buffer = yy_create_buffer(f, YY_BUF_SIZE);
    yy_switch_to_buffer(my_file_buffer);
}
#endif /*FLEX_SCANNER*/

int yywrap()
{
    return TRUE;
}

/* Parse a move from the given string s */
/* ^ at start of pattern WON'T work here unless using flex */
ChessMove yylexstr(boardIndex, s)
     int boardIndex;
     char *s;
{
    ChessMove ret;
    char *oldStringToLex;
#ifdef FLEX_SCANNER
    YY_BUFFER_STATE buffer, oldBuffer;
#endif
    
    yyboardindex = boardIndex;
    oldStringToLex = StringToLex;
    StringToLex = s;
#ifdef FLEX_SCANNER
    buffer = yy_create_buffer(stdin, YY_BUF_SIZE);
    oldBuffer = YY_CURRENT_BUFFER;
    yy_switch_to_buffer(buffer);
#endif /*FLEX_SCANNER*/

    ret = (ChessMove) yylex();

#ifdef FLEX_SCANNER
    if (oldBuffer != NULL) 
      yy_switch_to_buffer(oldBuffer);
    yy_delete_buffer(buffer);
#endif /*FLEX_SCANNER*/
    StringToLex = oldStringToLex;

    return ret;
}
