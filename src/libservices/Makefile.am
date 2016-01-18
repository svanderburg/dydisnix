AM_CPPFLAGS = -DDATADIR="\"$(prefix)/share/dydisnix\""

pkglib_LTLIBRARIES = libservices.la
pkginclude_HEADERS = serviceproperties.h

libservices_la_SOURCES = serviceproperties.c
libservices_la_CFLAGS = $(LIBXML2_CFLAGS) $(DISNIX_CFLAGS) $(GLIB_CFLAGS)
libservices_la_LIBADD = $(DISNIX_LIBS) $(LIBXSLT_LIBS) $(GLIB_LIBS) -lxmlutil
