#include "wplugin.h"

static char * makePluginExeName( const char * name )
{
    char buf[ MAX_PATH ];

    strcpy( buf, "" );

    strcat( buf, "plugins\\" );
    strcat( buf, name );
    strcat( buf, ".exe" );

    return strdup( buf );
}

WbPlugin * wbpCreate( const char * name )
{
    char buf[MAX_PATH];
    int result = 0;

    // Create the plugin
    WbPlugin * plugin = (WbPlugin *) malloc( sizeof(WbPlugin) );

    memset( plugin, 0, sizeof(WbPlugin) );

    plugin->name_ = strdup( name );
    plugin->exe_name_ = makePluginExeName( name );
    plugin->hPipe_ = INVALID_HANDLE_VALUE;
    plugin->hProcess_ = INVALID_HANDLE_VALUE;

    // Create the named pipe for plugin communication
    if( result == 0 ) {
        strcpy( buf, "\\\\.\\pipe\\" );
        strcat( buf, name );

        plugin->hPipe_ = CreateNamedPipe( buf,
            PIPE_ACCESS_DUPLEX,
            0, // Byte mode
            2, // Max instances
            4*1024,
            4*1024,
            1000, // Default timeout
            NULL );

        if( plugin->hPipe_ == INVALID_HANDLE_VALUE ) {
            DWORD err = GetLastError();
            result = -1;
        }
    }

    // Create the plugin process
    if( result == 0 ) {
        STARTUPINFO si;
        PROCESS_INFORMATION pi;

        ZeroMemory( &si, sizeof(si) );
        ZeroMemory( &pi, sizeof(pi) );

        si.cb = sizeof(si);

        strcpy( buf, "\"" );
        strcat( buf, plugin->exe_name_ );
        strcat( buf, "\"" );

        strcpy( buf, "\"C:\\Program Files\\Borland\\Delphi5\\Projects\\History\\History.exe\"" );

        if( CreateProcess( NULL,
            buf,
            NULL,
            NULL,
            FALSE, // Inherit handles
            0, // Creation flags
            NULL, // Environment
            NULL, // Current directory
            &si,
            &pi ) )
        {
            CloseHandle( pi.hThread );
            plugin->hProcess_ = pi.hProcess;
        }
        else {
            result = -2;
        }
    }

    // Destroy the plugin instance if something went wrong
    if( result != 0 ) {
        wbpDelete( plugin );
        plugin = 0;
    }

    return plugin;
}

void wbpDelete( WbPlugin * plugin )
{
    if( plugin != 0 ) {
        if( plugin->hPipe_ != INVALID_HANDLE_VALUE ) {
            CloseHandle( plugin->hPipe_ );
        }

        if( plugin->hProcess_ != INVALID_HANDLE_VALUE ) {
            CloseHandle( plugin->hProcess_ );
        }

        free( plugin->name_ );

        free( plugin->exe_name_ );

        plugin->name_ = 0;
        plugin->exe_name_ = 0;
        plugin->hPipe_ = INVALID_HANDLE_VALUE;
        plugin->hProcess_ = INVALID_HANDLE_VALUE;

        free( plugin );
    }
}

int wbpSendMessage( WbPlugin * plugin, const char * msg, size_t msg_len )
{
    int result = -1;

    if( plugin != 0 && plugin->hPipe_ != INVALID_HANDLE_VALUE ) {
        DWORD zf = 0;
        BOOL ok = TRUE;

        while( ok && (msg_len > 0) ) {
            DWORD cb = 0;

            ok = WriteFile( plugin->hPipe_,
                msg,
                msg_len,
                &cb,
                NULL );

            if( ok ) {
                if( cb > msg_len ) break; // Should *never* happen!

                msg_len -= cb;
                msg += cb;
            }

            if( cb == 0 ) {
                zf++;
                if( zf >= 3 ) ok = FALSE;
            }
            else {
                zf = 0;
            }
        }

        if( ok ) {
            result = 0;
        }
    }

    return result;
}

int wbpListInit( WbPluginList * list )
{
    list->item_count_ = 0;

    return 0;
}

int wbpListAdd( WbPluginList * list, WbPlugin * plugin )
{
    int result = -1;

    if( plugin != 0 ) {
        if( list->item_count_ < MaxWbPlugins ) {
            list->item_[ list->item_count_ ] = plugin;
            list->item_count_++;

            result = 0;
        }
    }

    return result;
}

WbPlugin * wbpListGet( WbPluginList * list, int index )
{
    WbPlugin * result = 0;

    if( index >= 0 && index < list->item_count_ ) {
        result = list->item_[ index ];
    }

    return result;
}

int wbpListGetCount( WbPluginList * list )
{
    return list->item_count_;
}

int wbpListDeleteAll( WbPluginList * list )
{
    int i;

    for( i=0; i<list->item_count_; i++ ) {
        wbpDelete( list->item_[i] );
    }

    return wbpListInit( list );
}

int wbpListBroadcastMessage( WbPluginList * list, const char * msg, size_t msg_len )
{
    int result = 0;
    int i;

    for( i=0; i<list->item_count_; i++ ) {
        if( wbpSendMessage( list->item_[i], msg, msg_len ) == 0 ) {
            result++;
        }
        else {
            // Error sending message to plugin...
        }
    }

    return result;
}
