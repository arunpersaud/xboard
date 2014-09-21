/*
 * evalgraph.h -- Evaluation Graph window
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
 ** See the file ChangeLog for a revision history.
 */

#define MIN_HIST_WIDTH  4
#define MAX_HIST_WIDTH  10

typedef enum { PEN_NONE, PEN_BLACK, PEN_DOTTED, PEN_BLUEDOTTED, PEN_BOLDWHITE, PEN_BOLDBLACK, PEN_BACKGD, PEN_ANY } PenType;

#define FILLED 1
#define OPEN   0

/* Module globals */
ChessProgramStats_Move * currPvInfo;
extern int currFirst;
extern int currLast;
extern int currCurrent;
extern int differentialView;

extern int nWidthPB;
extern int nHeightPB;

extern int MarginX;
extern int MarginW;
extern int MarginH;

// calls from back-end part into front-end part
void DrawSegment( int x, int y, int *lastX, int *lastY, int p );
void DrawRectangle( int left, int top, int right, int bottom, int side, int style );
void DrawEvalText(char *buf, int cbBuf, int y);
void EvalGraphSet P(( int first, int last, int current, ChessProgramStats_Move * pvInfo ));

// calls of front-end part into back-end part
extern int GetMoveIndexFromPoint( int x, int y );
extern void PaintEvalGraph( void );
extern char *MakeEvalTitle( char *title );
