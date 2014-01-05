/*
 * lists.c -- Includefile of lists.c
 *
 * Copyright 1995, 2009, 2010, 2011, 2012, 2013, 2014 Free Software Foundation, Inc.
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

/*
 * This file could well be a part of backend.c, but I prefer it this
 * way.
 */

#ifndef XB_LISTS
#define XB_LISTS


/* Type definition: Node of a double linked list.
 */
typedef struct XB_ListNode {
    struct XB_ListNode *succ;
    struct XB_ListNode *pred;
} ListNode;


/* Type definition: Double linked list.
 *
 * The list structure consists of two ListNode's: The pred entry of
 * the head being the succ entry of the tail. Thus a list is empty
 * if and only if it consists of 2 nodes. :-)
 */
typedef struct {
    struct XB_ListNode *head;     /*  The list structure consists of two  */
    struct XB_ListNode *tail;     /*  ListNode's: The pred entry of the   */
    struct XB_ListNode *tailPred; /*  head being the succ entry of the    */
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
