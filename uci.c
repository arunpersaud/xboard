/*
 * UCI support thru Polyglot
 *
 * Author: Alessandro Scotti (Jan 2006)
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * ------------------------------------------------------------------------
 */
#include "config.h"

#include <windows.h> /* required for all Windows applications */

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include "common.h"
#include "winboard.h"
#include "frontend.h"
#include "backend.h"

#define INIFILE_PREFIX      "polyglot_"
#define INIFILE_SUFFIX_1ST  "1st"
#define INIFILE_SUFFIX_2ND  "2nd"
#define INIFILE_EXT         ".ini"

static const char * GetIniFilename( ChessProgramState * cps )
{
    return cps == &first ? INIFILE_PREFIX INIFILE_SUFFIX_1ST INIFILE_EXT : INIFILE_PREFIX INIFILE_SUFFIX_2ND INIFILE_EXT;
 }

void InitEngineUCI( const char * iniDir, ChessProgramState * cps )
{
    if( cps->isUCI ) {
        const char * iniFileName = GetIniFilename( cps );
        char polyglotIniFile[ MAX_PATH ];
        FILE * f;

        /* Build name of initialization file */
        if( strchr( iniDir, ' ' ) != NULL ) {
            char iniDirShort[ MAX_PATH ];

            GetShortPathName( iniDir, iniDirShort, sizeof(iniDirShort) );

            strcpy( polyglotIniFile, iniDirShort );
        }
        else {
            strcpy( polyglotIniFile, iniDir );
        }
        
        strcat( polyglotIniFile, "\\" );
        strcat( polyglotIniFile, iniFileName );

        /* Create initialization file */
        f = fopen( polyglotIniFile, "w" );

        if( f != NULL ) {
            fprintf( f, "[Polyglot]\n" );

            if( cps->dir != 0 && strlen(cps->dir) > 0 ) {
                fprintf( f, "EngineDir = %s\n", cps->dir );
            }

            if( cps->program != 0 && strlen(cps->program) > 0 ) {
                fprintf( f, "EngineCommand = %s\n", cps->program );
            }

            fprintf( f, "Book = %s\n", appData.usePolyglotBook ? "true" : "false" );
            fprintf( f, "BookFile = %s\n", appData.polyglotBook );
        
            fprintf( f, "[Engine]\n" );
            fprintf( f, "Hash = %d\n", appData.defaultHashSize );

            fprintf( f, "NalimovPath = %s\n", appData.defaultPathEGTB );
            fprintf( f, "NalimovCache = %d\n", appData.defaultCacheSizeEGTB );

            fprintf( f, "OwnBook = %s\n", cps->hasOwnBookUCI ? "true" : "false" );

            fclose( f );

            /* Replace program with properly configured Polyglot */
            cps->dir = appData.polyglotDir;
            cps->program = (char *) malloc( strlen(polyglotIniFile) + 32 );
            strcpy( cps->program, "polyglot " );
            strcat( cps->program, polyglotIniFile );
        }
    }
}
