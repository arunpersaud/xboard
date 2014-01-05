dnl| configure.in
dnl|
dnl| Copyright 1992-2001, 2002, 2003, 2004, 2005, 2006, 2007,
dnl| 2008, 2009, 2010, 2011, 2012, 2013, 2014 Free Software Foundation, Inc.
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
AC_INIT([xboard],[4.7.3],[bug-xboard@gnu.org])

dnl| need this to be able to compile some files in a subdir (filebrowser)
AM_INIT_AUTOMAKE([subdir-objects])
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
AH_TEMPLATE([HAVE_LIBXPM],[template])
AH_TEMPLATE([USE_XAW3D],[template])
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

dnl | add compiler warnings only if compiler understands them
AC_MSG_CHECKING(whether compiler understands -Wall -Wno-parentheses)
SAVE_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS -Wall -Wno-parentheses"
AC_TRY_COMPILE([],[],
  AC_MSG_RESULT(yes),
  AC_MSG_RESULT(no)
  CFLAGS="$SAVE_CFLAGS")

dnl | check for pkg-config
AC_CHECK_PROGS([PKGCONFIG], [pkg-config], pkgconfig_not_found)
AS_IF( [test "x$PKGCONFIG" = xpkgconfig_not_found],
       AC_MSG_ERROR([cannot find pkg-config! Please install it.]))

dnl | check for cairo and librsvg
PKG_CHECK_MODULES([CAIRO], [ cairo >= 1.2.0 librsvg-2.0 >= 2.14.0 ])
AC_SUBST(CAIRO_CFLAGS)
AC_SUBST(CAIRO_LIBS)

dnl | check which front end to use
dnl | first check for gtk
dnl | then for Xaw3d
dnl | and then for Xaw
FRONTEND_CFLAGS=""
FRONTEND_LIBS=""

AC_ARG_WITH([gtk],
            [AS_HELP_STRING([--with-gtk],[use GTK front-end (experimental)])],
            [with_GTK=yes],
            [with_GTK=no])

AC_ARG_WITH([Xaw3d],
            [AS_HELP_STRING([--with-Xaw3d],[use Xaw3d front-end (not fully supported anymore)])],
            [with_Xaw3d=yes],
            [with_Xaw3d=no])

AC_ARG_WITH([Xaw],
            [AS_HELP_STRING([--with-Xaw],[use Xaw front-end (default)])],
            [with_Xaw=yes],
            [with_Xaw=no])

dnl | check for libraries
if test x"$with_GTK" = x"yes" ; then
  PKG_CHECK_MODULES([GTK], [ gtk+-2.0 >= 2.16.0 gmodule-2.0 ])
  FRONTEND_CFLAGS=$GTK_CFLAGS
  FRONTEND_LIBS=$GTK_LIBS
fi

if test x"$with_GTK" = x"no" ; then

dnl | general check for Xaw and Xaw3d
  AC_PATH_XTRA
  if test -n "$no_x" ; then
    AC_MSG_ERROR([ "    $PACKAGE requires the X Window System header files and libraries!
    They were not found on your system.  See FAQ topic C.2.
    configure failed"])
  fi

  save_cflags="$CFLAGS"
  save_cppflags="$CPPFLAGS"
  CFLAGS="$CFLAGS $X_CFLAGS"
  CPPFLAGS="$CPPFLAGS $X_CFLAGS"
  AC_CHECK_HEADER([X11/Intrinsic.h],[xt="yes"],[xt="no"])
  CFLAGS="$save_cflags"
  CPPFLAGS="$save_cppflags"

  if test "$xt" = "no" ; then
     AC_MSG_ERROR([Xt headers not found. Please install the X11 packages.])
  fi

dnl | check availability of Xaw3d
  FRONTEND_LIBS=
  AS_IF([test x"$with_Xaw3d" != x"no"],
  	    [AC_CHECK_LIB([Xaw3d],
                    [XawTextReplace],
                    [ FRONTEND_LIBS="-lXaw3d"
                      AC_DEFINE([USE_XAW3D], [1], [Define if you want to use Xaw3d])],
                    [AC_MSG_ERROR([--with-Xaw3d was given, but test for Xaw3d failed. Make sure that you have Xaw3d installed, else you might want to try the configure option --without-Xaw3d])],
			       [-lXaw])])

dnl | no Xaw3d, check for Xaw; make it the default
  if test x"$with_Xaw3d" = x"no" ; then
     with_Xaw=yes
     save_cflags="$CFLAGS"
     save_cppflags="$CPPFLAGS"
     CFLAGS="$CFLAGS $X_CFLAGS"
     CPPFLAGS="$CPPFLAGS $X_CFLAGS"
     AC_CHECK_HEADER([X11/Xaw/Dialog.h],[xaw_headers="yes"], [xaw_headers="no"])
     CFLAGS="$save_cflags"
     CPPFLAGS="$save_cppflags"

     if test x"$xaw_headers" = x"no" ; then
        AC_MSG_ERROR([Xaw headers not found. Please install the Xaw package and headers.])
     fi

     FRONTEND_LIBS="-lXaw"
  fi

fi

dnl | make results available in Makefile.am
AM_CONDITIONAL([withGTK],   [test x"$with_GTK" = x"yes"])
AM_CONDITIONAL([withXaw],   [test x"$with_Xaw3d" = x"yes"])
AM_CONDITIONAL([withXaw],   [test x"$with_Xaw" = x"yes"])


AC_SUBST(FRONTEND_CFLAGS)
AC_SUBST(FRONTEND_LIBS)

dnl | end Front-end check

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

    if test -d /opt/hppd/include/X11; then
      X_CFLAGS="$X_CFLAGS -I/opt/hppd/include"
      X_LIBS="$X_LIBS -L/opt/hppd/lib"
    elif test -d /usr/contrib/X11R5/include; then
      X_CFLAGS="$X_CFLAGS -I/usr/contrib/X11R5/include"
      X_LIBS="$X_LIBS -L/usr/contrib/X11R5/lib"
    elif test -d /usr/contrib/mitX11R5/include; then
      X_CFLAGS="$X_CFLAGS -I/usr/contrib/mitX11R5/include"
      X_LIBS="$X_LIBS -L/usr/contrib/mitX11R5/lib"
    elif test -d /MIT/X11R5/include; then
      X_CFLAGS="$X_CFLAGS -I/MIT/X11R5/include"
      X_LIBS="$X_LIBS -L/MIT/X11R5/lib"
    elif test -d /usr/local/include/X11R5; then
      X_CFLAGS="$X_CFLAGS -I/usr/local/include/X11R5"
      X_LIBS="$X_LIBS -L/usr/local/lib/X11R5"
    fi
    if test -d /usr/include/X11R5; then
      X_CFLAGS="$X_CFLAGS -I/usr/include/X11R5"
      X_LIBS="$X_LIBS -L/usr/lib/X11R5 -L/usr/lib/X11R4"
    elif test -d /usr/include/X11R4; then
      X_CFLAGS="$X_CFLAGS -I/usr/include/X11R4"
      X_LIBS="$X_LIBS -L/usr/lib/X11R4"
    fi
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

  *-*-sunos5* | *-*-solaris2* )
dnl| USE_PTYS=1

dnl| I'm not sure -lelf is needed, but it was in the old Imakefile.
dnl| The other libraries should all be found by Ac_PATH_XTRA or other
dnl| code above.

    X_LIBS="$X_LIBS -lelf"
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

AC_ARG_ENABLE(	[xpm],
		[AS_HELP_STRING([--enable-xpm],[libXpm will be used if found (default)])],
		[enable_xpm="$enableval"],
		[enable_xpm="yes"])

if test x"$enable_xpm" != "xno"; then
  save_cflags="$CFLAGS"
  save_cppflags="$CPPFLAGS"
  CFLAGS="$CFLAGS $X_CFLAGS"
  CPPFLAGS="$CPPFLAGS $X_CFLAGS"
  AC_CHECK_HEADERS(X11/xpm.h)
  CFLAGS="$save_cflags"
  CPPFLAGS="$save_cppflags"
  if test "$ac_cv_header_X11_xpm_h" = "yes"; then
    save_ldflags="$LDFLAGS"
    LDFLAGS="$LDFLAGS $X_LIBS"
    AC_CHECK_LIB(Xpm, XpmReadFileToPixmap,
                 [X_PRE_LIBS="-lXpm $X_PRE_LIBS"; AC_DEFINE(HAVE_LIBXPM)], [],
                 [$X_PRE_LIBS -lX11 $X_EXTRA_LIBS])
    LDFLAGS="$save_ldflags"
  fi
fi

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
AC_ARG_ENABLE( [zippy],
	       [AS_HELP_STRING([--enable-zippy],[support interfacing a chess program to ICS (default)])],
	       [],
	       [enable_zippy="no"])

if test x"$enable_zippy" != xno; then
  AC_DEFINE(ZIPPY, 1,[should zippy be enabled])
else
  AC_DEFINE(ZIPPY, 0,[should zippy be enabled])
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


if test x"$with_Xaw3d" = x"yes" -o  x"$with_Xaw" = x"yes" ; then
dnl | save all information for X in X_LIBS, also add the libraries themself
dnl | since AC_PATH_XTRA only adds the paths
X_LIBS="$X_PRE_LIBS $X_LIBS $X_EXTRA_LIBS -lXmu -lX11 -lXt "
AC_SUBST(X_CFLAGS)
AC_SUBST(X_LIBS)
fi

dnl | should we install desktop icons and mime associations?
AC_ARG_ENABLE(	[update-mimedb],
		[AS_HELP_STRING([--disable-update-mimedb],[disable the update-mime-database after install [default=no]])],
		[enable_mimedb="no"],
		[enable_mimedb="yes"])

AM_CONDITIONAL(ENABLE_UPDATE_MIMEDB, test x"$enable_mimedb" = "xyes")

if test x"$enable_mimedb" = "xyes"; then
   AC_PATH_PROG([XDG_MIME],[xdg-mime])
   AC_SUBST(XDG_MIME)
   AC_PATH_PROG([XDG_DESKTOP_MENU],[xdg-desktop-menu])
   AC_SUBST(XDG_DESKTOP_MENU)
   AC_PATH_PROG([XDG_ICON_RESOURCE],[xdg-icon-resource])
   AC_SUBST(XDG_ICON_RESOURCE)
fi

dnl |  output Makefile
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
echo "        update mimedb:   $enable_mimedb"
echo ""
echo "        NLS support:     $USE_NLS"
echo ""
echo "        GTK:             $with_GTK"
echo "        Xaw3d:           $with_Xaw3d"
echo "        Xaw:             $with_Xaw"
echo ""
echo "        xpm:             $enable_xpm"
echo "        ptys:            $enable_ptys"
echo "        zippy:           $enable_zippy"
echo "        sigint:          $enable_sigint"
