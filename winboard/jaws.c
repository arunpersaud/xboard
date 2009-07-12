/*
 * JAWS.c -- Code for Windows front end to XBoard to use it with JAWS
 * $Id: winboard.c,v 2.3 2003/11/25 05:25:20 mann Exp $
 *
 * Copyright 1991 by Digital Equipment Corporation, Maynard,
 * Massachusetts.  Enhancements Copyright
 * 1992-2001,2002,2003,2004,2005,2006,2007,2008,2009 Free Software
 * Foundation, Inc.
 *
 * XBoard borrows its colors and the bitmaps.xchess bitmap set from XChess,
 * which was written and is copyrighted by Wayne Christopher.
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

// This file collects all patches for the JAWS version, so they can all be included in winboard.c
// in one big swoop. At the bottom of this file you can read instructions for how to patch
// WinBoard to work with JAWS with the aid of this file. Note that the code in this file
// is for WinBoard 4.3 and later; for older WB versions you would have to throw out the
// piece names for all pieces from Guard to Unicorn, #define ONE as '1', AAA as 'a',
// BOARD_LEFT as 0, BOARD_RGHT and BOARD_HEIGHT as 8, and set holdingssizes to 0.
// You will need to build with jaws.rc in stead of winboard.rc.

// from common.h, but 'extern' added to it, so the actual declaraton can remain in backend.c

extern long whiteTimeRemaining, blackTimeRemaining, timeControl, timeIncrement;

#if 0
// from moves.h, but no longer needed, as the new routines are all moved to winboard.c

extern char* PieceToName P((ChessSquare p, int i));
extern char* SquareToChar P((int Xpos)); 
extern char* SquareToNum P((int Ypos));
extern int CoordToNum P((char c));

#endif

// from moves.c, added WinBoard_F piece types and ranks / files

char *squareToChar[] = { "ay", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l" };

char *squareToNum[] = {"naught", "1", "2", "3", "4", "5", "6", "7", "8", "9" };

char *ordinals[] = {"zeroth", "first", "second", "third", "fourth", "fifth", "sixth", "seventh", "eighth", "nineth"};

char *pieceToName[] = {
		"White Pawn", "White Knight", "White Bishop", "White Rook", "White Queen", 
		"White Guard", "White Elephant", "White Arch Bishop", "White Chancellor",
		"White General", "White Man", "White Cannon", "White Night Rider",
		"White Crowned Bishop", "White Crowned Rook", "White Grass Hopper", "White Veteran",
		"White Falcon", "White Amazon", "White Snake", "White Unicorn",
		"White King",
		"Black Pawn", "Black Knight", "Black Bishop", "Black Rook", "Black Queen",
		"Black Guard", "Black Elephant", "Black Arch Bishop", "Black Chancellor",
		"Black General", "Black Man", "Black Cannon", "Black Night Rider",
		"Black Crowned Bishop", "Black Crowned Rook", "Black Grass Hopper", "Black Veteran",
		"Black Falcon", "Black Amazon", "Black Snake", "Black Unicorn",
		"Black King",
		"Empty"
	};

int CoordToNum(c)
		char c;
{
	if(isdigit(c)) return c - ONE;
	if(c >= 'a') return c - AAA;
	return 0;
}

char* PieceToName(p, i)
	ChessSquare p;
	int i;
{
	if(i) return pieceToName[(int) p];
		return pieceToName[(int) p]+6;
}

char* SquareToChar(x)
			int x;
{
		return squareToChar[x - BOARD_LEFT];
}

char* SquareToNum(y)
			int y;
{
		return squareToNum[y + (gameInfo.boardHeight < 10)];
}


// from winboard.c: all new routines

#include "jfwapi.h"
#include "jaws.h"

typedef JFWAPI BOOL (WINAPI *PSAYSTRING)(LPCTSTR lpszStrinToSpeak, BOOL bInterrupt);

PSAYSTRING RealSayString;

VOID SayString(char *mess, BOOL flag)
{ // for debug file
	char buf[MSG_SIZ], *p;
	if(appData.debugMode) fprintf(debugFP, "SAY '%s'\n", mess);
	strcpy(buf, mess);
	if(p = StrCaseStr(buf, "Xboard adjudication:")) {
		int i;
		for(i=19; i>1; i--) p[i] = p[i-1];
		p[1] = ' ';
	}
	RealSayString(buf, flag);
}

//static int fromX = 0, fromY = 0;
static int oldFromX, oldFromY;
static int timeflag;
static int suppressClocks = 0;
static int suppressOneKey = 0;

BOOL
InitJAWS()
{	// to be called at beginning of WinMain, after InitApplication and InitInstance
	HINSTANCE hApi = LoadLibrary("jfwapi32.dll");
	if(!hApi) {
		DisplayInformation("Missing jfwapi32.dll");	   
		return (FALSE);
	}

	RealSayString = (PSAYSTRING)GetProcAddress(hApi, "JFWSayString");
	if(!RealSayString) {
		DisplayInformation("SayString returned a null pointer");
		return (FALSE);
	}

	{
		// [HGM] kludge to reduce need for modification of winboard.c: mak tinyLayout menu identical
		// to standard layout, so that code for switching between them does not have to be deleted
		HMENU hmenu = GetMenu(hwndMain);
		int i;

		menuBarText[0][5] = "&JAWS";
		for(i=0; i<7; i++) menuBarText[1][i] = menuBarText[0][i];
		for (i=0; menuBarText[tinyLayout][i]; i++) {
			ModifyMenu(hmenu, i, MF_STRING|MF_BYPOSITION|MF_POPUP, 
					(UINT)GetSubMenu(hmenu, i), menuBarText[tinyLayout][i]);
		}
		DrawMenuBar(hwndMain);
	}

	/* initialize cursor position */
	fromX = fromY = 0;
	SetHighlights(fromX, fromY, -1, -1);
	DrawPosition(FALSE, NULL);
	oldFromX = oldFromY = -1;

	if(hwndConsole) SetFocus(hwndConsole);
	return TRUE;
}

VOID
KeyboardEvent(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	ChessSquare currentPiece;
	char *piece, *xchar, *ynum ;
	int n;

	if(fromX == -1 || fromY == -1) {
		fromX = BOARD_LEFT; fromY = 0;
        }
	switch(wParam) {
	case VK_LEFT:
		if(fromX == BOARD_RGHT+1) fromX -= 2; else
		if(fromX == BOARD_LEFT) { if(fromY >= BOARD_HEIGHT - gameInfo.holdingsSize) fromX -= 2; } else
		if(fromX > BOARD_LEFT) fromX--;
		break;
	case VK_RIGHT:
		if(fromX == BOARD_LEFT-2) fromX += 2; else
		if(fromX == BOARD_RGHT-1) { if(fromY < gameInfo.holdingsSize) fromX += 2; } else
		if(fromX < BOARD_RGHT-1) fromX++;
		break;
	case VK_UP:
		if(fromX == BOARD_RGHT+1) { if(fromY < gameInfo.holdingsSize - 1) fromY++; } else
		if(fromY < BOARD_HEIGHT-1) fromY++;
		break;
	case VK_DOWN:
		if(fromX == BOARD_LEFT-2) { if(fromY > BOARD_HEIGHT - gameInfo.holdingsSize) fromY--; } else
		if(fromY > 0) fromY--;
		break;
	}
	SetHighlights(fromX, fromY, -1, -1);
	DrawPosition(FALSE, NULL);
	currentPiece = boards[currentMove][fromY][fromX];
	piece = PieceToName(currentPiece,1);
	if(currentPiece != EmptySquare) MessageBeep(currentPiece < (int)BlackPawn ? MB_OK : MB_ICONEXCLAMATION);
	if(fromX == BOARD_LEFT - 2) {
		SayString("black holdings", FALSE);
		if(currentPiece != EmptySquare) {
			char buf[MSG_SIZ];
			n = boards[currentMove][fromY][1];
			sprintf(buf, "%d %s%s", n, piece+6, n == 1 ? "" : "s");
			SayString(buf, TRUE);
		}
	} else
	if(fromX == BOARD_RGHT + 1) {
		SayString("white holdings", FALSE);
		if(currentPiece != EmptySquare) {
			char buf[MSG_SIZ];
			n = boards[currentMove][fromY][BOARD_WIDTH-2];
			sprintf(buf, "%d %s%s", n, piece+6, n == 1 ? "" : "s");
			SayString(buf, TRUE);
		}
	} else
	if(fromX >= BOARD_LEFT && fromX < BOARD_RGHT) {
		char buf[MSG_SIZ];
		xchar = SquareToChar(fromX);
		ynum = SquareToNum(fromY);
		if(currentPiece != EmptySquare) {
//			SayString(piece[0] == 'W' ? "white" : "black", TRUE);
			sprintf(buf, "%s %s %s", piece, xchar, ynum);
		} else sprintf(buf, "%s %s", xchar, ynum);
		SayString(buf, TRUE);
	}
	return;
}

extern char castlingRights[MAX_MOVES][BOARD_SIZE];
int PosFlags(int nr);

typedef struct {
    int rf, ff, rt, ft;
    int onlyCaptures;
    int count;
} ReadClosure;

extern void ReadCallback P((Board board, int flags, ChessMove kind,
				int rf, int ff, int rt, int ft,
				VOIDSTAR closure));

void ReadCallback(board, flags, kind, rf, ff, rt, ft, closure)
     Board board;
     int flags;
     ChessMove kind;
     int rf, ff, rt, ft;
     VOIDSTAR closure;
{
    register ReadClosure *cl = (ReadClosure *) closure;
    ChessSquare possiblepiece;
    char *piece, *xchar, *ynum ;

//if(appData.debugMode) fprintf(debugFP, "%c%c%c%c\n", ff+AAA, rf+ONE, ft+AAA, rt+ONE);
    if(cl->ff == ff && cl->rf == rf) {
	possiblepiece = board[rt][ft];
	if(possiblepiece != EmptySquare) {
		piece = PieceToName(possiblepiece,1);
		xchar = SquareToChar(ft);
		ynum  = SquareToNum(rt);
		SayString(xchar , FALSE);
		SayString(ynum, FALSE);
		SayString(piece, FALSE);
		cl->count++;
	}
    }
    if(cl->ft == ft && cl->rt == rt) {
	possiblepiece = board[rf][ff];
		piece = PieceToName(possiblepiece,1);
		xchar = SquareToChar(ff);
		ynum  = SquareToNum(rf);
		SayString(xchar , FALSE);
		SayString(ynum, FALSE);
		SayString(piece, FALSE);
		cl->count++;
    }
}

VOID
PossibleAttackMove()
{
	ReadClosure cl;
	ChessSquare piece, victim;
	int removedSelectedPiece = 0, swapColor;

//if(appData.debugMode) fprintf(debugFP, "PossibleAttackMove %d %d %d %d\n", fromX, fromY, oldFromX, oldFromY);
	if(fromY < 0 || fromY >= BOARD_HEIGHT) return;
	if(fromX < BOARD_LEFT || fromX >= BOARD_RGHT) { SayString("holdings",FALSE); return; }

	piece = boards[currentMove][fromY][fromX];
	if(piece == EmptySquare) { // if square is empty, try to substitute selected piece
	    if(oldFromX >= 0 && oldFromY >= 0) {
		piece = boards[currentMove][oldFromY][oldFromX];
		boards[currentMove][oldFromY][oldFromX] = EmptySquare;
		removedSelectedPiece = 1;
		SayString("Your", FALSE);
		SayString(PieceToName(piece, 0), FALSE);
		SayString("would have", FALSE);
	    } else { SayString("You must select a piece first", FALSE); return; }
	}

	victim = boards[currentMove][fromY][fromX];
	boards[currentMove][fromY][fromX] = piece; // make sure piece is actally there
	SayString("possible captures from here are", FALSE);

	swapColor = piece <  (int)BlackPawn && !WhiteOnMove(currentMove) ||
		    piece >= (int)BlackPawn &&  WhiteOnMove(currentMove);
	cl.count = 0; cl.rf = fromY; cl.ff = fromX; cl.rt = cl.ft = -1;
	GenLegal(boards[currentMove], PosFlags(currentMove + swapColor), EP_NONE, 
						castlingRights[currentMove], ReadCallback, (VOIDSTAR) &cl);
	if(cl.count == 0) SayString("None", FALSE);
	boards[currentMove][fromY][fromX] = victim; // repair

	if( removedSelectedPiece ) boards[currentMove][oldFromY][oldFromX] = piece;
}


VOID
PossibleAttacked()
{
	ReadClosure cl;
	ChessSquare piece = EmptySquare, victim;

	if(fromY < 0 || fromY >= BOARD_HEIGHT) return;
	if(fromX < BOARD_LEFT || fromX >= BOARD_RGHT) { SayString("holdings",FALSE); return; }

	if(oldFromX >= 0 && oldFromY >= 0) { // if piece is selected, remove it
		piece = boards[currentMove][oldFromY][oldFromX];
		boards[currentMove][oldFromY][oldFromX] = EmptySquare;
	}

	SayString("Pieces that can capture you are", FALSE);

	victim = boards[currentMove][fromY][fromX]; // put dummy piece on target square, to activate Pawn captures
	boards[currentMove][fromY][fromX] = WhiteOnMove(currentMove) ? WhiteQueen : BlackQueen;
	cl.count = 0; cl.rt = fromY; cl.ft = fromX; cl.rf = cl.ff = -1;
	GenLegal(boards[currentMove], PosFlags(currentMove+1), EP_NONE, 
						castlingRights[currentMove], ReadCallback, (VOIDSTAR) &cl);
	if(cl.count == 0) SayString("None", FALSE);

	SayString("You are defended by", FALSE);

	boards[currentMove][fromY][fromX] = WhiteOnMove(currentMove) ? BlackQueen : WhiteQueen;
	cl.count = 0; cl.rt = fromY; cl.ft = fromX; cl.rf = cl.ff = -1;
	GenLegal(boards[currentMove], PosFlags(currentMove), EP_NONE, 
						castlingRights[currentMove], ReadCallback, (VOIDSTAR) &cl);
	if(cl.count == 0) SayString("None", FALSE);
	boards[currentMove][fromY][fromX] = victim; // put back original occupant

	if(oldFromX >= 0 && oldFromY >= 0) { // put back possibl selected piece
		boards[currentMove][oldFromY][oldFromX] = piece;
	}
}

VOID
ReadRow()
{
	ChessSquare currentpiece; 
	char *piece, *xchar, *ynum ;
	int xPos, count=0;
	ynum = SquareToNum(fromY);
	
	if(fromY < 0) return;

	for (xPos=BOARD_LEFT; xPos<BOARD_RGHT; xPos++) {
		currentpiece = boards[currentMove][fromY][xPos];	
		if(currentpiece != EmptySquare) {
			piece = PieceToName(currentpiece,1);
			xchar = SquareToChar(xPos);
			SayString(xchar , FALSE);
			SayString(ynum, FALSE);
			SayString(piece, FALSE);
			count++;
		}
	}
	if(count == 0) {
		SayString("rank", FALSE);
		SayString(ynum, FALSE);
		SayString("empty", FALSE);
	}
}

VOID
ReadColumn()
{
	ChessSquare currentpiece; 
	char *piece, *xchar, *ynum ;
	int yPos, count=0;
	xchar = SquareToChar(fromX);
	
	if(fromX < 0) return;

	for (yPos=0; yPos<BOARD_HEIGHT; yPos++) {
		currentpiece = boards[currentMove][yPos][fromX];	
		if(currentpiece != EmptySquare) {
			piece = PieceToName(currentpiece,1);
			ynum = SquareToNum(yPos);
			SayString(xchar , FALSE);
			SayString(ynum, FALSE);
			SayString(piece, FALSE);
			count++;
		}
	}
	if(count == 0) {
		SayString(xchar, FALSE);
		SayString("file empty", FALSE);
	}
}

VOID
SayUpperDiagnols()
{
	ChessSquare currentpiece; 
	char *piece, *xchar, *ynum ;
	int yPos, xPos;
	
	if(fromX < 0 || fromY < 0) return;

	if(fromX < BOARD_RGHT-1 && fromY < BOARD_HEIGHT-1) {
		SayString("The diagnol squares to your upper right contain", FALSE);
		yPos = fromY+1;
		xPos = fromX+1;
		while(yPos<BOARD_HEIGHT && xPos<BOARD_RGHT) {
			currentpiece = boards[currentMove][yPos][xPos];	
			piece = PieceToName(currentpiece,1);
			xchar = SquareToChar(xPos);
			ynum = SquareToNum(yPos);
			SayString(xchar , FALSE);
			SayString(ynum, FALSE);
			SayString(piece, FALSE);
			yPos++;
			xPos++;
		}
	}
	else SayString("There is no squares to your upper right", FALSE);

	if(fromX > BOARD_LEFT && fromY < BOARD_HEIGHT-1) {
		SayString("The diagnol squares to your upper left contain", FALSE);
		yPos = fromY+1;
		xPos = fromX-1;
		while(yPos<BOARD_HEIGHT && xPos>=BOARD_LEFT) {
			currentpiece = boards[currentMove][yPos][xPos];	
			piece = PieceToName(currentpiece,1);
			xchar = SquareToChar(xPos);
			ynum = SquareToNum(yPos);
			SayString(xchar , FALSE);
			SayString(ynum, FALSE);
			SayString(piece, FALSE);
			yPos++;
			xPos--;
		}
	}
	else SayString("There is no squares to your upper left", FALSE);
}

VOID
SayLowerDiagnols()
{
	ChessSquare currentpiece; 
	char *piece, *xchar, *ynum ;
	int yPos, xPos;
	
	if(fromX < 0 || fromY < 0) return;

	if(fromX < BOARD_RGHT-1 && fromY > 0) {
		SayString("The diagnol squares to your lower right contain", FALSE);
		yPos = fromY-1;
		xPos = fromX+1;
		while(yPos>=0 && xPos<BOARD_RGHT) {
			currentpiece = boards[currentMove][yPos][xPos];	
			piece = PieceToName(currentpiece,1);
			xchar = SquareToChar(xPos);
			ynum = SquareToNum(yPos);
			SayString(xchar , FALSE);
			SayString(ynum, FALSE);
			SayString(piece, FALSE);
			yPos--;
			xPos++;
		}
	}
	else SayString("There is no squares to your lower right", FALSE);

	if(fromX > BOARD_LEFT && fromY > 0) {
		SayString("The diagnol squares to your lower left contain", FALSE);
		yPos = fromY-1;
		xPos = fromX-1;
		while(yPos>=0 && xPos>=BOARD_LEFT) {
			currentpiece = boards[currentMove][yPos][xPos];	
			piece = PieceToName(currentpiece,1);
			xchar = SquareToChar(xPos);
			ynum = SquareToNum(yPos);
			SayString(xchar , FALSE);
			SayString(ynum, FALSE);
			SayString(piece, FALSE);
			yPos--;
			xPos--;
		}
	}
	else SayString("There is no squares to your lower left", FALSE);
}

VOID
SayKnightMoves()
{
	ChessSquare currentpiece, oldpiece; 
	char *piece, *xchar, *ynum ;

	oldpiece = boards[currentMove][fromY][fromX];
	if(oldpiece == WhiteKnight || oldpiece == BlackKnight) 
		SayString("The possible squares a Knight could move to are", FALSE);
	else
		SayString("The squares a Knight could possibly attack from are", FALSE);

	if (fromY+2 < BOARD_HEIGHT && fromX-1 >= BOARD_LEFT) {
		currentpiece = boards[currentMove][fromY+2][fromX-1];
		if(((oldpiece == WhiteKnight) && (currentpiece > WhiteKing))
			|| ((oldpiece == BlackKnight) && (currentpiece < BlackPawn || currentpiece == EmptySquare))
			|| (oldpiece == EmptySquare) && (currentpiece == WhiteKnight || currentpiece == BlackKnight))
		{
			piece = PieceToName(currentpiece,1);
			xchar = SquareToChar(fromX-1);
			ynum = SquareToNum(fromY+2);
			SayString(xchar , FALSE);
			SayString(ynum, FALSE);
			SayString(piece, FALSE);
		}
	}

	if (fromY+2 < BOARD_HEIGHT && fromX+1 < BOARD_RGHT) {
		currentpiece = boards[currentMove][fromY+2][fromX+1];
		if(((oldpiece == WhiteKnight) && (currentpiece > WhiteKing))
			|| ((oldpiece == BlackKnight) && (currentpiece < BlackPawn || currentpiece == EmptySquare))
			|| (oldpiece == EmptySquare) && (currentpiece == WhiteKnight || currentpiece == BlackKnight))
		{
			piece = PieceToName(currentpiece,1);
			xchar = SquareToChar(fromX+1);
			ynum = SquareToNum(fromY+2);
			SayString(xchar , FALSE);
			SayString(ynum, FALSE);
			SayString(piece, FALSE);
		}
	}
	
	if (fromY+1 < BOARD_HEIGHT && fromX+2 < BOARD_RGHT) {
		currentpiece = boards[currentMove][fromY+1][fromX+2];
		if(((oldpiece == WhiteKnight) && (currentpiece > WhiteKing))
			|| ((oldpiece == BlackKnight) && (currentpiece < BlackPawn || currentpiece == EmptySquare))
			|| (oldpiece == EmptySquare) && (currentpiece == WhiteKnight || currentpiece == BlackKnight))
		{
			piece = PieceToName(currentpiece,1);
			xchar = SquareToChar(fromX+2);
			ynum = SquareToNum(fromY+1);
			SayString(xchar , FALSE);
			SayString(ynum, FALSE);
			SayString(piece, FALSE);
		}
	}
	
	if (fromY-1 >= 0 && fromX+2 < BOARD_RGHT) {
		currentpiece = boards[currentMove][fromY-1][fromX+2];
		if(((oldpiece == WhiteKnight) && (currentpiece > WhiteKing))
			|| ((oldpiece == BlackKnight) && (currentpiece < BlackPawn || currentpiece == EmptySquare))
			|| (oldpiece == EmptySquare) && (currentpiece == WhiteKnight || currentpiece == BlackKnight))
		{
			piece = PieceToName(currentpiece,1);
			xchar = SquareToChar(fromX+2);
			ynum = SquareToNum(fromY-1);
			SayString(xchar , FALSE);
			SayString(ynum, FALSE);
			SayString(piece, FALSE);
		}
	}
	
	if (fromY-2 >= 0 && fromX+1 < BOARD_RGHT) {
		currentpiece = boards[currentMove][fromY-2][fromX+1];
		if(((oldpiece == WhiteKnight) && (currentpiece > WhiteKing))
			|| ((oldpiece == BlackKnight) && (currentpiece < BlackPawn || currentpiece == EmptySquare))
			|| (oldpiece == EmptySquare) && (currentpiece == WhiteKnight || currentpiece == BlackKnight))
		{
			piece = PieceToName(currentpiece,1);
			xchar = SquareToChar(fromX+1);
			ynum = SquareToNum(fromY-2);
			SayString(xchar , FALSE);
			SayString(ynum, FALSE);
			SayString(piece, FALSE);
		}
	}
	
	if (fromY-2 >= 0 && fromX-1 >= BOARD_LEFT) {
		currentpiece = boards[currentMove][fromY-2][fromX-1];
		if(((oldpiece == WhiteKnight) && (currentpiece > WhiteKing))
			|| ((oldpiece == BlackKnight) && (currentpiece < BlackPawn || currentpiece == EmptySquare))
			|| (oldpiece == EmptySquare) && (currentpiece == WhiteKnight || currentpiece == BlackKnight))
		{
			piece = PieceToName(currentpiece,1);
			xchar = SquareToChar(fromX-1);
			ynum = SquareToNum(fromY-2);
			SayString(xchar , FALSE);
			SayString(ynum, FALSE);
			SayString(piece, FALSE);
		}
	}
	
	if (fromY-1 >= 0 && fromX-2 >= BOARD_LEFT) {
		currentpiece = boards[currentMove][fromY-1][fromX-2];
		if(((oldpiece == WhiteKnight) && (currentpiece > WhiteKing))
			|| ((oldpiece == BlackKnight) && (currentpiece < BlackPawn || currentpiece == EmptySquare))
			|| (oldpiece == EmptySquare) && (currentpiece == WhiteKnight || currentpiece == BlackKnight))
		{
			piece = PieceToName(currentpiece,1);
			xchar = SquareToChar(fromX-2);
			ynum = SquareToNum(fromY-1);
			SayString(xchar , FALSE);
			SayString(ynum, FALSE);
			SayString(piece, FALSE);
		}
	}
	
	if (fromY+1 < BOARD_HEIGHT && fromX-2 >= BOARD_LEFT) {
		currentpiece = boards[currentMove][fromY+1][fromX-2];
		if(((oldpiece == WhiteKnight) && (currentpiece > WhiteKing))
			|| ((oldpiece == BlackKnight) && (currentpiece < BlackPawn || currentpiece == EmptySquare))
			|| (oldpiece == EmptySquare) && (currentpiece == WhiteKnight || currentpiece == BlackKnight))
		{
			piece = PieceToName(currentpiece,1);
			xchar = SquareToChar(fromX-2);
			ynum = SquareToNum(fromY+1);
			SayString(xchar , FALSE);
			SayString(ynum, FALSE);
			SayString(piece, FALSE);
		}
	}
}

VOID
SayPieces(ChessSquare p)
{
	ChessSquare currentpiece;  
	char *piece, *xchar, *ynum ;
	int yPos, xPos, count = 0;
	char buf[50];

	if(p == WhitePlay)   SayString("White pieces", FALSE); else
	if(p == BlackPlay)   SayString("Black pieces", FALSE); else
	if(p == EmptySquare) SayString("Pieces", FALSE); else {
		sprintf(buf, "%ss", PieceToName(p,1));
		SayString(buf, FALSE);
	}
	SayString("are located", FALSE);
	for(yPos=0; yPos<BOARD_HEIGHT; yPos++) {
		for(xPos=BOARD_LEFT; xPos<BOARD_RGHT; xPos++) {
			currentpiece = boards[currentMove][yPos][xPos];	
			if(p == BlackPlay && currentpiece >= BlackPawn && currentpiece <= BlackKing ||
			   p == WhitePlay && currentpiece >= WhitePawn && currentpiece <= WhiteKing   )
				piece = PieceToName(currentpiece,0);
			else if(p == EmptySquare && currentpiece != EmptySquare)
				piece = PieceToName(currentpiece,1);
			else if(p == currentpiece)
				piece = NULL;
			else continue;
				
				if(count == 0) SayString("at", FALSE);
				xchar = SquareToChar(xPos);
				ynum = SquareToNum(yPos);
				SayString(xchar , FALSE);
				SayString(ynum, FALSE);
				if(piece) SayString(piece, FALSE);
				count++;
		}
	}
	if(count == 0) SayString("nowhere", FALSE);
}

VOID
SayCurrentPos()
{
	ChessSquare currentpiece;
	char *piece, *xchar, *ynum ;
	if(fromX <  BOARD_LEFT) { SayString("You strayed into the white holdings", FALSE); return; }
	if(fromX >= BOARD_RGHT) { SayString("You strayed into the black holdings", FALSE); return; }
	currentpiece = boards[currentMove][fromY][fromX];	
	piece = PieceToName(currentpiece,1);
	ynum = SquareToNum(fromY);
	xchar = SquareToChar(fromX);
	SayString("Your current position is", FALSE);
	SayString(xchar, FALSE);
	SayString(ynum, FALSE);
	SayString(piece, FALSE);
	if((fromX-BOARD_LEFT) ^ fromY)
		SayString("on a dark square",FALSE);
	else 
		SayString("on a light square",FALSE);

	PossibleAttacked();
	return;
}

VOID
SayAllBoard()
{
	int Xpos, Ypos;
	ChessSquare currentpiece;
	char *piece, *xchar, *ynum ;
	
	if(gameInfo.holdingsWidth) {
		int first = 0;
		for(Ypos=0; Ypos<gameInfo.holdingsSize; Ypos++) {
			int n = boards[currentMove][Ypos][BOARD_WIDTH-2];
			if(n) {  char buf[MSG_SIZ];
				if(!first++) SayString("white holds", FALSE);
				currentpiece = boards[currentMove][Ypos][BOARD_WIDTH-1];	
				piece = PieceToName(currentpiece,0);
				sprintf(buf, "%d %s%s", n, piece, (n==1 ? "" : "s") );
				SayString(buf, FALSE);
			}
		}
		first = 0;
		for(Ypos=BOARD_HEIGHT-1; Ypos>=BOARD_HEIGHT - gameInfo.holdingsSize; Ypos--) {
			int n = boards[currentMove][Ypos][1];
			if(n) {  char buf[MSG_SIZ];
				if(!first++) SayString("black holds", FALSE);
				currentpiece = boards[currentMove][Ypos][0];	
				piece = PieceToName(currentpiece,0);
				sprintf(buf, "%d %s%s", n, piece, (n==1 ? "" : "s") );
				SayString(buf, FALSE);
			}
		}
	}

	for(Ypos=BOARD_HEIGHT-1; Ypos>=0; Ypos--) {
		ynum = ordinals[Ypos + (gameInfo.boardHeight < 10)];
		SayString(ynum, FALSE);
		SayString("rank", FALSE);
		for(Xpos=BOARD_LEFT; Xpos<BOARD_RGHT; Xpos++) {
			currentpiece = boards[currentMove][Ypos][Xpos];	
			if(currentpiece != EmptySquare) {
				int count = 0, oldX = Xpos;
				char buf[50];
				piece = PieceToName(currentpiece,1);
				while(Xpos < BOARD_RGHT && boards[currentMove][Ypos][Xpos] == currentpiece)
					Xpos++, count++;
				if(count > 1) { 
					sprintf(buf, "%d %ss", count, piece);
				} else	sprintf(buf, "%s", piece);
				Xpos--;
				SayString(buf, FALSE);
			} else {
				int count = 0, oldX = Xpos;
				while(Xpos < BOARD_RGHT && boards[currentMove][Ypos][Xpos] == EmptySquare)
					Xpos++, count++;
				if(Xpos == BOARD_RGHT && oldX == BOARD_LEFT)
					SayString("all", FALSE);
				else{
				    if(count > 1) { 
					char buf[10];
					sprintf(buf, "%d", count);
					SayString(buf, FALSE);
				    }
				    Xpos--;
				}
				SayString("empty", FALSE);
			}
		}
	}
	
}

VOID
SayWhosTurn()
{
	if(gameMode == MachinePlaysBlack || gameMode == IcsPlayingBlack) {
		if(WhiteOnMove(currentMove))
			SayString("It is your turn", FALSE);
		else	SayString("It is your opponents turn", FALSE);
	} else if(gameMode == MachinePlaysWhite || gameMode == IcsPlayingWhite) {
		if(WhiteOnMove(currentMove))
			SayString("It is your opponents turn", FALSE);
		else	SayString("It is your turn", FALSE);
	} else {
		if(WhiteOnMove(currentMove)) 
			SayString("White is on move here", FALSE);
		else	SayString("Black is on move here", FALSE);
	}
}
	

VOID
SayMachineMove(int evenIfDuplicate)
{
	int len, xPos, yPos, moveNr, secondSpace = 0, castle = 0, n;
	ChessSquare currentpiece;
	char *piece, *xchar, *ynum, *p;
	char c, buf[MSG_SIZ], comment[MSG_SIZ];
	static char disambiguation[2];
	static int previousMove = 0;

	if(appData.debugMode) fprintf(debugFP, "Message = '%s'\n", messageText);
	if(gameMode == BeginningOfGame) return;
	if(messageText[0] == '[') return;
	comment[0]= 0;
	    if(isdigit(messageText[0])) { // message is move, possibly with thinking output
		int dotCount = 0, spaceCount = 0;
		sscanf(messageText, "%d", &moveNr);
		len = 0;
		// [HGM] show: better extraction of move
		while (messageText[len] != NULLCHAR) {
		    if(messageText[len] == '.' && spaceCount == 0) dotCount++;
		    if(messageText[len] == ' ') { if(++spaceCount == 2) secondSpace = len; }
		    if(messageText[len] == '{') { // we detected a comment
			if(isalpha(messageText[len+1]) ) sscanf(messageText+len, "{%[^}]}", comment);
			break;
		    }
		    if(messageText[len] == '[') { // we detected thinking output
			int depth; float score=0; char c, lastMover = (dotCount == 3 ? 'B' : 'W');
			if(sscanf(messageText+len+1, "%d]%c%f", &depth, &c, &score) > 1) {
			    if(c == ' ') { // if not explicitly specified, figure out source of thinking output
				switch(gameMode) {
				  case MachinePlaysWhite:
				  case IcsPlayingWhite:
				    c = 'W'; break;
				  case IcsPlayingBlack:
				  case MachinePlaysBlack:
				    c = 'B'; 
				  default:
				    break;
				}
			    }
			    if(c != lastMover) return; // line is thinking output of future move, ignore.
			    if(2*moveNr - (dotCount < 2) == previousMove)
				return; // do not repeat same move; likely ponder output
			    sprintf(buf, "score %s %d at %d ply", 
					score > 0 ? "plus" : score < 0 ? "minus" : "",
					(int) (fabs(score)*100+0.5),
					depth );
			    SayString(buf, FALSE); // move + thinking output describing it; say it.
			}
			while(messageText[len-1] == ' ') len--; // position just behind move;
			break;
		    }
		    if(messageText[len] == '(') { // ICS time printed behind move
			while(messageText[len+1] && messageText[len] != ')') len++; // skip it
		    }
		    len++;
		}
		if(secondSpace) len = secondSpace; // position behind move
		if(messageText[len-1] == '+' || messageText[len-1] == '#') {  /* you are in checkmate */
			len--; // strip off check or mate indicator
		}
		if(messageText[len-2] == '=') {  /* promotion */
			len-=2; // strip off promotion piece
			SayString("promotion", FALSE);
		}

		n = 2*moveNr - (dotCount < 2);

		if(previousMove != 2*moveNr + (dotCount > 1) || evenIfDuplicate) { 
		    char number[20];
		    previousMove = 2*moveNr + (dotCount > 1); // remember move nr of move last spoken
		    sprintf(number, "%d", moveNr);

		    yPos = CoordToNum(messageText[len-1]);  /* turn char coords to ints */
		    xPos = CoordToNum(messageText[len-2]);
		    if(xPos < 0 || xPos > 11) return; // prevent crashes if no coord string available to speak
		    if(yPos < 0 || yPos > 9)  return;
		    currentpiece = boards[n][yPos][xPos];	
		    piece = PieceToName(currentpiece,0);
		    ynum = SquareToNum(yPos);
		    xchar = SquareToChar(xPos);
		    c = messageText[len-3];
		    if(c == 'x') c = messageText[len-4];
		    if(!isdigit(c) && c < 'a' && c != '@') c = 0;
		    disambiguation[0] = c;
		    SayString(WhiteOnMove(n) ? "Black" : "White", FALSE);
		    SayString("move", FALSE);
		    SayString(number, FALSE);
//		    if(c==0 || c=='@') SayString("a", FALSE);
		    // intercept castling moves
		    p = StrStr(messageText, "O-O-O");
		    if(p && p-messageText < len) {
			SayString("queen side castling",FALSE);
			castle = 1;
		    } else {
			p = StrStr(messageText, "O-O");
			if(p && p-messageText < len) {
			    SayString("king side castling",FALSE);
			    castle = 1;
			}
		    }
		    if(!castle) {
			SayString(piece, FALSE);
			if(c == '@') SayString("dropped on", FALSE); else
			if(c) SayString(disambiguation, FALSE);
			SayString("to", FALSE);
			SayString(xchar, FALSE);
			SayString(ynum, FALSE);
			if(messageText[len-3] == 'x') {
				currentpiece = boards[n-1][yPos][xPos];
				if(currentpiece != EmptySquare) {
					piece = PieceToName(currentpiece,0);
					SayString("Capturing a",FALSE);
					SayString(piece, FALSE);
				} else SayString("Capturing onn passann",FALSE);
			}
			if(messageText[len] == '+') SayString("check", FALSE); else
			if(messageText[len] == '#') {
				SayString("finishing off", FALSE);
				SayString(WhiteOnMove(n) ? "White" : "Black", FALSE);
			}
		    }
		}

	        /* say comment after move, possibly with result */
		p = NULL;
	        if(StrStr(messageText, " 1-0")) p = "white wins"; else
	        if(StrStr(messageText, " 0-1")) p = "black wins"; else
	        if(StrStr(messageText, " 1/2-1/2")) p = "game ends in a draw";
	        if(comment[0]) {
		    if(p) {
			if(!StrCaseStr(comment, "draw") && 
			   !StrCaseStr(comment, "white") && 
			   !StrCaseStr(comment, "black") ) {
				SayString(p, FALSE);
				SayString("due to", FALSE);
			}
		    }
		    SayString(comment, FALSE); // alphabetic comment (usually game end)
	        } else if(p) SayString(p, FALSE);

	    } else {
		/* starts not with digit */
		if(StrCaseStr(messageText, "illegal")) PlayIcsUnfinishedSound();
		SayString(messageText, FALSE);
	    }

}

VOID
SayClockTime()
{
	char buf1[50], buf2[50];
	char *str1, *str2;
	static long int lastWhiteTime, lastBlackTime;

	suppressClocks = 1; // if user is using alt+T command, no reason to display them
	if(abs(lastWhiteTime - whiteTimeRemaining) < 1000 && abs(lastBlackTime - blackTimeRemaining) < 1000)
		suppressClocks = 0; // back on after two requests in rapid succession
	sprintf(buf1, "%s", TimeString(whiteTimeRemaining));
	str1 = buf1;
	SayString("White's remaining time is", FALSE);
	SayString(str1, FALSE);
	sprintf(buf2, "%s", TimeString(blackTimeRemaining));
	str2 = buf2;
	SayString("Black's remaining time is", FALSE);
	SayString(str2, FALSE);
	lastWhiteTime = whiteTimeRemaining;
	lastBlackTime = blackTimeRemaining;
}

VOID
Toggle(Boolean *b, char *mess)
{
	*b = !*b;
	SayString(mess, FALSE);
	SayString("is now", FALSE);
	SayString(*b ? "on" : "off", FALSE);
}

/* handles keyboard moves in a click-click fashion */
VOID
KeyboardMove(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	ChessSquare currentpiece;
	char *piece;
	
	static BOOLEAN sameAgain = FALSE;
	switch (message) {
	case WM_KEYDOWN:
		sameAgain = FALSE;
		if(oldFromX == fromX && oldFromY == fromY) {
			sameAgain = TRUE;
			/* click on same square */
			break;
		}
		else if(oldFromX != -1) {
			
			ChessSquare pdown, pup;
      pdown = boards[currentMove][oldFromY][oldFromX];
      pup = boards[currentMove][fromY][fromX];
		
		if (gameMode == EditPosition ||
			!((WhitePawn <= pdown && pdown <= WhiteKing &&
				 WhitePawn <= pup && pup <= WhiteKing) ||
				(BlackPawn <= pdown && pdown <= BlackKing &&
				 BlackPawn <= pup && pup <= BlackKing))) {
			/* EditPosition, empty square, or different color piece;
			click-click move is possible */
		
			if (IsPromotion(oldFromX, oldFromY, fromX, fromY)) {
				if (appData.alwaysPromoteToQueen) {
					UserMoveEvent(oldFromX, oldFromY, fromX, fromY, 'q');
				}
				else {
					toX = fromX; toY = fromY; fromX = oldFromX; fromY = oldFromY;
					PromotionPopup(hwnd);
					fromX = toX; fromY = toY;
				}	
			}
			else {
				UserMoveEvent(oldFromX, oldFromY, fromX, fromY, NULLCHAR);
			}
		oldFromX = oldFromY = -1;
		break;
		}
		
		}
		/* First downclick, or restart on a square with same color piece */
		if (OKToStartUserMove(fromX, fromY)) {
		oldFromX = fromX;
		oldFromY = fromY;
		currentpiece = boards[currentMove][fromY][fromX];	
		piece = PieceToName(currentpiece,1);
		SayString(piece, FALSE);
		SayString("selected", FALSE);
		}
		else {
		oldFromX = oldFromY = -1;
		}
		break;

	case WM_KEYUP:
		if (oldFromX == fromX && oldFromY == fromY) {
      /* Upclick on same square */
      if (sameAgain) {
	/* Clicked same square twice: abort click-click move */
			oldFromX = oldFromY = -1;
			currentpiece = boards[currentMove][fromY][fromX];	
			piece = PieceToName(currentpiece,0);
			SayString(piece, FALSE);
			SayString("unselected", FALSE);
			}
		}
	}
}

int
NiceTime(int x)
{	// return TRUE for times we want to announce
	if(x<0) return 0;
	x = (x+50)/100;   // tenth of seconds
	if(x <= 100) return (x%10 == 0);
	if(x <= 600) return (x%100 == 0);
	if(x <= 6000) return (x%600 == 0);
	return (x%3000 == 0);
}

#if 0
	    if(isalpha((char)wParam)) {
		/* capitals of any ind are intercepted and distinguished by left and right shift */
		int mine = GetKeyState(VK_RSHIFT) < 0;
		if(mine || GetKeyState(VK_LSHIFT) < 0) {
#endif

#define JAWS_ALT_INTERCEPT \
	    if(suppressOneKey) {\
		suppressOneKey = 0;\
		if(GetKeyState(VK_MENU) < 0 && GetKeyState(VK_CONTROL) < 0) break;\
	    }\
	    if ((char)wParam == 022 && gameMode == EditPosition) { /* <Ctl R>. Pop up piece menu */\
		POINT pt; int x, y;\
		SquareToPos(fromY, fromX, &x, &y);\
		pt.x = x; pt.y = y;\
        	if(gameInfo.variant != VariantShogi)\
		    MenuPopup(hwnd, pt, LoadMenu(hInst, "PieceMenu"), -1);\
	        else\
		    MenuPopup(hwnd, pt, LoadMenu(hInst, "ShogiPieceMenu"), -1);\
		break;\
	    }\

#define JAWS_REPLAY \
    case '\020': /* ctrl P */\
      { char buf[MSG_SIZ];\
	if(GetWindowText(hwnd, buf, MSG_SIZ-1))\
		SayString(buf, FALSE);\
      }\
      return 0;\

#define JAWS_KB_NAVIGATION \
\
	case WM_KEYDOWN:\
\
		if(GetKeyState(VK_MENU) < 0 && GetKeyState(VK_CONTROL) < 0) {\
		    /* Control + Alt + letter used for speaking piece positions */\
		    static int lastTime; static char lastChar;\
		    int mine = 0, time = GetTickCount(); char c;\
\
		    if((char)wParam == lastChar && time-lastTime < 250) mine = 1;\
		    lastChar = wParam; lastTime = time;\
		    c = wParam;\
\
		    if(gameMode == IcsPlayingWhite || gameMode == MachinePlaysBlack) mine = !mine;\
\
		    if(ToLower(c) == 'x') {\
			SayPieces(mine ? WhitePlay : BlackPlay);\
			suppressOneKey = 1;\
			break;\
		    } else\
		    if(CharToPiece(c) != EmptySquare) {\
			SayPieces(CharToPiece(mine ? ToUpper(c) : ToLower(c)));\
			suppressOneKey = 1;\
			break;\
		    }\
		}\
\
		switch (wParam) {\
		case VK_LEFT:\
		case VK_RIGHT:\
		case VK_UP:\
		case VK_DOWN:\
			KeyboardEvent(hwnd, message, wParam, lParam);\
			break;\
		case VK_SPACE:\
			KeyboardMove(hwnd, message, wParam, lParam);\
			break;\
		}\
		break;\
	case WM_KEYUP:\
		switch (wParam) {\
		case VK_SPACE:\
			KeyboardMove(hwnd, message, wParam, lParam);\
			break;\
		}\
		break;\

#define JAWS_MENU_ITEMS \
		case IDM_PossibleAttackMove:  /*What can I possible attack from here */\
			PossibleAttackMove();\
			break;\
\
		case IDM_PossibleAttacked:  /*what can possible attack this square*/\
			PossibleAttacked();\
			break;\
\
		case IDM_ReadRow:   /* Read the current row of pieces */\
			ReadRow();\
			break;\
\
		case IDM_ReadColumn:   /* Read the current column of pieces */\
			ReadColumn();\
			break;\
\
		case IDM_SayCurrentPos: /* Say current position including color */\
			SayCurrentPos();\
			break;\
\
		case IDM_SayAllBoard:  /* Say the whole board from bottom to top */\
			SayAllBoard();\
			break;\
\
		case IDM_SayMachineMove:  /* Say the last move made */\
			timeflag = 1;\
			SayMachineMove(1);\
			break;\
\
		case IDM_SayUpperDiagnols:  /* Says the diagnol positions above you */\
			SayUpperDiagnols();\
			break;\
\
		case IDM_SayLowerDiagnols:  /* Say the diagnol positions below you */\
			SayLowerDiagnols();\
			break;\
\
		case IDM_SayBlackPieces: /*Say the opponents pieces */\
			SayPieces(BlackPlay);\
			break;\
\
		case IDM_SayWhitePieces: /*Say the opponents pieces */\
			SayPieces(WhitePlay);\
			break;\
\
		case IDM_SayClockTime:  /*Say the clock time */\
			SayClockTime();\
			break;\
\
		case IDM_SayWhosTurn:   /* Say whos turn it its */\
			SayWhosTurn();\
			break;\
\
		case IDM_SayKnightMoves:  /* Say Knights (L-shaped) move */\
			SayKnightMoves();\
			break;\
\
		case OPT_PonderNextMove:  /* Toggle option setting */\
			Toggle(&appData.ponderNextMove, "ponder");\
			break;\
\
		case OPT_AnimateMoving:  /* Toggle option setting */\
			Toggle(&appData.animate, "animate moving");\
			break;\
\
		case OPT_AutoFlag:  /* Toggle option setting */\
			Toggle(&appData.autoCallFlag, "auto flag");\
			break;\
\
		case OPT_AlwaysQueen:  /* Toggle option setting */\
			Toggle(&appData.alwaysPromoteToQueen, "always promote to queen");\
			break;\
\
		case OPT_TestLegality:  /* Toggle option setting */\
			Toggle(&appData.testLegality, "legality testing");\
			break;\
\
		case OPT_HideThinkFromHuman:  /* Toggle option setting */\
			Toggle(&appData.hideThinkingFromHuman, "hide thinking");\
			ShowThinkingEvent();\
			break;\
\
		case OPT_SaveExtPGN:  /* Toggle option setting */\
			Toggle(&appData.saveExtendedInfoInPGN, "extended P G N info");\
			break;\
\
		case OPT_ExtraInfoInMoveHistory:  /* Toggle option setting */\
			Toggle(&appData.showEvalInMoveHistory, "extra info in move histoty");\
			break;\
\


#define JAWS_INIT if (!InitJAWS()) return (FALSE);

#define JAWS_DELETE(X)

#define JAWS_SILENCE if(suppressClocks) return;

#define SAY(S) SayString((S), FALSE)

#define SAYMACHINEMOVE() SayMachineMove(0)

// After inclusion of this file somewhere early in winboard.c, the remaining part of the patch
// is scattered over winboard.c for actually calling the routines.
//
// * move fromX, fromY declaration to front, before incusion of this file. (Can be permanent change in winboard.c.)
// * call InitJAWS(), after calling InitIntance(). (Using JAWS_INIT macro)
// * add keyboard cases in main switch of WndProc, though JAWS_KB_NAVIGATION above, e.g. before WM_CHAR case.
// * change the WM_CHAR case of same switch from "if(appData.icsActive)" to "if(appData.icsActive JAWS_IF_TAB)"
// * add new menu cases in WM_COMMAND case of WndProc, e.g. before IDM_Forward. (throug macro defined above)
// * add SAYMACHINEMOVE(); at the end of DisplayMessage();
// * add SAY("board"); in WM_CHAR case of ConsoleTextSubclass, just before "SetFocus(buttondesc..."
