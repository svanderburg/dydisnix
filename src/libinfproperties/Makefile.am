AM_CPPFLAGS = -DDATADIR="\"$(prefix)/share/dydisnix\""

pkglib_LTLIBRARIES = libinfproperties.la
pkginclude_HEADERS = targetstable2.h

libinfproperties_la_SOURCES = targetstable2.c
libinfproperties_la_CFLAGS = $(LIBXML2_CFLAGS) $(DISNIX_CFLAGS) $(GLIB_CFLAGS)
libinfproperties_la_LIBADD = $(DISNIX_LIBS) $(LIBXSLT_LIBS) $(GLIB_LIBS) -linfrastructure -lprocreact
