/*
 * wplugin.h
 *
 * Copyright 2009 Free Software Foundation, Inc.
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

#ifndef WPLUGIN_H_
#define WPLUGIN_H_

#include <windows.h>

#define MaxWbPlugins 16

typedef struct WbPlugin_tag
{
    char * name_;
    char * exe_name_;
    HANDLE hPipe_;
    HANDLE hProcess_;
} WbPlugin;

typedef struct WbPluginList_tag
{
    int item_count_;
    WbPlugin * item_[MaxWbPlugins];
} WbPluginList;

WbPlugin * wbpCreate( const char * name );

void wbpDelete( WbPlugin * plugin );

int wbpSendMessage( WbPlugin * plugin, const char * msg, size_t msg_len );

int wbpListInit( WbPluginList * list );

int wbpListAdd( WbPluginList * list, WbPlugin * plugin );

WbPlugin * wbpListGet( WbPluginList * list, int index );

int wbpListGetCount( WbPluginList * list );

int wbpListDeleteAll( WbPluginList * list );

int wbpListBroadcastMessage( WbPluginList * list, const char * msg, size_t msg_len );

#endif // WPLUGIN_H_
