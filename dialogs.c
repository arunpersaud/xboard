/*
 * dialogs.c -- platform-independent code for dialogs of XBoard
 *
 * Copyright 2000, 2009, 2010, 2011, 2012, 2013, 2014 Free Software Foundation, Inc.
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
#include "frontend.h"
#include "backend.h"
#include "xboard2.h"
#include "menus.h"
#include "dialogs.h"
#include "gettext.h"

#ifdef ENABLE_NLS
# define  _(s) gettext (s)
# define N_(s) gettext_noop (s)
#else
# define  _(s) (s)
# define N_(s)  s
#endif


int values[MAX_OPTIONS];
ChessProgramState *currentCps;

//----------------------------Generic dialog --------------------------------------------

// cloned from Engine Settings dialog (and later merged with it)

char *marked[NrOfDialogs];
Boolean shellUp[NrOfDialogs];

void
MarkMenu (char *item, int dlgNr)
{
    MarkMenuItem(marked[dlgNr] = item, True);
}

void
AddLine (Option *opt, char *s)
{
    AppendText(opt, s);
    AppendText(opt, "\n");
}

//---------------------------------------------- Update dialog controls ------------------------------------

int
SetCurrentComboSelection (Option *opt)
{
    int j;
    if(!opt->textValue) opt->value = *(int*)opt->target; /* numeric */else {
	for(j=0; opt->choice[j]; j++) // look up actual value in list of possible values, to get selection nr
	    if(*(char**)opt->target && !strcmp(*(char**)opt->target, ((char**)opt->textValue)[j])) break;
	opt->value = j + (opt->choice[j] == NULL);
    }
    return opt->value;
}

void
GenericUpdate (Option *opts, int selected)
{
    int i;
    char buf[MSG_SIZ];

    for(i=0; ; i++)
      {
	if(selected >= 0) { if(i < selected) continue; else if(i > selected) break; }
	switch(opts[i].type)
	  {
	  case TextBox:
	  case FileName:
	  case PathName:
	    SetWidgetText(&opts[i],  *(char**) opts[i].target, -1);
	    break;
	  case Spin:
	    sprintf(buf, "%d", *(int*) opts[i].target);
	    SetWidgetText(&opts[i], buf, -1);
	    break;
	  case Fractional:
	    sprintf(buf, "%4.2f", *(float*) opts[i].target);
	    SetWidgetText(&opts[i], buf, -1);
	    break;
	  case CheckBox:
	    SetWidgetState(&opts[i],  *(Boolean*) opts[i].target);
	    break;
	  case ComboBox:
	    if(opts[i].min & COMBO_CALLBACK) break;
	    SetCurrentComboSelection(opts+i);
	    // TODO: actually display this (but it is never used that way...)
	    break;
	  case EndMark:
	    return;
	  default:
	    printf("GenericUpdate: unexpected case in switch.\n");
	  case ListBox:
	  case Button:
	  case SaveButton:
	  case Label:
	  case Break:
	    break;
	  }
      }
}

//------------------------------------------- Read out dialog controls ------------------------------------

int
GenericReadout (Option *opts, int selected)
{
    int i, j, res=1;
    char *val;
    char buf[MSG_SIZ], **dest;
    float x;
	for(i=0; ; i++) { // send all options that had to be OK-ed to engine
	    if(selected >= 0) { if(i < selected) continue; else if(i > selected) break; }
	    switch(opts[i].type) {
		case TextBox:
		case FileName:
		case PathName:
		    GetWidgetText(&opts[i], &val);
		    dest = currentCps ? &(opts[i].textValue) : (char**) opts[i].target;
		    if(*dest == NULL || strcmp(*dest, val)) {
			if(currentCps) {
			    snprintf(buf, MSG_SIZ,  "option %s=%s\n", opts[i].name, val);
			    SendToProgram(buf, currentCps);
			} else {
			    if(*dest) free(*dest);
			    *dest = malloc(strlen(val)+1);
			}
			safeStrCpy(*dest, val, MSG_SIZ - (*dest - opts[i].name)); // copy text there
		    }
		    break;
		case Spin:
		case Fractional:
		    GetWidgetText(&opts[i], &val);
		    x = 0.0; // Initialise because sscanf() will fail if non-numeric text is entered
		    sscanf(val, "%f", &x);
		    if(x > opts[i].max) x = opts[i].max;
		    if(x < opts[i].min) x = opts[i].min;
		    if(opts[i].type == Fractional)
			*(float*) opts[i].target = x; // engines never have float options!
		    else {
			if(currentCps) {
			  if(opts[i].value != x) { // only to engine if changed
			    snprintf(buf, MSG_SIZ,  "option %s=%.0f\n", opts[i].name, x);
			    SendToProgram(buf, currentCps);
			  }
			} else *(int*) opts[i].target = x;
			opts[i].value = x;
		    }
		    break;
		case CheckBox:
		    j = 0;
		    GetWidgetState(&opts[i], &j);
		    if(opts[i].value != j) {
			opts[i].value = j;
			if(currentCps) {
			    snprintf(buf, MSG_SIZ,  "option %s=%d\n", opts[i].name, j);
			    SendToProgram(buf, currentCps);
			} else *(Boolean*) opts[i].target = j;
		    }
		    break;
		case ComboBox:
		    if(opts[i].min & COMBO_CALLBACK) break;
		    if(!opts[i].textValue) { *(int*)opts[i].target = values[i]; break; } // numeric
		    val = ((char**)opts[i].textValue)[values[i]];
		    if(currentCps) {
			if(opts[i].value == values[i]) break; // not changed
			opts[i].value = values[i];
			snprintf(buf, MSG_SIZ,  "option %s=%s\n", opts[i].name,	opts[i].choice[values[i]]);
			SendToProgram(buf, currentCps);
		    } else if(val && (*(char**) opts[i].target == NULL || strcmp(*(char**) opts[i].target, val))) {
		      if(*(char**) opts[i].target) free(*(char**) opts[i].target);
		      *(char**) opts[i].target = strdup(val);
		    }
		    break;
		case EndMark:
		    if(opts[i].target && selected != -2) // callback for implementing necessary actions on OK (like redraw)
			res = ((OKCallback*) opts[i].target)(i);
		    break;
	    default:
		printf("GenericReadout: unexpected case in switch.\n");
		case ListBox:
		case Button:
		case SaveButton:
		case Label:
		case Break:
		case Skip:
	      break;
	    }
	    if(opts[i].type == EndMark) break;
	}
	return res;
}

//------------------------------------------- Match Options ------------------------------------------------------

char *engineName, *engineChoice, *tfName;
char *engineList[MAXENGINES] = {" "}, *engineMnemonic[MAXENGINES];

static void AddToTourney P((int n, int sel));
static void CloneTourney P((void));
static void ReplaceParticipant P((void));
static void UpgradeParticipant P((void));
static void PseudoOK P((void));

static int
MatchOK (int n)
{
    ASSIGN(appData.participants, engineName);
    if(!CreateTourney(tfName) || matchMode) return matchMode || !appData.participants[0];
    PopDown(MasterDlg); // early popdown to prevent FreezeUI called through MatchEvent from causing XtGrab warning
    MatchEvent(2); // start tourney
    return FALSE;  // no double PopDown!
}

static void
DoTimeControl(int n)
{
  TimeControlProc();
}

static void
DoCommonEngine(int n)
{
  UciMenuProc();
}

static void
DoGeneral(int n)
{
  OptionsProc();
}

#define PARTICIPANTS 6 /* This MUST be the number of the Option for &engineName!*/

static Option matchOptions[] = {
{ 0,  0,          0, NULL, (void*) &tfName, ".trn", NULL, FileName, N_("Tournament file:          ") },
{ 0,  0,          0, NULL, NULL, NULL, NULL, Label, N_("For concurrent playing of tourney with multiple XBoards:") },
{ 0,  0,          0, NULL, (void*) &appData.roundSync, "", NULL, CheckBox, N_("Sync after round") },
{ 0,  0,          0, NULL, (void*) &appData.cycleSync, "", NULL, CheckBox, N_("Sync after cycle") },
{ 0,  LR,       175, NULL, NULL, NULL, NULL, Label, N_("Tourney participants:") },
{ 0, SAME_ROW|RR, 175, NULL, NULL, NULL, NULL, Label, N_("Select Engine:") },
{ 200, T_VSCRL | T_FILL | T_WRAP,
                175, NULL, (void*) &engineName, NULL, NULL, TextBox, "" },
{ 200, SAME_ROW|RR,
                175, NULL, (void*) engineMnemonic, (char*) &AddToTourney, NULL, ListBox, "" },
{ 0, SAME_ROW, 0, NULL, NULL, NULL, NULL, Break, "" }, // to decouple alignment above and below boxes
//{ 0,  COMBO_CALLBACK | NO_GETTEXT,
//		  0, NULL, (void*) &AddToTourney, (char*) (engineMnemonic+1), (engineMnemonic+1), ComboBox, N_("Select Engine:") },
{ 0,  0,         10, NULL, (void*) &appData.tourneyType, "", NULL, Spin, N_("Tourney type (0 = round-robin, 1 = gauntlet):") },
{ 0,  1, 1000000000, NULL, (void*) &appData.tourneyCycles, "", NULL, Spin, N_("Number of tourney cycles (or Swiss rounds):") },
{ 0,  1, 1000000000, NULL, (void*) &appData.defaultMatchGames, "", NULL, Spin, N_("Default Number of Games in Match (or Pairing):") },
{ 0,  0, 1000000000, NULL, (void*) &appData.matchPause, "", NULL, Spin, N_("Pause between Match Games (msec):") },
{ 0,  0,          0, NULL, (void*) &appData.saveGameFile, ".pgn .game", NULL, FileName, N_("Save Tourney Games on:") },
{ 0,  0,          0, NULL, (void*) &appData.loadGameFile, ".pgn .game", NULL, FileName, N_("Game File with Opening Lines:") },
{ 0, -2, 1000000000, NULL, (void*) &appData.loadGameIndex, "", NULL, Spin, N_("Game Number (-1 or -2 = Auto-Increment):") },
{ 0,  0,          0, NULL, (void*) &appData.loadPositionFile, ".fen .epd .pos", NULL, FileName, N_("File with Start Positions:") },
{ 0, -2, 1000000000, NULL, (void*) &appData.loadPositionIndex, "", NULL, Spin, N_("Position Number (-1 or -2 = Auto-Increment):") },
{ 0,  0, 1000000000, NULL, (void*) &appData.rewindIndex, "", NULL, Spin, N_("Rewind Index after this many Games (0 = never):") },
{ 0,  0,          0, NULL, (void*) &appData.defNoBook, "", NULL, CheckBox, N_("Disable own engine books by default") },
{ 0,  0,          0, NULL, (void*) &DoTimeControl, NULL, NULL, Button, N_("Time Control") },
{ 0, SAME_ROW,    0, NULL, (void*) &DoCommonEngine, NULL, NULL, Button, N_("Common Engine") },
{ 0, SAME_ROW,    0, NULL, (void*) &DoGeneral, NULL, NULL, Button, N_("General Options") },
{ 0, SAME_ROW,    0, NULL, (void*) &PseudoOK, NULL, NULL, Button, N_("Continue Later") },
{ 0,  0,          0, NULL, (void*) &ReplaceParticipant, NULL, NULL, Button, N_("Replace Engine") },
{ 0, SAME_ROW,    0, NULL, (void*) &UpgradeParticipant, NULL, NULL, Button, N_("Upgrade Engine") },
{ 0, SAME_ROW,    0, NULL, (void*) &CloneTourney, NULL, NULL, Button, N_("Clone Tourney") },
{ 0, SAME_ROW,    0, NULL, (void*) &MatchOK, "", NULL, EndMark , "" }
};

static void
ReplaceParticipant ()
{
    GenericReadout(matchOptions, PARTICIPANTS);
    Substitute(strdup(engineName), True);
}

static void
UpgradeParticipant ()
{
    GenericReadout(matchOptions, PARTICIPANTS);
    Substitute(strdup(engineName), False);
}

static void
PseudoOK ()
{
    GenericReadout(matchOptions, -2); // read all, but suppress calling of MatchOK
    ASSIGN(appData.participants, engineName);
    PopDown(MasterDlg); // early popdown to prevent FreezeUI called through MatchEvent from causing XtGrab warning
}

static void
CloneTourney ()
{
    FILE *f;
    char *name;
    GetWidgetText(matchOptions, &name);
    if(name && name[0] && (f = fopen(name, "r")) ) {
	char *saveSaveFile;
	saveSaveFile = appData.saveGameFile; appData.saveGameFile = NULL; // this is a persistent option, protect from change
	ParseArgsFromFile(f);
	engineName = appData.participants; GenericUpdate(matchOptions, -1);
	FREE(appData.saveGameFile); appData.saveGameFile = saveSaveFile;
    } else DisplayError(_("First you must specify an existing tourney file to clone"), 0);
}

static void
AddToTourney (int n, int sel)
{
    int nr;
    char buf[MSG_SIZ];
    if(sel < 1) buf[0] = NULLCHAR; // back to top level
    else if(engineList[sel][0] == '#') safeStrCpy(buf, engineList[sel], MSG_SIZ); // group header, open group
    else { // normal line, select engine
	AddLine(&matchOptions[PARTICIPANTS], engineMnemonic[sel]);
	return;
    }
    nr = NamesToList(firstChessProgramNames, engineList, engineMnemonic, buf); // replace list by only the group contents
    ASSIGN(engineMnemonic[0], buf);
    LoadListBox(&matchOptions[PARTICIPANTS+1], _("# no engines are installed"), -1, -1);
    HighlightWithScroll(&matchOptions[PARTICIPANTS+1], 0, nr);
}

void
MatchOptionsProc ()
{
   if(matchOptions[PARTICIPANTS+1].type != ListBox) {
	DisplayError(_("Internal error: PARTICIPANTS set wrong"), 0);
	return;
   }
   NamesToList(firstChessProgramNames, engineList, engineMnemonic, "");
   matchOptions[9].min = -(appData.pairingEngine[0] != NULLCHAR); // with pairing engine, allow Swiss
   ASSIGN(tfName, appData.tourneyFile[0] ? appData.tourneyFile : MakeName(appData.defName));
   ASSIGN(engineName, appData.participants);
   ASSIGN(engineMnemonic[0], "");
   GenericPopUp(matchOptions, _("Tournament Options"), MasterDlg, BoardWindow, MODAL, 0);
}

// ------------------------------------------- General Options --------------------------------------------------

static int oldShow, oldBlind, oldPonder;

static int
GeneralOptionsOK (int n)
{
	int newPonder = appData.ponderNextMove;
	appData.ponderNextMove = oldPonder;
	PonderNextMoveEvent(newPonder);
	if(!appData.highlightLastMove) ClearHighlights(), ClearPremoveHighlights();
	if(oldShow != appData.showCoords || oldBlind != appData.blindfold) DrawPosition(TRUE, NULL);
	return 1;
}

static Option generalOptions[] = {
{ 0,  0, 0, NULL, (void*) &appData.whitePOV, "", NULL, CheckBox, N_("Absolute Analysis Scores") },
{ 0,  0, 0, NULL, (void*) &appData.sweepSelect, "", NULL, CheckBox, N_("Almost Always Queen (Detour Under-Promote)") },
{ 0,  0, 0, NULL, (void*) &appData.animateDragging, "", NULL, CheckBox, N_("Animate Dragging") },
{ 0,  0, 0, NULL, (void*) &appData.animate, "", NULL, CheckBox, N_("Animate Moving") },
{ 0,  0, 0, NULL, (void*) &appData.autoCallFlag, "", NULL, CheckBox, N_("Auto Flag") },
{ 0,  0, 0, NULL, (void*) &appData.autoFlipView, "", NULL, CheckBox, N_("Auto Flip View") },
{ 0,  0, 0, NULL, (void*) &appData.blindfold, "", NULL, CheckBox, N_("Blindfold") },
/* TRANSLATORS: the drop menu is used to drop a piece, e.g. during bughouse or editing a position */
{ 0,  0, 0, NULL, (void*) &appData.dropMenu, "", NULL, CheckBox, N_("Drop Menu") },
{ 0,  0, 0, NULL, (void*) &appData.variations, "", NULL, CheckBox, N_("Enable Variation Trees") },
{ 0,  0, 0, NULL, (void*) &appData.headers, "", NULL, CheckBox, N_("Headers in Engine Output Window") },
{ 0,  0, 0, NULL, (void*) &appData.hideThinkingFromHuman, "", NULL, CheckBox, N_("Hide Thinking from Human") },
{ 0,  0, 0, NULL, (void*) &appData.highlightLastMove, "", NULL, CheckBox, N_("Highlight Last Move") },
{ 0,  0, 0, NULL, (void*) &appData.highlightMoveWithArrow, "", NULL, CheckBox, N_("Highlight with Arrow") },
{ 0,  0, 0, NULL, (void*) &appData.oneClick, "", NULL, CheckBox, N_("One-Click Moving") },
{ 0,  0, 0, NULL, (void*) &appData.periodicUpdates, "", NULL, CheckBox, N_("Periodic Updates (in Analysis Mode)") },
{ 0, SAME_ROW, 0, NULL, NULL, NULL, NULL, Break, "" },
{ 0,  0, 0, NULL, (void*) &appData.autoExtend, "", NULL, CheckBox, N_("Play Move(s) of Clicked PV (Analysis)") },
{ 0,  0, 0, NULL, (void*) &appData.ponderNextMove, "", NULL, CheckBox, N_("Ponder Next Move") },
{ 0,  0, 0, NULL, (void*) &appData.popupExitMessage, "", NULL, CheckBox, N_("Popup Exit Messages") },
{ 0,  0, 0, NULL, (void*) &appData.popupMoveErrors, "", NULL, CheckBox, N_("Popup Move Errors") },
{ 0,  0, 0, NULL, (void*) &appData.showEvalInMoveHistory, "", NULL, CheckBox, N_("Scores in Move List") },
{ 0,  0, 0, NULL, (void*) &appData.showCoords, "", NULL, CheckBox, N_("Show Coordinates") },
{ 0,  0, 0, NULL, (void*) &appData.markers, "", NULL, CheckBox, N_("Show Target Squares") },
{ 0,  0, 0, NULL, (void*) &appData.useStickyWindows, "", NULL, CheckBox, N_("Sticky Windows") },
{ 0,  0, 0, NULL, (void*) &appData.testLegality, "", NULL, CheckBox, N_("Test Legality") },
{ 0,  0, 0, NULL, (void*) &appData.topLevel, "", NULL, CheckBox, N_("Top-Level Dialogs") },
{ 0, 0,10,  NULL, (void*) &appData.flashCount, "", NULL, Spin, N_("Flash Moves (0 = no flashing):") },
{ 0, 1,10,  NULL, (void*) &appData.flashRate, "", NULL, Spin, N_("Flash Rate (high = fast):") },
{ 0, 5,100, NULL, (void*) &appData.animSpeed, "", NULL, Spin, N_("Animation Speed (high = slow):") },
{ 0, 1,5,   NULL, (void*) &appData.zoom, "", NULL, Spin, N_("Zoom factor in Evaluation Graph:") },
{ 0,  0, 0, NULL, (void*) &GeneralOptionsOK, "", NULL, EndMark , "" }
};

void
OptionsProc ()
{
   oldPonder = appData.ponderNextMove;
   oldShow = appData.showCoords; oldBlind = appData.blindfold;
   GenericPopUp(generalOptions, _("General Options"), TransientDlg, BoardWindow, MODAL, 0);
}

//---------------------------------------------- New Variant ------------------------------------------------

static void Pick P((int n));

static char warning[MSG_SIZ];
static int ranksTmp, filesTmp, sizeTmp;

static Option variantDescriptors[] = {
{ VariantNormal,        0, 135, NULL, (void*) &Pick, "#FFFFFF", NULL, Button, N_("Normal")},
{ VariantMakruk, SAME_ROW, 135, NULL, (void*) &Pick, "#FFFFFF", NULL, Button, N_("Makruk")},
{ VariantFischeRandom,  0, 135, NULL, (void*) &Pick, "#FFFFFF", NULL, Button, N_("FRC")},
{ VariantShatranj,SAME_ROW,135, NULL, (void*) &Pick, "#FFFFFF", NULL, Button, N_("Shatranj")},
{ VariantWildCastle,    0, 135, NULL, (void*) &Pick, "#FFFFFF", NULL, Button, N_("Wild castle")},
{ VariantKnightmate,SAME_ROW,135,NULL,(void*) &Pick, "#FFFFFF", NULL, Button, N_("Knightmate")},
{ VariantNoCastle,      0, 135, NULL, (void*) &Pick, "#FFFFFF", NULL, Button, N_("No castle")},
{ VariantCylinder,SAME_ROW,135, NULL, (void*) &Pick, "#FFFFFF", NULL, Button, N_("Cylinder *")},
{ Variant3Check,        0, 135, NULL, (void*) &Pick, "#FFFFFF", NULL, Button, N_("3-checks")},
{ VariantBerolina,SAME_ROW,135, NULL, (void*) &Pick, "#FFFFFF", NULL, Button, N_("berolina *")},
{ VariantAtomic,        0, 135, NULL, (void*) &Pick, "#FFFFFF", NULL, Button, N_("atomic")},
{ VariantTwoKings,SAME_ROW,135, NULL, (void*) &Pick, "#FFFFFF", NULL, Button, N_("two kings")},
{ -1,                   0, 135, NULL, (void*) &Pick, "#FFFFFF", NULL, Button, N_(" ")}, // dummy, to have good alignment
{ VariantSpartan,SAME_ROW, 135, NULL, (void*) &Pick, "#FF0000", NULL, Button, N_("Spartan")},
{ 0, 0, 0, NULL, NULL, NULL, NULL, Label, N_("Board size ( -1 = default for selected variant):")},
{ 0, -1, BOARD_RANKS-1, NULL, (void*) &ranksTmp, "", NULL, Spin, N_("Number of Board Ranks:") },
{ 0, -1, BOARD_FILES,   NULL, (void*) &filesTmp, "", NULL, Spin, N_("Number of Board Files:") },
{ 0, -1, BOARD_RANKS-1, NULL, (void*) &sizeTmp,  "", NULL, Spin, N_("Holdings Size:") },
{ 0, 0, 275, NULL, NULL, NULL, NULL, Label, warning },
{ 0, 0, 275, NULL, NULL, NULL, NULL, Label, N_("Variants marked with * can only be played\nwith legality testing off.")},
{ 0, SAME_ROW, 0, NULL, NULL, NULL, NULL, Break, ""},
{ VariantASEAN,         0, 135, NULL, (void*) &Pick, "#FFFFFF", NULL, Button, N_("ASEAN")},
{ VariantGreat,  SAME_ROW, 135, NULL, (void*) &Pick, "#BFBFFF", NULL, Button, N_("Great Shatranj (10x8)")},
{ VariantSChess,        0, 135, NULL, (void*) &Pick, "#FFBFBF", NULL, Button, N_("Seirawan")},
{ VariantFalcon, SAME_ROW, 135, NULL, (void*) &Pick, "#BFBFFF", NULL, Button, N_("Falcon (10x8)")},
{ VariantSuper,         0, 135, NULL, (void*) &Pick, "#FFBFBF", NULL, Button, N_("Superchess")},
{ VariantCapablanca,SAME_ROW,135,NULL,(void*) &Pick, "#BFBFFF", NULL, Button, N_("Capablanca (10x8)")},
{ VariantCrazyhouse,    0, 135, NULL, (void*) &Pick, "#FFBFBF", NULL, Button, N_("Crazyhouse")},
{ VariantGothic, SAME_ROW, 135, NULL, (void*) &Pick, "#BFBFFF", NULL, Button, N_("Gothic (10x8)")},
{ VariantBughouse,      0, 135, NULL, (void*) &Pick, "#FFBFBF", NULL, Button, N_("Bughouse")},
{ VariantJanus,  SAME_ROW, 135, NULL, (void*) &Pick, "#BFBFFF", NULL, Button, N_("Janus (10x8)")},
{ VariantSuicide,       0, 135, NULL, (void*) &Pick, "#FFFFBF", NULL, Button, N_("Suicide")},
{ VariantCapaRandom,SAME_ROW,135,NULL,(void*) &Pick, "#BFBFFF", NULL, Button, N_("CRC (10x8)")},
{ VariantGiveaway,      0, 135, NULL, (void*) &Pick, "#FFFFBF", NULL, Button, N_("give-away")},
{ VariantGrand,  SAME_ROW, 135, NULL, (void*) &Pick, "#5070FF", NULL, Button, N_("grand (10x10)")},
{ VariantLosers,        0, 135, NULL, (void*) &Pick, "#FFFFBF", NULL, Button, N_("losers")},
{ VariantShogi,  SAME_ROW, 135, NULL, (void*) &Pick, "#BFFFFF", NULL, Button, N_("shogi (9x9)")},
{ VariantFairy,         0, 135, NULL, (void*) &Pick, "#BFBFBF", NULL, Button, N_("fairy")},
{ VariantXiangqi, SAME_ROW,135, NULL, (void*) &Pick, "#BFFFFF", NULL, Button, N_("xiangqi (9x10)")},
{ VariantLion,          0, 135, NULL, (void*) &Pick, "#BFBFBF", NULL, Button, N_("mighty lion")},
{ VariantCourier, SAME_ROW,135, NULL, (void*) &Pick, "#BFFFBF", NULL, Button, N_("courier (12x8)")},
{ VariantChuChess,      0, 135, NULL, (void*) &Pick, "#BFBFBF", NULL, Button, N_("elven chess (10x10)")},
{ VariantChu,    SAME_ROW, 135, NULL, (void*) &Pick, "#BFFFBF", NULL, Button, N_("chu shogi (12x12)")},
//{ -1,                   0, 135, NULL, (void*) &Pick, "#FFFFFF", NULL, Button, N_(" ")}, // dummy, to have good alignment
// optional buttons for engine-defined variants
{ 0, NO_OK, 0, NULL, NULL, "", NULL, EndMark , "" },
{ 0, SAME_ROW, 0, NULL, NULL, NULL, NULL, Skip, ""},
{ VariantUnknown,       0, 135, NULL, (void*) &Pick, "#FFFFFF", NULL, Skip, NULL },
{ VariantUnknown, SAME_ROW,135, NULL, (void*) &Pick, "#FFFFFF", NULL, Skip, NULL },
{ VariantUnknown,       0, 135, NULL, (void*) &Pick, "#FFFFFF", NULL, Skip, NULL },
{ VariantUnknown, SAME_ROW,135, NULL, (void*) &Pick, "#FFFFFF", NULL, Skip, NULL },
{ VariantUnknown,       0, 135, NULL, (void*) &Pick, "#FFFFFF", NULL, Skip, NULL },
{ VariantUnknown, SAME_ROW,135, NULL, (void*) &Pick, "#FFFFFF", NULL, Skip, NULL },
{ VariantUnknown,       0, 135, NULL, (void*) &Pick, "#FFFFFF", NULL, Skip, NULL },
{ VariantUnknown, SAME_ROW,135, NULL, (void*) &Pick, "#FFFFFF", NULL, Skip, NULL },
{ VariantUnknown,       0, 135, NULL, (void*) &Pick, "#FFFFFF", NULL, Skip, NULL },
{ VariantUnknown, SAME_ROW,135, NULL, (void*) &Pick, "#FFFFFF", NULL, Skip, NULL },
{ VariantUnknown,       0, 135, NULL, (void*) &Pick, "#FFFFFF", NULL, Skip, NULL },
{ VariantUnknown, SAME_ROW,135, NULL, (void*) &Pick, "#FFFFFF", NULL, Skip, NULL },
{ VariantUnknown,       0, 135, NULL, (void*) &Pick, "#FFFFFF", NULL, Skip, NULL },
{ VariantUnknown, SAME_ROW,135, NULL, (void*) &Pick, "#FFFFFF", NULL, Skip, NULL },
{ VariantUnknown,       0, 135, NULL, (void*) &Pick, "#FFFFFF", NULL, Skip, NULL },
{ VariantUnknown, SAME_ROW,135, NULL, (void*) &Pick, "#FFFFFF", NULL, Skip, NULL },
{ VariantUnknown,       0, 135, NULL, (void*) &Pick, "#FFFFFF", NULL, Skip, NULL },
{ VariantUnknown, SAME_ROW,135, NULL, (void*) &Pick, "#FFFFFF", NULL, Skip, NULL },
{ VariantUnknown,       0, 135, NULL, (void*) &Pick, "#FFFFFF", NULL, Skip, NULL },
{ VariantUnknown, SAME_ROW,135, NULL, (void*) &Pick, "#FFFFFF", NULL, Skip, NULL },
{ VariantUnknown,       0, 135, NULL, (void*) &Pick, "#FFFFFF", NULL, Skip, NULL },
{ VariantUnknown, SAME_ROW,135, NULL, (void*) &Pick, "#FFFFFF", NULL, Skip, NULL },
{ 0, NO_OK, 0, NULL, NULL, "", NULL, EndMark , "" }
};

static void
Pick (int n)
{
	VariantClass v = variantDescriptors[n].value;
	if(v == VariantUnknown) safeStrCpy(engineVariant, variantDescriptors[n].name, MSG_SIZ); else *engineVariant = NULLCHAR;
	GenericReadout(variantDescriptors, -1); // read new ranks and file settings
	if(!appData.noChessProgram) {
	    char buf[MSG_SIZ];
	    if (!SupportedVariant(first.variants, v, filesTmp, ranksTmp, sizeTmp, first.protocolVersion, first.tidy)) {
		DisplayError(variantError, 0);
		return; /* ignore OK if first engine does not support it */
	    } else
	    if (second.initDone &&
		!SupportedVariant(second.variants, v, filesTmp, ranksTmp, sizeTmp, second.protocolVersion, second.tidy)) {
                snprintf(buf, MSG_SIZ,  _("Warning: second engine (%s) does not support this!"), second.tidy);
		DisplayError(buf, 0);   /* use of second engine is optional; only warn user */
	    }
	}

	gameInfo.variant = v;
	appData.variant = VariantName(v);

	shuffleOpenings = FALSE; /* [HGM] shuffle: possible shuffle reset when we switch */
	startedFromPositionFile = FALSE; /* [HGM] loadPos: no longer valid in new variant */
	appData.fischerCastling = FALSE; /* [HGM] fischer: no longer valid in new variant */
	appData.NrRanks = ranksTmp;
	appData.NrFiles = filesTmp;
	appData.holdingsSize = sizeTmp;
	appData.pieceToCharTable = NULL;
	appData.pieceNickNames = "";
	appData.colorNickNames = "";
        PopDown(TransientDlg);
	Reset(True, True);
        return;
}

void
NewVariantProc ()
{
   static int start;
   int i, last;
   ranksTmp = filesTmp = sizeTmp = -1; // prefer defaults over actual settings
   if(appData.noChessProgram) sprintf(warning, _("Only bughouse is not available in viewer mode.")); else
   sprintf(warning, _("All variants not supported by the first engine\n(currently %s) are disabled."), first.tidy);
   if(!start) {
	while(variantDescriptors[start].type != EndMark) start++; // locate spares
	start += 2; // conditional EndMark and Break
   }
   last = -1;
   for(i=0; variantDescriptors[start+i].type != EndMark; i++) { // create buttons for engine-defined variants
     char *v = EngineDefinedVariant(&first, i);
     if(v) {
	last =  i;
	ASSIGN(variantDescriptors[start+i].name, v);
	variantDescriptors[start+i].type = Button;
     } else variantDescriptors[start+i].type = Skip;
   }
   if(!(last&1)) { // odd number, add filler
	ASSIGN(variantDescriptors[start+last+1].name, " ");
	variantDescriptors[start+last+1].type = Button;
	variantDescriptors[start+last+1].value = Skip;
   }
   variantDescriptors[start-2].type = (last < 0 ? EndMark : Skip);
   variantDescriptors[start-1].type = (last < 6 ? Skip : Break);
   safeStrCpy(engineVariant+100, engineVariant, 100); *engineVariant = NULLCHAR; // yeghh...
   GenericPopUp(variantDescriptors, _("New Variant"), TransientDlg, BoardWindow, MODAL, 0);
   safeStrCpy(engineVariant, engineVariant+100, MSG_SIZ); // must temporarily clear to avoid enabling all variant buttons
}

//------------------------------------------- Common Engine Options -------------------------------------

static int oldCores;
static char *egtPath;

static int
CommonOptionsOK (int n)
{
	int newPonder = appData.ponderNextMove;
	if(*egtPath != '/' && strchr(egtPath, ':')) {
	    ASSIGN(appData.egtFormats, egtPath);
	} else {
	    ASSIGN(appData.defaultPathEGTB, egtPath);
	}
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

static Option commonEngineOptions[] = {
{ 0,  0,    0, NULL, (void*) &appData.ponderNextMove, "", NULL, CheckBox, N_("Ponder Next Move") },
{ 0,  0, 1000, NULL, (void*) &appData.smpCores, "", NULL, Spin, N_("Maximum Number of CPUs per Engine:") },
{ 0,  0,    0, NULL, (void*) &appData.polyglotDir, "", NULL, PathName, N_("Polygot Directory:") },
{ 0,  0,16000, NULL, (void*) &appData.defaultHashSize, "", NULL, Spin, N_("Hash-Table Size (MB):") },
{ 0,  0,    0, NULL, (void*) &egtPath, "", NULL, PathName, N_("EGTB Path:") },
{ 0,  0, 1000, NULL, (void*) &appData.defaultCacheSizeEGTB, "", NULL, Spin, N_("EGTB Cache Size (MB):") },
{ 0,  0,    0, NULL, (void*) &appData.usePolyglotBook, "", NULL, CheckBox, N_("Use GUI Book") },
{ 0,  0,    0, NULL, (void*) &appData.polyglotBook, ".bin", NULL, FileName, N_("Opening-Book Filename:") },
{ 0,  0,  100, NULL, (void*) &appData.bookDepth, "", NULL, Spin, N_("Book Depth (moves):") },
{ 0,  0,  100, NULL, (void*) &appData.bookStrength, "", NULL, Spin, N_("Book Variety (0) vs. Strength (100):") },
{ 0,  0,    0, NULL, (void*) &appData.firstHasOwnBookUCI, "", NULL, CheckBox, N_("Engine #1 Has Own Book") },
{ 0,  0,    0, NULL, (void*) &appData.secondHasOwnBookUCI, "", NULL, CheckBox, N_("Engine #2 Has Own Book          ") },
{ 0,SAME_ROW,0,NULL, (void*) &CommonOptionsOK, "", NULL, EndMark , "" }
};

void
UciMenuProc ()
{
   oldCores = appData.smpCores;
   oldPonder = appData.ponderNextMove;
   if(appData.egtFormats && *appData.egtFormats) { ASSIGN(egtPath, appData.egtFormats); }
   else { ASSIGN(egtPath, appData.defaultPathEGTB); }
   GenericPopUp(commonEngineOptions, _("Common Engine Settings"), TransientDlg, BoardWindow, MODAL, 0);
}

//------------------------------------------ Adjudication Options --------------------------------------

static Option adjudicationOptions[] = {
{ 0, 0,    0, NULL, (void*) &appData.checkMates, "", NULL, CheckBox, N_("Detect all Mates") },
{ 0, 0,    0, NULL, (void*) &appData.testClaims, "", NULL, CheckBox, N_("Verify Engine Result Claims") },
{ 0, 0,    0, NULL, (void*) &appData.materialDraws, "", NULL, CheckBox, N_("Draw if Insufficient Mating Material") },
{ 0, 0,    0, NULL, (void*) &appData.trivialDraws, "", NULL, CheckBox, N_("Adjudicate Trivial Draws (3-Move Delay)") },
{ 0, 0,100,   NULL, (void*) &appData.ruleMoves, "", NULL, Spin, N_("N-Move Rule:") },
{ 0, 0,    6, NULL, (void*) &appData.drawRepeats, "", NULL, Spin, N_("N-fold Repeats:") },
{ 0, 0,1000,  NULL, (void*) &appData.adjudicateDrawMoves, "", NULL, Spin, N_("Draw after N Moves Total:") },
{ 0, -5000,0, NULL, (void*) &appData.adjudicateLossThreshold, "", NULL, Spin, N_("Win / Loss Threshold:") },
{ 0, 0,    0, NULL, (void*) &first.scoreIsAbsolute, "", NULL, CheckBox, N_("Negate Score of Engine #1") },
{ 0, 0,    0, NULL, (void*) &second.scoreIsAbsolute, "", NULL, CheckBox, N_("Negate Score of Engine #2") },
{ 0,SAME_ROW, 0, NULL, NULL, "", NULL, EndMark , "" }
};

void
EngineMenuProc ()
{
   GenericPopUp(adjudicationOptions, _("Adjudicate non-ICS Games"), TransientDlg, BoardWindow, MODAL, 0);
}

//--------------------------------------------- ICS Options ---------------------------------------------

static int
IcsOptionsOK (int n)
{
    ParseIcsTextColors();
    return 1;
}

Option icsOptions[] = {
{ 0, 0, 0, NULL, (void*) &appData.autoKibitz, "",  NULL, CheckBox, N_("Auto-Kibitz") },
{ 0, 0, 0, NULL, (void*) &appData.autoComment, "", NULL, CheckBox, N_("Auto-Comment") },
{ 0, 0, 0, NULL, (void*) &appData.autoObserve, "", NULL, CheckBox, N_("Auto-Observe") },
{ 0, 0, 0, NULL, (void*) &appData.autoRaiseBoard, "", NULL, CheckBox, N_("Auto-Raise Board") },
{ 0, 0, 0, NULL, (void*) &appData.autoCreateLogon, "", NULL, CheckBox, N_("Auto-Create Logon Script") },
{ 0, 0, 0, NULL, (void*) &appData.bgObserve, "",   NULL, CheckBox, N_("Background Observe while Playing") },
{ 0, 0, 0, NULL, (void*) &appData.dualBoard, "",   NULL, CheckBox, N_("Dual Board for Background-Observed Game") },
{ 0, 0, 0, NULL, (void*) &appData.getMoveList, "", NULL, CheckBox, N_("Get Move List") },
{ 0, 0, 0, NULL, (void*) &appData.quietPlay, "",   NULL, CheckBox, N_("Quiet Play") },
{ 0, 0, 0, NULL, (void*) &appData.seekGraph, "",   NULL, CheckBox, N_("Seek Graph") },
{ 0, 0, 0, NULL, (void*) &appData.autoRefresh, "", NULL, CheckBox, N_("Auto-Refresh Seek Graph") },
{ 0, 0, 0, NULL, (void*) &appData.autoBox, "", NULL, CheckBox, N_("Auto-InputBox PopUp") },
{ 0, 0, 0, NULL, (void*) &appData.quitNext, "", NULL, CheckBox, N_("Quit after game") },
{ 0, 0, 0, NULL, (void*) &appData.premove, "",     NULL, CheckBox, N_("Premove") },
{ 0, 0, 0, NULL, (void*) &appData.premoveWhite, "", NULL, CheckBox, N_("Premove for White") },
{ 0, 0, 0, NULL, (void*) &appData.premoveWhiteText, "", NULL, TextBox, N_("First White Move:") },
{ 0, 0, 0, NULL, (void*) &appData.premoveBlack, "", NULL, CheckBox, N_("Premove for Black") },
{ 0, 0, 0, NULL, (void*) &appData.premoveBlackText, "", NULL, TextBox, N_("First Black Move:") },
{ 0, SAME_ROW, 0, NULL, NULL, NULL, NULL, Break, "" },
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
{ 0, 0, 0, NULL, (void*) &appData.colorNormal, "", NULL, TextBox, N_("Other Text Colors:") },
{ 0, 0, 0, NULL, (void*) &IcsOptionsOK, "", NULL, EndMark , "" }
};

void
IcsOptionsProc ()
{
   GenericPopUp(icsOptions, _("ICS Options"), TransientDlg, BoardWindow, MODAL, 0);
}

//-------------------------------------------- Load Game Options ---------------------------------

static char *modeNames[] = { N_("Exact position match"), N_("Shown position is subset"), N_("Same material with exactly same Pawn chain"),
		      N_("Same material"), N_("Material range (top board half optional)"), N_("Material difference (optional stuff balanced)"), NULL };
static char *modeValues[] = { "1", "2", "3", "4", "5", "6" };
static char *searchMode, *countRange;

static int
LoadOptionsOK ()
{
    appData.minPieces = appData.maxPieces = 0;
    sscanf(countRange, "%d-%d", &appData.minPieces, &appData.maxPieces);
    if(appData.maxPieces < appData.minPieces) appData.maxPieces = appData.minPieces;
    appData.searchMode = atoi(searchMode);
    return 1;
}

static Option loadOptions[] = {
{ 0,  0, 0,     NULL, (void*) &appData.autoDisplayTags, "", NULL, CheckBox, N_("Auto-Display Tags") },
{ 0,  0, 0,     NULL, (void*) &appData.autoDisplayComment, "", NULL, CheckBox, N_("Auto-Display Comment") },
{ 0, LR, 0,     NULL, NULL, NULL, NULL, Label, N_("Auto-Play speed of loaded games\n(0 = instant, -1 = off):") },
{ 0, -1,10000000, NULL, (void*) &appData.timeDelay, "", NULL, Fractional, N_("Seconds per Move:") },
{ 0, LR, 0,     NULL, NULL, NULL, NULL, Label,  N_("\noptions to use in game-viewer mode:") },
{ 0, 0,300,     NULL, (void*) &appData.viewerOptions, "", NULL, TextBox,  "" },
{ 0, LR,  0,    NULL, NULL, NULL, NULL, Label,  N_("\nThresholds for position filtering in game list:") },
{ 0, 0,5000,    NULL, (void*) &appData.eloThreshold1, "", NULL, Spin, N_("Elo of strongest player at least:") },
{ 0, 0,5000,    NULL, (void*) &appData.eloThreshold2, "", NULL, Spin, N_("Elo of weakest player at least:") },
{ 0, 0,5000,    NULL, (void*) &appData.dateThreshold, "", NULL, Spin, N_("No games before year:") },
{ 0, 1,50,      NULL, (void*) &appData.stretch, "", NULL, Spin, N_("Minimum nr consecutive positions:") },
{ 0, 0,197,     NULL, (void*) &countRange, "", NULL, TextBox,  "Final nr of pieces" },
{ 0, 0,205,     NULL, (void*) &searchMode, (char*) modeValues, modeNames, ComboBox, N_("Search mode:") },
{ 0, 0, 0,      NULL, (void*) &appData.ignoreColors, "", NULL, CheckBox, N_("Also match reversed colors") },
{ 0, 0, 0,      NULL, (void*) &appData.findMirror, "", NULL, CheckBox, N_("Also match left-right flipped position") },
{ 0,  0, 0,     NULL, (void*) &LoadOptionsOK, "", NULL, EndMark , "" }
};

void
LoadOptionsPopUp (DialogClass parent)
{
   ASSIGN(countRange, "");
   ASSIGN(searchMode, modeValues[appData.searchMode-1]);
   GenericPopUp(loadOptions, _("Load Game Options"), TransientDlg, parent, MODAL, 0);
}

void
LoadOptionsProc ()
{   // called from menu
    LoadOptionsPopUp(BoardWindow);
}

//------------------------------------------- Save Game Options --------------------------------------------

static Option saveOptions[] = {
{ 0, 0, 0, NULL, (void*) &appData.autoSaveGames, "", NULL, CheckBox, N_("Auto-Save Games") },
{ 0, 0, 0, NULL, (void*) &appData.onlyOwn, "", NULL, CheckBox, N_("Own Games Only") },
{ 0, 0, 0, NULL, (void*) &appData.saveGameFile, ".pgn", NULL, FileName,  N_("Save Games on File:") },
{ 0, 0, 0, NULL, (void*) &appData.savePositionFile, ".fen", NULL, FileName,  N_("Save Final Positions on File:") },
{ 0, 0, 0, NULL, (void*) &appData.pgnEventHeader, "", NULL, TextBox,  N_("PGN Event Header:") },
{ 0, 0, 0, NULL, (void*) &appData.oldSaveStyle, "", NULL, CheckBox, N_("Old Save Style (as opposed to PGN)") },
{ 0, 0, 0, NULL, (void*) &appData.numberTag, "", NULL, CheckBox, N_("Include Number Tag in tourney PGN") },
{ 0, 0, 0, NULL, (void*) &appData.saveExtendedInfoInPGN, "", NULL, CheckBox, N_("Save Score/Depth Info in PGN") },
{ 0, 0, 0, NULL, (void*) &appData.saveOutOfBookInfo, "", NULL, CheckBox, N_("Save Out-of-Book Info in PGN           ") },
{ 0, SAME_ROW, 0, NULL, NULL, "", NULL, EndMark , "" }
};

void
SaveOptionsProc ()
{
   GenericPopUp(saveOptions, _("Save Game Options"), TransientDlg, BoardWindow, MODAL, 0);
}

//----------------------------------------------- Sound Options ---------------------------------------------

static void Test P((int n));
static char *trialSound;

static char *soundNames[] = {
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
	N_("Roar"),
	N_("Slap"),
	N_("Wood Thunk"),
	NULL,
	N_("User File")
};

static char *soundFiles[] = { // sound files corresponding to above names
	"",
	"$",
	NULL, // kludge alert: as first thing in the dialog readout this is replaced with the user-given .WAV filename
	"honkhonk.wav",
	"cymbal.wav",
	"ding1.wav",
	"gong.wav",
	"laser.wav",
	"penalty.wav",
	"phone.wav",
	"pop2.wav",
	"roar.wav",
	"slap.wav",
	"woodthunk.wav",
	NULL,
	NULL
};

static Option soundOptions[] = {
{ 0, 0, 0, NULL, (void*) (soundFiles+2) /* kludge! */, ".wav", NULL, FileName, N_("User WAV File:") },
{ 0, 0, 0, NULL, (void*) &appData.soundProgram, "", NULL, TextBox, N_("Sound Program:") },
{ 0, 0, 0, NULL, (void*) &trialSound, (char*) soundFiles, soundNames, ComboBox, N_("Try-Out Sound:") },
{ 0, SAME_ROW, 0, NULL, (void*) &Test, NULL, NULL, Button, N_("Play") },
{ 0, 0, 0, NULL, (void*) &appData.soundMove, (char*) soundFiles, soundNames, ComboBox, N_("Move:") },
{ 0, 0, 0, NULL, (void*) &appData.soundIcsWin, (char*) soundFiles, soundNames, ComboBox, N_("Win:") },
{ 0, 0, 0, NULL, (void*) &appData.soundIcsLoss, (char*) soundFiles, soundNames, ComboBox, N_("Lose:") },
{ 0, 0, 0, NULL, (void*) &appData.soundIcsDraw, (char*) soundFiles, soundNames, ComboBox, N_("Draw:") },
{ 0, 0, 0, NULL, (void*) &appData.soundIcsUnfinished, (char*) soundFiles, soundNames, ComboBox, N_("Unfinished:") },
{ 0, 0, 0, NULL, (void*) &appData.soundIcsAlarm, (char*) soundFiles, soundNames, ComboBox, N_("Alarm:") },
{ 0, 0, 0, NULL, (void*) &appData.soundChallenge, (char*) soundFiles, soundNames, ComboBox, N_("Challenge:") },
{ 0, SAME_ROW, 0, NULL, NULL, NULL, NULL, Break, "" },
{ 0, 0, 0, NULL, (void*) &appData.soundDirectory, "", NULL, PathName, N_("Sounds Directory:") },
{ 0, 0, 0, NULL, (void*) &appData.soundShout, (char*) soundFiles, soundNames, ComboBox, N_("Shout:") },
{ 0, 0, 0, NULL, (void*) &appData.soundSShout, (char*) soundFiles, soundNames, ComboBox, N_("S-Shout:") },
{ 0, 0, 0, NULL, (void*) &appData.soundChannel, (char*) soundFiles, soundNames, ComboBox, N_("Channel:") },
{ 0, 0, 0, NULL, (void*) &appData.soundChannel1, (char*) soundFiles, soundNames, ComboBox, N_("Channel 1:") },
{ 0, 0, 0, NULL, (void*) &appData.soundTell, (char*) soundFiles, soundNames, ComboBox, N_("Tell:") },
{ 0, 0, 0, NULL, (void*) &appData.soundKibitz, (char*) soundFiles, soundNames, ComboBox, N_("Kibitz:") },
{ 0, 0, 0, NULL, (void*) &appData.soundRequest, (char*) soundFiles, soundNames, ComboBox, N_("Request:") },
{ 0, 0, 0, NULL, (void*) &appData.soundRoar, (char*) soundFiles, soundNames, ComboBox, N_("Lion roar:") },
{ 0, 0, 0, NULL, (void*) &appData.soundSeek, (char*) soundFiles, soundNames, ComboBox, N_("Seek:") },
{ 0, SAME_ROW, 0, NULL, NULL, "", NULL, EndMark , "" }
};

static void
Test (int n)
{
    GenericReadout(soundOptions, 1);
    if(soundFiles[values[2]]) PlaySoundFile(soundFiles[values[2]]);
}

void
SoundOptionsProc ()
{
   free(soundFiles[2]);
   soundFiles[2] = strdup("*");
   GenericPopUp(soundOptions, _("Sound Options"), TransientDlg, BoardWindow, MODAL, 0);
}

//--------------------------------------------- Board Options --------------------------------------

static void DefColor P((int n));
static void AdjustColor P((int i));
static void ThemeSel P((int n, int sel));
static int BoardOptionsOK P((int n));

static char oldPieceDir[MSG_SIZ];
extern char *engineLine, *nickName; // defined later on

#define THEMELIST 1

static Option boardOptions[] = {
{   0,LR|T2T, 0, NULL, NULL, NULL, NULL, Label, N_("Selectable themes:") },
{ 300,LR|TB,200, NULL, (void*) engineMnemonic, (char*) &ThemeSel, NULL, ListBox, "" },
{   0,LR|T2T, 0, NULL, NULL, NULL, NULL, Label, N_("New name for current theme:") },
{ 0, 0, 0, NULL, (void*) &nickName, "", NULL, TextBox, "" },
{ 0,SAME_ROW, 0, NULL, NULL, NULL, NULL, Break, NULL },
{ 0,          0, 70, NULL, (void*) &appData.whitePieceColor, "", NULL, TextBox, N_("White Piece Color:") },
{ 1000, SAME_ROW, 0, NULL, (void*) &DefColor, NULL, (char**) "#FFFFCC", Button, "      " },
/* TRANSLATORS: R = single letter for the color red */
{    1, SAME_ROW, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, N_("R") },
/* TRANSLATORS: G = single letter for the color green */
{    2, SAME_ROW, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, N_("G") },
/* TRANSLATORS: B = single letter for the color blue */
{    3, SAME_ROW, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, N_("B") },
/* TRANSLATORS: D = single letter to make a color darker */
{    4, SAME_ROW, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, N_("D") },
{ 0,          0, 70, NULL, (void*) &appData.blackPieceColor, "", NULL, TextBox, N_("Black Piece Color:") },
{ 1000, SAME_ROW, 0, NULL, (void*) &DefColor, NULL, (char**) "#202020", Button, "      " },
{    1, SAME_ROW, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, N_("R") },
{    2, SAME_ROW, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, N_("G") },
{    3, SAME_ROW, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, N_("B") },
{    4, SAME_ROW, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, N_("D") },
{ 0,          0, 70, NULL, (void*) &appData.lightSquareColor, "", NULL, TextBox, N_("Light Square Color:") },
{ 1000, SAME_ROW, 0, NULL, (void*) &DefColor, NULL, (char**) "#C8C365", Button, "      " },
{    1, SAME_ROW, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, N_("R") },
{    2, SAME_ROW, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, N_("G") },
{    3, SAME_ROW, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, N_("B") },
{    4, SAME_ROW, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, N_("D") },
{ 0,          0, 70, NULL, (void*) &appData.darkSquareColor, "", NULL, TextBox, N_("Dark Square Color:") },
{ 1000, SAME_ROW, 0, NULL, (void*) &DefColor, NULL, (char**) "#77A26D", Button, "      " },
{    1, SAME_ROW, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, N_("R") },
{    2, SAME_ROW, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, N_("G") },
{    3, SAME_ROW, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, N_("B") },
{    4, SAME_ROW, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, N_("D") },
{ 0,          0, 70, NULL, (void*) &appData.highlightSquareColor, "", NULL, TextBox, N_("Highlight Color:") },
{ 1000, SAME_ROW, 0, NULL, (void*) &DefColor, NULL, (char**) "#FFFF00", Button, "      " },
{    1, SAME_ROW, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, N_("R") },
{    2, SAME_ROW, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, N_("G") },
{    3, SAME_ROW, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, N_("B") },
{    4, SAME_ROW, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, N_("D") },
{ 0,          0, 70, NULL, (void*) &appData.premoveHighlightColor, "", NULL, TextBox, N_("Premove Highlight Color:") },
{ 1000, SAME_ROW, 0, NULL, (void*) &DefColor, NULL, (char**) "#FF0000", Button, "      " },
{    1, SAME_ROW, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, N_("R") },
{    2, SAME_ROW, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, N_("G") },
{    3, SAME_ROW, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, N_("B") },
{    4, SAME_ROW, 0, NULL, (void*) &AdjustColor, NULL, NULL, Button, N_("D") },
{ 0, 0, 0, NULL, (void*) &appData.upsideDown, "", NULL, CheckBox, N_("Flip Pieces Shogi Style        (Colored buttons restore default)") },
//{ 0, 0, 0, NULL, (void*) &appData.allWhite, "", NULL, CheckBox, N_("Use Outline Pieces for Black") },
{ 0, 0, 0, NULL, (void*) &appData.monoMode, "", NULL, CheckBox, N_("Mono Mode") },
{ 0, 0, 200, NULL, (void*) &appData.logoSize, "", NULL, Spin, N_("Logo Size (0=off, requires restart):") },
{ 0,-1, 5, NULL, (void*) &appData.overrideLineGap, "", NULL, Spin, N_("Line Gap (-1 = default for board size):") },
{ 0, 0, 0, NULL, (void*) &appData.useBitmaps, "", NULL, CheckBox, N_("Use Board Textures") },
{ 0, 0, 0, NULL, (void*) &appData.liteBackTextureFile, ".png", NULL, FileName, N_("Light-Squares Texture File:") },
{ 0, 0, 0, NULL, (void*) &appData.darkBackTextureFile, ".png", NULL, FileName, N_("Dark-Squares Texture File:") },
{ 0, 0, 0, NULL, (void*) &appData.trueColors, "", NULL, CheckBox, N_("Use external piece bitmaps with their own colors") },
{ 0, 0, 0, NULL, (void*) &appData.pieceDirectory, "", NULL, PathName, N_("Directory with Pieces Images:") },
{ 0, 0, 0, NULL, (void*) &BoardOptionsOK, "", NULL, EndMark , "" }
};

static int
BoardOptionsOK (int n)
{
    if(n && (n = SelectedListBoxItem(&boardOptions[THEMELIST])) > 0 && *engineList[n] != '#') { // called by pressing OK, and theme selected
	ASSIGN(engineLine, engineList[n]);
    }
    LoadTheme();
    return 1;
}

static void
SetColorText (int n, char *buf)
{
    SetWidgetText(&boardOptions[n-1], buf, TransientDlg);
    SetColor(buf, &boardOptions[n]);
}

static void
DefColor (int n)
{
    SetColorText(n, (char*) boardOptions[n].choice);
}

void
RefreshColor (int source, int n)
{
    int col, j, r, g, b, step = 10;
    char *s, buf[MSG_SIZ]; // color string
    GetWidgetText(&boardOptions[source], &s);
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

static void
AdjustColor (int i)
{
    int n = boardOptions[i].value;
    RefreshColor(i-n-1, n);
}

void
ThemeSel (int n, int sel)
{
    int nr;
    char buf[MSG_SIZ];
    if(sel < 1) buf[0] = NULLCHAR; // back to top level
    else if(engineList[sel][0] == '#') safeStrCpy(buf, engineList[sel], MSG_SIZ); // group header, open group
    else { // normal line, select engine
	ASSIGN(engineLine, engineList[sel]);
	LoadTheme();
        PopDown(TransientDlg);
	return;
    }
    nr = NamesToList(appData.themeNames, engineList, engineMnemonic, buf); // replace list by only the group contents
    ASSIGN(engineMnemonic[0], buf);
    LoadListBox(&boardOptions[THEMELIST], _("# no themes are defined"), -1, -1);
    HighlightWithScroll(&boardOptions[THEMELIST], 0, nr);
}

void
BoardOptionsProc ()
{
   strncpy(oldPieceDir, appData.pieceDirectory, MSG_SIZ-1); // to see if it changed
   ASSIGN(engineLine, "");
   ASSIGN(nickName, "");
   ASSIGN(engineMnemonic[0], "");
   NamesToList(appData.themeNames, engineList, engineMnemonic, "");
   GenericPopUp(boardOptions, _("Board Options"), TransientDlg, BoardWindow, MODAL, 0);
}

//-------------------------------------------- ICS Text Menu Options ------------------------------

Option textOptions[100];
static void PutText P((char *text, int pos));
static void NewChat P((char *name));
static char clickedWord[MSG_SIZ], click;

void
SendString (char *p)
{
    char buf[MSG_SIZ], buf2[MSG_SIZ], *q;
    if(q = strstr(p, "$name")) { // in Xaw this is already intercepted
	if(!shellUp[TextMenuDlg] || !clickedWord[0]) return;
	strncpy(buf2, p, MSG_SIZ);
	snprintf(buf2 + (q-p), MSG_SIZ -(q-p), "%s%s", clickedWord, q+5);
        p = buf2;
    }
    if(!strcmp(p, "$copy")) { // special case for copy selection
        CopySomething(clickedWord);
    } else
    if(!strcmp(p, "$chat")) { // special case for opening chat
        NewChat(clickedWord);
    } else
    if(q = strstr(p, "$input")) {
	if(!shellUp[TextMenuDlg]) return;
	strncpy(buf, p, MSG_SIZ);
	strncpy(buf + (q-p), q+6, MSG_SIZ-(q-p));
	PutText(buf, q-p);
    } else {
	snprintf(buf, MSG_SIZ, "%s\n", p);
	SendToICS(buf);
    }
    if(click) { // popped up by memo click
	click = clickedWord[0] = 0;
	PopDown(TextMenuDlg);
    }
}

void
IcsTextPopUp ()
{
   int i=0, j;
   char *p, *q, *r;
   if((p = icsTextMenuString) == NULL) return;
   do {
	q = r = p; while(*p && *p != ';') p++;
	if(textOptions[i].name == NULL) textOptions[i].name = (char*) malloc(MSG_SIZ);
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
   MarkMenu("View.ICStextmenu", TextMenuDlg);
   GenericPopUp(textOptions, _("ICS text menu"), TextMenuDlg, BoardWindow, NONMODAL, appData.topLevel);
}

void
IcsTextProc ()
{
    if(shellUp[TextMenuDlg]) PopDown(TextMenuDlg);
    else IcsTextPopUp();
}

//---------------------------------------------------- Edit Comment -----------------------------------

static char *commentText;
static int commentIndex;
static void ClearComment P((int n));
static void SaveChanges P((int n));
int savedIndex;  /* gross that this is global (and even across files...) */

static int CommentClick P((Option *opt, int n, int x, int y, char *val, int index));

static int
NewComCallback (int n)
{
    ReplaceComment(commentIndex, commentText);
    return 1;
}

Option commentOptions[] = {
{ 200, T_VSCRL | T_FILL | T_WRAP | T_TOP, 250, NULL, (void*) &commentText, NULL, (char **) &CommentClick, TextBox, "", &appData.commentFont },
{ 0,     0,     50, NULL, (void*) &ClearComment, NULL, NULL, Button, N_("clear") },
{ 0, SAME_ROW, 100, NULL, (void*) &SaveChanges, NULL, NULL, Button, N_("save changes") },
{ 0, SAME_ROW,  0,  NULL, (void*) &NewComCallback, "", NULL, EndMark , "" }
};

static int
CommentClick (Option *opt, int n, int x, int y, char *val, int index)
{
	if(n != 3) return FALSE; // only button-3 press is of interest
	ReplaceComment(savedIndex, val);
	if(savedIndex != currentMove) ToNrEvent(savedIndex);
	LoadVariation( index, val ); // [HGM] also does the actual moving to it, now
	return TRUE;
}

static void
SaveChanges (int n)
{
    GenericReadout(commentOptions, 0);
    ReplaceComment(commentIndex, commentText);
}

static void
ClearComment (int n)
{
    SetWidgetText(&commentOptions[0], "", CommentDlg);
}

void
NewCommentPopup (char *title, char *text, int index)
{
    if(DialogExists(CommentDlg)) { // if already exists, alter title and content
	SetDialogTitle(CommentDlg, title);
	SetWidgetText(&commentOptions[0], text, CommentDlg);
    }
    if(commentText) free(commentText); commentText = strdup(text);
    commentIndex = index;
    MarkMenu("View.Comments", CommentDlg);
    if(GenericPopUp(commentOptions, title, CommentDlg, BoardWindow, NONMODAL, appData.topLevel))
	AddHandler(&commentOptions[0], CommentDlg, 1);
}

void
EditCommentPopUp (int index, char *title, char *text)
{
    savedIndex = index;
    if (text == NULL) text = "";
    NewCommentPopup(title, text, index);
}

void
CommentPopUp (char *title, char *text)
{
    savedIndex = currentMove; // [HGM] vari
    NewCommentPopup(title, text, currentMove);
}

void
CommentPopDown ()
{
    PopDown(CommentDlg);
}


void
EditCommentProc ()
{
    if (PopDown(CommentDlg)) { // popdown succesful
//	MarkMenuItem("Edit.EditComment", False);
//	MarkMenuItem("View.Comments", False);
    } else // was not up
	EditCommentEvent();
}

//------------------------------------------------------ Edit Tags ----------------------------------

static void changeTags P((int n));
static char *tagsText, **resPtr;

static int TagsClick P((Option *opt, int n, int x, int y, char *val, int index));

static int
NewTagsCallback (int n)
{
    if(bookUp) SaveToBook(tagsText), DisplayBook(currentMove); else
    if(resPtr) { ASSIGN(*resPtr, tagsText); } else
    ReplaceTags(tagsText, &gameInfo);
    return 1;
}

static void
NewMove ()
{
    addToBookFlag = !addToBookFlag;
}

static Option tagsOptions[] = {
{   0,   0,   0, NULL, NULL, NULL, NULL, Label,  NULL },
{ 200, T_VSCRL | T_FILL | T_WRAP | T_TOP, 200, NULL, (void*) &tagsText, NULL, (char **) &TagsClick, TextBox, "", &appData.tagsFont },
{   0,   0, 100, NULL, (void*) &NewMove,    NULL, NULL, Button, N_("add next move") },
{ 0,SAME_ROW,100,NULL, (void*) &changeTags, NULL, NULL, Button, N_("save changes") },
{ 0,SAME_ROW, 0, NULL, (void*) &NewTagsCallback, "", NULL, EndMark , "" }
};

static int TagsClick (Option *opt, int n, int x, int y, char *val, int index)
{
    if(!bookUp || n != 3) return FALSE; // only button-3 press in Edit Book is of interest
    PlayBookMove(val, index);
    return TRUE;
}

static void
changeTags (int n)
{
    GenericReadout(tagsOptions, 1);
    if(bookUp) SaveToBook(tagsText), DisplayBook(currentMove); else
    if(resPtr) { ASSIGN(*resPtr, tagsText); } else
    ReplaceTags(tagsText, &gameInfo);
}

void
NewTagsPopup (char *text, char *msg)
{
    char *title = bookUp ? _("Edit book") : _("Tags");

    tagsOptions[2].type = bookUp ? Button : Skip;
    tagsOptions[3].min = bookUp ? SAME_ROW : 0;
    if(DialogExists(TagsDlg)) { // if already exists, alter title and content
	SetWidgetText(&tagsOptions[1], text, TagsDlg);
	SetDialogTitle(TagsDlg, title);
    }
    if(tagsText) free(tagsText); tagsText = strdup(text);
    tagsOptions[0].name = msg;
    MarkMenu("View.Tags", TagsDlg);
    GenericPopUp(tagsOptions, title, TagsDlg, BoardWindow, NONMODAL, appData.topLevel);
}

void
TagsPopUp (char *tags, char *msg)
{
    NewTagsPopup(tags, cmailMsgLoaded ? msg : NULL);
}

void
EditTagsPopUp (char *tags, char **dest)
{   // wrapper to preserve old name used in back-end
    resPtr = dest; 
    NewTagsPopup(tags, NULL);
}

void
TagsPopDown()
{
    PopDown(TagsDlg);
    bookUp = False;
}

void
EditTagsProc ()
{
  if (bookUp || !PopDown(TagsDlg)) EditTagsEvent();
}

void
AddBookMove (char *text)
{
    AppendText(&tagsOptions[1], text);
}

//---------------------------------------------- ICS Input Box ----------------------------------

char *icsText;

// [HGM] code borrowed from winboard.c (which should thus go to backend.c!)
#define HISTORY_SIZE 64
static char *history[HISTORY_SIZE];
static int histIn = 0, histP = 0;

static void
SaveInHistory (char *cmd)
{
  if (history[histIn] != NULL) {
    free(history[histIn]);
    history[histIn] = NULL;
  }
  if (*cmd == NULLCHAR) return;
  history[histIn] = StrSave(cmd);
  histIn = (histIn + 1) % HISTORY_SIZE;
  if (history[histIn] != NULL) {
    free(history[histIn]);
    history[histIn] = NULL;
  }
  histP = histIn;
}

static char *
PrevInHistory (char *cmd)
{
  int newhp;
  if (histP == histIn) {
    if (history[histIn] != NULL) free(history[histIn]);
    history[histIn] = StrSave(cmd);
  }
  newhp = (histP - 1 + HISTORY_SIZE) % HISTORY_SIZE;
  if (newhp == histIn || history[newhp] == NULL) return NULL;
  histP = newhp;
  return history[histP];
}

static char *
NextInHistory ()
{
  if (histP == histIn) return NULL;
  histP = (histP + 1) % HISTORY_SIZE;
  return history[histP];
}
// end of borrowed code

#define INPUT 0

Option boxOptions[] = {
{  30, T_TOP, 400, NULL, (void*) &icsText, NULL, NULL, TextBox, "" },
{  0,  NO_OK,   0, NULL, NULL, "", NULL, EndMark , "" }
};

void
ICSInputSendText ()
{
    char *val;

    GetWidgetText(&boxOptions[INPUT], &val);
    SaveInHistory(val);
    SendMultiLineToICS(val);
    SetWidgetText(&boxOptions[INPUT], "", InputBoxDlg);
}

void
IcsKey (int n)
{   // [HGM] input: let up-arrow recall previous line from history
    char *val = NULL; // to suppress spurious warning

    if (!shellUp[InputBoxDlg]) return;
    switch(n) {
      case 0:
	ICSInputSendText();
	return;
      case 1:
	GetWidgetText(&boxOptions[INPUT], &val);
	val = PrevInHistory(val);
	break;
      case -1:
	val = NextInHistory();
    }
    SetWidgetText(&boxOptions[INPUT], val = val ? val : "", InputBoxDlg);
    SetInsertPos(&boxOptions[INPUT], strlen(val));
}

void
ICSInputBoxPopUp ()
{
    MarkMenu("View.ICSInputBox", InputBoxDlg);
    if(GenericPopUp(boxOptions, _("ICS input box"), InputBoxDlg, BoardWindow, NONMODAL, 0))
	AddHandler(&boxOptions[INPUT], InputBoxDlg, 3);
    CursorAtEnd(&boxOptions[INPUT]);
}

void
IcsInputBoxProc ()
{
    if (!PopDown(InputBoxDlg)) ICSInputBoxPopUp();
}

//--------------------------------------------- Move Type In ------------------------------------------

static int TypeInOK P((int n));

Option typeOptions[] = {
{ 30, T_TOP, 400, NULL, (void*) &icsText, NULL, NULL, TextBox, "" },
{ 0,  NO_OK,   0, NULL, (void*) &TypeInOK, "", NULL, EndMark , "" }
};

static int
TypeInOK (int n)
{
    TypeInDoneEvent(icsText);
    return TRUE;
}

void
PopUpMoveDialog (char firstchar)
{
    static char buf[2];
    buf[0] = firstchar; ASSIGN(icsText, buf);
    if(GenericPopUp(typeOptions, _("Type a move"), TransientDlg, BoardWindow, MODAL, 0))
	AddHandler(&typeOptions[0], TransientDlg, 2);
    CursorAtEnd(&typeOptions[0]);
}

void
BoxAutoPopUp (char *buf)
{       // only used in Xaw. GTK calls ConsoleAutoPopUp in stead (when we type to board)
	if(!appData.autoBox) return;
	if(appData.icsActive) { // text typed to board in ICS mode: divert to ICS input box
	    if(DialogExists(InputBoxDlg)) { // box already exists: append to current contents
		char *p, newText[MSG_SIZ];
		GetWidgetText(&boxOptions[INPUT], &p);
		snprintf(newText, MSG_SIZ, "%s%c", p, *buf);
		SetWidgetText(&boxOptions[INPUT], newText, InputBoxDlg);
		if(shellUp[InputBoxDlg]) HardSetFocus (&boxOptions[INPUT], InputBoxDlg); //why???
	    } else icsText = buf; // box did not exist: make sure it pops up with char in it
	    ICSInputBoxPopUp();
	} else PopUpMoveDialog(*buf);
}

//------------------------------------------ Engine Settings ------------------------------------

void
SettingsPopUp (ChessProgramState *cps)
{
   if(!cps->nrOptions) { DisplayNote(_("Engine has no options")); return; }
   currentCps = cps;
   GenericPopUp(cps->option, _("Engine Settings"), TransientDlg, BoardWindow, MODAL, 0);
}

void
FirstSettingsProc ()
{
    SettingsPopUp(&first);
}

void
SecondSettingsProc ()
{
   if(WaitForEngine(&second, SettingsMenuIfReady)) return;
   SettingsPopUp(&second);
}

//----------------------------------------------- Load Engine --------------------------------------

char *engineDir, *engineLine, *nickName, *params;
Boolean isUCI, hasBook, storeVariant, v1, addToList, useNick, secondEng;

static void EngSel P((int n, int sel));
static int InstallOK P((int n));

static Option installOptions[] = {
{   0,LR|T2T, 0, NULL, NULL, NULL, NULL, Label, N_("Select engine from list:") },
{ 300,LR|TB,200, NULL, (void*) engineMnemonic, (char*) &EngSel, NULL, ListBox, "" },
{ 0,SAME_ROW, 0, NULL, NULL, NULL, NULL, Break, NULL },
{   0,  LR,   0, NULL, NULL, NULL, NULL, Label, N_("or specify one below:") },
{   0,  0,    0, NULL, (void*) &nickName, NULL, NULL, TextBox, N_("Nickname (optional):") },
{   0,  0,    0, NULL, (void*) &useNick, NULL, NULL, CheckBox, N_("Use nickname in PGN player tags of engine-engine games") },
{   0,  0,    0, NULL, (void*) &engineDir, NULL, NULL, PathName, N_("Engine Directory:") },
{   0,  0,    0, NULL, (void*) &engineName, NULL, NULL, FileName, N_("Engine Command:") },
{   0,  LR,   0, NULL, NULL, NULL, NULL, Label, N_("(Directory will be derived from engine path when empty)") },
{   0,  0,    0, NULL, (void*) &isUCI, NULL, NULL, CheckBox, N_("UCI") },
{   0,  0,    0, NULL, (void*) &v1, NULL, NULL, CheckBox, N_("WB protocol v1 (do not wait for engine features)") },
{   0,  0,    0, NULL, (void*) &hasBook, NULL, NULL, CheckBox, N_("Must not use GUI book") },
{   0,  0,    0, NULL, (void*) &addToList, NULL, NULL, CheckBox, N_("Add this engine to the list") },
{   0,  0,    0, NULL, (void*) &storeVariant, NULL, NULL, CheckBox, N_("Force current variant with this engine") },
{   0,  0,    0, NULL, (void*) &InstallOK, "", NULL, EndMark , "" }
};

static int
InstallOK (int n)
{
    if(n && (n = SelectedListBoxItem(&installOptions[1])) > 0) { // called by pressing OK, and engine selected
	ASSIGN(engineLine, engineList[n]);
    }
    PopDown(TransientDlg); // early popdown, to allow FreezeUI to instate grab
    if(!secondEng) Load(&first, 0); else Load(&second, 1);
    return FALSE; // no double PopDown!
}

static void
EngSel (int n, int sel)
{
    int nr;
    char buf[MSG_SIZ];
    if(sel < 1) buf[0] = NULLCHAR; // back to top level
    else if(engineList[sel][0] == '#') safeStrCpy(buf, engineList[sel], MSG_SIZ); // group header, open group
    else { // normal line, select engine
	ASSIGN(engineLine, engineList[sel]);
	InstallOK(0);
	return;
    }
    nr = NamesToList(firstChessProgramNames, engineList, engineMnemonic, buf); // replace list by only the group contents
    ASSIGN(engineMnemonic[0], buf);
    LoadListBox(&installOptions[1], _("# no engines are installed"), -1, -1);
    HighlightWithScroll(&installOptions[1], 0, nr);
}

static void
LoadEngineProc (int engineNr, char *title)
{
   isUCI = storeVariant = v1 = useNick = False; addToList = hasBook = True; // defaults
   secondEng = engineNr;
   if(engineLine)   free(engineLine);   engineLine = strdup("");
   if(engineDir)    free(engineDir);    engineDir = strdup(".");
   if(nickName)     free(nickName);     nickName = strdup("");
   if(params)       free(params);       params = strdup("");
   ASSIGN(engineMnemonic[0], "");
   NamesToList(firstChessProgramNames, engineList, engineMnemonic, "");
   GenericPopUp(installOptions, title, TransientDlg, BoardWindow, MODAL, 0);
}

void
LoadEngine1Proc ()
{
    LoadEngineProc (0, _("Load first engine"));
}

void
LoadEngine2Proc ()
{
    LoadEngineProc (1, _("Load second engine"));
}

//----------------------------------------------------- Edit Book -----------------------------------------

void
EditBookProc ()
{
    EditBookEvent();
}

//--------------------------------------------------- New Shuffle Game ------------------------------

static void SetRandom P((int n));

static int
ShuffleOK (int n)
{
    ResetGameEvent();
    return 1;
}

static Option shuffleOptions[] = {
  {   0,  0,    0, NULL, (void*) &shuffleOpenings, NULL, NULL, CheckBox, N_("shuffle") },
  {   0,  0,    0, NULL, (void*) &appData.fischerCastling, NULL, NULL, CheckBox, N_("Fischer castling") },
  { 0,-1,2000000000, NULL, (void*) &appData.defaultFrcPosition, "", NULL, Spin, N_("Start-position number:") },
  {   0,  0,    0, NULL, (void*) &SetRandom, NULL, NULL, Button, N_("randomize") },
  {   0,  SAME_ROW,    0, NULL, (void*) &SetRandom, NULL, NULL, Button, N_("pick fixed") },
  { 0,SAME_ROW, 0, NULL, (void*) &ShuffleOK, "", NULL, EndMark , "" }
};

static void
SetRandom (int n)
{
    int r = n==2 ? -1 : random() & (1<<30)-1;
    char buf[MSG_SIZ];
    snprintf(buf, MSG_SIZ,  "%d", r);
    SetWidgetText(&shuffleOptions[1], buf, TransientDlg);
    SetWidgetState(&shuffleOptions[0], True);
}

void
ShuffleMenuProc ()
{
    GenericPopUp(shuffleOptions, _("New Shuffle Game"), TransientDlg, BoardWindow, MODAL, 0);
}

//------------------------------------------------------ Time Control -----------------------------------

static int TcOK P((int n));
int tmpMoves, tmpTc, tmpInc, tmpOdds1, tmpOdds2, tcType;

static void SetTcType P((int n));

static char *
Value (int n)
{
	static char buf[MSG_SIZ];
	snprintf(buf, MSG_SIZ, "%d", n);
	return buf;
}

static Option tcOptions[] = {
{   0,  0,    0, NULL, (void*) &SetTcType, NULL, NULL, Button, N_("classical") },
{   0,SAME_ROW,0,NULL, (void*) &SetTcType, NULL, NULL, Button, N_("incremental") },
{   0,SAME_ROW,0,NULL, (void*) &SetTcType, NULL, NULL, Button, N_("fixed max") },
{   0,  0,  200, NULL, (void*) &tmpMoves, NULL, NULL, Spin, N_("Moves per session:") },
{   0,  0,10000, NULL, (void*) &tmpTc,    NULL, NULL, Spin, N_("Initial time (min):") },
{   0, 0, 10000, NULL, (void*) &tmpInc,   NULL, NULL, Spin, N_("Increment or max (sec/move):") },
{   0,  0,    0, NULL, NULL, NULL, NULL, Label, N_("Time-Odds factors:") },
{   0,  1, 1000, NULL, (void*) &tmpOdds1, NULL, NULL, Spin, N_("Engine #1") },
{   0,  1, 1000, NULL, (void*) &tmpOdds2, NULL, NULL, Spin, N_("Engine #2 / Human") },
{   0,  0,    0, NULL, (void*) &TcOK, "", NULL, EndMark , "" }
};

static int
TcOK (int n)
{
    char *tc;
    if(tcType == 0 && tmpMoves <= 0) return 0;
    if(tcType == 2 && tmpInc <= 0) return 0;
    GetWidgetText(&tcOptions[4], &tc); // get original text, in case it is min:sec
    searchTime = 0;
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

static void
SetTcType (int n)
{
    switch(tcType = n) {
      case 0:
	SetWidgetText(&tcOptions[3], Value(tmpMoves), TransientDlg);
	SetWidgetText(&tcOptions[4], Value(tmpTc), TransientDlg);
	SetWidgetText(&tcOptions[5], _("Unused"), TransientDlg);
	break;
      case 1:
	SetWidgetText(&tcOptions[3], _("Unused"), TransientDlg);
	SetWidgetText(&tcOptions[4], Value(tmpTc), TransientDlg);
	SetWidgetText(&tcOptions[5], Value(tmpInc), TransientDlg);
	break;
      case 2:
	SetWidgetText(&tcOptions[3], _("Unused"), TransientDlg);
	SetWidgetText(&tcOptions[4], _("Unused"), TransientDlg);
	SetWidgetText(&tcOptions[5], Value(tmpInc), TransientDlg);
    }
}

void
TimeControlProc ()
{
   tmpMoves = appData.movesPerSession;
   tmpInc = appData.timeIncrement; if(tmpInc < 0) tmpInc = 0;
   tmpOdds1 = tmpOdds2 = 1; tcType = 0;
   tmpTc = atoi(appData.timeControl);
   GenericPopUp(tcOptions, _("Time Control"), TransientDlg, BoardWindow, MODAL, 0);
   SetTcType(searchTime ? 2 : appData.timeIncrement < 0 ? 0 : 1);
}

//------------------------------- Ask Question -----------------------------------------

int SendReply P((int n));
char pendingReplyPrefix[MSG_SIZ];
ProcRef pendingReplyPR;
char *answer;

Option askOptions[] = {
{ 0, 0, 0, NULL, NULL, NULL, NULL, Label,  NULL },
{ 0, 0, 0, NULL, (void*) &answer, "", NULL, TextBox, "" },
{ 0, 0, 0, NULL, (void*) &SendReply, "", NULL, EndMark , "" }
};

int
SendReply (int n)
{
    char buf[MSG_SIZ];
    int err;
    char *reply=answer;
//    GetWidgetText(&askOptions[1], &reply);
    safeStrCpy(buf, pendingReplyPrefix, sizeof(buf)/sizeof(buf[0]) );
    if (*buf) strncat(buf, " ", MSG_SIZ - strlen(buf) - 1);
    strncat(buf, reply, MSG_SIZ - strlen(buf) - 1);
    strncat(buf, "\n",  MSG_SIZ - strlen(buf) - 1);
    OutputToProcess(pendingReplyPR, buf, strlen(buf), &err); // does not go into debug file??? => bug
    if (err) DisplayFatalError(_("Error writing to chess program"), err, 0);
    return TRUE;
}

void
AskQuestion (char *title, char *question, char *replyPrefix, ProcRef pr)
{
    safeStrCpy(pendingReplyPrefix, replyPrefix, sizeof(pendingReplyPrefix)/sizeof(pendingReplyPrefix[0]) );
    pendingReplyPR = pr;
    ASSIGN(answer, "");
    askOptions[0].name = question;
    if(GenericPopUp(askOptions, title, AskDlg, BoardWindow, MODAL, 0))
	AddHandler(&askOptions[1], AskDlg, 2);
}

//---------------------------- Promotion Popup --------------------------------------

static int count;

static void PromoPick P((int n));

static Option promoOptions[] = {
{   0,         0,    0, NULL, (void*) &PromoPick, NULL, NULL, Button, NULL },
{   0,  SAME_ROW,    0, NULL, (void*) &PromoPick, NULL, NULL, Button, NULL },
{   0,  SAME_ROW,    0, NULL, (void*) &PromoPick, NULL, NULL, Button, NULL },
{   0,  SAME_ROW,    0, NULL, (void*) &PromoPick, NULL, NULL, Button, NULL },
{   0,  SAME_ROW,    0, NULL, (void*) &PromoPick, NULL, NULL, Button, NULL },
{   0,  SAME_ROW,    0, NULL, (void*) &PromoPick, NULL, NULL, Button, NULL },
{   0,  SAME_ROW,    0, NULL, (void*) &PromoPick, NULL, NULL, Button, NULL },
{   0,  SAME_ROW,    0, NULL, (void*) &PromoPick, NULL, NULL, Button, NULL },
{   0, SAME_ROW | NO_OK, 0, NULL, NULL, "", NULL, EndMark , "" }
};

static void
PromoPick (int n)
{
    int promoChar = promoOptions[n+count].value;

    PopDown(PromoDlg);

    if (promoChar == 0) fromX = -1;
    if (fromX == -1) return;

    if (! promoChar) {
	fromX = fromY = -1;
	ClearHighlights();
	return;
    }
    if(promoChar == '=' && !IS_SHOGI(gameInfo.variant)) promoChar = NULLCHAR;
    UserMoveEvent(fromX, fromY, toX, toY, promoChar);

    if (!appData.highlightLastMove || gotPremove) ClearHighlights();
    if (gotPremove) SetPremoveHighlights(fromX, fromY, toX, toY);
    fromX = fromY = -1;
}

static void
SetPromo (char *name, int nr, char promoChar)
{
    ASSIGN(promoOptions[nr].name, name);
    promoOptions[nr].value = promoChar;
    promoOptions[nr].min = SAME_ROW;
}

void
PromotionPopUp (char choice)
{ // choice depends on variant: prepare dialog acordingly
  count = 8;
  SetPromo(_("Cancel"), --count, -1); // Beware: GenericPopUp cannot handle user buttons named "cancel" (lowe case)!
  if(choice != '+') {
    if (!appData.testLegality || gameInfo.variant == VariantSuicide ||
        gameInfo.variant == VariantSpartan && !WhiteOnMove(currentMove) ||
        gameInfo.variant == VariantGiveaway) {
      SetPromo(_("King"), --count, 'k');
    }
    if(gameInfo.variant == VariantSpartan && !WhiteOnMove(currentMove)) {
      SetPromo(_("Captain"), --count, 'c');
      SetPromo(_("Lieutenant"), --count, 'l');
      SetPromo(_("General"), --count, 'g');
      SetPromo(_("Warlord"), --count, 'w');
    } else {
      SetPromo(_("Knight"), --count, 'n');
      SetPromo(_("Bishop"), --count, 'b');
      SetPromo(_("Rook"), --count, 'r');
      if(gameInfo.variant == VariantCapablanca ||
         gameInfo.variant == VariantGothic ||
         gameInfo.variant == VariantCapaRandom) {
        SetPromo(_("Archbishop"), --count, 'a');
        SetPromo(_("Chancellor"), --count, 'c');
      }
      SetPromo(_("Queen"), --count, 'q');
      if(gameInfo.variant == VariantChuChess)
        SetPromo(_("Lion"), --count, 'l');
    }
  } else // [HGM] shogi
  {
      SetPromo(_("Defer"), --count, '=');
      SetPromo(_("Promote"), --count, '+');
  }
  promoOptions[count].min = 0;
  GenericPopUp(promoOptions + count, "Promotion", PromoDlg, BoardWindow, NONMODAL, 0);
}

//---------------------------- Chat Windows ----------------------------------------------

static char *line, *memo, *chatMemo, *partner, *texts[MAX_CHAT], dirty[MAX_CHAT], *inputs[MAX_CHAT], *icsLine, *tmpLine;
static int activePartner;
int hidden = 1;

void ChatSwitch P((int n));
int  ChatOK P((int n));

#define CHAT_ICS     6
#define CHAT_PARTNER 8
#define CHAT_OUT    11
#define CHAT_PANE   12
#define CHAT_IN     13

void PaneSwitch P((void));
void ClearChat P((void));

WindowPlacement wpTextMenu;

int
ContextMenu (Option *opt, int button, int x, int y, char *text, int index)
{ // callback for ICS-output clicks; handles button 3, passes on other events
  int h;
  if(button == -3) return TRUE; // supress default GTK context menu on up-click
  if(button != 3) return FALSE;
  if(index == -1) { // pre-existing selection in memo
    strncpy(clickedWord, text, MSG_SIZ);
  } else { // figure out what word was clicked
    char *start, *end;
    start = end = text + index;
    while(isalnum(*end)) end++;
    while(start > text && isalnum(start[-1])) start--;
    clickedWord[0] = NULLCHAR;
    if(end-start >= 80) end = start + 80; // intended for small words and numbers
    strncpy(clickedWord, start, end-start); clickedWord[end-start] = NULLCHAR;
  }
  click = !shellUp[TextMenuDlg]; // request auto-popdown of textmenu when we popped it up
  h = wpTextMenu.height; // remembered height of text menu
  if(h <= 0) h = 65;     // when not available, position w.r.t. top
  GetPlacement(ChatDlg, &wpTextMenu);
  if(opt->target == (void*) &chatMemo) wpTextMenu.y += (wpTextMenu.height - 30)/2; // click in chat
  wpTextMenu.x += x - 50; wpTextMenu.y += y - h + 50; // request positioning
  if(wpTextMenu.x < 0) wpTextMenu.x = 0;
  if(wpTextMenu.y < 0) wpTextMenu.y = 0;
  wpTextMenu.width = wpTextMenu.height = -1;
  IcsTextPopUp();
  return TRUE;
}

Option chatOptions[] = {
{  0,  0,   0, NULL, NULL, NULL, NULL, Label , N_("Chats:") },
{ 1, SAME_ROW|TT, 75, NULL, (void*) &ChatSwitch, NULL, NULL, Button, N_("New Chat") },
{ 2, SAME_ROW|TT, 75, NULL, (void*) &ChatSwitch, NULL, NULL, Button, N_("New Chat") },
{ 3, SAME_ROW|TT, 75, NULL, (void*) &ChatSwitch, NULL, NULL, Button, N_("New Chat") },
{ 4, SAME_ROW|TT, 75, NULL, (void*) &ChatSwitch, NULL, NULL, Button, N_("New Chat") },
{ 5, SAME_ROW|TT, 75, NULL, (void*) &ChatSwitch, NULL, NULL, Button, N_("New Chat") },
{ 250, T_VSCRL | T_FILL | T_WRAP | T_TOP,    510, NULL, (void*) &memo, NULL, (void*) &ContextMenu, TextBox, "" },
{  0,  0,   0, NULL, NULL, "", NULL, Break , "" },
{ 0,   T_TOP,    100, NULL, (void*) &partner, NULL, NULL, TextBox, N_("Chat partner:") },
{  0, SAME_ROW, 0, NULL, (void*) &ClearChat,  NULL, NULL, Button, N_("End Chat") },
{  0, SAME_ROW, 0, NULL, (void*) &PaneSwitch, NULL, NULL, Button, N_("Hide") },
{ 250, T_VSCRL | T_FILL | T_WRAP | T_TOP,    510, NULL, (void*) &chatMemo, NULL, (void*) &ContextMenu, TextBox, "" },
{  0,  0,   0, NULL, NULL, "", NULL, Break , "" },
{  0,    0,  510, NULL, (void*) &line, NULL, NULL, TextBox, "" },
{ 0, NO_OK|SAME_ROW, 0, NULL, (void*) &ChatOK, NULL, NULL, EndMark , "" }
};

static void
PutText (char *text, int pos)
{
    char buf[MSG_SIZ], *p;
    DialogClass dlg = ChatDlg;
    Option *opt = &chatOptions[CHAT_IN];

    if(strstr(text, "$add ") == text) {
	GetWidgetText(&boxOptions[INPUT], &p);
	snprintf(buf, MSG_SIZ, "%s%s", p, text+5); text = buf;
	pos += strlen(p) - 5;
    }
    if(shellUp[InputBoxDlg]) opt = &boxOptions[INPUT], dlg = InputBoxDlg; // for the benefit of Xaw give priority to ICS Input Box
    SetWidgetText(opt, text, dlg);
    SetInsertPos(opt, pos);
    HardSetFocus(opt, dlg);
    CursorAtEnd(opt);
}

int
IcsHist (int n, Option *opt, DialogClass dlg)
{   // [HGM] input: let up-arrow recall previous line from history
    char *val = NULL; // to suppress spurious warning
    int chat, start;

    if(opt != &chatOptions[CHAT_IN] && !(opt == &chatOptions[CHAT_PARTNER] && n == 33)) return 0;
    switch(n) {
      case 33: // <Esc>
	if(hidden) BoardToTop();
	else PaneSwitch();
	break;
      case 15:
	NewChat(lastTalker);
	break;
      case 14:
	for(chat=0; chat < MAX_CHAT; chat++) if(!chatPartner[chat][0]) break;
	if(chat < MAX_CHAT) ChatSwitch(chat + 1);
	break;
      case 10: // <Tab>
	chat = start = (activePartner - hidden + MAX_CHAT) % MAX_CHAT;
        while(!dirty[chat = (chat + 1)%MAX_CHAT]) if(chat == start) break;
	if(!dirty[chat])
        while(!chatPartner[chat = (chat + 1)%MAX_CHAT][0]) if(chat == start) break;
	if(chat == start && hidden) chat = 0; // if all unused, start left
        ChatSwitch(chat + 1);
	break;
      case 1:
	GetWidgetText(opt, &val);
	val = PrevInHistory(val);
	break;
      case -1:
	val = NextInHistory();
    }
    SetWidgetText(opt, val = val ? val : "", dlg);
    SetInsertPos(opt, strlen(val));
    return 1;
}

void
OutputChatMessage (int partner, char *mess)
{
    char *p = texts[partner];
    int len = strlen(mess) + 1;

    if(p) len += strlen(p);
    texts[partner] = (char*) malloc(len);
    snprintf(texts[partner], len, "%s%s", p ? p : "", mess);
    FREE(p);
    if(partner == activePartner && !hidden) {
	AppendText(&chatOptions[CHAT_OUT], mess);
	SetInsertPos(&chatOptions[CHAT_OUT], len-2);
    } else {
	SetColor("#FFC000", &chatOptions[partner + 1]);
	dirty[partner] = 1;
    }
}

int
ChatOK (int n)
{   // can only be called through <Enter> in chat-partner text-edit, as there is no OK button
    char buf[MSG_SIZ];

    if(!hidden && (!partner || strcmp(partner, chatPartner[activePartner]))) {
	safeStrCpy(chatPartner[activePartner], partner, MSG_SIZ);
	SetWidgetText(&chatOptions[CHAT_OUT], "", -1); // clear text if we alter partner
	SetWidgetText(&chatOptions[CHAT_IN], "", ChatDlg); // clear text if we alter partner
	SetWidgetLabel(&chatOptions[activePartner+1], chatPartner[activePartner][0] ? chatPartner[activePartner] : _("New Chat"));
	HardSetFocus(&chatOptions[CHAT_IN], 0);
    }
    if(line[0] || hidden) { // something was typed (for ICS commands we also allow empty line!)
	SetWidgetText(&chatOptions[CHAT_IN], "", ChatDlg);
	// from here on it could be back-end
	if(line[strlen(line)-1] == '\n') line[strlen(line)-1] = NULLCHAR;
	SaveInHistory(line);
	if(hidden || !*chatPartner[activePartner]) snprintf(buf, MSG_SIZ, "%s\n", line); else // command for ICS
	if(!strcmp("whispers", chatPartner[activePartner]))
	      snprintf(buf, MSG_SIZ, "whisper %s\n", line); // WHISPER box uses "whisper" to send
	else if(!strcmp("shouts", chatPartner[activePartner]))
	      snprintf(buf, MSG_SIZ, "shout %s\n", line); // SHOUT box uses "shout" to send
	else {
	    if(!atoi(chatPartner[activePartner])) {
		snprintf(buf, MSG_SIZ, "> %s\n", line); // echo only tells to handle, not channel
		OutputChatMessage(activePartner, buf);
		snprintf(buf, MSG_SIZ, "xtell %s %s\n", chatPartner[activePartner], line);
	    } else
		snprintf(buf, MSG_SIZ, "tell %s %s\n", chatPartner[activePartner], line);
	}
	SendToICS(buf);
    }
    return FALSE; // never pop down
}

void
DelayedSetText ()
{
    SetWidgetText(&chatOptions[CHAT_IN], tmpLine, -1); // leave focus on chat-partner field!
    SetInsertPos(&chatOptions[CHAT_IN], strlen(tmpLine));
}

void
DelayedScroll ()
{   // If we do this immediately it does it before shrinking the memo, so the lower half remains hidden (Ughh!)
    SetInsertPos(&chatOptions[CHAT_ICS], 999999);
    SetWidgetText(&chatOptions[CHAT_IN], tmpLine, ChatDlg);
    SetInsertPos(&chatOptions[CHAT_IN], strlen(tmpLine));
}

void
ChatSwitch (int n)
{
    int i, j;
    char *v;
    if(chatOptions[CHAT_ICS].type == Skip) hidden = 0; // In Xaw there is no ICS pane we can hide behind
    Show(&chatOptions[CHAT_PANE], 0); // show
    if(hidden) ScheduleDelayedEvent(DelayedScroll, 50); // Awful!
    else ScheduleDelayedEvent(DelayedSetText, 50);
    GetWidgetText(&chatOptions[CHAT_IN], &v);
    if(hidden) { ASSIGN(icsLine, v); } else { ASSIGN(inputs[activePartner], v); }
    hidden = 0;
    activePartner = --n;
    if(!texts[n]) texts[n] = strdup("");
    dirty[n] = 0;
    SetWidgetText(&chatOptions[CHAT_OUT], texts[n], ChatDlg);
    SetInsertPos(&chatOptions[CHAT_OUT], strlen(texts[n]));
    SetWidgetText(&chatOptions[CHAT_PARTNER], chatPartner[n], ChatDlg);
    for(i=j=0; i<MAX_CHAT; i++) {
	SetWidgetLabel(&chatOptions[++j], *chatPartner[i] ? chatPartner[i] : _("New Chat"));
	SetColor(dirty[i] ? "#FFC000" : "#FFFFFF", &chatOptions[j]);
    }
    if(!inputs[n]) { ASSIGN(inputs[n], ""); }
//    SetWidgetText(&chatOptions[CHAT_IN], inputs[n], ChatDlg); // does not work (in this widget only)
//    SetInsertPos(&chatOptions[CHAT_IN], strlen(inputs[n]));
    tmpLine = inputs[n]; // for the delayed event
    HardSetFocus(&chatOptions[strcmp(chatPartner[n], "") ? CHAT_IN : CHAT_PARTNER], 0);
}

void
PaneSwitch ()
{
    char *v;
    Show(&chatOptions[CHAT_PANE], hidden = 1); // hide
    GetWidgetText(&chatOptions[CHAT_IN], &v);
    ASSIGN(inputs[activePartner], v);
    if(!icsLine) { ASSIGN(icsLine, ""); }
    tmpLine = icsLine; ScheduleDelayedEvent(DelayedSetText, 50);
//    SetWidgetText(&chatOptions[CHAT_IN], icsLine, ChatDlg); // does not work (in this widget only)
//    SetInsertPos(&chatOptions[CHAT_IN], strlen(icsLine));
}

void
ClearChat ()
{   // clear the chat to make it free for other use
    chatPartner[activePartner][0] = NULLCHAR;
    ASSIGN(texts[activePartner], "");
    ASSIGN(inputs[activePartner], "");
    SetWidgetText(&chatOptions[CHAT_PARTNER], "", ChatDlg);
    SetWidgetText(&chatOptions[CHAT_OUT], "", ChatDlg);
    SetWidgetText(&chatOptions[CHAT_IN], "", ChatDlg);
    SetWidgetLabel(&chatOptions[activePartner+1], _("New Chat"));
    HardSetFocus(&chatOptions[CHAT_PARTNER], 0);
}

static void
NewChat (char *name)
{   // open a chat on program request. If no empty one available, use last
    int i;
    for(i=0; i<MAX_CHAT-1; i++) if(!chatPartner[i][0]) break;
    safeStrCpy(chatPartner[i], name, MSG_SIZ);
    ChatSwitch(i+1);
}

void
ConsoleWrite(char *message, int count)
{
    if(shellUp[ChatDlg] && chatOptions[CHAT_ICS].type != Skip) { // in Xaw this is a no-op
	AppendColorized(&chatOptions[CHAT_ICS], message, count);
	SetInsertPos(&chatOptions[CHAT_ICS], 999999);
    }
}

void
ChatPopUp ()
{
    if(GenericPopUp(chatOptions, _("ICS Interaction"), ChatDlg, BoardWindow, NONMODAL, appData.topLevel))
	AddHandler(&chatOptions[CHAT_PARTNER], ChatDlg, 2), AddHandler(&chatOptions[CHAT_IN], ChatDlg, 2); // treats return as OK
    Show(&chatOptions[CHAT_PANE], hidden = 1); // hide
//    HardSetFocus(&chatOptions[CHAT_IN], 0);
    MarkMenu("View.OpenChatWindow", ChatDlg);
    CursorAtEnd(&chatOptions[CHAT_IN]);
}

void
ChatProc ()
{
    if(shellUp[ChatDlg]) PopDown(ChatDlg);
    else ChatPopUp();
}

void
ConsoleAutoPopUp (char *buf)
{
	if(!appData.autoBox) return;
	if(appData.icsActive) { // text typed to board in ICS mode: divert to ICS input box
	    if(DialogExists(ChatDlg)) { // box already exists: append to current contents
		char *p, newText[MSG_SIZ];
		GetWidgetText(&chatOptions[CHAT_IN], &p);
		snprintf(newText, MSG_SIZ, "%s%c", p, *buf);
		SetWidgetText(&chatOptions[CHAT_IN], newText, ChatDlg);
		if(shellUp[ChatDlg]) HardSetFocus (&chatOptions[CHAT_IN], ChatDlg); //why???
	    } else { ASSIGN(line, buf); } // box did not exist: make sure it pops up with char in it
	    ChatPopUp();
	} else PopUpMoveDialog(*buf);
}

//--------------------------------- Game-List options dialog ------------------------------------------

char *strings[LPUSERGLT_SIZE];
int stringPtr;

void
GLT_ClearList ()
{
    strings[0] = NULL;
    stringPtr = 0;
}

void
GLT_AddToList (char *name)
{
    strings[stringPtr++] = name;
    strings[stringPtr] = NULL;
}

Boolean
GLT_GetFromList (int index, char *name)
{
  safeStrCpy(name, strings[index], MSG_SIZ);
  return TRUE;
}

void
GLT_DeSelectList ()
{
}

static void GLT_Button P((int n));
static int GLT_OK P((int n));

static Option listOptions[] = {
{300, LR|TB, 200, NULL, (void*) strings, NULL, NULL, ListBox, "" }, // For GTK we need to specify a height, as default would just show 3 lines
{ 0,    0,     0, NULL, (void*) &GLT_Button, NULL, NULL, Button, N_("factory") },
{ 0, SAME_ROW, 0, NULL, (void*) &GLT_Button, NULL, NULL, Button, N_("up") },
{ 0, SAME_ROW, 0, NULL, (void*) &GLT_Button, NULL, NULL, Button, N_("down") },
{ 0, SAME_ROW, 0, NULL, (void*) &GLT_OK, "", NULL, EndMark , "" }
};

static int
GLT_OK (int n)
{
    GLT_ParseList();
    appData.gameListTags = strdup(lpUserGLT);
    GameListUpdate();
    return 1;
}

static void
GLT_Button (int n)
{
    int index = SelectedListBoxItem (&listOptions[0]);
    char *p;
    if (index < 0) {
	DisplayError(_("No tag selected"), 0);
	return;
    }
    p = strings[index];
    if (n == 3) {
        if(index >= strlen(GLT_ALL_TAGS)) return;
	strings[index] = strings[index+1];
	strings[++index] = p;
        LoadListBox(&listOptions[0], "?", index, index-1); // only change the two specified entries
    } else
    if (n == 2) {
        if(index == 0) return;
	strings[index] = strings[index-1];
	strings[--index] = p;
        LoadListBox(&listOptions[0], "?", index, index+1);
    } else
    if (n == 1) {
      safeStrCpy(lpUserGLT, GLT_DEFAULT_TAGS, LPUSERGLT_SIZE);
      GLT_TagsToList(lpUserGLT);
      index = 0;
      LoadListBox(&listOptions[0], "?", -1, -1);
    }
    HighlightListBoxItem(&listOptions[0], index);
}

void
GameListOptionsPopUp (DialogClass parent)
{
    safeStrCpy(lpUserGLT, appData.gameListTags, LPUSERGLT_SIZE);
    GLT_TagsToList(lpUserGLT);

    GenericPopUp(listOptions, _("Game-list options"), TransientDlg, parent, MODAL, 0);
}

void
GameListOptionsProc ()
{
    GameListOptionsPopUp(BoardWindow);
}

//----------------------------- Error popup in various uses -----------------------------

/*
 * [HGM] Note:
 * XBoard has always had some pathologic behavior with multiple simultaneous error popups,
 * (which can occur even for modal popups when asynchrounous events, e.g. caused by engine, request a popup),
 * and this new implementation reproduces that as well:
 * Only the shell of the last instance is remembered in shells[ErrorDlg] (which replaces errorShell),
 * so that PopDowns ordered from the code always refer to that instance, and once that is down,
 * have no clue as to how to reach the others. For the Delete Window button calling PopDown this
 * has now been repaired, as the action routine assigned to it gets the shell passed as argument.
 */

int errorUp = False;

void
ErrorPopDown ()
{
    if (!errorUp) return;
    dialogError = errorUp = False;
    PopDown(ErrorDlg); PopDown(FatalDlg); // on explicit request we pop down any error dialog
    if (errorExitStatus != -1) ExitEvent(errorExitStatus);
}

static int
ErrorOK (int n)
{
    dialogError = errorUp = False;
    PopDown(n == 1 ? FatalDlg : ErrorDlg); // kludge: non-modal dialogs have one less (dummy) option
    if (errorExitStatus != -1) ExitEvent(errorExitStatus);
    return FALSE; // prevent second Popdown !
}

static Option errorOptions[] = {
{   0,  0,    0, NULL, NULL, NULL, NULL, Label,  NULL }, // dummy option: will never be displayed
{   0,  0,    0, NULL, NULL, NULL, NULL, Label,  NULL }, // textValue field will be set before popup
{ 0,NO_CANCEL,0, NULL, (void*) &ErrorOK, "", NULL, EndMark , "" }
};

void
ErrorPopUp (char *title, char *label, int modal)
{
    errorUp = True;
    errorOptions[1].name = label;
    if(dialogError = shellUp[TransientDlg])
	GenericPopUp(errorOptions+1, title, FatalDlg, TransientDlg, MODAL, 0); // pop up as daughter of the transient dialog
    else
	GenericPopUp(errorOptions+modal, title, modal ? FatalDlg: ErrorDlg, BoardWindow, modal, 0); // kludge: option start address indicates modality
}

void
DisplayError (String message, int error)
{
    char buf[MSG_SIZ];

    if (error == 0) {
	if (appData.debugMode || appData.matchMode) {
	    fprintf(stderr, "%s: %s\n", programName, message);
	}
    } else {
	if (appData.debugMode || appData.matchMode) {
	    fprintf(stderr, "%s: %s: %s\n",
		    programName, message, strerror(error));
	}
	snprintf(buf, sizeof(buf), "%s: %s", message, strerror(error));
	message = buf;
    }
    ErrorPopUp(_("Error"), message, FALSE);
}


void
DisplayMoveError (String message)
{
    fromX = fromY = -1;
    ClearHighlights();
    DrawPosition(TRUE, NULL); // selective redraw would miss the from-square of the rejected move, displayed empty after drag, but not marked damaged!
    if (appData.debugMode || appData.matchMode) {
	fprintf(stderr, "%s: %s\n", programName, message);
    }
    if (appData.popupMoveErrors) {
	ErrorPopUp(_("Error"), message, FALSE);
    } else {
	DisplayMessage(message, "");
    }
}


void
DisplayFatalError (String message, int error, int status)
{
    char buf[MSG_SIZ];

    errorExitStatus = status;
    if (error == 0) {
	fprintf(stderr, "%s: %s\n", programName, message);
    } else {
	fprintf(stderr, "%s: %s: %s\n",
		programName, message, strerror(error));
	snprintf(buf, sizeof(buf), "%s: %s", message, strerror(error));
	message = buf;
    }
    if(mainOptions[W_BOARD].handle) {
	if (appData.popupExitMessage) {
	    ErrorPopUp(status ? _("Fatal Error") : _("Exiting"), message, TRUE);
	} else {
	    ExitEvent(status);
	}
    }
}

void
DisplayInformation (String message)
{
    ErrorPopDown();
    ErrorPopUp(_("Information"), message, TRUE);
}

void
DisplayNote (String message)
{
    ErrorPopDown();
    ErrorPopUp(_("Note"), message, FALSE);
}

void
DisplayTitle (char *text)
{
    char title[MSG_SIZ];
    char icon[MSG_SIZ];

    if (text == NULL) text = "";

    if(partnerUp) { SetDialogTitle(DummyDlg, text); return; }

    if (*text != NULLCHAR) {
      safeStrCpy(icon, text, sizeof(icon)/sizeof(icon[0]) );
      safeStrCpy(title, text, sizeof(title)/sizeof(title[0]) );
    } else if (appData.icsActive) {
        snprintf(icon, sizeof(icon), "%s", appData.icsHost);
	snprintf(title, sizeof(title), "%s: %s", programName, appData.icsHost);
    } else if (appData.cmailGameName[0] != NULLCHAR) {
        snprintf(icon, sizeof(icon), "%s", "CMail");
	snprintf(title,sizeof(title), "%s: %s", programName, "CMail");
#ifdef GOTHIC
    // [HGM] license: This stuff should really be done in back-end, but WinBoard already had a pop-up for it
    } else if (gameInfo.variant == VariantGothic) {
      safeStrCpy(icon,  programName, sizeof(icon)/sizeof(icon[0]) );
      safeStrCpy(title, GOTHIC,     sizeof(title)/sizeof(title[0]) );
#endif
#ifdef FALCON
    } else if (gameInfo.variant == VariantFalcon) {
      safeStrCpy(icon, programName, sizeof(icon)/sizeof(icon[0]) );
      safeStrCpy(title, FALCON, sizeof(title)/sizeof(title[0]) );
#endif
    } else if (appData.noChessProgram) {
      safeStrCpy(icon, programName, sizeof(icon)/sizeof(icon[0]) );
      safeStrCpy(title, programName, sizeof(title)/sizeof(title[0]) );
    } else {
      safeStrCpy(icon, first.tidy, sizeof(icon)/sizeof(icon[0]) );
	snprintf(title,sizeof(title), "%s: %s", programName, first.tidy);
    }
    SetWindowTitle(text, title, icon);
}

#define PAUSE_BUTTON "P"
#define PIECE_MENU_SIZE 18
static String pieceMenuStrings[2][PIECE_MENU_SIZE+1] = {
    { N_("White"), "----", N_("Pawn"), N_("Knight"), N_("Bishop"), N_("Rook"),
      N_("Queen"), N_("King"), "----", N_("Elephant"), N_("Cannon"),
      N_("Archbishop"), N_("Chancellor"), "----", N_("Promote"), N_("Demote"),
      N_("Empty square"), N_("Clear board"), NULL },
    { N_("Black"), "----", N_("Pawn"), N_("Knight"), N_("Bishop"), N_("Rook"),
      N_("Queen"), N_("King"), "----", N_("Elephant"), N_("Cannon"),
      N_("Archbishop"), N_("Chancellor"), "----", N_("Promote"), N_("Demote"),
      N_("Empty square"), N_("Clear board"), NULL }
};
/* must be in same order as pieceMenuStrings! */
static ChessSquare pieceMenuTranslation[2][PIECE_MENU_SIZE] = {
    { WhitePlay, (ChessSquare) 0, WhitePawn, WhiteKnight, WhiteBishop,
	WhiteRook, WhiteQueen, WhiteKing, (ChessSquare) 0, WhiteAlfil,
	WhiteCannon, WhiteAngel, WhiteMarshall, (ChessSquare) 0,
	PromotePiece, DemotePiece, EmptySquare, ClearBoard },
    { BlackPlay, (ChessSquare) 0, BlackPawn, BlackKnight, BlackBishop,
	BlackRook, BlackQueen, BlackKing, (ChessSquare) 0, BlackAlfil,
	BlackCannon, BlackAngel, BlackMarshall, (ChessSquare) 0,
	PromotePiece, DemotePiece, EmptySquare, ClearBoard },
};

#define DROP_MENU_SIZE 6
static String dropMenuStrings[DROP_MENU_SIZE+1] = {
    "----", N_("Pawn"), N_("Knight"), N_("Bishop"), N_("Rook"), N_("Queen"), NULL
  };
/* must be in same order as dropMenuStrings! */
static ChessSquare dropMenuTranslation[DROP_MENU_SIZE] = {
    (ChessSquare) 0, WhitePawn, WhiteKnight, WhiteBishop,
    WhiteRook, WhiteQueen
};

// [HGM] experimental code to pop up window just like the main window, using GenercicPopUp

static Option *Exp P((int n, int x, int y));
void MenuCallback P((int n));
void SizeKludge P((int n));
static Option *LogoW P((int n, int x, int y));
static Option *LogoB P((int n, int x, int y));

static int pmFromX = -1, pmFromY = -1;
void *userLogo;

void
DisplayLogos (Option *w1, Option *w2)
{
	void *whiteLogo = first.programLogo, *blackLogo = second.programLogo;
	if(appData.autoLogo) {
	  if(appData.noChessProgram) whiteLogo = blackLogo = NULL;
	  if(appData.icsActive) whiteLogo = blackLogo = second.programLogo;
	  switch(gameMode) { // pick logos based on game mode
	    case IcsObserving:
		whiteLogo = second.programLogo; // ICS logo
		blackLogo = second.programLogo;
	    default:
		break;
	    case IcsPlayingWhite:
		if(!appData.zippyPlay) whiteLogo = userLogo;
		blackLogo = second.programLogo; // ICS logo
		break;
	    case IcsPlayingBlack:
		whiteLogo = second.programLogo; // ICS logo
		blackLogo = appData.zippyPlay ? first.programLogo : userLogo;
		break;
	    case TwoMachinesPlay:
	        if(first.twoMachinesColor[0] == 'b') {
		    whiteLogo = second.programLogo;
		    blackLogo = first.programLogo;
		}
		break;
	    case MachinePlaysWhite:
		blackLogo = userLogo;
		break;
	    case MachinePlaysBlack:
		whiteLogo = userLogo;
		blackLogo = first.programLogo;
	  }
	}
	DrawLogo(w1, whiteLogo);
	DrawLogo(w2, blackLogo);
}

static void
PMSelect (int n)
{   // user callback for board context menus
    if (pmFromX < 0 || pmFromY < 0) return;
    if(n == W_DROP) DropMenuEvent(dropMenuTranslation[values[n]], pmFromX, pmFromY);
    else EditPositionMenuEvent(pieceMenuTranslation[n - W_MENUW][values[n]], pmFromX, pmFromY);
}

static void
CCB (int n)
{
    shiftKey = (ShiftKeys() & 3) != 0;
    if(n < 0) { // button != 1
	n = -n;
	if(shiftKey && (gameMode == MachinePlaysWhite || gameMode == MachinePlaysBlack)) {
	    AdjustClock(n == W_BLACK, 1);
	}
    } else
    ClockClick(n == W_BLACK);
}

Option mainOptions[] = { // description of main window in terms of generic dialog creator
{ 0, 0xCA, 0, NULL, NULL, "", NULL, BarBegin, "" }, // menu bar
  { 0, COMBO_CALLBACK, 0, NULL, (void*)&MenuCallback, NULL, NULL, DropDown, N_("File") },
  { 0, COMBO_CALLBACK, 0, NULL, (void*)&MenuCallback, NULL, NULL, DropDown, N_("Edit") },
  { 0, COMBO_CALLBACK, 0, NULL, (void*)&MenuCallback, NULL, NULL, DropDown, N_("View") },
  { 0, COMBO_CALLBACK, 0, NULL, (void*)&MenuCallback, NULL, NULL, DropDown, N_("Mode") },
  { 0, COMBO_CALLBACK, 0, NULL, (void*)&MenuCallback, NULL, NULL, DropDown, N_("Action") },
  { 0, COMBO_CALLBACK, 0, NULL, (void*)&MenuCallback, NULL, NULL, DropDown, N_("Engine") },
  { 0, COMBO_CALLBACK, 0, NULL, (void*)&MenuCallback, NULL, NULL, DropDown, N_("Options") },
  { 0, COMBO_CALLBACK, 0, NULL, (void*)&MenuCallback, NULL, NULL, DropDown, N_("Help") },
{ 0, 0, 0, NULL, (void*)&SizeKludge, "", NULL, BarEnd, "" },
{ 0, LR|T2T|BORDER|SAME_ROW, 0, NULL, NULL, NULL, NULL, Label, "1" }, // optional title in window
{ 50,    LL|TT,            100, NULL, (void*) &LogoW, NULL, NULL, Skip, "" }, // white logo
{ 12,   L2L|T2T,           200, NULL, (void*) &CCB, NULL, NULL, Label, "White" }, // white clock
{ 13,   R2R|T2T|SAME_ROW,  200, NULL, (void*) &CCB, NULL, NULL, Label, "Black" }, // black clock
{ 50,    RR|TT|SAME_ROW,   100, NULL, (void*) &LogoB, NULL, NULL, Skip, "" }, // black logo
{ 0, LR|T2T|BORDER,        401, NULL, NULL, "", NULL, Skip, "2" }, // backup for title in window (if no room for other)
{ 0, LR|T2T|BORDER,        270, NULL, NULL, NULL, NULL, Label, "message", &appData.font }, // message field
{ 0, RR|TT|SAME_ROW,       125, NULL, NULL, "", NULL, BoxBegin, "" }, // (optional) button bar
  { 0,    0,     0, NULL, (void*) &ToStartEvent,  NULL, NULL, Button, N_("<<"), &appData.font },
  { 0, SAME_ROW, 0, NULL, (void*) &BackwardEvent, NULL, NULL, Button, N_("<"),  &appData.font },
  { 0, SAME_ROW, 0, NULL, (void*) &PauseEvent,    NULL, NULL, Button, N_(PAUSE_BUTTON), &appData.font },
  { 0, SAME_ROW, 0, NULL, (void*) &ForwardEvent,  NULL, NULL, Button, N_(">"),  &appData.font },
  { 0, SAME_ROW, 0, NULL, (void*) &ToEndEvent,    NULL, NULL, Button, N_(">>"), &appData.font },
{ 0, 0, 0, NULL, NULL, "", NULL, BoxEnd, "" },
{ 401, LR|TB, 401, NULL, (char*) &Exp, NULL, NULL, Graph, "shadow board" }, // board
  { 2, COMBO_CALLBACK, 0, NULL, (void*) &PMSelect, NULL, pieceMenuStrings[0], PopUp, "menuW" },
  { 2, COMBO_CALLBACK, 0, NULL, (void*) &PMSelect, NULL, pieceMenuStrings[1], PopUp, "menuB" },
  { -1, COMBO_CALLBACK, 0, NULL, (void*) &PMSelect, NULL, dropMenuStrings, PopUp, "menuD" },
{ 0,  NO_OK, 0, NULL, NULL, "", NULL, EndMark , "" }
};

Option *
LogoW (int n, int x, int y)
{
    if(n == 10) DisplayLogos(&mainOptions[W_WHITE-1], NULL);
    return NULL;
}

Option *
LogoB (int n, int x, int y)
{
    if(n == 10) DisplayLogos(NULL, &mainOptions[W_BLACK+1]);
    return NULL;
}

void
SizeKludge (int n)
{   // callback called by GenericPopUp immediately after sizing the menu bar
    int width = BOARD_WIDTH*(squareSize + lineGap) + lineGap;
    int w = width - 44 - mainOptions[n].min;
    mainOptions[W_TITLE].max = w; // width left behind menu bar
    if(w < 0.4*width) // if no reasonable amount of space for title, force small layout
	mainOptions[W_SMALL].type = mainOptions[W_TITLE].type, mainOptions[W_TITLE].type = Skip;
}

void
MenuCallback (int n)
{
    MenuProc *proc = (MenuProc *) (((MenuItem*)(mainOptions[n].choice))[values[n]].proc);

    if(!proc) RecentEngineEvent(values[n] - firstEngineItem); else (proc)();
}

static Option *
Exp (int n, int x, int y)
{
    static int but1, but3, oldW, oldH;
    int menuNr = -3, sizing, f, r;
    TimeMark now;

    if(n == 0) { // motion
	if(SeekGraphClick(Press, x, y, 1)) return NULL;
	if((but1 || dragging == 2) && !PromoScroll(x, y)) DragPieceMove(x, y);
	if(but3) MovePV(x, y, lineGap + BOARD_HEIGHT * (squareSize + lineGap));
	if(appData.highlightDragging) {
	    f = EventToSquare(x, BOARD_WIDTH);  if ( flipView && f >= 0) f = BOARD_WIDTH - 1 - f;
	    r = EventToSquare(y, BOARD_HEIGHT); if (!flipView && r >= 0) r = BOARD_HEIGHT - 1 - r;
	    HoverEvent(x, y, f, r);
	}
	return NULL;
    }
    if(n != 10 && PopDown(PromoDlg)) fromX = fromY = -1; // user starts fiddling with board when promotion dialog is up
    else GetTimeMark(&now);
    shiftKey = ShiftKeys();
    controlKey = (shiftKey & 0xC) != 0;
    shiftKey = (shiftKey & 3) != 0;
    switch(n) {
	case  1: LeftClick(Press,   x, y), but1 = 1; break;
	case -1: LeftClick(Release, x, y), but1 = 0; break;
	case  2: shiftKey = !shiftKey;
	case  3: menuNr = RightClick(Press,   x, y, &pmFromX, &pmFromY), but3 = 1; break;
	case -2: shiftKey = !shiftKey;
	case -3: menuNr = RightClick(Release, x, y, &pmFromX, &pmFromY), but3 = 0; break;
	case  4: BackwardEvent(); break;
	case  5: ForwardEvent(); break;
	case 10:
	    sizing = (oldW != x || oldH != y);
	    oldW = x; oldH = y;
	    InitDrawingHandle(mainOptions + W_BOARD);
	    if(sizing && SubtractTimeMarks(&now, &programStartTime) > 10000) return NULL; // don't redraw while sizing (except at startup)
	    DrawPosition(True, NULL);
	default:
	    return NULL;
    }

    switch(menuNr) {
      case 0: return &mainOptions[shiftKey ? W_MENUW: W_MENUB];
      case 1: SetupDropMenu(); return &mainOptions[W_DROP];
      case 2:
      case -1: ErrorPopDown();
      case -2:
      default: break; // -3, so no clicks caught
    }
    return NULL;
}

Option *
BoardPopUp (int squareSize, int lineGap, void *clockFontThingy)
{
    int i, size = BOARD_WIDTH*(squareSize + lineGap) + lineGap, logo = appData.logoSize;
    int f = 2*appData.fixedSize; // width fudge, needed for unknown reasons to not clip board
    mainOptions[W_WHITE].choice = (char**) clockFontThingy;
    mainOptions[W_BLACK].choice = (char**) clockFontThingy;
    mainOptions[W_BOARD].value = BOARD_HEIGHT*(squareSize + lineGap) + lineGap;
    mainOptions[W_BOARD].max = mainOptions[W_SMALL].max = size; // board size
    mainOptions[W_SMALL].max = size - 2; // board title (subtract border!)
    mainOptions[W_BLACK].max = mainOptions[W_WHITE].max = size/2-3; // clock width
    mainOptions[W_MESSG].max = appData.showButtonBar ? size-135+f : size-2+f; // message
    mainOptions[W_MENU].max = size-40; // menu bar
    mainOptions[W_TITLE].type = appData.titleInWindow ? Label : Skip ;
    if(logo && logo <= size/4) { // Activate logos
	mainOptions[W_WHITE-1].type = mainOptions[W_BLACK+1].type = Graph;
	mainOptions[W_WHITE-1].max  = mainOptions[W_BLACK+1].max  = logo;
	mainOptions[W_WHITE-1].value= mainOptions[W_BLACK+1].value= logo/2;
	mainOptions[W_WHITE].min  |= SAME_ROW;
	mainOptions[W_WHITE].max  = mainOptions[W_BLACK].max  -= logo + 4;
	mainOptions[W_WHITE].name = mainOptions[W_BLACK].name = "Double\nHeight";
    }
    if(!appData.showButtonBar) for(i=W_BUTTON; i<W_BOARD; i++) mainOptions[i].type = Skip;
    for(i=0; i<8; i++) mainOptions[i+1].choice = (char**) menuBar[i].mi;
    AppendEnginesToMenu(appData.recentEngineList);
    GenericPopUp(mainOptions, "XBoard", BoardWindow, BoardWindow, NONMODAL, 1); // allways top-level
    return mainOptions;
}

static Option *
SlaveExp (int n, int x, int y)
{
    if(n == 10) { // expose event
	flipView = !flipView; partnerUp = !partnerUp;
	DrawPosition(True, NULL); // [HGM] dual: draw other board in other orientation
	flipView = !flipView; partnerUp = !partnerUp;
    }
    return NULL;
}

Option dualOptions[] = { // auxiliary board window
{ 0, L2L|T2T,              198, NULL, NULL, NULL, NULL, Label, "White" }, // white clock
{ 0, R2R|T2T|SAME_ROW,     198, NULL, NULL, NULL, NULL, Label, "Black" }, // black clock
{ 0, LR|T2T|BORDER,        401, NULL, NULL, NULL, NULL, Label, "This feature is experimental" }, // message field
{ 401, LR|TT, 401, NULL, (char*) &SlaveExp, NULL, NULL, Graph, "shadow board" }, // board
{ 0,  NO_OK, 0, NULL, NULL, "", NULL, EndMark , "" }
};

void
SlavePopUp ()
{
    int size = BOARD_WIDTH*(squareSize + lineGap) + lineGap;
    // copy params from main board
    dualOptions[0].choice = mainOptions[W_WHITE].choice;
    dualOptions[1].choice = mainOptions[W_BLACK].choice;
    dualOptions[3].value = BOARD_HEIGHT*(squareSize + lineGap) + lineGap;
    dualOptions[3].max = dualOptions[2].max = size; // board width
    dualOptions[0].max = dualOptions[1].max = size/2 - 3; // clock width
    GenericPopUp(dualOptions, "XBoard", DummyDlg, BoardWindow, NONMODAL, appData.topLevel);
    SlaveResize(dualOptions+3);
}

void
DisplayWhiteClock (long timeRemaining, int highlight)
{
    if(appData.noGUI) return;
    if(twoBoards && partnerUp) {
	DisplayTimerLabel(&dualOptions[0], _("White"), timeRemaining, highlight);
	return;
    }
    DisplayTimerLabel(&mainOptions[W_WHITE], _("White"), timeRemaining, highlight);
    if(highlight) SetClockIcon(0);
}

void
DisplayBlackClock (long timeRemaining, int highlight)
{
    if(appData.noGUI) return;
    if(twoBoards && partnerUp) {
	DisplayTimerLabel(&dualOptions[1], _("Black"), timeRemaining, highlight);
	return;
    }
    DisplayTimerLabel(&mainOptions[W_BLACK], _("Black"), timeRemaining, highlight);
    if(highlight) SetClockIcon(1);
}


//---------------------------------------------

void
DisplayMessage (char *message, char *extMessage)
{
  /* display a message in the message widget */

  char buf[MSG_SIZ];

  if (extMessage)
    {
      if (*message)
	{
	  snprintf(buf, sizeof(buf), "%s  %s", message, extMessage);
	  message = buf;
	}
      else
	{
	  message = extMessage;
	};
    };

    safeStrCpy(lastMsg, message, MSG_SIZ); // [HGM] make available

  /* need to test if messageWidget already exists, since this function
     can also be called during the startup, if for example a Xresource
     is not set up correctly */
  if(mainOptions[W_MESSG].handle)
    SetWidgetLabel(&mainOptions[W_MESSG], message);

  return;
}

//----------------------------------- File Browser -------------------------------

#ifdef HAVE_DIRENT_H
#include <dirent.h>
#else
#include <sys/dir.h>
#define dirent direct
#endif

#include <sys/stat.h>

#define MAXFILES 1000

static ChessProgramState *savCps;
static FILE **savFP;
static char *fileName, *extFilter, *savMode, **namePtr;
static int folderPtr, filePtr, oldVal, byExtension, extFlag, pageStart, cnt;
static char curDir[MSG_SIZ], title[MSG_SIZ], *folderList[MAXFILES], *fileList[MAXFILES];

static char *FileTypes[] = {
"Chess Games",
"Chess Positions",
"Tournaments",
"Opening Books",
"Sound files",
"Images",
"Settings (*.ini)",
"Log files",
"All files",
NULL,
"PGN",
"Old-Style Games",
"FEN",
"Old-Style Positions",
NULL,
NULL
};

static char *Extensions[] = {
".pgn .game",
".fen .epd .pos",
".trn",
".bin",
".wav",
".ini",
".log",
"",
"INVALID",
".pgn",
".game",
".fen",
".pos",
NULL,
""
};

void DirSelProc P((int n, int sel));
void FileSelProc P((int n, int sel));
void SetTypeFilter P((int n));
int BrowseOK P((int n));
void Switch P((int n));
void CreateDir P((int n));

Option browseOptions[] = {
{   0,    LR|T2T,      500, NULL, NULL, NULL, NULL, Label, title },
{   0,    L2L|T2T,     250, NULL, NULL, NULL, NULL, Label, N_("Directories:") },
{   0,R2R|T2T|SAME_ROW,100, NULL, NULL, NULL, NULL, Label, N_("Files:") },
{   0, R2R|TT|SAME_ROW, 70, NULL, (void*) &Switch, NULL, NULL, Button, N_("by name") },
{   0, R2R|TT|SAME_ROW, 70, NULL, (void*) &Switch, NULL, NULL, Button, N_("by type") },
{ 300,    L2L|TB,      250, NULL, (void*) folderList, (char*) &DirSelProc, NULL, ListBox, "" },
{ 300, R2R|TB|SAME_ROW,250, NULL, (void*) fileList, (char*) &FileSelProc, NULL, ListBox, "" },
{   0,       0,        300, NULL, (void*) &fileName, NULL, NULL, TextBox, N_("Filename:") },
{   0,    SAME_ROW,    120, NULL, (void*) &CreateDir, NULL, NULL, Button, N_("New directory") },
{   0, COMBO_CALLBACK, 150, NULL, (void*) &SetTypeFilter, NULL, FileTypes, ComboBox, N_("File type:") },
{   0,    SAME_ROW,      0, NULL, (void*) &BrowseOK, "", NULL, EndMark , "" }
};

int
BrowseOK (int n)
{
	if(!fileName[0]) { // it is enough to have a file selected
	    if(browseOptions[6].textValue) { // kludge: if callback specified we browse for file
		int sel = SelectedListBoxItem(&browseOptions[6]);
		if(sel < 0 || sel >= filePtr) return FALSE;
		ASSIGN(fileName, fileList[sel]);
	    } else { // we browse for path
		ASSIGN(fileName, curDir); // kludge: without callback we browse for path
	    }
	}
	if(!fileName[0]) return FALSE; // refuse OK when no file
	if(!savMode[0]) { // browsing for name only (dialog Browse button)
		if(fileName[0] == '/') // We already had a path name
		    snprintf(title, MSG_SIZ, "%s", fileName);
		else
		    snprintf(title, MSG_SIZ, "%s/%s", curDir, fileName);
		SetWidgetText((Option*) savFP, title, TransientDlg);
		currentCps = savCps; // could return to Engine Settings dialog!
		return TRUE;
	}
	*savFP = fopen(fileName, savMode);
	if(*savFP == NULL) return FALSE; // refuse OK if file not openable
	ASSIGN(*namePtr, fileName);
	ScheduleDelayedEvent(DelayedLoad, 50);
	currentCps = savCps; // not sure this is ever non-null
	return TRUE;
}

int
AlphaNumCompare (char *p, char *q)
{
    while(*p) {
	if(isdigit(*p) && isdigit(*q) && atoi(p) != atoi(q))
	     return (atoi(p) > atoi(q) ? 1 : -1);
	if(*p != *q) break;
	p++, q++;
    }
    if(*p == *q) return 0;
    return (*p > *q ? 1 : -1);
}

int
Comp (const void *s, const void *t)
{
    char *p = *(char**) s, *q = *(char**) t;
    if(extFlag) {
	char *h; int r;
	while(h = strchr(p, '.')) p = h+1;
	if(p == *(char**) s) p = "";
	while(h = strchr(q, '.')) q = h+1;
	if(q == *(char**) t) q = "";
	r = AlphaNumCompare(p, q);
	if(r) return r;
    }
    return AlphaNumCompare( *(char**) s, *(char**) t );
}

void
ListDir (int pathFlag)
{
	DIR *dir;
	struct dirent *dp;
	struct stat statBuf;
	static int lastFlag;

	if(pathFlag < 0) pathFlag = lastFlag;
	lastFlag = pathFlag;
	dir = opendir(".");
	getcwd(curDir, MSG_SIZ);
	snprintf(title, MSG_SIZ, "%s   %s", _("Contents of"), curDir);
	folderPtr = filePtr = cnt = 0; // clear listing

	while (dp = readdir(dir)) { // pass 1: list foders
	    char *s = dp->d_name;
	    if(!stat(s, &statBuf) && S_ISDIR(statBuf.st_mode)) { // stat succeeds and tells us it is directory
		if(s[0] == '.' && strcmp(s, "..")) continue; // suppress hidden, except ".."
		ASSIGN(folderList[folderPtr], s); if(folderPtr < MAXFILES-2) folderPtr++;
	    } else if(!pathFlag) {
		char *s = dp->d_name, match=0;
//		if(cnt == pageStart) { ASSIGN }
		if(s[0] == '.') continue; // suppress hidden files
		if(extFilter[0]) { // [HGM] filter on extension
		    char *p = extFilter, *q;
		    do {
			if(q = strchr(p, ' ')) *q = 0;
			if(strstr(s, p)) match++;
			if(q) *q = ' ';
		    } while(q && (p = q+1));
		    if(!match) continue;
		}
		if(filePtr == MAXFILES-2) continue;
		if(cnt++ < pageStart) continue;
		ASSIGN(fileList[filePtr], s); filePtr++;
	    }
	}
	if(filePtr == MAXFILES-2) { ASSIGN(fileList[filePtr], _("  next page")); filePtr++; }
	FREE(folderList[folderPtr]); folderList[folderPtr] = NULL;
	FREE(fileList[filePtr]); fileList[filePtr] = NULL;
	closedir(dir);
	extFlag = 0;         qsort((void*)folderList, folderPtr, sizeof(char*), &Comp);
	extFlag = byExtension; qsort((void*)fileList, filePtr < MAXFILES-2 ? filePtr : MAXFILES-2, sizeof(char*), &Comp);
}

void
Refresh (int pathFlag)
{
    ListDir(pathFlag); // and make new one
    LoadListBox(&browseOptions[5], "", -1, -1);
    LoadListBox(&browseOptions[6], "", -1, -1);
    SetWidgetLabel(&browseOptions[0], title);
}

static char msg1[] = N_("FIRST TYPE DIRECTORY NAME HERE");
static char msg2[] = N_("TRY ANOTHER NAME");

void
CreateDir (int n)
{
    char *name, *errmsg = "";
    GetWidgetText(&browseOptions[n-1], &name);
    if(!strcmp(name, msg1) || !strcmp(name, msg2)) return;
    if(!name[0]) errmsg = _(msg1); else
    if(mkdir(name, 0755)) errmsg = _(msg2);
    else {
	chdir(name);
	Refresh(-1);
    }
    SetWidgetText(&browseOptions[n-1], errmsg, BrowserDlg);
}

void
Switch (int n)
{
    if(byExtension == (n == 4)) return;
    extFlag = byExtension = (n == 4);
    qsort((void*)fileList, filePtr < MAXFILES-2 ? filePtr : MAXFILES-2, sizeof(char*), &Comp);
    LoadListBox(&browseOptions[6], "", -1, -1);
}

void
SetTypeFilter (int n)
{
    int j = values[n];
    if(j == browseOptions[n].value) return; // no change
    browseOptions[n].value = j;
    SetWidgetLabel(&browseOptions[n], FileTypes[j]);
    ASSIGN(extFilter, Extensions[j]);
    pageStart = 0;
    Refresh(-1); // uses pathflag remembered by ListDir
    values[n] = oldVal; // do not disturb combo settings of underlying dialog
}

void
FileSelProc (int n, int sel)
{
    if(sel < 0 || fileList[sel] == NULL) return;
    if(sel == MAXFILES-2) { pageStart = cnt; Refresh(-1); return; }
    ASSIGN(fileName, fileList[sel]);
    if(BrowseOK(0)) PopDown(BrowserDlg);
}

void
DirSelProc (int n, int sel)
{
    if(!chdir(folderList[sel])) { // cd succeeded, so we are in new directory now
	Refresh(-1);
    }
}

void
Browse (DialogClass dlg, char *label, char *proposed, char *ext, Boolean pathFlag, char *mode, char **name, FILE **fp)
{
    int j=0;
    savFP = fp; savMode = mode, namePtr = name, savCps = currentCps, oldVal = values[9]; // save params, for use in callback
    ASSIGN(extFilter, ext);
    ASSIGN(fileName, proposed ? proposed : "");
    for(j=0; Extensions[j]; j++) // look up actual value in list of possible values, to get selection nr
	if(extFilter && !strcmp(extFilter, Extensions[j])) break;
    if(Extensions[j] == NULL) { j++; ASSIGN(FileTypes[j], extFilter); }
    browseOptions[9].value = j;
    browseOptions[6].textValue = (char*) (pathFlag ? NULL : &FileSelProc); // disable file listbox during path browsing
    pageStart = 0; ListDir(pathFlag);
    currentCps = NULL;
    GenericPopUp(browseOptions, label, BrowserDlg, dlg, MODAL, 0);
    SetWidgetLabel(&browseOptions[9], FileTypes[j]);
}

static char *openName;
FileProc fileProc;
char *fileOpenMode;
FILE *openFP;

void
DelayedLoad ()
{
  (void) (*fileProc)(openFP, 0, openName);
}

void
FileNamePopUp (char *label, char *def, char *filter, FileProc proc, char *openMode)
{
    fileProc = proc;		/* I can't see a way not */
    fileOpenMode = openMode;	/*   to use globals here */
    FileNamePopUpWrapper(label, def, filter, proc, False, openMode, &openName, &openFP);
}

void
ActivateTheme (int col)
{
    if(appData.overrideLineGap >= 0) lineGap = appData.overrideLineGap; else lineGap = defaultLineGap;
    InitDrawingParams(strcmp(oldPieceDir, appData.pieceDirectory));
    InitDrawingSizes(-1, 0);
    DrawPosition(True, NULL);
}

