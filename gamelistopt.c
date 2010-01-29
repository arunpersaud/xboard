/*
 * gamelistopt.c -- Game list window for WinBoard
 *
 * Copyright 1995,2009 Free Software Foundation, Inc.
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

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <fcntl.h>
#include <math.h>

#include "common.h"
#include "frontend.h"
#include "backend.h"

void GLT_ClearList();
void GLT_DeSelectList();
void GLT_AddToList( char *name );
Boolean GLT_GetFromList( int index, char *name );

// back-end
typedef struct {
    char id;
    char * name;
} GLT_Item;

// back-end: translation table tag id-char <-> full tag name
static GLT_Item GLT_ItemInfo[] = {
    { GLT_EVENT,      "Event" },
    { GLT_SITE,       "Site" },
    { GLT_DATE,       "Date" },
    { GLT_ROUND,      "Round" },
    { GLT_PLAYERS,    "Players" },
    { GLT_RESULT,     "Result" },
    { GLT_WHITE_ELO,  "White Rating" },
    { GLT_BLACK_ELO,  "Black Rating" },
    { GLT_TIME_CONTROL,"Time Control" },
    { GLT_VARIANT,    "Variant" },
    { GLT_OUT_OF_BOOK,PGN_OUT_OF_BOOK },
    { GLT_RESULT_COMMENT, "Result Comment" }, // [HGM] rescom
    { 0, 0 }
};

char lpUserGLT[64];

// back-end: convert the tag id-char to a full tag name
char * GLT_FindItem( char id )
{
    char * result = 0;

    GLT_Item * list = GLT_ItemInfo;

    while( list->id != 0 ) {
        if( list->id == id ) {
            result = list->name;
            break;
        }

        list++;
    }

    return result;
}

// back-end: build the list of tag names
void
GLT_TagsToList( char * tags )
{
    char * pc = tags;

    GLT_ClearList();

    while( *pc ) {
        GLT_AddToList( GLT_FindItem(*pc) );
        pc++;
    }

    GLT_AddToList( "\t --- Hidden tags ---" );

    pc = GLT_ALL_TAGS;

    while( *pc ) {
        if( strchr( tags, *pc ) == 0 ) {
            GLT_AddToList( GLT_FindItem(*pc) );
        }
        pc++;
    }

    GLT_DeSelectList();
}

// back-end: retrieve item from dialog and translate to id-char
char
GLT_ListItemToTag( int index )
{
    char result = '\0';
    char name[128];

    GLT_Item * list = GLT_ItemInfo;

    if( GLT_GetFromList(index, name) ) {
        while( list->id != 0 ) {
            if( strcmp( list->name, name ) == 0 ) {
                result = list->id;
                break;
            }

            list++;
        }
    }

    return result;
}

// back-end: add items id-chars one-by-one to temp tags string
void
GLT_ParseList()
{
    char * pc = lpUserGLT;
    int idx = 0;
    char id;

    do {
	id = GLT_ListItemToTag( idx );
	*pc++ = id;
	idx++;
    } while( id != '\0' );
}

