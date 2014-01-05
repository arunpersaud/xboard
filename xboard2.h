/*
 * xboard2.h -- Move list window, part of X front end for XBoard
 *
 * Copyright 2012, 2013, 2014 Free Software Foundation, Inc.
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

void SendToProgram P((char *message, ChessProgramState *cps));
void SendToICS P((char *buf));
void InitDrawingSizes P((int i, int j));

extern int searchTime;
extern int squareSize, lineGap, defaultLineGap;
extern int startedFromPositionFile;
extern char *icsTextMenuString;
extern int hi2X, hi2Y;
