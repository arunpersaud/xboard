/*
 * wengineo.h -- Clipboard routines for WinBoard
 *
 * Copyright 2000, 2009, 2010, 2011, 2012, 2013, 2014 Free Software Foundation, Inc.
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
 *------------------------------------------------------------------------
 ** See the file ChangeLog for a revision history.  */

// [HGM] define numbers to indicate icons, for referring to them in platform-independent way
#define nColorBlack   1
#define nColorWhite   2
#define nColorUnknown 3
#define nClear        4
#define nPondering    5
#define nThinking     6
#define nAnalyzing    7

// [HGM] same for output fields (note that there are two of each type, one per color)
#define nColorIcon 1
#define nLabel     2
#define nStateIcon 3
#define nStateData 4
#define nLabelNPS  5
#define nMemo      6 /* must match the ranking of the TextBox in following option list! */

#define STRIDE 7 /* must match the number of option in one pane! */
#define MEMO  (nMemo-1)

/* Module variables */
#define H_MARGIN            2
#define V_MARGIN            2
#define LABEL_V_DISTANCE    1   /* Distance between label and memo */
#define SPLITTER_SIZE       4   /* Distance between first memo and second label */

#define ICON_SIZE           14

enum ENGINE_STATE { STATE_UNKNOWN, STATE_THINKING, STATE_IDLE, STATE_PONDERING, STATE_ANALYZING };

extern int  windowMode;
extern char exclusionHeader[];
extern int highTextStart[2], highTextEnd[2];

// back-end called by front-end
void SetEngineState( int which, enum ENGINE_STATE state, char * state_data );

// front-end called by back-end
void SetEngineColorIcon( int which );
void SetIcon( int which, int field, int nIcon );
void DoSetWindowText(int which, int field, char *s_label);
void InsertIntoMemo( int which, char * text, int where );
void DoClearMemo(int which);
void ResizeWindowControls( int mode );
int  EngineOutputDialogExists();
void SetEngineOutputTitle( char *title );
void Collapse P((int colNr));
void InitEngineOutput P((Option *opt, Option *memo2)); // XB only
void DrawWidgetIcon P((Option *opt, int nIcon)); // XB only
