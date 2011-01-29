/*
 * xgamelist.h -- Game list window, part of X front end for XBoard
 *
 * Copyright 1995, 2009, 2010, 2011 Free Software Foundation, Inc.
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

#ifndef _XGAMEL_H
#define _XGAMEL_H 1

void ShowGameListProc P((Widget w, XEvent *event,
			 String *prms, Cardinal *nprms));
void LoadSelectedProc P((Widget w, XEvent *event,
			 String *prms, Cardinal *nprms));
void SetFilterProc P((Widget w, XEvent *event,
			 String *prms, Cardinal *nprms));

extern Widget gameListShell;
#endif /* _XGAMEL_H */
