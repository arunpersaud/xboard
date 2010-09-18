
if ZIPPY
  ZPY = zippy.c zippy.h
else
  ZPY =
endif

bin_PROGRAMS = xboard
xboard_SOURCES = backend.c backend.h backendz.h \
	 	 book.c \
		 childio.c childio.h \
	 	 common.h \
		 frontend.h \
	  	 gamelist.c \
	 	 gettext.h  \
		 lists.c lists.h \
		 moves.c moves.h \
		 parser.l parser.h \
		 pgntags.c \
	  	 uci.c \
	 	 xboard.c xboard.h args.h \
		 xedittags.c xedittags.h \
		 engineoutput.c engineoutput.h \
		 xengineoutput.c \
		 evalgraph.c evalgraph.h xevalgraph.c xevalgraph.h\
	 	 xgamelist.c xgamelist.h\
		 xhistory.c  xhistory.h \
		 xoptions.c \
		 filebrowser/selfile.c filebrowser/selfile.h \
		 filebrowser/draw.c filebrowser/path.c \
		 filebrowser/dir.c filebrowser/xstat.h \
		 $(ZPY)
xboard_LDADD = -lm @XAW_LIBS@ @X_LIBS@ 

EXTRA_DIST = pixmaps bitmaps winboard sounds filebrowser/README \
	xboard.texi gpl.texinfo texi2man texinfo.tex xboard.man \
	COPYRIGHT FAQ.html engine-intf.html ics-parsing.txt readme.htm readme_HGM.txt zippy.README

DISTCLEANFILES = stamp-h

AM_CPPFLAGS=-DINFODIR='"$(infodir)"' @X_CFLAGS@  -DSYSCONFDIR='"$(sysconfdir)"'

info_TEXINFOS =  xboard.texi
xboard_TEXINFOS =  copyright.texi 
man6_MANS = xboard.man

dist_sysconf_DATA = xboard.conf

icondir = $(datadir)/icons/hicolor/48x48/apps
dist_icon_DATA = xboard.png
svgicondir = $(datadir)/icons/hicolor/scalable/apps
dist_svgicon_DATA = xboard.svg


xboard.man: xboard.texi copyright.texi gpl.texinfo version.texi
	$(srcdir)/texi2man $(srcdir)/xboard.texi > xboard.man || (rm -f xboard.man ; false)
