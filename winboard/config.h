/* config.h.in.  Generated automatically from configure.in by autoheader.  */


/* Define if you have <sys/wait.h> that is POSIX.1 compatible.  */
/*#undef HAVE_SYS_WAIT_H*/

/* Define if you need to in order for stat and other things to work.  */
/*#undef _POSIX_SOURCE*/

/* Define as the return type of signal handlers (int or void).  */
/*#undef RETSIGTYPE*/

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/* Define if you can safely include both <sys/time.h> and <time.h>.  */
/*#undef TIME_WITH_SYS_TIME*/

/* Define if lex declares yytext as a char * by default, not a char[].  */
/*#undef YYTEXT_POINTER*/

/*#define FIRST_PTY_LETTER 'p'*/

#define HAVE_FCNTL_H 1

#define HAVE_GETHOSTNAME 0

#define HAVE_GETTIMEOFDAY 0

/* Use our own random() defined in winboard.c. */
#define HAVE_RANDOM 0

#define HAVE_SYS_SOCKET_H 0

/*#undef IBMRTAIX*/

#define LAST_PTY_LETTER 'q'

/* Name of package */
#define PACKAGE "WinBoard"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "bug-xboard@gnu.org"

/* Define to the full name of this package. */
#define PACKAGE_NAME "WinBoard"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "WinBoard 4.8.0"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "winboard"

/* Define to the version of this package. */
#define PACKAGE_VERSION "4.8.0"

/* Define the Windows-specific FILE version info.  this *MUST* be four comma separated 16-bit integers */
/* remember to not start a number with 0 (octal), dates like 2014,0901 would lead to an error */
#define PACKAGE_FILEVERSION 4,8,2014,929

#define PTY_ITERATION

#define PTY_NAME_SPRINTF

#define PTY_TTY_NAME_SPRINTF

#define REMOTE_SHELL ""

/*#undef RTU*/

/*#undef UNIPLUS*/

#define USE_PTYS 0

/*#undef X_WCHAR*/

#ifndef __BORLANDC__
#define WIN32 1
#else
#define WIN32
#endif

#define ZIPPY 1

/* Define if you have the _getpty function.  */
/*#undef HAVE__GETPTY*/

/* Define if you have the ftime function.  */
#define HAVE_FTIME 1

/* Define if you have the grantpt function.  */
/*#undef HAVE_GRANTPT*/

/* Define if you have the rand48 function.  */
/*#undef HAVE_RAND48*/

/* Define if you have the sysinfo function.  */
/*#undef HAVE_SYSINFO*/

/* Define if you have the <lan/socket.h> header file.  */
/*#undef HAVE_LAN_SOCKET_H*/

/* Define if you have the <string.h> header file.  */
#define HAVE_STRING_H 1

/* Define if you have the <stropts.h> header file.  */
/*#undef HAVE_STROPTS_H*/

/* Define if you have the <sys/fcntl.h> header file.  */
#define HAVE_SYS_FCNTL_H 0

/* Define if you have the <sys/systeminfo.h> header file.  */
/*#undef HAVE_SYS_SYSTEMINFO_H*/

/* Define if you have the <sys/time.h> header file.  */
/*#undef HAVE_SYS_TIME_H*/

/* Define if you have the <unistd.h> header file.  */
/*#undef HAVE_UNISTD_H*/

/* Define if you have the i library (-li).  */
/*#undef HAVE_LIBI*/

/* Define if you have the seq library (-lseq).  */
/*#undef HAVE_LIBSEQ*/

/*
  Options
  -DEMULATE_RSH -DREMOTE_SHELL=\"\" is necessary on Windows 95, because it
    does not have its own rsh command.  It works better this way on NT too,
    because the NT rsh does not propagate signals to the remote process.
  -DATTENTION is included even though I haven't been able to send signals to
    child processes on Windows, because at least I can send them over rsh to
    Unix programs.  On Windows I send a newline instead, which wakes up the
    chess program if it's polling.  On my GNU Chess port the newline actually 
    works even for Move Now.
*/
#define EMULATE_RSH 1
#define ATTENTION 1

#ifdef __BORLANDC__
#define _strdup(x) strdup(x)
#define STRICT
#define _winmajor 3  /* windows 95 */
#endif

/* Some definitions required by MSVC 4.1 */ 
#ifndef WM_MOUSEWHEEL 
#define WM_MOUSEWHEEL 0x020A 
#endif 
#ifndef SCF_DEFAULT 
#define SCF_DEFAULT 0x0000 
#define SCF_ALL 0x0004 
#endif 

#ifdef _MSC_VER
#define snprintf _snprintf
#define inline __inline
#if _MSC_VER < 1500
#define vsnprintf _vsnprintf
#endif
#endif
