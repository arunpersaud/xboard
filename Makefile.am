
if ZIPPY
  ZPY = zippy.c zippy.h
else
  ZPY =
endif

bin_PROGRAMS = xboard

xboard_SOURCES = backend.c backend.h backendz.h \
	 	 book.c \
		 callback.c callback.h \
		 childio.c childio.h \
	 	 common.h \
		 frontend.h \
	  	 gamelist.c \
	 	 gettext.h  \
		 interface.c interface.h \
		 lists.c lists.h \
		 moves.c moves.h \
		 parser.l parser.h \
		 pgntags.c \
	  	 uci.c \
	 	 xboard.c xboard.h args.h \
		 xedittags.c xedittags.h \
		 engineoutput.c engineoutput.h \
		 xengineoutput.c \
		 evalgraph.c evalgraph.h xevalgraph.c \
	 	 xgamelist.c xgamelist.h\
		 xhistory.c  xhistory.h \
		 xoptions.c \
		 $(ZPY)

xboard_LDADD = -lm  @XAW_LIBS@ @GTK_LIBS@

EXTRA_DIST = pixmaps bitmaps winboard\
	xboard.texi gpl.texinfo texi2man texinfo.tex xboard.man \
	COPYRIGHT FAQ.html engine-intf.html ics-parsing.txt readme.htm readme_HGM.txt zippy.README

DISTCLEANFILES = stamp-h

AM_CPPFLAGS=-DINFODIR='"$(infodir)"' @GTK_CFLAGS@

info_TEXINFOS =  xboard.texi
xboard_TEXINFOS =  copyright.texi 
man6_MANS = xboard.man

install-data-local: $(srcdir)/xboard.conf
	$(INSTALL_DATA) $(srcdir)/xboard.conf $(DESTDIR)/etc/xboard

xboard.man: xboard.texi copyright.texi gpl.texinfo version.texi
	$(srcdir)/texi2man $(srcdir)/xboard.texi > xboard.man || (rm -f xboard.man ; false)
