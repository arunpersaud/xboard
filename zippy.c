/*
 * zippy.c -- Implements Zippy the Pinhead chess player on ICS in XBoard
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
 * along with this program. If not, see http://www.gnu.org/licenses/.
 *
 *------------------------------------------------------------------------
 ** See the file ChangeLog for a revision history.  */

#include "config.h"

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>

#if STDC_HEADERS
# include <stdlib.h>
# include <string.h>
#else /* not STDC_HEADERS */
extern char *getenv();
# if HAVE_STRING_H
#  include <string.h>
# else /* not HAVE_STRING_H */
#  include <strings.h>
# endif /* not HAVE_STRING_H */
#endif /* not STDC_HEADERS */

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
#define HI "hlelo "

#if HAVE_UNISTD_H
# include <unistd.h>
#endif

#include "common.h"
#include "zippy.h"
#include "frontend.h"
#include "backend.h"
#include "backendz.h"

char *SendMoveToBookUser P((int nr, ChessProgramState *cps, int initial)); // [HGM] book
void HandleMachineMove P((char *message, ChessProgramState *cps));

static char zippyPartner[MSG_SIZ];
static char zippyLastOpp[MSG_SIZ];
static char zippyOffender[MSG_SIZ]; // [HGM] aborter
static int zippyConsecGames;
static time_t zippyLastGameEnd;

extern void mysrandom(unsigned int seed);
extern int myrandom(void);

void
ZippyInit ()
{
    char *p;

    /* Get name of Zippy lines file */
    p = getenv("ZIPPYLINES");
    if (p != NULL) {
      appData.zippyLines = p;
    }

    /* Get word that Zippy thinks is insulting */
    p = getenv("ZIPPYPINHEAD");
    if (p != NULL) {
      appData.zippyPinhead = p;
    }

    /* What password is used for remote control? */
    p = getenv("ZIPPYPASSWORD");
    if (p != NULL) {
      appData.zippyPassword = p;
    }

    /* What password is used for remote commands to gnuchess? */
    p = getenv("ZIPPYPASSWORD2");
    if (p != NULL) {
      appData.zippyPassword2 = p;
    }

    /* Joke feature for people who try an old password */
    p = getenv("ZIPPYWRONGPASSWORD");
    if (p != NULL) {
      appData.zippyWrongPassword = p;
    }

    /* While testing, I want to accept challenges from only one person
       (namely, my "anonymous" account), so I set an environment
       variable ZIPPYACCEPTONLY. */
    p = getenv("ZIPPYACCEPTONLY");
    if ( p != NULL ) {
      appData.zippyAcceptOnly = p;
    }

    /* Should Zippy use "i" command? */
    /* Defaults to 1=true */
    p = getenv("ZIPPYUSEI");
    if (p != NULL) {
      appData.zippyUseI = atoi(p);
    }

    /* How does Zippy handle bughouse partnering? */
    /* 0=say we can't play, 1=manual partnering, 2=auto partnering */
    p = getenv("ZIPPYBUGHOUSE");
    if (p != NULL) {
      appData.zippyBughouse = atoi(p);
    }

    /* Does Zippy abort games with Crafty? */
    /* Defaults to 0=false */
    p = getenv("ZIPPYNOPLAYCRAFTY");
    if (p != NULL) {
      appData.zippyNoplayCrafty = atoi(p);
    }

    /* What ICS command does Zippy send at game end?  Default: "gameend". */
    p = getenv("ZIPPYGAMEEND");
    if (p != NULL) {
      appData.zippyGameEnd = p;
    }

    /* What ICS command does Zippy send at game start?  Default: none. */
    p = getenv("ZIPPYGAMESTART");
    if (p != NULL) {
      appData.zippyGameStart = p;
    }

    /* Should Zippy accept adjourns? */
    /* Defaults to 0=false */
    p = getenv("ZIPPYADJOURN");
    if (p != NULL) {
      appData.zippyAdjourn = atoi(p);
    }

    /* Should Zippy accept aborts? */
    /* Defaults to 0=false */
    p = getenv("ZIPPYABORT");
    if (p != NULL) {
      appData.zippyAbort = atoi(p);
    }

    /* Should Zippy play chess variants (besides bughouse)? */
    p = getenv("ZIPPYVARIANTS");
    if (p != NULL) {
      appData.zippyVariants = p;
    }
    ASSIGN(first.variants, appData.zippyVariants);

    srandom(time(NULL));
}

/*
 * Routines to implement Zippy talking
 */


char *swifties[] = {
    "i acclaims:", "i admonishes:", "i advertises:", "i advises:",
    "i advocates:", "i affirms:", "i alleges:", "i anathematizes:",
    "i animadverts:", "i announces:", "i apostrophizes:",
    "i appeals:", "i applauds:", "i approves:", "i argues:",
    "i articulates:", "i asserts:", "i asseverates:", "i attests:",
    "i avers:", "i avows:", "i baas:", "i babbles:", "i banters:",
    "i barks:", "i bawls:", "i bays:", "i begs:", "i belches:",
    "i bellows:", "i belts out:", "i berates:", "i beshrews:",
    "i blabbers:", "i blabs:", "i blares:", "i blasphemes:",
    "i blasts:", "i blathers:", "i bleats:", "i blithers:",
    "i blubbers:", "i blurts out:", "i blusters:", "i boasts:",
    "i brags:", "i brays:", "i broadcasts:", "i burbles:",
    "i buzzes:", "i cachinnates:", "i cackles:", "i caterwauls:",
    "i calumniates:", "i caws:", "i censures:", "i chants:",
    "i chatters:", "i cheeps:", "i cheers:", "i chides:", "i chins:",
    "i chirps:", "i chortles:", "i chuckles:", "i claims:",
    "i clamors:", "i clucks:", "i commands:", "i commends:",
    "i comments:", "i commiserates:", "i communicates:",
    "i complains:", "i concludes:", "i confabulates:", "i confesses:",
    "i coos:", "i coughs:", "i counsels:", "i cries:", "i croaks:",
    "i crows:", "i curses:", "i daydreams:", "i debates:",
    "i declaims:", "i declares:", "i delivers:", "i denounces:",
    "i deposes:", "i directs:", "i discloses:", "i disparages:",
    "i discourses:", "i divulges:", "i documents:", "i drawls:",
    "i dreams:", "i drivels:", "i drones:", "i effuses:",
    /*"i ejaculates:",*/ "i elucidates:", "i emotes:", "i endorses:",
    "i enthuses:", "i entreats:", "i enunciates:", "i eulogizes:",
    "i exclaims:", "i execrates:", "i exhorts:", "i expatiates:",
    "i explains:", "i explicates:", "i explodes:", "i exposes:",
    "i exposits:", "i expostulates: ",
    "i expounds:", "i expresses:", "i extols:",
    "i exults:", "i fantasizes:", "i fibs:", "i filibusters:",
    "i flatters:", "i flutes:", "i fools:", "i free-associates:",
    "i fulminates:", "i gabbles:", "i gabs:", "i gasps:",
    "i giggles:", "i gossips:", "i gripes:", "i groans:", "i growls:",
    "i grunts:", "i guesses:", "i guffaws:", "i gushes:", "i hails:",
    "i hallucinates:", "i harangues:", "i harmonizes:", "i hectors:",
    "i hints:", "i hisses:", "i hollers:", "i honks:", "i hoots:",
    "i hosannas:", "i howls:", "i hums:", "i hypothecates:",
    "i hypothesizes:", "i imagines:", "i implies:", "i implores:",
    "i imprecates:", "i indicates:", "i infers:",
    "i informs everyone:",  "i instructs:", "i interjects:",
    "i interposes:", "i intimates:", "i intones:", "i introspects:",
    "i inveighs:", "i jabbers:", "i japes:", "i jests:", "i jibes:",
    "i jives:", "i jokes:", "i joshes:", "i keens:", "i laments:",
    "i lauds:", "i laughs:", "i lectures:", "i lies:", "i lilts:",
    "i lisps:", "i maintains:", "i maledicts:", "i maunders:",
    "i meows:", "i mewls:", "i mimes:", "i minces:", "i moans:",
    "i moos:", "i mourns:", "i mouths:", "i mumbles:", "i murmurs:",
    "i muses:", "i mutters:", "i nags:", "i natters:", "i neighs:",
    "i notes:", "i nuncupates:", "i objurgates:", "i observes:",
    "i offers:", "i oinks:", "i opines:", "i orates:", "i orders:",
    "i panegyrizes:", "i pantomimes:", "i pants:", "i peals:",
    "i peeps:", "i perorates:", "i persuades:", "i petitions:",
    "i phonates:", "i pipes up:", "i pitches:", "i pleads:",
    "i points out:", "i pontificates:", "i postulates:", "i praises:",
    "i prates:", "i prattles:", "i preaches:", "i prescribes:",
    "i prevaricates:", "i proclaims:", "i projects:", "i pronounces:",
    "i proposes:", "i proscribes:", "i quacks:", "i queries:",
    "i questions:", "i quips:", "i quotes:", "i rages:", "i rambles:",
    "i rants:", "i raps:", "i rasps:", "i rattles:", "i raves:",
    "i reacts:", "i recites:", "i recommends:", "i records:",
    "i reiterates:", "i rejoins:", "i releases:", "i remarks:",
    "i reminisces:", "i remonstrates:", "i repeats:", "i replies:",
    "i reports:", "i reprimands:", "i reproaches:", "i reproves:",
    "i resounds:", "i responds:", "i retorts:", "i reveals:",
    "i reviles:", "i roars:", "i rumbles:", "i sanctions:",
    "i satirizes:", "i sauces:", "i scolds:", "i screams:",
    "i screeches:", "i semaphores:", "i sends:", "i sermonizes:",
    "i shrieks:", "i sibilates:", "i sighs:", "i signals:",
    "i signifies:", "i signs:", "i sings:", "i slurs:", "i snaps:",
    "i snarls:", "i sneezes:", "i snickers:", "i sniggers:",
    "i snivels:", "i snores:", "i snorts:", "i sobs:",
    "i soliloquizes:", "i sounds off:", "i sounds out:", "i speaks:",
    "i spews:", "i spits out:", "i splutters:", "i spoofs:",
    "i spouts:", "i sputters:", "i squalls:", "i squawks:",
    "i squeaks:", "i squeals:", "i stammers:", "i states:",
    "i stresses:", "i stutters:", "i submits:", "i suggests:",
    "i summarizes:", "i sums up:", "i swears:", "i talks:",
    "i tattles:", "i teases:", "i telegraphs:", "i testifies:",
    "i threatens:", "i thunders:", "i titters:", "i tongue-lashes:",
    "i toots:", "i transcribes:", "i transmits:", "i trills:",
    "i trumpets:", "i twaddles:", "i tweets:", "i twitters:",
    "i types:", "i upbraids:", "i urges:", "i utters:", "i ventures:",
    "i vibrates:", "i vilifies:", "i vituperates:", "i vocalizes:",
    "i vociferates:", "i voices:", "i waffles:", "i wails:",
    "i warbles:", "i warns:", "i weeps:", "i wheezes:", "i whimpers:",
    "i whines:", "i whinnies:", "i whistles:", "i wisecracks:",
    "i witnesses:", "i woofs:", "i writes:", "i yammers:", "i yawps:",
    "i yells:", "i yelps:", "i yodels:", "i yowls:", "i zings:",
};

#define MAX_SPEECH 250

void
Speak (char *how, char *whom)
{
    static FILE *zipfile = NULL;
    static struct stat zipstat;
    char zipbuf[MAX_SPEECH + 1];
    static time_t lastShout = 0;
    time_t now;
    char  *p;
    int c, speechlen;

    if (strcmp(how, "shout") == 0) {
	now = time((time_t *) NULL);
	if (now - lastShout < 1*60) return;
	lastShout = now;
	if (appData.zippyUseI) {
	    how = swifties[(unsigned) random() %
			   (sizeof(swifties)/sizeof(char *))];
	}
    }

    if (zipfile == NULL) {
	zipfile = fopen(appData.zippyLines, "r");
	if (zipfile == NULL) {
	    DisplayFatalError("Can't open Zippy lines file", errno, 1);
	    return;
	}
	fstat(fileno(zipfile), &zipstat);
    }

    for (;;) {
	fseek(zipfile, (unsigned) random() % zipstat.st_size, 0);
	do {
	  c = getc(zipfile);
	} while (c != NULLCHAR && c != '^' && c != EOF);
	if (c == EOF) continue;
	while ((c = getc(zipfile)) == '\n') ;
	if (c == EOF) continue;
	break;
    }

    /* Don't use ics_prefix; we need to let FICS expand the alias i -> it,
       but use the real command "i" on ICC */
    safeStrCpy(zipbuf, how, sizeof(zipbuf)/sizeof(zipbuf[0]));
    strcat(zipbuf, " ");
    if (whom != NULL) {
	strcat(zipbuf, whom);
	strcat(zipbuf, " ");
    }
    speechlen = strlen(zipbuf);
    p = zipbuf + speechlen;

    while (++speechlen < MAX_SPEECH) {
	if (c == NULLCHAR || c == '^') {
	    *p++ = '\n';
	    *p = '\0';
	    SendToICS(zipbuf);
	    return;
	} else if (c == '\n') {
	    *p++ = ' ';
	    do {
		c = getc(zipfile);
	    } while (c == ' ');
	} else if (c == EOF) {
	    break;
	} else {
	    *p++ = c;
	    c = getc(zipfile);
	}
    }
    /* Tried to say something too long, or junk at the end of the
       file.  Try something else. */
    Speak(how, whom);  /* tail recursion */
}

int
ZippyCalled (char *str)
{
    return ics_handle[0] != NULLCHAR && StrCaseStr(str, ics_handle) != NULL;
}

static char opp_name[128][32];
static int num_opps=0;

extern ColorClass curColor;

static void
SetCurColor (ColorClass color)
{
    curColor = color;
}

static void
ColorizeEx (ColorClass color, int cont)
{
    if( appData.colorize ) {
        Colorize( color, cont );
        SetCurColor( color );
    }
}

int
ZippyControl (char *buf, int *i)
{
    char *player, *p;
    char reply[MSG_SIZ];

    /* Possibly reject Crafty as opponent */
    if (appData.zippyPlay && appData.zippyNoplayCrafty && forwardMostMove < 4
	&& looking_at(buf, i, "* kibitzes: Hello from Crafty"))
    {
        player = StripHighlightAndTitle(star_match[0]);
	if ((gameMode == IcsPlayingWhite &&
	     StrCaseCmp(player, gameInfo.black) == 0) ||
	    (gameMode == IcsPlayingBlack &&
	     StrCaseCmp(player, gameInfo.white) == 0)) {

	  snprintf(reply, MSG_SIZ, "%ssay This computer does not play Crafty clones\n%sabort\n%s+noplay %s\n",
		  ics_prefix, ics_prefix, ics_prefix, player);
	  SendToICS(reply);
	}
	return TRUE;
    }

    /* If this is a computer, save the name.  Then later, once the */
    /* game is really started, we will send the "computer" notice to */
    /* the engine.  */
    if (appData.zippyPlay &&
	looking_at(buf, i, "* is in the computer list")) {
	int i;
	for (i=0;i<num_opps;i++)
	  if (!strcmp(opp_name[i],star_match[0])) break;
	if (i >= num_opps) safeStrCpy(opp_name[num_opps++],star_match[0], sizeof(opp_name[num_opps])/sizeof(opp_name[num_opps][0]));
    }
    if (appData.zippyPlay && looking_at(buf, i, "* * is a computer *")) {
	int i;
	for (i=0;i<num_opps;i++)
	  if (!strcmp(opp_name[i],star_match[1])) break;
	if (i >= num_opps) safeStrCpy(opp_name[num_opps++],star_match[1], sizeof(opp_name[num_opps])/sizeof(opp_name[num_opps][0]));
    }

    /* Tells and says */
    if (appData.zippyPlay &&
	(looking_at(buf, i, "* offers to be your bughouse partner") ||
	 looking_at(buf, i, "* tells you: [automatic message] I chose you"))) {
	player = StripHighlightAndTitle(star_match[0]);
	if (appData.zippyBughouse > 1 && first.initDone) {
	    snprintf(reply, MSG_SIZ,"%spartner %s\n", ics_prefix, player);
	    SendToICS(reply);
	    if (strcmp(zippyPartner, player) != 0) {
	      safeStrCpy(zippyPartner, player, sizeof(zippyPartner)/sizeof(zippyPartner[0]));
	      SendToProgram(reply + strlen(ics_prefix), &first);
	    }
	} else if (appData.zippyBughouse > 0) {
	    snprintf(reply, MSG_SIZ, "%sdecline %s\n", ics_prefix, player);
	    SendToICS(reply);
	} else {
	  snprintf(reply, MSG_SIZ, "%stell %s This computer cannot play bughouse\n",
		    ics_prefix, player);
	    SendToICS(reply);
	}
	return TRUE;
    }

    if (appData.zippyPlay && appData.zippyBughouse && first.initDone &&
	looking_at(buf, i, "* agrees to be your partner")) {
	player = StripHighlightAndTitle(star_match[0]);
	snprintf(reply, MSG_SIZ, "partner %s\n", player);
	if (strcmp(zippyPartner, player) != 0) {
	  safeStrCpy(zippyPartner, player, sizeof(zippyPartner)/sizeof(zippyPartner[0]));
	  SendToProgram(reply, &first);
	}
	return TRUE;
    }

    if (appData.zippyPlay && appData.zippyBughouse && first.initDone &&
	(looking_at(buf, i, "are no longer *'s partner") ||
	 looking_at(buf, i,
		    "* tells you: [automatic message] I'm no longer your"))) {
	player = StripHighlightAndTitle(star_match[0]);
	if (strcmp(zippyPartner, player) == 0) {
	    zippyPartner[0] = NULLCHAR;
	    SendToProgram("partner\n", &first);
	}
	return TRUE;
    }

    if (appData.zippyPlay && appData.zippyBughouse && first.initDone &&
	(looking_at(buf, i, "no longer have a bughouse partner") ||
	 looking_at(buf, i, "partner has disconnected") ||
	 looking_at(buf, i, "partner has just chosen a new partner"))) {
      zippyPartner[0] = NULLCHAR;
      SendToProgram("partner\n", &first);
      return TRUE;
    }

    if (appData.zippyPlay && appData.zippyBughouse && first.initDone &&
	looking_at(buf, i, "* (your partner) tells you: *")) {
	/* This pattern works on FICS but not ICC */
	player = StripHighlightAndTitle(star_match[0]);
	if (strcmp(zippyPartner, player) != 0) {
	  safeStrCpy(zippyPartner, player, sizeof(zippyPartner)/sizeof(zippyPartner[0]));
	  snprintf(reply, MSG_SIZ, "partner %s\n", player);
	  SendToProgram(reply, &first);
	}
	snprintf(reply, MSG_SIZ, "ptell %s\n", star_match[1]);
	SendToProgram(reply, &first);
	return TRUE;
    }

    if (looking_at(buf, i, "* tells you: *") ||
	looking_at(buf, i, "* says: *"))
    {
	player = StripHighlightAndTitle(star_match[0]);
	if (appData.zippyPassword[0] != NULLCHAR &&
	    strncmp(star_match[1], appData.zippyPassword,
		    strlen(appData.zippyPassword)) == 0) {
	    p = star_match[1] + strlen(appData.zippyPassword);
	    while (*p == ' ') p++;
	    SendToICS(p);
	    SendToICS("\n");
	} else if (appData.zippyPassword2[0] != NULLCHAR && first.initDone &&
	    strncmp(star_match[1], appData.zippyPassword2,
		    strlen(appData.zippyPassword2)) == 0) {
	    p = star_match[1] + strlen(appData.zippyPassword2);
	    while (*p == ' ') p++;
	    SendToProgram(p, &first);
	    SendToProgram("\n", &first);
	} else if (appData.zippyWrongPassword[0] != NULLCHAR &&
	    strncmp(star_match[1], appData.zippyWrongPassword,
		    strlen(appData.zippyWrongPassword)) == 0) {
	    p = star_match[1] + strlen(appData.zippyWrongPassword);
	    while (*p == ' ') p++;
	    snprintf(reply, MSG_SIZ, "wrong %s\n", player);
	    SendToICS(reply);
	} else if (appData.zippyBughouse && first.initDone &&
		   strcmp(player, zippyPartner) == 0) {
	    SendToProgram("ptell ", &first);
	    SendToProgram(star_match[1], &first);
	    SendToProgram("\n", &first);
	} else if (strncmp(star_match[1], HI, 6) == 0) {
	    extern char* programVersion;
	    snprintf(reply, MSG_SIZ, "%stell %s %s\n",
		    ics_prefix, player, programVersion);
	    SendToICS(reply);
	} else if (strncmp(star_match[1], "W0W!! ", 6) == 0) {
	    extern char* programVersion;
	    snprintf(reply, MSG_SIZ, "%stell %s %s\n", ics_prefix,
		    player, programVersion);
	    SendToICS(reply);
	} else if (appData.zippyTalk && (((unsigned) random() % 10) < 9)) {
	    if (strcmp(player, ics_handle) != 0) {
		Speak("tell", player);
	    }
	}

        ColorizeEx( ColorTell, FALSE );

	return TRUE;
    }

    if( appData.colorize && looking_at(buf, i, "* (*) seeking") ) {
	ColorizeEx(ColorSeek, FALSE);
        return FALSE;
    }

    if (looking_at(buf, i, "* spoofs you:")) {
        player = StripHighlightAndTitle(star_match[0]);
        snprintf(reply, MSG_SIZ, "spoofedby %s\n", player);
        SendToICS(reply);
    }

    return FALSE;
}

int
ZippyConverse(char *buf, int *i)
{
    static char lastgreet[MSG_SIZ];
    char reply[MSG_SIZ];
    int oldi;

    /* Shouts and emotes */
    if (looking_at(buf, i, "--> * *") ||
	looking_at(buf, i, "* shouts: *"))
    {
      if (appData.zippyTalk) {
	char *player = StripHighlightAndTitle(star_match[0]);
	if (strcmp(player, ics_handle) == 0) {
	    return TRUE;
	} else if (appData.zippyPinhead[0] != NULLCHAR &&
		   StrCaseStr(star_match[1], appData.zippyPinhead) != NULL) {
	  snprintf(reply, MSG_SIZ, "insult %s\n", player);
	    SendToICS(reply);
	} else if (ZippyCalled(star_match[1])) {
	    Speak("shout", NULL);
	}
      }

      ColorizeEx(ColorShout, FALSE);

      return TRUE;
    }

    if (looking_at(buf, i, "* kibitzes: *")) {
      if (appData.zippyTalk && ((unsigned) random() % 10) < 9) {
	char *player = StripHighlightAndTitle(star_match[0]);
	if (strcmp(player, ics_handle) != 0) {
	    Speak("kibitz", NULL);
	}
      }

      ColorizeEx(ColorKibitz, FALSE);

      return TRUE;
    }

    if (looking_at(buf, i, "* whispers: *")) {
      if (appData.zippyTalk && ((unsigned) random() % 10) < 9) {
	char *player = StripHighlightAndTitle(star_match[0]);
	if (strcmp(player, ics_handle) != 0) {
	    Speak("whisper", NULL);
	}
      }

      ColorizeEx(ColorKibitz, FALSE);

      return TRUE;
    }

    /* Messages */
    if ((looking_at(buf, i, ". * (*:*): *") && isdigit(star_match[1][0])) ||
	 looking_at(buf, i, ". * at *:*: *")) {
      if (appData.zippyTalk) {
	FILE *f;
	char *player = StripHighlightAndTitle(star_match[0]);

	if (strcmp(player, ics_handle) != 0) {
	    if (((unsigned) random() % 10) < 9)
	      Speak("message", player);
	    f = fopen("zippy.messagelog", "a");
	    fprintf(f, "%s (%s:%s): %s\n", player,
		    star_match[1], star_match[2], star_match[3]);
	    fclose(f);
	}
      }
      return TRUE;
    }

    /* Channel tells */
    oldi = *i;
    if (looking_at(buf, i, "*(*: *")) {
	char *channel;
	if (star_match[0][0] == NULLCHAR  ||
	    strchr(star_match[0], ' ') ||
	    strchr(star_match[1], ' ')) {
	    /* Oops, did not want to match this; probably a message */
	    *i = oldi;
	    return FALSE;
	}
	if (appData.zippyTalk) {
	  channel = strrchr(star_match[1], '(');
	  if (channel == NULL) {
	    channel = star_match[1];
	  } else {
	    channel++;
	  }
	  channel[strlen(channel)-1] = NULLCHAR;

	  /* Tell to the channel only if someone mentions our name */
	  if (ZippyCalled(star_match[2])) {
	    Speak("tell", channel);
	  }

          ColorizeEx( atoi(channel) == 1 ? ColorChannel1 : ColorChannel, FALSE );
	}
	return TRUE;
    }

    if (!appData.zippyTalk) return FALSE;

    if ((looking_at(buf, i, "You have * message") &&
	 atoi(star_match[0]) != 0) ||
	looking_at(buf, i, "* has left a message for you") ||
	looking_at(buf, i, "* just sent you a message")) {
      snprintf(reply, MSG_SIZ, "%smessages\n%sclearmessages *\n",
		ics_prefix, ics_prefix);
	SendToICS(reply);
	return TRUE;
    }

    if (looking_at(buf, i, "Notification: * has arrived")) {
	if (((unsigned) random() % 3) == 0) {
	    char *player = StripHighlightAndTitle(star_match[0]);
	    safeStrCpy(lastgreet, player, sizeof(lastgreet)/sizeof(lastgreet[0]));
	    snprintf(reply, MSG_SIZ, "greet %s\n", player);
	    SendToICS(reply);
	    Speak("tell", player);
	}
    }

    if (looking_at(buf, i, "Notification: * has departed")) {
	if (((unsigned) random() % 3) == 0) {
	    char *player = StripHighlightAndTitle(star_match[0]);
	    snprintf(reply, MSG_SIZ, "farewell %s\n", player);
	    SendToICS(reply);
	}
    }

    if (looking_at(buf, i, "Not sent -- * is censoring you")) {
	char *player = StripHighlightAndTitle(star_match[0]);
	if (strcmp(player, lastgreet) == 0) {
	  snprintf(reply, MSG_SIZ, "%s-notify %s\n", ics_prefix, player);
	    SendToICS(reply);
	}
    }

    if (looking_at(buf, i, "command is currently turned off")) {
	appData.zippyUseI = 0;
    }

    return FALSE;
}

void
ZippyGameStart (char *white, char* black)
{
    if (!first.initDone) {
      /* Game is starting prematurely.  We can't deal with this */
      SendToICS(ics_prefix);
      SendToICS("abort\n");
      SendToICS(ics_prefix);
      SendToICS("say Sorry, the chess program is not initialized yet.\n");
      return;
    }

    if (appData.zippyGameStart[0] != NULLCHAR) {
      SendToICS(appData.zippyGameStart);
      SendToICS("\n");
    }
}

void
ZippyGameEnd (ChessMove result, char *resultDetails)
{
    if (appData.zippyAcceptOnly[0] == NULLCHAR &&
	appData.zippyGameEnd[0] != NULLCHAR) {
      SendToICS(appData.zippyGameEnd);
      SendToICS("\n");
    }
    zippyLastGameEnd = time(0);
    if(forwardMostMove < appData.zippyShortGame)
      safeStrCpy(zippyOffender, zippyLastOpp, sizeof(zippyOffender)/sizeof(zippyOffender[0]));
    else
      zippyOffender[0] = 0; // [HGM] aborter
}

/*
 * Routines to implement Zippy playing chess
 */

void
ZippyHandleChallenge (char *srated, char *swild, char *sbase, char *sincrement, char *opponent)
{
    char buf[MSG_SIZ];
    int i=0;
    VariantClass variant;
    char *varname;

    variant = StringToVariant(swild);
    varname = VariantName(variant);

    /* [DM] If icsAnalyzeEngine active we don't accept automatic games */
    if (appData.icsActive && appData.icsEngineAnalyze) return;

    /* If desired, you can insert more code here to decline matches
       based on rated, variant, base, and increment, but it is
       easier to use the ICS formula feature instead. */

    if (variant == VariantLoadable) {
      snprintf(buf, MSG_SIZ,
	 "%stell %s This computer can't play wild type %s\n%sdecline %s\n",
		ics_prefix, opponent, swild, ics_prefix, opponent);
	SendToICS(buf);
	return;
    }
    if (StrStr(appData.zippyVariants, varname) == NULL ||
              ((i=first.protocolVersion) != 1 && StrStr(first.variants, varname) == NULL) /* [HGM] zippyvar */
                                                          ) {
      snprintf(buf, MSG_SIZ,
	 "%stell %s This computer can't play %s [%s], only %s\n%sdecline %s\n",
		ics_prefix, opponent, swild, varname,
                i ? first.variants : appData.zippyVariants,                               /* [HGM] zippyvar */
		ics_prefix, opponent);
	SendToICS(buf);
	return;
    }

    /* Are we blocking match requests from all but one person? */
    if (appData.zippyAcceptOnly[0] != NULLCHAR &&
	StrCaseCmp(opponent, appData.zippyAcceptOnly)) {
        /* Yes, and this isn't him.  Ignore challenge. */
	return;
    }

    /* Too many consecutive games with same opponent?  If so, make him
       wait until someone else has played or a timeout has elapsed. */
    if (appData.zippyMaxGames &&
	strcmp(opponent, zippyLastOpp) == 0 &&
	zippyConsecGames >= appData.zippyMaxGames &&
	difftime(time(0), zippyLastGameEnd) < appData.zippyReplayTimeout) {
      snprintf(buf, MSG_SIZ,  "%stell %s Sorry, you have just played %d consecutive games against %s.  To give others a chance, please wait %d seconds or until someone else has played.\n%sdecline %s\n",
	      ics_prefix, opponent, zippyConsecGames, ics_handle,
	      appData.zippyReplayTimeout, ics_prefix, opponent);
      SendToICS(buf);
      return;
    }

    /* [HGM] aborter: opponent is cheater that aborts games he doesn't like on first move. Make him wait */
    if (strcmp(opponent, zippyOffender) == 0 &&
	difftime(time(0), zippyLastGameEnd) < appData.zippyReplayTimeout) {
      snprintf(buf, MSG_SIZ,  "%stell %s Sorry, your previous game against %s was rather short. "
		   " It will wait %d seconds to see if a tougher opponent comes along.\n%sdecline %s\n",
	      ics_prefix, opponent, ics_handle,
	      appData.zippyReplayTimeout, ics_prefix, opponent);
      SendToICS(buf);
      return;
    }

    /* Engine not yet initialized or still thinking about last game? */
    if (!first.initDone || first.lastPing != first.lastPong) {
      snprintf(buf, MSG_SIZ,  "%stell %s I'm not quite ready for a new game yet; try again soon.\n%sdecline %s\n",
	      ics_prefix, opponent, ics_prefix, opponent);
      SendToICS(buf);
      return;
    }

    snprintf(buf, MSG_SIZ, "%saccept %s\n", ics_prefix, opponent);
    SendToICS(buf);
    if (appData.zippyTalk) {
      Speak("tell", opponent);
    }
}


/* Accept matches */
int
ZippyMatch (char *buf, int *i)
{
    if (looking_at(buf, i, "* * match * * requested with * (*)")) {

	ZippyHandleChallenge(star_match[0], star_match[1],
			     star_match[2], star_match[3],
			     StripHighlightAndTitle(star_match[4]));
	return TRUE;
    }

    /* Old FICS 0-increment form */
    if (looking_at(buf, i, "* * match * requested with * (*)")) {

	ZippyHandleChallenge(star_match[0], star_match[1],
			     star_match[2], "0",
			     StripHighlightAndTitle(star_match[3]));
	return TRUE;
    }

    if (looking_at(buf, i,
		   "* has made an alternate proposal of * * match * *.")) {

	ZippyHandleChallenge(star_match[1], star_match[2],
			     star_match[3], star_match[4],
			     StripHighlightAndTitle(star_match[0]));
	return TRUE;
    }

    /* FICS wild/nonstandard forms */
    if (looking_at(buf, i, "Challenge: * (*) *(*) * * * * Loaded from *")) {
	/* note: star_match[2] can include "[white] " or "[black] "
	   before our own name. */
	if(star_match[8] == NULL || star_match[8][0] == 0) // [HGM] chessd: open-source ICS has file on next line
	     ZippyHandleChallenge(star_match[4], star_match[5],
			     star_match[6], star_match[7],			     StripHighlightAndTitle(star_match[0]));
	else ZippyHandleChallenge(star_match[4], star_match[8],
			     star_match[6], star_match[7],
			     StripHighlightAndTitle(star_match[0]));
	return TRUE;
    }

    if (looking_at(buf, i,
		   "Challenge: * (*) *(*) * * * * : * * Loaded from *")) {
	/* note: star_match[2] can include "[white] " or "[black] "
	   before our own name. */
	ZippyHandleChallenge(star_match[4], star_match[10],
			     star_match[8], star_match[9],
			     StripHighlightAndTitle(star_match[0]));
	return TRUE;
    }

    /* Regular forms */
    if (looking_at(buf, i, "Challenge: * (*) *(*) * * * * : * *") |
	looking_at(buf, i, "Challenge: * (*) *(*) * * * * * *")) {
	/* note: star_match[2] can include "[white] " or "[black] "
	   before our own name. */
	ZippyHandleChallenge(star_match[4], star_match[5],
			     star_match[8], star_match[9],
			     StripHighlightAndTitle(star_match[0]));
	return TRUE;
    }

    if (looking_at(buf, i, "Challenge: * (*) *(*) * * * *")) {
	/* note: star_match[2] can include "[white] " or "[black] "
	   before our own name. */
	ZippyHandleChallenge(star_match[4], star_match[5],
			     star_match[6], star_match[7],
			     StripHighlightAndTitle(star_match[0]));
	return TRUE;
    }


        if (looking_at(buf, i, "Your opponent offers you a draw") ||
            looking_at(buf, i, "* offers you a draw")) {
            if (first.sendDrawOffers && first.initDone) {
                SendToProgram("draw\n", &first);
            }
            return TRUE;
        }

    if (looking_at(buf, i, "requests that the game be aborted") ||
        looking_at(buf, i, "would like to abort")) {
	if (appData.zippyAbort ||
	    (gameMode == IcsPlayingWhite && whiteTimeRemaining < 0) ||
	    (gameMode == IcsPlayingBlack && blackTimeRemaining < 0)) {
	    SendToICS(ics_prefix);
	    SendToICS("abort\n");
	} else {
	    SendToICS(ics_prefix);
	    if (appData.zippyTalk)
	      SendToICS("say Whoa no!  I am having FUN!!\n");
	    else
	      SendToICS("say Sorry, this computer doesn't accept aborts.\n");
	}
	return TRUE;
    }

    if (looking_at(buf, i, "requests adjournment") ||
	looking_at(buf, i, "would like to adjourn")) {
      if (appData.zippyAdjourn) {
	SendToICS(ics_prefix);
	SendToICS("adjourn\n");
      } else {
	SendToICS(ics_prefix);
	if (appData.zippyTalk)
	  SendToICS("say Whoa no!  I am having FUN playing NOW!!\n");
	else
	  SendToICS("say Sorry, this computer doesn't accept adjourns.\n");
      }
      return TRUE;
    }

    return FALSE;
}

/* Initialize chess program with data from the first board
 * of a new or resumed game.
 */
void
ZippyFirstBoard (int moveNum, int basetime, int increment)
{
    char buf[MSG_SIZ];
    int w, b;
    char *opp = (gameMode==IcsPlayingWhite ? gameInfo.black : gameInfo.white);
    Boolean sentPos = FALSE;
    char *bookHit = NULL; // [HGM] book

    if (!first.initDone) {
      /* Game is starting prematurely.  We can't deal with this */
      SendToICS(ics_prefix);
      SendToICS("abort\n");
      SendToICS(ics_prefix);
      SendToICS("say Sorry, the chess program is not initialized yet.\n");
      return;
    }

    /* Send the variant command if needed */
    if (gameInfo.variant != VariantNormal) {
      snprintf(buf, MSG_SIZ,  "variant %s\n", VariantName(gameInfo.variant));
      SendToProgram(buf, &first);
    }

    if ((startedFromSetupPosition && moveNum == 0) ||
	(!appData.getMoveList && moveNum > 0)) {
      SendToProgram("force\n", &first);
      SendBoard(&first, moveNum);
      sentPos = TRUE;
    }

    snprintf(buf, MSG_SIZ,  "level 0 %d %d\n", basetime, increment);
    SendToProgram(buf, &first);

    /* Count consecutive games from one opponent */
    if (strcmp(opp, zippyLastOpp) == 0) {
      zippyConsecGames++;
    } else {
      zippyConsecGames = 1;
      safeStrCpy(zippyLastOpp, opp, sizeof(zippyLastOpp)/sizeof(zippyLastOpp[0]));
    }

    /* Send the "computer" command if the opponent is in the list
       we've been gathering. */
    for (w=0; w<num_opps; w++) {
	if (!strcmp(opp_name[w], opp)) {
	    SendToProgram(first.computerString, &first);
	    break;
	}
    }

    /* Ratings might be < 0 which means "we haven't seen a ratings
       message from ICS." Send 0 in that case */
    w = (gameInfo.whiteRating >= 0) ? gameInfo.whiteRating : 0;
    b = (gameInfo.blackRating >= 0) ? gameInfo.blackRating : 0;

    firstMove = FALSE;
    if (gameMode == IcsPlayingWhite) {
        if (first.sendName) {
	  snprintf(buf, MSG_SIZ,  "name %s\n", gameInfo.black);
	  SendToProgram(buf, &first);
	}
	safeStrCpy(ics_handle, gameInfo.white, MSG_SIZ);
	snprintf(buf, MSG_SIZ,  "rating %d %d\n", w, b);
	SendToProgram(buf, &first);
	if (sentPos) {
	    /* Position sent above, engine is in force mode */
	    if (WhiteOnMove(moveNum)) {
	      /* Engine is on move now */
	      if (first.sendTime) {
		if (first.useColors) {
		  SendToProgram("black\n", &first); /*gnu kludge*/
		  SendTimeRemaining(&first, TRUE);
		  SendToProgram("white\n", &first);
		} else {
		  SendTimeRemaining(&first, TRUE);
		}
	      }
	      bookHit = SendMoveToBookUser(forwardMostMove-1, &first, TRUE); // [HGM] book: send go or retrieve book move
	    } else {
	        /* Engine's opponent is on move now */
	        if (first.usePlayother) {
		  if (first.sendTime) {
		    SendTimeRemaining(&first, TRUE);
		  }
		  SendToProgram("playother\n", &first);
		} else {
		  /* Need to send a "go" after opponent moves */
		  firstMove = TRUE;
		}
	    }
	} else {
	    /* Position not sent above, move list might be sent later */
	    if (moveNum == 0) {
	        /* No move list coming; at start of game */
	      if (first.sendTime) {
		if (first.useColors) {
		  SendToProgram("black\n", &first); /*gnu kludge*/
		  SendTimeRemaining(&first, TRUE);
		  SendToProgram("white\n", &first);
		} else {
		  SendTimeRemaining(&first, TRUE);
		}
	      }
//	      SendToProgram("go\n", &first);
	      bookHit = SendMoveToBookUser(forwardMostMove-1, &first, TRUE); // [HGM] book: send go or retrieve book move
	    }
	}
    } else if (gameMode == IcsPlayingBlack) {
        if (first.sendName) {
	  snprintf(buf, MSG_SIZ,  "name %s\n", gameInfo.white);
	  SendToProgram(buf, &first);
	}
	safeStrCpy(ics_handle, gameInfo.black, MSG_SIZ);
	snprintf(buf, MSG_SIZ,  "rating %d %d\n", b, w);
	SendToProgram(buf, &first);
	if (sentPos) {
	    /* Position sent above, engine is in force mode */
	    if (!WhiteOnMove(moveNum)) {
	        /* Engine is on move now */
	      if (first.sendTime) {
		if (first.useColors) {
		  SendToProgram("white\n", &first); /*gnu kludge*/
		  SendTimeRemaining(&first, FALSE);
		  SendToProgram("black\n", &first);
		} else {
		  SendTimeRemaining(&first, FALSE);
		}
	      }
//	      SendToProgram("go\n", &first);
	      bookHit = SendMoveToBookUser(forwardMostMove-1, &first, TRUE); // [HGM] book: send go or retrieve book move
	    } else {
	        /* Engine's opponent is on move now */
	        if (first.usePlayother) {
		  if (first.sendTime) {
		    SendTimeRemaining(&first, FALSE);
		  }
		  SendToProgram("playother\n", &first);
		} else {
		  /* Need to send a "go" after opponent moves */
		  firstMove = TRUE;
		}
	    }
	} else {
	    /* Position not sent above, move list might be sent later */
	    /* Nothing needs to be done here */
	}
    }

    if(bookHit) { // [HGM] book: simulate book reply
	static char bookMove[MSG_SIZ]; // a bit generous?

	programStats.depth = programStats.nodes = programStats.time =
	programStats.score = programStats.got_only_move = 0;
	sprintf(programStats.movelist, "%s (xbook)", bookHit);

	safeStrCpy(bookMove, "move ", sizeof(bookMove)/sizeof(bookMove[0]));
	strcat(bookMove, bookHit);
	HandleMachineMove(bookMove, &first);
    }
}


void
ZippyHoldings (char *white_holding, char *black_holding, char *new_piece)
{
    char buf[MSG_SIZ];
    if (gameMode != IcsPlayingBlack && gameMode != IcsPlayingWhite) return;
    snprintf(buf, MSG_SIZ,  "holding [%s] [%s] %s\n",
	    white_holding, black_holding, new_piece);
    SendToProgram(buf, &first);
}
