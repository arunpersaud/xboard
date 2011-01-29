/*
 * pgntags.c -- Functions to manage PGN tags
 *
 * Copyright 1995, 2009, 2010, 2011 Free Software Foundation, Inc.
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
 * along with this program. If not, see http://www.gnu.org/licenses/.  *
 *
 * ------------------------------------------------------------------------
 *
 * This file could well be a part of backend.c, but I prefer it this
 * way.
 */

#include "config.h"

#include <stdio.h>
#include <errno.h>
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

#include "common.h"
#include "frontend.h"
#include "backend.h"
#include "parser.h"

static char *PGNTagsStatic P((GameInfo *));



/* Parse PGN tags; returns 0 for success or error number
 */
int ParsePGNTag(tag, gameInfo)
    char *tag;
    GameInfo *gameInfo;
{
    char *name, *value, *p, *oldTags;
    int len;
    int success;

    name = tag;
    while (!isalpha(*name) && !isdigit(*name)) {
	name++;
    }
    p = name;
    while (*p != ' ' && *p != '\t' && *p != '\n') {
	p++;
    }
    *p = NULLCHAR;
    value = strchr(p + 1, '"') + 1;
    p = strrchr(value, '"');
    *p = NULLCHAR;

    if (StrCaseCmp(name, "Event") == 0) {
	success = StrSavePtr(value, &gameInfo->event) != NULL;
    } else if (StrCaseCmp(name, "Site") == 0) {
	success = StrSavePtr(value, &gameInfo->site) != NULL;
    } else if (StrCaseCmp(name, "Date") == 0) {
	success = StrSavePtr(value, &gameInfo->date) != NULL;
    } else if (StrCaseCmp(name, "Round") == 0) {
	success = StrSavePtr(value, &gameInfo->round) != NULL;
    } else if (StrCaseCmp(name, "White") == 0) {
	success = StrSavePtr(value, &gameInfo->white) != NULL;
    } else if (StrCaseCmp(name, "Black") == 0) {
	success = StrSavePtr(value, &gameInfo->black) != NULL;
    }
    /* Fold together the various ways of denoting White/Black rating */
    else if ((StrCaseCmp(name, "WhiteElo")==0) ||
	     (StrCaseCmp(name, "WhiteUSCF")==0) ) {
      success = TRUE;
      gameInfo->whiteRating = atoi( value );
    } else if ((StrCaseCmp(name, "BlackElo")==0) ||
	       (StrCaseCmp(name, "BlackUSCF")==0)) {
      success = TRUE;
      gameInfo->blackRating = atoi( value );
    }
    else if (StrCaseCmp(name, "Result") == 0) {
	if (strcmp(value, "1-0") == 0)
	    gameInfo->result = WhiteWins;
	else if (strcmp(value, "0-1") == 0)
	    gameInfo->result = BlackWins;
	else if (strcmp(value, "1/2-1/2") == 0)
	    gameInfo->result = GameIsDrawn;
	else
	    gameInfo->result = GameUnfinished;
	success = TRUE;
    } else if (StrCaseCmp(name, "TimeControl") == 0) {
//	int tc, mps, inc = -1;
//	if(sscanf(value, "%d/%d", &mps, &tc) == 2 || )
	success = StrSavePtr(value, &gameInfo->timeControl) != NULL;
    } else if (StrCaseCmp(name, "FEN") == 0) {
	success = StrSavePtr(value, &gameInfo->fen) != NULL;
    } else if (StrCaseCmp(name, "SetUp") == 0) {
	/* ignore on input; presence of FEN governs */
	success = TRUE;
    } else if (StrCaseCmp(name, "Variant") == 0) {
        /* xboard-defined extension */
        gameInfo->variant = StringToVariant(value);
	success = TRUE;
    } else if (StrCaseCmp(name, PGN_OUT_OF_BOOK) == 0) {
        /* [AS] Out of book annotation */
        success = StrSavePtr(value, &gameInfo->outOfBook) != NULL;
    } else {
	if (gameInfo->extraTags == NULL) {
	    oldTags = "";
	} else {
	    oldTags = gameInfo->extraTags;
	}
	/* Buffer size includes 7 bytes of space for [ ""]\n\0 */
	len = strlen(oldTags) + strlen(value) + strlen(name) + 7;
	if ((p = (char *) malloc(len))  !=  NULL) {
	    sprintf(p, "%s[%s \"%s\"]\n", oldTags, name, value);
	    if (gameInfo->extraTags != NULL) free(gameInfo->extraTags);
	    gameInfo->extraTags = p;
	    success = TRUE;
	} else {
	    success = FALSE;
	}
    }
    return(success ? 0 : ENOMEM);
}



/* Return a static buffer with a game's data.
 */
static char *PGNTagsStatic(gameInfo)
    GameInfo *gameInfo;
{
    static char buf[8192];
    char buf1[MSG_SIZ];

    buf[0] = NULLCHAR;

    snprintf(buf1, MSG_SIZ, "[Event \"%s\"]\n",
	    gameInfo->event ? gameInfo->event : "?");
    strcat(buf, buf1);
    snprintf(buf1, MSG_SIZ, "[Site \"%s\"]\n",
	    gameInfo->site ? gameInfo->site : "?");
    strcat(buf, buf1);
    snprintf(buf1, MSG_SIZ, "[Date \"%s\"]\n",
	    gameInfo->date ? gameInfo->date : "?");
    strcat(buf, buf1);
    snprintf(buf1, MSG_SIZ, "[Round \"%s\"]\n",
	    gameInfo->round ? gameInfo->round : "-");
    strcat(buf, buf1);
    snprintf(buf1, MSG_SIZ, "[White \"%s\"]\n",
	    gameInfo->white ? gameInfo->white : "?");
    strcat(buf, buf1);
    snprintf(buf1, MSG_SIZ, "[Black \"%s\"]\n",
	    gameInfo->black ? gameInfo->black : "?");
    strcat(buf, buf1);
    snprintf(buf1, MSG_SIZ, "[Result \"%s\"]\n", PGNResult(gameInfo->result));
    strcat(buf, buf1);

    if (gameInfo->whiteRating >= 0 ) {
	snprintf(buf1, MSG_SIZ, "[WhiteElo \"%d\"]\n", gameInfo->whiteRating );
	strcat(buf, buf1);
    }
    if ( gameInfo->blackRating >= 0 ) {
	snprintf(buf1, MSG_SIZ, "[BlackElo \"%d\"]\n", gameInfo->blackRating );
	strcat(buf, buf1);
    }
    if (gameInfo->timeControl != NULL) {
	snprintf(buf1, MSG_SIZ, "[TimeControl \"%s\"]\n", gameInfo->timeControl);
	strcat(buf, buf1);
    }
    if (gameInfo->variant != VariantNormal) {
        snprintf(buf1, MSG_SIZ, "[Variant \"%s\"]\n", VariantName(gameInfo->variant));
	strcat(buf, buf1);
    }
    if (gameInfo->extraTags != NULL) {
	strcat(buf, gameInfo->extraTags);
    }
    return buf;
}



/* Print game info
 */
void PrintPGNTags(fp, gameInfo)
     FILE *fp;
     GameInfo *gameInfo;
{
    fprintf(fp, "%s", PGNTagsStatic(gameInfo));
}


/* Return a non-static buffer with a games info.
 */
char *PGNTags(gameInfo)
    GameInfo *gameInfo;
{
    return StrSave(PGNTagsStatic(gameInfo));
}


/* Returns pointer to a static string with a result.
 */
char *PGNResult(result)
     ChessMove result;
{
    switch (result) {
      case GameUnfinished:
      default:
	return "*";
      case WhiteWins:
	return "1-0";
      case BlackWins:
	return "0-1";
      case GameIsDrawn:
	return "1/2-1/2";
    }
}

/* Returns 0 for success, nonzero for error */
int
ReplaceTags(tags, gameInfo)
     char *tags;
     GameInfo *gameInfo;
{
    ChessMove moveType;
    int err;

    ClearGameInfo(gameInfo);
    yynewstr(tags);
    for (;;) {
	yyboardindex = 0;
	moveType = (ChessMove) Myylex();
	if (moveType == (ChessMove) 0) {
	    break;
	} else if (moveType == PGNTag) {
	    err = ParsePGNTag(yy_text, gameInfo);
	    if (err != 0) return err;
	}
    }
    /* just one problem...if there is a result in the new tags,
     * DisplayMove() won't ever show it because ClearGameInfo() set
     * gameInfo->resultDetails to NULL. So we must plug something in if there
     * is a result.
     */
    if (gameInfo->result != GameUnfinished) {
      if (gameInfo->resultDetails) free(gameInfo->resultDetails);
      gameInfo->resultDetails = strdup("");
    }
    return 0;
}
