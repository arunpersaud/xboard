/*
 * gamelist.c -- Functions to manage a gamelist
 *
 * Copyright 1995, 2009, 2010, 2011, 2012, 2013, 2014 Free Software Foundation, Inc.
 *
 * Enhancements Copyright 2005 Alessandro Scotti
 *
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

#include "config.h"

#include <stdio.h>
#include <errno.h>
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

#include "common.h"
#include "frontend.h"
#include "backend.h"
#include "parser.h"
#include "moves.h"
#include "gettext.h"

#ifdef ENABLE_NLS
# define  _(s) gettext (s)
# define N_(s) gettext_noop (s)
#else
# define  _(s) (s)
# define N_(s)  s
#endif


/* Variables
 */
List gameList;
extern Board initialPosition;
extern int quickFlag;
extern int movePtr;

/* Local function prototypes
 */
static void GameListDeleteGame P((ListGame *));
static ListGame *GameListCreate P((void));
static void GameListFree P((List *));
static int GameListNewGame P((ListGame **));

/* [AS] Wildcard pattern matching */
Boolean
HasPattern (const char * text, const char * pattern)
{
    while( *pattern != '\0' ) {
        if( *pattern == '*' ) {
            while( *pattern == '*' ) {
                pattern++;
            }

            if( *pattern == '\0' ) {
                return TRUE;
            }

            while( *text != '\0' ) {
                if( HasPattern( text, pattern ) ) {
                    return TRUE;
                }
                text++;
            }
        }
        else if( (*pattern == *text) || ((*pattern == '?') && (*text != '\0')) ) {
            pattern++;
            text++;
            continue;
        }

        return FALSE;
    }

    return TRUE;
}

Boolean
SearchPattern (const char * text, const char * pattern)
{
    Boolean result = TRUE;

    if( pattern != NULL && *pattern != '\0' ) {
        if( *pattern == '*' ) {
            result = HasPattern( text, pattern );
        }
        else {
            result = FALSE;

            while( *text != '\0' ) {
                if( HasPattern( text, pattern ) ) {
                    result = TRUE;
                    break;
                }
                text++;
            }
        }
    }

    return result;
}

/* Delete a ListGame; implies removint it from a list.
 */
static void
GameListDeleteGame (ListGame *listGame)
{
    if (listGame) {
	if (listGame->gameInfo.event) free(listGame->gameInfo.event);
	if (listGame->gameInfo.site) free(listGame->gameInfo.site);
	if (listGame->gameInfo.date) free(listGame->gameInfo.date);
	if (listGame->gameInfo.round) free(listGame->gameInfo.round);
	if (listGame->gameInfo.white) free(listGame->gameInfo.white);
	if (listGame->gameInfo.black) free(listGame->gameInfo.black);
	if (listGame->gameInfo.fen) free(listGame->gameInfo.fen);
	if (listGame->gameInfo.resultDetails) free(listGame->gameInfo.resultDetails);
	if (listGame->gameInfo.timeControl) free(listGame->gameInfo.timeControl);
	if (listGame->gameInfo.extraTags) free(listGame->gameInfo.extraTags);
        if (listGame->gameInfo.outOfBook) free(listGame->gameInfo.outOfBook);
	ListNodeFree((ListNode *) listGame);
    }
}


/* Free the previous list of games.
 */
static void
GameListFree (List *gameList)
{
  while (!ListEmpty(gameList))
    {
	GameListDeleteGame((ListGame *) gameList->head);
    }
}



/* Initialize a new GameInfo structure.
 */
void
GameListInitGameInfo (GameInfo *gameInfo)
{
    gameInfo->event = NULL;
    gameInfo->site = NULL;
    gameInfo->date = NULL;
    gameInfo->round = NULL;
    gameInfo->white = NULL;
    gameInfo->black = NULL;
    gameInfo->result = GameUnfinished;
    gameInfo->fen = NULL;
    gameInfo->resultDetails = NULL;
    gameInfo->timeControl = NULL;
    gameInfo->extraTags = NULL;
    gameInfo->whiteRating = -1; /* unknown */
    gameInfo->blackRating = -1; /* unknown */
    gameInfo->variant = VariantNormal;
    gameInfo->variantName = NULL;
    gameInfo->outOfBook = NULL;
    gameInfo->resultDetails = NULL;
}


/* Create empty ListGame; returns ListGame or NULL, if out of memory.
 *
 * Note, that the ListGame is *not* added to any list
 */
static ListGame *
GameListCreate ()
{
    ListGame *listGame;

    if ((listGame = (ListGame *) ListNodeCreate(sizeof(*listGame)))) {
	GameListInitGameInfo(&listGame->gameInfo);
    }
    return(listGame);
}


/* Creates a new game for the gamelist.
 */
static int
GameListNewGame (ListGame **listGamePtr)
{
    if (!(*listGamePtr = (ListGame *) GameListCreate())) {
	GameListFree(&gameList);
	return(ENOMEM);
    }
    ListAddTail(&gameList, (ListNode *) *listGamePtr);
    return(0);
}


/* Build the list of games in the open file f.
 * Returns 0 for success or error number.
 */
int
GameListBuild (FILE *f)
{
    ChessMove cm, lastStart;
    int gameNumber;
    ListGame *currentListGame = NULL;
    int error, scratch=100, plyNr=0, fromX, fromY, toX, toY;
    int offset;
    char lastComment[MSG_SIZ], buf[MSG_SIZ];
    TimeMark t, t2;

    GetTimeMark(&t);
    GameListFree(&gameList);
    yynewfile(f);
    gameNumber = 0;
    movePtr = 0;

    lastStart = (ChessMove) 0;
    yyskipmoves = FALSE;
    do {
        yyboardindex = scratch;
	offset = yyoffset();
	quickFlag = plyNr + 1;
	cm = (ChessMove) Myylex();
	switch (cm) {
	  case GNUChessGame:
	    if ((error = GameListNewGame(&currentListGame))) {
		rewind(f);
		yyskipmoves = FALSE;
		return(error);
	    }
	    currentListGame->number = ++gameNumber;
	    currentListGame->offset = offset;
	    if(1) { CopyBoard(boards[scratch], initialPosition); plyNr = 0; currentListGame->moves = PackGame(boards[scratch]); }
	    if (currentListGame->gameInfo.event != NULL) {
		free(currentListGame->gameInfo.event);
	    }
	    currentListGame->gameInfo.event = StrSave(yy_text);
	    lastStart = cm;
	    break;
	  case XBoardGame:
	    lastStart = cm;
	    break;
	  case MoveNumberOne:
	    switch (lastStart) {
	      case GNUChessGame:
		break;		/*  ignore  */
	      case PGNTag:
		lastStart = cm;
		break;		/*  Already started */
	      case (ChessMove) 0:
	      case MoveNumberOne:
	      case XBoardGame:
		if ((error = GameListNewGame(&currentListGame))) {
		    rewind(f);
		    yyskipmoves = FALSE;
		    return(error);
		}
		currentListGame->number = ++gameNumber;
		currentListGame->offset = offset;
		if(1) { CopyBoard(boards[scratch], initialPosition); plyNr = 0; currentListGame->moves = PackGame(boards[scratch]); }
		lastStart = cm;
		break;
	      default:
		break;		/*  impossible  */
	    }
	    break;
	  case PGNTag:
	    lastStart = cm;
	    if ((error = GameListNewGame(&currentListGame))) {
		rewind(f);
		yyskipmoves = FALSE;
		return(error);
	    }
	    currentListGame->number = ++gameNumber;
	    currentListGame->offset = offset;
	    ParsePGNTag(yy_text, &currentListGame->gameInfo);
	    do {
		yyboardindex = 1;
		offset = yyoffset();
		cm = (ChessMove) Myylex();
		if (cm == PGNTag) {
		    ParsePGNTag(yy_text, &currentListGame->gameInfo);
		}
	    } while (cm == PGNTag || cm == Comment);
	    if(1) {
		int btm=0;
		if(currentListGame->gameInfo.fen) ParseFEN(boards[scratch], &btm, currentListGame->gameInfo.fen, FALSE);
		else CopyBoard(boards[scratch], initialPosition);
		plyNr = (btm != 0);
		currentListGame->moves = PackGame(boards[scratch]);
	    }
	    if(cm != NormalMove) break;
	  case IllegalMove:
		if(appData.testLegality) break;
	  case NormalMove:
	    /* Allow the first game to start with an unnumbered move */
	    yyskipmoves = FALSE;
	    if (lastStart == (ChessMove) 0) {
	      if ((error = GameListNewGame(&currentListGame))) {
		rewind(f);
		yyskipmoves = FALSE;
		return(error);
	      }
	      currentListGame->number = ++gameNumber;
	      currentListGame->offset = offset;
	      if(1) { CopyBoard(boards[scratch], initialPosition); plyNr = 0; currentListGame->moves = PackGame(boards[scratch]); }
	      lastStart = MoveNumberOne;
	    }
	  case WhiteCapturesEnPassant:
	  case BlackCapturesEnPassant:
	  case WhitePromotion:
	  case BlackPromotion:
	  case WhiteNonPromotion:
	  case BlackNonPromotion:
	  case WhiteKingSideCastle:
	  case WhiteQueenSideCastle:
	  case BlackKingSideCastle:
	  case BlackQueenSideCastle:
	  case WhiteKingSideCastleWild:
	  case WhiteQueenSideCastleWild:
	  case BlackKingSideCastleWild:
	  case BlackQueenSideCastleWild:
	  case WhiteHSideCastleFR:
	  case WhiteASideCastleFR:
	  case BlackHSideCastleFR:
	  case BlackASideCastleFR:
		fromX = currentMoveString[0] - AAA;
		fromY = currentMoveString[1] - ONE;
		toX = currentMoveString[2] - AAA;
		toY = currentMoveString[3] - ONE;
		plyNr++;
		ApplyMove(fromX, fromY, toX, toY, currentMoveString[4], boards[scratch]);
		if(currentListGame && currentListGame->moves) PackMove(fromX, fromY, toX, toY, boards[scratch][toY][toX]);
	    break;
        case WhiteWins: // [HGM] rescom: save last comment as result details
        case BlackWins:
        case GameIsDrawn:
        case GameUnfinished:
	    if(!currentListGame) break;
	    if (currentListGame->gameInfo.resultDetails != NULL) {
		free(currentListGame->gameInfo.resultDetails);
	    }
	    if(yy_text[0] == '{') {
		char *p;
		safeStrCpy(lastComment, yy_text+1, sizeof(lastComment)/sizeof(lastComment[0]));
		if((p = strchr(lastComment, '}'))) *p = 0;
		currentListGame->gameInfo.resultDetails = StrSave(lastComment);
	    }
	    break;
	  default:
	    break;
	}
	if(gameNumber % 1000 == 0) {
	    snprintf(buf, MSG_SIZ, _("Reading game file (%d)"), gameNumber);
	    DisplayTitle(buf); DoEvents();
	}
    }
    while (cm != (ChessMove) 0);

 if(currentListGame) {
    if(!currentListGame->moves) DisplayError("Game cache overflowed\nPosition-searching might not work properly", 0);

    if (appData.debugMode) {
	for (currentListGame = (ListGame *) gameList.head;
	     currentListGame->node.succ;
	     currentListGame = (ListGame *) currentListGame->node.succ) {

	    fprintf(debugFP, "Parsed game number %d, offset %ld:\n",
		    currentListGame->number, currentListGame->offset);
	    PrintPGNTags(debugFP, &currentListGame->gameInfo);
	}
    }
  }
    if(appData.debugMode) { GetTimeMark(&t2);printf("GameListBuild %ld msec\n", SubtractTimeMarks(&t2,&t)); }
    quickFlag = 0;
    PackGame(boards[scratch]); // for appending end-of-game marker.
    DisplayTitle("WinBoard");
    rewind(f);
    yyskipmoves = FALSE;
    return 0;
}


/* Clear an existing GameInfo structure.
 */
void
ClearGameInfo (GameInfo *gameInfo)
{
    if (gameInfo->event != NULL) {
	free(gameInfo->event);
    }
    if (gameInfo->site != NULL) {
	free(gameInfo->site);
    }
    if (gameInfo->date != NULL) {
	free(gameInfo->date);
    }
    if (gameInfo->round != NULL) {
	free(gameInfo->round);
    }
    if (gameInfo->white != NULL) {
	free(gameInfo->white);
    }
    if (gameInfo->black != NULL) {
	free(gameInfo->black);
    }
    if (gameInfo->resultDetails != NULL) {
	free(gameInfo->resultDetails);
    }
    if (gameInfo->fen != NULL) {
	free(gameInfo->fen);
    }
    if (gameInfo->timeControl != NULL) {
	free(gameInfo->timeControl);
    }
    if (gameInfo->extraTags != NULL) {
	free(gameInfo->extraTags);
    }
    if (gameInfo->variantName != NULL) {
        free(gameInfo->variantName);
    }
    if (gameInfo->outOfBook != NULL) {
        free(gameInfo->outOfBook);
    }
    GameListInitGameInfo(gameInfo);
}

/* [AS] Replaced by "dynamic" tag selection below */
char *
GameListLineOld (int number, GameInfo *gameInfo)
{
    char *event = (gameInfo->event && strcmp(gameInfo->event, "?") != 0) ?
		     gameInfo->event : gameInfo->site ? gameInfo->site : "?";
    char *white = gameInfo->white ? gameInfo->white : "?";
    char *black = gameInfo->black ? gameInfo->black : "?";
    char *date = gameInfo->date ? gameInfo->date : "?";
    int len = 10 + strlen(event) + 2 + strlen(white) + 1 +
      strlen(black) + 11 + strlen(date) + 1;
    char *ret = (char *) malloc(len);
    sprintf(ret, "%d. %s, %s-%s, %s, %s",
	    number, event, white, black, PGNResult(gameInfo->result), date);
    return ret;
}

#define MAX_FIELD_LEN   80  /* To avoid overflowing the buffer */

char *
GameListLine (int number, GameInfo * gameInfo)
{
    char buffer[2*MSG_SIZ];
    char * buf = buffer;
    char * glt = appData.gameListTags;

    buf += sprintf( buffer, "%d.", number );

    while( *glt != '\0' ) {
        *buf++ = ' ';

        switch( *glt ) {
        case GLT_EVENT:
            strncpy( buf, gameInfo->event ? gameInfo->event : "?", MAX_FIELD_LEN );
            break;
        case GLT_SITE:
            strncpy( buf, gameInfo->site ? gameInfo->site : "?", MAX_FIELD_LEN );
            break;
        case GLT_DATE:
            strncpy( buf, gameInfo->date ? gameInfo->date : "?", MAX_FIELD_LEN );
            break;
        case GLT_ROUND:
            strncpy( buf, gameInfo->round ? gameInfo->round : "?", MAX_FIELD_LEN );
            break;
        case GLT_PLAYERS:
            strncpy( buf, gameInfo->white ? gameInfo->white : "?", MAX_FIELD_LEN );
            buf[ MAX_FIELD_LEN-1 ] = '\0';
            buf += strlen( buf );
            *buf++ = '-';
            strncpy( buf, gameInfo->black ? gameInfo->black : "?", MAX_FIELD_LEN );
            break;
        case GLT_RESULT:
	    safeStrCpy( buf, PGNResult(gameInfo->result), 2*MSG_SIZ );
            break;
        case GLT_WHITE_ELO:
            if( gameInfo->whiteRating > 0 )
	      sprintf( buf,  "%d", gameInfo->whiteRating );
            else
	      safeStrCpy( buf, "?" , 2*MSG_SIZ);
            break;
        case GLT_BLACK_ELO:
            if( gameInfo->blackRating > 0 )
                sprintf( buf, "%d", gameInfo->blackRating );
            else
	      safeStrCpy( buf, "?" , 2*MSG_SIZ);
            break;
        case GLT_TIME_CONTROL:
            strncpy( buf, gameInfo->timeControl ? gameInfo->timeControl : "?", MAX_FIELD_LEN );
            break;
        case GLT_VARIANT:
            strncpy( buf, gameInfo->variantName ? gameInfo->variantName : VariantName(gameInfo->variant), MAX_FIELD_LEN );
//            strncpy( buf, VariantName(gameInfo->variant), MAX_FIELD_LEN );
            break;
        case GLT_OUT_OF_BOOK:
            strncpy( buf, gameInfo->outOfBook ? gameInfo->outOfBook : "?", MAX_FIELD_LEN );
            break;
        case GLT_RESULT_COMMENT:
            strncpy( buf, gameInfo->resultDetails ? gameInfo->resultDetails : "res?", MAX_FIELD_LEN );
            break;
        default:
            break;
        }

        buf[MAX_FIELD_LEN-1] = '\0';

        buf += strlen( buf );

        glt++;

        if( *glt != '\0' ) {
            *buf++ = ',';
        }
    }

    *buf = '\0';

    return strdup( buffer );
}

char *
GameListLineFull (int number, GameInfo * gameInfo)
{
    char * event = gameInfo->event ? gameInfo->event : "?";
    char * site = gameInfo->site ? gameInfo->site : "?";
    char * white = gameInfo->white ? gameInfo->white : "?";
    char * black = gameInfo->black ? gameInfo->black : "?";
    char * round = gameInfo->round ? gameInfo->round : "?";
    char * date = gameInfo->date ? gameInfo->date : "?";
    char * oob = gameInfo->outOfBook ? gameInfo->outOfBook : "";
    char * reason = gameInfo->resultDetails ? gameInfo->resultDetails : "";

    int len = 64 + strlen(event) + strlen(site) + strlen(white) + strlen(black) + strlen(date) + strlen(oob) + strlen(reason);

    char *ret = (char *) malloc(len);

    sprintf(ret, "%d, \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\"",
	number, event, site, round, white, black, PGNResult(gameInfo->result), reason, date, oob );

    return ret;
}
// --------------------------------------- Game-List options dialog --------------------------------------

// back-end
typedef struct {
    char id;
    char * name;
} GLT_Item;

// back-end: translation table tag id-char <-> full tag name
static GLT_Item GLT_ItemInfo[] = {
    { GLT_EVENT,      "Event" },
    { GLT_SITE,       "Site" },
    { GLT_DATE,       "Date" },
    { GLT_ROUND,      "Round" },
    { GLT_PLAYERS,    "Players" },
    { GLT_RESULT,     "Result" },
    { GLT_WHITE_ELO,  "White Rating" },
    { GLT_BLACK_ELO,  "Black Rating" },
    { GLT_TIME_CONTROL,"Time Control" },
    { GLT_VARIANT,    "Variant" },
    { GLT_OUT_OF_BOOK,PGN_OUT_OF_BOOK },
    { GLT_RESULT_COMMENT, "Result Comment" }, // [HGM] rescom
    { 0, 0 }
};

char lpUserGLT[LPUSERGLT_SIZE];

// back-end: convert the tag id-char to a full tag name
char *
GLT_FindItem (char id)
{
    char * result = 0;

    GLT_Item * list = GLT_ItemInfo;

    while( list->id != 0 ) {
        if( list->id == id ) {
            result = list->name;
            break;
        }

        list++;
    }

    return result;
}

// back-end: build the list of tag names
void
GLT_TagsToList (char *tags)
{
    char * pc = tags;

    GLT_ClearList();

    while( *pc ) {
        GLT_AddToList( GLT_FindItem(*pc) );
        pc++;
    }

    GLT_AddToList( "     --- Hidden tags ---     " );

    pc = GLT_ALL_TAGS;

    while( *pc ) {
        if( strchr( tags, *pc ) == 0 ) {
            GLT_AddToList( GLT_FindItem(*pc) );
        }
        pc++;
    }

    GLT_DeSelectList();
}

// back-end: retrieve item from dialog and translate to id-char
char
GLT_ListItemToTag (int index)
{
    char result = '\0';
    char name[MSG_SIZ];

    GLT_Item * list = GLT_ItemInfo;

    if( GLT_GetFromList(index, name) ) {
        while( list->id != 0 ) {
            if( strcmp( list->name, name ) == 0 ) {
                result = list->id;
                break;
            }

            list++;
        }
    }

    return result;
}

// back-end: add items id-chars one-by-one to temp tags string
void
GLT_ParseList ()
{
    char * pc = lpUserGLT;
    int idx = 0;
    char id;

    do {
	id = GLT_ListItemToTag( idx );
	*pc++ = id;
	idx++;
    } while( id != '\0' );
}
