/*
 * xgamelist.h -- Game list window, part of X front end for XBoard
 * $Id: xhistory.h,v 2.1 2003/10/27 19:21:00 mann Exp $
 *
 * Copyright 1995 Free Software Foundation, Inc.
 *
 * The following terms apply to the enhanced version of XBoard distributed
 * by the Free Software Foundation:
 * ------------------------------------------------------------------------
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 * ------------------------------------------------------------------------
 *
 * See the file ChangeLog for a revision history.
 */

#ifndef _XHISTL_H
#define _XHISTL_H 1

void HistoryShowProc P((Widget w, XEvent *event,
			String *prms, Cardinal *nprms));
void HistoryPopDown P((Widget w, XtPointer client_data,
		       XtPointer call_data));

#endif /* _XHISTL_H */
 
