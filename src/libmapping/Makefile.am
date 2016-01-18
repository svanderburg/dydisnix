pkglib_LTLIBRARIES = libmapping.la
pkginclude_HEADERS = candidatetargetmapping.h

libmapping_la_SOURCES = candidatetargetmapping.c
libmapping_la_CFLAGS = $(LIBXML2_CFLAGS) $(GLIB_CFLAGS) $(DISNIX_CFLAGS)
libmapping_la_LIBADD = $(GLIB_LIBS) $(DISNIX_LIBS) -lxmlutil
