%a 10000
%o 10000
%e 2000
%k 2500
%p 7000
%n 1000
%{
/*
 * parser.l -- lex parser of algebraic chess moves for XBoard
 *
 * Copyright 1991 by Digital Equipment Corporation, Maynard,
 * Massachusetts.
 *
 * Enhancements Copyright 1992-2001, 2002, 2003, 2004, 2005,
 * 2006, 2007, 2008, 2009, 2010, 2011 Free Software Foundation, Inc.
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
 * along with this program. If not, see http://www.gnu.org/licenses/.  
 *
 *------------------------------------------------------------------------
 ** See the file ChangeLog for a revision history.  */

/* This parser handles all forms of promotion.
 * The parser resolves ambiguous moves by searching and check-testing.
 * It also parses comments of the form [anything] or (anything).
 *
 * [HGM] Parser extensively modified for bigger boards, Shogi-like syntax,
 * and unknow pieces. All pieces are now mandatory upper case, but can be
 * any letter A-Z. Files must be lower case (as before), but can run upto 'l'.
 * Ranks can be 0-9. The parser returns 0 for off-board files and ranks.
 * For an unknown piece (as mover or promotion piece) it returns
 * IllegalMove, like it does when the piece doesn't match.
 * Promotions can now also be appended Shogi-style, a bare '=' or '+',
 * and this is then returned as promotion character. The piece indicator
 * can be prefixed by a '+' to indicate it is a promoted piece.
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
/* [AP] use prototypes in function declarations */
#define YY_USE_PROTOS

#ifdef YY_USE_PROTOS
#define YY_PROTO(proto) proto
#else
#define YY_PROTO(proto) ()
#endif
/* end of [AP] fix */

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

"+"?[A-Z][/]?[a-l][0-9][xX:-]?[a-l][0-9](([=/]?\(?[A-Z]\)?)|[=+])? {
    /*
     * Fully-qualified algebraic move, possibly with promotion
     */
    int skip1 = 0, skip2 = 0, skip3 = 0, promoted = 0;
    ChessSquare piece;
    ChessMove result;
    char c;
    
    if (yyskipmoves) return (int) AmbiguousMove; /* not disambiguated */

    if (yytext[0] == '+') skip1 = skip3 = promoted = 1; /* [HGM] Shogi promoted */

    /* remove the / */
    if (yytext[1+skip1] == '/')  skip1++; 
    
    /* remove the [xX:-] */
    if ((yytext[3+skip1] == 'x') || (yytext[3+skip1] == 'X') ||
        (yytext[3+skip1] == '-') || (yytext[3+skip1] == ':')) skip2 = 1;
    
    currentMoveString[0] = yytext[1+skip1];
    currentMoveString[1] = yytext[2+skip1];
    currentMoveString[2] = yytext[3+skip1+skip2];
    currentMoveString[3] = yytext[4+skip1+skip2];
    currentMoveString[4] = NULLCHAR;
    
    if (appData.debugMode) {
        fprintf(debugFP, "Parser Qa1b2: yyleng=%d\n",
        yyleng);
    }

    if (yyleng-skip1-skip2 > 5) { char c;
        if (yytext[yyleng-1] == ')') {
            c = currentMoveString[4] = ToLower(yytext[yyleng-2]);
	} else {
            c = currentMoveString[4] = ToLower(yytext[yyleng-1]);
	}
        if(c == '+' && gameInfo.variant != VariantShogi) c = currentMoveString[4] = NULLCHAR; // + means check outside Shogi
	currentMoveString[5] = NULLCHAR;
    }

    if (appData.debugMode) {
        fprintf(debugFP, "parser: %s\n", currentMoveString);
    }
    /* [HGM] do not allow values beyond board size */
    if(currentMoveString[1] - ONE >= BOARD_HEIGHT ||
       currentMoveString[1] - ONE <  0            ||
       currentMoveString[0] - AAA >= BOARD_RGHT   ||
       currentMoveString[3] - ONE >= BOARD_HEIGHT ||
       currentMoveString[3] - ONE <  0            ||
       currentMoveString[2] - AAA >= BOARD_RGHT   ||
       currentMoveString[0] - AAA <  BOARD_LEFT   ||
       currentMoveString[2] - AAA <  BOARD_LEFT     )
      return ImpossibleMove;

    piece = boards[yyboardindex]
      [currentMoveString[1] - ONE][currentMoveString[0] - AAA];
    if(PieceToChar(piece) == '+' && appData.icsActive) promoted = 1, yytext[skip3] = PieceToChar(DEMOTED piece); // trust ICS
    if(promoted) piece = (ChessSquare) (DEMOTED piece);
    c = PieceToChar(piece);
    if(c == '~') c = PieceToChar((ChessSquare) (DEMOTED piece));
    if (ToLower(yytext[skip3]) != ToLower(c))
      return (int) IllegalMove;

    result = LegalityTest(boards[yyboardindex],
			  PosFlags(yyboardindex)&~F_MANDATORY_CAPTURE, // [HGM] losers: might think we can e.p.!
                          currentMoveString[1] - ONE,
                          currentMoveString[0] - AAA,
                          currentMoveString[3] - ONE,
                          currentMoveString[2] - AAA,
			  currentMoveString[4]);

    if (currentMoveString[4] == NULLCHAR) {
      if(result == WhitePromotion  || result == BlackPromotion) {
        if(gameInfo.variant == VariantCourier || gameInfo.variant == VariantShatranj)
            currentMoveString[4] = PieceToChar(BlackFerz);
        else if(gameInfo.variant == VariantGreat)
            currentMoveString[4] = PieceToChar(BlackMan);
        else if(gameInfo.variant == VariantShogi)
            currentMoveString[4] = '+';
        else
            currentMoveString[4] = PieceToChar(BlackQueen);
      } else if(result == WhiteNonPromotion  || result == BlackNonPromotion)
            currentMoveString[4] = '=';
      currentMoveString[5] = NULLCHAR;
    }

    return (int) result;
}

[a-l][0-9][xX:-]?[a-l][0-9](([=/]?\(?[A-Za-z]\)?)|[=+])?      {
    /*
     * Simple algebraic move, possibly with promotion
     * [HGM] Engine moves are received in this format, with lower-case promoChar!
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

    if (yyleng-skip > 4) { char c;
	if (yytext[yyleng-1] == ')') {
            c = currentMoveString[4] = ToLower(yytext[yyleng-2]);
	} else {
            c = currentMoveString[4] = ToLower(yytext[yyleng-1]);
	}
        if(c == '+' && gameInfo.variant != VariantShogi) currentMoveString[4] = NULLCHAR; // + means check outside Shogi
	currentMoveString[5] = NULLCHAR;
    }

    /* [HGM] do not allow values beyond board size */
    if(currentMoveString[1] - ONE >= BOARD_HEIGHT ||
       currentMoveString[1] - ONE <  0            ||
       currentMoveString[0] - AAA >= BOARD_RGHT   ||
       currentMoveString[3] - ONE >= BOARD_HEIGHT ||
       currentMoveString[3] - ONE <  0            ||
       currentMoveString[2] - AAA >= BOARD_RGHT   ||
       currentMoveString[0] - AAA <  BOARD_LEFT   ||
       currentMoveString[2] - AAA <  BOARD_LEFT     )
      return ImpossibleMove;

    result = LegalityTest(boards[yyboardindex],
			  PosFlags(yyboardindex)&~F_MANDATORY_CAPTURE, // [HGM] losers: might think we can e.p.!
                          currentMoveString[1] - ONE,
                          currentMoveString[0] - AAA,
                          currentMoveString[3] - ONE,
                          currentMoveString[2] - AAA,
			  currentMoveString[4]);

    if (currentMoveString[4] == NULLCHAR) {
      if(result == WhitePromotion  || result == BlackPromotion) {
        if(gameInfo.variant == VariantShatranj || gameInfo.variant == VariantCourier || gameInfo.variant == VariantMakruk)
            currentMoveString[4] = PieceToChar(BlackFerz);
        else if(gameInfo.variant == VariantGreat)
            currentMoveString[4] = PieceToChar(BlackMan);
        else if(gameInfo.variant == VariantShogi)
            currentMoveString[4] = '+'; // Queen might not be defined in mini variants!
        else
            currentMoveString[4] = PieceToChar(BlackQueen);
      } else if(result == WhiteNonPromotion  || result == BlackNonPromotion)
            currentMoveString[4] = '=';
      currentMoveString[5] = NULLCHAR;
    } else if(appData.testLegality && gameInfo.variant != VariantSChess && // strip off unnecessary and false promo characters
       !(result == WhitePromotion  || result == BlackPromotion ||
         result == WhiteNonPromotion || result == BlackNonPromotion)) currentMoveString[4] = NULLCHAR;

    return (int) result;
}

[A-L][0-9][xX:-]?[A-L][0-9]      {
    /*
     * Simple algebraic move, in capitals
     * [HGM] Some Xiangqi engines use this format ('ICCS notation'). So no promotions!
     */
    int skip = 0;
    ChessMove result;

    if (yyskipmoves) return (int) AmbiguousMove; /* not disambiguated */

    /* remove the [xX:-] */
    if ((yytext[2] == 'x') || (yytext[2] == 'X') ||
	(yytext[2] == '-') || (yytext[2] == ':')) skip = 1;

    currentMoveString[0] = yytext[0]+32;
    currentMoveString[1] = yytext[1];
    currentMoveString[2] = yytext[2+skip]+32;
    currentMoveString[3] = yytext[3+skip];
    currentMoveString[4] = NULLCHAR;

    /* [HGM] do not allow values beyond board size */
    if(currentMoveString[1] - ONE >= BOARD_HEIGHT ||
       currentMoveString[1] - ONE <  0            ||
       currentMoveString[0] - AAA >= BOARD_RGHT   ||
       currentMoveString[3] - ONE >= BOARD_HEIGHT ||
       currentMoveString[3] - ONE <  0            ||
       currentMoveString[2] - AAA >= BOARD_RGHT   ||
       currentMoveString[0] - AAA <  BOARD_LEFT   ||
       currentMoveString[2] - AAA <  BOARD_LEFT     )
      return ImpossibleMove;

    result = LegalityTest(boards[yyboardindex],
			  PosFlags(yyboardindex)&~F_MANDATORY_CAPTURE, // [HGM] losers: might think we can e.p.!
                          currentMoveString[1] - ONE,
                          currentMoveString[0] - AAA,
                          currentMoveString[3] - ONE,
                          currentMoveString[2] - AAA,
			  currentMoveString[4]);

    return (int) result;
}

[a-l][0-9]((=?\(?[A-Za-z]\)?)|[=+])?       {
    /*
     * Pawn move, possibly with promotion
     */
    DisambiguateClosure cl;
    int skip = 0;

    if (yyskipmoves) return (int) AmbiguousMove; /* not disambiguated */

    /* remove the =() */
    if (yytext[2] == '=' && yytext[3] != NULLCHAR) skip++;
    if (yytext[2+skip] == '(') skip++;

    cl.pieceIn = WhiteOnMove(yyboardindex) ? WhitePawn : BlackPawn;
    cl.rfIn = -1;
    cl.ffIn = yytext[0] - AAA;
    cl.rtIn = yytext[1] - ONE;
    cl.ftIn = yytext[0] - AAA;
    cl.promoCharIn = ToLower(yytext[2+skip]);
    if(cl.promoCharIn == '+' && gameInfo.variant != VariantShogi) cl.promoCharIn = NULLCHAR; // + means check outside Shogi

    /* [HGM] do not allow values beyond board size */
    if(cl.rtIn >= BOARD_HEIGHT ||
       cl.rtIn <  0            ||
       cl.ffIn >= BOARD_RGHT   ||
       cl.ftIn <  BOARD_LEFT     )
      return ImpossibleMove;

    Disambiguate(boards[yyboardindex], PosFlags(yyboardindex), &cl);

    currentMoveString[0] = cl.ff + AAA;
    currentMoveString[1] = cl.rf + ONE;
    currentMoveString[2] = cl.ft + AAA;
    currentMoveString[3] = cl.rt + ONE;
    currentMoveString[4] = cl.promoChar;
    currentMoveString[5] = NULLCHAR;

    return (int) cl.kind;
}


(ab|bc|cd|de|ef|fg|gh|hi|ij|jk|kl|lk|kj|ji|ih|hg|gf|fe|ed|dc|cb|ba|aa|bb|cc|dd|ee|ff|gg|hh|ii|jj|kk|ll|([a-l][xX:-][a-l]))((=?\(?[A-Z]\)?)|ep|"e.p."|=)? {
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
    if (yytext[2+skip1] == '=' && yytext[3+skip1] != NULLCHAR) skip2++;
    if (yytext[2+skip1+skip2] == '(') skip2++;

    cl.pieceIn = WhiteOnMove(yyboardindex) ? WhitePawn : BlackPawn;
    cl.rfIn = -1;
    cl.ffIn = yytext[0] - AAA;
    cl.rtIn = -1;
    cl.ftIn = yytext[1+skip1] - AAA;
    cl.promoCharIn = yytext[2+skip1+skip2];
    if(cl.promoCharIn == '+' && gameInfo.variant != VariantShogi) cl.promoCharIn = NULLCHAR; // + means check outside Shogi

    /* [HGM] do not allow values beyond board size */
    if(cl.ffIn >= BOARD_RGHT  ||
       cl.ffIn <  BOARD_LEFT  ||
       cl.ftIn >= BOARD_RGHT  ||
       cl.ftIn <  BOARD_LEFT     )
      return ImpossibleMove;

    Disambiguate(boards[yyboardindex], PosFlags(yyboardindex), &cl);

    currentMoveString[0] = cl.ff + AAA;
    currentMoveString[1] = cl.rf + ONE;
    currentMoveString[2] = cl.ft + AAA;
    currentMoveString[3] = cl.rt + ONE;
    currentMoveString[4] = cl.promoChar;
    currentMoveString[5] = NULLCHAR;

    return (int) cl.kind;
}

[a-l][xX:]?[a-l][0-9]((=?\(?[A-Z]\)?)|ep|"e.p."|[=+])? {
    /*
     * unambiguously abbreviated Pawn capture, possibly with promotion
     */
    int skip = 0;
    ChessMove result; char c;

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

    /* [HGM] do not allow values beyond board size */
    if(currentMoveString[0] - AAA >= BOARD_RGHT   ||
       currentMoveString[3] - ONE >= BOARD_HEIGHT ||
       currentMoveString[3] - ONE <  0            ||
       currentMoveString[2] - AAA >= BOARD_RGHT   ||
       currentMoveString[0] - AAA <  BOARD_LEFT   ||
       currentMoveString[2] - AAA <  BOARD_LEFT     )
      return ImpossibleMove;

    if (gameInfo.variant == VariantXiangqi && /* [HGM] In Xiangqi rank stays same */
         currentMoveString[0] != currentMoveString[2] ) {
        currentMoveString[1] = yytext[2+skip];
    } else 
    if (WhiteOnMove(yyboardindex)) {
        if (yytext[2+skip] == ONE) return (int) ImpossibleMove;
	currentMoveString[1] = yytext[2+skip] - 1;
	if(boards[yyboardindex][currentMoveString[1]-ONE][currentMoveString[0]-AAA] != WhitePawn) 
		return ImpossibleMove;
    } else {
        currentMoveString[1] = currentMoveString[3] + 1;
        if (currentMoveString[3] == ONE+BOARD_HEIGHT-1) return (int) ImpossibleMove;
	if(boards[yyboardindex][currentMoveString[1]-ONE][currentMoveString[0]-AAA] != BlackPawn) 
		return ImpossibleMove;
    }
    if (yyleng-skip > 3) {
	if (yytext[yyleng-1] == ')')
          c = currentMoveString[4] = ToLower(yytext[yyleng-2]);
	else
          c = currentMoveString[4] = ToLower(yytext[yyleng-1]);
	currentMoveString[5] = NULLCHAR;
        if(c == '+' && gameInfo.variant != VariantShogi) c = currentMoveString[4] = NULLCHAR; // + means check outside Shogi
    } else {
	currentMoveString[4] = NULLCHAR;
    }

    result = LegalityTest(boards[yyboardindex],
			  PosFlags(yyboardindex)&~F_MANDATORY_CAPTURE, // [HGM] losers: might think we can e.p.!
                          currentMoveString[1] - ONE,
                          currentMoveString[0] - AAA,
                          currentMoveString[3] - ONE,
                          currentMoveString[2] - AAA,
			  currentMoveString[4]);

    if (currentMoveString[4] == NULLCHAR) {
      if(result == WhitePromotion  || result == BlackPromotion) {
        currentMoveString[4] = PieceToChar(BlackQueen);
	// [HGM] shatranj: take care of variants without Queen
	if(gameInfo.variant == VariantShatranj || gameInfo.variant == VariantCourier || gameInfo.variant == VariantMakruk)
            currentMoveString[4] = PieceToChar(BlackFerz);
	if(gameInfo.variant == VariantGreat)
            currentMoveString[4] = PieceToChar(BlackMan);
	if(gameInfo.variant == VariantShogi)
            currentMoveString[4] = '+';
      } else if(result == WhiteNonPromotion  || result == BlackNonPromotion)
            currentMoveString[4] = '=';
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
			  PosFlags(yyboardindex)&~F_MANDATORY_CAPTURE, // [HGM] losers: might think we can e.p.!
                          currentMoveString[1] - ONE,
                          currentMoveString[0] - AAA,
                          currentMoveString[3] - ONE,
                          currentMoveString[2] - AAA,
			  currentMoveString[4]);

    if (result == WhiteCapturesEnPassant || result == BlackCapturesEnPassant)
      return (int) result;
    else { // [HGM] all very nice, but this messed up the input move that we might want to accept with legality testing off...
      if (WhiteOnMove(yyboardindex)) // undo the damage
	     currentMoveString[1]--,  currentMoveString[3]--;
	else currentMoveString[1]++,  currentMoveString[3]++;
      return (int) IllegalMove;
    }
}

"+"?[A-Z][xX:-]?[a-l][0-9](([=/]?\(?[A-Z]\)?)|[=+])?  {
    /*
     * piece move, possibly ambiguous
     */
    DisambiguateClosure cl;
    int skip = 0, skip2 = 0, promoted = 0;

    if (yyskipmoves) return (int) AmbiguousMove; /* not disambiguated */

    if(yytext[0] == '+') promoted = skip = skip2 = 1;

    /* remove the [xX:-] */
    if ((yytext[1+skip] == 'x') || (yytext[1+skip] == 'X')
        || (yytext[1+skip] == ':') || (yytext[1+skip] == '-')) skip++;

    if (WhiteOnMove(yyboardindex)) {
        cl.pieceIn = CharToPiece(ToUpper(yytext[skip2]));
    } else {
        cl.pieceIn = CharToPiece(ToLower(yytext[skip2]));
    }
    if(promoted) cl.pieceIn = (ChessSquare) (PROMOTED cl.pieceIn);

    cl.rfIn = -1;
    cl.ffIn = -1;
    cl.rtIn = yytext[2+skip] - ONE;
    cl.ftIn = yytext[1+skip] - AAA;
    cl.promoCharIn = NULLCHAR;

    if(yyleng-skip > 3) /* [HGM] in some variants pieces promote */
        cl.promoCharIn = yytext[yyleng-1-(yytext[yyleng-1]==')')];
    if(cl.promoCharIn == '+' && gameInfo.variant != VariantShogi) cl.promoCharIn = NULLCHAR; // + means check outside Shogi

    if (appData.debugMode) {
        fprintf(debugFP, "Parser Qa1: yyleng=%d,  %d(%d,%d)-(%d,%d) = %d (%c)\n",
        yyleng,
        cl.pieceIn,cl.ffIn,cl.rfIn,cl.ftIn,cl.rtIn,cl.promoCharIn,cl.promoCharIn?cl.promoCharIn:' ');
    }

    /* [HGM] but do not allow values beyond board size */
    if(cl.rtIn >= BOARD_HEIGHT ||
       cl.rtIn <  0            ||
       cl.ftIn >= BOARD_RGHT   ||
       cl.ftIn <  BOARD_LEFT     )
      return ImpossibleMove;

    Disambiguate(boards[yyboardindex], PosFlags(yyboardindex), &cl);

    currentMoveString[0] = cl.ff + AAA;
    currentMoveString[1] = cl.rf + ONE;
    currentMoveString[2] = cl.ft + AAA;
    currentMoveString[3] = cl.rt + ONE;
    currentMoveString[4] = cl.promoChar;
    currentMoveString[5] = NULLCHAR;

    return (int) cl.kind;
}

"+"?[A-Z][a-l0-9][xX:-]?[a-l][0-9](([=/]?\(?[A-Z]\)?)|[=+])?   {
    /*
     * piece move with rank or file disambiguator
     */
    DisambiguateClosure cl;
    int skip = 0, skip2 = 0; int promoted=0;

    if (yyskipmoves) return (int) AmbiguousMove; /* not disambiguated */

    if(yytext[0]=='+') promoted = skip = skip2 = 1;

    /* remove the [xX:-] */
    if ((yytext[2+skip] == 'x') || (yytext[2+skip] == 'X')
        || (yytext[2+skip] == ':') || (yytext[2+skip] == '-')) skip++;

    if (WhiteOnMove(yyboardindex)) {
        cl.pieceIn = CharToPiece(ToUpper(yytext[skip2]));
    } else {
        cl.pieceIn = CharToPiece(ToLower(yytext[skip2]));
    }
    if(promoted) cl.pieceIn = (ChessSquare) (PROMOTED cl.pieceIn);

    if (isalpha(yytext[1+skip2])) {
	cl.rfIn = -1;
        cl.ffIn = yytext[1+skip2] - AAA;
       
        if(cl.ffIn >= BOARD_RGHT ||
           cl.ffIn <  BOARD_LEFT   ) return 0;
    } else {
        cl.rfIn = yytext[1+skip2] - ONE;
	cl.ffIn = -1;
        if(cl.rfIn >= BOARD_HEIGHT ||
           cl.rfIn <  0) return 0;
    }
    cl.rtIn = yytext[3+skip] - ONE;
    cl.ftIn = yytext[2+skip] - AAA;
    cl.promoCharIn = NULLCHAR;

    if(yyleng-skip > 4) /* [HGM] in some variants pieces promote */
        cl.promoCharIn = yytext[yyleng-1-(yytext[yyleng-1]==')')];
    if(cl.promoCharIn == '+' && gameInfo.variant != VariantShogi) cl.promoCharIn = NULLCHAR; // + means check outside Shogi

    /* [HGM] do not allow values beyond board size */
    if(cl.rtIn >= BOARD_HEIGHT ||
       cl.rtIn <  0            ||
       cl.ftIn >= BOARD_RGHT   ||
       cl.ftIn <  BOARD_LEFT     )
      return ImpossibleMove;

    Disambiguate(boards[yyboardindex], PosFlags(yyboardindex), &cl);

    currentMoveString[0] = cl.ff + AAA;
    currentMoveString[1] = cl.rf + ONE;
    currentMoveString[2] = cl.ft + AAA;
    currentMoveString[3] = cl.rt + ONE;
    currentMoveString[4] = cl.promoChar;
    currentMoveString[5] = NULLCHAR;

    return (int) cl.kind;
}

000|0-0-0|ooo|OOO|o-o-o|O-O-O	{
    int rf, ff, rt, ft;

    if (yyskipmoves) return (int) AmbiguousMove; /* not disambiguated */

    /* [HGM] all squares referenced to board edges in stead of absolute */
    if (WhiteOnMove(yyboardindex)) {
        if (boards[yyboardindex][0][(BOARD_WIDTH-1)>>1] == WhiteKing) {
	    /* ICS wild castling */
	    rf = 0;
            ff = (BOARD_WIDTH-1)>>1;
	    rt = 0;
            ft = BOARD_RGHT-3;
	} else {
	    rf = 0;
            ff = BOARD_WIDTH>>1;
	    rt = 0;
            ft = BOARD_LEFT+2;
	}
    } else{ 
        if (boards[yyboardindex][BOARD_HEIGHT-1][(BOARD_WIDTH-1)>>1] == BlackKing) {
	    /* ICS wild castling */
            rf = BOARD_HEIGHT-1;
            ff = (BOARD_WIDTH-1)>>1;
            rt = BOARD_HEIGHT-1;
            ft = BOARD_RGHT-3;
	} else {
            rf = BOARD_HEIGHT-1;
            ff = BOARD_WIDTH>>1;
            rt = BOARD_HEIGHT-1;
            ft = BOARD_LEFT+2;
	}
    }
    if(PosFlags(0) & F_FRC_TYPE_CASTLING) {

        if (WhiteOnMove(yyboardindex)) {
            ff = initialRights[2];
            ft = initialRights[1];
        } else {
            ff = initialRights[5];
            ft = initialRights[4];
        }
        if (appData.debugMode) 
        {
          fprintf(debugFP, "Parser FRC long %d %d\n", ff, ft);
        };
        if(ff == NoRights || ft == NoRights) return ImpossibleMove;
    }
    sprintf(currentMoveString, "%c%c%c%c",ff+AAA,rf+ONE,ft+AAA,rt+ONE);
    if (appData.debugMode) {
        fprintf(debugFP, "long castling %d %d\n", ff, ft);
    }
    return (int) LegalityTest(boards[yyboardindex],
			      PosFlags(yyboardindex)&~F_MANDATORY_CAPTURE, // [HGM] losers: e.p.!
			      rf, ff, rt, ft, NULLCHAR);
}

00|0-0|oo|OO|o-o|O-O	{
    int rf, ff, rt, ft;

    if (yyskipmoves) return (int) AmbiguousMove; /* not disambiguated */

    if (WhiteOnMove(yyboardindex)) {
        if (boards[yyboardindex][0][(BOARD_WIDTH-1)>>1] == WhiteKing) {
	    /* ICS wild castling */
	    rf = 0;
            ff = (BOARD_WIDTH-1)>>1;
	    rt = 0;
            ft = BOARD_LEFT+1;
	} else {
	    rf = 0;
            ff = BOARD_WIDTH>>1;
	    rt = 0;
            ft = BOARD_RGHT-2;
	}
    } else {
        if (boards[yyboardindex][BOARD_HEIGHT-1][(BOARD_WIDTH-1)>>1] == BlackKing) {
	    /* ICS wild castling */
            rf = BOARD_HEIGHT-1;
            ff = (BOARD_WIDTH-1)>>1;
            rt = BOARD_HEIGHT-1;
            ft = BOARD_LEFT+1;
	} else {
            rf = BOARD_HEIGHT-1;
            ff = BOARD_WIDTH>>1;
            rt = BOARD_HEIGHT-1;
            ft = BOARD_RGHT-2;
	}
    }
    if(PosFlags(0) & F_FRC_TYPE_CASTLING) {
        if (WhiteOnMove(yyboardindex)) {
            ff = initialRights[2];
            ft = initialRights[0];
        } else {
            ff = initialRights[5];
            ft = initialRights[3];
        }
    if (appData.debugMode) {
        fprintf(debugFP, "Parser FRC short %d %d\n", ff, ft);
    }
        if(ff == NoRights || ft == NoRights) return ImpossibleMove;
    }
    sprintf(currentMoveString, "%c%c%c%c",ff+AAA,rf+ONE,ft+AAA,rt+ONE);
    if (appData.debugMode) {
        fprintf(debugFP, "short castling %d %d\n", ff, ft);
    }

    return (int) LegalityTest(boards[yyboardindex],
			      PosFlags(yyboardindex)&~F_MANDATORY_CAPTURE, // [HGM] losers: e.p.!
			      rf, ff, rt, ft, NULLCHAR);
}

[A-Za-z][@*][a-l][0-9] {

    if (yyskipmoves) return (int) AmbiguousMove; /* not disambiguated */

    /* Bughouse piece drop. */
    currentMoveString[1] = '@';
    currentMoveString[2] = yytext[2];
    currentMoveString[3] = yytext[3];
    currentMoveString[4] = NULLCHAR;

    if (appData.debugMode) {
        fprintf(debugFP, "Drop: %s\n", currentMoveString);
    }
    /* [HGM] do not allow values beyond board size */
    if(currentMoveString[3] - ONE >= BOARD_HEIGHT ||
       currentMoveString[2] - AAA >= BOARD_WIDTH     )
      return ImpossibleMove;

    if (WhiteOnMove(yyboardindex)) {
	currentMoveString[0] = ToUpper(yytext[0]);
    } else {
	currentMoveString[0] = ToLower(yytext[0]);
    }
    if(CharToPiece(currentMoveString[0]) == EmptySquare) return ImpossibleMove; // Unknown piece;

    return LegalityTest(boards[yyboardindex], PosFlags(yyboardindex), DROP_RANK, // [HGM] does drops now too
                        CharToPiece(currentMoveString[0]), currentMoveString[3] - ONE, currentMoveString[2] - AAA, NULLCHAR);
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

(([Ww](hite)?)|([Bb](lack)?))" "(([Mm]ates)|([Ww][io]n(s)?)) { 
    return (int) (ToUpper(yytext[0]) == 'W' ? WhiteWins : BlackWins);
}

(([Ww](hite)?)|([Bb](lack)?))" "(([Mm]ated)|([Ll]os[tes]+)) { 
    return (int) (ToUpper(yytext[0]) == 'W' ? BlackWins : WhiteWins);
}

("{"[^\}]*"}"[ \n])?(1-0|"1 - 0"|"1/0"|"1 / 0"|"1:0"|"1 : 0")(" (".*")"|" {".*"}")? { 
    return (int) WhiteWins;
}

("{"[^\}]*"}"[ \n])?(0-1|"0 - 1"|"0/1"|"0 / 1"|"0:1"|"0 : 1")(" (".*")"|" {".*"}")? { 
    return (int) BlackWins;
}

("{"[^\}]*"}"[ \n])?("1/2"|"1 / 2")(" "?[-:]" "?("1/2"|"1 / 2"))?(" (".*")"|" {".*"}")? {
    return (int) GameIsDrawn;
}

("{"[^\}]*"}"[ \n])?"*"(" (".*")"|" {".*"}")? {
    return (int) GameUnfinished;
}

[1-9][0-9]*/"."?[ \t\n]*[a-lnprqoA-Z+]    {
    /* move numbers */
    if ((yyleng == 1) && (yytext[0] == '1'))
      return (int) MoveNumberOne;
    else return (int) Nothing; // [HGM] make sure something is returned, for gathering parsed text
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

\(  {                               /* Opening parentheses */
    return (int) Open; 
}

\)   { 				      /* closing parentheses */
    return (int) Close; 
}       

^[-a-zA-Z0-9]+:" ".*(\n[ \t]+.*)*  {
    return (int) Nothing;                 /* Skip mail headers */
}

[a-zA-Z0-9'-]+			{
    return (int) Nothing;                 /* Skip random words */
}

.|\n				{
    return (int) Nothing;                 /* Skip everything else */
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
    if(appData.debugMode) fprintf(debugFP, "PARSER BUG: unmatched character '%c' (0%o)\n",
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
	  if(appData.debugMode) fprintf(debugFP, "PARSER BUG: unput buffer overflow '%c' (0%o)\n",
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
    int pos = yy_c_buf_p - YY_CURRENT_BUFFER->yy_ch_buf;

    return(ftell(YY_CURRENT_BUFFER->yy_input_file) -
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
ChessMove yylexstr(boardIndex, s, text, len)
     int boardIndex, len;
     char *s, *text;
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

    ret = (ChessMove) Myylex();
     strncpy(text, yy_text, len-1); // [HGM] vari: yy_text is not available to caller after buffer switch ?!?
     text[len-1] = NULLCHAR;

#ifdef FLEX_SCANNER
    if (oldBuffer != NULL) 
      yy_switch_to_buffer(oldBuffer);
    yy_delete_buffer(buffer);
#endif /*FLEX_SCANNER*/
    StringToLex = oldStringToLex;

    return ret;
}

int Myylex()
{   // [HGM] wrapper for yylex, which treats nesting of parentheses
    int symbol, nestingLevel = 0, i=0;
    char *p;
    static char buf[256*MSG_SIZ];
    buf[0] = NULLCHAR;
    do { // eat away anything not at level 0
        symbol = yylex();
        if(symbol == Open) nestingLevel++;
        if(nestingLevel) { // save all parsed text between (and including) the ()
            for(p=yytext; *p && i<256*MSG_SIZ-2;) buf[i++] = *p++;
            buf[i] = NULLCHAR;
        }
        if(symbol == 0) break; // ran into EOF
        if(symbol == Close) symbol = Comment, nestingLevel--;
    } while(nestingLevel || symbol == Nothing);
    yy_text = buf[0] ? buf : (char*)yytext;
    return symbol;
}
