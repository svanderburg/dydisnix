AM_CPPFLAGS = -DDATADIR="\"$(prefix)/share/dydisnix\""

pkglib_LTLIBRARIES = libgraph.la
pkginclude_HEADERS = node.h

libgraph_la_SOURCES = node.c
libgraph_la_CFLAGS = $(LIBXML2_CFLAGS) $(DISNIX_CFLAGS) $(GLIB_CFLAGS)
libgraph_la_LIBADD = $(GLIB_LIBS) $(DISNIX_LIBS) -lnixxml
