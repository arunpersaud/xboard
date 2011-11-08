dnl| configure.in
dnl|
dnl| Copyright 1992-2001, 2002, 2003, 2004, 2005, 2006, 2007, 
dnl| 2008, 2009, 2010, 2011 Free Software Foundation, Inc.
dnl|
dnl| GNU XBoard is free software: you can redistribute it and/or modify
dnl| it under the terms of the GNU General Public License as published by
dnl| the Free Software Foundation, either version 3 of the License, or (at
dnl| your option) any later version.
dnl| 
dnl| GNU XBoard is distributed in the hope that it will be useful, but
dnl| WITHOUT ANY WARRANTY; without even the implied warranty of
dnl| MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
dnl| General Public License for more details.
dnl| 
dnl| You should have received a copy of the GNU General Public License
dnl| along with this program. If not, see http://www.gnu.org/licenses/.  
dnl| 
dnl| --------------------------------------------------------------------
dnl|
dnl| You can process this file with autoconf to produce a configure script.
dnl| However, normally the supplied configure script will work fine.
dnl|
dnl| If you do need to change the configure script, instead of editing
dnl| it directly, try to edit configure.in (in a way that will keep
dnl| it portable to sites and systems other than your own), and run autoconf 
dnl| to regenerate configure.  Then submit your changes to be folded into
dnl| the standard version of xboard.

dnl| define second argument as VERSION.PATCHLEVEL. e.g. 4.4.0j
AC_INIT([xboard],[master-20110822],[bug-xboard@gnu.org])

AM_INIT_AUTOMAKE()
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_CONFIG_HEADERS([config.h])

dnl | a bunch of templates for defines used below
AH_TEMPLATE([FIRST_PTY_LETTER],[template])
AH_TEMPLATE([HAVE_FCNTL_H],[template])
AH_TEMPLATE([HAVE_GETHOSTNAME],[template])
AH_TEMPLATE([HAVE_GETTIMEOFDAY],[template])
AH_TEMPLATE([HAVE_RANDOM],[template])
AH_TEMPLATE([HAVE_SYS_SOCKET_H],[template])
AH_TEMPLATE([IBMRTAIX],[template])
AH_TEMPLATE([LAST_PTY_LETTER],[template])
AH_TEMPLATE([PTY_ITERATION],[template])
AH_TEMPLATE([PTY_NAME_SPRINTF],[template])
AH_TEMPLATE([PTY_OPEN],[template])
AH_TEMPLATE([PTY_TTY_NAME_SPRINTF],[template])
AH_TEMPLATE([REMOTE_SHELL],[template])
AH_TEMPLATE([RTU],[template])
AH_TEMPLATE([UNIPLUS],[template])
AH_TEMPLATE([USE_PTYS],[template])
AH_TEMPLATE([X_WCHAR],[template])
AH_TEMPLATE([ATTENTION],[template])
AH_TEMPLATE([DEFINED_SYS_ERRLIST],[template])
AH_TEMPLATE([X_LOCALE],[template])

if test -z "$CFLAGS" ; then
dnl| Prevent the next macro from setting CFLAGS to -g
  CFLAGS=" "
fi
AC_PROG_CC

dnl| need this to be able to compile files in a subdir
AM_PROG_CC_C_O 

AC_PROG_CPP
AC_ISC_POSIX
AC_PROG_INSTALL

dnl| add gettext support
AM_GNU_GETTEXT_VERSION(0.17)
AM_GNU_GETTEXT([external])

AC_CHECK_PROGS(RSH, remsh rsh, rsh)
AC_CHECK_PROGS(MINFO, makeinfo, makeinfo_not_found)
if test "$MINFO" = makeinfo_not_found ; then
   echo Please install \"makeinfo\"
   exit 1
fi
AC_DEFINE_UNQUOTED(REMOTE_SHELL, "$RSH")
AC_CHECK_PROG(NROFF, nroff, [nroff -man], cat)
AC_SUBST(NROFFFLAGS)
AC_PATH_PROGS(AWKPATH, awk mawk gawk nawk)
AC_PATH_PROGS(PERLPATH, perl)

PKG_CHECK_MODULES([GTK], [gtk+-2.0 >= 2.16.0 gmodule-export-2.0 ])
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

AC_HEADER_STDC
AC_HEADER_TIME
AC_HEADER_SYS_WAIT
AC_HEADER_DIRENT
AC_TYPE_SIGNAL
AC_CHECK_HEADERS(stropts.h sys/time.h string.h unistd.h sys/systeminfo.h)
AC_CHECK_HEADERS(fcntl.h sys/fcntl.h, break)
AC_CHECK_HEADERS(sys/socket.h lan/socket.h, break)
AC_CHECK_HEADER(stddef.h, [], AC_DEFINE(X_WCHAR, 1))

AC_CHECK_FUNCS(_getpty grantpt setitimer usleep)
AC_CHECK_FUNCS(gettimeofday ftime, break)
AC_CHECK_FUNCS(random rand48, break)
AC_CHECK_FUNCS(gethostname sysinfo, break)
AC_CHECK_FUNC(setlocale, [],
AC_CHECK_LIB(i, setlocale, [], AC_DEFINE(X_LOCALE, 1)))

AC_CHECK_LIB(seq, getpseudotty)

dnl | test if user wants ot use Xaw3d or Xaw

AC_CANONICAL_HOST

dnl| The following info is mostly gathered from GNU Emacs 19.24.  Basically,
dnl| we are trying to find out whether this is a System-V derivative in
dnl| which pipes don't work with select() and if so, whether there is anything
dnl| strange about the way to open a pty.  Some of the work was done above
dnl| by looking for _getpty, grantpt, and getpseudotty.  A few other strange
dnl| properties of particular systems are also handled here.

dnl| 4/6/97 I'm not sure there really are any systems where pipes
dnl| don't work with select(), and ptys cause problems on many
dnl| systems, so I'm changing the default to disable ptys in all
dnl| cases.  I will change it back if I get bug reports that are fixed
dnl| by doing a "configure --enable-ptys"

USE_PTYS=0
case "$host" in
  *-*-hpux* )
    AC_DEFINE(PTY_TTY_NAME_SPRINTF,
	[sprintf (pty_name, "/dev/pty/tty%c%x", c, i);])
    AC_DEFINE(PTY_NAME_SPRINTF,
	[sprintf (pty_name, "/dev/ptym/pty%c%x", c, i);])
dnl| USE_PTYS=1
    if test "$GCC" = yes; then
      CONF_CFLAGS="-fwritable-strings"
    else

dnl| Note: You might be able to build xboard even if your compiler does not
dnl| support ANSI C (-Aa).  xboard itself does not require ANSI C.  I don't
dnl| know whether the X header files on HP-UX require it.

      CONF_CFLAGS="-Aa -D_HPUX_SOURCE"
    fi

dnl| HP doesn't supply a full set of X header files and libraries.  People
dnl| often have some things installed in one place and some in another. 
dnl| AC_PATH_XTRA will find only one place, so we try to add all the
dnl| likely ones that might be missing here.  It might be better to
dnl| change AC_PATH_XTRA to try to extract this information from imake,
dnl| since folks who install the missing bits often configure their
dnl| imake to find them, but I don't want to delve into autoconf and
dnl| hack on its internals.

  ;;

  romp-ibm-aix* )
    AC_DEFINE(IBMRTAIX, 1)
dnl| USE_PTYS=1
  ;;

  i386-ibm-aix )
dnl| USE_PTYS=1
    if test "$GCC" = yes; then
      CONF_CFLAGS="-fwritable-strings"
    fi
  ;;

  *-*-aix3* | *-*-bosx* )
    AC_DEFINE(PTY_ITERATION, [for (c = 0; !c; c++)])
    AC_DEFINE(PTY_NAME_SPRINTF, [strcpy (pty_name, "/dev/ptc");])
    AC_DEFINE(PTY_TTY_NAME_SPRINTF, [strcpy (pty_name, ttyname (fd));])
dnl| USE_PTYS=1
  ;;

  *-*-cxux* )
    AC_DEFINE(FIRST_PTY_LETTER, 'A')
    AC_DEFINE(LAST_PTY_LETTER, 'P')
dnl| USE_PTYS=1
  ;;

  *-*-uniplus* )
    AC_DEFINE(UNIPLUS, 1)
dnl| USE_PTYS=1
  ;;

  *-*-rtu* )
    AC_DEFINE(FIRST_PTY_LETTER, 'z')
    AC_DEFINE(PTY_TTY_NAME_SPRINTF,
        [sprintf (pty_name, "/dev/ttyp%x", i);])
    AC_DEFINE(PTY_NAME_SPRINTF,
	[sprintf (pty_name, "/dev/pty%x", i);])
    AC_DEFINE(RTU, 1)
dnl| USE_PTYS=1
  ;;

  *-*-iris* | *-*-irix3* )
    AC_DEFINE(PTY_ITERATION, [for (c = 0; !c; c++)])
    AC_DEFINE(PTY_NAME_SPRINTF, [strcpy (pty_name, "/dev/ptc");])
    AC_DEFINE(PTY_TTY_NAME_SPRINTF,
	[sprintf (pty_name, "/dev/ttyq%d", minor(stb.st_rdev));])
dnl| USE_PTYS=1
  ;;

  *-*-irix* )
dnl| USE_PTYS=1
  ;;

  *-*-sunos4* | *-*-solaris1* )

dnl| Work around a bug in the SunOS 4.x linker.  Not needed if you have patches
dnl|  100512-02 and 100573-03 from Sun.  The X FAQ says that the following is
dnl|  "overkill," but doesn't explain what should be done instead.

    if test "$GCC" = yes; then
      PRE_XMULIB="-static"
      POST_XMULIB="-dynamic"
    else
      PRE_XMULIB="-Bstatic"
      POST_XMULIB="-Bdynamic"
    fi
  ;;

  *-*-sco* )
    AC_DEFINE(PTY_ITERATION, [for (i = 0; ; i++)])
    AC_DEFINE(PTY_NAME_SPRINTF, [sprintf (pty_name, "/dev/ptyp%d", i);])
    AC_DEFINE(PTY_TTY_NAME_SPRINTF, [sprintf (pty_name, "/dev/ttyp%d", i);])
dnl| USE_PTYS=1
  ;;
	
  *-*-dynix* | *-*-ptx* )
dnl| USE_PTYS=1
  ;;

  *-*-esix* )
dnl| USE_PTYS=1
  ;;

  *-*-usg5-4* | *-*-sysvr4* )
dnl| USE_PTYS=1
  ;;

  *-*-usg* | *-*-sysv* | *-*-aix* )
dnl| USE_PTYS=1
  ;;

  vax-*-ultrix )
    if test "$GCC" = yes; then
      CONF_CFLAGS="-fwritable-strings"
    fi
  ;;

dnl| add some libs for OS X
  *-apple-* )
  ;;
esac

AC_SUBST(PRE_XMULIB)
AC_SUBST(POST_XMULIB)
AC_SUBST(CONF_CFLAGS)
AC_SUBST(CONF_LDFLAGS)

AC_MSG_CHECKING(whether ptys or pipes should be used)
AC_ARG_ENABLE( [ptys], 
	       [AS_HELP_STRING([--enable-ptys],[force use of pseudo-ttys with child processes])],
[if test "$enableval" = yes; then
  USE_PTYS=1
  enable_ptys="ptys"
  AC_MSG_RESULT([ptys (user override)])
fi
if test "$enableval" = no; then
  USE_PTYS=0
  enable_ptys="pipes"
  AC_MSG_RESULT([pipes (user override)])
fi],
[if test "$USE_PTYS" = 1; then
  AC_MSG_RESULT(ptys)
  enable_ptys="ptys"
else
  AC_MSG_RESULT(pipes)
  enable_ptys="pipes"
fi])
AC_DEFINE_UNQUOTED(USE_PTYS, $USE_PTYS)


dnl | define not to build zippy as a default, so that autoheader is happy
AC_DEFINE(ZIPPY, 0,[should zippy be enabled])
AC_ARG_ENABLE( [zippy], 
	       [AS_HELP_STRING([--enable-zippy],[support interfacing a chess program to ICS (default)])],
	       [], 
	       [enable_zippy="yes"])

if test x"$enable_zippy" != xno; then
  AC_DEFINE(ZIPPY, 1,[should zippy be enabled])
fi
AM_CONDITIONAL([ZIPPY], [test x$enable_zippy != xno])


AC_ARG_ENABLE(sigint, 
[AS_HELP_STRING([--enable-sigint],[sending SIGINT (^C) wakes up GNU Chess (default)])],
[if test "$enableval" = yes; then
  AC_DEFINE(ATTENTION, 1)
fi
enable_sigint=$enableval
],
[enable_sigint="yes"
AC_DEFINE(ATTENTION, 1)])


AC_ARG_ENABLE(enable_warnings, AC_HELP_STRING(--enable-warnings, [enable compiler warnings. [[default=no]]]
))
AC_MSG_CHECKING([if compiler warnings is to be enabled])
if test "$enable_warnings" != "yes"; then
  enable_warnings="no"
fi
AC_MSG_RESULT($enable_warnings)

if test "$enable_warnings" = "yes"; then
    CFLAGS="$CFLAGS -DGDK_PIXBUF_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED -Wunused-function -Wunused-label -Wunused-value -Wunused-variable "
fi


AC_CONFIG_FILES([Makefile cmail po/Makefile.in])
AC_CONFIG_COMMANDS([test-stamp-h],[test -z "$CONFIG_HEADERS" || date > stamp-h])
AC_CONFIG_COMMANDS([chmod-cmail],[chmod 755 cmail])
AC_OUTPUT

dnl
dnl Output a summary
dnl
echo ""
echo " Configurations summary:"
echo ""
echo "        prefix:          $prefix       "
echo "        datarootdir:     $datarootdir  "
echo "        datadir:         $datadir   (icons will go in \$datadir/icons/hicolor/...)"
echo "                                    (bitmaps will go in \$datadir/games/xboard/...)"
echo "        infodir:         $infodir   (info files will go here)"
echo "        sysconfdir:      $sysconfdir   (xboard.conf will go here)"
echo ""
echo "        NLS support:     $USE_NLS"
echo ""
echo "        ptys:            $enable_ptys"
echo "        zippy:           $enable_zippy"
echo "        sigint:          $enable_sigint"
