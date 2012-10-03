/*
 * xevalgraph.h
 *
 * Copyright 2010, 2011, 2012 Free Software Foundation, Inc.
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

#ifndef XB_XEVALGRAPH
#define XB_XEVALGRAPH

void EvalGraphSet P(( int first, int last, int current, ChessProgramStats_Move * pvInfo ));
float Color P((char *col, int n));
void SetPen P((cairo_t *cr, float w, char *col, int dash));

#endif
