/*
 * help.h
 *
 * Copyright 2009, 2010, 2011, 2012, 2013, 2014 Free Software Foundation, Inc.
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

/* Windows html help function to avoid having to link with the htmlhlp.lib  */

#include <windows.h>
#include <stdio.h>
#include "config.h"
#include "help.h"

FILE *debugFP;

HWND WINAPI
HtmlHelp( HWND hwnd, LPCSTR helpFile, UINT action, DWORD_PTR data )
{
	PROCESS_INFORMATION helpProcInfo;
	STARTUPINFO siStartInfo;
	char buf[100];
	static int status = 0;
	FILE *f;

	if(status < 0) return NULL;

	if(!status) {
		f = fopen(helpFile, "r");
		if(f == NULL) {
			status = -1;
			return NULL;
		}
		status = 1;
		fclose(f);
	}

	siStartInfo.cb = sizeof(STARTUPINFO);
	siStartInfo.lpReserved = NULL;
	siStartInfo.lpDesktop = NULL;
	siStartInfo.lpTitle = NULL;
	siStartInfo.dwFlags = STARTF_USESTDHANDLES;
	siStartInfo.cbReserved2 = 0;
	siStartInfo.lpReserved2 = NULL;
	siStartInfo.hStdInput = NULL;
	siStartInfo.hStdOutput = NULL;
	siStartInfo.hStdError = debugFP;

	snprintf(buf, sizeof(buf)/sizeof(buf[0]),"Hh.exe %s", helpFile);

	// ignore the other parameters; just start the viewer with the help file
	if(  CreateProcess(NULL,
			   buf,		   /* command line */
			   NULL,	   /* process security attributes */
			   NULL,	   /* primary thread security attrs */
			   FALSE,	   /* handles are inherited */
			   DETACHED_PROCESS|CREATE_NEW_PROCESS_GROUP,
			   NULL,	   /* use parent's environment */
			   NULL,
			   &siStartInfo,   /* STARTUPINFO pointer */
			   &helpProcInfo)  /* receives PROCESS_INFORMATION */
		) return hwnd; else return NULL;
}

//HWND WINAPI
int
MyHelp(HWND hwnd, LPSTR helpFile, UINT action, DWORD_PTR data)
{
	static int status = 0;
	FILE *f;

	if(status < 0) return 0;

	if(!status) {
		f = fopen(helpFile, "r");
		if(f == NULL) {
			status = -1;
			return 0;
		}
		status = 1;
		fclose(f);
	}
	return WinHelp(hwnd, helpFile, action, data);
}
