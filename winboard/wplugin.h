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
