/*
 * lists.c -- Includefile of lists.c
 * XBoard $Id: lists.h,v 2.1 2003/10/27 19:21:00 mann Exp $
 *
 * Copyright 1995 Free Software Foundation, Inc.
 *
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111 USA.
 * ------------------------------------------------------------------------
 *
 * This file could well be a part of backend.c, but I prefer it this
 * way.
 */

#ifndef _LISTS_H
#define _LISTS_H


/* Type definition: Node of a double linked list.
 */
typedef struct _ListNode {
    struct _ListNode *succ;
    struct _ListNode *pred;
} ListNode;


/* Type definition: Double linked list.
 *
 * The list structure consists of two ListNode's: The pred entry of
 * the head being the succ entry of the tail. Thus a list is empty
 * if and only if it consists of 2 nodes. :-)
 */
typedef struct {
    struct _ListNode *head;     /*  The list structure consists of two  */
    struct _ListNode *tail;     /*  ListNode's: The pred entry of the   */
    struct _ListNode *tailPred; /*  head being the succ entry of the    */
} List;                         /*  tail.                               */



/* Function prototypes
 */
extern int ListEmpty P((List *));
void ListNew P((List *));
void ListRemove P((ListNode *));
void ListNodeFree P((ListNode *));
ListNode *ListNodeCreate P((size_t));
void ListInsert P((ListNode *, ListNode *));
void ListAddHead P((List *, ListNode *));
void ListAddTail P((List *, ListNode *));
ListNode *ListElem P((List *, int));


#endif
