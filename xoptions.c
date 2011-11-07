/*
 * xoptions.c -- Move list window, part of X front end for XBoard
 *
 * Copyright 2000, 2009, 2010, 2011 Free Software Foundation, Inc.
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

// [HGM] this file is the counterpart of woptions.c, containing xboard popup menus
// similar to those of WinBoard, to set the most common options interactively.

#include "config.h"

#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>  

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

#if HAVE_UNISTD_H
# include <unistd.h>
#endif
#include <stdint.h>

#include "common.h"
#include "backend.h"
#include "xboard.h"
#include "gettext.h"

#ifdef ENABLE_NLS
# define  _(s) gettext (s)
# define N_(s) gettext_noop (s)
#else
# define  _(s) (s)
# define N_(s)  s
#endif

extern GtkBuilder *builder;

// [HGM] the following code for makng menu popups was cloned from the FileNamePopUp routines


//--------------------------- Engine-specific options menu ----------------------------------

typedef void ButtonCallback(int n);
typedef int OKCallback(int n);

int values[MAX_OPTIONS];
ChessProgramState *currentCps;
static Option *currentOption;
static Boolean browserUp;
ButtonCallback *comboCallback;

void GetWidgetTextGTK(GtkWidget *w, char **buf)
{        
    GtkTextIter start;
    GtkTextIter end;    

    if (GTK_IS_TEXT_BUFFER(w)) {
        gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(w), &start);
        gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(w), &end);
        *buf = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(w), &start, &end, FALSE);
    }
    else {
        printf("error in GetWidgetText, invalid widget\n");
        buf = NULL; 
    }
}

void SetSpinValue(Option *opt, int val, int n)
{    
    if (opt->type == Spin)
      {
        if (val == -1)
           gtk_widget_set_sensitive(opt->handle, FALSE);
        else
          {
            gtk_widget_set_sensitive(opt->handle, TRUE);      
            gtk_spin_button_set_value(opt->handle, val);
          }
      }
    else
      printf("error in SetSpinValue, unknown type %d\n", opt->type);    
}

void SetWidgetTextGTK(GtkWidget *w, char *text)
{
    if (!GTK_IS_TEXT_BUFFER(w)) {
        printf("error: SetWidgetTextGTK arg is not a GtkTextBuffer\n");
        return;
    }    
    gtk_text_buffer_set_text(GTK_TEXT_BUFFER(w), text, -1);
}

void ComboSelect(GtkWidget *widget, gpointer gptr)
{
    gint g, i;

    i = (intptr_t)gptr;    
    g = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));    
    values[i] = g; // store in temporary, for transfer at OK

    if(currentOption[i].min & 1 && !currentCps && comboCallback) (comboCallback)(i);
}


//----------------------------Generic dialog --------------------------------------------

// cloned from Engine Settings dialog (and later merged with it)

extern WindowPlacement wpComment, wpTags, wpMoveHistory;
char *trialSound;
static int oldCores, oldPonder;
int MakeColors P((void));
int GenericReadout P((int selected));
//Widget shells[10]; // This is still referenced in xboard.c - needs removing
GtkWidget *shellsGTK[10];
Boolean shellUp[10];
WindowPlacement *wp[10] = { NULL, &wpComment, &wpTags, NULL, NULL, NULL, NULL, &wpMoveHistory };
Option *dialogOptions[10];

void GetMenuItemName(int dlgNr, gchar *name)
{
    switch(dlgNr) {
      case 1: // comment popup
        strcpy(name, "viewcomments");
        break;	   
      case 2: // tags popup
        strcpy(name, "viewtags");	
        break;
      case 3: // ics text menu
        strcpy(name, "ViewICStextmenu");	
        break;
      case 4: // ics input box
        strcpy(name, "ViewICSinputbox");	
        break;
      case 7: // move history
        strcpy(name, "viewmovehistory");
        break;	
      case 100: // engine output 
        strcpy(name, "viewengineoutput");	
        break;
      case 101: // evaluation graph
        strcpy(name, "viewevaluationgraph");	
        break;
      case 102: // gamelist
        strcpy(name, "viewgamelist");	
        break;
      default:        
        strcpy(name, "");
        return;        
    }
}

/* Sets a check box on (True) or off (False) in the menu */
void SetCheckMenuItemActive(gchar *name, int menuDlgNr, gboolean active)
{
    gchar menuItemName[50];

    if (name != NULL)
        strcpy(menuItemName, name);
    else
        GetMenuItemName(menuDlgNr, menuItemName);
    
    if (strcmp(menuItemName, "") == 0) return;

    GtkCheckMenuItem *chk;
    chk = GTK_CHECK_MENU_ITEM(gtk_builder_get_object(GTK_BUILDER(builder), menuItemName));
    if (chk == NULL) {
        printf("Error: failed to get check menu item %s from builder\n", menuItemName);
        return;        
    }
    chk->active = active;   // set the check box without emitting any signals
}

int PopDown(int n)
{
    //Arg args[10];    
    
    if (!shellUp[n]) return 0;    
    
    // when popping down uncheck the check box of the menu item
    SetCheckMenuItemActive(NULL, n, False);    

    gtk_widget_hide(shellsGTK[n]);
    if(n == 0) {
        gtk_widget_destroy(shellsGTK[n]);
        shellsGTK[n] = NULL;
    }    
    shellUp[n] = False;
    if(!n) currentCps = NULL; // if an Engine Settings dialog was up, we must be popping it down now
    return 1;
}

gboolean GenericPopDown(w, event, gdata)
     GtkWidget *w;
     GdkEvent  *event;
     gpointer  gdata; 
{
    int data = (intptr_t) gdata; /* dialog number dlgnr */
    
    if(browserUp) return True; // prevent closing dialog when it has an open file-browse daughter
    PopDown(data);
    return True; /* don't propagate to default handler */
}

char *engineName, *engineDir, *engineChoice, *engineLine, *nickName, *params, *tfName;
Boolean isUCI, hasBook, storeVariant, v1, addToList, useNick;
extern Option installOptions[], matchOptions[];
char *engineNr[] = { N_("First Engine"), N_("Second Engine"), NULL };
char *engineList[100] = {" "}, *engineMnemonic[100] = {""};

int AppendText(Option *opt, char *s)
{    
    char *v;
    int len;
    GtkTextIter end;    
  
    GetWidgetTextGTK(opt->handle, &v);
    len = strlen(v);
    g_free(v);
    gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(opt->handle), &end);
    gtk_text_buffer_insert(opt->handle, &end, s, -1);

    return len;
}

void AddLine(Option *opt, char *s)
{
    GtkTextBuffer *textbuffer;
    GtkTextIter end;

    /* only used on matchOptions tourney manager at present */    
    if (opt->type==TextBox && opt->min){
        textbuffer = opt->handle;       
        gtk_text_buffer_get_end_iter (textbuffer, &end);
        gtk_text_buffer_insert(textbuffer, &end, s, -1);
        gtk_text_buffer_insert(textbuffer, &end, "\n", -1);
    }
    else
        printf("Error in xoptions AddLine - widget not a textbuffer\n");

}

void AddToTourney(int n)
{    
    GenericReadout(4);  // selected engine
    AddLine(&matchOptions[3], engineChoice);
}

int MatchOK(int n)
{
    ASSIGN(appData.participants, engineName);
    if(!CreateTourney(tfName) || matchMode) return matchMode || !appData.participants[0];
    PopDown(0); // early popdown to prevent FreezeUI called through MatchEvent from causing XtGrab warning
    MatchEvent(2); // start tourney
    return 1;
}

void ReplaceParticipant()
{
    GenericReadout(3);
    Substitute(strdup(engineName), True);
}

void UpgradeParticipant()
{
    GenericReadout(3);
    Substitute(strdup(engineName), False);
}

Option matchOptions[] = {
{ 0,  0,          0, NULL, (void*) &tfName, ".trn", NULL, FileName, N_("Tournament file:") },
{ 0,  0,          0, NULL, (void*) &appData.roundSync, "", NULL, CheckBox, N_("Sync after round    (for concurrent playing of a single") },
{ 0,  0,          0, NULL, (void*) &appData.cycleSync, "", NULL, CheckBox, N_("Sync after cycle      tourney with multiple XBoards)") },
{ 0xD, 150,       0, NULL, (void*) &engineName, "", NULL, TextBox, "Tourney participants:" },
{ 0,  1,          0, NULL, (void*) &engineChoice, (char*) (engineMnemonic+1), (engineMnemonic+1), ComboBox, N_("Select Engine:") },
{ 0,  0,         10, NULL, (void*) &appData.tourneyType, "", NULL, Spin, N_("Tourney type (0 = round-robin, 1 = gauntlet):") },
{ 0,  1, 1000000000, NULL, (void*) &appData.tourneyCycles, "", NULL, Spin, N_("Number of tourney cycles (or Swiss rounds):") },
{ 0,  1, 1000000000, NULL, (void*) &appData.defaultMatchGames, "", NULL, Spin, N_("Default Number of Games in Match (or Pairing):") },
{ 0,  0, 1000000000, NULL, (void*) &appData.matchPause, "", NULL, Spin, N_("Pause between Match Games (msec):") },
{ 0,  0,          0, NULL, (void*) &appData.saveGameFile, ".pgn", NULL, FileName, N_("Save Tourney Games on:") },
{ 0,  0,          0, NULL, (void*) &appData.loadGameFile, ".pgn", NULL, FileName, N_("Game File with Opening Lines:") },
{ 0, -2, 1000000000, NULL, (void*) &appData.loadGameIndex, "", NULL, Spin, N_("Game Number (-1 or -2 = Auto-Increment):") },
{ 0,  0,          0, NULL, (void*) &appData.loadPositionFile, ".fen", NULL, FileName, N_("File with Start Positions:") },
{ 0, -2, 1000000000, NULL, (void*) &appData.loadPositionIndex, "", NULL, Spin, N_("Position Number (-1 or -2 = Auto-Increment):") },
{ 0,  0, 1000000000, NULL, (void*) &appData.rewindIndex, "", NULL, Spin, N_("Rewind Index after this many Games (0 = never):") },
{ 0,  0,          0, NULL, (void*) &appData.defNoBook, "", NULL, CheckBox, N_("Disable own engine books by default") },
{ 0,  0,          0, NULL, (void*) &ReplaceParticipant, NULL, NULL, Button, N_("Replace Engine") },
{ 0,  1,          0, NULL, (void*) &UpgradeParticipant, NULL, NULL, Button, N_("Upgrade Engine") },
{ 0, 1, 0, NULL, (void*) &MatchOK, "", NULL, EndMark , "" }
};

int GeneralOptionsOK(int n)
{
	int newPonder = appData.ponderNextMove;
	appData.ponderNextMove = oldPonder;
	PonderNextMoveEvent(newPonder);
	return 1;
}

Option generalOptions[] = {
{ 0,  0, 0, NULL, (void*) &appData.whitePOV, "", NULL, CheckBox, N_("Absolute Analysis Scores") },
{ 0,  0, 0, NULL, (void*) &appData.sweepSelect, "", NULL, CheckBox, N_("Almost Always Queen (Detour Under-Promote)") },
{ 0,  0, 0, NULL, (void*) &appData.animateDragging, "", NULL, CheckBox, N_("Animate Dragging") },
{ 0,  0, 0, NULL, (void*) &appData.animate, "", NULL, CheckBox, N_("Animate Moving") },
{ 0,  0, 0, NULL, (void*) &appData.autoCallFlag, "", NULL, CheckBox, N_("Auto Flag") },
{ 0,  0, 0, NULL, (void*) &appData.autoFlipView, "", NULL, CheckBox, N_("Auto Flip View") },
{ 0,  0, 0, NULL, (void*) &appData.blindfold, "", NULL, CheckBox, N_("Blindfold") },
{ 0,  0, 0, NULL, (void*) &appData.dropMenu, "", NULL, CheckBox, N_("Drop Menu") },
{ 0,  0, 0, NULL, (void*) &appData.hideThinkingFromHuman, "", NULL, CheckBox, N_("Hide Thinking from Human") },
{ 0,  0, 0, NULL, (void*) &appData.highlightDragging, "", NULL, CheckBox, N_("Highlight Dragging (Show Move Targets)") },
{ 0,  0, 0, NULL, (void*) &appData.highlightLastMove, "", NULL, CheckBox, N_("Highlight Last Move") },
{ 0,  0, 0, NULL, (void*) &appData.highlightMoveWithArrow, "", NULL, CheckBox, N_("Highlight with Arrow") },
{ 0,  0, 0, NULL, (void*) &appData.ringBellAfterMoves, "", NULL, CheckBox, N_("Move Sound") },
{ 0,  0, 0, NULL, (void*) &appData.oneClick, "", NULL, CheckBox, N_("One-Click Moving") },
{ 0,  0, 0, NULL, (void*) &appData.periodicUpdates, "", NULL, CheckBox, N_("Periodic Updates (in Analysis Mode)") },
{ 0,  0, 0, NULL, (void*) &appData.ponderNextMove, "", NULL, CheckBox, N_("Ponder Next Move") },
{ 0,  0, 0, NULL, (void*) &appData.popupExitMessage, "", NULL, CheckBox, N_("Popup Exit Messages") },
{ 0,  0, 0, NULL, (void*) &appData.popupMoveErrors, "", NULL, CheckBox, N_("Popup Move Errors") },
{ 0,  0, 0, NULL, (void*) &appData.showEvalInMoveHistory, "", NULL, CheckBox, N_("Scores in Move List") },
{ 0,  0, 0, NULL, (void*) &appData.showCoords, "", NULL, CheckBox, N_("Show Coordinates") },
{ 0,  0, 0, NULL, (void*) &appData.markers, "", NULL, CheckBox, N_("Show Target Squares") },
{ 0,  0, 0, NULL, (void*) &appData.testLegality, "", NULL, CheckBox, N_("Test Legality") },
{ 0, 0, 10, NULL, (void*) &appData.flashCount, "", NULL, Spin, N_("Flash Moves (0 = no flashing):") },
{ 0, 1, 10, NULL, (void*) &appData.flashRate, "", NULL, Spin, N_("Flash Rate (high = fast):") },
{ 0, 5, 100,NULL, (void*) &appData.animSpeed, "", NULL, Spin, N_("Animation Speed (high = slow):") },
{ 0,  1, 5, NULL, (void*) &appData.zoom, "", NULL, Spin, N_("Zoom factor in Evaluation Graph:") },
{ 0,  0, 0, NULL, (void*) &GeneralOptionsOK, "", NULL, EndMark , "" }
};

void Pick(int n)
{
	VariantClass v = currentOption[n].value;
	if(!appData.noChessProgram) {
	    char *name = VariantName(v), buf[MSG_SIZ];
	    if (first.protocolVersion > 1 && StrStr(first.variants, name) == NULL) {
		/* [HGM] in protocol 2 we check if variant is suported by engine */
	      snprintf(buf, MSG_SIZ,  _("Variant %s not supported by %s"), name, first.tidy);
		DisplayError(buf, 0);
		return; /* ignore OK if first engine does not support it */
	    } else
	    if (second.initDone && second.protocolVersion > 1 && StrStr(second.variants, name) == NULL) {
	      snprintf(buf, MSG_SIZ,  _("Warning: second engine (%s) does not support this!"), second.tidy);
		DisplayError(buf, 0);   /* use of second engine is optional; only warn user */
	    }
	}

	GenericReadout(-1); // make sure ranks and file settings are read

	gameInfo.variant = v;
	appData.variant = VariantName(v);

	shuffleOpenings = FALSE; /* [HGM] shuffle: possible shuffle reset when we switch */
	startedFromPositionFile = FALSE; /* [HGM] loadPos: no longer valid in new variant */
	appData.pieceToCharTable = NULL;
	appData.pieceNickNames = "";
	appData.colorNickNames = "";
	Reset(True, True);
        PopDown(0);
        return;
}

Option variantDescriptors[] = {
{ VariantNormal, 0, 135, NULL, (void*) &Pick, "#FFFFFF", NULL, Button, N_("normal")},
{ VariantFairy, 1, 135, NULL, (void*) &Pick, "#BFBFBF", NULL, Button, N_("fairy")},
{ VariantFischeRandom, 0, 135, NULL, (void*) &Pick, "#FFFFFF", NULL, Button, N_("FRC")},
{ VariantSChess, 1, 135, NULL, (void*) &Pick, "#FFBFBF", NULL, Button, N_("Seirawan")},
{ VariantWildCastle, 0, 135, NULL, (void*) &Pick, "#FFFFFF", NULL, Button, N_("wild castle")},
{ VariantSuper, 1, 135, NULL, (void*) &Pick, "#FFBFBF", NULL, Button, N_("Superchess")},
{ VariantNoCastle, 0, 135, NULL, (void*) &Pick, "#FFFFFF", NULL, Button, N_("no castle")},
{ VariantCrazyhouse, 1, 135, NULL, (void*) &Pick, "#FFBFBF", NULL, Button, N_("crazyhouse")},
{ VariantKnightmate, 0, 135, NULL, (void*) &Pick, "#FFFFFF", NULL, Button, N_("knightmate")},
{ VariantBughouse, 1, 135, NULL, (void*) &Pick, "#FFBFBF", NULL, Button, N_("bughouse")},
{ VariantBerolina, 0, 135, NULL, (void*) &Pick, "#FFFFFF", NULL, Button, N_("berolina")},
{ VariantShogi, 1, 135, NULL, (void*) &Pick, "#BFFFFF", NULL, Button, N_("shogi (9x9)")},
{ VariantCylinder, 0, 135, NULL, (void*) &Pick, "#FFFFFF", NULL, Button, N_("cylinder")},
{ VariantXiangqi, 1, 135, NULL, (void*) &Pick, "#BFFFFF", NULL, Button, N_("xiangqi (9x10)")},
{ VariantShatranj, 0, 135, NULL, (void*) &Pick, "#FFFFFF", NULL, Button, N_("shatranj")},
{ VariantCourier, 1, 135, NULL, (void*) &Pick, "#BFFFBF", NULL, Button, N_("courier (12x8)")},
{ VariantMakruk, 0, 135, NULL, (void*) &Pick, "#FFFFFF", NULL, Button, N_("makruk")},
{ VariantGreat, 1, 135, NULL, (void*) &Pick, "#BFBFFF", NULL, Button, N_("Great Shatranj (10x8)")},
{ VariantAtomic, 0, 135, NULL, (void*) &Pick, "#FFFFFF", NULL, Button, N_("atomic")},
{ VariantFalcon, 1, 135, NULL, (void*) &Pick, "#BFBFFF", NULL, Button, N_("falcon (10x8)")},
{ VariantTwoKings, 0, 135, NULL, (void*) &Pick, "#FFFFFF", NULL, Button, N_("two kings")},
{ VariantCapablanca, 1, 135, NULL, (void*) &Pick, "#BFBFFF", NULL, Button, N_("Capablanca (10x8)")},
{ Variant3Check, 0, 135, NULL, (void*) &Pick, "#FFFFFF", NULL, Button, N_("3-checks")},
{ VariantGothic, 1, 135, NULL, (void*) &Pick, "#BFBFFF", NULL, Button, N_("Gothic (10x8)")},
{ VariantSuicide, 0, 135, NULL, (void*) &Pick, "#FFFFBF", NULL, Button, N_("suicide")},
{ VariantJanus, 1, 135, NULL, (void*) &Pick, "#BFBFFF", NULL, Button, N_("janus (10x8)")},
{ VariantGiveaway, 0, 135, NULL, (void*) &Pick, "#FFFFBF", NULL, Button, N_("give-away")},
{ VariantCapaRandom, 1, 135, NULL, (void*) &Pick, "#BFBFFF", NULL, Button, N_("CRC (10x8)")},
{ VariantLosers, 0, 135, NULL, (void*) &Pick, "#FFFFBF", NULL, Button, N_("losers")},
{ VariantGrand, 1, 135, NULL, (void*) &Pick, "#5070FF", NULL, Button, N_("grand (10x10)")},
{ VariantSpartan, 0, 135, NULL, (void*) &Pick, "#FF0000", NULL, Button, N_("Spartan")},
{ 0, 0, 0, NULL, NULL, NULL, NULL, Label, N_("Board size ( -1 = default for selected variant):")},
{ 0, -1, BOARD_RANKS-1, NULL, (void*) &appData.NrRanks, "", NULL, Spin, N_("Number of Board Ranks:") },
{ 0, -1, BOARD_FILES, NULL, (void*) &appData.NrFiles, "", NULL, Spin, N_("Number of Board Files:") },
{ 0, -1, BOARD_RANKS-1, NULL, (void*) &appData.holdingsSize, "", NULL, Spin, N_("Holdings Size:") },
{ 0, 0, 0, NULL, NULL, NULL, NULL, Label,
				N_("WARNING: variants with un-orthodox\n"
				  "pieces only have built-in bitmaps\n"
				  "for -boardSize middling, bulky and\n"
				  "petite, and substitute king or amazon\n"
				  "for missing bitmaps. (See manual.)")},
{ 0, 2, 0, NULL, NULL, "", NULL, EndMark , "" }
};

int CommonOptionsOK(int n)
{
	int newPonder = appData.ponderNextMove;
	// make sure changes are sent to first engine by re-initializing it
	// if it was already started pre-emptively at end of previous game
	if(gameMode == BeginningOfGame) Reset(True, True); else {
	    // Some changed setting need immediate sending always.
	    if(oldCores != appData.smpCores)
		NewSettingEvent(False, &(first.maxCores), "cores", appData.smpCores);
	    appData.ponderNextMove = oldPonder;
	    PonderNextMoveEvent(newPonder);
	}
	return 1;
}

Option commonEngineOptions[] = {
{ 0,     0, 0, NULL, (void*) &appData.ponderNextMove, "", NULL, CheckBox, N_("Ponder Next Move") },
{ 0,  0, 1000, NULL, (void*) &appData.smpCores, "", NULL, Spin, N_("Maximum Number of CPUs per Engine:") },
{ 0,     0, 0, NULL, (void*) &appData.polyglotDir, "", NULL, PathName, N_("Polygot Directory:") },
{ 0, 0, 16000, NULL, (void*) &appData.defaultHashSize, "", NULL, Spin, N_("Hash-Table Size (MB):") },
{ 0,     0, 0, NULL, (void*) &appData.defaultPathEGTB, "", NULL, PathName, N_("Nalimov EGTB Path:") },
{ 0,  0, 1000, NULL, (void*) &appData.defaultCacheSizeEGTB, "", NULL, Spin, N_("EGTB Cache Size (MB):") },
{ 0,     0, 0, NULL, (void*) &appData.usePolyglotBook, "", NULL, CheckBox, N_("Use GUI Book") },
{ 0,     0, 0, NULL, (void*) &appData.polyglotBook, ".bin", NULL, FileName, N_("Opening-Book Filename:") },
{ 0,   0, 100, NULL, (void*) &appData.bookDepth, "", NULL, Spin, N_("Book Depth (moves):") },
{ 0,   0, 100, NULL, (void*) &appData.bookStrength, "", NULL, Spin, N_("Book Variety (0) vs. Strength (100):") },
{ 0,     0, 0, NULL, (void*) &appData.firstHasOwnBookUCI, "", NULL, CheckBox, N_("Engine #1 Has Own Book") },
{ 0,     0, 0, NULL, (void*) &appData.secondHasOwnBookUCI, "", NULL, CheckBox, N_("Engine #2 Has Own Book          ") },
{ 0,     1, 0, NULL, (void*) &CommonOptionsOK, "", NULL, EndMark , "" }
};

Option adjudicationOptions[] = {
{ 0, 0,    0, NULL, (void*) &appData.checkMates, "", NULL, CheckBox, N_("Detect all Mates") },
{ 0, 0,    0, NULL, (void*) &appData.testClaims, "", NULL, CheckBox, N_("Verify Engine Result Claims") },
{ 0, 0,    0, NULL, (void*) &appData.materialDraws, "", NULL, CheckBox, N_("Draw if Insufficient Mating Material") },
{ 0, 0,    0, NULL, (void*) &appData.trivialDraws, "", NULL, CheckBox, N_("Adjudicate Trivial Draws (3-Move Delay)") },
{ 0, 0,  100, NULL, (void*) &appData.ruleMoves, "", NULL, Spin, N_("N-Move Rule:") },
{ 0, 0,    6, NULL, (void*) &appData.drawRepeats, "", NULL, Spin, N_("N-fold Repeats:") },
{ 0, 0, 1000, NULL, (void*) &appData.adjudicateDrawMoves, "", NULL, Spin, N_("Draw after N Moves Total:") },
{ 0,-5000, 0, NULL, (void*) &appData.adjudicateLossThreshold, "", NULL, Spin, N_("Win / Loss Threshold:") },
{ 0, 0,    0, NULL, (void*) &first.scoreIsAbsolute, "", NULL, CheckBox, N_("Negate Score of Engine #1") },
{ 0, 0,    0, NULL, (void*) &second.scoreIsAbsolute, "", NULL, CheckBox, N_("Negate Score of Engine #2") },
{ 0, 1,    0, NULL, NULL, "", NULL, EndMark , "" }
};

int IcsOptionsOK(int n)
{
    ParseIcsTextColors();
    return 1;
}

Option icsOptions[] = {
{ 0, 0, 0, NULL, (void*) &appData.autoKibitz, "",  NULL, CheckBox, N_("Auto-Kibitz") },
{ 0, 0, 0, NULL, (void*) &appData.autoComment, "", NULL, CheckBox, N_("Auto-Comment") },
{ 0, 0, 0, NULL, (void*) &appData.autoObserve, "", NULL, CheckBox, N_("Auto-Observe") },
{ 0, 0, 0, NULL, (void*) &appData.autoRaiseBoard, "", NULL, CheckBox, N_("Auto-Raise Board") },
{ 0, 0, 0, NULL, (void*) &appData.bgObserve, "",   NULL, CheckBox, N_("Background Observe while Playing") },
{ 0, 0, 0, NULL, (void*) &appData.dualBoard, "",   NULL, CheckBox, N_("Dual Board for Background-Observed Game") },
{ 0, 0, 0, NULL, (void*) &appData.getMoveList, "", NULL, CheckBox, N_("Get Move List") },
{ 0, 0, 0, NULL, (void*) &appData.quietPlay, "",   NULL, CheckBox, N_("Quiet Play") },
{ 0, 0, 0, NULL, (void*) &appData.seekGraph, "",   NULL, CheckBox, N_("Seek Graph") },
{ 0, 0, 0, NULL, (void*) &appData.autoRefresh, "", NULL, CheckBox, N_("Auto-Refresh Seek Graph") },
{ 0, 0, 0, NULL, (void*) &appData.premove, "",     NULL, CheckBox, N_("Premove") },
{ 0, 0, 0, NULL, (void*) &appData.premoveWhite, "", NULL, CheckBox, N_("Premove for White") },
{ 0, 0, 0, NULL, (void*) &appData.premoveWhiteText, "", NULL, TextBox, N_("First White Move:") },
{ 0, 0, 0, NULL, (void*) &appData.premoveBlack, "", NULL, CheckBox, N_("Premove for Black") },
{ 0, 0, 0, NULL, (void*) &appData.premoveBlackText, "", NULL, TextBox, N_("First Black Move:") },
{ 0, 0, 0, NULL, NULL, NULL, NULL, Break, "" },
{ 0, 0, 0, NULL, (void*) &appData.icsAlarm, "", NULL, CheckBox, N_("Alarm") },
{ 0, 0, 100000000, NULL, (void*) &appData.icsAlarmTime, "", NULL, Spin, N_("Alarm Time (msec):") },
//{ 0, 0, 0, NULL, (void*) &appData.chatBoxes, "", NULL, TextBox, N_("Startup Chat Boxes:") },
{ 0, 0, 0, NULL, (void*) &appData.colorize, "", NULL, CheckBox, N_("Colorize Messages") },
{ 0, 0, 0, NULL, (void*) &appData.colorShout, "", NULL, TextBox, N_("Shout Text Colors:") },
{ 0, 0, 0, NULL, (void*) &appData.colorSShout, "", NULL, TextBox, N_("S-Shout Text Colors:") },
{ 0, 0, 0, NULL, (void*) &appData.colorChannel1, "", NULL, TextBox, N_("Channel #1 Text Colors:") },
{ 0, 0, 0, NULL, (void*) &appData.colorChannel, "", NULL, TextBox, N_("Other Channel Text Colors:") },
{ 0, 0, 0, NULL, (void*) &appData.colorKibitz, "", NULL, TextBox, N_("Kibitz Text Colors:") },
{ 0, 0, 0, NULL, (void*) &appData.colorTell, "", NULL, TextBox, N_("Tell Text Colors:") },
{ 0, 0, 0, NULL, (void*) &appData.colorChallenge, "", NULL, TextBox, N_("Challenge Text Colors:") },
{ 0, 0, 0, NULL, (void*) &appData.colorRequest, "", NULL, TextBox, N_("Request Text Colors:") },
{ 0, 0, 0, NULL, (void*) &appData.colorSeek, "", NULL, TextBox, N_("Seek Text Colors:") },
{ 0, 0, 0, NULL, (void*) &IcsOptionsOK, "", NULL, EndMark , "" }
};

char *modeNames[] = { N_("Exact position match"), N_("Shown position is subset"), N_("Same material with exactly same Pawn chain"), 
		      N_("Same material"), N_("Material range (top board half optional)"), N_("Material difference (optional stuff balanced)"), NULL };
char *modeValues[] = { "1", "2", "3", "4", "5", "6" };
char *searchMode;

int LoadOptionsOK()
{
    appData.searchMode = atoi(searchMode);
    return 1;
}

Option loadOptions[] = {
{ 0, 0, 0, NULL, (void*) &appData.autoDisplayTags, "", NULL, CheckBox, N_("Auto-Display Tags") },
{ 0, 0, 0, NULL, (void*) &appData.autoDisplayComment, "", NULL, CheckBox, N_("Auto-Display Comment") },
{ 0, 0, 0, NULL, NULL, NULL, NULL, Label, N_("Auto-Play speed of loaded games\n(0 = instant, -1 = off):") },
{ 0, -1, 10000000, NULL, (void*) &appData.timeDelay, "", NULL, Fractional, N_("Seconds per Move:") },
{   0,  0,    0, NULL, NULL, NULL, NULL, Label,  N_("\nThresholds for position filtering in game list:") },
{ 0, 0, 5000, NULL, (void*) &appData.eloThreshold1, "", NULL, Spin, N_("Elo of strongest player at least:") },
{ 0, 0, 5000, NULL, (void*) &appData.eloThreshold2, "", NULL, Spin, N_("Elo of weakest player at least:") },
{ 0, 0, 5000, NULL, (void*) &appData.dateThreshold, "", NULL, Spin, N_("No games before year:") },
{ 0, 1, 50, NULL, (void*) &appData.stretch, "", NULL, Spin, N_("Minimum nr consecutive positions:") },
{ 1, 0, 180, NULL, (void*) &searchMode, (char*) modeNames, modeValues, ComboBox, N_("Seach mode:") },
{ 0, 0, 0, NULL, (void*) &appData.ignoreColors, "", NULL, CheckBox, N_("Also match reversed colors") },
{ 0, 0, 0, NULL, (void*) &appData.findMirror, "", NULL, CheckBox, N_("Also match left-right flipped position") },
{ 0,  0, 0, NULL, (void*) &LoadOptionsOK, "", NULL, EndMark , "" }
};

Option saveOptions[] = {
{ 0, 0, 0, NULL, (void*) &appData.autoSaveGames, "", NULL, CheckBox, N_("Auto-Save Games") },
{ 0, 0, 0, NULL, (void*) &appData.saveGameFile, ".pgn", NULL, FileName,  N_("Save Games on File:") },
{ 0, 0, 0, NULL, (void*) &appData.savePositionFile, ".fen", NULL, FileName,  N_("Save Final Positions on File:") },
{ 0, 0, 0, NULL, (void*) &appData.pgnEventHeader, "", NULL, TextBox,  N_("PGN Event Header:") },
{ 0, 0, 0, NULL, (void*) &appData.oldSaveStyle, "", NULL, CheckBox, N_("Old Save Style (as opposed to PGN)") },
{ 0, 0, 0, NULL, (void*) &appData.saveExtendedInfoInPGN, "", NULL, CheckBox, N_("Save Score/Depth Info in PGN") },
{ 0, 0, 0, NULL, (void*) &appData.saveOutOfBookInfo, "", NULL, CheckBox, N_("Save Out-of-Book Info in PGN           ") },
{ 0, 1, 0, NULL, NULL, "", NULL, EndMark , "" }
};

char *soundNames[] = {
	N_("No Sound"),
	N_("Default Beep"),
	N_("Above WAV File"),
	N_("Car Horn"),
	N_("Cymbal"),
	N_("Ding"),
	N_("Gong"),
	N_("Laser"),
	N_("Penalty"),
	N_("Phone"),
	N_("Pop"),
	N_("Slap"),
	N_("Wood Thunk"),
	NULL,
	N_("User File")
};

char *soundFiles[] = { // sound files corresponding to above names
	"",
	"$",
	"*", // kludge alert: as first thing in the dialog readout this is replaced with the user-given .WAV filename
	"honkhonk.wav",
	"cymbal.wav",
	"ding1.wav",
	"gong.wav",
	"laser.wav",
	"penalty.wav",
	"phone.wav",
	"pop2.wav",
	"slap.wav",
	"woodthunk.wav",
	NULL,
	NULL
};

void Test(int n)
{    
    GenericReadout(2);
    if(soundFiles[values[3]]) PlaySound(soundFiles[values[3]]);
}

Option soundOptions[] = {
{ 0, 0, 0, NULL, (void*) &appData.soundProgram, "", NULL, TextBox, N_("Sound Program:") },
{ 0, 0, 0, NULL, (void*) &appData.soundDirectory, "", NULL, PathName, N_("Sounds Directory:") },
{ 0, 0, 0, NULL, (void*) (soundFiles+2) /* kludge! */, ".wav", NULL, FileName, N_("User WAV File:") },
{ 0, 0, 0, NULL, (void*) &trialSound, (char*) soundNames, soundFiles, ComboBox, N_("Try-Out Sound:") },
{ 0, 1, 0, NULL, (void*) &Test, NULL, NULL, Button, N_("Play") },
{ 0, 0, 0, NULL, (void*) &appData.soundMove, (char*) soundNames, soundFiles, ComboBox, N_("Move:") },
{ 0, 0, 0, NULL, (void*) &appData.soundIcsWin, (char*) soundNames, soundFiles, ComboBox, N_("Win:") },
{ 0, 0, 0, NULL, (void*) &appData.soundIcsLoss, (char*) soundNames, soundFiles, ComboBox, N_("Lose:") },
{ 0, 0, 0, NULL, (void*) &appData.soundIcsDraw, (char*) soundNames, soundFiles, ComboBox, N_("Draw:") },
{ 0, 0, 0, NULL, (void*) &appData.soundIcsUnfinished, (char*) soundNames, soundFiles, ComboBox, N_("Unfinished:") },
{ 0, 0, 0, NULL, (void*) &appData.soundIcsAlarm, (char*) soundNames, soundFiles, ComboBox, N_("Alarm:") },
{ 0, 0, 0, NULL, (void*) &appData.soundShout, (char*) soundNames, soundFiles, ComboBox, N_("Shout:") },
{ 0, 0, 0, NULL, (void*) &appData.soundSShout, (char*) soundNames, soundFiles, ComboBox, N_("S-Shout:") },
{ 0, 0, 0, NULL, (void*) &appData.soundChannel, (char*) soundNames, soundFiles, ComboBox, N_("Channel:") },
{ 0, 0, 0, NULL, (void*) &appData.soundChannel1, (char*) soundNames, soundFiles, ComboBox, N_("Channel 1:") },
{ 0, 0, 0, NULL, (void*) &appData.soundTell, (char*) soundNames, soundFiles, ComboBox, N_("Tell:") },
{ 0, 0, 0, NULL, (void*) &appData.soundKibitz, (char*) soundNames, soundFiles, ComboBox, N_("Kibitz:") },
{ 0, 0, 0, NULL, (void*) &appData.soundChallenge, (char*) soundNames, soundFiles, ComboBox, N_("Challenge:") },
{ 0, 0, 0, NULL, (void*) &appData.soundRequest, (char*) soundNames, soundFiles, ComboBox, N_("Request:") },
{ 0, 0, 0, NULL, (void*) &appData.soundSeek, (char*) soundNames, soundFiles, ComboBox, N_("Seek:") },
{ 0, 1, 0, NULL, NULL, "", NULL, EndMark , "" }
};

void SetColorText(int n, char *buf)
{
    GdkColor color;

    /* Update the RGB colour (e.g. #FFFFCC) on the board options */
    gtk_entry_set_text (GTK_ENTRY (currentOption[n-1].handle), buf);

    /* set the colour of the colour button to the colour that will be used */
    gdk_color_parse( buf, &color );
    gtk_widget_modify_bg ( GTK_WIDGET(currentOption[n].handle), GTK_STATE_NORMAL, &color );
}

void DefColor(int n)
{    
    SetColorText(n, (char*) currentOption[n].choice);
}

void RefreshColor(int source, int n)
{
    int col, j, r, g, b, step = 10;;
    const gchar *s;
    char buf[MSG_SIZ]; // color string

    s = gtk_entry_get_text (GTK_ENTRY (currentOption[source].handle));
    if(sscanf(s, "#%x", &col) != 1) return;   // malformed
    b = col & 0xFF; g = col & 0xFF00; r = col & 0xFF0000;
    switch(n) {
	case 1: r += 0x10000*step;break;
	case 2: g += 0x100*step;  break;
	case 3: b += step;        break;
	case 4: r -= 0x10000*step; g -= 0x100*step; b -= step; break;
    }
    if(r < 0) r = 0; if(g < 0) g = 0; if(b < 0) b = 0;
    if(r > 0xFF0000) r = 0xFF0000; if(g > 0xFF00) g = 0xFF00; if(b > 0xFF) b = 0xFF;
    col = r | g | b;
    snprintf(buf, MSG_SIZ, "#%06x", col);
    for(j=1; j<7; j++) if(buf[j] >= 'a') buf[j] -= 32; // capitalize
    SetColorText(source+1, buf);
}

void AdjustColor(int i)
{
    int n = currentOption[i].value;
    RefreshColor(i-n-1, n);
}

int BoardOptionsOK(int n)
{
    if(appData.overrideLineGap >= 0)
        lineGapGTK = appData.overrideLineGap;
    else
        lineGapGTK = GetLineGap();

    if(appData.overrideLineGap >= 0) lineGap = appData.overrideLineGap; else lineGap = defaultLineGap;
    //MakeColors();
    LoadSvgFiles();
    InitDrawingSizes(-1, 0);
    DrawPosition(True, NULL);
    return 1;
}

Option boardOptions[] = {
{ 0,   0, 70, NULL, (void*) &appData.whitePieceColor, "", NULL, TextBox, N_("White Piece Color:") },
{ 1000, 1, 0, NULL, (void*) &DefColor, NULL, (char**) "#FFFFCC", Button, "      " },
{    1, 1, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, "R" },
{    2, 1, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, "G" },
{    3, 1, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, "B" },
{    4, 1, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, "D" },
{ 0,   0, 70, NULL, (void*) &appData.blackPieceColor, "", NULL, TextBox, N_("Black Piece Color:") },
{ 1000, 1, 0, NULL, (void*) &DefColor, NULL, (char**) "#202020", Button, "      " },
{    1, 1, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, "R" },
{    2, 1, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, "G" },
{    3, 1, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, "B" },
{    4, 1, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, "D" },
{ 0,   0, 70, NULL, (void*) &appData.lightSquareColor, "", NULL, TextBox, N_("Light Square Color:") },
{ 1000, 1, 0, NULL, (void*) &DefColor, NULL, (char**) "#C8C365", Button, "      " },
{    1, 1, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, "R" },
{    2, 1, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, "G" },
{    3, 1, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, "B" },
{    4, 1, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, "D" },
{ 0,   0, 70, NULL, (void*) &appData.darkSquareColor, "", NULL, TextBox, N_("Dark Square Color:") },
{ 1000, 1, 0, NULL, (void*) &DefColor, NULL, (char**) "#77A26D", Button, "      " },
{    1, 1, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, "R" },
{    2, 1, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, "G" },
{    3, 1, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, "B" },
{    4, 1, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, "D" },
{ 0,   0, 70, NULL, (void*) &appData.highlightSquareColor, "", NULL, TextBox, N_("Highlight Color:") },
{ 1000, 1, 0, NULL, (void*) &DefColor, NULL, (char**) "#FFFF00", Button, "      " },
{    1, 1, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, "R" },
{    2, 1, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, "G" },
{    3, 1, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, "B" },
{    4, 1, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, "D" },
{ 0,   0, 70, NULL, (void*) &appData.premoveHighlightColor, "", NULL, TextBox, N_("Premove Highlight Color:") },
{ 1000, 1, 0, NULL, (void*) &DefColor, NULL, (char**) "#FF0000", Button, "      " },
{    1, 1, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, "R" },
{    2, 1, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, "G" },
{    3, 1, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, "B" },
{    4, 1, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, "D" },
//{ 0, 0, 0, NULL, (void*) &appData.upsideDown, "", NULL, CheckBox, N_("Flip Pieces Shogi Style        (Colored buttons restore default)") },
//{ 0, 0, 0, NULL, (void*) &appData.allWhite, "", NULL, CheckBox, N_("Use Outline Pieces for Black") },
//{ 0, 0, 0, NULL, (void*) &appData.monoMode, "", NULL, CheckBox, N_("Mono Mode") },
{ 0,-1, 5, NULL, (void*) &appData.overrideLineGap, "", NULL, Spin, N_("Line Gap ( -1 = default for board size):") },
{ 0, 0, 0, NULL, (void*) &appData.useBoardTexture, "", NULL, CheckBox, N_("Use Board Textures") },
{ 0, 0, 0, NULL, (void*) &appData.liteBackTextureFile, ".svg", NULL, FileName, N_("Light-Squares Texture File:") },
{ 0, 0, 0, NULL, (void*) &appData.darkBackTextureFile, ".svg", NULL, FileName, N_("Dark-Squares Texture File:") },
//{ 0, 0, 0, NULL, (void*) &appData.bitmapDirectory, "", NULL, PathName, N_("Directory with Bitmap Pieces:") },
//{ 0, 0, 0, NULL, (void*) &appData.pixmapDirectory, "", NULL, PathName, N_("Directory with Pixmap Pieces:") },
{ 0, 0, 0, NULL, (void*) &BoardOptionsOK, "", NULL, EndMark , "" }
};

int GenericReadout(int selected)
{
    GtkWidget *checkbutton;
    GtkWidget *spinner;
    GtkWidget *entry;
    GtkTextBuffer *textbuffer;    
    GtkTextIter start;
    GtkTextIter end;
    String val;
    char **dest;
    char buf[MSG_SIZ];
    float x;
    int i, j, res=1;
          
    for (i=0;;i++) {
        if(selected >= 0) { if(i < selected) continue; else if(i > selected) break; }             
        switch(currentOption[i].type) {
          case TextBox:
          case FileName:
          case PathName:

            if (currentOption[i].type==TextBox && currentOption[i].min > 80){
                textbuffer = currentOption[i].handle;
                gtk_text_buffer_get_start_iter (textbuffer, &start);
                gtk_text_buffer_get_end_iter (textbuffer, &end);
                val = gtk_text_buffer_get_text (textbuffer, &start, &end, FALSE);                    
            } else {
                entry = currentOption[i].handle;
                val = (String)gtk_entry_get_text (GTK_ENTRY (entry));
            }
            dest = currentCps ? &(currentOption[i].textValue) : (char**) currentOption[i].target;
            if(*dest == NULL || strcmp(*dest, val)) {
                if(currentCps) {
                    snprintf(buf, MSG_SIZ,  "option %s=%s\n", currentOption[i].name, val);
                    SendToProgram(buf, currentCps);
                } else {
                    if(*dest) free(*dest);
                    *dest = malloc(strlen(val)+1);
                }
                safeStrCpy(*dest, val, MSG_SIZ - (*dest - currentOption[i].name)); // copy text there
            }
            break;
          case Spin:
          case Fractional:
            if (currentOption[i].type == Spin) {
               spinner = currentOption[i].handle;
               x = gtk_spin_button_get_value (GTK_SPIN_BUTTON(spinner));                   
            }
            else {
               entry = currentOption[i].handle;                
               val = (String)gtk_entry_get_text (GTK_ENTRY (entry));
               sscanf(val, "%f", &x);
            }                  
            if(x > currentOption[i].max) x = currentOption[i].max;
            if(x < currentOption[i].min) x = currentOption[i].min;
            if(currentOption[i].type == Fractional)
                *(float*) currentOption[i].target = x; // engines never have float options!
            else if(currentOption[i].value != x) {
                currentOption[i].value = x;
                if(currentCps) {
                    snprintf(buf, MSG_SIZ,  "option %s=%.0f\n", currentOption[i].name, x);
                    SendToProgram(buf, currentCps);
                } else *(int*) currentOption[i].target = x;
            }	
            break;
          case CheckBox:               
            checkbutton = currentOption[i].handle;
            j = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (checkbutton));
            if(currentOption[i].value != j) {
                currentOption[i].value = j;
                if(currentCps) {
                    snprintf(buf, MSG_SIZ,  "option %s=%d\n", currentOption[i].name, j);
                    SendToProgram(buf, currentCps);
                } else *(Boolean*) currentOption[i].target = j;
            }        
            break;
          case Button:
          case SaveButton:
          case Label:
          case Break:
            break;
          case ComboBox:
             val = ((char**)currentOption[i].choice)[values[i]];                 
             if (val && strcmp(val," ") == 0) val = NULL; // Fix issue with Load New Engine.. dialog 
             if(currentCps) {
                 if(currentOption[i].value == values[i]) break; // not changed
                     currentOption[i].value = values[i];
                     snprintf(buf, MSG_SIZ,  "option %s=%s\n", currentOption[i].name,
                             ((char**)currentOption[i].textValue)[values[i]]);
                     SendToProgram(buf, currentCps);
             } else if(val && (*(char**) currentOption[i].target == NULL || strcmp(*(char**) currentOption[i].target, val))) {
                 if(*(char**) currentOption[i].target) free(*(char**) currentOption[i].target);
                 *(char**) currentOption[i].target = strdup(val);
             }
            break;
          case EndMark:                              
            if(currentOption[i].target) // callback for implementing necessary actions on OK (like redraw)
              res = ((OKCallback*) currentOption[i].target)(i);
            break;
        default:
            printf("GenericReadout: unexpected case in switch %d.\n", currentOption[i].type);
            break;
        }
        if(currentOption[i].type == EndMark) break;
    }
    return res;       
}

static char *oneLiner  = "<Key>Return:	redraw-display()\n";

/* GTK callback used when OK/cancel clicked in genericpopup for non-modal dialog */
void GenericPopUpCallback(w, resptype, gdata)
     GtkWidget *w;
     GtkResponseType  resptype;
     gpointer  gdata;
{
    int data = (intptr_t) gdata; /* dialog number dlgnr */

    /* OK pressed */    
    if (resptype == GTK_RESPONSE_ACCEPT) {
        if (GenericReadout(-1)) PopDown(data);
        return;
    }

    /* cancel pressed */
    PopDown(data);    
}

void GenericCallback(GtkWidget *widget, gpointer gdata)
{
    const gchar *name;
    char buf[MSG_SIZ];    
    int data = (intptr_t) gdata;   

    currentOption = dialogOptions[data>>16]; data &= 0xFFFF;
    
    if(currentCps) {
        //if(currentOption[data].type == SaveButton) GenericReadout(-1);
        name = gtk_button_get_label (GTK_BUTTON(widget));         
        snprintf(buf, MSG_SIZ,  "option %s\n", name);
        SendToProgram(buf, currentCps);
    } else ((ButtonCallback*) currentOption[data].target)(data);   
}

void Browse(GtkWidget *widget, gpointer gdata)
{
    GtkWidget *entry;
    GtkWidget *dialog;
    GtkFileFilter *gtkfilter;
    GtkFileFilter *gtkfilter_all;
    int opt_i = (intptr_t) gdata;
    GtkFileChooserAction fc_action;
  
    gtkfilter     = gtk_file_filter_new();
    gtkfilter_all = gtk_file_filter_new();

    char fileext[10] = "*";

    /* select file or folder depending on option_type */
    if (currentOption[opt_i].type == PathName)
        fc_action = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;
    else
        fc_action = GTK_FILE_CHOOSER_ACTION_OPEN;

    dialog = gtk_file_chooser_dialog_new ("Open File",
                      NULL,
                      fc_action,
                      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                      NULL);

    /* one filter to show everything */
    gtk_file_filter_add_pattern(gtkfilter_all, "*");
    gtk_file_filter_set_name   (gtkfilter_all, "All Files");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),gtkfilter_all);
    
    /* filter for specific filetypes e.g. pgn or fen */
    if (currentOption[opt_i].textValue != NULL && (strcmp(currentOption[opt_i].textValue, "") != 0) )    
      {          
        strcat(fileext, currentOption[opt_i].textValue);    
        gtk_file_filter_add_pattern(gtkfilter, fileext);
        gtk_file_filter_set_name (gtkfilter, currentOption[opt_i].textValue);
        gtk_file_chooser_add_filter (GTK_FILE_CHOOSER(dialog),gtkfilter);
        /* activate filter */
        gtk_file_chooser_set_filter (GTK_FILE_CHOOSER(dialog),gtkfilter);
      }
    else
      gtk_file_chooser_set_filter (GTK_FILE_CHOOSER(dialog),gtkfilter_all);       

    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
      {
        char *filename;
        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));             
        entry = currentOption[opt_i].handle;
        gtk_entry_set_text (GTK_ENTRY (entry), filename);        
        g_free (filename);

      }
    gtk_widget_destroy (dialog);
    dialog = NULL;
}

int
GenericPopUp(Option *option, char *title, int dlgNr)
{    
    GtkWidget *dialog = NULL;
    gint       w;
    GtkWidget *label;
    GtkWidget *box;
    GtkWidget *checkbutton;
    GtkWidget *entry;
    GtkWidget *hbox;    
    GtkWidget *button;
    GtkWidget *table;
    GtkWidget *spinner;    
    GtkAdjustment *spinner_adj;
    GtkWidget *combobox;
    GtkWidget *textview;
    GtkTextBuffer *textbuffer;           
    GdkColor color;     
    GtkWidget *actionarea;
    GtkWidget *sw;    

    int i, j, arraysize, left, top, height=999, width=1;    
    char def[MSG_SIZ];
    
    if(shellUp[dlgNr]) return 0; // already up   

    if(dlgNr && shellsGTK[dlgNr]) {
        gtk_widget_show(shellsGTK[dlgNr]);
        shellUp[dlgNr] = True;
        return 0;
    }

    dialogOptions[dlgNr] = option; // make available to callback
    // post currentOption globally, so Spin and Combo callbacks can already use it
    // WARNING: this kludge does not work for persistent dialogs, so that these cannot have spin or combo controls!
    currentOption = option;

    if(currentCps) { // Settings popup for engine: format through heuristic
        int n = currentCps->nrOptions;
        if(!n) { DisplayNote(_("Engine has no options")); return 0; }
        if(n > 50) width = 4; else if(n>24) width = 2; else width = 1;
        height = n / width + 1;
        /*if(n && (currentOption[n-1].type == Button || currentOption[n-1].type == SaveButton)) currentOption[n].min = 1; // OK on same line */
        currentOption[n].type = EndMark; currentOption[n].target = NULL; // delimit list by callback-less end mark
    }    

    dialog = gtk_dialog_new_with_buttons( title,
                                      NULL,
                                      dlgNr ? GTK_DIALOG_DESTROY_WITH_PARENT : GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,                                             
                                      GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
                                      GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
                                      NULL );      

    shellsGTK[dlgNr] = dialog;
    box = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );
    gtk_box_set_spacing(GTK_BOX(box), 5);    

    arraysize = 0;
    for (i=0;option[i].type != EndMark;i++) {
        arraysize++;   
    }

    table = gtk_table_new(arraysize, 3, FALSE);
    gtk_table_set_col_spacings(GTK_TABLE(table), 20);
    left = 0;
    top = -1;    

    for (i=0;option[i].type != EndMark;i++) {
        top++;
        if (top >= height) {
            top = 0;
            left = left + 3;
            gtk_table_resize(GTK_TABLE(table), height, left + 3);   
        }                
        switch(option[i].type) {
          case Fractional:           
	    snprintf(def, MSG_SIZ,  "%.2f", *(float*)option[i].target);
	    option[i].value = *(float*)option[i].target;
            goto tBox;
          case Spin:
            if(!currentCps) option[i].value = *(int*)option[i].target;
            snprintf(def, MSG_SIZ,  "%d", option[i].value);
          case TextBox:
	  case FileName:            
     	  case PathName:
          tBox:
            label = gtk_label_new(option[i].name);
            /* Left Justify */
            gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);

            /* width */
            w = option[i].type == Spin || option[i].type == Fractional ? 70 : option[i].max ? option[i].max : 205;
	    if(option[i].type == FileName || option[i].type == PathName) w -= 55;

            if (option[i].type==TextBox && option[i].min > 80){                
                textview = gtk_text_view_new();                
                gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textview), GTK_WRAP_WORD);                                
                /* add textview to scrolled window so we have vertical scroll bar */
                sw = gtk_scrolled_window_new(NULL, NULL);
                gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
                gtk_container_add(GTK_CONTAINER(sw), textview);
                gtk_widget_set_size_request(GTK_WIDGET(sw), w, -1);
 
                textbuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));                
                gtk_widget_set_size_request(textview, -1, option[i].min);
                /* check if label is empty */ 
                if (strcmp(option[i].name,"") != 0) {
                    gtk_table_attach_defaults(GTK_TABLE(table), label, left, left+1, top, top+1);
                    gtk_table_attach_defaults(GTK_TABLE(table), sw, left+1, left+3, top, top+1);
                }
                else {
                    /* no label so let textview occupy all columns */
                    gtk_table_attach_defaults(GTK_TABLE(table), sw, left, left+3, top, top+1);
                } 
                if ( *(char**)option[i].target != NULL )
                    gtk_text_buffer_set_text (textbuffer, *(char**)option[i].target, -1);
                else
                    gtk_text_buffer_set_text (textbuffer, "", -1); 
                option[i].handle = (void*)textbuffer;
                break; 
            }

            entry = gtk_entry_new();

            if (option[i].type==Spin || option[i].type==Fractional)
                gtk_entry_set_text (GTK_ENTRY (entry), def);
            else if (currentCps)
                gtk_entry_set_text (GTK_ENTRY (entry), option[i].textValue);
            else if ( *(char**)option[i].target != NULL )
                gtk_entry_set_text (GTK_ENTRY (entry), *(char**)option[i].target);            

            //gtk_entry_set_width_chars (GTK_ENTRY (entry), 18);
            gtk_entry_set_max_length (GTK_ENTRY (entry), w);

            // left, right, top, bottom
            if (strcmp(option[i].name, "") != 0) gtk_table_attach_defaults(GTK_TABLE(table), label, left, left+1, top, top+1);
            //gtk_table_attach_defaults(GTK_TABLE(table), entry, 1, 2, i, i+1);            

            if (option[i].type == Spin) {                
                spinner_adj = (GtkAdjustment *) gtk_adjustment_new (option[i].value, option[i].min, option[i].max, 1.0, 0.0, 0.0);
                spinner = gtk_spin_button_new (spinner_adj, 1.0, 0);
                gtk_table_attach_defaults(GTK_TABLE(table), spinner, left+1, left+3, top, top+1);
                option[i].handle = (void*)spinner;
            }
            else if (option[i].type == FileName || option[i].type == PathName) {
                gtk_table_attach_defaults(GTK_TABLE(table), entry, left+1, left+2, top, top+1);
                button = gtk_button_new_with_label ("Browse");
                gtk_table_attach_defaults(GTK_TABLE(table), button, left+2, left+3, top, top+1);
                g_signal_connect (button, "clicked", G_CALLBACK (Browse), (gpointer)(intptr_t) i);
                option[i].handle = (void*)entry;                 
            }
            else {
                hbox = gtk_hbox_new (FALSE, 0);
                if (strcmp(option[i].name, "") == 0)
                    gtk_table_attach_defaults(GTK_TABLE(table), hbox, left, left+3, top, top+1);
                else
                    gtk_table_attach_defaults(GTK_TABLE(table), hbox, left+1, left+3, top, top+1);
                gtk_box_pack_start (GTK_BOX (hbox), entry, TRUE, TRUE, 0);
                //gtk_table_attach_defaults(GTK_TABLE(table), entry, left+1, left+3, top, top+1); 
                option[i].handle = (void*)entry;
            }                        		
            break;
          case CheckBox:
            checkbutton = gtk_check_button_new_with_label(option[i].name);            
            if(!currentCps) option[i].value = *(Boolean*)option[i].target;
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbutton), option[i].value);
            gtk_table_attach_defaults(GTK_TABLE(table), checkbutton, left, left+3, top, top+1);                            
            option[i].handle = (void *)checkbutton;            
            break; 
	  case Label:            
            label = gtk_label_new(option[i].name);
            /* Left Justify */
            gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
            gtk_table_attach_defaults(GTK_TABLE(table), label, left, left+3, top, top+1);                       
	    break;
          case SaveButton:
          case Button:
            button = gtk_button_new_with_label (option[i].name);

            /* set button color on view board dialog */
            if(option[i].choice && ((char*)option[i].choice)[0] == '#' && !currentCps) {
                gdk_color_parse( *(char**) option[i-1].target, &color );
                gtk_widget_modify_bg ( GTK_WIDGET(button), GTK_STATE_NORMAL, &color );
	    }

            /* set button color on new variant dialog */
            if(option[i].textValue) {
                gdk_color_parse( option[i].textValue, &color );
                gtk_widget_modify_bg ( GTK_WIDGET(button), GTK_STATE_NORMAL, &color );
                gtk_widget_set_sensitive(button, appData.noChessProgram || option[i].value < 0
					 || strstr(first.variants, VariantName(option[i].value)));                 
            }
            
            if (!(option[i].min & 1)) {
               if(option[i].textValue) // for new variant dialog give buttons equal space so they line up nicely
                   hbox = gtk_hbox_new (TRUE, 0);
               else
                   hbox = gtk_hbox_new (FALSE, 0);
               // if only 1 button then put it in 1st column of table only
               if ( (arraysize >= (i+1)) && option[i+1].type != Button )
                   gtk_table_attach_defaults(GTK_TABLE(table), hbox, left, left+1, top, top+1);
               else
                   gtk_table_attach_defaults(GTK_TABLE(table), hbox, left, left+3, top, top+1);
            }            
            gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);           
            g_signal_connect (button, "clicked", G_CALLBACK (GenericCallback), (gpointer)(intptr_t) i + (dlgNr<<16));           
            option[i].handle = (void*)button;            
            break;  
	  case ComboBox:
            label = gtk_label_new(option[i].name);
            /* Left Justify */
            gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
            gtk_table_attach_defaults(GTK_TABLE(table), label, left, left+1, top, top+1);

            combobox = gtk_combo_box_new_text();            

            for(j=0;;j++) {
               if (  ((char **) option[i].textValue)[j] == NULL) break;
               gtk_combo_box_append_text(GTK_COMBO_BOX(combobox), ((char **) option[i].textValue)[j]);                          
            }

            if(currentCps)
                option[i].choice = (char**) option[i].textValue;
            else {            
                for(j=0; option[i].choice[j]; j++) {                
                    if(*(char**)option[i].target && !strcmp(*(char**)option[i].target, option[i].choice[j])) break;
                }
                /* If choice is NULL set to first */
                if (option[i].choice[j] == NULL)
                   option[i].value = 0;
                else 
                   option[i].value = j;
            }

            //option[i].value = j + (option[i].choice[j] == NULL);            
            gtk_combo_box_set_active(GTK_COMBO_BOX(combobox), option[i].value); 
            

            hbox = gtk_hbox_new (FALSE, 0);
            gtk_table_attach_defaults(GTK_TABLE(table), hbox, left+1, left+3, top, top+1);
            gtk_box_pack_start (GTK_BOX (hbox), combobox, TRUE, TRUE, 0);
            //gtk_table_attach_defaults(GTK_TABLE(table), combobox, 1, 2, i, i+1);

            g_signal_connect(G_OBJECT(combobox), "changed", G_CALLBACK(ComboSelect), (gpointer) (intptr_t) i);

            option[i].handle = (void*)combobox;
            values[i] = option[i].value;            
            break;
	  case Break:
            top = height; // force next option to start in a new column
            break; 
	default:
	    printf("GenericPopUp: unexpected case in switch. i=%d type=%d name=%s.\n", i, option[i].type, option[i].name);
	    break;
	}        
    }

    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox),
                        table, TRUE, TRUE, 0);    

    /* Show dialog */
    gtk_widget_show_all( dialog );    

    /* hide OK/cancel buttons */
    if((option[i].min & 2)) {
        actionarea = gtk_dialog_get_action_area(GTK_DIALOG(dialog));
        gtk_widget_hide(actionarea);
    }

    g_signal_connect (dialog, "response",
                      G_CALLBACK (GenericPopUpCallback),
                      (gpointer)(intptr_t) dlgNr);
    g_signal_connect (dialog, "delete-event",
                      G_CALLBACK (GenericPopDown),
                      (gpointer)(intptr_t) dlgNr);
    shellUp[dlgNr] = True;

    return 1;
}

void IcsOptionsProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    GenericPopUp(icsOptions, _("ICS Options"), 0);
}

void LoadOptionsProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    ASSIGN(searchMode, modeValues[appData.searchMode-1]);
    GenericPopUp(loadOptions, _("Load Game Options"), 0);
}

void SaveOptionsProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    GenericPopUp(saveOptions, _("Save Game Options"), 0);
}

void SoundOptionsProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
   soundFiles[2] = "*";   
   GenericPopUp(soundOptions, _("Sound Options"), 0);
}

void BoardOptionsProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    GenericPopUp(boardOptions, _("Board Options"), 0);
}

void EngineMenuProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    GenericPopUp(adjudicationOptions, "Adjudicate non-ICS Games", 0);
}

void UciMenuProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
   oldCores = appData.smpCores;
   oldPonder = appData.ponderNextMove;   
   GenericPopUp(commonEngineOptions, _("Common Engine Settings"), 0);
}

void NewVariantProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    GenericPopUp(variantDescriptors, _("New Variant"), 0);
}

void OptionsProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
   oldPonder = appData.ponderNextMove;   
   GenericPopUp(generalOptions, _("General Options"), 0);   
}

void MatchOptionsProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
   NamesToList(firstChessProgramNames, engineList, engineMnemonic);
   comboCallback = &AddToTourney;
   matchOptions[5].min = -(appData.pairingEngine[0] != NULLCHAR); // with pairing engine, allow Swiss
   ASSIGN(tfName, appData.tourneyFile[0] ? appData.tourneyFile : MakeName(appData.defName));   
   ASSIGN(engineName, appData.participants);
   GenericPopUp(matchOptions, _("Match Options"), 0);
}

Option textOptions[100];
void PutText P((char *text, int pos));

void SendString(char *p)
{
    char buf[MSG_SIZ], *q;

    if((q = strstr(p, "$input"))) {
	if(!shellUp[4]) return;
	strncpy(buf, p, MSG_SIZ);
	strncpy(buf + (q-p), q+6, MSG_SIZ-(q-p));
	PutText(buf, q-p);
	return;
    }
    snprintf(buf, MSG_SIZ, "%s\n", p);
    SendToICS(buf);
}

void SendText(int n)
{
    char *p = (char*) textOptions[n].choice;
    gchar *name, *q;
    char buf[MSG_SIZ];

    if(strstr(p, "$name")) {
        // get the text selected in the terminal window and substitute it for $name        
        GtkClipboard *cb;
        GdkDisplay *gdisp = gdk_display_get_default();
        if (gdisp == NULL) return;
        cb = gtk_clipboard_get_for_display(gdisp, GDK_SELECTION_PRIMARY);
        name = gtk_clipboard_wait_for_text(cb);
        if (name == NULL) return;        
        strncpy(buf, p, MSG_SIZ);
        q = strstr(p, "$name");
        snprintf(buf + (q-p), MSG_SIZ -(q-p), "%s%s", name, q+5);
        g_free(name);
        SendString(buf);
    } else{
        SendString(p);
    }
}

void IcsTextProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
   int i=0, j;
   char *p, *q, *r;

   if((p = icsTextMenuString) == NULL) return;
   do {
	q = r = p; while(*p && *p != ';') p++;
	for(j=0; j<p-q; j++) textOptions[i].name[j] = *r++;
	textOptions[i].name[j++] = 0;
	if(!*p) break;
	if(*++p == '\n') p++; // optional linefeed after button-text terminating semicolon
	q = p;
	textOptions[i].choice = (char**) (r = textOptions[i].name + j);
	while(*p && (*p != ';' || p[1] != '\n')) textOptions[i].name[j++] = *p++;
	textOptions[i].name[j++] = 0;
	if(*p) p += 2;
	textOptions[i].max = 135;
	textOptions[i].min = i&1;
	textOptions[i].handle = NULL;
	textOptions[i].target = &SendText;
	textOptions[i].textValue = strstr(r, "$input") ? "#80FF80" : strstr(r, "$name") ? "#FF8080" : "#FFFFFF";
	textOptions[i].type = Button;
   } while(++i < 99 && *p);
   if(i == 0) return;
   textOptions[i].type = EndMark;
   textOptions[i].target = NULL;
   textOptions[i].min = 2;   
   SetCheckMenuItemActive(NULL, 3, True); // set GTK menu item to checked   
   GenericPopUp(textOptions, _("ICS text menu"), 3);
}

static char *commentText;
static int commentIndex;
void ClearComment P((int n));

int NewComCallback(int n)
{
    ReplaceComment(commentIndex, commentText);
    return 1;
}

void SaveChanges(int n)
{
    GenericReadout(0);
    ReplaceComment(commentIndex, commentText);
}

Option commentOptions[] = {
{ 0xD, 200, 250, NULL, (void*) &commentText, "", NULL, TextBox, "" },
{   0,  0,   50, NULL, (void*) &ClearComment, NULL, NULL, Button, "clear" },
{   0,  1,  100, NULL, (void*) &SaveChanges, NULL, NULL, Button, "save changes" },
{   0,  1,    0, NULL, (void*) &NewComCallback, "", NULL, EndMark , "" }
};

void ClearTextWidget(GtkWidget *w)
{
    if (!GTK_IS_TEXT_BUFFER(w)) {
        printf("error: ClearTextWidget arg is not a GtkTextBuffer\n");
        return;
    }    
    gtk_text_buffer_set_text(GTK_TEXT_BUFFER(w), "", -1);
}

void ClearComment(int n)
{
    ClearTextWidget(commentOptions[0].handle);    
}

gboolean NewCommentCB(w, eventbutton, gptr)
     GtkWidget *w;
     GdkEventButton  *eventbutton;
     gpointer  gptr;
{
    GtkTextBuffer *tb;
 
    /* if not a right-click then propagate to default handler  */
    if (eventbutton->type != GDK_BUTTON_PRESS || eventbutton->button !=3) return False;

    /* get textbuffer */
    tb = GTK_TEXT_BUFFER(gptr);

    /* user has right clicked in the textbox */
    /* call CommentClick in xboard.c */
    CommentClick(tb);

    return True; /* don't propagate right click to default handler */   
}

/* get textview widget from genericpopup dialog */
/* called by NewCommentPopup in xoptions.c and */
/* HistoryPopUp in xhistory.c */
GtkWidget *GetTextView(dialog)
    GtkWidget *dialog;
{
    GtkWidget *w;
    GList *gl, *g;
   
    /* Find the dialogs GtkTextView widget */
    //dialog = shellsGTK[1];
    w = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    if (!GTK_IS_VBOX(w)) {
        printf("warning - dialog vbox not found\n");
        return NULL;
    }    

    /* get gtkTable */
    gl = gtk_container_get_children(GTK_CONTAINER(w));
    w = GTK_WIDGET(gl->data);
    g_list_free(gl);
    if (!GTK_IS_TABLE(w)) {
        printf("warning dialog table not found\n");
        return NULL;
    }

    /* get scrolled window */
    gl = gtk_container_get_children(GTK_CONTAINER(w));
    //listlen = g_list_length(gl);
    g = gl;
    w = NULL;
    while (g) {       
        w = GTK_WIDGET(g->data);        
        if (GTK_IS_SCROLLED_WINDOW(w)) break;
        w = NULL;        
        g = g->next;
    }
    g_list_free(gl);

    if (!w) {
        printf("warning dialog scrolled window not found\n");
        return NULL;
    }

    /* get GtkTextView */
    gl = gtk_container_get_children(GTK_CONTAINER(w));
    w = GTK_WIDGET(gl->data);
    g_list_free(gl);
    if (!GTK_IS_TEXT_VIEW(w)) {    
        printf("warning dialog textview not found\n");
        return NULL;
    }
    return w;
}

void NewCommentPopup(char *title, char *text, int index)
{    
    GtkWidget *textview;

    if(shellsGTK[1]) { // if already exists, alter title and content
        //XtSetArg(args[0], XtNtitle, title);
        //XtSetValues(shells[1], args, 1);
        SetWidgetTextGTK(commentOptions[0].handle, text);
    }

    if(commentText) free(commentText); commentText = strdup(text);
    commentIndex = index;    
    SetCheckMenuItemActive(NULL, 1, True); // ensure check box is checked on GTK menu item

    if(!GenericPopUp(commentOptions, title, 1)) return;   

    textview = GetTextView(shellsGTK[1]);

    if (!textview) return;

    /* connect to TextView so we can check for a right-click */
    g_signal_connect (GTK_TEXT_VIEW(textview), "button-press-event",
                      G_CALLBACK (NewCommentCB),
                      (gpointer) commentOptions[0].handle);   
}

static char *tagsText;

int NewTagsCallback(int n)
{
    ReplaceTags(tagsText, &gameInfo);
    return 1;
}

void changeTags(int n)
{
    GenericReadout(1);
    if(bookUp) SaveToBook(tagsText); else
    ReplaceTags(tagsText, &gameInfo);
}

Option tagsOptions[] = {
{   0,  0,    0, NULL, NULL, NULL, NULL, Label,  "" },
{ 0xD, 200, 200, NULL, (void*) &tagsText, "", NULL, TextBox, "" },
{   0,  0,  100, NULL, (void*) &changeTags, NULL, NULL, Button, "save changes" },
{   0,  1,    0, NULL, (void*) &NewTagsCallback, "", NULL, EndMark , "" }
};

void NewTagsPopup(char *text, char *msg)
{    
    //Arg args[16];
    char *title = bookUp ? _("Edit book") : _("Tags");

    if(shellsGTK[2]) { // if already exists, alter title and content
	SetWidgetTextGTK(tagsOptions[1].handle, text);
	//XtSetArg(args[0], XtNtitle, title);
	//XtSetValues(shells[2], args, 1);
    }
    if(tagsText) free(tagsText); tagsText = strdup(text);
    tagsOptions[0].textValue = msg;   
    SetCheckMenuItemActive(NULL, 2, True); // ensure check box is checked on GTK menu item
    GenericPopUp(tagsOptions, title, 2);
}

char *icsText;

Option boxOptions[] = {
{   0, 30,  400, NULL, (void*) &icsText, "", NULL, TextBox, "" },
{   0,  3,    0, NULL, NULL, "", NULL, EndMark , "" }
};


void PutText(char *text, int pos)
{   
    char buf[MSG_SIZ]; 
    const gchar *p;
    GtkEntry *edit; 

    edit = boxOptions[0].handle;
    if (!GTK_IS_ENTRY(edit)) return;

    if(strstr(text, "$add ") == text) {        
        p = gtk_entry_get_text(GTK_ENTRY(edit));    
	snprintf(buf, MSG_SIZ, "%s%s", p, text+5); text = buf;
	pos += strlen(p) - 5;
    }
    gtk_entry_set_text(GTK_ENTRY(edit), text);
}

void ICSInputSendText P((void));
char *PrevInHistory P((char *cmd));
char *NextInHistory P((void));

gboolean keypressicsCB(w, eventkey, data)
     GtkWidget *w;
     GdkEventKey  *eventkey;
     gpointer  data;
{
    GtkEntry *edit;
    String val;    

    if (!shellUp[4]) return True;

    switch(eventkey-> keyval) {
      case GDK_Return:
        if (shellUp[4] == True)
            ICSInputSendText(); 
        break;             
      case GDK_Up:                
        edit = boxOptions[0].handle;
        val = (String)gtk_entry_get_text (GTK_ENTRY (edit));        
        val = PrevInHistory(val);        
        /* clear the text in the GTKEntry */        
        gtk_entry_set_text(edit, "");    
        if(val) {
            gtk_entry_set_text (GTK_ENTRY (edit), val);
        }
        break;
      case GDK_Down:        
        edit = boxOptions[0].handle;
        val = NextInHistory();
        /* clear the text in the GTKEntry */        
        gtk_entry_set_text(edit, "");    
        if(val) {
            gtk_entry_set_text (GTK_ENTRY (edit), val);
        }
        break;  
    default:
      //printf("keypressicsCB: unexpected case in switch %d.\n", currentOption[i].type);
      break;
    }

    return False; /* propagate to default handler */

}

void InputBoxPopup()
{   
    if(!GenericPopUp(boxOptions, _("ICS input box"), 4)) return;   
    g_signal_connect (boxOptions[0].handle, "key-press-event",
                      G_CALLBACK (keypressicsCB),
                      (gpointer)(intptr_t) "1");    
}

void activateCB(entry, data)
     GtkEntry *entry;
     gpointer  data;
{
    const gchar *val;
   
    val = gtk_entry_get_text(GTK_ENTRY(entry));
    TypeInDoneEvent((char*)val);    

    PopDown(0);
}

void PopUpMoveDialog(char firstchar)
{    
    static char buf[2];   
    buf[0] = firstchar;buf[1]='\0'; icsText = buf;
    
    if (!GenericPopUp(boxOptions, _("Type a move"), 0)) return;
    gtk_editable_set_position(boxOptions[0].handle, 1);
    g_signal_connect (boxOptions[0].handle, "activate",
                      G_CALLBACK (activateCB), NULL);                     
}

void MoveTypeInProc(eventkey)
    GdkEventKey  *eventkey;
{
    char buf[10];
    buf[0]=eventkey->keyval;
    buf[1]='\0';
    if (*buf > 32)        
        PopUpMoveDialog(*buf);
    
/* old Xt version
    char buf[10], keys[32];
    KeySym sym;
    KeyCode metaL, metaR;
    int n = XLookupString(&(event->xkey), buf, 10, &sym, NULL);
    XQueryKeymap(xDisplay,keys);
    metaL = XKeysymToKeycode(xDisplay, XK_Meta_L);
    metaR = XKeysymToKeycode(xDisplay, XK_Meta_R);
    if ( n == 1 && *buf > 32 && !(keys[metaL>>3]&1<<(metaL&7)) && !(keys[metaR>>3]&1<<(metaR&7))) // printable, no alt        
        PopUpMoveDialog(*buf);


//    if ( n == 1 && *buf >= 32 && !(keys[metaL>>3]&1<<(metaL&7)) && !(keys[metaR>>3]&1<<(metaR&7))) { // printable, no alt
//	if(appData.icsActive) { // text typed to board in ICS mode: divert to ICS input box
//	    if(shells[4]) { // box already exists: append to current contents
//		char *p, newText[MSG_SIZ];
//		GetWidgetText(&boxOptions[0], &p);
//		snprintf(newText, MSG_SIZ, "%s%c", p, *buf);
//		SetWidgetText(&boxOptions[0], newText, 4);
//		if(shellUp[4]) XSetInputFocus(xDisplay, XtWindow(boxOptions[0].handle), RevertToPointerRoot, CurrentTime); //why???
//	    } else icsText = buf; // box did not exist: make sure it pops up with char in it
//	    InputBoxPopup();
//	} else PopUpMoveDialog(*buf);
//    }
*/
}

void
SettingsPopUp(ChessProgramState *cps)
{
   currentCps = cps;   
   GenericPopUp(cps->option, _("Engine Settings"), 0);
}

void FirstSettingsProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    SettingsPopUp(&first);
}

void SecondSettingsProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
   if(WaitForEngine(&second, SettingsMenuIfReady)) return;
   SettingsPopUp(&second);
}

int InstallOK(int n)
{
    PopDown(0); // early popdown, to allow FreezeUI to instate grab
    if(engineChoice[0] == engineNr[0][0])  Load(&first, 0); else Load(&second, 1);
    return 1;
}

Option installOptions[] = {
{   0,  0,    0, NULL, (void*) &engineLine, (char*) engineMnemonic, engineList, ComboBox, N_("Select engine from list:") },
{   0,  0,    0, NULL, NULL, NULL, NULL, Label, N_("or specify one below:") },
{   0,  0,    0, NULL, (void*) &nickName, NULL, NULL, TextBox, N_("Nickname (optional):") },
{   0,  0,    0, NULL, (void*) &useNick, NULL, NULL, CheckBox, N_("Use nickname in PGN player tags of engine-engine games") },
{   0,  0,    0, NULL, (void*) &engineDir, NULL, NULL, PathName, N_("Engine Directory:") },
{   0,  0,    0, NULL, (void*) &engineName, NULL, NULL, FileName, N_("Engine Command:") },
{   0,  0,    0, NULL, NULL, NULL, NULL, Label, N_("(Directory will be derived from engine path when empty)") },
{   0,  0,    0, NULL, (void*) &isUCI, NULL, NULL, CheckBox, N_("UCI") },
{   0,  0,    0, NULL, (void*) &v1, NULL, NULL, CheckBox, N_("WB protocol v1 (do not wait for engine features)") },
{   0,  0,    0, NULL, (void*) &hasBook, NULL, NULL, CheckBox, N_("Must not use GUI book") },
{   0,  0,    0, NULL, (void*) &addToList, NULL, NULL, CheckBox, N_("Add this engine to the list") },
{   0,  0,    0, NULL, (void*) &storeVariant, NULL, NULL, CheckBox, N_("Force current variant with this engine") },
{   0,  0,    0, NULL, (void*) &engineChoice, (char*) engineNr, engineNr, ComboBox, N_("Load mentioned engine as") },
{   0,  1,    0, NULL, (void*) &InstallOK, "", NULL, EndMark , "" }
};

void LoadEngineProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
   isUCI = storeVariant = v1 = useNick = False; addToList = hasBook = True; // defaults
   if(engineChoice) free(engineChoice); engineChoice = strdup(engineNr[0]);
   if(engineLine)   free(engineLine);   engineLine = strdup("");
   if(engineDir)    free(engineDir);    engineDir = strdup("");
   if(nickName)     free(nickName);     nickName = strdup("");
   if(params)       free(params);       params = strdup("");
   NamesToList(firstChessProgramNames, engineList, engineMnemonic);   
   GenericPopUp(installOptions, _("Load engine"), 0);
}

void EditBookProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    EditBookEvent();
}

void SetRandom P((int n));

int ShuffleOK(int n)
{
    ResetGameEvent();
    return 1;
}

Option shuffleOptions[] = {
{   0,  0,   50, NULL, (void*) &shuffleOpenings, NULL, NULL, CheckBox, "shuffle" },
{ 0,-1,2000000000, NULL, (void*) &appData.defaultFrcPosition, "", NULL, Spin, N_("Start-position number:") },
{   0,  0,    0, NULL, (void*) &SetRandom, NULL, NULL, Button, "randomize" },
{   0,  1,    0, NULL, (void*) &SetRandom, NULL, NULL, Button, "pick fixed" },
{   0,  1,    0, NULL, (void*) &ShuffleOK, "", NULL, EndMark , "" }
};

void SetRandom(int n)
{
    int r = n==2 ? -1 : rand() & ((1<<30)-1);

    gtk_spin_button_set_value (shuffleOptions[1].handle, r);  
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(shuffleOptions[0].handle), True);
}

void ShuffleMenuProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
    GenericPopUp(shuffleOptions, _("New Shuffle Game"), 0);
}

int tmpMoves, tmpTc, tmpInc, tmpOdds1, tmpOdds2, tcType;

void ShowTC(int n)
{
}

void SetTcType P((int n));

char *Value(int n)
{
	static char buf[MSG_SIZ];
	snprintf(buf, MSG_SIZ, "%d", n);
	return buf;
}

int TcOK(int n)
{
    char tc[10];
    int x;

    if(tcType == 0 && tmpMoves <= 0) return 0;
    if(tcType == 2 && tmpInc <= 0) return 0;
    /*GetWidgetText(&currentOption[4], &tc); // get original text, in case it is min:sec    */
    searchTime = 0;
    /* can't have min:sec on gtk spin button */
    x = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(currentOption[4].handle));
    snprintf(tc, 10,  "%d", x);
    switch(tcType) {
      case 0:
	if(!ParseTimeControl(tc, -1, tmpMoves)) return 0;
	appData.movesPerSession = tmpMoves;
	ASSIGN(appData.timeControl, tc);
	appData.timeIncrement = -1;
	break;
      case 1:
	if(!ParseTimeControl(tc, tmpInc, 0)) return 0;
	ASSIGN(appData.timeControl, tc);
	appData.timeIncrement = tmpInc;
	break;
      case 2:
	searchTime = tmpInc;
    }
    appData.firstTimeOdds = first.timeOdds = tmpOdds1;
    appData.secondTimeOdds = second.timeOdds = tmpOdds2;
    Reset(True, True);
    return 1;
}

Option tcOptions[] = {
{   0,  0,    0, NULL, (void*) &SetTcType, NULL, NULL, Button, N_("classical") },
{   0,  1,    0, NULL, (void*) &SetTcType, NULL, NULL, Button, N_("incremental") },
{   0,  1,    0, NULL, (void*) &SetTcType, NULL, NULL, Button, N_("fixed max") },
{   0,  0,  200, NULL, (void*) &tmpMoves, NULL, NULL, Spin, N_("Moves per session:") },
{   0,  0,10000, NULL, (void*) &tmpTc, NULL, NULL, Spin, N_("Initial time (min):") },
{   0, 0, 10000, NULL, (void*) &tmpInc, NULL, NULL, Spin, N_("Increment or max (sec/move):") },
{   0,  0,    0, NULL, NULL, NULL, NULL, Label, N_("Time-Odds factors:") },
{   0,  1, 1000, NULL, (void*) &tmpOdds1, NULL, NULL, Spin, N_("Engine #1") },
{   0,  1, 1000, NULL, (void*) &tmpOdds2, NULL, NULL, Spin, N_("Engine #2 / Human") },
{   0,  0,    0, NULL, (void*) &TcOK, "", NULL, EndMark , "" }
};

void SetTcType(int n)
{
    switch(tcType = n) {
      case 0:        
	SetSpinValue(&tcOptions[3], tmpMoves, 0);
	SetSpinValue(&tcOptions[4], tmpTc, 0);
	SetSpinValue(&tcOptions[5], -1, 0);
	break;
      case 1:        
	SetSpinValue(&tcOptions[3], -1, 0);
	SetSpinValue(&tcOptions[4], tmpTc, 0);
	SetSpinValue(&tcOptions[5], tmpInc, 0);
	break;
      case 2:        
	SetSpinValue(&tcOptions[3], -1, 0);
	SetSpinValue(&tcOptions[4], -1, 0);
	SetSpinValue(&tcOptions[5], tmpInc, 0);
    }
}

void TimeControlProcGTK(object, user_data)
     GtkObject *object;
     gpointer user_data;
{
   tmpMoves = appData.movesPerSession;
   tmpInc = appData.timeIncrement; if(tmpInc < 0) tmpInc = 0;
   tmpOdds1 = tmpOdds2 = 1; tcType = 0;
   tmpTc = atoi(appData.timeControl);   
   GenericPopUp(tcOptions, _("Time Control"), 0);
}

//---------------------------- Chat Windows ----------------------------------------------

void OutputChatMessage(int partner, char *mess)
{
    return; // dummy
}

