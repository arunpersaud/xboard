/*
 * UCI support thru Polyglot
 *
 * Author: Alessandro Scotti (Jan 2006)
 *
 * Copyright 2006 Alessandro Scotti
 *
 * Enhancement Copyright 2009, 2010, 2011, 2012, 2013, 2014 Free Software Foundation, Inc.
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
 * along with this program. If not, see http://www.gnu.org/licenses/.
 *
 * ------------------------------------------------------------------------
 */
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "backend.h"
Boolean GetArgValue(char *a);

void
InitEngineUCI (const char *iniDir, ChessProgramState *cps)
{   // replace engine command line by adapter command with expanded meta-symbols
    if( cps->isUCI ) {
        char *p, *q;
        char polyglotCommand[MSG_SIZ];

        if(cps->isUCI == 2) p = appData.ucciAdapter; else
        p = appData.adapterCommand;
        q = polyglotCommand;
        while(*p) {
          if(*p == '\\') p++; else
          if(*p == '%') { // substitute marker
            char argName[MSG_SIZ], buf[MSG_SIZ], *s = buf;
            if(*++p == '%') { // second %, expand as f or s in option name (e.g. %%cp -> fcp)
              *s++ = cps == &first ? 'f' : 's';
              p++;
            }
            while(isdigit(*p) || isalpha(*p)) *s++ = *p++; // copy option name
            *s = NULLCHAR;
            if(cps == &second) { // change options for first into those for second engine
              if(strstr(buf, "first") == buf) sprintf(argName, "second%s", buf+5); else
              if(buf[0] == 'f') sprintf(argName, "s%s", buf+1); else
		safeStrCpy(argName, buf, sizeof(argName)/sizeof(argName[0]));
            } else safeStrCpy(argName, buf, sizeof(argName)/sizeof(argName[0]));
            if(GetArgValue(argName)) { // look up value of option with this name
              s = argName;
              while(*s) *q++ = *s++;
            } else DisplayFatalError("Bad adapter command", 0, 1);
            continue;
          }
          if(*p) *q++ = *p++;
        }
        *q = NULLCHAR;
        cps->program = StrSave(polyglotCommand);
        cps->dir = appData.polyglotDir;
    }
}
