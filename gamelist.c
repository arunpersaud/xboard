/*
 * gamelist.c -- Functions to manage a gamelist
 * XBoard $Id: gamelist.c,v 2.1 2003/10/27 19:21:00 mann Exp $
 *
 * Copyright 1995 Free Software Foundation, Inc.
 *
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111 USA.
 * ------------------------------------------------------------------------
 */

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


/* Variables
 */
List gameList;


/* Local function prototypes
 */
static void GameListDeleteGame P((ListGame *));
static ListGame *GameListCreate P((void));
static void GameListFree P((List *));
static int GameListNewGame P((ListGame **));

/* Delete a ListGame; implies removint it from a list.
 */
static void GameListDeleteGame(listGame)
    ListGame *listGame;
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
	ListNodeFree((ListNode *) listGame);
    }
}


/* Free the previous list of games.
 */
static void GameListFree(gameList)
    List *gameList;
{
    while (!ListEmpty(gameList))
    {
	GameListDeleteGame((ListGame *) gameList->head);
    }
}



/* Initialize a new GameInfo structure.
 */
void GameListInitGameInfo(gameInfo)
    GameInfo *gameInfo;
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
}


/* Create empty ListGame; returns ListGame or NULL, if out of memory.
 *
 * Note, that the ListGame is *not* added to any list
 */
static ListGame *GameListCreate()

{
    ListGame *listGame;

    if ((listGame = (ListGame *) ListNodeCreate(sizeof(*listGame)))) {
	GameListInitGameInfo(&listGame->gameInfo);
    }
    return(listGame);
}


/* Creates a new game for the gamelist.
 */
static int GameListNewGame(listGamePtr)
     ListGame **listGamePtr;
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
int GameListBuild(f)
    FILE *f;
{
    ChessMove cm, lastStart;
    int gameNumber;
    ListGame *currentListGame = NULL;
    int error;
    int offset;

    GameListFree(&gameList);
    yynewfile(f);
    gameNumber = 0;

    lastStart = (ChessMove) 0;
    yyskipmoves = FALSE;
    do {
        yyboardindex = 0;
	offset = yyoffset();
	cm = (ChessMove) yylex();
	switch (cm) {
	  case GNUChessGame:
	    if ((error = GameListNewGame(&currentListGame))) {
		rewind(f);
		yyskipmoves = FALSE;
		return(error);
	    }
	    currentListGame->number = ++gameNumber;
	    currentListGame->offset = offset;
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
		cm = (ChessMove) yylex();
		if (cm == PGNTag) {
		    ParsePGNTag(yy_text, &currentListGame->gameInfo);
		}
	    } while (cm == PGNTag || cm == Comment);
	    break;
	  case NormalMove:
	    /* Allow the first game to start with an unnumbered move */
	    yyskipmoves = TRUE;
	    if (lastStart == (ChessMove) 0) {
	      if ((error = GameListNewGame(&currentListGame))) {
		rewind(f);
		yyskipmoves = FALSE;
		return(error);
	      }
	      currentListGame->number = ++gameNumber;
	      currentListGame->offset = offset;
	      lastStart = MoveNumberOne;
	    }
	    break;
	  default:
	    break;
	}
    }
    while (cm != (ChessMove) 0);


    if (appData.debugMode) {
	for (currentListGame = (ListGame *) gameList.head;
	     currentListGame->node.succ;
	     currentListGame = (ListGame *) currentListGame->node.succ) {

	    fprintf(debugFP, "Parsed game number %d, offset %ld:\n",
		    currentListGame->number, currentListGame->offset);
	    PrintPGNTags(debugFP, &currentListGame->gameInfo);
	}
    }

    rewind(f);
    yyskipmoves = FALSE;
    return 0;
}


/* Clear an existing GameInfo structure.
 */
void ClearGameInfo(gameInfo)
    GameInfo *gameInfo;
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

    GameListInitGameInfo(gameInfo);
}

char *
GameListLine(number, gameInfo)
     int number;
     GameInfo *gameInfo;
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

