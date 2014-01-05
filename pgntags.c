/*
 * pgntags.c -- Functions to manage PGN tags
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


/* Parse PGN tags; returns 0 for success or error number
 */
int
ParsePGNTag (char *tag, GameInfo *gameInfo)
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


/* Print game info */
void
PrintPGNTags (FILE *fp, GameInfo *gameInfo)
{
    fprintf(fp, "[Event \"%s\"]\n", gameInfo->event ? gameInfo->event : "?");
    fprintf(fp, "[Site \"%s\"]\n", gameInfo->site ? gameInfo->site : "?");
    fprintf(fp, "[Date \"%s\"]\n", gameInfo->date ? gameInfo->date : "?");
    fprintf(fp, "[Round \"%s\"]\n", gameInfo->round ? gameInfo->round : "-");
    fprintf(fp, "[White \"%s\"]\n", gameInfo->white ? gameInfo->white : "?");
    fprintf(fp, "[Black \"%s\"]\n", gameInfo->black ? gameInfo->black : "?");
    fprintf(fp, "[Result \"%s\"]\n", PGNResult(gameInfo->result));
    if (gameInfo->whiteRating >= 0)
	fprintf(fp, "[WhiteElo \"%d\"]\n", gameInfo->whiteRating);
    if (gameInfo->blackRating >= 0)
	fprintf(fp, "[BlackElo \"%d\"]\n", gameInfo->blackRating);
    if (gameInfo->timeControl)
	fprintf(fp, "[TimeControl \"%s\"]\n", gameInfo->timeControl);
    if (gameInfo->variant != VariantNormal)
        fprintf(fp, "[Variant \"%s\"]\n", VariantName(gameInfo->variant));
    if (gameInfo->extraTags)
	fputs(gameInfo->extraTags, fp);
}


/* Return a non-static buffer with a games info.
 */
char *
PGNTags (GameInfo *gameInfo)
{
    size_t len;
    char *buf;
    char *p;

    // First calculate the needed buffer size.
    // Then we don't have to check the buffer size later.
    len = 12 + 11 + 11 + 12 + 12 + 12 + 25 + 1; // The first 7 tags
    if (gameInfo->event) len += strlen(gameInfo->event);
    if (gameInfo->site)  len += strlen(gameInfo->site);
    if (gameInfo->date)  len += strlen(gameInfo->date);
    if (gameInfo->round) len += strlen(gameInfo->round);
    if (gameInfo->white) len += strlen(gameInfo->white);
    if (gameInfo->black) len += strlen(gameInfo->black);
    if (gameInfo->whiteRating >= 0) len += 40;
    if (gameInfo->blackRating >= 0) len += 40;
    if (gameInfo->timeControl) len += strlen(gameInfo->timeControl) + 20;
    if (gameInfo->variant != VariantNormal) len += 50;
    if (gameInfo->extraTags) len += strlen(gameInfo->extraTags);

    buf = malloc(len);
    if (!buf)
	return 0;

    p = buf;
    p += sprintf(p, "[Event \"%s\"]\n", gameInfo->event ? gameInfo->event : "?");
    p += sprintf(p, "[Site \"%s\"]\n", gameInfo->site ? gameInfo->site : "?");
    p += sprintf(p, "[Date \"%s\"]\n", gameInfo->date ? gameInfo->date : "?");
    p += sprintf(p, "[Round \"%s\"]\n", gameInfo->round ? gameInfo->round : "-");
    p += sprintf(p, "[White \"%s\"]\n", gameInfo->white ? gameInfo->white : "?");
    p += sprintf(p, "[Black \"%s\"]\n", gameInfo->black ? gameInfo->black : "?");
    p += sprintf(p, "[Result \"%s\"]\n", PGNResult(gameInfo->result));
    if (gameInfo->whiteRating >= 0)
	p += sprintf(p, "[WhiteElo \"%d\"]\n", gameInfo->whiteRating);
    if (gameInfo->blackRating >= 0)
	p += sprintf(p, "[BlackElo \"%d\"]\n", gameInfo->blackRating);
    if (gameInfo->timeControl)
	p += sprintf(p, "[TimeControl \"%s\"]\n", gameInfo->timeControl);
    if (gameInfo->variant != VariantNormal)
        p += sprintf(p, "[Variant \"%s\"]\n", VariantName(gameInfo->variant));
    if (gameInfo->extraTags)
	strcpy(p, gameInfo->extraTags);
    return buf;
}


/* Returns pointer to a static string with a result.
 */
char *
PGNResult (ChessMove result)
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
ReplaceTags (char *tags, GameInfo *gameInfo)
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
