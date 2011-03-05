### if we are compiling with ZIPPY we need some extra source files

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
	xboard.texi gpl.texinfo texi2man texinfo.tex xboard.man xboard.desktop \
	COPYRIGHT FAQ.html engine-intf.html ics-parsing.txt readme.htm zippy.README \
	xboard.conf.in SHORTLOG DIFFSTAT history.c

DISTCLEANFILES = stamp-h

AM_CPPFLAGS=-DINFODIR='"$(infodir)"' @X_CFLAGS@  -DSYSCONFDIR='"$(sysconfdir)"' -DBITMAPDIR='"$(datadir)/games/xboard/bitmaps/"'

info_TEXINFOS =  xboard.texi
xboard_TEXINFOS =  copyright.texi 
man6_MANS = xboard.man


### config file

sysconf_DATA = xboard.conf


### icon files

icondir = $(datadir)/icons/hicolor/48x48/apps
dist_icon_DATA = xboard.png
svgicondir = $(datadir)/icons/hicolor/scalable/apps
dist_svgicon_DATA = xboard.svg


### desktop files for KDE and gnome

Applicationsdir = $(datadir)/applications
Applications_DATA = xboard.desktop


### directory and files to be used as bitmaps

bitmapdir = $(datadir)/games/xboard
dist_bitmap_DATA = pixmaps/wood_d.xpm pixmaps/wood_l.xpm


### set correct dir in xboard.conf

edit = sed -e 's|@bitmapdir[@]|$(bitmapdir)|g'

xboard.conf: Makefile $(srcdir)/xboard.conf.in
	@echo '  'GEN '  ' $@;
	@rm -f $@ $@.tmp
	@$(edit) $(srcdir)/$@.in >$@.tmp
	@mv $@.tmp $@

distclean-local:
	-rm -f xboard.conf

### the man page depends on several files, so we add a special rule

xboard.man: xboard.texi copyright.texi gpl.texinfo version.texi
	$(srcdir)/texi2man $(srcdir)/xboard.texi > xboard.man || (rm -f xboard.man ; false)
