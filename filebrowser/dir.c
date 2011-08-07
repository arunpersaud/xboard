/*
 * Copyright 1989 Software Research Associates, Inc., Tokyo, Japan
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Software Research Associates not be used
 * in advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.  Software Research Associates
 * makes no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * SOFTWARE RESEARCH ASSOCIATES DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
 * IN NO EVENT SHALL SOFTWARE RESEARCH ASSOCIATES BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Erik M. van der Poel
 *         Software Research Associates, Inc., Tokyo, Japan
 *         erik@sra.co.jp
 */

#include <stdio.h>
#include <stdlib.h> /* for qsort */
#include "../config.h" /* to check for dirent.h */

#ifdef SEL_FILE_IGNORE_CASE
#include <ctype.h>
#endif /* def SEL_FILE_IGNORE_CASE */

#ifdef HAVE_DIRENT_H
#include <dirent.h>
#else
#include <sys/dir.h>
#define dirent direct
#endif

#include <sys/stat.h>

#include "selfile.h"

#ifdef SEL_FILE_IGNORE_CASE
int
SFcompareEntries(p, q)
	SFEntry	*p;
	SFEntry	*q;
{
	register char	*r, *s;
	register char	c1, c2;

	r = p->real;
	s = q->real;

	c1 = *r++;
	if (islower(c1)) {
		c1 = toupper(c1);
	}
	c2 = *s++;
	if (islower(c2)) {
		c2 = toupper(c2);
	}

	while (c1 == c2) {
		if (!c1) {
			return strcmp(p->real, q->real);
		}
		c1 = *r++;
		if (islower(c1)) {
			c1 = toupper(c1);
		}
		c2 = *s++;
		if (islower(c2)) {
			c2 = toupper(c2);
		}
	}

	return c1 - c2;
}
#else /* def SEL_FILE_IGNORE_CASE */
int
SFcompareEntries(p, q)
	SFEntry	*p;
	SFEntry	*q;
{
	return strcmp(p->real, q->real);
}
#endif /* def SEL_FILE_IGNORE_CASE */

int
SFgetDir(dir)
	SFDir	*dir;
{
	SFEntry		*result = NULL;
	int		alloc = 0;
	int		i;
	DIR		*dirp;
	struct dirent	*dp;
	char		*str;
	int		len;
	int		maxChars;
	struct stat	statBuf;

	maxChars = strlen(dir->dir) - 1;

	dir->entries = NULL;
	dir->nEntries = 0;
	dir->nChars = 0;

	result = NULL;
	i = 0;

	dirp = opendir(".");
	if (!dirp) {
		return 1;
	}

	(void) stat(".", &statBuf);
	dir->mtime = statBuf.st_mtime;

	while (dp = readdir(dirp)) {

		struct stat statBuf;
		if(!strcmp(dp->d_name, ".")) continue; /* Throw away "." */
		if(!strcmp(dp->d_name, "..")) continue; /* Throw away ".." */
#ifndef S_IFLNK
#endif /* ndef S_IFLNK */
		if (i >= alloc) {
			alloc = 2 * (alloc + 1);
			result = (SFEntry *) XtRealloc((char *) result,
				(unsigned) (alloc * sizeof(SFEntry)));
		}
		result[i].statDone = 0;
		str = dp->d_name;
		len = strlen(str);
		result[i].real = XtMalloc((unsigned) (len + 2));
		(void) strcat(strcpy(result[i].real, str), " ");
		if (len > maxChars) {
			maxChars = len;
		}
		result[i].shown = result[i].real;
		if(SFpathFlag) { // [HGM] only show directories
			if (stat(str, &statBuf) || SFstatChar(&statBuf) != '/') continue;
		} else if(SFfilterBuffer[0]) { // [HGM] filter on extension
		    char *p = SFfilterBuffer, match, *q;
		    match = !(stat(str, &statBuf) || SFstatChar(&statBuf) != '/');
		    do {
			if(q = strchr(p, ' ')) *q = 0;
			if(strstr(str, p)) match++;
			if(q) *q = ' ';
		    } while(q && (p = q+1));
		    if(!match) continue;
		}
		i++;
	}

	qsort((char *) result, (size_t) i, sizeof(SFEntry), SFcompareEntries);

	dir->entries = result;
	dir->nEntries = i;
	dir->nChars = maxChars + 1;

	closedir(dirp);

	return 0;
}
