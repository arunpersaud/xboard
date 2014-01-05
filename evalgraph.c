/*
 * evalgraph.c - Evaluation graph back-end part
 *
 * Author: Alessandro Scotti (Dec 2005)
 *
 * Copyright 2005 Alessandro Scotti
 *
 * Enhancments Copyright 2009, 2010, 2011, 2012, 2013, 2014 Free Software Foundation, Inc.
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

// code refactored by HGM to obtain front-end / back-end separation

#include "config.h"

#include <stdio.h>

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
#include "evalgraph.h"

/* Module globals */
ChessProgramStats_Move * currPvInfo;
int currFirst = 0;
int currLast = 0;
int currCurrent = -1;
int range = 1;

int nWidthPB = 0;
int nHeightPB = 0;

int MarginX = 18;
int MarginW = 4;
int MarginH = 4;

// back-end
static void
DrawLine (int x1, int y1, int x2, int y2, int penType)
{
    DrawSegment( x1, y1, NULL, NULL, PEN_NONE );
    DrawSegment( x2, y2, NULL, NULL, penType );
}

// back-end
static void
DrawLineEx (int x1, int y1, int x2, int y2, int penType)
{
    int savX, savY;
    DrawSegment( x1, y1, &savX, &savY, PEN_NONE );
    DrawSegment( x2, y2, NULL, NULL, penType );
    DrawSegment( savX, savY, NULL, NULL, PEN_NONE );
}

// back-end
static int
GetPvScore (int index)
{
    int score = currPvInfo[ index ].score;

    if( index & 1 ) score = -score; /* Flip score for black */

    return score;
}

char *
MakeEvalTitle (char *title)
{
    int score, depth;
    static char buf[MSG_SIZ];

    if( currCurrent <0 ) return title; // currCurrent = -1 crashed WB on start without ini file!
    score = currPvInfo[ currCurrent ].score;
    depth = currPvInfo[ currCurrent ].depth;

    if( depth <=0 ) return title;
    if( currCurrent & 1 ) score = -score; /* Flip score for black */
    snprintf(buf, MSG_SIZ, "%s {%d: %s%.2f/%-2d %d}", title, currCurrent/2+1,
				score>0 ? "+" : " ", score/100., depth, (currPvInfo[currCurrent].time+50)/100);

    return buf;
}

// back-end
/*
    For a centipawn value, this function returns the height of the corresponding
    histogram, centered on the reference axis.

    Note: height can be negative!
*/
static int
GetValueY (int value)
{
    if( value < -range*700 ) value = -range*700;
    if( value > +range*700 ) value = +range*700;
    if(value > 100*range)  value += (appData.zoom - 1)*100*range; else
    if(value < -100*range) value -= (appData.zoom - 1)*100*range; else
	value *= appData.zoom;
    return (nHeightPB / 2) - (int)(value * (nHeightPB - 2*MarginH) / ((1200. + 200.*appData.zoom)*range));
}

// the brush selection is made part of the DrawLine, by passing a style argument
// the wrapper for doing the text output makes this back-end
static void
DrawAxisSegmentHoriz (int value, Boolean drawValue)
{
    int y = GetValueY( range*value*100 );

    if( drawValue ) {
        char buf[MSG_SIZ], *b = buf;

        if( value > 0 ) *b++ = '+';
	sprintf(b, "%d", range*value);

	DrawEvalText(buf, strlen(buf), y);
    }
    // [HGM] counts on DrawEvalText to have select transparent background for dotted line!
    DrawLine( MarginX, y, MarginX + MarginW, y, PEN_BLACK ); // Y-axis tick marks
    DrawLine( MarginX + MarginW, y, nWidthPB - MarginW, y, PEN_DOTTED ); // hor grid
}

// The DrawLines again must select their own brush.
// the initial brush selection is useless? BkMode needed for dotted line and text
static void
DrawAxis ()
{
    int cy = nHeightPB / 2, space = nHeightPB/(6 + appData.zoom);

    DrawAxisSegmentHoriz( +5, TRUE );
    DrawAxisSegmentHoriz( +3, space >= 20 );
    DrawAxisSegmentHoriz( +1, space >= 20 && space*appData.zoom >= 40 );
    DrawAxisSegmentHoriz(  0, TRUE );
    DrawAxisSegmentHoriz( -1, space >= 20 && space*appData.zoom >= 40 );
    DrawAxisSegmentHoriz( -3, space >= 20 );
    DrawAxisSegmentHoriz( -5, TRUE );

    DrawLine( MarginX + MarginW, cy, nWidthPB - MarginW, cy, PEN_BLACK ); // x-axis
    DrawLine( MarginX + MarginW, MarginH, MarginX + MarginW, nHeightPB - MarginH, PEN_BLACK ); // y-axis
}

// back-end
static void
DrawHistogram (int x, int y, int width, int value, int side)
{
    int left, top, right, bottom;

    if( value > -appData.evalThreshold*range && value < +appData.evalThreshold*range ) return;

    left = x;
    right = left + width + 1;

    if( value > 0 ) {
        top = GetValueY( value );
        bottom = y+1;
    }
    else {
        top = y;
        bottom = GetValueY( value ) + 1;
    }


    if( width == MIN_HIST_WIDTH ) {
        right--;
        DrawRectangle( left, top, right, bottom, side, FILLED );
    }
    else {
        DrawRectangle( left, top, right, bottom, side, OPEN );
    }
}

// back-end
static void
DrawSeparator (int index, int x)
{
    if( index > 0 ) {
        if( index == currCurrent ) {
            DrawLineEx( x, MarginH, x, nHeightPB - MarginH, PEN_BLUEDOTTED );
        }
        else if( (index % 20) == 0 ) {
            DrawLineEx( x, MarginH, x, nHeightPB - MarginH, PEN_DOTTED );
        }
    }
}

// made back-end by replacing MoveToEx and LineTo by DrawSegment
/* Actually draw histogram as a diagram, cause there's too much data */
static void
DrawHistogramAsDiagram (int cy, int paint_width, int hist_count)
{
    double step;
    int i;

    /* Rescale the graph every few moves (as opposed to every move) */
    hist_count -= hist_count % 8;
    hist_count += 8;
    hist_count /= 2;

    step = (double) paint_width / (hist_count + 1);

    for( i=0; i<2; i++ ) {
        int index = currFirst;
        int side = (currCurrent + i + 1) & 1; /* Draw current side last */
        double x = MarginX + MarginW;

        if( (index & 1) != side ) {
            x += step / 2;
            index++;
        }

        DrawSegment( (int) x, cy, NULL, NULL, PEN_NONE );

        index += 2;

        while( index < currLast ) {
            x += step;

            DrawSeparator( index, (int) x );

            /* Extend line up to current point */
            if( currPvInfo[index].depth > 0 ) {
	      DrawSegment((int) x, GetValueY( GetPvScore(index) ), NULL, NULL, (side==0 ? PEN_BOLDWHITE: PEN_BOLDBLACK) );
            }

            index += 2;
        }
    }
}

// back-end, delete pen selection
static void
DrawHistogramFull (int cy, int hist_width, int hist_count)
{
    int i;

//    SelectObject( hdcPB, GetStockObject(BLACK_PEN) );

    for( i=0; i<hist_count; i++ ) {
        int index = currFirst + i;
        int x = MarginX + MarginW + index * hist_width;

        /* Draw a separator every 10 moves */
        DrawSeparator( index, x );

        /* Draw histogram */
        if( currPvInfo[i].depth > 0 ) {
            DrawHistogram( x, cy, hist_width, GetPvScore(index), index & 1 );
        }
    }
}

typedef struct {
    int cy;
    int hist_width;
    int hist_count;
    int paint_width;
} VisualizationData;

// back-end
static Boolean
InitVisualization (VisualizationData *vd)
{
    Boolean result = FALSE;

    vd->cy = nHeightPB / 2;
    vd->hist_width = MIN_HIST_WIDTH;
    vd->hist_count = currLast - currFirst;
    vd->paint_width = nWidthPB - MarginX - 2*MarginW;

    if( vd->hist_count > 0 ) {
        result = TRUE;

        /* Compute width */
        vd->hist_width = vd->paint_width / vd->hist_count;

        if( vd->hist_width > MAX_HIST_WIDTH ) vd->hist_width = MAX_HIST_WIDTH;

        vd->hist_width -= vd->hist_width % 2;
    }

    return result;
}

// back-end
static void
DrawHistograms ()
{
    VisualizationData vd;

    if( InitVisualization( &vd ) ) {
        if( vd.hist_width < MIN_HIST_WIDTH ) {
            DrawHistogramAsDiagram( vd.cy, vd.paint_width, vd.hist_count );
        }
        else {
            DrawHistogramFull( vd.cy, vd.hist_width, vd.hist_count );
        }
    }
}

// back-end
int
GetMoveIndexFromPoint (int x, int y)
{
    int result = -1;
    int start_x = MarginX + MarginW;
    VisualizationData vd;

    if( x >= start_x && InitVisualization( &vd ) ) {
        /* Almost an hack here... we duplicate some of the paint logic */
        if( vd.hist_width < MIN_HIST_WIDTH ) {
            double step;

            vd.hist_count -= vd.hist_count % 8;
            vd.hist_count += 8;
            vd.hist_count /= 2;

            step = (double) vd.paint_width / (vd.hist_count + 1);
            step /= 2;

            result = (int) (0.5 + (double) (x - start_x) / step);
        }
        else {
            result = (x - start_x) / vd.hist_width;
        }
    }

    if( result >= currLast ) {
        result = -1;
    }

    return result;
}

// init and display part split of so they can be moved to front end
void
PaintEvalGraph (void)
{
    VariantClass v = gameInfo.variant;
    range = (gameInfo.holdingsWidth && v != VariantSuper && v != VariantGreat && v != VariantSChess) ? 2 : 1; // [HGM] double range in drop games
    /* Draw */
    DrawRectangle(0, 0, nWidthPB, nHeightPB, 2, FILLED);
    DrawAxis();
    DrawHistograms();
}
