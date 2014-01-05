/*
 * JAWS.c -- Code for Windows front end to XBoard to use it with JAWS
 *
 * Copyright 1991 by Digital Equipment Corporation, Maynard,
 * Massachusetts.
 *
 * Enhancements Copyright 1992-2001, 2002, 2003, 2004, 2005, 2006,
 * 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014 Free Software Foundation, Inc.
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

// from resource.h

#define IDM_PossibleAttackMove          1800
#define IDM_PossibleAttacked            1801
#define IDM_SayMachineMove              1802
#define IDM_ReadRow                     1803
#define IDM_ReadColumn                  1804
#define IDM_SayCurrentPos               1805
#define IDM_SayAllBoard                 1806
#define IDM_SayUpperDiagnols            1807
#define IDM_SayLowerDiagnols            1808
#define IDM_SayClockTime                1810
#define IDM_SayWhosTurn                 1811
#define IDM_SayKnightMoves              1812
#define ID_SHITTY_HI                    1813
#define IDM_SayWhitePieces              1816
#define IDM_SayBlackPieces              1817


// from common.h, but 'extern' added to it, so the actual declaraton can remain in backend.c

extern long whiteTimeRemaining, blackTimeRemaining, timeControl, timeIncrement;

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

char *pieceTypeName[] = {
		"Pawn", "Knight", "Bishop", "Rook", "Queen",
		"Guard", "Elephant", "Arch Bishop", "Chancellor",
		"General", "Man", "Cannon", "Night Rider",
		"Crowned Bishop", "Crowned Rook", "Grass Hopper", "Veteran",
		"Falcon", "Amazon", "Snake", "Unicorn",
		"King",
		"Pawn", "Knight", "Bishop", "Rook", "Queen",
		"Guard", "Elephant", "Arch Bishop", "Chancellor",
		"General", "Man", "Cannon", "Night Rider",
		"Crowned Bishop", "Crowned Rook", "Grass Hopper", "Veteran",
		"Falcon", "Amazon", "Snake", "Unicorn",
		"King",
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
		return pieceTypeName[(int) p];
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
#define JFWAPI __declspec(dllimport)
JFWAPI BOOL WINAPI JFWSayString (LPCTSTR lpszStrinToSpeak, BOOL bInterrupt);

typedef JFWAPI BOOL (WINAPI *PSAYSTRING)(LPCTSTR lpszStrinToSpeak, BOOL bInterrupt);

PSAYSTRING RealSayString;

VOID SayString(char *mess, BOOL flag)
{ // for debug file
	static char buf[8000], *p;
        int l = strlen(buf);
	if(appData.debugMode) fprintf(debugFP, "SAY '%s'\n", mess);
        if(l) buf[l++] = ' '; // separate by space from previous
	safeStrCpy(buf+l, _(mess), 8000-1-l); // buffer
        if(!flag) return; // wait for flush
	if(p = StrCaseStr(buf, "Xboard adjudication:")) {
		int i;
		for(i=19; i>1; i--) p[i] = p[i-1];
		p[1] = ' ';
	}
	RealSayString(buf, !strcmp(mess, " ")); // kludge to indicate flushing of interruptable speach
	if(appData.debugMode) fprintf(debugFP, "SPEAK '%s'\n", buf);
	buf[0] = NULLCHAR;
}

//static int fromX = 0, fromY = 0;
static int oldFromX, oldFromY;
static int timeflag;
static int suppressClocks = 0;
static int suppressOneKey = 0;
static HANDLE hAccelJAWS;

typedef struct { char *name; int code; } MenuItemDesc;

MenuItemDesc menuItemJAWS[] = {
{"Say Clock &Time\tAlt+T",      IDM_SayClockTime },
{"-", 0 },
{"Say Last &Move\tAlt+M",       IDM_SayMachineMove },
{"Say W&ho's Turn\tAlt+X",      IDM_SayWhosTurn },
{"-", 0 },
{"Say Complete &Position\tAlt+P",IDM_SayAllBoard },
{"Say &White Pieces\tAlt+W",    IDM_SayWhitePieces },
{"Say &Black Pieces\tAlt+B",    IDM_SayBlackPieces },
{"Say Board &Rank\tAlt+R",      IDM_ReadRow },
{"Say Board &File\tAlt+F",      IDM_ReadColumn },
{"-", 0 },
{"Say &Upper Diagonals\tAlt+U",  IDM_SayUpperDiagnols },
{"Say &Lower Diagonals\tAlt+L",  IDM_SayLowerDiagnols },
{"Say K&night Moves\tAlt+N",    IDM_SayKnightMoves },
{"Say Current &Square\tAlt+S",  IDM_SayCurrentPos },
{"Say &Attacks\tAlt+A",         IDM_PossibleAttackMove },
{"Say Attacke&d\tAlt+D",        IDM_PossibleAttacked },
{NULL, 0}
};

ACCEL acceleratorsJAWS[] = {
{FVIRTKEY|FALT, 'T', IDM_SayClockTime },
{FVIRTKEY|FALT, 'M', IDM_SayMachineMove },
{FVIRTKEY|FALT, 'X', IDM_SayWhosTurn },
{FVIRTKEY|FALT, 'P', IDM_SayAllBoard },
{FVIRTKEY|FALT, 'W', IDM_SayWhitePieces },
{FVIRTKEY|FALT, 'B', IDM_SayBlackPieces },
{FVIRTKEY|FALT, 'R', IDM_ReadRow },
{FVIRTKEY|FALT, 'F', IDM_ReadColumn },
{FVIRTKEY|FALT, 'U', IDM_SayUpperDiagnols },
{FVIRTKEY|FALT, 'L', IDM_SayLowerDiagnols },
{FVIRTKEY|FALT, 'N', IDM_SayKnightMoves },
{FVIRTKEY|FALT, 'S', IDM_SayCurrentPos },
{FVIRTKEY|FALT, 'A', IDM_PossibleAttackMove },
{FVIRTKEY|FALT, 'D', IDM_PossibleAttacked }
};

void
AdaptMenu()
{
	HMENU menuMain, menuJAWS;
	MENUBARINFO helpMenuInfo;
	int i;

	helpMenuInfo.cbSize = sizeof(helpMenuInfo);
	menuMain = GetMenu(hwndMain);
	menuJAWS = CreatePopupMenu();

	for(i=0; menuItemJAWS[i].name; i++) {
	    if(menuItemJAWS[i].name[0] == '-')
		 AppendMenu(menuJAWS, MF_SEPARATOR, (UINT_PTR) 0, NULL);
	    else AppendMenu(menuJAWS, MF_ENABLED|MF_STRING,
			(UINT_PTR) menuItemJAWS[i].code, (LPCTSTR) _(menuItemJAWS[i].name));
	}
	InsertMenu(menuMain, 7, MF_BYPOSITION|MF_POPUP|MF_ENABLED|MF_STRING,
		(UINT_PTR) menuJAWS, "&JAWS");
	oldMenuItemState[8] = oldMenuItemState[7];
	DrawMenuBar(hwndMain);
}

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
		// [HGM] kludge to reduce need for modification of winboard.c: make tinyLayout menu identical
		// to standard layout, so that code for switching between them does not have to be deleted
		int i;

		AdaptMenu();
		menuBarText[0][8] = menuBarText[0][7]; menuBarText[0][7] = "&JAWS";
		for(i=0; i<9; i++) menuBarText[1][i] = menuBarText[0][i];
	}

	hAccelJAWS = CreateAcceleratorTable(acceleratorsJAWS, 14);

	/* initialize cursor position */
	fromX = fromY = 0;
	SetHighlights(fromX, fromY, -1, -1);
	DrawPosition(FALSE, NULL);
	oldFromX = oldFromY = -1;

	if(hwndConsole) SetFocus(hwndConsole);
	return TRUE;
}

int beeps[] = { 1, 0, 0, 0, 0 };
int beepCodes[] = { 0, MB_OK, MB_ICONERROR, MB_ICONQUESTION, MB_ICONEXCLAMATION, MB_ICONASTERISK };
static int dropX = -1, dropY = -1;

VOID
KeyboardEvent(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	ChessSquare currentPiece;
	char *piece, *xchar, *ynum ;
	int n, beepType = 1; // empty beep

	if(fromX == -1 || fromY == -1) { // if we just dropped piece, stay at that square
		fromX = dropX; fromY = dropY;
		dropX = dropY = -1; // but only once
        }
	if(fromX == -1 || fromY == -1) {
		fromX = BOARD_LEFT; fromY = 0;
        }
	switch(wParam) {
	case VK_LEFT:
		if(fromX == BOARD_RGHT+1) fromX -= 2; else
		if(fromX == BOARD_LEFT) { if(fromY >= BOARD_HEIGHT - gameInfo.holdingsSize) fromX -= 2; else beepType = 0; } else
		if(fromX > BOARD_LEFT) fromX--; else beepType = 0; // off-board beep
		break;
	case VK_RIGHT:
		if(fromX == BOARD_LEFT-2) fromX += 2; else
		if(fromX == BOARD_RGHT-1) { if(fromY < gameInfo.holdingsSize) fromX += 2; else beepType = 0; } else
		if(fromX < BOARD_RGHT-1) fromX++; else beepType = 0;
		break;
	case VK_UP:
		if(fromX == BOARD_RGHT+1) { if(fromY < gameInfo.holdingsSize - 1) fromY++; else beepType = 0; } else
		if(fromY < BOARD_HEIGHT-1) fromY++; else beepType = 0;
		break;
	case VK_DOWN:
		if(fromX == BOARD_LEFT-2) { if(fromY > BOARD_HEIGHT - gameInfo.holdingsSize) fromY--; else beepType = 0; } else
		if(fromY > 0) fromY--; else beepType = 0;
		break;
	}
	SetHighlights(fromX, fromY, -1, -1);
	DrawPosition(FALSE, NULL);
	currentPiece = boards[currentMove][fromY][fromX];
	piece = PieceToName(currentPiece,1);
	if(beepType == 1 && currentPiece != EmptySquare) beepType = currentPiece < (int) BlackPawn ? 2 : 3; // white or black beep
	if(beeps[beepType] == beeps[1] && (fromX == BOARD_RGHT+1 || fromX == BOARD_LEFT-2)) beepType = 4; // holdings beep
	beepType = beeps[beepType]%6;
	if(beepType) MessageBeep(beepCodes[beepType]);
	if(fromX == BOARD_LEFT - 2) {
		SayString("black holdings", FALSE);
		if(currentPiece != EmptySquare) {
			char buf[MSG_SIZ];
			n = boards[currentMove][fromY][1];
			snprintf(buf, MSG_SIZ, "%d %s%s", n, PieceToName(currentPiece,0), n == 1 ? "" : "s");
			SayString(buf, FALSE);
		}
		SayString(" ", TRUE);
	} else
	if(fromX == BOARD_RGHT + 1) {
		SayString("white holdings", FALSE);
		if(currentPiece != EmptySquare) {
			char buf[MSG_SIZ];
			n = boards[currentMove][fromY][BOARD_WIDTH-2];
			snprintf(buf, MSG_SIZ,"%d %s%s", n, PieceToName(currentPiece,0), n == 1 ? "" : "s");
			SayString(buf, FALSE);
		}
		SayString(" ", TRUE);
	} else
	if(fromX >= BOARD_LEFT && fromX < BOARD_RGHT) {
		char buf[MSG_SIZ];
		xchar = SquareToChar(fromX);
		ynum = SquareToNum(fromY);
		if(currentPiece != EmptySquare) {
		  snprintf(buf, MSG_SIZ, "%s %s %s", xchar, ynum, piece);
		} else snprintf(buf, MSG_SIZ, "%s %s", xchar, ynum);
		SayString(buf, FALSE);
		SayString(" ", TRUE);
	}
	return;
}

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
	if(fromX < BOARD_LEFT || fromX >= BOARD_RGHT) { SayString("holdings",TRUE); return; }

	piece = boards[currentMove][fromY][fromX];
	if(piece == EmptySquare) { // if square is empty, try to substitute selected piece
	    if(oldFromX >= 0 && oldFromY >= 0) {
		piece = boards[currentMove][oldFromY][oldFromX];
		boards[currentMove][oldFromY][oldFromX] = EmptySquare;
		removedSelectedPiece = 1;
		SayString("Your", FALSE);
		SayString(PieceToName(piece, 0), FALSE);
		SayString("would have", FALSE);
	    } else { SayString("You must select a piece first", TRUE); return; }
	}

	victim = boards[currentMove][fromY][fromX];
	boards[currentMove][fromY][fromX] = piece; // make sure piece is actally there
	SayString("possible captures from here are", FALSE);

	swapColor = piece <  (int)BlackPawn && !WhiteOnMove(currentMove) ||
		    piece >= (int)BlackPawn &&  WhiteOnMove(currentMove);
	cl.count = 0; cl.rf = fromY; cl.ff = fromX; cl.rt = cl.ft = -1;
	GenLegal(boards[currentMove], PosFlags(currentMove + swapColor), ReadCallback, (VOIDSTAR) &cl);
	if(cl.count == 0) SayString("None", FALSE);
	SayString("", TRUE); // flush
	boards[currentMove][fromY][fromX] = victim; // repair

	if( removedSelectedPiece ) boards[currentMove][oldFromY][oldFromX] = piece;
}


VOID
PossibleAttacked()
{
	ReadClosure cl;
	ChessSquare piece = EmptySquare, victim;

	if(fromY < 0 || fromY >= BOARD_HEIGHT) return;
	if(fromX < BOARD_LEFT || fromX >= BOARD_RGHT) { SayString("holdings",TRUE); return; }

	if(oldFromX >= 0 && oldFromY >= 0) { // if piece is selected, remove it
		piece = boards[currentMove][oldFromY][oldFromX];
		boards[currentMove][oldFromY][oldFromX] = EmptySquare;
	}

	SayString("Pieces that can capture you are", FALSE);

	victim = boards[currentMove][fromY][fromX]; // put dummy piece on target square, to activate Pawn captures
	boards[currentMove][fromY][fromX] = WhiteOnMove(currentMove) ? WhiteQueen : BlackQueen;
	cl.count = 0; cl.rt = fromY; cl.ft = fromX; cl.rf = cl.ff = -1;
	GenLegal(boards[currentMove], PosFlags(currentMove+1), ReadCallback, (VOIDSTAR) &cl);
	if(cl.count == 0) SayString("None", FALSE);

	SayString("You are defended by", FALSE);

	boards[currentMove][fromY][fromX] = WhiteOnMove(currentMove) ? BlackQueen : WhiteQueen;
	cl.count = 0; cl.rt = fromY; cl.ft = fromX; cl.rf = cl.ff = -1;
	GenLegal(boards[currentMove], PosFlags(currentMove), ReadCallback, (VOIDSTAR) &cl);
	if(cl.count == 0) SayString("None", FALSE);
	SayString("", TRUE); // flush
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
	SayString("", TRUE); // flush
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
	SayString("", TRUE); // flush
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
	SayString("", TRUE); // flush
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
	SayString("", TRUE); // flush
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
	SayString("", TRUE); // flush
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
	  snprintf(buf, sizeof(buf)/sizeof(buf[0]),"%ss", PieceToName(p,1));
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
	SayString("", TRUE); // flush
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
	if(((fromX-BOARD_LEFT) ^ fromY)&1)
		SayString("on a light square",FALSE);
	else
		SayString("on a dark square",FALSE);

	PossibleAttacked();
	SayString("", TRUE); // flush
}

VOID
SayAllBoard()
{
	int Xpos, Ypos;
	ChessSquare currentpiece;
	char *piece, *ynum ;

	if(gameInfo.holdingsWidth) {
		int first = 0;
		for(Ypos=0; Ypos<gameInfo.holdingsSize; Ypos++) {
			int n = boards[currentMove][Ypos][BOARD_WIDTH-2];
			if(n) {
			  char buf[MSG_SIZ];
			  if(!first++)
			    SayString("white holds", FALSE);
			  currentpiece = boards[currentMove][Ypos][BOARD_WIDTH-1];
			  piece = PieceToName(currentpiece,0);
			  snprintf(buf, MSG_SIZ,"%d %s%s", n, piece, (n==1 ? "" : "s") );
			  SayString(buf, FALSE);
			}
		}
		first = 0;
		for(Ypos=BOARD_HEIGHT-1; Ypos>=BOARD_HEIGHT - gameInfo.holdingsSize; Ypos--) {
			int n = boards[currentMove][Ypos][1];
			if(n) {
			  char buf[MSG_SIZ];
			  if(!first++)
			    SayString("black holds", FALSE);
			  currentpiece = boards[currentMove][Ypos][0];
			  piece = PieceToName(currentpiece,0);
			  snprintf(buf, MSG_SIZ, "%d %s%s", n, piece, (n==1 ? "" : "s") );
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
				int count = 0;
				char buf[50];
				piece = PieceToName(currentpiece,1);
				while(Xpos < BOARD_RGHT && boards[currentMove][Ypos][Xpos] == currentpiece)
					Xpos++, count++;
				if(count > 1)
				  snprintf(buf, sizeof(buf)/sizeof(buf[0]), "%d %ss", count, piece);
				else
				  snprintf(buf, sizeof(buf)/sizeof(buf[0]), "%s", piece);
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
					snprintf(buf, sizeof(buf)/sizeof(buf[0]),"%d", count);
					SayString(buf, FALSE);
				    }
				    Xpos--;
				}
				SayString("empty", FALSE);
			}
		}
	}
	SayString("", TRUE); // flush
}

VOID
SayWhosTurn()
{
	if(gameMode == MachinePlaysBlack || gameMode == IcsPlayingWhite) {
		if(WhiteOnMove(currentMove))
			SayString("It is your turn", FALSE);
		else	SayString("It is your opponents turn", FALSE);
	} else if(gameMode == MachinePlaysWhite || gameMode == IcsPlayingBlack) {
		if(WhiteOnMove(currentMove))
			SayString("It is your opponents turn", FALSE);
		else	SayString("It is your turn", FALSE);
	} else {
		if(WhiteOnMove(currentMove))
			SayString("White is on move here", FALSE);
		else	SayString("Black is on move here", FALSE);
	}
	SayString("", TRUE); // flush
}

extern char *commentList[];

VOID
SayMachineMove(int evenIfDuplicate)
{
	int len, xPos, yPos, moveNr, secondSpace = 0, castle = 0, n;
	ChessSquare currentpiece;
	char *piece, *xchar, *ynum, *p, checkMark = 0;
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
			    if(c != lastMover && !evenIfDuplicate) return; // line is thinking output of future move, ignore.
			    if(2*moveNr - (dotCount < 2) == previousMove)
				return; // do not repeat same move; likely ponder output
			    snprintf(buf, MSG_SIZ, "score %s %d at %d ply",
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
		      checkMark = messageText[len]; // make sure still seen after we stip off promo piece
		}
		if(messageText[len-2] == '=') {  /* promotion */
			len-=2; // strip off promotion piece
			SayString("promotion", FALSE);
		}

		n = 2*moveNr - (dotCount < 2);

		if(previousMove != 2*moveNr + (dotCount > 1) || evenIfDuplicate) {
		    char number[20];
		    previousMove = 2*moveNr + (dotCount > 1); // remember move nr of move last spoken
		    snprintf(number, sizeof(number)/sizeof(number[0]),"%d", moveNr);

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
		    }
		    if(checkMark == '+') SayString("check", FALSE); else
		    if(checkMark == '#') {
				SayString("finishing off", FALSE);
				SayString(WhiteOnMove(n) ? "White" : "Black", FALSE);
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

		if(commentDialog && commentList[currentMove]) SetFocus(commentDialog);

	    } else {
		/* starts not with digit */
		if(StrCaseStr(messageText, "illegal")) PlayIcsUnfinishedSound();
		SayString(messageText, FALSE);
	    }

	SayString("", TRUE); // flush
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
	snprintf(buf1, sizeof(buf1)/sizeof(buf1[0]),"%s", TimeString(whiteTimeRemaining));
	str1 = buf1;
	SayString("White clock", FALSE);
	SayString(str1, FALSE);
	snprintf(buf2, sizeof(buf2)/sizeof(buf2[0]), "%s", TimeString(blackTimeRemaining));
	str2 = buf2;
	SayString("Black clock", FALSE);
	SayString(str2, FALSE);
	lastWhiteTime = whiteTimeRemaining;
	lastBlackTime = blackTimeRemaining;
	SayString("", TRUE); // flush
}

VOID
Toggle(Boolean *b, char *mess)
{
	*b = !*b;
	SayString(mess, FALSE);
	SayString("is now", FALSE);
	SayString(*b ? "on" : "off", FALSE);
	SayString("", TRUE); // flush
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
			char promoChoice = NULLCHAR;

			if (HasPromotionChoice(oldFromX, oldFromY, fromX, fromY, &promoChoice)) {
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
				UserMoveEvent(oldFromX, oldFromY, fromX, fromY, promoChoice);
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
		SayString("selected", TRUE);
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
			SayString("unselected", TRUE);
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

#define JAWS_ARGS \
  { "beepOffBoard", ArgInt, (LPVOID) beeps, TRUE, (ArgIniType) 1 },\
  { "beepEmpty", ArgInt, (LPVOID) (beeps+1), TRUE, (ArgIniType) 0 },\
  { "beepWhite", ArgInt, (LPVOID) (beeps+2), TRUE, (ArgIniType) 0 },\
  { "beepBlack", ArgInt, (LPVOID) (beeps+3), TRUE, (ArgIniType) 0 },\
  { "beepHoldings", ArgInt, (LPVOID) (beeps+4), TRUE, (ArgIniType) 0 },\

#define JAWS_ALT_INTERCEPT \
	    if(suppressOneKey) {\
		suppressOneKey = 0;\
		if(GetKeyState(VK_MENU) < 0 && GetKeyState(VK_CONTROL) < 0) break;\
	    }\
	    if ((char)wParam == 022 && gameMode == EditPosition) { /* <Ctl R>. Pop up piece menu */\
		POINT pt; int x, y;\
		SquareToPos(fromY, fromX, &x, &y);\
		dropX = fromX; dropY = fromY;\
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
		SayString(buf, TRUE);\
      }\
      return 0;\

#define JAWS_KBDOWN_NAVIGATION \
\
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
                        shiftKey = GetKeyState(VK_SHIFT) < 0;\
			KeyboardMove(hwnd, message, wParam, lParam);\
			break;\
		}\

#define JAWS_KBUP_NAVIGATION \
		switch (wParam) {\
		case VK_SPACE:\
			KeyboardMove(hwnd, message, wParam, lParam);\
			break;\
		}\

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


#define JAWS_ACCEL \
	!(!frozen && TranslateAccelerator(hwndMain, hAccelJAWS, &msg)) &&

#define JAWS_INIT if (!InitJAWS()) return (FALSE);

#define JAWS_DELETE(X)

#define JAWS_SILENCE if(suppressClocks) return;

#define JAWS_COPYRIGHT \
	SetDlgItemText(hDlg, OPT_MESS, "Auditory/Keyboard Enhancements  By:  Ed Rodriguez (sort of)");

#define SAY(S) SayString((S), TRUE)

#define SAYMACHINEMOVE() SayMachineMove(0)

// After inclusion of this file somewhere early in winboard.c, the remaining part of the patch
// is scattered over winboard.c for actually calling the routines.
