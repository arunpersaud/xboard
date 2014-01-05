/*
 * lists.c -- Functions to implement a double linked list XBoard
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

#include "config.h"

#include <stdio.h>
#if STDC_HEADERS
# include <stdlib.h>
#endif /* not STDC_HEADERS */

#include "common.h"
#include "lists.h"



/* Check, if List l is empty; returns TRUE, if it is, FALSE
 * otherwise.
 */
int
ListEmpty (List *l)
{
    return(l->head == (ListNode *) &l->tail);
}


/* Initialize a list. Must be executed before list is used.
 */
void
ListNew (List *l)
{
    l->head = (ListNode *) &l->tail;
    l->tail = NULL;
    l->tailPred = (ListNode *) l;
}


/* Remove node n from the list it is inside.
 */
void
ListRemove (ListNode *n)
{
    if (n->succ != NULL) {  /*  Be safe  */
	n->pred->succ = n->succ;
	n->succ->pred = n->pred;
	n->succ = NULL;     /*  Mark node as no longer being member */
	n->pred = NULL;     /*  of a list.                          */
    }
}


/* Delete node n.
 */
void
ListNodeFree (ListNode *n)
{
    if (n) {
	ListRemove(n);
	free(n);
    }
}


/* Create a list node with size s. Returns NULL, if out of memory.
 */
ListNode *
ListNodeCreate (size_t s)
{
    ListNode *n;

    if ((n = (ListNode*) malloc(s))) {
	n->succ = NULL; /*  Mark node as not being member of a list.    */
	n->pred = NULL;
    }
    return(n);
}


/* Insert node n into the list of node m after m.
 */
void
ListInsert (ListNode *m, ListNode *n)
{
    n->succ = m->succ;
    n->pred = m;
    m->succ = n;
    n->succ->pred = n;
}


/* Add node n to the head of list l.
 */
void
ListAddHead (List *l, ListNode *n)
{
    ListInsert((ListNode *) &l->head, n);
}


/* Add node n to the tail of list l.
 */
void
ListAddTail (List *l, ListNode *n)
{
    ListInsert((ListNode *) l->tailPred, n);
}


/* Return element with number n of list l. (NULL, if n doesn't exist.)
 * Counting starts with 0.
 */
ListNode *
ListElem (List *l, int n)
{
    ListNode *ln;

    for (ln = l->head;  ln->succ;  ln = ln->succ) {
	if (!n--) {
	    return (ln);
	}
    }

    return(NULL);
}
